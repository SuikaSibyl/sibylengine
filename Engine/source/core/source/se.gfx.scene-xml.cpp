#include <tinyparser-mitsuba.h>
#include "se.gfx.hpp"
#include "se.gfx.scene-loader.hpp"
#include "se.editor.hpp"
#include <filesystem>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#include <happly/happly.hpp>

namespace se {
namespace gfx {

  /// To support spectral data, we need to convert spectral measurements (how much energy at each wavelength) to
  /// RGB. To do this, we first convert the spectral data to CIE XYZ, by
  /// integrating over the XYZ response curve. Here we use an analytical response
  /// curve proposed by Wyman et al.: https://jcgt.org/published/0002/02/01/
  inline float xFit_1931(float wavelength) {
    float t1 = (wavelength - float(442.0)) * ((wavelength < float(442.0)) ? float(0.0624) : float(0.0374));
    float t2 = (wavelength - float(599.8)) * ((wavelength < float(599.8)) ? float(0.0264) : float(0.0323));
    float t3 = (wavelength - float(501.1)) * ((wavelength < float(501.1)) ? float(0.0490) : float(0.0382));
    return float(0.362) * exp(-float(0.5) * t1 * t1) +
      float(1.056) * exp(-float(0.5) * t2 * t2) -
      float(0.065) * exp(-float(0.5) * t3 * t3);
  }
  inline float yFit_1931(float wavelength) {
    float t1 = (wavelength - float(568.8)) * ((wavelength < float(568.8)) ? float(0.0213) : float(0.0247));
    float t2 = (wavelength - float(530.9)) * ((wavelength < float(530.9)) ? float(0.0613) : float(0.0322));
    return float(0.821) * exp(-float(0.5) * t1 * t1) +
      float(0.286) * exp(-float(0.5) * t2 * t2);
  }
  inline float zFit_1931(float wavelength) {
    float t1 = (wavelength - float(437.0)) * ((wavelength < float(437.0)) ? float(0.0845) : float(0.0278));
    float t2 = (wavelength - float(459.0)) * ((wavelength < float(459.0)) ? float(0.0385) : float(0.0725));
    return float(1.217) * exp(-float(0.5) * t1 * t1) +
      float(0.681) * exp(-float(0.5) * t2 * t2);
  }
  inline vec3 XYZintegral_coeff(float wavelength) {
    return Vector3{ xFit_1931(wavelength), yFit_1931(wavelength), zFit_1931(wavelength) };
  }

  inline vec3 integrate_XYZ(const std::vector<std::pair<float, float>>& data) {
    static const float CIE_Y_integral = 106.856895;
    static const float wavelength_beg = 400;
    static const float wavelength_end = 700;
    if (data.size() == 0) {
      return vec3{ 0, 0, 0 };
    }
    vec3 ret = vec3{ 0, 0, 0 };
    int data_pos = 0;
    // integrate from wavelength 400 nm to 700 nm, increment by 1nm at a time
    // linearly interpolate from the data
    for (float wavelength = wavelength_beg; wavelength <= wavelength_end; wavelength += float(1)) {
      // assume the spectrum data is sorted by wavelength
      // move data_pos such that wavelength is between two data or at one end
      while (data_pos < (int)data.size() - 1 &&
        !((data[data_pos].first <= wavelength &&
          data[data_pos + 1].first > wavelength) ||
          data[0].first > wavelength)) {
        data_pos += 1;
      }
      float measurement = 0;
      if (data_pos < (int)data.size() - 1 && data[0].first <= wavelength) {
        float curr_data = data[data_pos].second;
        float next_data = data[std::min(data_pos + 1, (int)data.size() - 1)].second;
        float curr_wave = data[data_pos].first;
        float next_wave = data[std::min(data_pos + 1, (int)data.size() - 1)].first;
        // linearly interpolate
        measurement = curr_data * (next_wave - wavelength) / (next_wave - curr_wave) +
          next_data * (wavelength - curr_wave) / (next_wave - curr_wave);
      }
      else {
        // assign the endpoint
        measurement = data[data_pos].second;
      }
      vec3 coeff = XYZintegral_coeff(wavelength);
      ret += coeff * measurement;
    }
    float wavelength_span = wavelength_end - wavelength_beg;
    ret *= (wavelength_span / (CIE_Y_integral * (wavelength_end - wavelength_beg)));
    return ret;
  }

  inline vec3 XYZ_to_RGB(const vec3& xyz) {
    return vec3{
        float(3.240479) * xyz[0] - float(1.537150) * xyz[1] - float(0.498535) * xyz[2],
        float(-0.969256) * xyz[0] + float(1.875991) * xyz[1] + float(0.041556) * xyz[2],
        float(0.055648) * xyz[0] - float(0.204043) * xyz[1] + float(1.057311) * xyz[2] };
  }

  vec3 spectrum_to_rgb(TPM_NAMESPACE::Spectrum spec) {
    std::vector<std::pair<float, float>> vec(spec.wavelengths().size());
    for (int i = 0; i < spec.wavelengths().size(); ++i) {
      vec[i] = { spec.wavelengths()[i], spec.weights()[i] };
    }
    vec3 xyz = integrate_XYZ(vec);
    return XYZ_to_RGB(xyz);
  }

  struct xmlLoaderEnv {
    std::string directory;
    std::unordered_map<TPM_NAMESPACE::Object const*, TextureHandle>   textures;
    std::unordered_map<TPM_NAMESPACE::Object const*, MaterialHandle>  materials;
    std::unordered_map<TPM_NAMESPACE::Object const*, MediumHandle>    mediums;
  };

  auto loadXMLTextures(TPM_NAMESPACE::Object const* node,
    xmlLoaderEnv* env) noexcept -> TextureHandle {
    if (env->textures.find(node) != env->textures.end()) {
      return env->textures[node];
    }
    if (node->type() != TPM_NAMESPACE::OT_TEXTURE) {
      se::error("GFX :: Mitsuba Loader :: Try load texture node not actually texture.");
    }
    std::string filename = node->property("filename").getString();
    std::string tex_path = env->directory + "/" + filename;
    TextureHandle texture = gfx::GFXContext::load_texture_file(tex_path);
    env->textures[node] = texture;
    return texture;
  }

  auto loadXMLMaterial(TPM_NAMESPACE::Object const* node,
    xmlLoaderEnv* env) noexcept -> gfx::MaterialHandle {
    PROFILE_SCOPE_FUNCTION();
    if (env->materials.find(node) != env->materials.end()) {
      return env->materials[node];
    }
    if (node->type() != TPM_NAMESPACE::OT_BSDF) {
      se::error("gfx :: XML Loader :: Try load material node, but the node is actually not bsdf.");
      return gfx::MaterialHandle{};
    }

    TPM_NAMESPACE::Object const* mat_node = nullptr;
    if (node->pluginType() == "dielectric") {
      mat_node = node;
    }
    else if (node->pluginType() == "roughdielectric") {
      mat_node = node;
    }
    else if (node->pluginType() == "thindielectric") {
      mat_node = node;
    }
    else if (node->pluginType() == "twosided") {
      if (node->anonymousChildren().size() == 0) {
        se::error("Mitsuba Loader :: Material loading exception.");
        return gfx::MaterialHandle{};
      }
      mat_node = node->anonymousChildren()[0].get();
    }
    else if (node->pluginType() == "mask") {
      if (node->anonymousChildren().size() == 0) {
        se::error("Mitsuba Loader :: Material loading exception.");
        return gfx::MaterialHandle{};
      }
      return loadXMLMaterial(node->anonymousChildren()[0].get(), env);
    }
    else if (node->pluginType() == "bumpmap") {
      if (node->anonymousChildren().size() == 0) {
        se::error("Mitsuba Loader :: Material loading exception.");
        return gfx::MaterialHandle{};
      }
      return loadXMLMaterial(node->anonymousChildren()[0].get(), env);
    }
    else {
      mat_node = node;
    }

    MaterialHandle mat = GFXContext::create_material_empty();
    std::string name = std::string(node->id());
    if (name == "") name = "unnamed material";
    mat->m_name = name;

    if (mat_node->pluginType() == "roughplastic") {
      mat->m_packet.bxdfType = 3;
      float eta = mat_node->property("int_ior").getNumber(1.5f);
      float alpha = mat_node->property("alpha").getNumber(1.f);
      TPM_NAMESPACE::Color reflectance = mat_node->property("diffuse_reflectance").getColor({ 1.f, 1.f,1.f });
      mat->m_packet.vec4Data0 = { reflectance.r, reflectance.g, reflectance.b, 1.0 };
      mat->m_packet.vec4Data1.w = alpha;
      TPM_NAMESPACE::Color spec_reflectance = mat_node->property("specular_reflectance").getColor({ 1.f, 1.f,1.f });
      mat->m_packet.vec4Data2 = { spec_reflectance.r, spec_reflectance.g, spec_reflectance.b, eta };
    }
    else if (mat_node->pluginType() == "diffuse") {
      mat->m_packet.bxdfType = 0;
      if (mat_node->property("reflectance").type() == TPM_NAMESPACE::PT_COLOR) {
        TPM_NAMESPACE::Color reflectance = mat_node->property("reflectance").getColor({ 1.f, 1.f,1.f });
        mat->m_packet.vec4Data0 = { reflectance.r, reflectance.g, reflectance.b, 1.0 };
      }
      else if (mat_node->property("reflectance").type() == TPM_NAMESPACE::PT_SPECTRUM) {
        TPM_NAMESPACE::Spectrum reflectance = mat_node->property("reflectance").getSpectrum();
        if (reflectance.isUniform()) {
          mat->m_packet.vec4Data0 = { reflectance.uniformValue(), 1.0 };
        }
        else {
          mat->m_packet.vec4Data0 = spectrum_to_rgb(reflectance);
        }
      }
      else {
        mat->m_packet.vec4Data0 = vec4(1, 1, 1, 1);
      }
    }
    else if (mat_node->pluginType() == "roughconductor") {
      mat->m_packet.bxdfType = 1;
      TPM_NAMESPACE::Color eta = mat_node->property("eta").getColor();
      TPM_NAMESPACE::Color k = mat_node->property("k").getColor();
      float alpha = mat_node->property("alpha").getNumber(1.f);
      mat->m_packet.vec4Data0 = vec4{ k.r, k.g, k.b, 1.0 };
      mat->m_packet.vec4Data1.w = alpha;
      TPM_NAMESPACE::Color spec = mat_node->property("specular_reflectance").getColor();
      mat->m_packet.vec4Data2 = vec4{ eta.r, eta.g, eta.b, spec.r };
    }
    else if (mat_node->pluginType() == "roughdielectric") {
      auto look_up_ior = [](std::string const& name) -> float {
        if (name == "bk7") return 1.5046;
        if (name == "air") return 1.000277;
        return 1.5046;
      };

      mat->m_packet.bxdfType = 2;
      float alpha = mat_node->property("alpha").getNumber(1.f);
      std::string int_ior = mat_node->property("int_ior").getString();
      std::string ext_ior = mat_node->property("ext_ior").getString();

      mat->m_packet.vec4Data1.w = alpha;
      mat->m_packet.vec4Data2.w = look_up_ior(int_ior) / look_up_ior(ext_ior);
    }
    else {
      mat->m_packet.bxdfType = 0;
      mat->m_packet.vec4Data0 = vec4{ 1,1,1,1 };
    }

    for (auto const& child : mat_node->namedChildren()) {
      if (child.first == "diffuse_reflectance" ||
        child.first == "reflectance") {
        mat->m_basecolorTex = loadXMLTextures(child.second.get(), env);
      }
    }
    env->materials[node] = mat;
    return mat;
  }

  auto load_obj_mesh(std::string path, Scene& scene) noexcept -> MeshHandle {
    PROFILE_SCOPE_FUNCTION();
    // load obj file
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path =
      std::filesystem::path(path).parent_path().string();  // Path to material files
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path, reader_config)) {
      if (!reader.Error().empty()) {
        se::error("TinyObjReader: " + reader.Error());
      }
      return MeshHandle{};
    }
    if (!reader.Warning().empty()) {
      se::warn("TinyObjReader: " + reader.Warning());
    }
    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    std::vector<float> vertexBufferV = {};
    std::vector<float> positionBufferV = {};
    std::vector<uint32_t> indexBufferWV = {};
    positionBufferV.reserve(attrib.vertices.size());
    vertexBufferV.reserve(attrib.vertices.size() / 3 * 8);
    indexBufferWV.reserve(attrib.vertices.size());

    // create mesh resource
    MeshHandle mesh = GFXContext::create_mesh_empty();

    // check whether tangent is need in mesh attributes
    bool needTangent = false;

    // Loop over shapes
    uint64_t global_index_offset = 0;
    uint32_t submesh_vertex_offset = 0, submesh_index_offset = 0;
    for (size_t s = 0; s < shapes.size(); s++) {
      uint32_t vertex_offset = 0;
      std::unordered_map<uint64_t, uint32_t> uniqueVertices{};
      vec3 position_max = vec3(-1e9);
      vec3 position_min = vec3(1e9);
      // Loop over faces(polygon)
      size_t index_offset = 0;
      for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
        size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
        // require tangent
        if (fv != 3) {
          se::error(
            "GFX :: SceneNodeLoader_obj :: non-triangle geometry not "
            "supported when required TANGENT attribute now.");
          return MeshHandle{};
        }
        vec3 tangent;
        vec3 bitangent;
        if (needTangent) {
          vec3 positions[3];
          vec3 normals[3];
          vec2 uvs[3];
          for (size_t v = 0; v < fv; v++) {
            // index finding
            tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
            positions[v] = { attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                            attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                            attrib.vertices[3 * size_t(idx.vertex_index) + 2] };
            if (attrib.normals.size() == 0) {
              normals[v] = { 0, 0, 0 };
            }
            else {
              normals[v] = { attrib.normals[3 * size_t(idx.normal_index) + 0],
                            attrib.normals[3 * size_t(idx.normal_index) + 1],
                            attrib.normals[3 * size_t(idx.normal_index) + 2] };
            }
            if (attrib.texcoords.size() == 0) {
              uvs[v] = { 0, 0 };
            }
            else {
              uvs[v] = { attrib.texcoords[2 * size_t(idx.texcoord_index) + 0],
                        -attrib.texcoords[2 * size_t(idx.texcoord_index) + 1] };

            }
          }
          vec3 edge1 = positions[1] - positions[0];
          vec3 edge2 = positions[2] - positions[0];
          vec2 deltaUV1 = uvs[1] - uvs[0];
          vec2 deltaUV2 = uvs[2] - uvs[0];

          float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

          tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
          tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
          tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
          tangent = normalize(tangent);

          bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
          bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
          bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
          bitangent = normalize(bitangent);
        }
        // Loop over vertices in the face.
        for (size_t v = 0; v < fv; v++) {
          // index finding
          tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
          // atrributes filling
          std::vector<float> vertex = {};
          std::vector<float> position = {};
          for (auto const& entry : defaultMeshDataLayout.layout) {
            // vertex position
            if (entry.info == MeshDataLayout::VertexInfo::POSITION) {
              if (entry.format == rhi::VertexFormat::FLOAT32X3) {
                tinyobj::real_t vx =
                  attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy =
                  attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz =
                  attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                //vertex.push_back(vx);
                //vertex.push_back(vy);
                //vertex.push_back(vz);
                position_min = se::min(position_min, vec3{ vx,vy,vz });
                position_max = se::max(position_max, vec3{ vx,vy,vz });
                if (defaultMeshLoadConfig.usePositionBuffer) {
                  position.push_back(vx);
                  position.push_back(vy);
                  position.push_back(vz);
                }
              }
              else {
                se::error(
                  "GFX :: SceneNodeLoader_obj :: unwanted vertex format for "
                  "POSITION attributes.");
                return MeshHandle{};
              }
            }
            else if (entry.info == MeshDataLayout::VertexInfo::NORMAL) {
              // Check if `normal_index` is zero or positive. negative = no
              // normal data
              if (idx.normal_index >= 0) {
                tinyobj::real_t nx =
                  attrib.normals[3 * size_t(idx.normal_index) + 0];
                tinyobj::real_t ny =
                  attrib.normals[3 * size_t(idx.normal_index) + 1];
                tinyobj::real_t nz =
                  attrib.normals[3 * size_t(idx.normal_index) + 2];
                vertex.push_back(nx);
                vertex.push_back(ny);
                vertex.push_back(nz);
              }
              else {
                vertex.push_back(0);
                vertex.push_back(0);
                vertex.push_back(0);
              }
            }
            else if (entry.info == MeshDataLayout::VertexInfo::UV) {
              if (idx.texcoord_index >= 0) {
                tinyobj::real_t tx =
                  attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                tinyobj::real_t ty =
                  attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                vertex.push_back(tx);
                vertex.push_back(1 - ty);
              }
              else {
                vertex.push_back(0);
                vertex.push_back(0);
              }
            }
            else if (entry.info == MeshDataLayout::VertexInfo::TANGENT) {
              if (isnan(tangent.x) || isnan(tangent.y) || isnan(tangent.z)) {
                vertex.push_back(0);
                vertex.push_back(0);
                vertex.push_back(0);
              }
              else {
                vertex.push_back(tangent.x);
                vertex.push_back(tangent.y);
                vertex.push_back(tangent.z);
              }
            }
            else if (entry.info == MeshDataLayout::VertexInfo::COLOR) {
              // Optional: vertex colors
              tinyobj::real_t red =
                attrib.colors[3 * size_t(idx.vertex_index) + 0];
              tinyobj::real_t green =
                attrib.colors[3 * size_t(idx.vertex_index) + 1];
              tinyobj::real_t blue =
                attrib.colors[3 * size_t(idx.vertex_index) + 2];
              vertex.push_back(red);
              vertex.push_back(green);
              vertex.push_back(blue);
            }
            else if (entry.info == MeshDataLayout::VertexInfo::CUSTOM) {
            }
          }

          if (defaultMeshLoadConfig.deduplication) {
            se::error("obj vertex deduplication is not impl yet.");
          }
          else {
            vertexBufferV.insert(vertexBufferV.end(), vertex.begin(),
              vertex.end());
            positionBufferV.insert(positionBufferV.end(), position.begin(),
              position.end());
            // index filling
            if (defaultMeshLoadConfig.layout.format == rhi::IndexFormat::UINT16_t)
              indexBufferWV.push_back(vertex_offset);
            else if (defaultMeshLoadConfig.layout.format == rhi::IndexFormat::UINT32_T)
              indexBufferWV.push_back(vertex_offset);
            ++vertex_offset;
          }
        }
        index_offset += fv;
        // per-face material
        shapes[s].mesh.material_ids[f];
      }
      global_index_offset += index_offset;

      // load Material
      Mesh::MeshPrimitive sePrimitive;
      sePrimitive.offset = submesh_index_offset;
      sePrimitive.size = index_offset;
      sePrimitive.baseVertex = submesh_vertex_offset;
      sePrimitive.numVertex = positionBufferV.size() / 3 - submesh_vertex_offset;
      sePrimitive.max = position_max;
      sePrimitive.min = position_min;
      mesh.get()->m_primitives.emplace_back(std::move(sePrimitive));
      // todo:: add material
      submesh_index_offset = global_index_offset;
      submesh_vertex_offset += positionBufferV.size() / 3 - submesh_vertex_offset;
    }
    { // register mesh
      PROFILE_SCOPE_NAME(UploadGPUBuffer);
      MiniBuffer buffer;
      buffer.m_isReference = true;

      buffer.m_data = positionBufferV.data();
      buffer.m_size = sizeof(float) * positionBufferV.size();

      bool need_rt = bool(gfx::GFXContext::device()->from_which_adapter()->from_which_context()
        ->get_context_extensions_flags() & rhi::ContextExtensionEnum::RAY_TRACING);
      Flags<rhi::BufferUsageEnum> rt_usage = 0;
      if (need_rt) rt_usage |= rhi::BufferUsageEnum::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY;

      mesh->m_positionBuffer = gfx::GFXContext::create_buffer_host(buffer,
        rhi::BufferUsageEnum::STORAGE |
        rhi::BufferUsageEnum::SHADER_DEVICE_ADDRESS | rt_usage);
      mesh->m_positionBuffer->m_job = "Mesh position buffer";
      mesh->m_positionBuffer->m_host.resize(buffer.m_size);
      memcpy(mesh->m_positionBuffer->m_host.data(), buffer.m_data, buffer.m_size);

      buffer.m_data = indexBufferWV.data();
      buffer.m_size = sizeof(uint32_t) * indexBufferWV.size();
      mesh->m_indexBuffer = gfx::GFXContext::create_buffer_host(buffer,
        rhi::BufferUsageEnum::INDEX |
        rhi::BufferUsageEnum::SHADER_DEVICE_ADDRESS | rt_usage);
      mesh->m_indexBuffer->m_job = "Mesh index buffer";
      mesh->m_indexBuffer->m_host.resize(buffer.m_size);
      memcpy(mesh->m_indexBuffer->m_host.data(), buffer.m_data, buffer.m_size);

      buffer.m_data = vertexBufferV.data();
      buffer.m_size = sizeof(uint32_t) * vertexBufferV.size();
      mesh->m_vertexBuffer = gfx::GFXContext::create_buffer_host(buffer,
        rhi::BufferUsageEnum::STORAGE |
        rhi::BufferUsageEnum::SHADER_DEVICE_ADDRESS);
      mesh->m_vertexBuffer->m_job = "Mesh vertex buffer";
      mesh->m_vertexBuffer->m_host.resize(buffer.m_size);
      memcpy(mesh->m_vertexBuffer->m_host.data(), buffer.m_data, buffer.m_size);
      PROFILE_SCOPE_STOP(UploadGPUBuffer);
    }
    return mesh;
  }

  auto loadPlyMesh(std::string path, Scene& scene) noexcept -> MeshHandle {
    PROFILE_SCOPE_FUNCTION();
    // Construct a data object by reading from file
    happly::PLYData plyIn(path);
    // Get mesh-style data from the object
    std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
    std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

    std::vector<float> vertexBufferV = {};
    std::vector<float> positionBufferV = {};
    std::vector<uint32_t> indexBufferWV = {};
    // create mesh resource
    MeshHandle mesh = GFXContext::create_mesh_empty();

    // translate the shape to se format
    uint32_t vertex_offset = 0;
    vec3 position_max = vec3(-1e9);
    vec3 position_min = vec3(1e9);
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < fInd.size(); f++) {
      std::vector<size_t> const& faceInd = fInd[f];
      // require tangent
      if (faceInd.size() != 3) {
        se::error(
          "GFX :: SceneNodeLoader_obj :: non-triangle geometry not "
          "supported when required TANGENT attribute now.");
        return MeshHandle{};
      }

      vec3 tangent = {};
      vec3 bitangent = {};

      vec3 positions[3];
      for (size_t v = 0; v < 3; v++) {
        // index finding
        size_t idx = faceInd[index_offset + v];
        positions[v] = { float(vPos[idx][0]), float(vPos[idx][1]), float(vPos[idx][2]) };
      }
      vec3 edge1 = positions[1] - positions[0];
      vec3 edge2 = positions[2] - positions[0];
      vec3 normal = normalize(cross(edge1, edge2));

      // Loop over vertices in the face.
      for (size_t v = 0; v < faceInd.size(); v++) {
        // index finding
        size_t idx = faceInd[index_offset + v];
        // atrributes filling
        std::vector<float> vertex = {};
        std::vector<float> position = {};

        for (auto const& entry : defaultMeshDataLayout.layout) {
          // vertex position
          if (entry.info == MeshDataLayout::VertexInfo::POSITION) {
            if (entry.format == rhi::VertexFormat::FLOAT32X3) {
              float vx = vPos[idx][0];
              float vy = vPos[idx][1];
              float vz = vPos[idx][2];
              vertex.push_back(vx);
              vertex.push_back(vy);
              vertex.push_back(vz);
              position_min = se::min(position_min, vec3{ vx,vy,vz });
              position_max = se::max(position_max, vec3{ vx,vy,vz });
              if (defaultMeshLoadConfig.usePositionBuffer) {
                position.push_back(vx);
                position.push_back(vy);
                position.push_back(vz);
              }
            }
            else {
              se::error(
                "GFX :: SceneNodeLoader_obj :: unwanted vertex format for "
                "POSITION attributes.");
              return MeshHandle{};
            }
          }
          else if (entry.info == MeshDataLayout::VertexInfo::NORMAL) {
            vertex.push_back(normal.x);
            vertex.push_back(normal.y);
            vertex.push_back(normal.z);
          }
          else if (entry.info == MeshDataLayout::VertexInfo::UV) {
            vertex.push_back(0);
            vertex.push_back(0);
          }
          else if (entry.info == MeshDataLayout::VertexInfo::TANGENT) {
            vertex.push_back(tangent.x);
            vertex.push_back(tangent.y);
            vertex.push_back(tangent.z);
          }
          else if (entry.info == MeshDataLayout::VertexInfo::COLOR) {
            vertex.push_back(0);
            vertex.push_back(0);
            vertex.push_back(0);
          }
          else if (entry.info == MeshDataLayout::VertexInfo::CUSTOM) {
          }
        }
        //
        {
          vertexBufferV.insert(vertexBufferV.end(), vertex.begin() + 3, vertex.end());
          positionBufferV.insert(positionBufferV.end(), position.begin(), position.end());
          // index filling
          if (defaultMeshLoadConfig.layout.format == rhi::IndexFormat::UINT16_t)
            indexBufferWV.push_back(vertex_offset);
          else if (defaultMeshLoadConfig.layout.format == rhi::IndexFormat::UINT32_T)
            indexBufferWV.push_back(vertex_offset);
          ++vertex_offset;
        }
      }

    }

    index_offset += fInd.size() * 3;

    // load Material
    Mesh::MeshPrimitive sePrimitive;
    sePrimitive.offset = 0;
    sePrimitive.size = index_offset;
    sePrimitive.baseVertex = 0;
    sePrimitive.numVertex = positionBufferV.size() / 3 - 0;
    sePrimitive.max = position_max;
    sePrimitive.min = position_min;
    mesh.get()->m_primitives.emplace_back(std::move(sePrimitive));

    { // register mesh
      MiniBuffer buffer;
      buffer.m_isReference = true;

      buffer.m_data = positionBufferV.data();
      buffer.m_size = sizeof(float) * positionBufferV.size();

      bool need_rt = bool(gfx::GFXContext::device()->from_which_adapter()->from_which_context()
        ->get_context_extensions_flags() & rhi::ContextExtensionEnum::RAY_TRACING);
      Flags<rhi::BufferUsageEnum> rt_usage = 0;
      if (need_rt) rt_usage |= rhi::BufferUsageEnum::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY;

      mesh->m_positionBuffer = gfx::GFXContext::create_buffer_host(buffer,
        rhi::BufferUsageEnum::STORAGE |
        rhi::BufferUsageEnum::SHADER_DEVICE_ADDRESS | rt_usage);
      mesh->m_positionBuffer->m_job = "Mesh position buffer";

      buffer.m_data = indexBufferWV.data();
      buffer.m_size = sizeof(uint32_t) * indexBufferWV.size();
      mesh->m_indexBuffer = gfx::GFXContext::create_buffer_host(buffer,
        rhi::BufferUsageEnum::INDEX |
        rhi::BufferUsageEnum::SHADER_DEVICE_ADDRESS | rt_usage);
      mesh->m_indexBuffer->m_job = "Mesh index buffer";

      buffer.m_data = vertexBufferV.data();
      buffer.m_size = sizeof(uint32_t) * vertexBufferV.size();
      mesh->m_vertexBuffer = gfx::GFXContext::create_buffer_host(buffer,
        rhi::BufferUsageEnum::STORAGE |
        rhi::BufferUsageEnum::SHADER_DEVICE_ADDRESS);
      mesh->m_vertexBuffer->m_job = "Mesh vertex buffer";
    }
    return mesh;
  }

  auto loadXMLMesh(TPM_NAMESPACE::Object const* node, xmlLoaderEnv * env,
    Node & gfxNode, Scene* scene) -> void {
    PROFILE_SCOPE_FUNCTION();
    if (node->type() != TPM_NAMESPACE::OT_SHAPE) {
      se::error("gfx :: xml loader :: try to load shape, but xml node is not a shape");
      return;
    }

    auto handle_material_medium = [&node, &env, &gfxNode, &scene](MeshRenderer& mesh_renderer) {
      std::optional<MaterialHandle> mat;
      if (node->anonymousChildren().size() > 0) {
        se::vec3 radiance = se::vec3(0);
        for (auto& subnode : node->anonymousChildren()) {
          if (subnode->type() == TPM_NAMESPACE::OT_BSDF) {
            // material
            TPM_NAMESPACE::Object* mat_node = subnode.get();
            mat = loadXMLMaterial(mat_node, env);
          }
          else if (subnode->type() == TPM_NAMESPACE::OT_EMITTER) {
            if (subnode->pluginType() == "area") {
              bool is_rgb;
              TPM_NAMESPACE::Color rgb = subnode->property("radiance").
                getColor(TPM_NAMESPACE::Color(0, 0, 0), &is_rgb);
              if (is_rgb) radiance = se::vec3(rgb.r, rgb.g, rgb.b);
              bool is_float;
              float intensity = subnode->property("radiance").
                getNumber(0.f, &is_float);
              if (is_float) radiance = se::vec3(intensity);
              bool is_spectrum;
              TPM_NAMESPACE::Spectrum spectrum = subnode->property("radiance").
                getSpectrum({}, &is_spectrum);
              if (is_spectrum && spectrum.isUniform())
                radiance = se::vec3(spectrum.uniformValue());
              else if (is_spectrum)
                radiance = spectrum_to_rgb(spectrum);
            }
          }
        }
        if (mat.has_value()) {
          if (radiance.r == 0
            && radiance.g == 0
            && radiance.b == 0) {
          }
          else {
            MaterialHandle mat_copy = GFXContext::create_material_empty();
            *(mat_copy.get()) = *(mat.value().get());
            mat = mat_copy;
            mat.value()->m_packet.vec4Data1 = radiance;
          }
          for (auto& primitive : mesh_renderer.m_mesh->m_primitives) {
            primitive.material = mat.value();
          }
          for (auto& primitive : mesh_renderer.m_mesh->m_customPrimitives) {
            primitive.material = mat.value();
          }
        }

        if (radiance.r > 0 || radiance.g > 0 || radiance.b > 0) {
          auto& light_component = gfxNode.add_component<Light>();
          //light_component.primitives.push_back(0);
          light_component.light.light_type = LightTypeEnum::MESH_PRIMITIVE;

          if (!mat.has_value()) {
            mat = GFXContext::create_material_empty();
            mat.value()->m_packet.bxdfType = 0;
            mat.value()->m_packet.vec4Data1 = radiance;
            for (auto& primitive : mesh_renderer.m_mesh->m_primitives) {
              primitive.material = mat.value();
            }
            for (auto& primitive : mesh_renderer.m_mesh->m_customPrimitives) {
              primitive.material = mat.value();
            }
          }
        }
      }

      for (auto& subnode : node->namedChildren()) {
        if (subnode.first == "exterior") {
          for (auto& primitive : mesh_renderer.m_mesh->m_primitives)
            primitive.exterior = env->mediums[subnode.second.get()];
          for (auto& primitive : mesh_renderer.m_mesh->m_customPrimitives)
            primitive.exterior = env->mediums[subnode.second.get()];
        }
        else if (subnode.first == "interior") {
          for (auto& primitive : mesh_renderer.m_mesh->m_primitives)
            primitive.interior = env->mediums[subnode.second.get()];
          for (auto& primitive : mesh_renderer.m_mesh->m_customPrimitives)
            primitive.interior = env->mediums[subnode.second.get()];
        }
      }
    };

    if (node->pluginType() == "obj") {
      std::string filename = node->property("filename").getString();
      std::string obj_path = env->directory + "/" + filename;
      MeshHandle mesh = load_obj_mesh(obj_path, *scene);

      auto& mesh_renderer = gfxNode.add_component<MeshRenderer>();
      mesh_renderer.m_mesh = mesh;

      { // process the transform
        TPM_NAMESPACE::Transform transform =
          node->property("to_world").getTransform();
        se::mat4 mat = {
          transform.matrix[0],  transform.matrix[1],  transform.matrix[2],
          transform.matrix[3],  transform.matrix[4],  transform.matrix[5],
          transform.matrix[6],  transform.matrix[7],  transform.matrix[8],
          transform.matrix[9],  transform.matrix[10], transform.matrix[11],
          transform.matrix[12], transform.matrix[13], transform.matrix[14],
          transform.matrix[15] };
        //mat = se::transpose(mat);
        se::vec3 t, s; se::Quaternion quat;
        se::decompose(mat, &t, &quat, &s);

        auto* transformComponent = gfxNode.get_component<Transform>();
        transformComponent->translation = t;
        transformComponent->scale = s;
        transformComponent->rotation = quat;
      }
      handle_material_medium(mesh_renderer);
    }
    else if (node->pluginType() == "ply") {
      std::string filename = node->property("filename").getString();
      std::string obj_path = env->directory + "/" + filename;
      MeshHandle mesh = loadPlyMesh(obj_path, *scene);

      auto& mesh_renderer = gfxNode.add_component<MeshRenderer>();
      mesh_renderer.m_mesh = mesh;

      { // process the transform
        TPM_NAMESPACE::Transform transform =
          node->property("to_world").getTransform();
        se::mat4 mat = {
          transform.matrix[0],  transform.matrix[1],  transform.matrix[2],
          transform.matrix[3],  transform.matrix[4],  transform.matrix[5],
          transform.matrix[6],  transform.matrix[7],  transform.matrix[8],
          transform.matrix[9],  transform.matrix[10], transform.matrix[11],
          transform.matrix[12], transform.matrix[13], transform.matrix[14],
          transform.matrix[15] };
        //mat = se::transpose(mat);
        se::vec3 t, s; se::Quaternion quat;
        se::decompose(mat, &t, &quat, &s);

        auto* transformComponent = gfxNode.get_component<Transform>();
        transformComponent->translation = t;
        transformComponent->scale = s;
        transformComponent->rotation = quat;
      }
      handle_material_medium(mesh_renderer);
    }
    else if (node->pluginType() == "cube") {
      std::string engine_path = Configuration::string_property("engine_path");
      std::string obj_path = engine_path + "assets/meshes/cube.obj";
      MeshHandle mesh = load_obj_mesh(obj_path, *scene);
      mesh->m_customPrimitives.emplace_back();
      Mesh::CustomPrimitive& cube_primitive = mesh->m_customPrimitives.back();
      cube_primitive.primitiveType = 3;
      cube_primitive.min = -vec3(1.f);
      cube_primitive.max = vec3(1.f);
      
      auto& mesh_renderer = gfxNode.add_component<MeshRenderer>();
      mesh_renderer.m_mesh = mesh;

      { // process the transform
        TPM_NAMESPACE::Transform transform =
          node->property("to_world").getTransform();
        se::mat4 mat = {
          transform.matrix[0],  transform.matrix[1],  transform.matrix[2],
          transform.matrix[3],  transform.matrix[4],  transform.matrix[5],
          transform.matrix[6],  transform.matrix[7],  transform.matrix[8],
          transform.matrix[9],  transform.matrix[10], transform.matrix[11],
          transform.matrix[12], transform.matrix[13], transform.matrix[14],
          transform.matrix[15] };
        //mat = se::transpose(mat);
        se::vec3 t, s; se::Quaternion quat;
        se::decompose(mat, &t, &quat, &s);

        auto* transformComponent = gfxNode.get_component<Transform>();
        transformComponent->translation = t;
        transformComponent->scale = s;
        transformComponent->rotation = quat;
      }

      handle_material_medium(mesh_renderer);
    }
    else if (node->pluginType() == "rectangle") {
      std::string engine_path = Configuration::string_property("engine_path");
      std::string obj_path = engine_path + "assets/meshes/rectangle.obj";
      MeshHandle mesh = load_obj_mesh(obj_path, *scene);
      mesh->m_customPrimitives.emplace_back();
      Mesh::CustomPrimitive& rectangle_primitive = mesh->m_customPrimitives.back();
      rectangle_primitive.primitiveType = 2;
      rectangle_primitive.min = -vec3(1.f);
      rectangle_primitive.max = vec3(1.f);

      auto& mesh_renderer = gfxNode.add_component<MeshRenderer>();
      mesh_renderer.m_mesh = mesh;

      { // process the transform
        TPM_NAMESPACE::Transform transform =
          node->property("to_world").getTransform();
        se::mat4 mat = {
          transform.matrix[0],  transform.matrix[1],  transform.matrix[2],
          transform.matrix[3],  transform.matrix[4],  transform.matrix[5],
          transform.matrix[6],  transform.matrix[7],  transform.matrix[8],
          transform.matrix[9],  transform.matrix[10], transform.matrix[11],
          transform.matrix[12], transform.matrix[13], transform.matrix[14],
          transform.matrix[15] };
        //mat = se::transpose(mat);
        se::vec3 t, s; se::Quaternion quat;
        se::decompose(mat, &t, &quat, &s);

        auto* transformComponent = gfxNode.get_component<Transform>();
        transformComponent->translation = t;
        transformComponent->scale = s;
        transformComponent->rotation = quat;
      }

      handle_material_medium(mesh_renderer);
    }
    else if (node->pluginType() == "sphere") {
      std::string engine_path = Configuration::string_property("engine_path");
      std::string obj_path = engine_path + "assets/meshes/sphere.obj";
      MeshHandle mesh = load_obj_mesh(obj_path, *scene);
      mesh->m_customPrimitives.emplace_back();
      Mesh::CustomPrimitive& sphere_primitive = mesh->m_customPrimitives.back();
      sphere_primitive.primitiveType = 1;
      sphere_primitive.min = -vec3(1.f);
      sphere_primitive.max = vec3(1.f);

      auto& mesh_renderer = gfxNode.add_component<MeshRenderer>();
      mesh_renderer.m_mesh = mesh;

      { // process the transform
        TPM_NAMESPACE::Transform transform =
          node->property("to_world").getTransform();
        float radius = node->property("radius").getNumber();
        TPM_NAMESPACE::Vector center = node->property("center").getVector();
        auto* transformComponent = gfxNode.get_component<Transform>();
        transformComponent->translation = vec3(center.x, center.y, center.z);
        transformComponent->scale = vec3(radius);
        transformComponent->rotation = Quaternion();
      }
      handle_material_medium(mesh_renderer);
    }
    else {
      se::error("gfx :: xml loader :: Unkown mesh type:" + node->pluginType());
    }
  }

	auto Scene::load_xml(std::string const& path) noexcept -> void {
    TPM_NAMESPACE::SceneLoader loader;
    try {
      PROFILE_SCOPE_NAME(XMLRead);
      auto scene_xml = loader.loadFromFile(path.c_str());
      xmlLoaderEnv env;
      env.directory = std::filesystem::path(path).parent_path().string();
      PROFILE_SCOPE_STOP(XMLRead);

      auto process_xml_node = [&](
        TPM_NAMESPACE::Object* obj
        ) {
          switch (obj->type()) {
          case TPM_NAMESPACE::OT_SCENE:
            break;
          case TPM_NAMESPACE::OT_BSDF:
            //loadMaterial(node, env);
            break;
          case TPM_NAMESPACE::OT_FILM:
            break;
          case TPM_NAMESPACE::OT_INTEGRATOR:
            break;
          case TPM_NAMESPACE::OT_MEDIUM: {
            //MediumHandle medium = GFXContext::load_medium_empty();
            //medium->packet.scale = obj->property("scale").getNumber(1.f);
            //if (obj->pluginType() == "homogeneous") {
            //  medium->packet.bitfield = (uint32_t)Medium::MediumType::Homogeneous;
            //  TPM_NAMESPACE::Color sigma_a = obj->property("sigma_a").getColor();
            //  TPM_NAMESPACE::Color sigma_s = obj->property("sigma_s").getColor();
            //  medium->packet.sigmaA = { sigma_a.r, sigma_a.g, sigma_a.b };
            //  medium->packet.sigmaS = { sigma_s.r, sigma_s.g, sigma_s.b };
            //  medium->packet.scale = obj->property("scale").getNumber();
            //  medium->packet.type = Medium::MediumType::Homogeneous;
            //}
            //else if (obj->pluginType() == "heterogeneous") {
            //  bounds3 bound;
            //  for (auto child : obj->namedChildren()) {
            //    std::string name = child.first;
            //    if (name == "albedo") {
            //      mitsuba_parse_volume_spectrum(child.second.get(), medium);
            //    }
            //    else if (name == "density") {
            //      //mitsuba_parse_volume_spectrum(child.second.get(), medium);
            //      auto* node = child.second.get();
            //      std::string type = node->pluginType();
            //      if (type == "constvolume") {
            //        TPM_NAMESPACE::Color rgb = node->property("value").getColor();
            //        medium->packet.sigmaS = { rgb.r, rgb.g, rgb.b };
            //        medium->packet.sigmaA = { 1.f - rgb.r, 1.f - rgb.g, 1.f - rgb.b };
            //      }
            //      else if (type == "gridvolume") {
            //        std::string filename = node->property("filename").getString();
            //        medium->density = mitsuba_volume_loader(env.directory + "/" + filename);
            //        bound = unionBounds(bound, medium->density->bounds);
            //      }
            //      else {
            //        root::print::error(std::string("Unknown volume type:") + type);
            //      }
            //    }
            //  }

            //  medium->packet.type = Medium::MediumType::GridMedium;
            //  medium->packet.bound_min = bound.pMin;
            //  medium->packet.bound_max = bound.pMax;

            //  // create majorant grid
            //  medium->majorantGrid = Medium::MajorantGrid();
            //  medium->majorantGrid->res = ivec3(16, 16, 16);
            //  medium->majorantGrid->bounds = { medium->packet.bound_min, medium->packet.bound_max };
            //  medium->majorantGrid->voxels.resize(16 * 16 * 16);
            //  // Initialize _majorantGrid_ for _GridMedium_
            //  for (int z = 0; z < medium->majorantGrid->res.z; ++z)
            //    for (int y = 0; y < medium->majorantGrid->res.y; ++y)
            //      for (int x = 0; x < medium->majorantGrid->res.x; ++x) {
            //        bounds3 bounds = medium->majorantGrid->voxel_bounds(x, y, z);
            //        medium->majorantGrid->set(x, y, z, medium->density->max_value(bounds));
            //      }
            //}
            //env.mediums[obj] = medium;
            break;
          }
          case TPM_NAMESPACE::OT_PHASE:
            break;
          case TPM_NAMESPACE::OT_RFILTER:
            break;
          case TPM_NAMESPACE::OT_SAMPLER:
            break;
          case TPM_NAMESPACE::OT_SENSOR: {
            Node node = create_node(obj->id());
            auto& camera = node.add_component<Camera>();
            camera.zfar = 1000.f;
            camera.znear = 0.02f;
            camera.yfov = obj->property("fov").getNumber();
            camera.aspectRatio = 1.f;

            for (auto& child : obj->anonymousChildren()) {
              if (child->type() == TPM_NAMESPACE::OT_FILM) {
                int width = child->property("width").getInteger();
                int height = child->property("height").getInteger();
                //scene->resolution = { width,height };
                camera.aspectRatio = width * 1. / height;
                if (obj->property("fov_axis").getString() == "x") {
                  float tmp = width / std::tan(radians(camera.yfov) * 0.5);
                  camera.yfov = 2 * degrees(std::atan(height * 1. / tmp));
                }
              }
            }

            auto* transformComponent = node.get_component<Transform>();
            auto transform = obj->property("to_world").getTransform();
            se::mat4 transform_mat = {
              transform.matrix[0], transform.matrix[1], transform.matrix[2], transform.matrix[3],
              transform.matrix[4], transform.matrix[5], transform.matrix[6], transform.matrix[7],
              transform.matrix[8], transform.matrix[9], transform.matrix[10], transform.matrix[11],
              transform.matrix[12], transform.matrix[13], transform.matrix[14], transform.matrix[15]
            };
            se::vec3 t, s; Quaternion q;
            se::Transform rotate = se::rotate_y(180);
            se::decompose(transform_mat, &t, &q, &s);
            transformComponent->translation = t;
            transformComponent->scale = s;
            transformComponent->rotation = Quaternion(rotate.m) * q;

            for (auto& subnode : obj->namedChildren()) {
              if (subnode.second->type() == TPM_NAMESPACE::OT_MEDIUM)
                camera.medium = env.mediums[subnode.second.get()];
            }
            for (auto& subnode : obj->anonymousChildren()) {
              if (subnode->type() == TPM_NAMESPACE::OT_MEDIUM)
                camera.medium = env.mediums[subnode.get()];
            }

            m_roots.push_back(node);
            break;
          }
          case TPM_NAMESPACE::OT_SHAPE: {
            Node node = create_node(obj->id());
            loadXMLMesh(obj, &env, node, this);
            m_roots.push_back(node);
            break;
          }
          case TPM_NAMESPACE::OT_SUBSURFACE:
            break;
          case TPM_NAMESPACE::OT_TEXTURE:
            break;
          case TPM_NAMESPACE::OT_VOLUME:
            break;
          case TPM_NAMESPACE::_OT_COUNT:
            break;
          default:
            break;
          }
      };

      for (auto& object : scene_xml.anonymousChildren()) {
        process_xml_node(object.get());
      }
      for (auto& object : scene_xml.namedChildren()) {
        process_xml_node(object.second.get());
      }

    }
    catch (...) {
      se::error("gfx :: load xml failed!");
    }
	}
}
}
#include "se.gfx.hpp"
#include "se.gfx.scene-loader.hpp"
#include "se.editor.hpp"
#include <filesystem>
#include "ex.tinyprbrtloader.hpp"
#include <nanovdb/NanoVDB.h>
#define NANOVDB_USE_ZIP 1
#include <nanovdb/util/IO.h>

namespace se {
namespace gfx {

  auto nanovdb_float_grid_loader(nanovdb::GridHandle<nanovdb::HostBuffer>& grid) -> Medium::SampledGrid {
    nanovdb::NanoGrid<float>* floatGrid = grid.grid<float>();
    float minValue, maxValue;
    floatGrid->tree().extrema(minValue, maxValue);
    nanovdb::Vec3dBBox bbox = floatGrid->worldBBox();

    auto grid_bounds = floatGrid->indexBBox();
    int nx = floatGrid->indexBBox().dim()[0];
    int ny = floatGrid->indexBBox().dim()[1];
    int nz = floatGrid->indexBBox().dim()[2];

    std::vector<float> values;

    int z0 = grid_bounds.min()[2], z1 = grid_bounds.max()[2];
    int y0 = grid_bounds.min()[1], y1 = grid_bounds.max()[1];
    int x0 = grid_bounds.min()[0], x1 = grid_bounds.max()[0];

    bounds3 bounds = bounds3(vec3(bbox.min()[0], bbox.min()[1], bbox.min()[2]),
      vec3(bbox.max()[0], bbox.max()[1], bbox.max()[2]));

    int downsample = 0;
    // Fix the resolution to be a multiple of 2^downsample just to make
    // downsampling easy. Chop off one at a time from the bottom and top
    // of the range until we get there; the bounding box is updated as
    // well so that the remaining volume doesn't shift spatially.
    auto round = [=](int& low, int& high, float& c0, float& c1) {
      float delta = (c1 - c0) / (high - low);
      int mult = 1 << downsample; // want a multiple of this in resolution
      while ((high - low) % mult) {
        ++low;
        c0 += delta;
        if ((high - low) % mult) {
          --high;
          c1 -= delta;
        }
      }
      return high - low;
    };
    nz = round(z0, z1, bounds.pMin.z, bounds.pMax.z);
    ny = round(y0, y1, bounds.pMin.y, bounds.pMax.y);
    nx = round(x0, x1, bounds.pMin.x, bounds.pMax.x);

    int x_mid = (x0 + x1) / 2;
    int y_mid = (y0 + y1) / 2;
    int z_mid = (z0 + z1) / 2;

    for (int z = z0; z < z1; ++z)
      for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x) {
          values.push_back(floatGrid->getAccessor().getValue({ x, y, z }));
        }

    while (downsample > 0) {
      std::vector<float> v2;
      for (int z = 0; z < nz / 2; ++z)
        for (int y = 0; y < ny / 2; ++y)
          for (int x = 0; x < nx / 2; ++x) {
            auto v = [&](int dx, int dy, int dz) -> float {
              return values[(2 * x + dx) + nx * ((2 * y + dy) + ny * (2 * z + dz))];
            };
            v2.push_back((v(0, 0, 0) + v(1, 0, 0) + v(0, 1, 0) + v(1, 1, 0) +
              v(0, 0, 1) + v(1, 0, 1) + v(0, 1, 1) + v(1, 1, 1)) / 8);
          }

      values = std::move(v2);
      nx /= 2;
      ny /= 2;
      nz /= 2;
      --downsample;
    }

    Medium::SampledGrid sgrid;
    sgrid.nx = nx;
    sgrid.ny = ny;
    sgrid.nz = nz;
    sgrid.values = std::move(values);
    sgrid.bounds = bounds;
    return sgrid;
  }

  auto nanovdb_loader(std::string file_name, MediumHandle& medium) noexcept -> void {
    auto list = nanovdb::io::readGridMetaData(file_name);
    bounds3 bound;
    for (auto& m : list) {
      std::string grid_name = m.gridName;
      if (grid_name == "density") {
        nanovdb::GridHandle<nanovdb::HostBuffer> handle = nanovdb::io::readGrid(file_name, m.gridName);
        medium->density = nanovdb_float_grid_loader(handle);
        bound = unionBounds(bound, medium->density.value().bounds);
      }
      if (grid_name == "temperature") {
        nanovdb::GridHandle<nanovdb::HostBuffer> handle = nanovdb::io::readGrid(file_name, m.gridName);
        medium->temperatureGrid = nanovdb_float_grid_loader(handle);
        bound = unionBounds(bound, medium->temperatureGrid.value().bounds);
      }
    }
    medium->packet.boundMin = bound.pMin;
    medium->packet.boundMax = bound.pMax;
  }

  std::string loadFileAsString(const std::string& filePath) {
    std::ifstream file(filePath); // Open the file
    if (!file) {
      throw std::runtime_error("Unable to open file");
    }
    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the file's content into the stringstream
    return buffer.str(); // Return the string content
  }

  auto loadPbrtDefineddMesh(std::vector<tiny_pbrt_loader::Point3f> p,
    std::vector<int> indices, Scene& scene) noexcept -> MeshHandle {
    // load obj file
    std::vector<float> vertexBufferV = {};
    std::vector<float> positionBufferV = {};
    std::vector<uint32_t> indexBufferWV = {};
    // create mesh resource
    MeshHandle mesh = GFXContext::create_mesh_empty();

    // check whether tangent is need in mesh attributes
    bool needTangent = false;
    for (auto const& entry : defaultMeshDataLayout.layout)
      if (entry.info == MeshDataLayout::VertexInfo::TANGENT) needTangent = true;

    // Loop over shapes
    uint64_t global_index_offset = 0;
    uint32_t submesh_vertex_offset = 0, submesh_index_offset = 0;
    for (size_t s = 0; s < 1; s++) {
      int index_offset = 0;
      uint32_t vertex_offset = 0;
      std::unordered_map<uint64_t, uint32_t> uniqueVertices{};
      vec3 position_max = vec3(-1e9);
      vec3 position_min = vec3(1e9);
      // Loop over faces(polygon)
      for (size_t f = 0; f < indices.size() / 3; f++) {
        vec3 normal;
        vec3 tangent;
        vec3 bitangent;
        if (needTangent) {
          vec3 positions[3];
          vec3 normals[3];
          vec2 uvs[3];
          for (size_t v = 0; v < 3; v++) {
            // index finding
            int idx = indices[f * 3 + v];
            positions[v] = { p[idx].v[0], p[idx].v[1], p[idx].v[2] };
            normals[v] = { 0, 0, 0 };
            uvs[v] = { 0, 0 };
          }
          vec3 edge1 = positions[1] - positions[0];
          vec3 edge2 = positions[2] - positions[0];
          vec2 deltaUV1 = uvs[1] - uvs[0];
          vec2 deltaUV2 = uvs[2] - uvs[0];

          normal = normalize(cross(edge1, edge2));

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
        for (size_t v = 0; v < 3; v++) {
          // index finding
          int idx = indices[f * 3 + v];
          // atrributes filling
          std::vector<float> vertex = {};
          std::vector<float> position = {};
          for (auto const& entry : defaultMeshDataLayout.layout) {
            // vertex position
            if (entry.info == MeshDataLayout::VertexInfo::POSITION) {
              if (entry.format == rhi::VertexFormat::FLOAT32X3) {
                float vx = p[idx].v[0];
                float vy = p[idx].v[1];
                float vz = p[idx].v[2];
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
              // Check if `normal_index` is zero or positive. negative = no
              // normal data
              vertex.push_back(normal.x);
              vertex.push_back(normal.y);
              vertex.push_back(normal.z);
            }
            else if (entry.info == MeshDataLayout::VertexInfo::UV) {
              vertex.push_back(0);
              vertex.push_back(0);
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
            }
            else if (entry.info == MeshDataLayout::VertexInfo::CUSTOM) {
            }
          }

          {
            vertexBufferV.insert(vertexBufferV.end(), vertex.begin() + 3,
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
        index_offset += 3;
        //// per-face material
        //shapes[s].mesh.material_ids[f];
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

  se::mat4 pbrt_mat_to_semat4x4(tiny_pbrt_loader::TransformData const& pbrt_trans) {
    return se::mat4{
    (float)pbrt_trans.m[0][0],  (float)pbrt_trans.m[0][1], (float)pbrt_trans.m[0][2], (float)pbrt_trans.m[0][3],
    (float)pbrt_trans.m[1][0],  (float)pbrt_trans.m[1][1], (float)pbrt_trans.m[1][2], (float)pbrt_trans.m[1][3],
    (float)pbrt_trans.m[2][0],  (float)pbrt_trans.m[2][1], (float)pbrt_trans.m[2][2], (float)pbrt_trans.m[2][3],
    (float)pbrt_trans.m[3][0],  (float)pbrt_trans.m[3][1], (float)pbrt_trans.m[3][2], (float)pbrt_trans.m[3][3], };
  }

  void FillTransfromFromPBRT(tiny_pbrt_loader::TransformData const& pbrt_trans, Transform& transformComponent) {
    se::mat4 mat = {
    (float)pbrt_trans.m[0][0],  (float)pbrt_trans.m[0][1], (float)pbrt_trans.m[0][2], (float)pbrt_trans.m[0][3],
    (float)pbrt_trans.m[1][0],  (float)pbrt_trans.m[1][1], (float)pbrt_trans.m[1][2], (float)pbrt_trans.m[1][3],
    (float)pbrt_trans.m[2][0],  (float)pbrt_trans.m[2][1], (float)pbrt_trans.m[2][2], (float)pbrt_trans.m[2][3],
    (float)pbrt_trans.m[3][0],  (float)pbrt_trans.m[3][1], (float)pbrt_trans.m[3][2], (float)pbrt_trans.m[3][3], };
    //mat = se::transpose(mat);
    se::vec3 t, s; se::Quaternion quat;
    se::decompose(mat, &t, &quat, &s);

    transformComponent.translation = t;
    transformComponent.scale = s;
    transformComponent.rotation = quat;
  }

	auto Scene::load_pbrt(std::string const& path) noexcept -> void {
		std::string fileContent = loadFileAsString(path);
		std::string dir_path = std::filesystem::path(path).parent_path().string();
		std::unique_ptr<tiny_pbrt_loader::BasicScene> scene_pbrt = tiny_pbrt_loader::load_scene_from_string(fileContent, dir_path);
		std::string prefix = dir_path + "/";

    // camera
    {
      auto camera_node = create_node("camera");
      m_roots.push_back(camera_node);
      Transform* transformComponent = camera_node.get_component<Transform>();
      Camera& cameraComponent = camera_node.add_component<Camera>();
      //cameraComponent.yfov = scene_pbrt->camera.cameraFromWorld;
      FillTransfromFromPBRT(scene_pbrt->camera.cameraFromWorld, *transformComponent);
      se::Transform rotate = se::rotate_x(180);
      transformComponent->rotation = Quaternion(rotate.m) * transformComponent->rotation;
      transformComponent->translation.y *= -1;
      cameraComponent.yfov = scene_pbrt->camera.dict.GetOneFloat("fov", 0.f);
    }

    std::vector<std::optional<MaterialHandle>> material_map(scene_pbrt->materials.size());

    for (size_t i = 0; i < scene_pbrt->materials.size(); ++i) {
      auto& material = scene_pbrt->materials[i];
      std::optional<MaterialHandle> handle = std::nullopt;
      if (material.name == "interface") {
      }
      else {
        float a = 1.f;
      }
      material_map[i] = handle;
    }

    std::unordered_map<std::string, MediumHandle> medium_map;

    for (auto& medium : scene_pbrt->mediums) {
      MediumHandle medium_handle = GFXContext::create_medium_empty();
      medium_handle->packet.scale = medium.dict.GetOneFloat("scale", 1.f);
      medium_handle->packet.temperatureScale = medium.dict.GetOneFloat("temperaturescale", 1.f);
      medium_handle->packet.LeScale = medium.dict.GetOneFloat("Lescale", 1.f);
      tiny_pbrt_loader::Vector3f sigma_a = medium.dict.GetOneRGB3f("sigma_a", { 0.f ,0.f ,0.f });
      tiny_pbrt_loader::Vector3f sigma_s = medium.dict.GetOneRGB3f("sigma_s", { 0.f ,0.f ,0.f });
      medium_handle->packet.sigmaA = { sigma_a.v[0], sigma_a.v[1] ,sigma_a.v[2] };
      medium_handle->packet.sigmaS = { sigma_s.v[0], sigma_s.v[1] ,sigma_s.v[2] };
      //medium_handle->packet.geometryTransform = medium.dict[]
      std::string type = medium.dict.GetOneString("type", "");
      if (type == "nanovdb") {
        std::string filename = prefix + medium.dict.GetOneString("filename", "");
        nanovdb_loader(filename, medium_handle);
        medium_handle->packet.type = Medium::MediumType::GridMedium;

        // create majorant grid
        medium_handle->majorantGrid = Medium::MajorantGrid();
        medium_handle->majorantGrid->res = ivec3(16, 16, 16);
        medium_handle->majorantGrid->bounds = { medium_handle->packet.boundMin, medium_handle->packet.boundMax };
        medium_handle->majorantGrid->voxels.resize(16 * 16 * 16);
        // Initialize _majorantGrid_ for _GridMedium_
        for (int z = 0; z < medium_handle->majorantGrid->res.z; ++z)
          for (int y = 0; y < medium_handle->majorantGrid->res.y; ++y)
            for (int x = 0; x < medium_handle->majorantGrid->res.x; ++x) {
              bounds3 bounds = medium_handle->majorantGrid->voxel_bounds(x, y, z);
              medium_handle->majorantGrid->set(x, y, z, medium_handle->density->max_value(bounds));
            }
      }
      else if (type == "rgbgrid") {
        medium_handle->packet.type = Medium::MediumType::RGBGridMedium;
        int nx = medium.dict.GetOneInt("nx", 1);
        int ny = medium.dict.GetOneInt("ny", 1);
        int nz = medium.dict.GetOneInt("nz", 1);
        float g = medium.dict.GetOneFloat("g", 0.);
        float scale = medium.dict.GetOneFloat("scale", 1.f);
        medium_handle->packet.sigmaA = { 1,1,1 };
        medium_handle->packet.sigmaS = { 1,1,1 };
        medium_handle->packet.scale = scale;
        medium_handle->packet.aniso = { g };

        std::vector<tiny_pbrt_loader::Float> const& p0 = medium.dict.GetAllFloats("p0");
        std::vector<tiny_pbrt_loader::Float> const& p1 = medium.dict.GetAllFloats("p1");

        bounds3 bound;
        bound.pMin = { float(p0[0]), float(p0[1]), float(p0[2]) };
        bound.pMax = { float(p1[0]), float(p1[1]), float(p1[2]) };
        medium_handle->packet.boundMin = bound.pMin;
        medium_handle->packet.boundMax = bound.pMax;
        medium_handle->packet.geometryTransform = pbrt_mat_to_semat4x4(medium.objectFromRender);
        medium_handle->packet.geometryTransformInverse = pbrt_mat_to_semat4x4(medium.renderFromObject);

        // load sigma_a grid
        {
          Medium::SampledGrid sigma_a;
          sigma_a.nx = nx; sigma_a.ny = ny; sigma_a.nz = nz;
          sigma_a.bounds = bound; sigma_a.gridChannel = 3;
          auto const& sigma_a_double_array = medium.dict.GetAllFloats("sigma_a");
          sigma_a.values.resize(sigma_a_double_array.size());
          for (size_t i = 0; i < sigma_a_double_array.size(); ++i) {
            sigma_a.values[i] = sigma_a_double_array[i];
          }
          medium_handle->density = std::move(sigma_a);

          /*
          int downsample = 8;
          while (downsample > 0) {
            std::vector<float> v2;
            for (int z = 0; z < nz / 2; ++z)
              for (int y = 0; y < ny / 2; ++y)
                for (int x = 0; x < nx / 2; ++x) {
                  auto v = [&](int dx, int dy, int dz) -> float {
                    return values[(2 * x + dx) + nx * ((2 * y + dy) + ny * (2 * z + dz))];
                  };
                  v2.push_back((v(0, 0, 0) + v(1, 0, 0) + v(0, 1, 0) + v(1, 1, 0) +
                    v(0, 0, 1) + v(1, 0, 1) + v(0, 1, 1) + v(1, 1, 1)) / 8);
                }

            values = std::move(v2);
            nx /= 2;
            ny /= 2;
            nz /= 2;
            --downsample;
          }*/
        }

        // load sigma_s grid
        {
          Medium::SampledGrid sigma_s;
          sigma_s.nx = nx; sigma_s.ny = ny; sigma_s.nz = nz;
          sigma_s.bounds = bound; sigma_s.gridChannel = 3;
          auto const& sigma_s_double_array = medium.dict.GetAllFloats("sigma_s");
          sigma_s.values.resize(sigma_s_double_array.size());
          for (size_t i = 0; i < sigma_s_double_array.size(); ++i) {
            sigma_s.values[i] = sigma_s_double_array[i];
          }
          medium_handle->temperatureGrid = std::move(sigma_s);
        }

        // load sigma_s grid

        // create majorant grid
        medium_handle->majorantGrid = Medium::MajorantGrid();
        medium_handle->majorantGrid->res = ivec3(16, 16, 16);
        medium_handle->majorantGrid->bounds = { medium_handle->packet.boundMin, medium_handle->packet.boundMax };
        medium_handle->majorantGrid->voxels.resize(16 * 16 * 16);
        // Initialize _majorantGrid_ for _RGBGridMediumm_
        for (int z = 0; z < medium_handle->majorantGrid->res.z; ++z)
          for (int y = 0; y < medium_handle->majorantGrid->res.y; ++y)
            for (int x = 0; x < medium_handle->majorantGrid->res.x; ++x) {
              bounds3 bounds = medium_handle->majorantGrid->voxel_bounds(x, y, z);
              medium_handle->majorantGrid->set(x, y, z,
                (medium_handle->density->max_value(bounds) + medium_handle->temperatureGrid->max_value(bounds)) * scale
              );
            }

      }

      medium_map[medium.name] = medium_handle;
    }

    auto handle_material_medium = [&material_map, &medium_map](
      tiny_pbrt_loader::ShapeSceneEntity& shape,
      MeshRenderer& mesh_renderer
      ) {
        if (!material_map[shape.materialIndex].has_value()) {

        }
        else {

        }

        if (shape.insideMedium != "") {
          auto& handle = medium_map[shape.insideMedium];
          for (auto& primitive : mesh_renderer.m_mesh->m_primitives)
            primitive.interior = handle;
          for (auto& primitive : mesh_renderer.m_mesh->m_customPrimitives)
            primitive.interior = handle;
        }

        if (shape.outsideMedium != "") {
          auto& handle = medium_map[shape.outsideMedium];
          for (auto& primitive : mesh_renderer.m_mesh->m_primitives)
            primitive.exterior = handle;
          for (auto& primitive : mesh_renderer.m_mesh->m_customPrimitives)
            primitive.exterior = handle;
        }
    };

    for (auto& shape : scene_pbrt->shapes) {
      std::vector<tiny_pbrt_loader::Point3f> p = shape.dict.GetPoint3fArray("P");
      std::vector<int> idx = shape.dict.GetIntArray("indices");
      auto node = create_node(shape.name);
      m_roots.push_back(node);
      NodeProperty* prop = node.get_component<NodeProperty>();
      Transform* transformComponent = node.get_component<Transform>();
      auto& pbrt_trans = shape.renderFromObject;
      FillTransfromFromPBRT(pbrt_trans, *transformComponent);

      // if the mesh is defined by points and indices
      if (idx.size() > 0) {
        MeshRenderer& mesh_renderer = node.add_component<MeshRenderer>();
        mesh_renderer.m_mesh = loadPbrtDefineddMesh(p, idx, *this);
        handle_material_medium(shape, mesh_renderer);
      }
      else if (shape.name == "sphere") {
        const float radius = shape.dict.GetOneFloat("radius", 1.f);
        transformComponent->scale *= {radius, radius, radius};

        std::string engine_path = Configuration::string_property("engine_path");
        std::string obj_path = engine_path + "assets/meshes/sphere.obj";
        MeshHandle mesh = load_obj_mesh(obj_path, *this);
        Mesh::CustomPrimitive sphere_primitive;
        sphere_primitive.primitiveType = 1;
        sphere_primitive.min = -vec3(1.f);
        sphere_primitive.max = vec3(1.f);
        mesh->m_customPrimitives.emplace_back(std::move(sphere_primitive));

        auto& mesh_renderer = node.add_component<MeshRenderer>();
        mesh_renderer.m_mesh = mesh;

        handle_material_medium(shape, mesh_renderer);
      }
    }
	}

}
}
#pragma once
#include <se.rhi.hpp>

namespace se {
namespace gfx {
  /** Mesh data layout */
  struct MeshDataLayout {
    /** info types in vertex */
    enum struct VertexInfo {
      POSITION,
      NORMAL,
      TANGENT,
      UV,
      COLOR,
      CUSTOM,
    };
    /** an entry of the layout */
    struct Entry {
      rhi::VertexFormat format;
      VertexInfo info;
    };
    /* the list of vertex layout */
    std::vector<Entry> layout;
    /* index format */
    rhi::IndexFormat format;
  };

  inline MeshDataLayout defaultMeshDataLayout = { {
    {rhi::VertexFormat::FLOAT32X3,
      gfx::MeshDataLayout::VertexInfo::POSITION},
    {rhi::VertexFormat::FLOAT32X3, gfx::MeshDataLayout::VertexInfo::NORMAL},
    {rhi::VertexFormat::FLOAT32X3, gfx::MeshDataLayout::VertexInfo::TANGENT},
    {rhi::VertexFormat::FLOAT32X2, gfx::MeshDataLayout::VertexInfo::UV},
  },
  rhi::IndexFormat::UINT32_T };

  /** A setting config to guide loading of mesh resource */
  struct MeshLoaderConfig {
    MeshDataLayout layout = {};
    bool usePositionBuffer = true;
    bool residentOnHost = true;
    bool residentOnDevice = false;
    bool deduplication = false;
  };

  inline MeshLoaderConfig defaultMeshLoadConfig = { defaultMeshDataLayout, true, true, false, false };

  auto load_obj_mesh(std::string path, Scene& scene) noexcept -> MeshHandle;
  auto nanovdb_loader(std::string file_name, MediumHandle& medium) noexcept -> void;
}
}
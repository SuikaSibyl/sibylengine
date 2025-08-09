#pragma once
#include <cuda_runtime.h>
#include <se.rhi.hpp>

namespace se {
namespace rhi {
  enum struct DataType {
    Float16,
    Float32,
    Float64,
    UINT8,
    INT8,
    INT16,
    INT32,
    INT64,
  };

  struct CUDAExternalBuffer {
    cudaExternalMemory_t m_cudaMem;
    void* m_dataPtr;
    auto ptr() noexcept -> void* { return (void*)m_dataPtr; }
  };

  struct CUDASemaphore {
    cudaExternalSemaphore_t m_cudaSemaphore;
    rhi::Semaphore* m_rhiSemaphore;
    auto signal(uintptr_t stream_ptr) noexcept -> void;
    auto signal(uintptr_t stream_ptr, size_t signalValue) noexcept -> void;
    auto wait(uintptr_t stream_ptr) noexcept -> void;
    auto wait(uintptr_t stream_ptr, size_t waitValue) noexcept -> void;
  };

  struct CUDAContext {
    static auto initialize(rhi::Device* device) noexcept -> void;
    static auto initialize(std::array<uint64_t, 2> const& uuid) noexcept -> void;
    
    static auto export_to_cuda(
      rhi::Buffer* buffer
    ) noexcept -> std::unique_ptr<CUDAExternalBuffer>;

    static auto export_to_cuda(
      rhi::Semaphore* semaphore
    ) noexcept -> std::unique_ptr<CUDASemaphore>;
  };
}
}
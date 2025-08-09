#include "se.rhi.cuda.hpp"
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <curand.h>
#include <curand_kernel.h>

static const char* _cudaGetErrorEnum(cudaError_t error) {
  return cudaGetErrorName(error);
}

template <typename T>
void check(T result, char const* const func, const char* const file, int const line) {
  if (result) {
    se::error("CUDA error at {0}:{1} code={2}({3}) \"{4}\"", file, line,
      static_cast<unsigned int>(result), _cudaGetErrorEnum(result), func);
    exit(EXIT_FAILURE);
  }
}

#define checkCudaErrors(val) check((val), #val, __FILE__, __LINE__)

namespace se {
namespace rhi {
	auto CUDAContext::initialize(Device* device) noexcept -> void {
		std::array<uint64_t, 2> const uuid = device->query_uuid();
		se::rhi::CUDAContext::initialize(uuid);
	}

	auto CUDAContext::initialize(std::array<uint64_t, 2> const& uuid) noexcept -> void {
    int current_device = 0;
    int device_count = 0;
    int devices_prohibited = 0;
    cudaDeviceProp deviceProp;
    checkCudaErrors(cudaGetDeviceCount(&device_count));

    if (device_count == 0) {
      se::error("CUDA error: no devices supporting CUDA.");
      return;
    }

    // Find the GPU which is selected by Vulkan
    while (current_device < device_count) {
      cudaGetDeviceProperties(&deviceProp, current_device);
      if ((deviceProp.computeMode != cudaComputeModeProhibited)) {
        // Compare the cuda device UUID with vulkan UUID
        int ret = memcmp((void*)&deviceProp.uuid, uuid.data(), 16);
        if (ret == 0) {
          checkCudaErrors(cudaSetDevice(current_device));
          checkCudaErrors(cudaGetDeviceProperties(&deviceProp, current_device));
          se::info("CUDA :: GPU Device {0}: \"{1}\" with compute capability {2}.{3}",
            current_device, deviceProp.name, deviceProp.major, deviceProp.minor);
          break;
        }
      }
      else {
        devices_prohibited++;
      }
      current_device++;
    }

    if (devices_prohibited == device_count) {
      se::error("CUDA error: No Vulkan-CUDA Interop capable GPU found.");
      return;
    }
    int device = current_device;
    cudaDeviceProp prop = {};
    checkCudaErrors(cudaSetDevice(device));
    checkCudaErrors(cudaGetDeviceProperties(&prop, device));
	}

  auto CUDAContext::export_to_cuda(rhi::Buffer* buffer
  ) noexcept -> std::unique_ptr<CUDAExternalBuffer> {
    std::unique_ptr<CUDAExternalBuffer> cubuffer = std::make_unique<CUDAExternalBuffer>();
    se::rhi::Buffer::ExternalHandle handle = buffer->get_mem_handle();
    cudaExternalMemoryHandleDesc externalMemoryHandleDesc = {};
    #ifdef _WIN32
    externalMemoryHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
    externalMemoryHandleDesc.handle.win32.handle = (HANDLE)handle.handle;
    #elif defined(__linux__)
    externalMemoryHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueFd;
    externalMemoryHandleDesc.handle.fd = (int)(uintptr_t)handle.handle;
    #endif
    externalMemoryHandleDesc.size = handle.size + handle.offset;
    checkCudaErrors(cudaImportExternalMemory(&cubuffer->m_cudaMem, &externalMemoryHandleDesc));
    cudaExternalMemoryBufferDesc externalMemBufferDesc = {};
    externalMemBufferDesc.offset = handle.offset;
    externalMemBufferDesc.size = handle.size;
    externalMemBufferDesc.flags = 0;
    void** ptr = (void**)&cubuffer->m_dataPtr;
    checkCudaErrors(cudaExternalMemoryGetMappedBuffer(
      ptr, cubuffer->m_cudaMem, &externalMemBufferDesc));
    return cubuffer;
  }

  auto CUDAContext::export_to_cuda(
    rhi::Semaphore* semaphore
  ) noexcept -> std::unique_ptr<CUDASemaphore> {
    cudaExternalSemaphoreHandleDesc externalSemaphoreHandleDesc = {};
    if (semaphore->m_timelineSemaphore) {
      #ifdef _WIN32
      externalSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreWin32;
      #elif defined(__linux__)
      externalSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreFd;
      #endif
    }
    else {
      #ifdef _WIN32
      externalSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeOpaqueWin32;
      #elif defined(__linux__)
      externalSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeOpaqueFd;
      #endif
    }

    #ifdef _WIN32
    externalSemaphoreHandleDesc.handle.win32.handle = (HANDLE)semaphore->get_handle();
    #elif defined(__linux__)
    externalSemaphoreHandleDesc.handle.fd = (int)(uintptr_t)semaphore->get_handle();
    #endif

    externalSemaphoreHandleDesc.flags = 0;

    std::unique_ptr<CUDASemaphore> cusem = std::make_unique<CUDASemaphore>();
    checkCudaErrors(cudaImportExternalSemaphore(&cusem->m_cudaSemaphore, &externalSemaphoreHandleDesc));
    cusem->m_rhiSemaphore = semaphore;
    return cusem;
  }
  
  auto CUDASemaphore::signal(uintptr_t stream_ptr) noexcept -> void {
    cudaStream_t stream = reinterpret_cast<cudaStream_t>(stream_ptr);
    cudaExternalSemaphoreSignalParams params = {};
    memset(&params, 0, sizeof(params));
    cudaSignalExternalSemaphoresAsync(&m_cudaSemaphore, &params, 1, stream);
  }

  auto CUDASemaphore::signal(uintptr_t stream_ptr, size_t signalValue) noexcept -> void {
    cudaStream_t stream = reinterpret_cast<cudaStream_t>(stream_ptr);
    cudaExternalSemaphoreSignalParams signalParams = {};
    signalParams.flags = 0;
    signalParams.params.fence.value = signalValue;
    // Signal vulkan to continue with the updated buffers
    checkCudaErrors(cudaSignalExternalSemaphoresAsync(&m_cudaSemaphore, &signalParams, 1, stream));
    m_rhiSemaphore->m_currentValue = signalValue;
  }

  auto CUDASemaphore::wait(uintptr_t stream_ptr) noexcept -> void {
    cudaStream_t stream = reinterpret_cast<cudaStream_t>(stream_ptr);
    cudaExternalSemaphoreWaitParams params = {};
    memset(&params, 0, sizeof(params));
    cudaWaitExternalSemaphoresAsync(&m_cudaSemaphore, &params, 1, stream);
  }

  auto CUDASemaphore::wait(uintptr_t stream_ptr, size_t waitValue) noexcept -> void {
    cudaStream_t stream = reinterpret_cast<cudaStream_t>(stream_ptr);
    cudaExternalSemaphoreWaitParams waitParams = {};
    waitParams.flags = 0;
    waitParams.params.fence.value = waitValue;
    // Wait for vulkan to complete it's work
    checkCudaErrors(cudaWaitExternalSemaphoresAsync(&m_cudaSemaphore, &waitParams, 1, stream));
  }

}
}
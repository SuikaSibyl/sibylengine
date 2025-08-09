import os
import torch
import sibylengine as se
se.Configuration.set_config_file(os.path.dirname(os.path.abspath(__file__)) + "/runtime.config")

# define the main function that will execute in the example
def main(device: se.rhi.Device):
    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    # create a cross torch-vulkan tensor and
    # initialize it with a Torch (CUDA) tensor operation
    tensor = se.Tensor([1024, 1], False, se.rhi.DataType.Float32)
    tensor.as_torch().copy_(torch.arange(1024, dtype=torch.float32).reshape(1024, 1))
    # We signal 1 after the CUDA operation is done,
    semaphore = se.TimelineSemaphore()
    semaphore.signal(se.get_torch_cuda_stream(), 1)
    
    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    # start the vulkan compute pass
    # create a command encoder and render context
    square_pass = se.rdg.ComputePass("./shaders/square.slang")
    encoder = device.create_command_encoder()
    ctx = se.rdg.RenderContext(encoder)
    
    # record the compute pass 
    square_pass.update_bindings(ctx, [["rw_output", tensor.get_binding_resource()]])
    pass_encoder = square_pass.begin_pass(ctx)
    pass_encoder.dispatch_workgroups(4, 1, 1)
    pass_encoder.end()
    
    # submit the command encoder to the device,
    # let it first wait for the CUDA stream to be signaled
    # and signal 2 when the compute pass is finished
    device.get_graphics_queue().submit(
        [ encoder.finish() ],
        [ semaphore.get_vk_semaphore() ], [ 1 ],
        [ se.rhi.PipelineStages(se.rhi.PipelineStageEnum.COMPUTE_SHADER_BIT) ],
        [ semaphore.get_vk_semaphore() ], [ 2 ])
    
    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    # wait for the Vulkan compute pass to finish
    semaphore.wait(se.get_torch_cuda_stream(), 2)
    print("Tensor after compute pass:", tensor.as_torch())
    error = torch.mean(torch.abs(tensor.as_torch() - 
        torch.arange(1024, dtype=torch.float32, device='cuda').reshape(1024, 1) ** 2))
    print("L1 error of the result:", error)
    # wait everything to finish before releasing relevant resources
    device.wait_idle()

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# begin the sibylengine
# to enable the interoperation with torch/CUDA, we need to initialize
# with ContextExtensionEnum.CUDA_INTEROPERABILITY
se.gfx.GFXContext.initialize(None,
    se.rhi.ContextExtensionEnum.USE_AFTERMATH |
    se.rhi.ContextExtensionEnum.CUDA_INTEROPERABILITY)
device = se.gfx.GFXContext.device()
# also, we need to initialize the CUDA context
se.rhi.CUDAContext.initialize(device)

# execute the main computation
main(device)

# end the sibylengine
se.gfx.GFXContext.finalize()
print("SE Python interoperation example completed successfully.")
# tensor.py
import os
import torch
from functools import reduce
import operator
import ctypes

from . import pycore  # ensures pycore is loaded
from .pycore import rhi, gfx, Configuration  # directly import what you use

class Tensor:
    def __init__(self, shape=[16], requires_grad=False,
                 dtype: rhi.DataType = rhi.DataType.Float32):
        self.size = reduce(operator.mul, shape, 1)
        device = gfx.GFXContext.device()

        descriptor = rhi.BufferDescriptor(
            self.size * ctypes.sizeof(ctypes.c_float),
            rhi.BufferUsages(rhi.BufferUsageEnum.STORAGE),
            rhi.BufferShareMode.EXCLUSIVE,
            rhi.MemoryPropertys(rhi.MemoryPropertyEnum.DEVICE_LOCAL_BIT)
        )

        self.prim_se: rhi.Buffer = device.create_buffer(descriptor)
        self.prim_cu: rhi.CUDAExternalBuffer = rhi.CUDAContext.export_to_cuda(self.prim_se)
        self.prim_torch: torch.Tensor = rhi.CUDAContext.to_tensor(self.prim_cu, shape, dtype)

        if requires_grad:
            self.grad_se: rhi.Buffer = device.create_buffer(descriptor)
            self.grad_cu: rhi.CUDAExternalBuffer = rhi.CUDAContext.export_to_cuda(self.grad_se)
            self.grad_torch: torch.Tensor = rhi.CUDAContext.to_tensor(self.grad_cu, shape, dtype)
            self.prim_torch.requires_grad = True
            self.prim_torch.grad = self.grad_torch
        else:
            self.grad_se = None
            self.grad_cu = None
            self.grad_torch = None

    def as_torch(self) -> torch.Tensor:
      return self.prim_torch
  
    def prim(self) -> rhi.Buffer:
      return self.prim_se
  
    def grad(self) -> rhi.Buffer:
      return self.grad_se
  
    def get_binding_resource(self) -> rhi.BindingResource:
      return rhi.BindingResource(rhi.BufferBinding(self.prim(), 0, self.prim().size()))
  
    def get_binding_resource_grad(self) -> rhi.BindingResource:
      return rhi.BindingResource(rhi.BufferBinding(self.grad(), 0, self.grad().size()))

class TimelineSemaphore:
    def __init__(self):
        device = gfx.GFXContext.device()
        self.vkSemaphore = device.create_semaphore(True, True)
        self.cudaSemaphore = rhi.CUDAContext.export_to_cuda(self.vkSemaphore)
        self.stamp = 0
        
    def signal(self, stream_ptr, value):
        self.cudaSemaphore.signal(stream_ptr, value)
    
    def wait(self, stream_ptr, value):
        self.cudaSemaphore.wait(stream_ptr, value)
    
    def get_vk_semaphore(self) -> rhi.Semaphore:
        return self.vkSemaphore
      
    def get_next_stamp(self):
        self.stamp += 1
        return self.stamp
      
    def get_current_stamp(self):
        return self.stamp
    
    
def get_torch_cuda_stream():
    stream = torch.cuda.current_stream()
    return stream.cuda_stream


class EditorApplicationBase:
    def __init__(self, name:str="Hello, SIByL!", width:int=1280, height:int=720,
                 extensions:rhi.ContextExtensions = rhi.ContextExtensionEnum.NONE):
        self.window = pycore.Window(width, height, name)
        gfx.GFXContext.initialize(self.window, extensions)
        pycore.editor.EditorContext.initialize()
        self.device = gfx.GFXContext.device()
        rhi.CUDAContext.initialize(self.device)
        
    def run(self):
        self.on_initialize()
        while self.window.is_running():
            self.window.fetch_events()
            if self.window.is_resized() or pycore.editor.ImGuiContext.need_recreate():
                if self.window.get_width() == 0 or self.window.get_height() == 0: continue
                pycore.editor.ImGuiContext.recreate(self.window.get_width(), self.window.get_height())
                self.on_window_resized()
            if self.window.is_iconified(): continue
            
            pycore.gfx.GFXContext.get_flights().frame_start()
            pycore.editor.ImGuiContext.start_new_frame()
            
            # updating
            self.on_update()
            
            # create a command encoder
            encoder = self.device.create_command_encoder(
                pycore.gfx.GFXContext.get_flights().get_command_buffer())
            
            self.on_command_record(encoder)
            
            # start record the gui
            pycore.editor.EditorContext.begin_frame(encoder)    
            
            self.on_draw_gui()
            
            # submit the command
            self.device.get_graphics_queue().submit(
                [ encoder.finish() ],
                pycore.gfx.GFXContext.get_flights().get_image_available_semaphore(),
                pycore.gfx.GFXContext.get_flights().get_render_finished_semaphore(),
                pycore.gfx.GFXContext.get_flights().get_fence())

            pycore.editor.EditorContext.end_frame(pycore.gfx.GFXContext.get_flights().get_render_finished_semaphore())
            pycore.gfx.GFXContext.frame_end()
    
    def end(self):
        self.device.wait_idle()
        self.on_finalize()
        pycore.editor.EditorContext.finalize()
        gfx.GFXContext.finalize()
        self.window.destroy()
        
    def on_initialize(self):
        pass
    
    def on_update(self):
        pass
    
    def on_finalize(self):
        pass
    
    def on_command_record(self, encoder: rhi.CommandEncoder):
        pass
    
    def on_window_resized(self):
        pass
    
    def on_draw_gui(self):
        pass
    
    
def make_struct(name, fields: dict):
    class Struct(ctypes.Structure):
        _fields_ = [(key, type(val)) for key, val in fields.items()]
    Struct.__name__ = name
    return Struct(*[val.value for val in fields.values()])


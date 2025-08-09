import os
import sibylengine as se
se.Configuration.set_config_file(os.path.dirname(os.path.abspath(__file__)) + "/runtime.config")

class FooPass (se.rdg.RenderPass):
    def __init__(self):
        super().__init__()
        self.init("./shaders/editor/geometry-viewer.slang")
    
    def reflect(self, reflector: se.rdg.PassReflection) -> se.rdg.PassReflection:
        reflector.add_output("Color").is_texture()\
            .with_format(se.rhi.TextureFormat.RGBA32_FLOAT)\
            .consume_as_color_attachment_at(0)
        reflector.add_output("Depth").is_texture()\
            .with_format(se.rhi.TextureFormat.DEPTH32_FLOAT)\
            .consume_as_depth_stencil_attachment_at(0)
        return reflector
        
    def execute(self, rdrCtx: se.rdg.RenderContext, rdrDat: se.rdg.RenderData):        
        self.set_render_pass_descriptor(se.rhi.RenderPassDescriptor([
            se.rhi.RenderPassColorAttachment(rdrDat.get_texture("Color").get_rtv(0,0,1), 
            se.vec4(0,0,0,1), se.rhi.LoadOp.CLEAR, se.rhi.StoreOp.STORE),
        ], se.rhi.RenderPassDepthStencilAttachment(rdrDat.get_texture("Depth").get_dsv(0,0,1), 
            1, se.rhi.LoadOp.CLEAR, se.rhi.StoreOp.STORE, False, 
            0, se.rhi.LoadOp.DONT_CARE, se.rhi.StoreOp.DONT_CARE, False)))
        
        scene = rdrDat.get_scene()
        self.update_binding_scene(rdrCtx, scene)
        
        encoder = self.begin_pass(rdrCtx, rdrDat.get_texture("Color").get())
        scene.draw_meshes(encoder, 0)
        encoder.end()


class FooGraph(se.rdg.Graph):
    def __init__(self):
        super().__init__()
        self.fwd_pass = FooPass()
        self.add_pass(self.fwd_pass, "Render Pass")
        self.mark_output("Render Pass", "Color")


class EditorApplication(se.EditorApplicationBase):
    def __init__(self):
        super().__init__("SIByL Editor", 1280, 720,
            se.rhi.ContextExtensionEnum.DEBUG_UTILS |
            se.rhi.ContextExtensionEnum.USE_AFTERMATH |
            se.rhi.ContextExtensionEnum.FRAGMENT_BARYCENTRIC |
            se.rhi.ContextExtensionEnum.RAY_TRACING)

        self.foo_graph = FooGraph()
        self.foo_graph.build()
        
        self.scene = se.gfx.GFXContext.load_scene_pbrt("../scenes/ground_explosion/ground_explosion.pbrt")
        # self.scene = se.gfx.GFXContext.load_scene_xml("../scenes/veachmis/scene.xml")
        # self.scene = se.gfx.GFXContext.load_scene_gltf("../scenes/cbox/scene-test.gltf")
        se.editor.EditorContext.set_scene_display(self.scene)
        se.editor.EditorContext.set_graph_display(self.foo_graph)

    def on_update(self):
        self.scene.update_scripts()
        self.scene.update_gpu_scene()
        
    def on_command_record(self, encoder: se.rhi.CommandEncoder):
        self.foo_graph.get_render_data().set_scene(self.scene)
        self.foo_graph.execute(encoder)

        if self.foo_graph.get_output():
            se.editor.EditorContext.set_viewport_texture(self.foo_graph.get_output())
            
    def on_finalize(self):
        del self.foo_graph
        del self.scene
        

if __name__ == "__main__":
    app = EditorApplication()
    try: app.run()
    except KeyboardInterrupt: pass
    finally:
        app.end()
        print("Editor application exited gracefully.")
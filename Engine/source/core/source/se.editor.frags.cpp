#include "se.editor.hpp"
#include <imgui/includes/backends/imgui_impl_glfw.h>
#include <imgui/includes/backends/imgui_impl_vulkan.h>
#include <imnodes/imnodes.h>

namespace se {
namespace editor {
struct ImageInspectPass : public se::rdg::RenderPass {
  gfx::TextureHandle m_inputTexture;
  gfx::BufferHandle m_readbackBuffer;
  se::vec2 scales = se::vec2(1, 1);
  se::vec2 offsets = se::vec2(0, 0);
  int m_showChannel = 0;

  ImageInspectPass() { init("./shaders/editor/image-viewer.slang"); }

  virtual auto reflect(rdg::PassReflection& reflector) noexcept -> se::rdg::PassReflection {
    reflector.add_output("Color").is_texture()
      .with_format(se::rhi::TextureFormat::RGBA32_FLOAT)
      .consume_as_color_attachment_at(0);
    return reflector;
  }

  virtual auto execute(
    se::rdg::RenderContext* rdrCtx,
    se::rdg::RenderData const& rdrDat
  ) noexcept -> void {
    auto color = rdrDat.get_texture("Color");

    set_render_pass_descriptor(
      se::rhi::RenderPassDescriptor(
        std::vector<se::rhi::RenderPassColorAttachment>{
      se::rhi::RenderPassColorAttachment{ color->get_rtv(0, 0, 1), 
        {0, 0, 0, 1}, se::rhi::LoadOp::CLEAR, se::rhi::StoreOp::STORE }
    }));

    update_bindings(rdrCtx, {
      { "ro_texture", rhi::BindingResource(m_inputTexture->get_srv(0, 1, 0, 1)) },
      { "rw_buffer", m_readbackBuffer->get_binding_resource() },
    });

    struct PushConst {
      se::vec2 scales;
      se::vec2 offsets;
      int output_size;
      int show_channel;
    } pConst = {
      this->scales,
      this->offsets,
      int(color->m_texture->width()),
      m_showChannel
    };

    auto encoder = begin_pass(rdrCtx, color.get());
    encoder->push_constants(&pConst,
      se::rhi::ShaderStageEnum::VERTEX | se::rhi::ShaderStageEnum::FRAGMENT,
      0, sizeof(PushConst));
    encoder->draw(6, 1, 0, 0);
    encoder->end();
  }
};

struct ImageInspectGraph : public se::rdg::Graph {
  ImageInspectPass fwd_pass;
  ImageInspectGraph() {
    add_pass(&fwd_pass, "ImageInspect Pass");
    mark_output("ImageInspect Pass", "Color");
  }
};

ImageInspectorFragment::ImageInspectorFragment(gfx::TextureHandle texture) {
  m_texture = texture;
  m_graph = std::make_unique<ImageInspectGraph>();

  rhi::BufferDescriptor desc;
  desc.size = 32;
  desc.usage = rhi::BufferUsageEnum::STORAGE;
  desc.memoryProperties = rhi::MemoryPropertyEnum::HOST_VISIBLE_BIT | rhi::MemoryPropertyEnum::HOST_COHERENT_BIT;
  m_readbackBuffer = gfx::GFXContext::create_buffer_desc(desc);
  m_readbackInfo = static_cast<InteractInfo*>(m_readbackBuffer->memory_mapping());
}

float floor_signed(float f) {
  return (float)((f >= 0 || (int)f == f) ? (int)f : (int)f - 1);
}

auto ImageInspectorFragment::execute() noexcept -> gfx::Texture* {
  beat(); // tell the manager it is alive
  int max_width = std::min(int(ImGui::GetContentRegionAvail().x), int(ImGui::GetContentRegionAvail().y));
  ImVec2 contentStart = ImGui::GetCursorScreenPos(); // top-left of content region

  if (m_graph->m_standardSize.x != max_width) {
    m_graph->m_standardSize.x = max_width;
    m_graph->m_standardSize.y = max_width;
    gfx::GFXContext::device()->wait_idle();
    m_graph->build();
    m_imguiTex = nullptr;
  }

  bool hovered = ImGui::IsWindowHovered();
  const ImGuiIO& IO = ImGui::GetIO();

  {  //DRAGGING
    auto panButton = ImGuiMouseButton_Left;
      // start drag
    if (!m_isDragging && hovered && IO.MouseClicked[panButton]) {
      m_isDragging = true;
    }
    // carry on dragging
    else if (m_isDragging) {
      se::vec2 uvDelta = se::vec2(IO.MouseDelta.x, IO.MouseDelta.y) * 2 / max_width;
      m_panPos += uvDelta;
    }

    // end drag
    if (m_isDragging && (IO.MouseReleased[panButton] || !IO.MouseDown[panButton])) {
      m_isDragging = false;
    }
  }

  ImVec2 mousePos = ImGui::GetMousePos();
  mousePos = ImVec2(mousePos.x - contentStart.x, mousePos.y - contentStart.y);
  vec2 mousePosTexel = vec2(mousePos.x, mousePos.y);
  m_readbackInfo->pixel = ivec2(mousePosTexel);
  vec2 mouseUV = mousePosTexel / max_width; // normalized to [0,1] screen space
  // Get the position under mouse in shader space BEFORE zoom
  vec2 shaderPosUnderMouse = ((vec2{ 1 - mouseUV.x, 1-mouseUV.y } *2 - vec2(1)) + m_panPos) / m_scale;

  // ZOOM
  if (hovered && IO.MouseWheel != 0) {
    float zoomRate = m_zoomRate;
    float scale = m_scale.y;
    float prevScale = scale;


    bool keepTexelSizeRegular = scale > m_minimumGridSize;
    if (IO.MouseWheel > 0) {
      scale *= zoomRate;
      if (keepTexelSizeRegular) {
        // It looks nicer when all the grid cells are the same size
        // so keep scale integer when zoomed in
        scale = ImCeil(scale);
      }
    }
    else {
      scale /= zoomRate;
      if (keepTexelSizeRegular)
      {
        // See comment above. We're doing a floor this time to make
        // sure the scale always changes when scrolling
        scale = floor_signed(scale);
      }
    }
    /* To make it easy to get back to 1:1 size we ensure that we stop
     * here without going straight past it*/
    if ((prevScale < 1 && scale > 1) || (prevScale > 1 && scale < 1))
    {
      scale = 1;
    }
    
    m_scale = { scale, scale };

    // Adjust pan so the same point stays under the mouse
    m_panPos = shaderPosUnderMouse * m_scale - (vec2{ 1 - mouseUV.x, 1 - mouseUV.y } *2 - vec2(1));
  }

  std::vector<rhi::BarrierDescriptor> barriers = m_texture->consume(gfx::Texture::ConsumeEntry()
    .add_stage(rhi::PipelineStageEnum::FRAGMENT_SHADER_BIT)
    .set_layout(rhi::TextureLayoutEnum::GENERAL)
    .set_access(rhi::AccessFlagEnum::SHADER_READ_BIT));
  for (int i = 0; i < barriers.size(); ++i)
    editor::ImGuiContext::m_encoder->pipeline_barrier(barriers[i]);

  auto& pass = static_cast<ImageInspectGraph*>(m_graph.get())->fwd_pass;
  pass.scales = m_scale;
  pass.offsets = m_panPos;
  pass.m_inputTexture = m_texture;
  pass.m_readbackBuffer = m_readbackBuffer;
  pass.m_showChannel = m_showChannel;
  m_graph->execute(ImGuiContext::m_encoder);

  return m_graph->get_output().value().get();
}
}
}
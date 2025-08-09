#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <functional>
#include <se.rhi.hpp>
#include <se.gfx.hpp>
#include <se.rdg.hpp>

namespace se {
namespace editor {
  struct ImguiTexture {
    VkDescriptorSet m_descriptorSet = {};

    ImguiTexture(rhi::Sampler* sampler, 
      rhi::TextureView* view, 
      rhi::TextureLayoutEnum layout);
    auto get_texture_id() noexcept -> ImTextureID;
  };

  struct ImguiBackend {
    ImGui_ImplVulkanH_Window m_mainWindowData;
    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool;
    se::Window* m_bindedWindow = nullptr;
    rhi::Device* m_device;
    bool m_swapChainRebuild = false;

    ImguiBackend(rhi::Device*);
    ~ImguiBackend();
    /** setup the backend for the platform */
    auto setup_platform_backend() noexcept -> void;
    /** upload the fonts for the imgui */
    auto upload_fonts() noexcept -> void;
    /** get the window DPI */
    auto get_window_dpi() noexcept -> float;
    /** response to the window resize */
    auto on_window_resize(size_t, size_t) -> void;
    /** start a new frame */
    auto start_new_frame() -> void;
    /** render the editor frame */
    auto render(ImDrawData* draw_data,
      rhi::Semaphore* waitSemaphore = nullptr) -> void;
    /** present the current frame */
    auto present() -> void;
    /** create the ImGui Texture */
    auto create_imgui_texture(rhi::Sampler* sampler, rhi::TextureView* view,
      rhi::TextureLayoutEnum layout) noexcept -> std::unique_ptr<ImguiTexture>;
  };


  using RawImGuiCtx = ::ImGuiContext;

  struct ImGuiContext {
    /** imgui backend */
    static std::unique_ptr<ImguiBackend> m_imguiBackend;
    static float m_dpi;
    static ::ImGuiContext* m_imContext;
    static rhi::CommandEncoder* m_encoder;

    /** initialzier */
    static auto initialize(rhi::Device* device) -> void;
    static auto finalize() -> void;
    static auto get_raw_ctx() noexcept -> RawImGuiCtx*;
    static auto need_recreate() noexcept -> bool;
    static auto recreate(size_t width, size_t height) noexcept -> void;
    static auto start_new_frame() -> void;
    static auto start_gui_recording() -> void;
    static auto render(rhi::Semaphore* waitSemaphore = nullptr) -> void;
    static auto set_encoder(rhi::CommandEncoder* encoder) -> void;
    static auto get_dpi() noexcept -> float { return m_dpi; }
    static auto create_imgui_texture(rhi::Sampler* sampler, rhi::TextureView* view,
      rhi::TextureLayoutEnum layout) noexcept -> std::unique_ptr<ImguiTexture>;

    //static auto getImGuiTexture(gfx::TextureHandle texture) noexcept -> ImGuiTexture*;
  };

  struct Widget {
    /** widget info */
    struct WidgetInfo {
      ImVec2 windowPos;
      ImVec2 mousePos;
      bool isHovered;
      bool isFocused;
    } m_info;

    /** virtual destructor */
    virtual ~Widget() = default;
    /** virtual draw gui*/
    virtual auto on_draw_gui() noexcept -> void = 0;
    /** fetch common infomation */
    auto common_on_draw_gui() noexcept -> void;
  };

  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ Fragment                                                                  ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // ┃ A concept help to manage the resources in editor. Some part of the editor ┃
  // ┃ implies custom drawcall() and resource allocation. For example, the image ┃
  // ┃ viewer would do a specific draw to zoom and pan, it thus also includes a  ┃
  // ┃ rendergraph to describe such behavior.                                    ┃
  // ┃                                                                           ┃
  // ┃ When needed, fragment is registered to the global pool, however, it is    ┃
  // ┃ generally hard to tell when a fragement is no longer needed. Thus, we     ┃
  // ┃ test the heart beat across the frames and release those no longer beats.  ┃
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  struct IFragment {
    int32_t m_heartBeating = 0;
    virtual ~IFragment() = default;
    auto reset() noexcept -> void { m_heartBeating -= 1; }
    auto beat() noexcept -> void { m_heartBeating++; }
    auto alive() noexcept -> bool { return m_heartBeating >= 0; }
  };

  struct FragmentPool {
    std::list<std::unique_ptr<IFragment>> m_fragments;

    FragmentPool() = default;
    ~FragmentPool() = default;
    FragmentPool(const FragmentPool&) = delete;
    FragmentPool(FragmentPool&&) = default;
    FragmentPool& operator=(const FragmentPool&) = delete;
    FragmentPool& operator=(FragmentPool&&) = default;
    auto reset() noexcept -> void;
    auto clean() noexcept -> void;

    template <class T, typename... Args>
    auto register_fragment(Args&&... args) noexcept -> IFragment* {
      std::unique_ptr<T> ptr = std::make_unique<T>(std::forward<Args>(args)...);
      m_fragments.emplace_back(std::move(ptr));
      return m_fragments.back().get();
    }
  };

  struct EditorContext {
    using function = std::function<void()>;
    function m_inspectorDraw;
    FragmentPool m_fragmentPool;
    std::optional<gfx::SceneHandle> m_sceneDisplayed;
    std::optional<gfx::TextureHandle> m_viewportTexture;
    std::optional<rdg::Graph*> m_graph;
    bool m_viewportHovered;
    bool m_viewportFocused;
    /** all the widgets registered */
    // std::unordered_map<std::string, std::unique_ptr<Widget>> m_widgets;

    struct InspectorData {
      ivec2 m_mouseOffset = {};
      bool m_hovered = false;
      bool m_focused = false;
    } m_inspector;

    SINGLETON(EditorContext, {});
    /** initialize editor */
    static auto initialize() noexcept -> void;
    /** finalize editor */
    static auto finalize() noexcept -> void;
    /** get all widgets */
    // static auto widgets() noexcept -> std::unordered_map<std::string, std::unique_ptr<Widget>>&;
    /** draw gui*/
    static auto on_draw_gui() noexcept -> void;

    // inspector window
    static auto set_inspector_callback(function const& fn) noexcept -> void;
    static auto clear_inspector_callback() noexcept -> void;
    // viewport window
    static auto set_viewport_texture(gfx::TextureHandle texture) noexcept -> void;
    static auto clear_viewport_texture() noexcept -> void;

    static auto set_scene_display(gfx::SceneHandle scene) noexcept -> void;
    static auto set_graph_display(rdg::Graph* graph) noexcept -> void;

    static auto begin_frame(rhi::CommandEncoder* encoder) noexcept -> void;
    static auto end_frame(rhi::Semaphore* waitSemaphore) noexcept -> void;
    /** register a widget to editor */ 
    // template <class T>
    // static auto registerWidget() noexcept -> void { widgets()[typeid(T).name()] = std::make_unique<T>(); }
    // /** add a widget to editor */ 
    // template <class T>
    // static auto addWidget(std::string const& name) noexcept -> void { widgets()[name] = std::make_unique<T>(); }
    // /** remove a widget from editor */ 
    // template <class T>
    // static auto removeWidget(std::string const& name) noexcept -> void { widgets().erase(name); }
    // /** get a widget registered in editor */ 
    // template <class T>
    // static auto getWidget() noexcept -> T* {
    //   auto iter = widgets().find(std::string(typeid(T).name()));
    //   if (iter == widgets().end()) return nullptr;
    //   else return static_cast<T*>(iter->second.get());
    // }
  };

  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ Predefined fragments                                                      ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // 
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  struct ImageInspectorFragment : public IFragment {
    // the texture to inspect
    gfx::TextureHandle m_texture;
    gfx::BufferHandle m_readbackBuffer;
    gfx::SamplerHandle m_sampler;
    std::unique_ptr< editor::ImguiTexture> m_imguiTex;
    int m_showChannel = 0;

    struct InteractInfo {
      vec4 color;
      ivec2 pixel;
      ivec2 outPixel;
    };
    InteractInfo* m_readbackInfo;
    std::unique_ptr<se::rdg::Graph> m_graph = nullptr;
    se::vec2 m_scales = se::vec2(1, 1);
    se::vec2 m_offsets = se::vec2(0, 0);
    bool m_isDragging = false;
    se::vec2 m_panPos = se::vec2(0, 0);
    float m_zoomRate = 1.1f;
    float m_minimumGridSize = 4;
    se::vec2 m_scale = se::vec2(1, 1);

    ImageInspectorFragment(gfx::TextureHandle texture);
    auto round_pan_pos() noexcept -> void;
    auto execute() noexcept -> gfx::Texture*;
  };

  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ Predefined scripts                                                        ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // 
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

  struct EditorCameraControllerScript : public gfx::IScript {
    struct CameraState {
      float yaw = 0; float pitch = 0; float roll = 0;
      float x = 0; float y = 0; float z = 0;
      auto set_from_transform(gfx::Transform const& transform) noexcept -> void;
      auto lerp_towards(CameraState const& target, float positionLerpPct,
        float rotationLerpPct) noexcept -> void;
      auto update_transform(gfx::Transform& transform) noexcept -> void;
    };

    CameraState m_targetCameraState;
    CameraState m_interpolatingCameraState;
    float m_mouseSensitivityMultiplier = 0.01f;
    float m_boost = 3.5f;
    float m_positionLerpTime = 0.2f;
    float m_rotationLerpTime = 0.01f;
    float m_mouseSensitivity = 60.0f;
    float m_lastX = 0;
    float m_lastY = 0;
    bool m_invertY = true;
    bool m_justPressedMouse = true;
    bool m_inRotationMode = false;
    AnimationCurve m_mouseSensitivityCurve = { {0, 0.5, 0, 5}, {1, 2.5, 0, 0} };

    virtual auto on_init(gfx::Node& node) noexcept -> void;
    virtual auto on_update(gfx::Node& node, double delta) noexcept -> void;
    virtual auto on_end(gfx::Node& node) noexcept -> void;
  };
}
}
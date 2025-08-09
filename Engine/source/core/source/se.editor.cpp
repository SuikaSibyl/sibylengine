#include "se.editor.hpp"
#include <imgui/includes/backends/imgui_impl_glfw.h>
#include <imgui/includes/backends/imgui_impl_vulkan.h>
#include "se.gfx.hpp"
#include <imnodes/imnodes.h>
#include <cmath>

namespace se {
namespace editor {
  ImguiTexture::ImguiTexture(
    rhi::Sampler* sampler,
    rhi::TextureView* view,
    rhi::TextureLayoutEnum layout) {
    m_descriptorSet = ImGui_ImplVulkan_AddTexture(
      static_cast<rhi::Sampler*>(sampler)->m_textureSampler,
      static_cast<rhi::TextureView*>(view)->m_imageView,
      rhi::get_vk_image_layout(layout));
  }

  auto ImguiTexture::get_texture_id() noexcept -> ImTextureID {
    return (ImTextureID)m_descriptorSet;
  }

  inline int g_MinImageCount = 2;

  ImguiBackend::ImguiBackend(rhi::Device* device)
    : m_device(device)
    , m_bindedWindow(device->from_which_adapter()->from_which_context()->get_binded_window()) {
    rhi::Adapter* adapter = this->m_device->from_which_adapter();
    rhi::Context* context = this->m_device->from_which_adapter()->from_which_context();
    //fill main window data
    m_mainWindowData.Surface = context->get_vk_surface_khr();
    // select Surface Format
    VkFormat const requestSurfaceImageFormat[] = {
        VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    VkColorSpaceKHR const requestSurfaceColorSpace =
      VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    m_mainWindowData.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
      this->m_device->from_which_adapter()
      ->get_vk_physical_device(), m_mainWindowData.Surface, requestSurfaceImageFormat,
      (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);
    // select Surface present mode
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_IMMEDIATE_KHR };
    m_mainWindowData.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
      static_cast<rhi::Adapter*>(this->m_device->from_which_adapter())
      ->get_vk_physical_device(), m_mainWindowData.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    // Create SwapChain, RenderPass, Framebuffer, etc.
    rhi::Adapter::QueueFamilyIndices indices =
      static_cast<rhi::Adapter*>(this->m_device->from_which_adapter())
      ->get_queue_family_indices();
    IM_ASSERT(g_MinImageCount >= 2);
    int width, height;
    context->get_binded_window()->get_framebuffer_size(&width, &height);
    ImGui_ImplVulkanH_CreateOrResizeWindow(
      context->get_vk_instance(),
      adapter->get_vk_physical_device(),
      this->m_device->get_vk_device(),
      &m_mainWindowData, indices.m_graphicsFamily.value(), nullptr, width, height,
      g_MinImageCount);
    // Create Descriptor Pool
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000} };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(
      this->m_device->get_vk_device(),
      &pool_info, nullptr, &m_descriptorPool);
  }

  ImguiBackend::~ImguiBackend() {
    vkDeviceWaitIdle(m_device->get_vk_device());
    ImGui_ImplVulkanH_DestroyWindow(
      m_device->from_which_adapter()->from_which_context()->get_vk_instance(),
      m_device->get_vk_device(),
      &m_mainWindowData, nullptr);
    vkDestroyDescriptorPool(
      m_device->get_vk_device(),
      m_descriptorPool, nullptr);

    rhi::Context* m_context = m_device->from_which_adapter()->from_which_context();
    m_context->get_vk_surface_khr() = {};
  }

  auto ImguiBackend::get_window_dpi() noexcept -> float {
    return m_bindedWindow->get_high_dpi();
  }

  auto ImguiBackend::setup_platform_backend() noexcept -> void {
    auto adapter = m_device->from_which_adapter();
    auto context = adapter->from_which_context();
    rhi::Adapter::QueueFamilyIndices indices = adapter->get_queue_family_indices();
    ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)m_bindedWindow->get_handle(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = adapter->from_which_context()->get_vk_instance();
    init_info.PhysicalDevice = adapter->get_vk_physical_device();
    init_info.Device = m_device->get_vk_device();
    init_info.QueueFamily = indices.m_graphicsFamily.value();
    init_info.Queue = m_device->get_graphics_queue().m_queue;
    init_info.PipelineCache = m_pipelineCache;
    init_info.DescriptorPool = m_descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = m_mainWindowData.ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;
    init_info.RenderPass = m_mainWindowData.RenderPass;
    ImGui_ImplVulkan_Init(&init_info);
  }

  VkResult err;
  void check_vk_result(VkResult err) {
    if (err == 0) return;
    se::error("ImGui Vulkan Error: VkResult = {0}", (unsigned int)err);
    if (err < 0) return;
  }

  // Upload Fonts
  auto ImguiBackend::upload_fonts() noexcept -> void {
    ImGui_ImplVulkan_CreateFontsTexture();
  }

  auto ImguiBackend::on_window_resize(size_t x, size_t y) -> void {
    auto adapter = m_device->from_which_adapter();
    auto context = adapter->from_which_context();
    rhi::Adapter::QueueFamilyIndices indices = adapter->get_queue_family_indices();
    ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
    ImGui_ImplVulkanH_CreateOrResizeWindow(
      context->get_vk_instance(),
      adapter->get_vk_physical_device(),
      m_device->get_vk_device(),
      &m_mainWindowData, indices.m_graphicsFamily.value(), nullptr, x, y,
      g_MinImageCount);
    m_mainWindowData.FrameIndex = 0;
  }

  auto ImguiBackend::start_new_frame() -> void {
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
  }

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  auto ImguiBackend::render(ImDrawData* draw_data,
    rhi::Semaphore* waitSemaphore) -> void {
    m_mainWindowData.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
    m_mainWindowData.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
    m_mainWindowData.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
    m_mainWindowData.ClearValue.color.float32[3] = clear_color.w;

    VkResult err;
    ImGui_ImplVulkanH_Window* wd = &m_mainWindowData;

    // STEP 1: Wait for the frame to be available BEFORE acquiring image
    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
      err = vkWaitForFences(m_device->get_vk_device(), 1,
        &fd->Fence, VK_TRUE,
        UINT64_MAX);  // wait indefinitely instead of periodically checking      
      check_vk_result(err);

      err = vkResetFences(m_device->get_vk_device(), 1,
        &fd->Fence);
      check_vk_result(err);
    }

    // STEP 2: Acquire next image
    VkSemaphore image_acquired_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(m_device->get_vk_device(),
      wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE,
      &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
      // Recreate swapchain if it is out of date
      m_swapChainRebuild = true;
      // wait for the seamaphore to be signaled
      if (waitSemaphore) {
          // Recreate the semaphore
          m_device->wait_idle();
          vkDestroySemaphore(m_device->get_vk_device(), waitSemaphore->m_semaphore, nullptr);
          VkSemaphoreCreateInfo createInfo = {};
          createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
          vkCreateSemaphore(m_device->get_vk_device(), &createInfo, nullptr, &(waitSemaphore->m_semaphore));
      }
    }
    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
      return;
    }
    if (err != VK_SUBOPTIMAL_KHR) {
      check_vk_result(err);
    }
    
    {
      err = vkResetCommandPool(m_device->get_vk_device(),
        fd->CommandPool, 0);
      check_vk_result(err);
      VkCommandBufferBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
      check_vk_result(err);
    }
    {
      VkRenderPassBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      info.renderPass = wd->RenderPass;
      info.framebuffer = fd->Framebuffer;
      info.renderArea.extent.width = wd->Width;
      info.renderArea.extent.height = wd->Height;
      info.clearValueCount = 1;
      info.pClearValues = &wd->ClearValue;
      vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }
    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);
    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
      std::vector<VkSemaphore> waitSeams = { image_acquired_semaphore };
      std::vector<VkPipelineStageFlags> wait_stages = {
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
      if (waitSemaphore) {
        waitSeams.push_back(
          static_cast<rhi::Semaphore*>(waitSemaphore)->m_semaphore);
        wait_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
      }

      VkSubmitInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      info.waitSemaphoreCount = waitSeams.size();
      info.pWaitSemaphores = waitSeams.data();
      info.pWaitDstStageMask = wait_stages.data();
      info.commandBufferCount = 1;
      info.pCommandBuffers = &fd->CommandBuffer;
      info.signalSemaphoreCount = 1;
      info.pSignalSemaphores = &render_complete_semaphore;

      err = vkEndCommandBuffer(fd->CommandBuffer);
      check_vk_result(err);
      err = vkQueueSubmit(
        m_device->get_graphics_queue().m_queue,
        1, &info, fd->Fence);
      check_vk_result(err);
    }
  }

  auto ImguiBackend::present() -> void {
    VkSemaphore render_complete_semaphore =
      m_mainWindowData.FrameSemaphores[m_mainWindowData.SemaphoreIndex]
      .RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &m_mainWindowData.Swapchain;
    info.pImageIndices = &m_mainWindowData.FrameIndex;
    VkResult err = vkQueuePresentKHR(
      m_device->get_graphics_queue().m_queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
      // Recreate swapchain if it is out of date
      m_swapChainRebuild = true;
    }
    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
      return;
    }
    if (err != VK_SUBOPTIMAL_KHR)
      check_vk_result(err);

    m_mainWindowData.SemaphoreIndex =
      (m_mainWindowData.SemaphoreIndex + 1) %
      m_mainWindowData.ImageCount;  // Now we can use the next set of semaphores
  }

  auto ImguiBackend::create_imgui_texture(
    rhi::Sampler* sampler,
    rhi::TextureView* view,
    rhi::TextureLayoutEnum layout) noexcept
    -> std::unique_ptr<ImguiTexture> {
    return std::make_unique<ImguiTexture>(sampler, view, layout);
  }

  //auto ImGuiContext::get_imgui_texture(gfx::TextureHandle texture) noexcept -> ImGuiTexture* {
  //  auto& pool = ImGuiTexturePool;
  //  auto iter = pool.find(texture->texture.get());
  //  if (iter == pool.end()) {
  //    gfx::SamplerHandle sampler = gfx::GFXContext::create_sampler_desc(
  //      rhi::AddressMode::CLAMP_TO_EDGE, rhi::FilterMode::NEAREST,
  //      rhi::MipmapFilterMode::NEAREST);
  //    pool.insert({ texture->texture.get(),
  //      imguiBackend->createImGuiTexture(
  //        sampler.get(), texture->getSRV(0, 1, 0, 1),
  //        rhi::TextureLayout::SHADER_READ_ONLY_OPTIMAL) });
  //    return pool[texture->texture.get()].get();
  //  }
  //  else {
  //    return iter->second.get();
  //  }
  //}

  auto ImGuiContext::initialize(rhi::Device* device) -> void {
    m_imguiBackend = std::make_unique<ImguiBackend>(device);
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    m_imContext = ImGui::CreateContext();
    ImNodes::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    m_dpi = m_imguiBackend->get_window_dpi();
    
    std::string engine_path = Configuration::string_property("engine_path");
    std::string font_path = engine_path + "assets/fonts/opensans/OpenSans-Bold.ttf";
    se::info("{}", font_path);
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), m_dpi * 15.0f);
    io.FontDefault = io.Fonts->AddFontFromFileTTF(font_path.c_str(), m_dpi * 15.0f);


    // set dark theme
    {
      auto& colors = ImGui::GetStyle().Colors;
      // Back Grounds
      colors[ImGuiCol_WindowBg] = ImVec4{ 0.121568f, 0.121568f, 0.121568f, 1.0f };
      colors[ImGuiCol_DockingEmptyBg] = ImVec4{ 0.117647f, 0.117647f, 0.117647f, 1.0f };
      // Headers
      colors[ImGuiCol_Header] = ImVec4{ 0.121568f, 0.121568f, 0.121568f, 1.0f };
      colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.2392f, 0.2392f, 0.2392f, 1.0f };
      colors[ImGuiCol_HeaderActive] = ImVec4{ 0.2392f, 0.2392f, 0.2392f, 1.0f };
      // Buttons
      colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
      colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
      colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
      // Frame BG
      colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
      colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
      colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
      // Tabs
      colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
      colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
      colors[ImGuiCol_TabActive] = ImVec4{ 0.23922f, 0.23922f, 0.23922f, 1.0f };
      colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
      colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
      // Title
      colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
      colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.121568f, 0.121568f, 0.121568f, 1.0f };
      colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    }
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    m_imguiBackend->setup_platform_backend();
    m_imguiBackend->upload_fonts();
    
    std::string layout_path = Configuration::string_property("engine_path") + "/layouts/default.ini";
    ImGui::LoadIniSettingsFromDisk(layout_path.c_str());
  }

  auto ImGuiContext::finalize() -> void {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();
    m_imguiBackend = nullptr;
  }

  auto ImGuiContext::get_raw_ctx() noexcept -> RawImGuiCtx* {
    return m_imContext;
  }

  auto ImGuiContext::start_new_frame() -> void {
    m_imguiBackend->start_new_frame();
  }
    
  auto ImGuiContext::need_recreate() noexcept -> bool {
    return m_imguiBackend->m_swapChainRebuild;
  }
    
  auto ImGuiContext::recreate(size_t width, size_t height) noexcept -> void {
    m_imguiBackend->on_window_resize(width, height);
    m_imguiBackend->m_swapChainRebuild = false;
  }
  
  auto ImGuiContext::set_encoder(rhi::CommandEncoder* encoder) -> void {
    m_encoder = encoder;
  }

  auto ImGuiContext::start_gui_recording() -> void {
    ImGui::NewFrame();
    // Using Docking space
    {
      static bool dockspaceOpen = true;
      static bool opt_fullscreen = true;
      static bool opt_padding = false;
      static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

      // We are using the ImGuiWindowFlags_NoDocking flag to make the parent
      // window not dockable into, because it would be confusing to have two
      // docking targets within each others.
      ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        //ImGuiWindowFlags_MenuBar;// | ImGuiWindowFlags_NoDocking;
      if (opt_fullscreen) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        //ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar |
          ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_NoMove;
        window_flags |=
          ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
      }
      else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
      }
      // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will
      // render our background and handle the pass-thru hole, so we ask Begin() to
      // not render a background.
      //if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
      //  window_flags |= ImGuiWindowFlags_NoBackground;
      // Important: note that we proceed even if Begin() returns false (aka window
      // is collapsed). This is because we want to keep our DockSpace() active. If
      // a DockSpace() is inactive, all active windows docked into it will lose
      // their parent and become undocked. We cannot preserve the docking
      // relationship between an active window and an inactive docking, otherwise
      // any change of dockspace/settings would lead to windows being stuck in
      // limbo and never being visible.
      if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
      if (!opt_padding) ImGui::PopStyleVar();
      if (opt_fullscreen) ImGui::PopStyleVar(2);
      // Submit the DockSpace
      ImGuiIO& io = ImGui::GetIO();
      ImGuiStyle& style = ImGui::GetStyle();
       style.WindowMinSize.x = 350.0f;
      if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
      }
    }
  }

  auto ImGuiContext::render(rhi::Semaphore* waitSemaphore) -> void {
    // End docking space
    ImGui::End();
    // Do render ImGui stuffs
    ImGui::Render();
    ImDrawData* main_draw_data = ImGui::GetDrawData();
    const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f ||
      main_draw_data->DisplaySize.y <= 0.0f);

    if (!main_is_minimized) 
      m_imguiBackend->render(main_draw_data, waitSemaphore);
  
    // Recreate the semaphore when the main window is minimized
    if (main_is_minimized) {
      m_imguiBackend->m_device->wait_idle();
      vkDestroySemaphore(m_imguiBackend->m_device->get_vk_device(), waitSemaphore->m_semaphore, nullptr);
      VkSemaphoreCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      vkCreateSemaphore(m_imguiBackend->m_device->get_vk_device(), 
        &createInfo, nullptr, &(waitSemaphore->m_semaphore));
    }

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
    }
    
    // Present Main Platform Window
    if ((!main_is_minimized) && (!m_imguiBackend->m_swapChainRebuild)) 
      m_imguiBackend->present();
  }
  
  auto ImGuiContext::create_imgui_texture(rhi::Sampler* sampler, rhi::TextureView* view,
    rhi::TextureLayoutEnum layout) noexcept -> std::unique_ptr<ImguiTexture> {
    return std::make_unique<ImguiTexture>(sampler, view, layout);
  }

  float ImGuiContext::m_dpi;
  ::ImGuiContext* ImGuiContext::m_imContext = nullptr;
  rhi::CommandEncoder* ImGuiContext::m_encoder = nullptr;
  std::unique_ptr<ImguiBackend> ImGuiContext::m_imguiBackend = nullptr;

  //auto Widget::commonOnDrawGui() noexcept -> void {
  //  // get the screen pos
  //  info.windowPos = ImGui::GetWindowPos();
  //  // see whether it is hovered
  //  if (ImGui::IsWindowHovered()) info.isHovered = true;
  //  else info.isHovered = false;
  //  if (ImGui::IsWindowFocused()) info.isFocused = true;
  //  else info.isFocused = false;
  //}

  auto EditorContext::initialize() noexcept -> void {
    se::gfx::GFXContext::create_flights(2, nullptr);
    rhi::Device* device = se::gfx::GFXContext::device();
    se::editor::ImGuiContext::initialize(device);
    ImGui::SetCurrentContext(se::editor::ImGuiContext::m_imContext);
  }

  auto EditorContext::finalize() noexcept -> void {
    Singleton<EditorContext>::instance()->m_fragmentPool.m_fragments.clear();
    Singleton<EditorContext>::instance()->m_inspectorDraw = {};
    Singleton<EditorContext>::instance()->m_sceneDisplayed = {};
    clear_viewport_texture();
	  se::editor::ImGuiContext::finalize();
  }

  // auto EditorContext::widgets() noexcept -> std::unordered_map<std::string, std::unique_ptr<Widget>>& {
  //   return Singleton<EditorContext>::instance()->m_widgets;
  // }
  
  auto EditorContext::begin_frame(rhi::CommandEncoder* encoder) noexcept -> void {
    se::editor::ImGuiContext::start_gui_recording();
    se::editor::ImGuiContext::set_encoder(encoder);
    se::editor::EditorContext::on_draw_gui();
    Singleton<EditorContext>::instance()->m_fragmentPool.reset();
  }
  
  auto EditorContext::end_frame(rhi::Semaphore* waitSemaphore) noexcept -> void {
    se::editor::ImGuiContext::render(waitSemaphore);
    Singleton<EditorContext>::instance()->m_fragmentPool.clean();
  }

  auto EditorContext::set_inspector_callback(function const& fn) noexcept -> void {
    Singleton<EditorContext>::instance()->m_inspectorDraw = fn;
  }
  
  auto EditorContext::clear_inspector_callback() noexcept -> void {
    Singleton<EditorContext>::instance()->m_inspectorDraw = {};
  }
  
  auto EditorContext::set_viewport_texture(gfx::TextureHandle texture) noexcept -> void {
    Singleton<EditorContext>::instance()->m_viewportTexture = texture;
  }
  
  auto EditorContext::clear_viewport_texture() noexcept -> void {
    Singleton<EditorContext>::instance()->m_viewportTexture = std::nullopt;
  }

  auto EditorContext::set_scene_display(gfx::SceneHandle scene) noexcept -> void {
    Singleton<EditorContext>::instance()->m_sceneDisplayed = scene;
  }
  
  auto EditorContext::set_graph_display(rdg::Graph* graph) noexcept -> void {
    Singleton<EditorContext>::instance()->m_graph = graph;
  }

  auto FragmentPool::reset() noexcept -> void {
    for (auto& iter : m_fragments) {
      iter->reset();
    }
  }

  auto FragmentPool::clean() noexcept -> void {
    for (auto it = m_fragments.begin(); it != m_fragments.end(); ) {
      if ((*it)->m_heartBeating < -5) {
        it = m_fragments.erase(it); // erase returns the next iterator
      }
      else {
        ++it;
      }
    }
  }

  auto draw_texture_inspector(gfx::TextureHandle handle, editor::IFragment* fragment) { handle.draw_gui(fragment); }

  auto EditorContext::on_draw_gui() noexcept -> void {
    static bool show_configure_window = false;
    static bool show_resources_window = true;
    static bool show_scene_window = true;

    // Main menu
    // -------------------------------------------------
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open Floating Window")) {

        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Window")) {
        if (ImGui::MenuItem("Scene hierarchy")) {
          show_scene_window = true;  // Trigger window to open
        }
        if (ImGui::MenuItem("Resources")) {
          show_resources_window = true;  // Trigger window to open
        }
        if (ImGui::MenuItem("Configuration")) {
          show_configure_window = true;  // Trigger window to open
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Editor")) {
        if (ImGui::MenuItem("Load layout")) {
          std::string init_path = Configuration::string_property("engine_path") + "layouts";
          std::string path = Platform::open_file(".ini", init_path);
          ImGui::LoadIniSettingsFromDisk(path.c_str());
        }
        if (ImGui::MenuItem("Save layout")) {
          std::string init_path = Configuration::string_property("engine_path") + "layouts/new-layout.ini";
          std::string path = Platform::save_file(".ini", init_path);
          ImGui::SaveIniSettingsToDisk(path.c_str());
        }

        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    // windows
    // -------------------------------------------------
    ImGui::Begin("Inspector");
    if (Singleton<EditorContext>::instance()->m_inspectorDraw)
      Singleton<EditorContext>::instance()->m_inspectorDraw();
    ImGui::End();

    ImGui::Begin("Pipeline Viewer");
    if (Singleton<EditorContext>::instance()->m_graph)
      Singleton<EditorContext>::instance()->m_graph.value()->on_draw_inspector();
    ImGui::End();

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
    if (Singleton<EditorContext>::instance()->m_viewportTexture.has_value()) {
      gfx::TextureHandle texture = Singleton<EditorContext>::instance()->m_viewportTexture.value();

      if (ImGui::BeginMenuBar()) {
        if (ImGui::Button("Save image")) {
          std::string filepath = se::Platform::save_file(
            "", Worldtime::get().to_string() + ".exr");
          Singleton<gfx::GFXContext>::instance()->m_jobsFrameEnd.push_back(
            [tex = texture.m_handle.handle(),
            filepath = filepath]() {tex->save_image(filepath); });
        }
        if (ImGui::Button("Open inspector")) {
          editor::IFragment* frag =
            Singleton<editor::EditorContext>::instance()->m_fragmentPool.
            register_fragment<editor::ImageInspectorFragment>(texture);
          std::function<void()> fn = std::bind(&draw_texture_inspector, texture, frag);
          se::editor::EditorContext::set_inspector_callback(fn);
        }
        ImGui::EndMenuBar();
      }

      Singleton<EditorContext>::instance()->m_inspector.m_hovered = ImGui::IsWindowHovered();
      Singleton<EditorContext>::instance()->m_inspector.m_focused = ImGui::IsWindowFocused();

      std::vector<rhi::BarrierDescriptor> barriers = texture->consume(gfx::Texture::ConsumeEntry()
        .add_stage(rhi::PipelineStageEnum::FRAGMENT_SHADER_BIT)
        .set_layout(rhi::TextureLayoutEnum::SHADER_READ_ONLY_OPTIMAL)
        .set_access(rhi::AccessFlagEnum::SHADER_READ_BIT));
      for (int i = 0; i < barriers.size(); ++i)
        editor::ImGuiContext::m_encoder->pipeline_barrier(barriers[i]);

      ImGuiIO& io = ImGui::GetIO();
      ImVec2 canvasPos = ImGui::GetCursorScreenPos();
      ImVec2 mouseCanvasPos = {
        io.MousePos.x - canvasPos.x,
        io.MousePos.y - canvasPos.y,
      };
      Singleton<EditorContext>::instance()->m_inspector.m_mouseOffset = ivec2{ 
        int(mouseCanvasPos.x), int(mouseCanvasPos.y) };

      ImGui::Image(texture->get_imgui_texture()->get_texture_id(),
        { (float)texture->m_texture->width(), (float)texture->m_texture->height() },
        { 0, 0 }, { 1, 1 });
    }
    Singleton<EditorContext>::instance()->m_viewportHovered = ImGui::IsWindowHovered();
    Singleton<EditorContext>::instance()->m_viewportFocused = ImGui::IsWindowFocused();
    ImGui::End();

    // optional windows
    // -------------------------------------------------
    if (show_scene_window) {
      ImGui::Begin("Scene Hierarchy", &show_scene_window, ImGuiWindowFlags_MenuBar); // Pass the pointer to allow closing
      if (Singleton<EditorContext>::instance()->m_sceneDisplayed.has_value()) {
        Singleton<EditorContext>::instance()->m_sceneDisplayed->draw_gui();
      }
      ImGui::End();
    }
    if (show_resources_window) {
      ImGui::Begin("Resources", &show_resources_window); // Pass the pointer to allow closing
      se::gfx::GFXContext::on_draw_gui_resources();
      ImGui::End();
    }
    if (show_configure_window) {
      ImGui::Begin("Configuration", &show_configure_window); // Pass the pointer to allow closing
      se::Configuration::on_draw_gui();
      ImGui::End();
    }
  }
  auto EditorCameraControllerScript::CameraState::set_from_transform(
    gfx::Transform const& transform) noexcept -> void {
    auto rotationMatrix = transform.rotation.toMat3();
    se::vec3 eulerAngles = se::rotation_matrix_to_euler_angles(rotationMatrix);
    se::vec3 translation = transform.translation;
    pitch = eulerAngles.x * 180 / M_FLOAT_PI;
    yaw = eulerAngles.y * 180 / M_FLOAT_PI;
    roll = eulerAngles.z * 180 / M_FLOAT_PI;
    if (std::abs(roll + 180) < 1.f) {
      pitch = -(180.f - std::abs(pitch)) * pitch / std::abs(pitch);
      yaw = (180.f - std::abs(yaw)) * yaw / std::abs(yaw);
      roll = 0.f;
    }
    if (std::abs(roll - 180) < 1.f) {
      pitch = -(180.f - std::abs(pitch)) * pitch / std::abs(pitch);
      yaw = (180.f - std::abs(yaw)) * yaw / std::abs(yaw);
      roll = 0.f;
    }

    x = translation.x;
    y = translation.y;
    z = translation.z;
  }

  auto EditorCameraControllerScript::CameraState::lerp_towards(
    CameraState const& target, float positionLerpPct,
    float rotationLerpPct) noexcept -> void {
    yaw   = lerp(rotationLerpPct, yaw, target.yaw);
    pitch = lerp(rotationLerpPct, pitch, target.pitch);
    roll  = lerp(rotationLerpPct, roll, target.roll);
    x = lerp(positionLerpPct, x, target.x);
    y = lerp(positionLerpPct, y, target.y);
    z = lerp(positionLerpPct, z, target.z);
  }

  auto EditorCameraControllerScript::CameraState::update_transform(
    gfx::Transform& transform) noexcept -> void {
    transform.rotation = se::Quaternion(se::euler_angle_degree_to_rotation_matrix(se::vec3(pitch, yaw, roll)));
    transform.translation = se::vec3(x, y, z);
  }

  auto EditorCameraControllerScript::on_init(gfx::Node& node) noexcept -> void {
    gfx::Transform* transform = node.get_component<gfx::Transform>();
    m_targetCameraState.set_from_transform(*transform);
    m_interpolatingCameraState.set_from_transform(*transform);
  }

  auto get_input_translation_direction(Input* input) noexcept -> se::vec3 {
    se::vec3 direction(0.0f, 0.0f, 0.0f);
    if (input->is_key_pressed(Input::CodeEnum::key_w)) {
      direction += se::vec3(0, 0, +1);  // forward
    }
    if (input->is_key_pressed(Input::CodeEnum::key_s)) {
      direction += se::vec3(0, 0, -1);  // back
    }
    if (input->is_key_pressed(Input::CodeEnum::key_a)) {
      direction += se::vec3(-1, 0, 0);  // left
    }
    if (input->is_key_pressed(Input::CodeEnum::key_d)) {
      direction += se::vec3(1, 0, 0);  // right
    }
    if (input->is_key_pressed(Input::CodeEnum::key_q)) {
      direction += se::vec3(0, -1, 0);  // down
    }
    if (input->is_key_pressed(Input::CodeEnum::key_e)) {
      direction += se::vec3(0, 1, 0);  // up
    }
    return direction;
  }

  auto EditorCameraControllerScript::on_update(gfx::Node& node, double delta) noexcept -> void {
    gfx::Transform* transform = node.get_component<gfx::Transform>();
    if (transform == nullptr) return;

    if (transform->m_dirtyToGPU) {
      m_targetCameraState.set_from_transform(*transform);
    }

    // check the viewport is hovered
    bool hovered = Singleton<EditorContext>::instance()->m_viewportHovered;
    bool focused = Singleton<EditorContext>::instance()->m_viewportFocused;
    auto input = gfx::GFXContext::device()->from_which_adapter()
      ->from_which_context()->get_binded_window()->get_input();
    if (input->is_mouse_button_pressed(Input::CodeEnum::mouse_button_2) && hovered && focused)
      m_inRotationMode = true;
    if (!input->is_mouse_button_pressed(Input::CodeEnum::mouse_button_2))
      m_inRotationMode = false;

    bool isDirty = false;

    if (input->is_mouse_button_pressed(Input::CodeEnum::mouse_button_2)) {
      if (m_inRotationMode) {
        input->disable_cursor();
        float x = input->get_mouse_x();
        float y = input->get_mouse_y();
        if (m_justPressedMouse) {
          m_lastX = x;
          m_lastY = y;
          m_justPressedMouse = false;
        }
        else {
          se::vec2 mouseMovement = se::vec2(x - m_lastX, y - m_lastY) *
            0.0005f * m_mouseSensitivityMultiplier *
            m_mouseSensitivity;
          if (m_invertY) mouseMovement.y = -mouseMovement.y;
          m_lastX = x;
          m_lastY = y;

          float mouseSensitivityFactor =
            m_mouseSensitivityCurve.evaluate(mouseMovement.length()) * 180. /
            3.1415926;

          m_targetCameraState.yaw -= mouseMovement.x * mouseSensitivityFactor;
          m_targetCameraState.pitch += mouseMovement.y * mouseSensitivityFactor;
          isDirty = true;
        }
      }
    }
    else if (!m_justPressedMouse) {
      input->enable_cursor();
      m_justPressedMouse = true;
    }

    // translation
    se::vec3 translation = get_input_translation_direction(input);
    translation *= delta * 0.1;

    // speed up movement when shift key held
    if (input->is_key_pressed(Input::CodeEnum::key_left_shift)) {
      translation *= 10.0f;
    }

    // modify movement by a boost factor ( defined in Inspector and modified in
    // play mode through the mouse scroll wheel)
    float y = input->get_mouse_scroll_y();
    m_boost += y * 0.01f;
    translation *= powf(2.0f, m_boost);

    m_targetCameraState.pitch = std::clamp(
      m_targetCameraState.pitch, -89.99f, 89.99f);

    se::vec3 euler_angle = {
      m_targetCameraState.pitch,
      m_targetCameraState.yaw,
      m_targetCameraState.roll,
    };

    se::mat3 mat = euler_angle_degree_to_rotation_matrix(euler_angle);
    se::vec3 rotatedFoward = mat * se::vec3(0, 0, -1);
    se::vec3 up = se::vec3(0.0f, 1.0f, 0.0f);
    se::vec3 cameraRight = se::normalize(se::cross(rotatedFoward, up));
    se::vec3 cameraUp = se::cross(cameraRight, rotatedFoward);
    se::vec3 movement = translation.z * rotatedFoward +
      translation.x * cameraRight + translation.y * cameraUp;

    m_targetCameraState.x += movement.x;
    m_targetCameraState.y += movement.y;
    m_targetCameraState.z += movement.z;

    // Framerate-independent interpolation
    // calculate the lerp amount, such that we get 99% of the way to our target
    // in the specified time
    float positionLerpPct =
      1.f - expf(log(1.f - 0.99f) / m_positionLerpTime * delta);
    float rotationLerpPct =
      1.f - expf(log(1.f - 0.99f) / m_rotationLerpTime * delta);
    m_interpolatingCameraState.lerp_towards(m_targetCameraState, positionLerpPct,
      rotationLerpPct);

    if (m_interpolatingCameraState.x != transform->translation.x ||
      m_interpolatingCameraState.y != transform->translation.y ||
      m_interpolatingCameraState.z != transform->translation.z) {
      isDirty = true;
    }

    if (transform != nullptr) {
      if (isDirty) {
        transform->m_dirtyToFile = true;
        transform->m_dirtyToGPU = true;
      }
      m_interpolatingCameraState.update_transform(*transform);
    }
  }

  auto EditorCameraControllerScript::on_end(gfx::Node& node) noexcept -> void {

  }

}
}
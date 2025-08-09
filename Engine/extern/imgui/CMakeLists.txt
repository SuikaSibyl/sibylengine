# CPP library 
set(PROJECT_NAME imgui)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(${PROJECT_NAME} STATIC)
target_include_directories(${PROJECT_NAME} PUBLIC
  "${PROJECT_SOURCE_DIR}/Engine/extern/imgui/includes"
  "${PROJECT_SOURCE_DIR}/Engine/extern/glfw"
  "${PROJECT_SOURCE_DIR}/Engine/extern/vulkan/include"
   ${Vulkan_INCLUDE_DIRS})
target_sources(${PROJECT_NAME} PRIVATE 
  "includes/imgui.cpp"
  "includes/imgui_demo.cpp"
  "includes/imgui_draw.cpp"
  "includes/imgui_tables.cpp"
  "includes/imgui_widgets.cpp"
  "includes/backends/imgui_impl_glfw.cpp"
  "includes/backends/imgui_impl_vulkan.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Vulkan::Vulkan
    glfw
)
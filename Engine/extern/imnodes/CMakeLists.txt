
set(PROJECT_NAME imnodes)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PUBLIC
  "${PROJECT_SOURCE_DIR}/Engine/extern/imnodes")
target_sources(${PROJECT_NAME} PRIVATE 
  "imnodes.cpp"
  "imnodes.h"
  "imnodes_internal.h")
target_link_libraries(${PROJECT_NAME} PRIVATE 
    imgui
)
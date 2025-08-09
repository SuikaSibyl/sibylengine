# CPP library 
set(PROJECT_NAME tinyexr)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(${PROJECT_NAME} STATIC)
target_include_directories(${PROJECT_NAME} PUBLIC include
  "${PROJECT_SOURCE_DIR}/Engine/extern/tinyexr/includes")
target_sources(${PROJECT_NAME} PRIVATE 
  "includes/miniz.cpp"
  "includes/tinyexr.cpp" )
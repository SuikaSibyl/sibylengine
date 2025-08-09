# CPP library 
set(PROJECT_NAME tinymitsuba)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(${PROJECT_NAME} STATIC)
target_include_directories(${PROJECT_NAME} PUBLIC include
  "${PROJECT_SOURCE_DIR}/Engine/extern/tinymitsuba/includes")
target_sources(${PROJECT_NAME} PRIVATE 
  "includes/tinyxml2.cpp" 
  "includes/tinyparser-mitsuba.cpp")
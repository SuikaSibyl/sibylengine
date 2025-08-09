set(PROJECT_NAME spirv-reflect)
add_library(${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PUBLIC
  "${PROJECT_SOURCE_DIR}/Engine/extern/spvreflect")

target_sources(${PROJECT_NAME} PRIVATE 
  "${PROJECT_SOURCE_DIR}/Engine/extern/spvreflect/spirvreflect/spirv_reflect.h"
  "${PROJECT_SOURCE_DIR}/Engine/extern/spvreflect/spirvreflect/spirv_reflect.cpp")
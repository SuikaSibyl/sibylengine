# Set project setting
set(PROJECT_NAME yaml)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(${PROJECT_NAME} STATIC)
file(GLOB_RECURSE INC_FILES "include/*.h")
file(GLOB_RECURSE SRC_FILES "src/*.h" "src/*.cpp")
target_sources(${PROJECT_NAME}
    PRIVATE
    ${INC_FILES}
    ${SRC_FILES})
# Set Include Folder
target_include_directories(${PROJECT_NAME} PUBLIC
    "${PROJECT_SOURCE_DIR}/Engine/extern/yaml/include"
)
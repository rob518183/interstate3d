cmake_minimum_required(VERSION 3.10)
project(Interstate3D VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add filesystem library if needed
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    find_package(Filesystem REQUIRED)
endif()

# Find packages
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

# Find Bullet
find_package(Bullet REQUIRED)

# Find Assimp using pkg-config
pkg_check_modules(ASSIMP REQUIRED assimp)

# GLM is header-only, just include it
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "GLM not found")
endif()

# Optional Vulkan (commented out for now since not installed)
# find_package(Vulkan REQUIRED)
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    message(STATUS "Vulkan found - enabling Vulkan support")
    add_definitions(-DENABLE_VULKAN)
    add_definitions(-DGLFW_INCLUDE_VULKAN)
else()
    message(STATUS "Vulkan not found - Vulkan support disabled")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/external/stb)
include_directories(${GLM_INCLUDE_DIR})
include_directories(${ASSIMP_INCLUDE_DIRS})
include_directories(/usr/include/bullet)

# Add stb library
add_library(stb STATIC ${CMAKE_SOURCE_DIR}/external/stb/stb_image.cpp)
target_include_directories(stb PUBLIC ${CMAKE_SOURCE_DIR}/external/stb)

# Source files - include all cpp files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.hpp"
)

# Input system files
set(INPUT_FILES
    src/input/InputManager.cpp
)

# Game system files
set(GAME_FILES
    src/game/VehicleDemo.cpp
)

# Core system files
set(CORE_FILES
    src/core/ConfigManager.cpp
)

# Rendering system files
set(RENDERING_FILES
    src/rendering/ShaderManager.cpp
    src/rendering/VulkanBackend.cpp
)

# Add stub files explicitly (no longer needed for physics)
list(APPEND SOURCES ${INPUT_FILES} ${GAME_FILES} ${CORE_FILES} ${RENDERING_FILES})

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${ASSIMP_LIBRARIES}
    BulletDynamics
    BulletCollision
    LinearMath
    stb
)

# If Vulkan is available, link it
if(Vulkan_FOUND)
    target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
endif()

# Enable testing
enable_testing()

# Add GTest using FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add tests subdirectory after GTest is available
add_subdirectory(tests)

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Copy shaders to build directory
file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/shaders/*")
file(COPY ${SHADER_FILES} DESTINATION ${CMAKE_BINARY_DIR}/shaders)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

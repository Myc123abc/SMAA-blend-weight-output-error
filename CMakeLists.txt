cmake_minimum_required(VERSION 3.30)
project(SMAA_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# external libraries
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
include(FetchContent)
FetchContent_Declare(
  SDL3 
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG        release-3.2.12
)
FetchContent_MakeAvailable(SDL3)

include(FetchContent)
FetchContent_Declare(
  VMA
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG        v3.2.1
)
FetchContent_MakeAvailable(VMA)

include(FetchContent)
FetchContent_Declare(
  SMAA
  GIT_REPOSITORY https://github.com/iryoku/smaa.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(SMAA)

find_package(Vulkan REQUIRED)

# includes, sources, links
set(INCLUDE
  include
)

file(GLOB_RECURSE SOURCE src/*.cpp)

set(LIBS
  SDL3::SDL3-static
  GPUOpen::VulkanMemoryAllocator
)

# compile executable
add_executable(SMAA_test ${SOURCE})

target_include_directories(SMAA_test PUBLIC
  ${INCLUDE}
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(SMAA_test PUBLIC
  ${LIBS}
  ${Vulkan_LIBRARIES}
)

target_compile_definitions(SMAA_test PUBLIC 
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_FORCE_RADIANS
)

# copy SMAA.h to shader floder
file(COPY "${smaa_SOURCE_DIR}/SMAA.hlsl"
     DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/shader")
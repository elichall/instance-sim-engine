cmake_minimum_required(VERSION 3.10)
project(InstanceSimEngine VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- System Dependencies ---
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# OpenMP only needed for BenchmarkSim
find_package(OpenMP QUIET)

# --- GLAD: try system/nix, fall back to vendor/ ---
find_library(GLAD_LIBRARY glad)
find_path(GLAD_INCLUDE_DIR glad/glad.h)
if(GLAD_LIBRARY AND GLAD_INCLUDE_DIR)
  add_library(glad STATIC IMPORTED)
  set_target_properties(glad PROPERTIES
    IMPORTED_LOCATION "${GLAD_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLAD_INCLUDE_DIR}"
  )
  set(GLAD_TARGET glad)
else()
  set(GLAD_SOURCES ${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c)
  set(GLAD_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/vendor/glad/include)
  set(GLAD_TARGET "")
endif()

# --- GLM: try system/nix, fall back to vendor/ ---
find_package(glm QUIET CONFIG)
if(NOT glm_FOUND)
  add_library(glm::glm INTERFACE)
  target_include_directories(glm::glm INTERFACE ${CMAKE_SOURCE_DIR}/vendor/)
endif()

option(BUILD_APPS "Build demo applications" ON)

add_subdirectory(src)
if(BUILD_APPS)
  add_subdirectory(apps)
endif()

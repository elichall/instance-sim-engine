cmake_minimum_required(VERSION 3.10)
project(InstanceSimEngine VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- OpenGL: required everywhere ---
find_package(OpenGL REQUIRED)

# --- GLFW: try system/nix, fall back to vendor/glfw/ ---
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
  if(EXISTS "${CMAKE_SOURCE_DIR}/vendor/glfw/CMakeLists.txt")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/glfw")
  else()
    message(FATAL_ERROR
      "GLFW3 not found. Install via your system package manager:\n"
      "  Ubuntu/Debian: sudo apt install libglfw3-dev\n"
      "  Fedora:        sudo dnf install glfw-devel\n"
      "  macOS:         brew install glfw\n"
      "Or clone https://github.com/glfw/glfw into vendor/glfw/")
  endif()
endif()

# OpenMP only needed for BenchmarkSim
find_package(OpenMP QUIET)

# --- GLAD: try system/nix, fall back to vendor/glad/ ---
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

# --- GLM: try system/nix, fall back to error ---
find_package(glm QUIET CONFIG)
if(NOT glm_FOUND)
  message(FATAL_ERROR
    "GLM not found.\n"
    "Nix users: add 'glm' to buildInputs in your flake.\n"
    "Non-Nix users: install via your system package manager:\n"
    "  Ubuntu/Debian: sudo apt install libglm-dev\n"
    "  Fedora:        sudo dnf install glm-devel\n"
    "  macOS:         brew install glm")
endif()

option(BUILD_APPS "Build demo applications" ON)

add_subdirectory(src)
if(BUILD_APPS)
  add_subdirectory(apps)
endif()

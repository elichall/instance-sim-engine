cmake_minimum_required(VERSION 3.10)
project(InstanceSimEngine VERSION 1.0)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- 1. System Dependencies (Linux) ---
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# --- 2. Vendor Dependencies (Local) ---
# Compile GLAD into a library
add_library(glad vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

# GLM is header-only, so we create an "interface" library
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE vendor/)

# --- 3. Core Engine ---
# Tell the compiler where to look for your engine's .h files
include_directories(src)

# --- 4. The Applications ---
add_executable(TestGL apps/test_gl/main.cpp)
target_link_libraries(TestGL glad glm glfw OpenGL::GL)

add_executable(OrbitalSim apps/orbital_3body/main.cpp src/GraphicsEngine.cpp)
target_link_libraries(OrbitalSim glad glm glfw OpenGL::GL)

add_executable(RoboticSim apps/robotic_manipulator/main.cpp src/GraphicsEngine.cpp)
target_link_libraries(RoboticSim glad glm glfw OpenGL::GL)
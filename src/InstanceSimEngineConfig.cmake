include(CMakeFindDependencyMacro)

# Non-Nix users: these find_dependency calls ensure OpenGL, glfw3, and glm are
# available. Install them via your system package manager (apt, brew, etc.).
# Note: glfw3 must be installed with CMake config files (e.g. libglfw3-dev),
# not just pkg-config. Most distros ship both.
find_dependency(OpenGL)
find_dependency(glfw3)
find_dependency(glm CONFIG)

include("${CMAKE_CURRENT_LIST_DIR}/InstanceSimEngineTargets.cmake")

include(CMakeFindDependencyMacro)

# Non-Nix users: these find_dependency calls ensure OpenGL, glfw3, and glm are
# available. Install them via your system package manager (apt, brew, etc.).
find_dependency(OpenGL)
find_dependency(glfw3)
find_dependency(glm CONFIG)

include("${CMAKE_CURRENT_LIST_DIR}/InstanceSimEngineTargets.cmake")

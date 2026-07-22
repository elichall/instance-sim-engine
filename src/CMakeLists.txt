add_library(GraphicsEngine STATIC GraphicsEngine.cpp)

target_include_directories(GraphicsEngine PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

if(GLAD_TARGET)
  target_link_libraries(GraphicsEngine PUBLIC glfw OpenGL::GL glm::glm PRIVATE ${GLAD_TARGET})
else()
  target_sources(GraphicsEngine PRIVATE ${GLAD_SOURCES})
  target_include_directories(GraphicsEngine PRIVATE ${GLAD_INCLUDE_DIRS})
  target_link_libraries(GraphicsEngine PUBLIC glfw OpenGL::GL glm::glm)
endif()

install(TARGETS GraphicsEngine EXPORT InstanceSimEngineTargets
  ARCHIVE DESTINATION lib
)
install(FILES GraphicsEngine.h DESTINATION include)

install(EXPORT InstanceSimEngineTargets
  FILE InstanceSimEngineTargets.cmake
  NAMESPACE InstanceSimEngine::
  DESTINATION lib/cmake/InstanceSimEngine
)

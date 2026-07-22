add_executable(OrbitalSim orbital_3-body/main.cpp)
target_link_libraries(OrbitalSim PRIVATE GraphicsEngine)

add_executable(RoboticSim robotic_manipulator/main.cpp)
target_link_libraries(RoboticSim PRIVATE GraphicsEngine)

if(OpenMP_CXX_FOUND)
  add_executable(BenchmarkSim test_benchmark/main.cpp)
  target_link_libraries(BenchmarkSim PRIVATE GraphicsEngine OpenMP::OpenMP_CXX)
endif()

add_executable(TestGL test_gl/main.cpp)
if(GLAD_TARGET)
  target_link_libraries(TestGL PRIVATE ${GLAD_TARGET} glfw OpenGL::GL)
else()
  target_sources(TestGL PRIVATE ${GLAD_SOURCES})
  target_include_directories(TestGL PRIVATE ${GLAD_INCLUDE_DIRS})
  target_link_libraries(TestGL PRIVATE glfw OpenGL::GL)
endif()

// Vendor Files
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std libraries
#include <fstream>
#include <iostream>
#include <random>

// Personal Files
#include "GraphicsEngine.h"
#include "particle.h"
#include "physics.h"

int main() {
  // --- Benchmark Configuration ---
  const int numOfParticles = 1000;
  std::array<int, 2> MY_WINDOW_AREA = {800, 800};
  std::array<int, 2> resolution = {8, 4}; // LOD Spheres!

  // Engine(Shape, maxCapacity, windowSize, wireframe, resolution, lines,
  // maxLines, dynamicColor)
  Engine::GraphicsEngine engine(Engine::Shape::SPHERE, numOfParticles + 100,
                                MY_WINDOW_AREA, false, resolution, false, 0,
                                false);
  Engine::RenderPayload payload;
  ParticleSystem sys;

  if (!engine.init()) {
    std::cerr << "Engine failed to initialize. Terminating." << std::endl;
    return -1;
  }

  // --- Cinematic Galaxy Setup (Plummer Disk) ---
  std::random_device rd;
  std::mt19937 gen(rd());

  // We use 0.001 to 0.999 to avoid dividing by zero in the Plummer math
  std::uniform_real_distribution<float> rand01(0.001f, 0.999f);
  std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
  std::normal_distribution<float> noiseDist(0.0f, 1.0f);

  std::cout << "Generating " << numOfParticles << "-Particle Plummer Galaxy..."
            << std::endl;

  // 1. Supermassive Black Hole
  float blackHoleMass = 5000.0f;
  sys.addParticleCustom(glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, 0.0f), blackHoleMass,
                        glm::vec3(1.0f, 1.0f, 1.0f), // Pure white core
                        2.0f);

  // 2. The Plummer Scale Radius (Controls how dense the core is)
  float a = 8.0f;

  // 3. Orbiting Stars
  for (int i = 1; i < numOfParticles; i++) {

    // --- PLUMMER SPHERE MATH ---
    float u = rand01(gen);
    // Inverse transform sampling for Plummer radial distance
    float r = a / std::sqrt(std::pow(u, -2.0f / 3.0f) - 1.0f);

    // Generate random 3D spherical direction
    float theta = angleDist(gen);
    float phi = std::acos(1.0f - 2.0f * rand01(gen));

    // Convert to Cartesian and FLATTEN the Y-axis by 0.1 to make a disk
    glm::vec3 pos(r * std::sin(phi) * std::cos(theta), r * std::cos(phi) * 0.1f,
                  r * std::sin(phi) * std::sin(theta));

    // --- ORBITAL VELOCITY ---
    // Calculate velocity based on horizontal distance from Black Hole
    float rxz = std::sqrt(pos.x * pos.x + pos.z * pos.z) + 0.1f;
    float orbitalVel = std::sqrt(blackHoleMass / rxz);

    // Tangential velocity (perpendicular vector) with slight vertical noise
    glm::vec3 vel(
        (-pos.z / rxz) * orbitalVel + (noiseDist(gen) * 0.01f), // Was 0.4f
        noiseDist(gen) * 0.001f, // Almost zero vertical bobbing
        (pos.x / rxz) * orbitalVel + (noiseDist(gen) * 0.01f) // Was 0.4f
    );

    float mass = 0.1f + rand01(gen) * 0.9f;
    float diameter = 0.05f + mass * 0.1f;

    // --- CLAMPED COLOR GRADIENT ---
    // Normalize r against an outer visual boundary (~3x scale radius)
    float normalizedR = std::min(r / (a * 3.0f), 1.0f);

    glm::vec3 color(1.0f - normalizedR * 0.8f, // Red drops off at edges
                    0.9f - normalizedR * 0.6f, // Green drops off
                    0.5f + normalizedR * 0.5f // Blue increases heavily at edges
    );

    sys.addParticleCustom(pos, vel, mass, color, diameter);
  }

  // Tie the payload to the system
  payload.spatialMats = &sys.spatialMats;
  payload.colors = &sys.colors;

  // --- Time & Performance Tracking ---
  const float dt = 0.016f;
  float t = static_cast<float>(glfwGetTime());
  float accumulator = 0.0f;

  std::ofstream logFile("benchmark_performance.csv");
  logFile << "ParticleCount,PhysicsTimeMS,RenderTimeMS,TotalFPS\n";

  double previousTime = glfwGetTime();
  int frameCount = 0;
  int totalPhysicsTicks = 0; // NEW: Track exact ticks for accurate averages

  double physTime = 0.0;
  double renderTime = 0.0;

  std::cout << "Starting benchmark. Logging to benchmark_performance.csv..."
            << std::endl;

  // --- Main Simulation Loop ---
  while (!engine.shouldClose()) {
    float currentTime = static_cast<float>(glfwGetTime());
    float currentDelta = currentTime - t; // Renamed to avoid shadowing
    t = currentTime;

    frameCount++;
    if (currentTime - previousTime >= 1.0) {

      // Calculate the true average time of a single physics tick
      double avgPhysTick = (totalPhysicsTicks > 0)
                               ? (physTime * 1000.0) / totalPhysicsTicks
                               : 0.0;

      logFile << numOfParticles << "," << (physTime * 1000) << ","
              << (renderTime * 1000) << "," << frameCount << "\n";

      // Print real-time stats to terminal
      std::cout << "FPS: " << frameCount << " | Avg Phys Tick: " << avgPhysTick
                << "ms"
                << " | Total Phys: " << (physTime * 1000) << "ms"
                << " | Render: " << (renderTime * 1000) << "ms\n";

      frameCount = 0;
      totalPhysicsTicks = 0;
      previousTime = currentTime;
      physTime = double(0);
      renderTime = double(0);
    }

    if (currentDelta > 0.25f)
      currentDelta = 0.25f; // Catch lag spikes
    accumulator += currentDelta;

    // --- Physics Step ---
    double framePhysTime = 0.0;

    while (accumulator >= dt) {
      double physStart = glfwGetTime();
      orbitalPhysics(sys, dt);
      framePhysTime += glfwGetTime() - physStart;

      accumulator -= dt;
      totalPhysicsTicks++;
    }

    // Add to the total 1.0s tracker exactly ONCE per frame
    physTime += framePhysTime;

    // --- Render Step ---
    double renderStart = glfwGetTime();
    engine.renderFrame(payload);
    renderTime += glfwGetTime() - renderStart;
  }

  return 0;
}

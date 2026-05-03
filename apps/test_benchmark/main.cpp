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
    
    // Engine(Shape, maxCapacity, windowSize, wireframe, resolution, lines, maxLines, dynamicColor)
    Engine::GraphicsEngine engine(Engine::Shape::SPHERE, numOfParticles + 100, MY_WINDOW_AREA, false, resolution, false, 0, false);
    Engine::RenderPayload payload;
    ParticleSystem sys;

    if (!engine.init()) {
        std::cerr << "Engine failed to initialize. Terminating." << std::endl;
        return -1;
    }

    // --- Random Generation Setup ---
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);
    std::uniform_real_distribution<float> velDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> massDist(0.1f, 2.0f);
    std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);

    std::cout << "Generating " << numOfParticles << " randomized particles..." << std::endl;

    for (int i = 0; i < numOfParticles; i++) {
        // Generate random position
        glm::vec3 pos(posDist(gen), posDist(gen), posDist(gen));
        
        // Give it a subtle spin based on its position to create a "galaxy" effect
        glm::vec3 vel(-pos.z * 0.2f + velDist(gen), velDist(gen)*0.1f, pos.x * 0.2f + velDist(gen));
        
        float mass = massDist(gen);
        float diameter = 0.1f + (mass / 2.0f) * 0.2f; 
        glm::vec3 color(colorDist(gen), colorDist(gen)*0.5f, 1.0f - colorDist(gen)); // Cool fiery colors

        sys.addParticleCustom(pos, vel, mass, color, diameter);
    }

    // Tie the payload to the system
    payload.spatialMats = &sys.spatialMats;
    payload.colors      = &sys.colors;

    // --- Time & Performance Tracking ---
    const float dt = 0.016f; 
    float t = static_cast<float>(glfwGetTime());
    float accumulator = 0.0f;

    std::ofstream logFile("benchmark_performance.csv");
    logFile << "ParticleCount,PhysicsTimeMS,RenderTimeMS,TotalFPS\n";

    double previousTime = glfwGetTime();
    int frameCount = 0;
    double physTime = 0.0;
    double renderTime = 0.0;

    std::cout << "Starting benchmark. Logging to benchmark_performance.csv..." << std::endl;

    // --- Main Simulation Loop ---
    while (!engine.shouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        float frameTime = currentTime - t;
        t = currentTime;

        frameCount++;
        if (currentTime - previousTime >= 1.0) {
            logFile << numOfParticles << "," 
                    << (physTime * 1000) << "," 
                    << (renderTime * 1000) << "," 
                    << frameCount << "\n";
            
            // Print real-time stats to terminal
            std::cout << "FPS: " << frameCount << " | Phys: " << (physTime*1000) << "ms | Render: " << (renderTime*1000) << "ms\n";

            frameCount = 0;
            previousTime = currentTime;
            physTime = double(0);
            renderTime = double(0);
        }

        if (frameTime > 0.25f) frameTime = 0.25f; // Catch lag spikes
        accumulator += frameTime;

        // --- Physics Step ---
        while (accumulator >= dt) {
            double physStart = glfwGetTime();
            orbitalPhysics(sys, dt);
            physTime += glfwGetTime() - physStart;

            accumulator -= dt;
        }
        
        // --- Render Step ---
        double renderStart = glfwGetTime();
        engine.renderFrame(payload);
        renderTime += glfwGetTime() - renderStart;
    }

    return 0;
}
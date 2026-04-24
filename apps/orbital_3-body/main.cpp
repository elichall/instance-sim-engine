// Vendor Files
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std libraries
#include <fstream>
#include <iostream>

// Personal Files
#include "GraphicsEngine.h"
#include "particle.h"
#include "physics.h"

int main() {
    // --- Objects ---
    Engine::GraphicsEngine engine = {Engine::Shape::SPHERE, numOfParticles, MY_WINDOW_AREA};
    Engine::RenderPayload payload;

    ParticleSystem sys;

    // --- Time Keeping ---
    const float dt = 0.001f; 
    float t = static_cast<float>(glfwGetTime());
    float accumulator = 0.0f;

    // --- Performance Tracking ---
    std::ofstream logFile("orbital_performance.csv");
    logFile << "ParticleCount,PhysicsTimeMS,RenderTimeMS,TotalFPS,EnergyDrift\n";

    // compute time and frame rates for the graphics and physics engines
    double previousTime = glfwGetTime();
    float simTime = 0.0;
    int frameCount = 0;
    double physTime = 0.0;
    double renderTime = 0.0;
    
    // --- Setup Objects ---
    if (!engine.init()) {
        std::cerr << "Engine failed to initialize. Terminating." << std::endl;
        return -1; // Safely exit the program instead of crashing
    }

    // tie the payload to the system
    payload.spatialMats = &sys.spatialMats;
    payload.colors      = &sys.colors;

    // populate system and prepare for energy drift storage
    for (int i=0; i<numOfParticles; i++) {
        sys.addParticle(i);
    }
    double baselineEnergy = calculateSystemEnergy(sys);

    // sim for loop
    while (!engine.shouldClose()) {
        // update time
        float currentTime = static_cast<float>(glfwGetTime());
        float frameTime = currentTime - t;
        t = currentTime;

        // update framecount and track performance
        frameCount++;
        if (currentTime - previousTime >= 1.0) {
            double currentEnergy = calculateSystemEnergy(sys);
            double drift = std::abs(currentEnergy - baselineEnergy);

            logFile << numOfParticles << "," 
                    << (physTime*1000) << "," 
                    << (renderTime*1000) << "," 
                    << frameCount << "," 
                    << drift <<"\n";

            frameCount = 0;
            previousTime = currentTime; // Reset for the next second
            physTime = double(0);
            renderTime = double(0);
        }

        // catch massive lag spikes
        if (frameTime > 0.25f) {
            frameTime = 0.25f;
        }
        accumulator += frameTime;

        while (accumulator >= dt) {

            // update particles
            double physStart = glfwGetTime();
            orbitalPhysics(sys, dt);
            physTime += glfwGetTime() - physStart;

            // accounting for the time simulated
            simTime += dt;

            // track the first cycle time
            if (isFigure8 && !hasCompletedOrbit) {
                if (simTime >= montgomeryChenCycleTime) {
                    glm::vec3 posVec = sys.spatialMats[0][3];

                    float dist = glm::distance(posVec, position0[0]);

                    std::cout << "\n=========================================\n";
                    std::cout << "[VALIDATION] Figure-8 Orbit Complete!\n";
                    std::cout << "Sim Time: " << simTime << "s\n";
                    std::cout << "Closure Error: " << dist << " units\n";
                    std::cout << "=========================================\n" << std::endl;

                    hasCompletedOrbit = true;
                }
            }

            accumulator -= dt;
        }
    
    double renderStart = glfwGetTime();
    engine.renderFrame(payload);
    renderTime += glfwGetTime() - renderStart;

    }
    return 0;
}
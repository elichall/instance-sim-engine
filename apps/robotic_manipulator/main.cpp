// vendor files
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// standard libraries
#include <iostream>
#include <vector>

// headers
#include "GraphicsEngine.h" 
#include "robot.h"
#include "kinematics.h"
#include "trajectory.h"
#include "config.h"

int main() {
    // Initalize engine
    Engine::GraphicsEngine engine(Engine::CUBE, INSTANCE_OBJECTS_NUMBER_EST, MY_WINDOW_AREA, true, {1, 1}, true, LINE_DATA_LENGTH_EST, false);
    if (!engine.init()) return -1;
    Engine::RenderPayload payload;

    // Setup Robot & Data
    RobotSystem robot;
    
    // 1. Load the computed joint angles
    std::vector<RobotState> trajectory = loadControlPath("../apps/robotic_manipulator/data/control-path.csv");
    
    // 2. Load the ideal trajectory to compare against
    std::vector<glm::vec3> desiredPath = loadDesiredPath("../apps/robotic_manipulator/data/trajectory.csv");
    
    if (trajectory.empty() || desiredPath.empty()) return -1;
    
    // 3. Pre-calculate the ACTUAL path by running the joint angles through the Forward Kinematics 
    std::vector<glm::vec3> actualPath;
    for (const auto& state : trajectory) {
        updateManipulatorKinematics(robot, state);
        // The end effector is the final link in the hierarchy (index 4).
        // The translation vector (x,y,z) is stored in the 4th column of its transformation matrix.
        glm::vec3 endEffectorPos = glm::vec3(robot.spatialMats[4][3]);
        actualPath.push_back(endEffectorPos);
    }

    std::vector<Engine::LineData> traceLines(2);
    traceLines[0].points = desiredPath;
    traceLines[0].color = glm::vec3(1.0f, 1.0f, 1.0f); // white
    traceLines[0].drawCount = desiredPath.size();
    
    traceLines[1].points = actualPath;
    traceLines[1].color = glm::vec3(1.0f, 0.0f, 0.0f); // red

    // Time Management Variables
    float systemTime = 0.0f;
    float timeFactor = 1.0f; // Adjusted for the new dt timestep
    int currentIndex = 0;
    double lastTime = glfwGetTime();

    // Tie render payload to the correct data addresses
    payload.spatialMats = &robot.spatialMats;
    payload.colors = &robot.colors;
    payload.lines = &traceLines;

    while (!engine.shouldClose()) {
        
        // --- Time Management ---
        double currentGlfwTime = glfwGetTime();
        float dt = static_cast<float>(currentGlfwTime - lastTime);
        lastTime = currentGlfwTime;
        systemTime += (dt / timeFactor);

        // Loop the animation
        if (systemTime >= trajectory.back().time) {
            systemTime = 0.0f;
            currentIndex = 0;
        }

        // --- Kinematics ---
        // Interpolate linearly if not perfectly on a timestep
        RobotState interpolatedState;
        for (size_t i = currentIndex; i < trajectory.size() - 1; i++) {
            if (systemTime >= trajectory[i].time && systemTime < trajectory[i+1].time) {
                currentIndex = i;
                float weight = (systemTime - trajectory[i].time) / (trajectory[i+1].time - trajectory[i].time);
                
                interpolatedState.theta1 = glm::mix(trajectory[i].theta1, trajectory[i+1].theta1, weight);
                interpolatedState.d      = glm::mix(trajectory[i].d,      trajectory[i+1].d,      weight);
                interpolatedState.theta2 = glm::mix(trajectory[i].theta2, trajectory[i+1].theta2, weight);
                break;
            }
        }

        // Update memory
        updateManipulatorKinematics(robot, interpolatedState);

        // Update line draw count for actual path to acheive an animation effect
        traceLines[1].drawCount = currentIndex;

        // --- Render Frame ---
        engine.renderFrame(payload);
    }

    return 0;
}
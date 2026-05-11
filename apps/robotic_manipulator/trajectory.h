#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm> // For std::min
#include <glm/glm.hpp>
#include "robot.h"

// Loads the desired path from trajectory.csv (X, Y)
inline std::vector<glm::vec3> loadDesiredPath(const std::string& filepath) {
    std::vector<glm::vec3> path;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open CSV file at " << filepath << "!\n";
        return path;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        float x = 0.0f, y = 0.0f;

        try {
            if (std::getline(ss, token, ',')) x = std::stof(token);
            if (std::getline(ss, token, ',')) y = std::stof(token);
            
            // Set Z to 0.0f for a 2D planar path
            path.push_back(glm::vec3(x, y, 0.0f));
        } catch (...) {
            std::cerr << "WARNING: Failed to parse coordinate: " << line << "\n";
        }
    }
    
    std::cout << "SUCCESS: Loaded " << path.size() << " desired path coordinates.\n";
    return path;
}

// Loads the control path from control-path.csv (Rows: theta 1, d, theta 2)
inline std::vector<RobotState> loadControlPath(const std::string& filepath, float dt = 0.05f) {
    std::vector<RobotState> data;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open CSV file at " << filepath << "!\n";
        return data;
    }

    std::string line;
    std::vector<float> theta1, d, theta2;

    // Read the file horizontally
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string token;
        
        // First token is the row header (e.g., "theta 1")
        std::getline(ss, token, ',');
        
        std::vector<float>* targetList = nullptr;
        if (token.find("theta 1") != std::string::npos) targetList = &theta1;
        else if (token.find("d") != std::string::npos) targetList = &d;
        else if (token.find("theta 2") != std::string::npos) targetList = &theta2;

        if (targetList) {
            while (std::getline(ss, token, ',')) {
                try {
                    targetList->push_back(std::stof(token));
                } catch (...) {
                    // Ignore trailing commas or bad string conversions
                }
            }
        }
    }

    // Assemble the rows vertically into RobotStates
    size_t numFrames = std::min({theta1.size(), d.size(), theta2.size()});
    for (size_t i = 0; i < numFrames; i++) {
        RobotState state;
        
        // Since control-path.csv doesn't have explicit timestamps, we assign them sequentially
        state.time = static_cast<float>(i) * dt; 
        
        state.theta1 = theta1[i];
        state.d = d[i];
        state.theta2 = theta2[i];
        
        data.push_back(state);
    }

    std::cout << "SUCCESS: Loaded " << data.size() << " control frames from " << filepath << "\n";
    return data;
}

#endif
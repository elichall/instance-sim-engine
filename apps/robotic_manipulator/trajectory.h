#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
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
            path.push_back(glm::vec3(x, y, 0.0f));
        } catch (...) {
            // Ignore bad lines
        }
    }
    std::cout << "SUCCESS: Loaded " << path.size() << " desired path coordinates.\n";
    return path;
}

// Loads the control path from control-path.csv (Rows: theta 1, d, theta 2)
// Adapted for a 1000 Hz control loop (dt = 0.001 seconds)
inline std::vector<RobotState> loadControlPath(const std::string& filepath, float dt = 0.001f) {
    std::vector<RobotState> data;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open CSV file at " << filepath << "!\n";
        return data;
    }

    std::string line;
    std::vector<float> theta1, d, theta2;
    int rowIndex = 0;

    // Read the file horizontally by Row Index instead of String Headers
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string token;
        
        std::vector<float>* targetList = nullptr;
        if (rowIndex == 0) targetList = &theta1;
        else if (rowIndex == 1) targetList = &d;
        else if (rowIndex == 2) targetList = &theta2;

        if (targetList) {
            while (std::getline(ss, token, ',')) {
                try {
                    targetList->push_back(std::stof(token));
                } catch (...) {
                    // Ignore empty trailing commas
                }
            }
        }
        rowIndex++;
    }

    // Assemble the rows vertically into RobotStates
    size_t numFrames = std::min({theta1.size(), d.size(), theta2.size()});
    for (size_t i = 0; i < numFrames; i++) {
        RobotState state;
        
        // Map the columns to exact 1000 Hz physical time
        state.time = static_cast<float>(i) * dt; 
        
        state.theta1 = theta1[i];
        state.d = d[i];
        state.theta2 = theta2[i];
        
        data.push_back(state);
    }

    std::cout << "SUCCESS: Loaded " << data.size() << " control frames (" 
              << data.back().time << " seconds of simulation) from " << filepath << "\n";
    return data;
}

#endif
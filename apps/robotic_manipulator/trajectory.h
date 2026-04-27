#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <fstream>
#include <sstream>

void generateTestPaths(int numFrames, std::vector<glm::vec3>& outDesired, std::vector<glm::vec3>& outActual) {
    outDesired.clear();
    outActual.clear();
    for (int i = 0; i < numFrames; i++) {
        float progress = static_cast<float>(i) / static_cast<float>(numFrames - 1);
        float startX = 1.0f;
        float endX = -1.0f;
        float currentX = startX + progress * (endX - startX);
        float currentY = 0.5f; 
        float currentZ = 0.0f;

        outDesired.push_back(glm::vec3(currentX, currentY, currentZ));
        
        float errorZ = sin(progress * 15.0f) * 0.1f; 
        outActual.push_back(glm::vec3(currentX, currentY, currentZ + errorZ));
    }
}

std::vector<RobotState> loadSimulationData(const std::string& filepath) {
    std::vector<RobotState> data;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open CSV file at " << filepath << "!\n";
        std::cerr << "Check your file path and working directory." << std::endl;
        return data; 
    }

    std::string line;

    // Skip the header row 
    if (std::getline(file, line)) {
        // Header skipped, comment out to not skip
    }

    
    while (std::getline(file, line)) {
        // Skip empty lines (common at the end of CSVs)
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        RobotState state;

        try {
            // Read time
            std::getline(ss, token, ',');
            state.time = std::stof(token);

            // Read theta1
            std::getline(ss, token, ',');
            state.theta1 = std::stof(token);

            // Read d
            std::getline(ss, token, ',');
            state.d = std::stof(token);

            // Read theta2
            std::getline(ss, token, ',');
            state.theta2 = std::stof(token);

            data.push_back(state);
            
        } catch (const std::exception& e) {
            // catch failed read
            std::cerr << "WARNING: Failed to parse line: '" << line << "'\n";
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }

    file.close();
    std::cout << "SUCCESS: Loaded " << data.size() << " frames from " << filepath << std::endl;

    return data;
};

#endif
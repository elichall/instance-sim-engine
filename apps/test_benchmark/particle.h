#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/gtc/matrix_transform.hpp>
#include "GraphicsEngine.h"

class ParticleSystem {
public:
    // rendering data
    std::vector<glm::vec3> positions;
    std::vector<glm::mat4> spatialMats;
    std::vector<glm::vec3> colors;

    // physics data
    std::vector<glm::vec3> velocityVec;
    std::vector<glm::vec3> accelerations;
    std::vector<float> masses;

    void addParticleCustom(glm::vec3 pos, glm::vec3 vel, float mass, glm::vec3 color, float diameter) {
        glm::mat4 rotTranMat = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 scalingMat = glm::scale(glm::mat4(1.0f), glm::vec3(diameter));

        spatialMats.push_back(rotTranMat * scalingMat);
        positions.push_back(spatialMats.back()[3]);
        colors.push_back(color);
        velocityVec.push_back(vel);
        masses.push_back(mass);
        accelerations.push_back(glm::vec3(0.0f));
    }
};

#endif
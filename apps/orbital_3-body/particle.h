#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/gtc/matrix_transform.hpp>

#include "config.h"
#include "GraphicsEngine.h"

class ParticleSystem {
public:
    // rendering data
    std::vector<glm::mat4> spatialMats;
    std::vector<glm::vec3> colors;

    // physics data
    std::vector<glm::vec3> velocityVec;
    std::vector<glm::vec3> accelerations;
    std::vector<float> masses;

    void addParticle(int idx) {
        glm::mat4 rotTranMat = glm::translate(glm::mat4(1.0f), position0[idx]);
        glm::mat4 scalingMat = glm::scale(glm::mat4(1.0f), glm::vec3(diameter0[idx]));

        spatialMats.push_back(rotTranMat * scalingMat);
        colors.push_back(color0[idx]);
        velocityVec.push_back(velocity0[idx]);
        masses.push_back(mass0[idx]);
        accelerations.push_back(glm::vec3(0.0f));
    }
};

#endif
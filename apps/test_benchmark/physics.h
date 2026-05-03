#ifndef PHYSICS_H
#define PHYSICS_H

#include <omp.h>

#include "particle.h"

inline constexpr float gravConst = 1.0f;
inline constexpr float epsilon = 0.01f;

// changed from smart loop to brute force parallel process loop for speed
// smart optimizations like Newtons third law won't work if tasks are split between processors
void orbitalPhysics(ParticleSystem& system, float dt) {
    int sysSize = system.positions.size();

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < sysSize; i++) {
        glm::vec3 localAcc(0.0f);
        glm::vec3 posI = system.positions[i];

        for (int j = 0; j < sysSize; j++) {
            if (i == j) continue;

            glm::vec3 rVec = system.positions[j] - posI;
            float rSquared = glm::dot(rVec, rVec) + epsilon;
            
            float invR = 1.0f / glm::sqrt(rSquared);
            float invRCubed = invR * invR * invR;

            localAcc += gravConst * invRCubed * rVec * system.masses[j];
        }
        
        system.accelerations[i] = localAcc;
    }

    #pragma omp parallel for schedule(static)
    for (int i=0; i<sysSize; i++) {
        /*
        can't do classic kinematic 1/2 a t^2 + v*t because its "double counting" the accerlation
        find new velocity then find the position from it
        */
        system.velocityVec[i] += system.accelerations[i] * dt;

        system.positions[i] += system.velocityVec[i] * dt; 
        
        // glm uses column row notation
        system.spatialMats[i][3][0] = system.positions[i].x;
        system.spatialMats[i][3][1] = system.positions[i].y;
        system.spatialMats[i][3][2] = system.positions[i].z;
    }
}

double calculateSystemEnergy(const ParticleSystem& system) {
    double kinetic = 0.0;
    double potential = 0.0;
    int sysSize = system.positions.size();

    for (int i = 0; i < sysSize; i++) {
        double vSquared = glm::dot(system.velocityVec[i], system.velocityVec[i]);
        kinetic += 0.5 * system.masses[i] * vSquared;

        glm::vec3 posI = system.positions[i]; 
        for (int j = i + 1; j < sysSize; j++) {
            glm::vec3 posJ = system.positions[j];
            float rMag = glm::distance(posI, posJ);
            
            // Apply softening to potential energy to keep math strict
            float rMagSoftened = glm::sqrt(rMag*rMag + epsilon);
            potential -= (gravConst * system.masses[i] * system.masses[j]) / rMagSoftened;
        }
    }
    return kinetic + potential;
}

#endif
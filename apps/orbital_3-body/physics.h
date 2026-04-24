#ifndef PHYSICS_H
#define PHYSICS_H

#include <algorithm> // fill

#include "particle.h"

void orbitalPhysics(ParticleSystem& system, float dt) {
    int sysSize = system.spatialMats.size();

    std::fill(system.accelerations.begin(), system.accelerations.end(), glm::vec3(0.0f)); // wipe accelerations

    for (int i=0; i<sysSize; i++) {
        for (int j=i+1; j<sysSize; j++) {
            glm::vec3 posI = system.spatialMats[i][3];
            glm::vec3 posJ = system.spatialMats[j][3];

            glm::vec3 rVec = posJ - posI;

            float rSquared = glm::dot(rVec, rVec);
            float rCubed = rSquared * glm::sqrt(rSquared);

            glm::vec3 accOverMass = gravConst / rCubed * rVec;

            system.accelerations[i] += accOverMass * system.masses[j];
            system.accelerations[j] -= accOverMass * system.masses[i];
        }
    }

    for (int i=0; i<sysSize; i++) {
        /*
        can't do classic kinematic 1/2 a t^2 + v*t because its "double counting" the accerlation
        find new velocity then find the position from it
        */
        system.velocityVec[i] += system.accelerations[i] * dt;

        glm::vec3 dx = system.velocityVec[i] * dt; 
        
        // glm uses column row notation
        system.spatialMats[i][3][0] += dx.x;
        system.spatialMats[i][3][1] += dx.y;
        system.spatialMats[i][3][2] += dx.z;
    }
}

double calculateSystemEnergy(const ParticleSystem& system) {
    double kinetic = 0.0;
    double potential = 0.0;
    int sysSize = system.spatialMats.size();

    for (int i = 0; i < sysSize; i++) {
        
        double vSquared = glm::dot(system.velocityVec[i], system.velocityVec[i]);
        kinetic += 0.5 * system.masses[i] * vSquared;

        glm::vec3 posI = system.spatialMats[i][3];
        for (int j = i + 1; j < sysSize; j++) {
            glm::vec3 posJ = system.spatialMats[j][3];
            float rMag = glm::distance(posI, posJ);
            
            potential -= (gravConst * system.masses[i] * system.masses[j]) / rMag;
        }
    }
    return kinetic + potential;
}

#endif
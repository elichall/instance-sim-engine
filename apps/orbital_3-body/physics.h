#ifndef PHYSICS_H
#define PHYSICS_H

#include "particle.h"

void orbitalPhysics(ParticleSystem& system, float dt) {
    int sysSize = system.positions.size();

    for (int i=0; i<sysSize; i++) {
        for (int j=i+1; j<sysSize; j++) {
            glm::vec3 posI = system.positions[i];
            glm::vec3 posJ = system.positions[j];

            glm::vec3 rVec = posJ - posI;

            float rSquared = glm::dot(rVec, rVec);
            float invR = 1.0f / glm::sqrt(rSquared);
            float invRCubed = invR * invR * invR;

            glm::vec3 accOverMass = gravConst * invRCubed * rVec;

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

        system.positions[i] += system.velocityVec[i] * dt; 
        
        // glm uses column row notation
        system.spatialMats[i][3][0] = system.positions[i].x;
        system.spatialMats[i][3][1] = system.positions[i].y;
        system.spatialMats[i][3][2] = system.positions[i].z;

        system.accelerations[i] = glm::vec3(0.0f);
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
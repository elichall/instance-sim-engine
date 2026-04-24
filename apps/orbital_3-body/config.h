#ifndef CONFIG_H
#define CONFIG_H

#include <glm/gtc/matrix_transform.hpp>

const std::array<int,2> MY_WINDOW_AREA = {800, 800};
const int numOfParticles = 3;

const float gravConst = 1.0f; // 6.6743 * powf(10,-11)

// evaluating stability of figure 8 config if using it for benchmarking
const bool isFigure8 = true;
bool hasCompletedOrbit = false;
const float montgomeryChenCycleTime = 6.32591398f; 

// Initial Positions (x, y, z) for figure 8 solution
glm::vec3 position0[3] = {
    { 0.97000436f, -0.24308753f, 0.0f}, // Body 1 (Bottom Right)
    {-0.97000436f,  0.24308753f, 0.0f}, // Body 2 (Top Left)
    { 0.0f,         0.0f,        0.0f}  // Body 3 (Center)
};

// Initial Velocities (x, y, z)
glm::vec3 velocity0[3] = {
    { 0.4662036850f,  0.4323657300f, 0.0f}, // Body 1 moves Up-Right
    { 0.4662036850f,  0.4323657300f, 0.0f}, // Body 2 moves Up-Right
    {-0.93240737f,   -0.86473146f,   0.0f}  // Body 3 moves Down-Left
};

glm::mat4 rotation0[3] = {// identity for now
    glm::mat4(1.0f),
    glm::mat4(1.0f),
    glm::mat4(1.0f)
};

glm::mat4 angVel0[3] = {
    glm::mat4(1.0f),
    glm::mat4(1.0f),
    glm::mat4(1.0f)
};

// --- Particle Specifications --- 
float mass0[3] = {
    1.0f, 
    1.0f, 
    1.0f
};

float diameter0[3] = {
    0.25f, 
    0.25f, 
    0.25f
};

glm::vec3 color0[3] = {
    {0.8f, 0.2f, 0.2f}, // Body 1: Red
    {0.2f, 0.8f, 0.2f}, // Body 2: Green
    {0.2f, 0.3f, 0.9f}  // Body 3: Blue
};
// density, size, color

#endif
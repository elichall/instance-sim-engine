# Robotic Manipulator Kinematic Visualizer

An interactive 3D visualization of a robotic manipulator, driven by external control trajectories and rendered using highly optimized Instanced OpenGL.

This application demonstrates the separation of robotic control logic from graphical rendering. By ingesting pre-computed joint-space parameters (generated via external solvers like MATLAB or Python), the engine smoothly interpolates and renders the physical state of the robot in real-time, completely decoupled from the system's frame rate.

## Physics & Mathematics

* **Forward Kinematics (D-H Parameters):** The spatial positioning of each robotic link is calculated dynamically using standard Denavit-Hartenberg (D-H) transformation matrices. 
* **Data-Oriented Memory (SoA):** Instead of using legacy Object-Oriented representations of robotic links, the application flattens the D-H matrices and scaling weights into a contiguous block of memory (`std::vector<glm::mat4> spatialMats`). This Structure-of-Arrays (SoA) approach allows the application to stream the entire robot's physical state to the GPU's VRAM in a single memory transfer.
* **Time-Decoupled Interpolation:** The visualizer tracks real-world time via `glfwGetTime()`. It searches the ingested trajectory data and linearly interpolates the joint parameters (`theta1`, `d`, `theta2`) between the two closest frames, guaranteeing butter-smooth animation regardless of CPU bottlenecks.

## Path Tracing

To validate control algorithms, the application utilizes the engine's `GL_LINE_STRIP` overlay to dynamically draw end-effector paths over time:
* **White Path:** The theoretical "Desired" target path for the end effector.
* **Red Path:** The "Actual" measured path the end effector is taking, visually highlighting control deviations, overshoot, or steady-state error.

## Execution
The application requires a valid CSV trajectory file to run. The CMake build process is configured to pull this data automatically.

Ensure your trajectory file is located at `apps/robotic_manipulator/data/trajectory.csv` before running the executable from your build directory:

```bash
./RoboticSim
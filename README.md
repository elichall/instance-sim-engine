# Instance Sim Engine

A high-performance, Data-Oriented C++ simulation and visualization framework. 

This engine was built from scratch to decouple complex mathematical solvers from the render thread. It utilizes a Structure-of-Arrays (SoA) memory architecture and zero-allocation continuous physics loops to maximize CPU cache locality. Rendering is handled via highly optimized OpenGL Instanced Rendering pipelines, capable of rendering thousands of independent objects in a single draw call.

## Repository Architecture (Monorepo)
The core engine provides the physics abstractions and rendering pipeline, while specific engineering and physics simulations are containerized as individual "Apps".

* **[Orbital 3-Body Simulation](./apps/orbital_3body/README.md)** - A numerically stable $N$-body gravity simulator demonstrating Symplectic Euler integration and the Montgomery-Chen Figure-8 solution.
* **[Robotic Manipulator (WIP)](#)** - Forward/Inverse kinematics visualization utilizing D-H parameter transformation matrices.

## Dependencies
* **C++ Engine:** OpenGL (3.3+), GLFW, GLAD, GLM
* **Data Analysis:** Python 3, Pandas, Matplotlib

## Build & Execute Instructions
This project uses a standard out-of-source CMake build sequence.

```bash
# 1. Generate build files
mkdir build && cd build
cmake ..

# 2. Compile the engine (Uses all available CPU cores)
make -j$(nproc)

# 3. Execute the target App
./OrbitalSim
```

## Profiling & Analysis
The engine natively logs millisecond-accurate profiling data and physical system states to a CSV. To generate the engineering validation reports:

```bash
# From the root directory:
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python scripts/analyze_performance.py
```
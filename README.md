# Instance Sim Engine

A high-performance, Data-Oriented C++ simulation and visualization framework. 

This engine was built from scratch to decouple complex mathematical solvers from the render thread. It utilizes a Structure-of-Arrays (SoA) memory architecture and zero-allocation continuous physics loops to maximize CPU cache locality. Rendering is handled via highly optimized OpenGL Instanced Rendering pipelines, capable of rendering thousands of independent objects in a single draw call.

## Repository Architecture (Monorepo)
The core engine provides the physics abstractions and rendering pipeline, while specific engineering and physics simulations are containerized as individual "Apps".

* **[Orbital 3-Body Simulation](./apps/orbital_3-body/README.md)** - A numerically stable $N$-body gravity simulator demonstrating Symplectic Euler integration and the Montgomery-Chen Figure-8 solution.
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
python apps/orbital_3-body/analyze_orbital.py
```

## AI Usage Acknowledgment

This project utilized a Large Language Model (Gemini) as an interactive technical partner to assist in the architectural unification and validation of the simulation engine. The collaboration focused on translating high-level engineering goals into performant C++ and Python implementations.

Key areas of AI assistance included:

* **Architectural Sounding Board:** Providing feedback on Data-Oriented Design (SoA) patterns and monorepo structure to ensure long-term project scalability.
* **Computational Physics Tutoring:** Advising on the selection and implementation of symplectic integration methods to maintain numerical stability in chaotic systems.
* **Technical Troubleshooting:** Assisting with the configuration of modern C++ build systems (CMake), graphics pipeline debugging, and environment management.
* **Documentation & Reporting:** Collaborative drafting of technical documentation and automated analysis scripts to visualize engine performance.

**Human-Driven Logic:** While AI served as a valuable tool for accelerating development and enforcing industry best practices, the core structural decisions, logic synthesis, and final implementation remain entirely human-driven. Critical optimizations and the strategic isolation of profiling metrics to preserve real-time performance are the direct result of human architectural intuition and project vision.
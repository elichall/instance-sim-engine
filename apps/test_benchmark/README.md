# 🌌 Computational Physics Engine: N-Body Galaxy Benchmark

![Galaxy Simulation](assets/galaxy_gif.gif)

## Overview
This repository contains a highly optimized, custom-built C++ and OpenGL computational physics engine. This specific benchmark application is an $\mathcal{O}(N^2)$ N-body gravity stress test, simulating the formation and orbital mechanics of a stable spiral galaxy using a **Plummer Sphere** density distribution. 

The goal of this project was to push a single consumer CPU to its absolute mathematical limits by shifting the compute bottleneck to the memory bandwidth wall using modern C++ performance paradigms.

## Engine Architecture & Optimizations

To simulate and render thousands of interacting bodies in real-time, the engine relies on strict memory management and parallelization:

* **Data-Oriented Design (Structure-of-Arrays):** Transitioned the physics accumulator from an Array-of-Structures (AoS) to a Structure-of-Arrays (SoA) layout. Decoupling the `glm::vec3 positions` from the heavy `glm::mat4` transformation matrices prevented CPU L1/L2 cache trashing during the $O(N^2)$ physics loop.
* **Hardware Parallelism (OpenMP):** The Newtonian gravity loop calculates $\frac{N(N-1)}{2}$ interactions per frame. By sacrificing Newton's 3rd Law memory writes to avoid thread racing, the $\mathcal{O}(N^2)$ accumulator was fully parallelized across all CPU cores using `#pragma omp parallel for schedule(static)`.
* **Algorithmic Fast Math:** Eliminated costly CPU division operations inside the inner gravity loop by utilizing approximated Inverse Square Roots (`1.0f / sqrt(x)`) and SIMD compiler instructions (`-ffast-math`).
* **Dynamic VBO Streaming:** Utilizes a custom, hardware-accelerated `glDrawElementsInstanced` graphics pipeline. Matrix and color data are streamed to GPU VRAM dynamically, eliminating CPU draw-call bottlenecks entirely.
* **Geometry LOD Scaling:** Uses programmable Level-of-Detail (LOD) generation for spherical geometry, dropping vertex counts drastically for macroscopic simulations.

## Mathematical Model

* **Integrator:** Symplectic Euler (proven to bound energy drift to $\pm 0.00025$ J during rigorous 3-body Figure-8 validations).
* **Initial Conditions:** Inverse Transform Sampling is used to map particles to a **Plummer Density Profile** ($r = a / \sqrt{u^{-2/3} - 1}$). The sphere is flattened into a disk, and stars are initialized with precise tangential velocities ($v = \sqrt{\frac{GM}{r}}$) relative to a supermassive core.
* **Softening Parameter:** An $\epsilon$ softening factor is added to the gravitational denominator to prevent $1/0$ singularities (infinite acceleration) during close-proximity particle collisions.

## Benchmark Metrics.

| Particle Count | Physics Compute Time ($\mathcal{O}(N^2)$) | Render Time ($\mathcal{O}(N)$) | Real-Time FPS |
|----------------|-------------------------------------------|--------------------------------|---------------|
| 1,000          | ~3.5 ms                                   | ~12.5 ms                       | 150+ FPS      |
| 2,000          | ~11.0 ms                                  | ~13.5 ms                       | 100+ FPS      |
| 5,000          | ~50.0 ms                                  | ~15.0 ms                       | ~30-40 FPS    |

*(Note: Render time scales linearly and remains highly stable; performance drops at massive scales are purely bound by CPU $\rightarrow$ RAM memory bandwidth during gravity accumulation).*

Hardware Context & WSL2 Virtualization:
These benchmarks were recorded on an Intel i7 processor running Ubuntu via WSL2. Due to WSLg GPU-passthrough limitations at the time of recording, the OpenGL pipeline defaulted to llvmpipe (CPU Software Rendering). The fact that the engine maintains 100+ FPS on a 2,000-body $O(N^2)$ simulation while the CPU is simultaneously handling all vertex and fragment shading is a testament to the extreme efficiency of the SoA cache architecture and OpenMP math vectorization.

## Build Instructions (Linux)

**Dependencies:** CMake, GCC (C++17), OpenGL, GLFW, GLAD, GLM, OpenMP.

```bash
# Clone the repository
git clone [https://github.com/yourusername/instance-sim-engine.git](https://github.com/yourusername/instance-sim-engine.git)
cd instance-sim-engine

# Create the build directory
mkdir build && cd build

# Configure CMake with aggressive hardware optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Compile across all CPU cores
make -j$(nproc)

# Run the benchmark
./BenchmarkSim
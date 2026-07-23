# Instance Sim Engine

A high-performance, Data-Oriented C++ simulation and visualization framework. 

This engine was built from scratch to decouple complex mathematical solvers from the render thread. It utilizes a Structure-of-Arrays (SoA) memory architecture and zero-allocation continuous physics loops to maximize CPU cache locality. Rendering is handled via highly optimized OpenGL Instanced Rendering pipelines, capable of rendering thousands of independent objects in a single draw call.

## Repository Architecture (Monorepo)
The core engine provides the physics abstractions and rendering pipeline, while specific engineering and physics simulations are containerized as individual "Apps".

* **[Orbital 3-Body Simulation](./apps/orbital_3-body/README.md)** - A numerically stable $N$-body gravity simulator demonstrating Symplectic Euler integration and the Montgomery-Chen Figure-8 solution.
* **[Robotic Manipulator](./apps/robotic_manipulator/README.md)** - Forward kinematics visualization utilizing Denavit-Hartenberg (D-H) parameter transformation matrices and dynamic control-path tracing.
* **[N-Body Galaxy Benchmark](./apps/test_benchmark/README.md)** - A multi-threaded $\mathcal{O}(N^2)$ gravity stress test simulating a 5,000+ particle spiral galaxy via a Plummer Sphere distribution. Showcases OpenMP parallelization and SIMD-friendly inverse square root approximations.

## Dependencies
* **C++ Engine:** OpenGL (3.3+), GLFW, GLAD, GLM
* **Optional:** OpenMP (only for N-Body Galaxy Benchmark)
* **Data Analysis:** Python 3, Pandas, Matplotlib

## Build & Execute Instructions

There are three ways to build this project. Pick whichever fits your workflow.

### Option 1: Nix (Recommended)

Requires [Nix](https://nixos.org/download.html) with flakes enabled.

```bash
# Drop into a dev shell with all dependencies available
nix develop

# Build everything
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run an app
./build/apps/OrbitalSim
```

To build just the engine library (for use in other projects):
```bash
nix build .#engine
# Output: result/lib/libGraphicsEngine.a + result/include/GraphicsEngine.h
```

### Option 2: System Packages

Install dependencies via your system package manager, then build with CMake.

**Arch Linux:**
```bash
sudo pacman -S cmake gcc glfw-x11 glm
```

**Ubuntu/Debian:**
```bash
sudo apt install cmake g++ libglfw3-dev libglm-dev
```

**Fedora:**
```bash
sudo dnf install cmake gcc-c++ glfw-devel glm-devel
```

Then build:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/OrbitalSim
```

### Option 3: Vendor Dependencies

Clone the required headers/libs into `vendor/` for a fully self-contained build.

```bash
# GLAD (OpenGL 3.3 Core) — generate at https://glad.dav1d.de/
#   Language: C/C++, API: gl=3.3, Profile: core, Loader: yes
#   Place output as: vendor/glad/{src/glad.c, include/glad/glad.h, include/KHR/khrplatform.h}

# GLM (header-only math library)
git clone https://github.com/g-truc/glm.git vendor/glm

# GLFW (install via system packages — too large to vendor)
#   sudo pacman -S glfw-x11   OR   sudo apt install libglfw3-dev
```

Then build:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/OrbitalSim
```

## Using the Engine in Other Projects

The engine can be consumed as a pre-built static library or by compiling the source directly.

### As a Nix Flake Input

```nix
# In your project's flake.nix
inputs = {
  nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  instance-sim-engine.url = "github:yourname/instance-sim-engine";
};

outputs = { self, nixpkgs, instance-sim-engine, ... }:
  # ... per-system setup ...
  {
    packages.${system}.my-app = pkgs.stdenv.mkDerivation {
      buildInputs = [ instance-sim-engine.packages.${system}.engine ];
      # In your CMakeLists.txt: find_package(InstanceSimEngine REQUIRED)
    };
  };
```

### As a CMake Subdirectory

```cmake
# Your project's CMakeLists.txt
add_subdirectory(path/to/instance-sim-engine)
target_link_libraries(myapp PRIVATE GraphicsEngine)
```

### By Compiling Source Directly

Copy `src/GraphicsEngine.h` and `src/GraphicsEngine.cpp` into your project, then compile them alongside your code. You will need GLFW, OpenGL, GLM, and GLAD available at link time.

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

This project utilized Large Language Models (Gemini & Claude) as an interactive technical partner to assist in the architectural unification and validation of the simulation engine. The collaboration focused on translating high-level engineering goals into performant C++ and Python implementations. The primary purpose of which was to learn and refine skills not taught through classical mechanical engineering education.

Key areas of AI assistance included:

* **Architectural Sounding Board:** Providing feedback on Data-Oriented Design (SoA) patterns and monorepo structure to ensure long-term project scalability.
* **Computational Physics Tutoring:** Advising on the selection and implementation of symplectic integration methods to maintain numerical stability in chaotic systems, as well as applying inverse transform sampling for astrophysics distributions.
* **Technical Troubleshooting:** Assisting with the configuration of modern C++ build systems (CMake), OpenMP multithreading constraints, graphics pipeline debugging, and environment management.
* **Documentation & Reporting:** Collaborative drafting of technical documentation and automated analysis scripts to visualize engine performance.

**Human-Driven Logic:** While AI served as a valuable tool for accelerating development and enforcing industry best practices, the core structural decisions, logic synthesis, and final implementation remain entirely human-driven. Critical optimizations and the strategic isolation of profiling metrics to preserve real-time performance are the direct result of human architectural intuition and project vision.

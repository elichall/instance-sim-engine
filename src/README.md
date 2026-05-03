# Graphics Engine (OpenGL Instanced Renderer)

The `GraphicsEngine` is a custom, high-performance C++ rendering wrapper built on top of OpenGL. It is designed to visualize large-scale physics and engineering simulations in real-time by entirely decoupling the simulation logic from the rendering pipeline.

It utilizes **Instanced Rendering** to draw thousands of 3D objects in a single draw call, and a **Zero-Copy Payload** architecture to read simulation data directly from the CPU cache without duplicating memory.

## 🚀 Key Features
* **Massive Instancing:** Render 10,000+ objects simultaneously using `glDrawElementsInstanced`.
* **Dynamic VBO Resizing:** GPU buffers automatically scale up (by 1.5x) if the simulation object count exceeds the initial capacity.
* **Zero-Copy Payload:** The engine reads transformation matrices and colors directly via pointers, avoiding expensive memory copies.
* **Built-in Camera:** Integrated spherical-coordinate camera with mouse-drag rotation and scroll-wheel zooming.
* **Multi-Primitive Support:** Procedurally generated Level-of-Detail (LOD) Spheres and Cubes.
* **Line Rendering:** Dedicated shader pipeline for drawing continuous trajectory lines or kinematic links.

## 🛠 Dependencies
* **OpenGL 3.3+** (Core Profile)
* **GLFW** (Window and Input Management)
* **GLAD** (OpenGL Function Loader)
* **GLM** (Mathematics Library)

---

## How to Use the Engine
### 1. Initialization
Include the engine and create an instance. The constructor is highly configurable to optimize memory based on your simulation's needs.

```cpp
#include "GraphicsEngine.h"

// Example: 5000 Spheres, Window 800x800, Wireframes OFF, Low-Poly (12x6), Lines ON
Engine::GraphicsEngine engine(
    Engine::Shape::SPHERE,  // Base geometry shape
    5000,                   // Estimated max object capacity (Auto-scales if exceeded)
    {800, 800},             // Window resolution
    false,                  // Disable black wireframe outlines
    {12, 6},                // Sphere LOD resolution (Sectors x Stacks)
    true,                   // Enable line rendering
    1000,                   // Estimated max line vertices
    false                   // Dynamic colors OFF (Uploads colors to GPU only once)
);

// Initialize GLFW, GLAD, and allocate GPU memory
if (!engine.init()) {
    std::cerr << "Engine failed to initialize!" << std::endl;
    return -1;
}
```

### 2. The Render Payload
The engine does not own the simulation data. Instead, you pack pointers to your simulation's Data-Oriented arrays into a `RenderPayload` struct. 

```cpp
// 1. Your simulation data (Structure-of-Arrays)
std::vector<glm::mat4> spatialMats; 
std::vector<glm::vec3> colors;      
std::vector<Engine::LineData> lines; 

// 2. Link your data to the Payload
Engine::RenderPayload payload;
payload.spatialMats = &spatialMats;
payload.colors = &colors;
payload.lines = &lines;
```

### 3. The Render Loop
Pass the payload to `renderFrame()` every tick. The engine will automatically update the GPU buffers and draw the scene.

```cpp
while (!engine.shouldClose()) {
    // ... [Run Physics/Math Solvers Here] ...
    
    // Draw the frame
    engine.renderFrame(payload);
}
```

---

## Configuration Details

### Constructor Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| `instanceShape` | `Engine::Shape` | `CUBE` or `SPHERE`. Dictates the base geometry loaded into the VBO. |
| `maxInstanceCapacity` | `int` | Initial VRAM allocation. If `payload.spatialMats->size()` exceeds this, the engine reallocates VRAM dynamically. |
| `windowSize` | `std::array<int,2>` | The Width and Height of the GLFW window. |
| `usingWireFrame` | `bool` | If true, draws a black wireframe over the geometry (Uses `GL_POLYGON_OFFSET_LINE` to prevent z-fighting). |
| `resolution` | `std::array<int,2>` | **Spheres Only:** `{sectors, stacks}`. Higher numbers yield smoother spheres but cost more Vertex Shader time. |
| `usingLineFlag` | `bool` | Allocates the Line Shader pipeline. Set to `true` for orbits or robotic arms. |
| `maxLinePoints` | `int` | Initial VRAM allocation for line vertices. Auto-scales if exceeded. |
| `dynamicColorFlag` | `bool` | `true`: Streams colors to the GPU every frame (Fire/Heat maps).<br>`false`: Uploads colors to the GPU exactly once on frame 1 (Saves PCIe bandwidth). |

### Line Rendering
If `usingLineFlag` is enabled, you can pass line objects to the payload. A line is defined as a continuous strip of points.

```cpp
Engine::LineData orbitPath;
orbitPath.color = glm::vec3(1.0f, 1.0f, 1.0f); // White line
orbitPath.points = { vec3(0,0,0), vec3(1,1,1), vec3(2,0,2) };
orbitPath.drawCount = orbitPath.points.size();

std::vector<Engine::LineData> allLines;
allLines.push_back(orbitPath);

payload.lines = &allLines;
```

### Camera Controls
The engine includes a built-in spherical-coordinate camera mapped to GLFW callback events.

Rotate: Click and hold Left Mouse Button + Drag.

Zoom: Scroll Mouse Wheel up and down (Clamped between radius 1.0 and 50.0).
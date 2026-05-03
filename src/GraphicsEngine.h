#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <vector>
#include <array>

namespace Engine {

    enum Shape {
        CUBE = 0,
        SPHERE = 1
    };

    struct InstanceObjectShape {
    public:
        InstanceObjectShape(int desiredInstanceShape, std::array<int,2> resolution);

        int instanceShape;
        std::array<int,2> resolution;

        std::vector<GLfloat> shapeVertices;
        std::vector<GLuint>  shapeIndices;

    private:
        void fillShapeContainers();
        void buildSphere();
    };

    struct LineData {
        std::vector<glm::vec3> points; // the actual dots to connect
        glm::vec3 color;               // the color of this specific line
        int drawCount;                 // how many points to draw
    };

    struct RenderPayload {
        const std::vector<glm::mat4>* spatialMats = nullptr; // complete spatial scaling, rotating, and translating mat
        const std::vector<glm::vec3>* colors = nullptr;      // color of the instance

        const std::vector<LineData>* lines = nullptr;        // any lines desired to be drawn that frame     
    };

    class Camera {
    public:
        Camera(std::array<int,2> windowSize={800,800});

        glm::mat4 viewMat();

        void processMouseMovement(float xoffset, float yoffset);

        void processMouseScroll(float yoffset);

    private:
        float radius; // set a starting radius
        float yaw;   // Initialize looking down the Z axis
        float pitch;  // Initialize level with the equator

        const glm::vec3 targetVec = {0.0f, 0.0f, 0.0f}; // Focal point
        const glm::vec3 upVec = {0.0f, 1.0f, 0.0f};     // Global up
    };

    class GraphicsEngine {
    public:
        // --- Constructor ---
        GraphicsEngine( int instanceShape, 
                        int maxInstanceCapacity=10, 
                        std::array<int,2> windowSize={800,800},
                        bool usingWireFrame=true,
                        std::array<int,2> resolution={36,18}, 
                        bool usingLineFlag=false,
                        int maxLinePoints=1000,
                        bool dynamicColorFlag=false );
        // --- Destructor ---
        ~GraphicsEngine();

        // --- Public Methods ---
        // Setup GL
        bool init();
        // render a frame
        void renderFrame(const RenderPayload& payload);
        // watches for closed window
        bool shouldClose();
        // if the colors are static, pass the colors to the engine


    private:
        // --- Flags ---
        bool isLine; // is the engine going to be dealing with lines
        bool isFrame; // does the user want wire frames
        bool isDragging=false; // a flag for the camera
        bool isDynamicColor; // does the engine need to dynamically update color
        bool staticColorsAssigned=false; // has the engine been assigned static color

        // --- Stored Values ---
        float lastX;
        float lastY;

        int maxCapacity; // estimated max capacity of instanced objects over sim
        std::array<int,2> winSize;
        int maxLineSize;

        // --- Private Methods ---
        // Shaders
        bool compileShaders();
        // Create Gl objects
        bool glObjects();
        // Poling events (must be static to throw to glfw cause its in C and can't accept object tied functions)
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void cursorPositionCallback(GLFWwindow* window, double xposIn, double yposIn);
        static void scrollCallback(GLFWwindow* window, double xoffsetIn, double yoffsetIn);

        // --- Objects ---
        InstanceObjectShape shape;
        GLFWwindow* window;
        Camera camera;

        // GL objects
        GLuint VAO, geometryVBO, instanceVBO, colorVBO, EBO; 
        GLuint modelLoc, viewLoc, projLoc, wireframeColorLoc, isWireframeLoc;
        GLuint shaderProgram;

        // for line
        GLuint lineVAO, lineVBO;
        GLuint lineViewLoc, lineProjLoc, lineColorLoc;
        GLuint lineShaderProgram;
        
    };
}

#endif
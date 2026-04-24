// OpenGL Related
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standard Libraries 
#include <iostream>

// Personal Files
#include "GraphicsEngine.h"

const int MY_GLFW_MAJOR_VERSION = 3;
const int MY_GLFW_MINOR_VERSION = 3;

namespace Engine {
    /*
    ==============================================================================================
                                        GRAPHICS ENGINE 
    ==============================================================================================
    */
    // --- Constructor ---
    GraphicsEngine::GraphicsEngine( int instanceShape, 
                                    int maxInstanceCapacity,
                                    std::array<int,2> windowSize,
                                    bool usingLineFlag,
                                    int maxLinePoints ) 
                                    : shape(instanceShape), 
                                      maxCapacity(maxInstanceCapacity), 
                                      winSize(windowSize),
                                      isLine(usingLineFlag),
                                      maxLineSize(maxLinePoints),
                                      camera(windowSize) {
                                        lastX = windowSize[0] / 2;
                                        lastY = windowSize[1] / 2;
                                        isDragging = false;
                                      }

    // call at start of script to set up the engine
    bool GraphicsEngine::init() {
        // --- Setup GLFW ---
        // glfw's internal initalization
        glfwInit();

        // version of GLFW
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MY_GLFW_MAJOR_VERSION);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MY_GLFW_MINOR_VERSION);

        // core or complementary profile (informs glfw about what features it has)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // --- GLFW End Setup ---

        // --- Window Object Setup ---
        //                                    (width, height, title, fullscreen?, N/A)
        window = glfwCreateWindow(winSize[0], winSize[1], "Instance Engine", NULL, NULL);
        if (window == NULL) {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        // tells glfw we want to use the window in our current context (OpenGL object)
        glfwMakeContextCurrent(window);

        glfwSetWindowUserPointer(window, this);

        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
        glfwSetScrollCallback(window, scrollCallback);

        // glad integrates openGL with the glfw window object 
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        // area of the window we want open gl to render in (can integrate other things into one window)
        glViewport(0, 0, winSize[0], winSize[1]);
        // --- End of Window Object Setup ---

        // --- Optimization Options ---
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        // --- Prepare a Back Buffer Frame ---
        //           R, B, G, transparency (1-0 range)
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        // clean the back buffer and apply the new colour
        glClear(GL_COLOR_BUFFER_BIT);

        // swap the frame (buffer) into active (front)
        glfwSwapBuffers(window);

        compileShaders();

        glObjects();

        return true;
    };

    // call every loop to update instance objects with payload
    void GraphicsEngine::renderFrame(const RenderPayload& payload) {
        // Prepare to draw in back buffer
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 
        // create camera matrices
        glm::mat4 viewMat = camera.viewMat();
        // calculate aspect ratio dynamically
        float aspect = (height > 0) ? ((float)width / (float)height) : 1.0f;
        glm::mat4 projMat = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        int instObjectNum = payload.spatialMats->size();
        
        // Check if payload is larger than set capacity
        if (instObjectNum > maxCapacity) {
            // set new capacity (1.5x)
            maxCapacity = static_cast<int>(maxCapacity * 1.5f);
            if (instObjectNum > maxCapacity) {
                maxCapacity = instObjectNum; // catch case where it spiked massively
            }

            // Reallocate the GPU buffers
            // calling glBufferData destroys the old memory and allocates new memory
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData( GL_ARRAY_BUFFER, 
                          maxCapacity * sizeof(glm::mat4), 
                          nullptr, 
                          GL_DYNAMIC_DRAW );
            
            glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
            glBufferData( GL_ARRAY_BUFFER, 
                          maxCapacity * sizeof(glm::vec3), 
                          nullptr, 
                          GL_DYNAMIC_DRAW );
        }

        // --- Stream Data to VRAM ---
        glUseProgram(shaderProgram);

        // pass camera matrices to the instance shader program
        glUniformMatrix4fv( viewLoc, 
                            1, 
                            GL_FALSE, 
                            glm::value_ptr(viewMat) );
        glUniformMatrix4fv( projLoc, 
                            1, 
                            GL_FALSE, 
                            glm::value_ptr(projMat) );

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        // glBufferSubData replaces existing memory
        glBufferSubData( GL_ARRAY_BUFFER,                   // Target
                         0,                                 // Byte Offset
                         instObjectNum * sizeof(glm::mat4), // Total Byte Size
                         payload.spatialMats->data() );   // Pointer to start of data in memory block
        
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferSubData( GL_ARRAY_BUFFER, 
                         0, 
                         instObjectNum * sizeof(glm::vec3), 
                         payload.colors->data() );

        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind to protect state

        // --- Draw the Solid Cubes ---
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform1i(isWireframeLoc, 0); // toggle solid fill (use instance colors)

        int numCubeIndices = shape.shapeIndices.size();
        glDrawElementsInstanced( GL_TRIANGLES,    // type of primitive
                                 numCubeIndices,  // how any indicies we want to draw
                                 GL_UNSIGNED_INT, // type of data in each spot
                                 0,               // number of verticies from start we want to draw
                                 instObjectNum );  // number of object instances

        // --- Draw the Wireframe Edges ---
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // Set color to Black (RGBA)
        glUniform1i(isWireframeLoc, 1); // Toggle wireframe mode
        glUniform4f(wireframeColorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
        
        // Apply offset to stop clipping
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f); 
        // actually draw wireframe
        glDrawElementsInstanced( GL_TRIANGLES, 
                                 numCubeIndices, 
                                 GL_UNSIGNED_INT, 
                                 0, 
                                 instObjectNum);
        // disable offset to prevent issues next frame
        glDisable(GL_POLYGON_OFFSET_LINE);
        
        // Draw any lines the user specifies
        if (isLine && payload.lines != nullptr && !payload.lines->empty()) { // double protection
            glUseProgram(lineShaderProgram);

            // pass camera matrices to the instance shader program
            glUniformMatrix4fv( lineViewLoc, 
                                1, 
                                GL_FALSE, 
                                glm::value_ptr(viewMat) );
            glUniformMatrix4fv( lineProjLoc, 
                                1, 
                                GL_FALSE, 
                                glm::value_ptr(projMat) );

            glBindVertexArray(lineVAO);
            glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

            for (const auto& line : *payload.lines) {
                int numPoints = line.points.size();
                
                // catch too many points
                if (numPoints > maxLineSize) {
                    maxLineSize = static_cast<int>(numPoints * 1.5f); // scale the size

                    // reallocate buffer
                    glBufferData( GL_ARRAY_BUFFER, 
                                  maxLineSize * sizeof(glm::vec3), 
                                  nullptr, 
                                  GL_DYNAMIC_DRAW );
                }

                glUniform3fv(lineColorLoc, 1, glm::value_ptr(line.color));

                glBufferSubData( GL_ARRAY_BUFFER, 
                                 0, 
                                 numPoints * sizeof(glm::vec3), 
                                 line.points.data() );
                
                glDrawArrays(GL_LINE_STRIP, 0, line.drawCount);
            }
        }

        // advance the frame and listen to the inputs
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    bool GraphicsEngine::compileShaders() {
        // --- Populate Shader Items ---
        const char* instancedVertexShaderSource = "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "layout (location = 1) in mat4 instanceMatrix;\n" 
            "layout (location = 5) in vec3 aColor;\n" // NEW: Color attribute
            "out vec3 ParticleColor;\n"               // NEW: Output to rasterizer
            "uniform mat4 view;\n"
            "uniform mat4 projection;\n"
            "void main()\n"
            "{\n"
            "   ParticleColor = aColor;\n"            // Pass the data through
            "   gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);\n"
            "}\0";
        const char* fragmentShaderSource = "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec3 ParticleColor;\n"                // NEW: Input from Vertex Shader
            "uniform vec4 wireframeColor;\n"          // NEW: For the black edges
            "uniform int isWireframe;\n"              // NEW: State toggle
            "void main()\n"
            "{\n"
            "   if(isWireframe == 1)\n"
            "       FragColor = wireframeColor;\n"
            "   else\n"
            "       FragColor = vec4(ParticleColor, 1.0f);\n" // Append 1.0f for Alpha
            "}\n\0";

        // --- Compile Object Shaders ---
        // make a shader object and define it
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        // reference value, one string for whole shader, point to source code, NULL 
        glShaderSource(vertexShader, 1, &instancedVertexShaderSource, NULL);
        // GPU can't understand source code so we need to compile it and pass it to GPU
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram); // links the program to GL

        glDeleteShader(vertexShader); // frees the VRAM of these objects
        glDeleteShader(fragmentShader);

        modelLoc = glGetUniformLocation(shaderProgram, "model");
        viewLoc = glGetUniformLocation(shaderProgram, "view");
        projLoc = glGetUniformLocation(shaderProgram, "projection");
        wireframeColorLoc = glGetUniformLocation(shaderProgram, "wireframeColor");
        isWireframeLoc = glGetUniformLocation(shaderProgram, "isWireframe");

        // --- Do the Same for Line ---
        if (isLine) {
            const char* lineVertexShaderSource = "#version 330 core\n"
                "layout (location = 0) in vec3 aPos;\n"
                "uniform mat4 view;\n"
                "uniform mat4 projection;\n"
                "void main()\n"
                "{\n"
                "   gl_Position = projection * view * vec4(aPos, 1.0);\n"
                "}\0";

            const char* lineFragmentShaderSource = "#version 330 core\n"
                "out vec4 FragColor;\n"
                "uniform vec3 lineColor;\n" // Color passed from C++
                "void main()\n"
                "{\n"
                "   FragColor = vec4(lineColor, 1.0f);\n" 
                "}\n\0";

            // --- Compile Line Shaders ---
            GLuint lineVS = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(lineVS, 1, &lineVertexShaderSource, NULL);
            glCompileShader(lineVS);

            GLuint lineFS = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(lineFS, 1, &lineFragmentShaderSource, NULL);
            glCompileShader(lineFS);

            lineShaderProgram = glCreateProgram();
            glAttachShader(lineShaderProgram, lineVS);
            glAttachShader(lineShaderProgram, lineFS);
            glLinkProgram(lineShaderProgram);

            glDeleteShader(lineVS); // frees the VRAM of these objects
            glDeleteShader(lineFS);

            // Get Uniform Locations for the line shader
            lineViewLoc = glGetUniformLocation(lineShaderProgram, "view");
            lineProjLoc = glGetUniformLocation(lineShaderProgram, "projection");
            lineColorLoc = glGetUniformLocation(lineShaderProgram, "lineColor");
        }

        return true;
    };

    bool GraphicsEngine::glObjects() {
        // --- Creating and Passing Memory Buffers for Instances ---

        // vertex array object (how to use and interpret VBO)
        glGenVertexArrays(1, &VAO); // gen VAO before VBO
        glBindVertexArray(VAO);

        // vertex buffer object (actual data points)
        glGenBuffers(1, &geometryVBO);
        glBindBuffer(GL_ARRAY_BUFFER, geometryVBO); // binds what we do to VBO to the native gl array buffer of the same type
        
        // Pass data to GPU 
        glBufferData( GL_ARRAY_BUFFER,                              // type of object
                      shape.shapeVertices.size() * sizeof(GLfloat), // size of data array in bytes
                      shape.shapeVertices.data(),                   // the actual data
                      GL_STATIC_DRAW );                             // what we want to do with it GL
        // static(mod one and used many times),dynamic(modified a lot),stream(mod one and used a few times))_(draw,read,copy)
        
        // tell GPU how to read the buffer array
        int dataPerVertex = 3;
        glVertexAttribPointer( 0,               // pos of vertext attribute
                               dataPerVertex,               // size of data array
                               GL_FLOAT,        // data type
                               GL_FALSE,        // only matters if coords are ints
                               dataPerVertex*sizeof(float), // sprite of verticies (amount of data between each vertex)
                               (void*)0 );      // offset to where data begins
        glEnableVertexAttribArray(0); 

        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        int numOfInstanceObjects = maxCapacity;
        glBufferData( GL_ARRAY_BUFFER, 
                      numOfInstanceObjects * sizeof(glm::mat4), 
                      nullptr, 
                      GL_DYNAMIC_DRAW);

        // VBO can't contain mat4s so need to teach it to read 4 vec4s as a mat4
        std::size_t vec4Size = sizeof(glm::vec4); 
        for (unsigned int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(1 + i);
            // location, size, type, normalized, stride, offset
            glVertexAttribPointer( 1 + i, // location
                4,                        // size
                GL_FLOAT,                 // type
                GL_FALSE,                 // normalized?
                sizeof(glm::mat4),        // stride
                (void*)(i * vec4Size) );  // offset
        
            /* 
            The critical instancing function: tells the GPU to only advance
            this attribute once per instance, rather than once per vertex. 
            */  
            glVertexAttribDivisor(1 + i, 1); 
        }

        // color VBO 
        glGenBuffers(1, &colorVBO);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData( GL_ARRAY_BUFFER, 
                      numOfInstanceObjects * sizeof(glm::vec3), 
                      nullptr, 
                      GL_DYNAMIC_DRAW);

        int dataPerColor = 3; // RBG
        glVertexAttribPointer( 5, // not the number of particles but instead the location specified in shader 
                               dataPerColor, 
                               GL_FLOAT, 
                               GL_FALSE, 
                               sizeof(glm::vec3), 
                               (void*)0 );
        glEnableVertexAttribArray(5);
        glVertexAttribDivisor(5, 1); // Step once per particle instance

        // EBO (order of reading the geometry vertices)
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, 
                      shape.shapeIndices.size() * sizeof(GLuint), 
                      shape.shapeIndices.data(), 
                      GL_STATIC_DRAW );

        if (isLine) {
            glGenVertexArrays(1, &lineVAO);
            glBindVertexArray(lineVAO);

            glGenBuffers(1, &lineVBO); 
            glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
            glBufferData( GL_ARRAY_BUFFER,
                          maxLineSize * sizeof(glm::vec3),
                          nullptr,
                          GL_DYNAMIC_DRAW);

            int nDimSpace = 3; // number of data points needed to specify one point
            glVertexAttribPointer( 0, 
                                   nDimSpace, 
                                   GL_FLOAT, 
                                   GL_FALSE, 
                                   sizeof(glm::vec3), 
                                   (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0); // Unbind
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
        glBindVertexArray(0);

        return true;
    }

    bool GraphicsEngine::shouldClose() {
        return glfwWindowShouldClose(window);
    }
    // --- Deconstructor ---
    // free up memory after closing graphics engine object
    GraphicsEngine::~GraphicsEngine() {
        // Delete GPU buffers
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &geometryVBO);
        glDeleteBuffers(1, &instanceVBO);
        glDeleteBuffers(1, &colorVBO);
        glDeleteBuffers(1, &EBO);
        
        if (isLine) {
            glDeleteVertexArrays(1, &lineVAO);
            glDeleteBuffers(1, &lineVBO);
            glDeleteProgram(lineShaderProgram);
        }

        // delete shader program
        glDeleteProgram(shaderProgram);

        // close window and terminate GLFW and 
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    /*
    ==============================================================================================
                                    INSTANCE OBJECT SHAPE 
    ==============================================================================================
    */
    // --- Constructor ---
    InstanceObjectShape::InstanceObjectShape(int desiredInstanceShape) {
        instanceShape = desiredInstanceShape;
        fillShapeContainers();
    }

    void InstanceObjectShape::fillShapeContainers() {
        if (instanceShape == CUBE) {
            shapeVertices = {
                -0.5f, -0.5f, 0.5f, // front Lower left corner
                0.5f, -0.5f, 0.5f, // front Lower right corner
                0.5f, 0.5f, 0.5f, // front Upper right corner 
                -0.5f, 0.5f, 0.5f, // front Upper left corner

                -0.5f, -0.5f, -0.5f, // back Lower left corner
                0.5f, -0.5f, -0.5f, // back Lower right corner
                0.5f, 0.5f, -0.5f, // back Upper right corner
                -0.5f, 0.5f, -0.5f // back Upper left corner
            };
            
            shapeIndices = {
                0, 1, 2,   2, 3, 0, // Front face
                1, 5, 6,   6, 2, 1, // Right 
                5, 4, 7,   7, 6, 5, // Back  
                4, 0, 3,   3, 7, 4, // Left  
                3, 2, 6,   6, 7, 3, // Top   
                4, 5, 1,   1, 0, 4  // Bottom
            }; 

        } else if (instanceShape == SPHERE) {
            buildSphere();
        } else {
            std::cout << "Shape Not Recognized" << std::endl;
        }
    }

    void InstanceObjectShape::buildSphere() {
        float radius = 0.5f; // 1 unit diameter sphere

        int sectors = 36;
        int stacks = 18;

        for (int i = 0; i <= stacks; i++) {
            float ph = M_PI / 2.0f - M_PI * (float)i / (float)stacks;
            float cosph = cosf(ph);
            float sinph = sinf(ph);

            for (int j = 0; j <= sectors; j++) {
                float th = 2.0f * M_PI * (float)j / (float)sectors;

                GLfloat x = radius * cosph * sinf(th);
                GLfloat y = radius * sinph;
                GLfloat z = radius * cosph * cosf(th);

                shapeVertices.push_back(x);
                shapeVertices.push_back(y);
                shapeVertices.push_back(z);
            }
        }

        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < sectors; ++j) {
                GLuint k1 = i * (sectors + 1) + j;
                GLuint k2 = k1 + sectors + 1;

                // Top stack
                if (i != 0) { 
                    shapeIndices.push_back(k1);
                    shapeIndices.push_back(k2);
                    shapeIndices.push_back(k1 + 1);
                }
                // Bottom stack
                if (i != (stacks - 1)) { 
                    shapeIndices.push_back(k1 + 1);
                    shapeIndices.push_back(k2);
                    shapeIndices.push_back(k2 + 1);
                }
            }
        }
    }

    /*
    ==============================================================================================
                                            CAMERA 
    ==============================================================================================
    */
    Camera::Camera(std::array<int,2> windowSize) {
        radius = 5.0f; // set a starting radius
        yaw = 90.0f;   // initialize looking down the Z axis
        pitch = 0.0f;  // initialize level with the equator
    }

    glm::mat4 Camera::viewMat() {
        // Convert degrees to radians
        float radYaw = glm::radians(yaw);
        float radPitch = glm::radians(pitch);

        // spherical-to-cartesian transformation
        float camX = radius * cosf(radPitch) * cosf(radYaw);
        float camY = radius * sinf(radPitch);
        float camZ = radius * cosf(radPitch) * sinf(radYaw);

        glm::vec3 cameraPos = glm::vec3(camX, camY, camZ);
        return glm::lookAt(cameraPos, targetVec, upVec);
    }

    void Camera::processMouseMovement(float xoffset, float yoffset) {
        float sensitivity = 0.2f; // how fast the camera rotates
        yaw += xoffset * sensitivity;
        pitch += yoffset * sensitivity;

        // prevent gimbal lock
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    void Camera::processMouseScroll(float yoffset) {
        float zoomSensitivity = 1.0f;
        radius -= yoffset * zoomSensitivity;

        // clamp the zoom bounds
        if (radius < 1.0f) radius = 1.0f;
        if (radius > 50.0f) radius = 50.0f;
    }

    void GraphicsEngine::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        GraphicsEngine* engine = static_cast<GraphicsEngine*>(glfwGetWindowUserPointer(window));

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) engine->isDragging = true;
            else if (action == GLFW_RELEASE) engine->isDragging = false;       
        }
    }

    void GraphicsEngine::cursorPositionCallback(GLFWwindow* window, double xposIn, double yposIn) {
        GraphicsEngine* engine = static_cast<GraphicsEngine*>(glfwGetWindowUserPointer(window));

        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (engine->isDragging) {
            
            float xoffset = engine->lastX - xpos; 
            float yoffset = ypos - engine->lastY; 

            engine->camera.processMouseMovement(xoffset, yoffset);
        }
        
        engine->lastX = xpos;
        engine->lastY = ypos;
    }

    void GraphicsEngine::scrollCallback(GLFWwindow* window, double xoffsetIn, double yoffsetIn) {
        GraphicsEngine* engine = static_cast<GraphicsEngine*>(glfwGetWindowUserPointer(window));

        float yoffset = static_cast<float>(yoffsetIn);
        engine->camera.processMouseScroll(yoffset);
    }
}
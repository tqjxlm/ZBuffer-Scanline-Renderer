#pragma once

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include "ResourceManager.h"
#include "Camera.h"

class ZBufferScanLine;
class Shader;

class MainWindow {
public:

    MainWindow();

    ~MainWindow();

    // Pipeline
    void        init();

    void        gameLoop();

    // Init process
    void        initOpenGL();

    void        initPixelBuffer();

    void        loadResources();

    // Render process
    void        processScene();

    void        renderScene(GLubyte* buffer);

    void        drawToPBO();

    void        drawToScreen();

    void        renderQuad();

    // Input
    void        doMovement();

    static void keyboardEvent(GLFWwindow* window,
                              int         key,
                              int         scancode,
                              int         action,
                              int         mode);

    static void cursorMoveEvent(GLFWwindow* window,
                                double      xpos,
                                double      ypos);

    static void scrollCallEvent(GLFWwindow* window,
                                double      xoffset,
                                double      yoffset);

    static void mouseButtonEvent(GLFWwindow* window,
                                 int         button,
                                 int         action,
                                 int         mode);

    void setMode(int mode)
    {
        showModel_ = mode;
    }

    // Error handle
    void catchGLError(std::string info = "")
    {
        GLenum err;

        if ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "[" << info << "]" << "GL ERROR: " << err << std::endl;
        }
    }

private:

    static MainWindow* instance_;

    // Control
    Camera camera_;
    static bool keys_[1024];
    static double lastX_;
    static double lastY_;
    static bool firstMouse_;
    static bool leftPushed_;
    static bool rightPushed_;
    static bool isRendering_;
    static int showModel_;

    // Frame
    static float deltaTime_;
    static float lastFrame_;
    static float currentFrame_;
    static float timerFrame_;

    // OpenGL
    GLFWwindow* window_;
    Shader* screenShader_;
    GLuint screenTexture_;
    GLuint PBOs_[2];
    GLubyte* textureImages_[2];

    // Custom pipeline
    ZBufferScanLine* scanLine_;
    ResourceManager resourceManager_;
    std::vector<DrawableObject *>drawableObjects_;

    // Global settings
    int samples_       = 2;
    int windowWidth_   = 1024;
    int windowHeight_  = 768;
    int textureWidth_  = windowWidth_ * samples_;
    int textureHeight_ = windowHeight_ * samples_;
    float nearPlane_   = 0.1f;
    float farPlane_    = 100.0f;
    int bufferSize_    = textureWidth_ * textureHeight_ * 4;

    // Global matrices
    glm::mat4 viewMatrix_;
    glm::mat4 projectionMatrix_;
};

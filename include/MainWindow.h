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

    void        initFrameBuffer();

    void        initTexture();

    void        loadResources();

    // Render process
    void        processScene();

    void        renderScene(GLubyte *buffer);

    void        drawToPBO();

    void        drawToScreen();

    void        renderQuad();

    // Input
    void        doMovement();

    static void keyboardEvent(GLFWwindow *window,
                              int         key,
                              int         scancode,
                              int         action,
                              int         mode);

    static void cursorMoveEvent(GLFWwindow *window,
                                double      xpos,
                                double      ypos);

    static void scrollCallEvent(GLFWwindow *window,
                                double      xoffset,
                                double      yoffset);

    static void mouseButtonEvent(GLFWwindow *window,
                                 int         button,
                                 int         action,
                                 int         mode);

    // Resources management
    void loadDrawableObjects();

    void setMode(int mode)
    {
        showModel = mode;
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

    static MainWindow *instance;

    // Control
    Camera _camera;
    static bool keys[1024];
    static double lastX;
    static double lastY;
    static bool firstMouse;
    static bool leftPushed;
    static bool rightPushed;
    static bool isRendering;
    static int showModel;

    // Frame
    static float deltaTime;
    static float lastFrame;
    static float currentFrame;
    static float timerFrame;

    // OpenGL
    GLFWwindow *_window;
    Shader *_screenShader;
    GLuint _screenTexture;
    GLuint _pbo[2];
    GLubyte *_textureImage[2];

    // Custom pipeline
    ZBufferScanLine *_scanLine;
    ResourceManager _resourceManager;
    std::vector<DrawableObject *>_drawableObjects;

    // Global settings
    int _samples       = 2;
    int _windowWidth   = 1024;
    int _windowHeight  = 768;
    int _textureWidth  = _windowWidth * _samples;
    int _textureHeight = _windowHeight * _samples;
    float _nearPlane   = 0.1f;
    float _farPlane    = 100.0f;
    int _bufferSize    = _textureWidth * _textureHeight * 4;

    // Global matrices
    glm::mat4 _viewMatrix;
    glm::mat4 _projectionMatrix;
};

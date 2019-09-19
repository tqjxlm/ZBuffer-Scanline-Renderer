#include "MainWindow.h"

#include <glm/gtc/matrix_transform.hpp>

#include <thread> // std::this_thread::sleep_for
#include <chrono> // std::chrono::seconds

#include "ZBufferScanLine.h"
#include "Shader.h"

MainWindow *MainWindow::instance = NULL;
bool   MainWindow::keys[1024];
double MainWindow::lastX        = 400;
double MainWindow::lastY        = 300;
bool   MainWindow::firstMouse   = true;
bool   MainWindow::leftPushed   = false;
bool   MainWindow::rightPushed  = false;
bool   MainWindow::isRendering  = true;
float  MainWindow::deltaTime    = 0.0;
float  MainWindow::currentFrame = 0.0;
float  MainWindow::lastFrame    = 0.0;
float  MainWindow::timerFrame;
int    MainWindow::showModel = 0;

MainWindow::MainWindow() :
    _camera(90.0f, 0.0f, 50.0f)
{
    instance         = this;
    _scanLine        = new ZBufferScanLine(_textureWidth, _textureHeight, _nearPlane, _farPlane);
    _textureImage[0] = new GLubyte[_bufferSize];
    _textureImage[1] = new GLubyte[_bufferSize];
}

MainWindow::~MainWindow()
{
    delete _scanLine;
    delete _screenShader;
    delete[] _textureImage[0];
    delete[] _textureImage[1];

    // clean up texture
    glDeleteTextures(1, &_screenTexture);

    // clean up PBOs
    glDeleteBuffersARB(2, _pbo);
}

void MainWindow::init()
{
    initOpenGL();
    loadResources();
    initPixelBuffer();
    initTexture();
}

void MainWindow::gameLoop()
{
    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();

        if (!isRendering)
        {
            // std::this_thread::sleep_for(std::chrono::milliseconds(33));
            continue;
        }

        // Set frame time
        currentFrame = static_cast<float>(glfwGetTime());
        deltaTime    = currentFrame - lastFrame;
        lastFrame    = currentFrame;

        // Check events and update view
        doMovement();
        _viewMatrix = _camera.GetViewMatrix();

        // Main rendering
        drawToPBO();

        // Draw texture to screen
        drawToScreen();

        glfwSwapBuffers(_window);

        // catchGLError("Main Loop");

        // Print FPS
        static float tick  = currentFrame;
        static int   count = 0;

        if (currentFrame - tick > 0.5f)
        {
            tick = currentFrame;
            cout << "\r"
                 << "Polygons: " << _scanLine->getNumPolygon() << "\t"
                 << "FPS: " << count * 2;
            count = 0;
        }

        count++;
    }
}

void MainWindow::drawToPBO()
{
    static int index = 0;
    int nextIndex    = 0; // pbo index used for next frame

    index     = (index + 1) % 2;
    nextIndex = (index + 1) % 2;

    // bind the texture and PBO
    glBindTexture(GL_TEXTURE_2D, _screenTexture);

    // first PBO -> texture
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _pbo[index]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _textureWidth, _textureHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);

    // CPU -> second PBO
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _pbo[nextIndex]);
    processScene();
    renderScene(_textureImage[nextIndex]);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, _bufferSize, _textureImage[nextIndex], GL_STREAM_DRAW_ARB);
}

void MainWindow::keyboardEvent(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (action == GLFW_PRESS)
    {
        keys[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keys[key] = false;
    }

    if ((key == GLFW_KEY_SPACE) && (action == GLFW_PRESS))
    {
        isRendering = !isRendering;
    }
}

void MainWindow::cursorMoveEvent(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX      = xpos;
        lastY      = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    if ((instance->_camera.getMode() == Camera::WALK_THROUGH)
        || ((instance->_camera.getMode() == Camera::TRACK_BALL) && leftPushed))
    {
        instance->_camera.ProcessMouseMovement(static_cast<GLfloat>(xoffset), static_cast<GLfloat>(yoffset));
    }
}

void MainWindow::scrollCallEvent(GLFWwindow *window, double xoffset, double yoffset)
{
    instance->_camera.ProcessMouseScroll(static_cast<GLfloat>(yoffset));
}

void MainWindow::mouseButtonEvent(GLFWwindow *window, int button, int action, int mode)
{
    switch (button)
    {
    case (GLFW_MOUSE_BUTTON_LEFT):

        if (action == GLFW_RELEASE)
        {
            leftPushed = false;
        }

        if (action == GLFW_PRESS)
        {
            leftPushed = true;
        }

    case (GLFW_MOUSE_BUTTON_RIGHT):

        if (action == GLFW_RELEASE)
        {
            rightPushed = false;
        }

        if (action == GLFW_PRESS)
        {
            rightPushed = true;
        }

    default:
        break;
    }
}

void MainWindow::initOpenGL()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(       GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(            GLFW_RESIZABLE, GL_FALSE);

    _window = glfwCreateWindow(_windowWidth, _windowHeight, "Z-buffer Scanline by tqjxlm", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(_window);

    // Set the required callback functions
    glfwSetKeyCallback(_window, keyboardEvent);
    glfwSetCursorPosCallback(_window, cursorMoveEvent);
    glfwSetScrollCallback(_window, scrollCallEvent);
    glfwSetMouseButtonCallback(_window, mouseButtonEvent);

    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();

    // OpenGL options
    glDisable(GL_LIGHTING);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glDisable(GL_DEPTH_TEST);

    glViewport(0, 0, _windowWidth, _windowHeight);
    glClearColor(0, 0, 0, 0);

    // Init global matrices

    _viewMatrix       = _camera.GetViewMatrix();
    _projectionMatrix = glm::perspective(glm::radians(45.0f),
                                         (float)_textureWidth / (float)_textureHeight,
                                         _nearPlane,
                                         _farPlane);
}

void MainWindow::initPixelBuffer()
{
    glGenBuffersARB(2, _pbo);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _pbo[1]);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, _bufferSize, 0, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _pbo[2]);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, _bufferSize, 0, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    _screenShader = new Shader("resources/shaders/screenShader.vert", "resources/shaders/screenShader.frag");
}

void MainWindow::initFrameBuffer()
{}

void MainWindow::loadResources()
{
    loadDrawableObjects();
}

void MainWindow::processScene()
{
    _scanLine->reset();

    _scanLine->setViewDir(_camera.getFront());

    for (DrawableObject *object: _drawableObjects)
    {
        // Set mvp matrix for this model
        _scanLine->setMVP(_projectionMatrix * _viewMatrix * object->modelMatrix);

        // Insert polygon into scanline pipeline
        for (auto geometry: object->geometries)
        {
            for (auto face: geometry->faces)
            {
                _scanLine->insertPolygon(face, geometry, object->useTexture);
            }
        }
    }
}

void MainWindow::renderScene(GLubyte *buffer)
{
    _scanLine->draw(buffer);
}

// Shader version
void MainWindow::drawToScreen()
{
    // Draw a quad on screen using rendered texture
    _screenShader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _screenTexture);

    renderQuad();
}

void MainWindow::renderQuad()
{
    static GLuint quadVAO;

    if (quadVAO == 0)
    {
        GLuint quadVBO;
        float  quadVertices[] = {
            // positions        // texture Coords
            -1.0f, 1.0f,   0.0f,   0.0f,   1.0f,
            -1.0f, -1.0f,  0.0f,   0.0f,   0.0f,
            1.0f,  1.0f,   0.0f,   1.0f,   1.0f,
            1.0f,  -1.0f,  0.0f,   1.0f,   0.0f,
        };

        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void MainWindow::loadDrawableObjects()
{
    glm::mat4 model(1.0);
    DrawableObject *resource;

    // Load models
    switch (showModel)
    {
    case (0):
        _drawableObjects.push_back(_resourceManager.loadCube());
        break;

    case (1):
        model    = glm::translate(model, glm::vec3(0.0, -1.0, 0.0));
        model    = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
        resource = _resourceManager.loadModel("resources/models/p21/p21.obj", model);

        if (resource == NULL)
        {
            return;
        }

        _drawableObjects.push_back(resource);
        break;

    case (2):
        model    = glm::translate(model, glm::vec3(0.0, -0.2, 0.0));
        model    = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
        resource = _resourceManager.loadModel("resources/models/house_obj/house_obj.obj", model);

        if (resource == NULL)
        {
            return;
        }

        _drawableObjects.push_back(resource);
        break;

    case (3):
        model    = glm::translate(model, glm::vec3(0, -1, 0));
        resource = _resourceManager.loadModel("resources/models/T-90/T-90.obj", model);

        if (resource == NULL)
        {
            return;
        }

        _drawableObjects.push_back(resource);
        break;

    case (4):
        model    = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
        model    = glm::translate(model, glm::vec3(0, -7, 0));
        resource = _resourceManager.loadModel("resources/models/nanosuit_reflection/nanosuit.obj", model);

        if (resource == NULL)
        {
            return;
        }

        _drawableObjects.push_back(resource);
        break;

    default:
        break;
    }
}

void MainWindow::initTexture()
{
    glGenTextures(1, &_screenTexture);
    glBindTexture(GL_TEXTURE_2D, _screenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,      GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,      GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _textureWidth, _textureHeight, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void MainWindow::doMovement()
{
    if (_camera.getMode() == Camera::WALK_THROUGH)
    {
        // Camera controls
        if (keys[GLFW_KEY_W])
        {
            _camera.ProcessKeyboard(Camera::FORWARD, deltaTime);
        }

        if (keys[GLFW_KEY_S])
        {
            _camera.ProcessKeyboard(Camera::BACKWARD, deltaTime);
        }

        if (keys[GLFW_KEY_A])
        {
            _camera.ProcessKeyboard(Camera::LEFT, deltaTime);
        }

        if (keys[GLFW_KEY_D])
        {
            _camera.ProcessKeyboard(Camera::RIGHT, deltaTime);
        }
    }
}

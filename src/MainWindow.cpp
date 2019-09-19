#include "MainWindow.h"

#include <glm/gtc/matrix_transform.hpp>

#include <thread> // std::this_thread::sleep_for
#include <chrono> // std::chrono::seconds
using namespace std;

#include "ZBufferScanLine.h"
#include "Shader.h"

MainWindow * MainWindow::instance_ = nullptr;
bool   MainWindow::keys_[1024];
double MainWindow::lastX_        = 400;
double MainWindow::lastY_        = 300;
bool   MainWindow::firstMouse_   = true;
bool   MainWindow::leftPushed_   = false;
bool   MainWindow::rightPushed_  = false;
bool   MainWindow::isRendering_  = true;
float  MainWindow::deltaTime_    = 0.0;
float  MainWindow::currentFrame_ = 0.0;
float  MainWindow::lastFrame_    = 0.0;
float  MainWindow::timerFrame_;
int    MainWindow::showModel_ = 0;

MainWindow::MainWindow() :
    camera_(90.0f, 0.0f, 50.0f)
{
    instance_         = this;
    scanLine_         = new ZBufferScanLine(textureWidth_, textureHeight_, nearPlane_, farPlane_);
    textureImages_[0] = new GLubyte[bufferSize_];
    textureImages_[1] = new GLubyte[bufferSize_];
}

MainWindow::~MainWindow()
{
    delete scanLine_;
    delete screenShader_;
    delete[] textureImages_[0];
    delete[] textureImages_[1];

    // clean up texture
    glDeleteTextures(1, &screenTexture_);

    // clean up PBOs
    glDeleteBuffersARB(2, PBOs_);
}

void MainWindow::init()
{
    initOpenGL();
    loadResources();
    initPixelBuffer();
}

void MainWindow::gameLoop()
{
    while (!glfwWindowShouldClose(window_))
    {
        glfwPollEvents();

        if (!isRendering_)
        {
            // std::this_thread::sleep_for(std::chrono::milliseconds(33));
            continue;
        }

        // Set frame time
        currentFrame_ = static_cast<float>(glfwGetTime());
        deltaTime_    = currentFrame_ - lastFrame_;
        lastFrame_    = currentFrame_;

        // Check events and update view
        doMovement();
        viewMatrix_ = camera_.getViewMatrix();

        // Main rendering
        drawToPBO();

        // Draw texture to screen
        drawToScreen();

        glfwSwapBuffers(window_);

        // catchGLError("Main Loop");

        // Print FPS
        static float tick  = currentFrame_;
        static int   count = 0;

        if (currentFrame_ - tick > 0.5f)
        {
            tick = currentFrame_;
            cout << "\r"
                 << "Polygons: " << scanLine_->getNumPolygon() << "\t"
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
    glBindTexture(GL_TEXTURE_2D, screenTexture_);

    // first PBO -> texture
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, PBOs_[index]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth_, textureHeight_, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);

    // CPU -> second PBO
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, PBOs_[nextIndex]);
    processScene();
    renderScene(textureImages_[nextIndex]);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, bufferSize_, textureImages_[nextIndex], GL_STREAM_DRAW_ARB);
}

void MainWindow::keyboardEvent(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (action == GLFW_PRESS)
    {
        keys_[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keys_[key] = false;
    }

    if ((key == GLFW_KEY_SPACE) && (action == GLFW_PRESS))
    {
        isRendering_ = !isRendering_;
    }
}

void MainWindow::cursorMoveEvent(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse_)
    {
        lastX_      = xpos;
        lastY_      = ypos;
        firstMouse_ = false;
    }

    double xoffset = xpos - lastX_;
    double yoffset = lastY_ - ypos;

    lastX_ = xpos;
    lastY_ = ypos;

    if ((instance_->camera_.getMode() == Camera::WALK_THROUGH)
        || ((instance_->camera_.getMode() == Camera::TRACK_BALL) && leftPushed_))
    {
        instance_->camera_.processMouseMovement(static_cast<GLfloat>(xoffset), static_cast<GLfloat>(yoffset));
    }
}

void MainWindow::scrollCallEvent(GLFWwindow* window, double xoffset, double yoffset)
{
    instance_->camera_.processMouseScroll(static_cast<GLfloat>(yoffset));
}

void MainWindow::mouseButtonEvent(GLFWwindow* window, int button, int action, int mode)
{
    switch (button)
    {
    case (GLFW_MOUSE_BUTTON_LEFT):

        if (action == GLFW_RELEASE)
        {
            leftPushed_ = false;
        }

        if (action == GLFW_PRESS)
        {
            leftPushed_ = true;
        }

    case (GLFW_MOUSE_BUTTON_RIGHT):

        if (action == GLFW_RELEASE)
        {
            rightPushed_ = false;
        }

        if (action == GLFW_PRESS)
        {
            rightPushed_ = true;
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

    window_ = glfwCreateWindow(windowWidth_, windowHeight_, "Z-buffer Scanline by tqjxlm", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(window_);

    // Set the required callback functions
    glfwSetKeyCallback(window_, keyboardEvent);
    glfwSetCursorPosCallback(window_, cursorMoveEvent);
    glfwSetScrollCallback(window_, scrollCallEvent);
    glfwSetMouseButtonCallback(window_, mouseButtonEvent);

    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();

    // OpenGL options
    glDisable(GL_LIGHTING);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glDisable(GL_DEPTH_TEST);

    glViewport(0, 0, windowWidth_, windowHeight_);
    glClearColor(0, 0, 0, 0);

    // Init global matrices
    viewMatrix_       = camera_.getViewMatrix();
    projectionMatrix_ = glm::perspective(glm::radians(45.0f),
                                         (float)textureWidth_ / (float)textureHeight_,
                                         nearPlane_,
                                         farPlane_);
}

void MainWindow::initPixelBuffer()
{
    // PBO
    glGenBuffersARB(2, PBOs_);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, PBOs_[1]);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, bufferSize_, 0, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, PBOs_[2]);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, bufferSize_, 0, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    // The texture combined with PBO
    glGenTextures(1, &screenTexture_);
    glBindTexture(GL_TEXTURE_2D, screenTexture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,      GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,      GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth_, textureHeight_, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // A simple shader for drawing texture to screen
    screenShader_ = new Shader("resources/shaders/screenShader.vert", "resources/shaders/screenShader.frag");
}

void MainWindow::processScene()
{
    scanLine_->reset();

    scanLine_->setViewDir(camera_.getFront());

    for (DrawableObject* object: drawableObjects_)
    {
        // Set mvp matrix for this model
        scanLine_->setMVP(projectionMatrix_ * viewMatrix_ * object->modelMatrix);

        // Insert polygon into scanline pipeline
        for (auto geometry: object->geometries)
        {
            for (auto face: geometry->faces)
            {
                scanLine_->insertPolygon(face, geometry, object->useTexture);
            }
        }
    }
}

void MainWindow::renderScene(GLubyte* buffer)
{
    scanLine_->draw(buffer);
}

// Shader version
void MainWindow::drawToScreen()
{
    // Draw a quad on screen using rendered texture
    screenShader_->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture_);

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

void MainWindow::loadResources()
{
    glm::mat4 model(1.0);
    DrawableObject* resource;

    // Load models
    switch (showModel_)
    {
    case (0):
        drawableObjects_.push_back(resourceManager_.loadCube());
        break;

    case (1):
        model    = glm::translate(model, glm::vec3(0.0, -1.0, 0.0));
        model    = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
        resource = resourceManager_.loadModel("resources/models/p21/p21.obj", model);

        if (resource == nullptr)
        {
            return;
        }

        drawableObjects_.push_back(resource);
        break;

    case (2):
        model    = glm::translate(model, glm::vec3(0.0, -0.2, 0.0));
        model    = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
        resource = resourceManager_.loadModel("resources/models/house_obj/house_obj.obj", model);

        if (resource == nullptr)
        {
            return;
        }

        drawableObjects_.push_back(resource);
        break;

    case (3):
        model    = glm::translate(model, glm::vec3(0, -1, 0));
        resource = resourceManager_.loadModel("resources/models/T-90/T-90.obj", model);

        if (resource == nullptr)
        {
            return;
        }

        drawableObjects_.push_back(resource);
        break;

    case (4):
        model    = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
        model    = glm::translate(model, glm::vec3(0, -7, 0));
        resource = resourceManager_.loadModel("resources/models/nanosuit_reflection/nanosuit.obj", model);

        if (resource == nullptr)
        {
            return;
        }

        drawableObjects_.push_back(resource);
        break;

    default:
        break;
    }
}

void MainWindow::doMovement()
{
    if (camera_.getMode() == Camera::WALK_THROUGH)
    {
        // Camera controls
        if (keys_[GLFW_KEY_W])
        {
            camera_.processKeyboard(Camera::FORWARD, deltaTime_);
        }

        if (keys_[GLFW_KEY_S])
        {
            camera_.processKeyboard(Camera::BACKWARD, deltaTime_);
        }

        if (keys_[GLFW_KEY_A])
        {
            camera_.processKeyboard(Camera::LEFT, deltaTime_);
        }

        if (keys_[GLFW_KEY_D])
        {
            camera_.processKeyboard(Camera::RIGHT, deltaTime_);
        }
    }
}

#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// An abstract camera class that processes input and calculates the corresponding Euler Angles,
// Vectors and Matrices for use in OpenGL
class Camera {
public:

    // Defines several possible options for camera movement.
    // Used as abstraction to stay away from window-system specific input methods
    enum CameraMovement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    enum CameraMode
    {
        WALK_THROUGH,
        TRACK_BALL
    };

public:

    // Default: walk through
    Camera(glm::vec3 position);

    Camera(glm::vec3 position,
           GLfloat   yaw,
           GLfloat   pitch,
           glm::vec3 up
           );

    Camera(GLfloat posX,
           GLfloat posY,
           GLfloat posZ,
           GLfloat upX,
           GLfloat upY,
           GLfloat upZ,
           GLfloat yaw,
           GLfloat pitch);

    // Default: track ball
    Camera(GLfloat angleH,
           GLfloat angleV,
           GLfloat distance = 10.0f,
           glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up     = glm::vec3(0.0f, 1.0f, 0.0f)
           );

    void setPosition(const glm::vec3& position)
    {
        this->position_ = position;
        this->updateCameraVectors();
    }

    void setPosition(GLfloat posX, GLfloat posY, GLfloat posZ)
    {
        this->position_ = glm::vec3(posX, posY, posZ);
        this->updateCameraVectors();
    }

    void setUp(const glm::vec3& up)
    {
        this->worldUp_ = up;
        this->updateCameraVectors();
    }

    void setUp(GLfloat upX, GLfloat upY, GLfloat upZ)
    {
        this->worldUp_ = glm::vec3(upX, upY, upZ);
        this->updateCameraVectors();
    }

    void setYaw(GLfloat yaw)
    {
        this->yaw_ = yaw;
        this->updateCameraVectors();
    }

    void setPitch(GLfloat pitch)
    {
        this->pitch_ = pitch;
        this->updateCameraVectors();
    }

    void setMode(CameraMode mode)
    {
        this->mode_ = mode;
        this->updateCameraVectors();
    }

    const glm::vec3& getFront()
    {
        return front_;
    }

    CameraMode getMode()
    {
        return this->mode_;
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(this->position_, this->position_ + this->front_, this->up_);
    }

public:

    // Processes input received from any keyboard-like input system.
    // Accepts input parameter in the form of camera_ defined ENUM (to abstract it from windowing systems)
    void processKeyboard(CameraMovement direction,
                         GLfloat        deltaTime);

    // Processes input received from a mouse input system.
    // Expects the offset value in both the x and y direction.
    void processMouseMovement(GLfloat   xoffset,
                              GLfloat   yoffset,
                              GLboolean constrainPitch = true);

    // Processes input received from a mouse scroll-wheel event.
    // Only requires input on the vertical wheel-axis
    void processMouseScroll(GLfloat yoffset);

private:

    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();

private:

    CameraMode mode_ = TRACK_BALL;

    // Camera Attributes
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;
    glm::vec3 center_;

    // Euler Angles
    GLfloat yaw_;
    GLfloat pitch_;
    GLfloat angleH_; // Horizontal angle from positive x
    GLfloat angleV_; // Vertical angle from xz plane

    // Camera options
    GLfloat movementSpeed_;
    GLfloat mouseSensitivity_;
    GLfloat zoom_;
    GLfloat distance_;
};

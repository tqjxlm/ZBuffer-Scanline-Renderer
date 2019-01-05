#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for _camera movement.
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

// Default _camera values
const GLfloat  YAW        = -90.0f;
const GLfloat  PITCH      = 0.0f;
const GLfloat  SPEED      = 3.0f;
const GLfloat  SENSITIVTY = 0.25f;
const GLfloat  ZOOM       = 0.0f;


// An abstract _camera class that processes input and calculates the corresponding Eular Angles,
// Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Default: walk through
    Camera(glm::vec3 position, GLfloat yaw = YAW, GLfloat pitch = PITCH,
           glm::vec3 up                    = glm::vec3(0.0f, 1.0f, 0.0f)
           );

    Camera(GLfloat posX, GLfloat posY, GLfloat posZ,
           GLfloat upX, GLfloat upY, GLfloat upZ,
           GLfloat yaw, GLfloat pitch);

    // Default: track ball
    Camera(GLfloat angleH, GLfloat angleV, GLfloat distance = 10.0f,
           glm::vec3 center                                 = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up                                     = glm::vec3(0.0f, 1.0f, 0.0f)
           );

    void  setPosition(const glm::vec3 &position)
    {
        this->Position = position;
        this->updateCameraVectors();
    }

    void  setPosition(GLfloat posX, GLfloat posY, GLfloat posZ)
    {
        this->Position = glm::vec3(posX, posY, posZ);
        this->updateCameraVectors();
    }

    void  setUp(const glm::vec3 &up)
    {
        this->WorldUp = up;
        this->updateCameraVectors();
    }

    void  setUp(GLfloat upX, GLfloat upY, GLfloat upZ)
    {
        this->WorldUp = glm::vec3(upX, upY, upZ);
        this->updateCameraVectors();
    }

    void  setYaw(GLfloat yaw)
    {
        this->Yaw = yaw;
        this->updateCameraVectors();
    }

    void  setPitch(GLfloat pitch)
    {
        this->Pitch = pitch;
        this->updateCameraVectors();
    }

    void  setMode(CameraMode mode)
    {
        this->Mode = mode;
        this->updateCameraVectors();
    }

    const glm::vec3 & getFront()
    {
        return Front;
    }

    CameraMode  getMode()
    {
        return this->Mode;
    }

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    glm::mat4  GetViewMatrix();

    // Processes input received from any keyboard-like input system.
    // Accepts input parameter in the form of _camera defined ENUM (to abstract it from windowing systems)
    void       ProcessKeyboard(CameraMovement direction, GLfloat deltaTime);

    // Processes input received from a mouse input system.
    // Expects the offset value in both the x and y direction.
    void       ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true);

    // Processes input received from a mouse scroll-wheel event.
    // Only requires input on the vertical wheel-axis
    void       ProcessMouseScroll(GLfloat yoffset);

private:
    // Calculates the front vector from the Camera's (updated) Eular Angles
    void       updateCameraVectors();

    CameraMode  Mode = TRACK_BALL;

    // Camera Attributes
    glm::vec3  Position;
    glm::vec3  Front;
    glm::vec3  Up;
    glm::vec3  Right;
    glm::vec3  WorldUp;
    glm::vec3  Center;

    // Eular Angles
    GLfloat  Yaw;
    GLfloat  Pitch;
    GLfloat  AngleH; // Horizontal angle from positive x
    GLfloat  AngleV; // Vertical angle from xz plane

    // Camera options
    GLfloat  MovementSpeed;
    GLfloat  MouseSensitivity;
    GLfloat  Zoom;
    GLfloat  Distance;
};

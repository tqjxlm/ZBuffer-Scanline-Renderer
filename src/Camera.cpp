#include "Camera.h"

// Default camera_ values
static const GLfloat YAW         = -90.0f;
static const GLfloat PITCH       = 0.0f;
static const GLfloat SPEED       = 3.0f;
static const GLfloat SENSITIVITY = 0.25f;
static const GLfloat ZOOM        = 0.0f;

// Constructor with vectors
Camera::Camera(glm::vec3 position,
               GLfloat   yaw,
               GLfloat   pitch,
               glm::vec3 up
               ) :
    mode_(WALK_THROUGH),
    position_(position),
    worldUp_(up),
    yaw_(yaw),
    pitch_(pitch),
    front_(glm::vec3(0.0f, 0.0f, -1.0f)),
    movementSpeed_(SPEED),
    mouseSensitivity_(SENSITIVITY),
    zoom_(ZOOM)
{
    this->updateCameraVectors();
}

Camera::Camera(glm::vec3 position) :
    Camera(position, YAW, PITCH, glm::vec3(0.0f, 1.0f, 0.0f))
{}

// Constructor with scalar values
Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ,
               GLfloat upX, GLfloat upY, GLfloat upZ,
               GLfloat yaw, GLfloat pitch) :
    mode_(WALK_THROUGH),
    front_(glm::vec3(0.0f, 0.0f, -1.0f)),
    movementSpeed_(SPEED),
    mouseSensitivity_(SENSITIVITY),
    zoom_(ZOOM)
{
    this->position_ = glm::vec3(posX, posY, posZ);
    this->worldUp_  = glm::vec3(upX, upY, upZ);
    this->yaw_      = yaw;
    this->pitch_    = pitch;
    this->updateCameraVectors();
}

Camera::Camera(GLfloat   angleH,
               GLfloat   angleV,
               GLfloat   distance,
               glm::vec3 center,
               glm::vec3 up) :
    mode_(TRACK_BALL),
    angleH_(angleH),
    angleV_(angleV),
    worldUp_(up),
    center_(center),
    distance_(distance),
    front_(glm::vec3(0.0f, 0.0f, -1.0f)),
    movementSpeed_(SPEED),
    mouseSensitivity_(SENSITIVITY),
    zoom_(ZOOM)
{
    this->updateCameraVectors();
}

// Processes input received from any keyboard-like input system.
// Accepts input parameter in the form of camera_ defined ENUM (to abstract it from windowing systems)
void Camera::processKeyboard(CameraMovement direction, GLfloat deltaTime)
{
    GLfloat velocity = this->movementSpeed_ * deltaTime;

    if (direction == FORWARD)
    {
        this->position_ += this->front_ * velocity;
    }

    if (direction == BACKWARD)
    {
        this->position_ -= this->front_ * velocity;
    }

    if (direction == LEFT)
    {
        this->position_ -= this->right_ * velocity;
    }

    if (direction == RIGHT)
    {
        this->position_ += this->right_ * velocity;
    }
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch /*= true*/)
{
    xoffset *= this->mouseSensitivity_;
    yoffset *= this->mouseSensitivity_;

    switch (mode_)
    {
    case (WALK_THROUGH):
        this->yaw_   += xoffset;
        this->pitch_ += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (this->pitch_ > 89.0f)
            {
                this->pitch_ = 89.0f;
            }

            if (this->pitch_ < -89.0f)
            {
                this->pitch_ = -89.0f;
            }
        }

        break;

    case (TRACK_BALL):
        angleH_ += xoffset;
        angleV_ -= yoffset;

        if (constrainPitch)
        {
            if (this->angleV_ > 89.0f)
            {
                this->angleV_ = 89.0f;
            }

            if (this->angleV_ < -89.0f)
            {
                this->angleV_ = -89.0f;
            }
        }

        break;
    }

    // Update front_, right_ and up_ Vectors using the updated Euler angles
    this->updateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::processMouseScroll(GLfloat yoffset)
{
    yoffset *= this->mouseSensitivity_;

    switch (mode_)
    {
    case (WALK_THROUGH):
        break;

    case (TRACK_BALL):
        this->zoom_ -= yoffset;
        break;
    }

    this->updateCameraVectors();
}

// Calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors()
{
    // Calculate the new front_ vector
    switch (mode_)
    {
    case (WALK_THROUGH):
    {
        glm::vec3 front;
        front.x      = cos(glm::radians(this->yaw_)) * cos(glm::radians(this->pitch_));
        front.y      = sin(glm::radians(this->pitch_));
        front.z      = sin(glm::radians(this->yaw_)) * cos(glm::radians(this->pitch_));
        this->front_ = glm::normalize(front);
        break;
    }

    case (TRACK_BALL):
    {
        GLfloat   distance = distance_ * pow(2.0f, zoom_ - 1);
        glm::vec3 positionVector;
        positionVector.y = distance * sin(glm::radians(angleV_));
        positionVector.x = distance * cos(glm::radians(angleV_)) * cos(glm::radians(angleH_));
        positionVector.z = distance * cos(glm::radians(angleV_)) * sin(glm::radians(angleH_));

        this->position_ = center_ + positionVector;
        this->front_    = -glm::normalize(positionVector);
        break;
    }
    }

    // Also re-calculate the right_ and up_ vector
    this->right_ = glm::normalize(glm::cross(this->front_, this->worldUp_));

    // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    this->up_ = glm::normalize(glm::cross(this->right_, this->front_));
}

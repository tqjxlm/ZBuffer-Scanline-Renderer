#include "Camera.h"

// Constructor with vectors
Camera::Camera(glm::vec3 position /*= glm::vec3(0.0f, 0.0f, 0.0f)*/,
               GLfloat   yaw /*= YAW*/,
               GLfloat   pitch /*= PITCH*/,
               glm::vec3 up /*= glm::vec3(0.0f, 1.0f, 0.0f)*/
               ):
    Mode(WALK_THROUGH),
    Position(position),
    WorldUp(up),
    Yaw(yaw),
    Pitch(pitch),
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVTY),
    Zoom(ZOOM)
{
    this->updateCameraVectors();
}

// Constructor with scalar values
Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ,
               GLfloat upX, GLfloat upY, GLfloat upZ,
               GLfloat yaw, GLfloat pitch):
    Mode(WALK_THROUGH),
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVTY),
    Zoom(ZOOM)
{
    this->Position = glm::vec3(posX, posY, posZ);
    this->WorldUp  = glm::vec3(upX, upY, upZ);
    this->Yaw      = yaw;
    this->Pitch    = pitch;
    this->updateCameraVectors();
}

Camera::Camera(GLfloat   angleH,
               GLfloat   angleV,
               GLfloat   distance,
               glm::vec3 center,
               glm::vec3 up):
    Mode(TRACK_BALL),
    AngleH(angleH),
    AngleV(angleV),
    WorldUp(up),
    Center(center),
    Distance(distance),
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVTY),
    Zoom(ZOOM)
{
    this->updateCameraVectors();
}

// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
glm::mat4  Camera::GetViewMatrix()
{
    return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
}

// Processes input received from any keyboard-like input system. Accepts input parameter in the form of _camera defined ENUM (to abstract it from windowing
// systems)
void  Camera::ProcessKeyboard(CameraMovement direction, GLfloat deltaTime)
{
    GLfloat  velocity = this->MovementSpeed * deltaTime;

    if (direction == FORWARD)
    {
        this->Position += this->Front * velocity;
    }

    if (direction == BACKWARD)
    {
        this->Position -= this->Front * velocity;
    }

    if (direction == LEFT)
    {
        this->Position -= this->Right * velocity;
    }

    if (direction == RIGHT)
    {
        this->Position += this->Right * velocity;
    }
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void  Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch /*= true*/)
{
    xoffset *= this->MouseSensitivity;
    yoffset *= this->MouseSensitivity;

    switch (Mode)
    {
    case (WALK_THROUGH):
        this->Yaw   += xoffset;
        this->Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (this->Pitch > 89.0f)
            {
                this->Pitch = 89.0f;
            }

            if (this->Pitch < -89.0f)
            {
                this->Pitch = -89.0f;
            }
        }

        break;
    case (TRACK_BALL):
        AngleH += xoffset;
        AngleV -= yoffset;

        if (constrainPitch)
        {
            if (this->AngleV > 89.0f)
            {
                this->AngleV = 89.0f;
            }

            if (this->AngleV < -89.0f)
            {
                this->AngleV = -89.0f;
            }
        }

        break;
    }

    // Update Front, Right and Up Vectors using the updated Eular angles
    this->updateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void  Camera::ProcessMouseScroll(GLfloat yoffset)
{
    yoffset *= this->MouseSensitivity;

    switch (Mode)
    {
    case (WALK_THROUGH):
        break;
    case (TRACK_BALL):
        this->Zoom -= yoffset;
        break;
    }

    this->updateCameraVectors();
}

// Calculates the front vector from the Camera's (updated) Eular Angles
void  Camera::updateCameraVectors()
{
    // Calculate the new Front vector
    switch (Mode)
    {
    case (WALK_THROUGH):
    {
        glm::vec3  front;
        front.x     = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y     = sin(glm::radians(this->Pitch));
        front.z     = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->Front = glm::normalize(front);
    }
    break;
    case (TRACK_BALL):
    {
        GLfloat    distance = Distance * pow(2, Zoom - 1);
        glm::vec3  positionVector;
        positionVector.y = distance * sin(glm::radians(AngleV));
        positionVector.x = distance * cos(glm::radians(AngleV)) * cos(glm::radians(AngleH));
        positionVector.z = distance * cos(glm::radians(AngleV)) * sin(glm::radians(AngleH));

        this->Position = Center + positionVector;
        this->Front    = -glm::normalize(positionVector);
    }
    break;
    }

    // Also re-calculate the Right and Up vector
    this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up
    // or down which results in slower movement.
    this->Up = glm::normalize(glm::cross(this->Right, this->Front));
}

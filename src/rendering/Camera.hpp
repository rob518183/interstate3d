#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    enum class Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };
    
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);
    
    // Camera attributes
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getFront() const { return m_front; }
    glm::vec3 getUp() const { return m_up; }
    glm::vec3 getRight() const { return m_right; }
    glm::vec3 getWorldUp() const { return m_worldUp; }
    
    // Camera options
    float getFOV() const { return m_fov; }
    float getAspectRatio() const { return m_aspectRatio; }
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }
    
    void setPosition(const glm::vec3& position);
    void setUp(const glm::vec3& up);
    void setFOV(float fov);
    void setAspectRatio(float aspectRatio);
    void setPlanes(float nearPlane, float farPlane);
    
    // Movement
    void processKeyboard(Movement direction, float deltaTime);
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void processMouseScroll(float yOffset);
    
    // Matrices
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    
    // Camera types
    void setCameraType(bool isThirdPerson);
    void setTarget(const glm::vec3& target);
    void setDistance(float distance);
    
private:
    void updateCameraVectors();
    
    // Camera attributes
    glm::vec3 m_position;
    glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;
    
    // Euler angles
    float m_yaw;
    float m_pitch;
    
    // Camera options
    float m_movementSpeed = 2.5f;
    float m_mouseSensitivity = 0.1f;
    float m_fov = 45.0f;
    float m_aspectRatio = 16.0f / 9.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;
    
    // Third-person camera
    bool m_isThirdPerson = false;
    glm::vec3 m_target;
    float m_distance = 10.0f;
};

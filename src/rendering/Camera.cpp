#include "rendering/Camera.hpp"
#include <cmath>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_position(position), m_worldUp(up), m_yaw(yaw), m_pitch(pitch) {
    updateCameraVectors();
}

void Camera::setPosition(const glm::vec3& position) {
    m_position = position;
}

void Camera::setUp(const glm::vec3& up) {
    m_worldUp = glm::normalize(up);
    updateCameraVectors();
}

void Camera::setFOV(float fov) {
    m_fov = glm::clamp(fov, 1.0f, 180.0f);
}

void Camera::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
}

void Camera::setPlanes(float nearPlane, float farPlane) {
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

void Camera::processKeyboard(Movement direction, float deltaTime) {
    float velocity = m_movementSpeed * deltaTime;
    
    if (direction == Movement::FORWARD) {
        m_position += m_front * velocity;
    }
    if (direction == Movement::BACKWARD) {
        m_position -= m_front * velocity;
    }
    if (direction == Movement::LEFT) {
        m_position -= m_right * velocity;
    }
    if (direction == Movement::RIGHT) {
        m_position += m_right * velocity;
    }
    if (direction == Movement::UP) {
        m_position += m_worldUp * velocity;
    }
    if (direction == Movement::DOWN) {
        m_position -= m_worldUp * velocity;
    }
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= m_mouseSensitivity;
    yOffset *= m_mouseSensitivity;
    
    m_yaw += xOffset;
    m_pitch += yOffset;
    
    // Constrain pitch
    if (constrainPitch) {
        m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
    }
    
    updateCameraVectors();
}

void Camera::processMouseScroll(float yOffset) {
    m_fov -= yOffset;
    m_fov = glm::clamp(m_fov, 1.0f, 180.0f);
}

glm::mat4 Camera::getViewMatrix() const {
    if (m_isThirdPerson) {
        // Third-person camera looking at target
        return glm::lookAt(m_position, m_target, m_worldUp);
    } else {
        // First-person camera
        return glm::lookAt(m_position, m_position + m_front, m_worldUp);
    }
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::setCameraType(bool isThirdPerson) {
    m_isThirdPerson = isThirdPerson;
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
}

void Camera::setDistance(float distance) {
    m_distance = glm::max(distance, 1.0f);
}

void Camera::updateCameraVectors() {
    // Calculate new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    
    // Recalculate right and up vectors
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

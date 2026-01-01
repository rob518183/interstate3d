#pragma once

#include <glm/glm.hpp>

class Light {
public:
    Light(const glm::vec3& position = glm::vec3(0.0f),
          const glm::vec3& color = glm::vec3(1.0f),
          float intensity = 1.0f);
    
    // Getters
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getColor() const { return m_color; }
    float getIntensity() const { return m_intensity; }
    
    // Setters
    void setPosition(const glm::vec3& position) { m_position = position; }
    void setColor(const glm::vec3& color) { m_color = color; }
    void setIntensity(float intensity) { m_intensity = intensity; }
    
    // Light types
    enum class Type {
        DIRECTIONAL,
        POINT,
        SPOT
    };
    
    Type getType() const { return m_type; }
    void setType(Type type) { m_type = type; }
    
    // Directional light properties
    void setDirection(const glm::vec3& direction);
    glm::vec3 getDirection() const;
    
    // Spot light properties
    void setSpotProperties(float cutOff, float outerCutOff);
    float getCutOff() const { return m_cutOff; }
    float getOuterCutOff() const { return m_outerCutOff; }
    
    // Attenuation (for point and spot lights)
    void setAttenuation(float constant, float linear, float quadratic);
    glm::vec3 getAttenuation() const { return glm::vec3(m_constant, m_linear, m_quadratic); }
    
private:
    glm::vec3 m_position;
    glm::vec3 m_color;
    float m_intensity;
    
    Type m_type = Type::POINT;
    
    // Directional light
    glm::vec3 m_direction = glm::vec3(0.0f, -1.0f, 0.0f);
    
    // Spot light
    float m_cutOff = glm::cos(glm::radians(12.5f));
    float m_outerCutOff = glm::cos(glm::radians(15.0f));
    
    // Attenuation
    float m_constant = 1.0f;
    float m_linear = 0.09f;
    float m_quadratic = 0.032f;
};

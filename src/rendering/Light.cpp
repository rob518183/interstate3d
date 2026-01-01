#include "rendering/Light.hpp"

Light::Light(const glm::vec3& position, const glm::vec3& color, float intensity)
    : m_position(position), m_color(color), m_intensity(intensity) {
}

void Light::setDirection(const glm::vec3& direction) {
    m_direction = glm::normalize(direction);
}

glm::vec3 Light::getDirection() const {
    return m_direction;
}

void Light::setSpotProperties(float cutOff, float outerCutOff) {
    m_cutOff = cutOff;
    m_outerCutOff = outerCutOff;
}

void Light::setAttenuation(float constant, float linear, float quadratic) {
    m_constant = constant;
    m_linear = linear;
    m_quadratic = quadratic;
}

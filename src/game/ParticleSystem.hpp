#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec4 color;
    float size;
    float lifetime;
    float maxLifetime;
    bool active;
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();
    
    bool initialize(const glm::vec3& position, int maxParticles, float particleLifetime);
    void update(float deltaTime);
    void render();
    
    // Emission
    void emit(int count);
    void setEmissionRate(float rate) { m_emissionRate = rate; }
    
    // Properties
    void setPosition(const glm::vec3& position) { m_position = position; }
    void setColor(const glm::vec4& color) { m_baseColor = color; }
    void setSize(float size) { m_baseSize = size; }
    
    // Presets
    void setExplosionProperties();
    void setTrailProperties();
    void setSmokeProperties();
    void setFireProperties();
    
    // Getters
    bool isComplete() const { return m_activeParticles == 0; }
    int getActiveParticleCount() const { return m_activeParticles; }
    
private:
    void updateParticle(Particle& particle, float deltaTime);
    void resetParticle(Particle& particle);
    
    std::vector<Particle> m_particles;
    glm::vec3 m_position;
    glm::vec3 m_direction = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Emission
    float m_emissionRate = 10.0f;
    float m_emissionAccumulator = 0.0f;
    int m_maxParticles = 100;
    int m_activeParticles = 0;
    
    // Particle properties
    glm::vec4 m_baseColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
    float m_baseSize = 0.1f;
    float m_particleLifetime = 1.0f;
    
    // Physics
    glm::vec3 m_gravity = glm::vec3(0.0f, -9.81f, 0.0f);
    float m_drag = 0.1f;
    
    // Variation
    float m_positionVariation = 1.0f;
    float m_velocityVariation = 5.0f;
    float m_sizeVariation = 0.5f;
    float m_lifetimeVariation = 0.2f;
    
    // Rendering
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    bool m_initialized = false;
};

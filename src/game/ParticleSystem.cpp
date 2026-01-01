#include "game/ParticleSystem.hpp"
#include "core/Logger.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <random>

ParticleSystem::ParticleSystem() {
    m_particles.reserve(m_maxParticles);
}

ParticleSystem::~ParticleSystem() {
    if (m_initialized) {
        // OpenGL cleanup - placeholder for now
        // glDeleteVertexArrays(1, &m_VAO);
        // glDeleteBuffers(1, &m_VBO);
    }
}

bool ParticleSystem::initialize(const glm::vec3& position, int maxParticles, float particleLifetime) {
    m_position = position;
    m_maxParticles = maxParticles;
    m_particleLifetime = particleLifetime;
    
    // Resize particle array
    m_particles.resize(maxParticles);
    
    // Initialize all particles as inactive
    for (auto& particle : m_particles) {
        particle.active = false;
    }
    
    // Setup OpenGL buffers for rendering - placeholder for now
    // glGenVertexArrays(1, &m_VAO);
    // glGenBuffers(1, &m_VBO);
    
    m_initialized = true;
    Logger::info("Particle system initialized with " + std::to_string(maxParticles) + " particles");
    return true;
}

void ParticleSystem::update(float deltaTime) {
    // Update emission
    if (m_emissionRate > 0.0f) {
        m_emissionAccumulator += m_emissionRate * deltaTime;
        int particlesToEmit = static_cast<int>(m_emissionAccumulator);
        m_emissionAccumulator -= particlesToEmit;
        
        if (particlesToEmit > 0) {
            emit(particlesToEmit);
        }
    }
    
    // Update active particles
    for (auto& particle : m_particles) {
        if (particle.active) {
            updateParticle(particle, deltaTime);
        }
    }
    
    // Count active particles
    m_activeParticles = std::count_if(m_particles.begin(), m_particles.end(),
        [](const Particle& p) { return p.active; });
}

void ParticleSystem::render() {
    if (!m_initialized || m_activeParticles == 0) {
        return;
    }
    
    // OpenGL rendering - placeholder for now
    // In a real implementation, this would update vertex buffers and render particles
    Logger::debug("Rendering " + std::to_string(m_activeParticles) + " particles");
}

void ParticleSystem::emit(int count) {
    int emitted = 0;
    
    for (auto& particle : m_particles) {
        if (!particle.active && emitted < count) {
            resetParticle(particle);
            particle.active = true;
            emitted++;
        }
    }
    
    if (emitted < count) {
        Logger::warn("Could only emit " + std::to_string(emitted) + " of " + 
                    std::to_string(count) + " requested particles");
    }
}

void ParticleSystem::setExplosionProperties() {
    m_emissionRate = 0.0f; // One-time burst
    m_direction = glm::vec3(0.0f, 1.0f, 0.0f);
    m_positionVariation = 0.5f;
    m_velocityVariation = 20.0f;
    m_sizeVariation = 0.3f;
    m_lifetimeVariation = 0.5f;
    m_baseColor = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f);
    m_baseSize = 0.2f;
    m_drag = 0.5f;
    
    // Emit initial burst
    emit(m_maxParticles);
}

void ParticleSystem::setTrailProperties() {
    m_emissionRate = 50.0f;
    m_direction = glm::vec3(0.0f, -1.0f, 0.0f);
    m_positionVariation = 0.1f;
    m_velocityVariation = 2.0f;
    m_sizeVariation = 0.1f;
    m_lifetimeVariation = 0.1f;
    m_baseColor = glm::vec4(0.8f, 0.6f, 0.4f, 0.8f);
    m_baseSize = 0.05f;
    m_drag = 0.2f;
}

void ParticleSystem::setSmokeProperties() {
    m_emissionRate = 20.0f;
    m_direction = glm::vec3(0.0f, 1.0f, 0.0f);
    m_positionVariation = 0.3f;
    m_velocityVariation = 3.0f;
    m_sizeVariation = 0.5f;
    m_lifetimeVariation = 1.0f;
    m_baseColor = glm::vec4(0.5f, 0.5f, 0.5f, 0.6f);
    m_baseSize = 0.3f;
    m_drag = 0.1f;
}

void ParticleSystem::setFireProperties() {
    m_emissionRate = 30.0f;
    m_direction = glm::vec3(0.0f, 1.0f, 0.0f);
    m_positionVariation = 0.2f;
    m_velocityVariation = 5.0f;
    m_sizeVariation = 0.2f;
    m_lifetimeVariation = 0.3f;
    m_baseColor = glm::vec4(1.0f, 0.3f, 0.0f, 0.9f);
    m_baseSize = 0.1f;
    m_drag = 0.05f;
}

void ParticleSystem::updateParticle(Particle& particle, float deltaTime) {
    // Update lifetime
    particle.lifetime -= deltaTime;
    if (particle.lifetime <= 0.0f) {
        particle.active = false;
        return;
    }
    
    // Update physics
    particle.velocity += particle.acceleration * deltaTime;
    particle.velocity *= (1.0f - m_drag * deltaTime); // Apply drag
    particle.position += particle.velocity * deltaTime;
    
    // Update color based on lifetime
    float lifeRatio = particle.lifetime / particle.maxLifetime;
    particle.color.a = m_baseColor.a * lifeRatio;
    
    // Update size based on lifetime
    particle.size = m_baseSize * (1.0f + (1.0f - lifeRatio) * 0.5f);
}

void ParticleSystem::resetParticle(Particle& particle) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);
    static std::uniform_real_distribution<float> velDist(-1.0f, 1.0f);
    static std::uniform_real_distribution<float> sizeDist(0.5f, 1.5f);
    static std::uniform_real_distribution<float> lifeDist(0.8f, 1.2f);
    
    // Position
    particle.position = m_position + glm::vec3(
        posDist(gen) * m_positionVariation,
        posDist(gen) * m_positionVariation,
        posDist(gen) * m_positionVariation
    );
    
    // Velocity
    glm::vec3 randomDirection = glm::normalize(m_direction + glm::vec3(
        velDist(gen) * 0.3f,
        velDist(gen) * 0.3f,
        velDist(gen) * 0.3f
    ));
    
    float speed = 5.0f + velDist(gen) * m_velocityVariation;
    particle.velocity = randomDirection * speed;
    
    // Acceleration (gravity)
    particle.acceleration = m_gravity;
    
    // Color
    particle.color = m_baseColor;
    
    // Size
    particle.size = m_baseSize * sizeDist(gen);
    
    // Lifetime
    particle.maxLifetime = m_particleLifetime * lifeDist(gen);
    particle.lifetime = particle.maxLifetime;
    
    particle.active = true;
}

#include "game/Projectile.hpp"
#include "game/Vehicle.hpp"
#include "game/ParticleSystem.hpp"
#include "core/Logger.hpp"
#include <glm/gtc/matrix_transform.hpp>

Projectile::Projectile(const glm::vec3& position, const glm::vec3& velocity, 
                     float damage, float range, Vehicle* owner)
    : m_position(position), m_velocity(velocity), m_damage(damage), 
      m_range(range), m_owner(owner), m_active(true), m_lifetime(0.0f),
      m_maxLifetime(range / glm::length(velocity)), m_distanceTraveled(0.0f) {
    
    // Create trail effect if enabled
    if (m_hasTrail) {
        createTrail();
    }
    
    Logger::debug("Projectile created with damage: " + std::to_string(damage));
}

Projectile::~Projectile() {
    m_trailEffect.reset();
}

void Projectile::update(float deltaTime) {
    if (!m_active) {
        return;
    }
    
    // Update position
    m_position += m_velocity * deltaTime;
    
    // Apply gravity if enabled
    if (m_hasGravity) {
        m_velocity.y -= 9.81f * deltaTime;
    }
    
    // Update lifetime
    m_lifetime += deltaTime;
    m_distanceTraveled += glm::length(m_velocity * deltaTime);
    
    // Check if projectile should be deactivated
    if (m_lifetime >= m_maxLifetime || m_distanceTraveled >= m_range) {
        m_active = false;
        return;
    }
    
    // Update trail effect
    if (m_hasTrail && m_trailEffect) {
        updateTrail(deltaTime);
    }
}

void Projectile::render() {
    if (!m_active) {
        return;
    }
    
    // Render projectile as a small sphere or line
    // This would use the renderer to draw the projectile
    // For now, we'll just log the position for debugging
    
    // Render trail effect
    if (m_trailEffect) {
        m_trailEffect->render();
    }
}

void Projectile::onCollision(const glm::vec3& hitPoint, const glm::vec3& hitNormal, Vehicle* hitVehicle) {
    if (!m_active) {
        return;
    }
    
    // Calculate impact angle
    glm::vec3 normalizedVelocity = glm::normalize(m_velocity);
    float impactAngle = glm::dot(normalizedVelocity, -hitNormal);
    
    // Apply damage to hit vehicle
    if (hitVehicle && hitVehicle != m_owner) {
        hitVehicle->applyDamage(m_damage);
        Logger::info("Projectile hit vehicle for " + std::to_string(m_damage) + " damage");
    }
    
    // Handle ricochet or penetration
    if (m_canPenetrate && impactAngle > 0.5f) {
        // Penetration: reduce velocity and damage, continue flying
        m_velocity *= 0.7f; // Reduce speed due to penetration
        m_damage *= 0.8f;   // Reduce damage due to energy loss
        m_maxLifetime *= 1.2f; // Allow longer range after penetration
        
        Logger::debug("Projectile penetrated surface, continuing with reduced damage");
        return; // Continue flying
    } else {
        // Ricochet: reflect velocity off surface
        glm::vec3 reflection = glm::reflect(m_velocity, hitNormal);
        m_velocity = reflection * 0.85f; // Energy loss on ricochet
        
        // Adjust lifetime for ricochet
        m_maxLifetime *= 0.8f;
        
        Logger::debug("Projectile ricocheted off surface");
    }
    
    // Create explosion effect
    createExplosion(hitPoint);
    
    // For ricochet, don't deactivate immediately - allow bouncing
    if (!m_canPenetrate || impactAngle <= 0.5f) {
        // Deactivate after ricochet or failed penetration
        m_active = false;
    }
}

void Projectile::updatePhysics(float deltaTime) {
    // Update position based on velocity
    m_position += m_velocity * deltaTime;
    
    // Apply gravity if enabled
    if (m_hasGravity) {
        m_velocity.y -= 9.81f * deltaTime;
    }
    
    // Update lifetime
    m_lifetime += deltaTime;
    
    // Deactivate if max lifetime reached
    if (m_lifetime >= m_maxLifetime) {
        m_active = false;
    }
    
    // Update trail effect
    updateTrail(deltaTime);
}

void Projectile::addToWorld(void* world) {
    // TODO: Add projectile to physics world when Bullet integration is complete
    (void)world; // Suppress unused parameter warning
}

void Projectile::removeFromWorld(void* world) {
    // TODO: Remove projectile from physics world when Bullet integration is complete
    (void)world; // Suppress unused parameter warning
}

void Projectile::createExplosion(const glm::vec3& position) {
    // Create explosion particle effect
    auto explosion = std::make_unique<ParticleSystem>();
    explosion->initialize(position, 50, 2.0f); // 50 particles, 2 second duration
    explosion->setExplosionProperties();
    
    // The explosion would be managed by a particle system manager
    Logger::debug("Explosion created at position: " + 
                std::to_string(position.x) + ", " + 
                std::to_string(position.y) + ", " + 
                std::to_string(position.z));
}

void Projectile::createTrail() {
    m_trailEffect = std::make_unique<ParticleSystem>();
    m_trailEffect->initialize(m_position, 100, 0.5f); // 100 particles, 0.5 second lifetime
    m_trailEffect->setTrailProperties();
}

void Projectile::updateTrail(float deltaTime) {
    if (m_trailEffect) {
        // Update trail effect to follow projectile
        m_trailEffect->setPosition(m_position);
        m_trailEffect->update(deltaTime);
    }
}

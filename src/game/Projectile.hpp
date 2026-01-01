#pragma once

#include <memory>
#include <glm/glm.hpp>

class Vehicle;
class ParticleSystem;

class Projectile {
public:
    Projectile(const glm::vec3& position, const glm::vec3& velocity, 
               float damage, float range, Vehicle* owner);
    ~Projectile();
    
    void update(float deltaTime);
    void render();
    
    // Getters
    const glm::vec3& getPosition() const { return m_position; }
    const glm::vec3& getVelocity() const { return m_velocity; }
    float getDamage() const { return m_damage; }
    bool isActive() const { return m_active; }
    Vehicle* getOwner() const { return m_owner; }
    float getLifetime() const { return m_lifetime; }
    float getMaxLifetime() const { return m_maxLifetime; }
    
    // Setters
    void setExplosionRadius(float radius) { m_explosionRadius = radius; }
    void setTrailEffect(bool enabled) { m_hasTrail = enabled; }
    void setPenetration(bool enabled) { m_canPenetrate = enabled; }
    void setGravity(bool enabled) { m_hasGravity = enabled; }
    
    // Collision
    void onCollision(const glm::vec3& hitPoint, const glm::vec3& hitNormal, Vehicle* hitVehicle);
    
    // Physics integration
    void updatePhysics(float deltaTime);
    void addToWorld(void* world);
    void removeFromWorld(void* world);
    
private:
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    float m_damage;
    float m_range;
    Vehicle* m_owner;
    
    // State
    bool m_active;
    float m_lifetime;
    float m_maxLifetime;
    float m_distanceTraveled;
    
    // Properties
    float m_explosionRadius = 0.0f;
    bool m_hasTrail = false;
    bool m_canPenetrate = false;
    bool m_hasGravity = false;
    
    // Visual effects
    std::unique_ptr<ParticleSystem> m_trailEffect;
    
    void createExplosion(const glm::vec3& position);
    void createTrail();
    void updateTrail(float deltaTime);
};

#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Vehicle;
class Projectile;
class ParticleSystem;
class ProjectileManager;
class ParticleManager;
class Renderer;
class PhysicsWorld;
class Camera;

class GameWorld {
public:
    GameWorld();
    ~GameWorld();
    
    bool initialize();
    void shutdown();
    void update(float deltaTime);
    void render(Renderer* renderer);
    
    // Vehicle management
    std::shared_ptr<Vehicle> spawnVehicle(const glm::vec3& position, const std::string& type = "default");
    void addVehicle(const std::shared_ptr<Vehicle>& vehicle);
    void removeVehicle(const std::shared_ptr<Vehicle>& vehicle);
    void clearAllVehicles();
    
    // Projectile management
    void addProjectile(const std::shared_ptr<Projectile>& projectile);
    void removeProjectile(const std::shared_ptr<Projectile>& projectile);
    std::shared_ptr<Projectile> spawnProjectile(const glm::vec3& position, const glm::vec3& direction, 
                                               float damage, float range, Vehicle* owner);
    
    // Particle system management
    void addParticleSystem(const std::shared_ptr<ParticleSystem>& particleSystem);
    void removeParticleSystem(std::shared_ptr<ParticleSystem> system);
    
    // Event handlers
    void onProjectileHit(Projectile* projectile, Vehicle* vehicle, const glm::vec3& hitPoint);
    
    // Additional methods
    void setPlayerVehicle(const std::shared_ptr<Vehicle>& vehicle);
    void setCamera(const std::shared_ptr<Camera>& camera);
    void setWorldBounds(const glm::vec3& min, const glm::vec3& max);
    void setGravity(const glm::vec3& gravity);
    void handleCollisions();
    
    // Cleanup
    void cleanupEntities();
    
    // Getters
    const std::vector<std::shared_ptr<Vehicle>>& getVehicles() const { return m_vehicles; }
    const std::vector<std::shared_ptr<Projectile>>& getProjectiles() const;
    const std::vector<std::shared_ptr<ParticleSystem>>& getParticleSystems() const;
    
    // Player vehicle
    std::shared_ptr<Vehicle> getPlayerVehicle() const { return m_playerVehicle; }
    
private:
    void updateCamera(float deltaTime);
    void checkWorldBounds();
    void updateProjectiles(float deltaTime);
    void updateParticleSystems(float deltaTime);
    
    // Entities
    std::vector<std::shared_ptr<Vehicle>> m_vehicles;
    std::vector<std::shared_ptr<Projectile>> m_projectiles;
    std::vector<std::shared_ptr<ParticleSystem>> m_particleSystems;
    
    // Player
    std::shared_ptr<Vehicle> m_playerVehicle;
    std::shared_ptr<Camera> m_camera;
    
    // World properties
    glm::vec3 m_worldMin = glm::vec3(-1000.0f);
    glm::vec3 m_worldMax = glm::vec3(1000.0f);
    glm::vec3 m_gravity = glm::vec3(0.0f, -9.81f, 0.0f);
    
    // Managers
    std::unique_ptr<class ProjectileManager> m_projectileManager;
    std::unique_ptr<class ParticleManager> m_particleManager;
    
    bool m_initialized = false;
};

#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

class Projectile;
class GameWorld;

class ProjectileManager {
public:
    ProjectileManager();
    ~ProjectileManager();
    
    bool initialize();
    void shutdown();
    void update(float deltaTime);
    
    // Projectile management
    void addProjectile(std::shared_ptr<Projectile> projectile);
    void removeProjectile(std::shared_ptr<Projectile> projectile);
    void clearAllProjectiles();
    
    // Getters
    const std::vector<std::shared_ptr<Projectile>>& getProjectiles() const { return m_projectiles; }
    size_t getProjectileCount() const { return m_projectiles.size(); }
    
    // Collision detection
    void checkCollisions();
    
private:
    std::vector<std::shared_ptr<Projectile>> m_projectiles;
    bool m_initialized = false;
};

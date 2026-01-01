#include "game/ProjectileManager.hpp"
#include "game/Projectile.hpp"
#include "core/Logger.hpp"
#include <algorithm>

ProjectileManager::ProjectileManager() = default;

ProjectileManager::~ProjectileManager() {
    shutdown();
}

bool ProjectileManager::initialize() {
    Logger::info("Initializing Projectile Manager");
    
    m_initialized = true;
    Logger::info("Projectile Manager initialized successfully");
    return true;
}

void ProjectileManager::shutdown() {
    Logger::info("Shutting down Projectile Manager");
    
    clearAllProjectiles();
    m_initialized = false;
}

void ProjectileManager::update(float deltaTime) {
    if (!m_initialized) return;
    
    // Update all projectiles
    for (auto& projectile : m_projectiles) {
        if (projectile) {
            projectile->update(deltaTime);
        }
    }
    
    // Remove inactive projectiles
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const std::shared_ptr<Projectile>& projectile) {
                return !projectile || !projectile->isActive();
            }),
        m_projectiles.end()
    );
    
    // Check collisions
    checkCollisions();
}

void ProjectileManager::addProjectile(std::shared_ptr<Projectile> projectile) {
    if (projectile) {
        m_projectiles.push_back(projectile);
        Logger::debug("Projectile added to manager");
    }
}

void ProjectileManager::removeProjectile(std::shared_ptr<Projectile> projectile) {
    auto it = std::find(m_projectiles.begin(), m_projectiles.end(), projectile);
    if (it != m_projectiles.end()) {
        m_projectiles.erase(it);
        Logger::debug("Projectile removed from manager");
    }
}

void ProjectileManager::clearAllProjectiles() {
    m_projectiles.clear();
    Logger::debug("All projectiles cleared");
}

void ProjectileManager::checkCollisions() {
    // This would handle collision detection between projectiles and other objects
    // For now, it's a placeholder
}

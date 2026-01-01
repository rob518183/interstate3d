#include "game/GameWorld.hpp"
#include "game/Vehicle.hpp"
#include "game/Projectile.hpp"
#include "game/ParticleSystem.hpp"
#include "game/ProjectileManager.hpp"
#include "game/ParticleManager.hpp"
#include "physics/PhysicsWorld.hpp"
#include "rendering/Renderer.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <glm/gtx/quaternion.hpp>

GameWorld::GameWorld() = default;

GameWorld::~GameWorld() {
    shutdown();
}

bool GameWorld::initialize() {
    Logger::info("Initializing game world...");
    
    // Create managers
    m_projectileManager = std::make_unique<class ProjectileManager>();
    m_particleManager = std::make_unique<class ParticleManager>();
    
    // Initialize managers
    if (!m_projectileManager->initialize() || !m_particleManager->initialize()) {
        Logger::error("Failed to initialize game world managers");
        return false;
    }
    
    Logger::info("Game world initialized successfully");
    return true;
}

void GameWorld::shutdown() {
    Logger::info("Shutting down game world...");
    
    // Clear all entities
    m_vehicles.clear();
    m_projectiles.clear();
    m_particleSystems.clear();
    
    // Reset managers
    m_projectileManager.reset();
    m_particleManager.reset();
    
    m_playerVehicle.reset();
    m_camera.reset();
    
    m_initialized = false;
}

void GameWorld::update(float deltaTime) {
    if (!m_initialized) {
        return;
    }
    
    // Update all vehicles
    for (auto& vehicle : m_vehicles) {
        if (vehicle) {
            vehicle->update(deltaTime);
        }
    }
    
    // Update projectiles
    updateProjectiles(deltaTime);
    
    // Update particle systems
    updateParticleSystems(deltaTime);
    
    // Update camera to follow player
    updateCamera(deltaTime);
    
    // Handle collisions
    handleCollisions();
    
    // Clean up inactive entities
    cleanupEntities();
    
    // Check world bounds
    checkWorldBounds();
}

void GameWorld::render(Renderer* renderer) {
    if (!renderer || !m_initialized) {
        return;
    }
    
    // Set camera
    renderer->setCamera(m_camera);
    
    // Render all vehicles
    for (auto& vehicle : m_vehicles) {
        if (vehicle && vehicle->getChassisMesh()) {
            glm::mat4 chassisTransform = vehicle->getChassisTransform();
            renderer->renderMesh(vehicle->getChassisMesh(), chassisTransform);
            
            // Render wheels
            if (vehicle->getWheelMesh()) {
                for (int i = 0; i < 4; ++i) {
                    glm::mat4 wheelTransform = vehicle->getWheelTransform(i);
                    renderer->renderMesh(vehicle->getWheelMesh(), wheelTransform);
                }
            }
        }
    }
    
    // Render projectiles
    for (auto& projectile : m_projectiles) {
        if (projectile && projectile->isActive()) {
            // Render projectile as a small sphere or line
            // This would require a simple projectile mesh
        }
    }
    
    // Render particle systems
    for (auto& particleSystem : m_particleSystems) {
        if (particleSystem) {
            particleSystem->render();
        }
    }
}

void GameWorld::addVehicle(const std::shared_ptr<Vehicle>& vehicle) {
    if (vehicle) {
        m_vehicles.push_back(vehicle);
        Logger::info("Vehicle added to game world");
    }
}

void GameWorld::removeVehicle(const std::shared_ptr<Vehicle>& vehicle) {
    auto it = std::remove(m_vehicles.begin(), m_vehicles.end(), vehicle);
    if (it != m_vehicles.end()) {
        m_vehicles.erase(it, m_vehicles.end());
        Logger::info("Vehicle removed from game world");
    }
}

void GameWorld::addProjectile(const std::shared_ptr<Projectile>& projectile) {
    if (projectile) {
        m_projectiles.push_back(projectile);
        Logger::debug("Projectile added to game world");
    }
}

void GameWorld::removeProjectile(const std::shared_ptr<Projectile>& projectile) {
    auto it = std::remove(m_projectiles.begin(), m_projectiles.end(), projectile);
    if (it != m_projectiles.end()) {
        m_projectiles.erase(it, m_projectiles.end());
        Logger::debug("Projectile removed from game world");
    }
}

void GameWorld::addParticleSystem(const std::shared_ptr<ParticleSystem>& particleSystem) {
    if (particleSystem) {
        m_particleSystems.push_back(particleSystem);
        Logger::debug("Particle system added to game world");
    }
}

void GameWorld::setPlayerVehicle(const std::shared_ptr<Vehicle>& vehicle) {
    m_playerVehicle = vehicle;
    if (vehicle) {
        Logger::info("Player vehicle set");
    }
}

void GameWorld::setCamera(const std::shared_ptr<Camera>& camera) {
    m_camera = camera;
    if (camera) {
        Logger::info("Camera set");
    }
}

void GameWorld::setWorldBounds(const glm::vec3& min, const glm::vec3& max) {
    m_worldMin = min;
    m_worldMax = max;
    Logger::info("World bounds set: min(" + std::to_string(min.x) + ", " + 
                std::to_string(min.y) + ", " + std::to_string(min.z) + "), max(" +
                std::to_string(max.x) + ", " + std::to_string(max.y) + ", " + 
                std::to_string(max.z) + ")");
}

void GameWorld::setGravity(const glm::vec3& gravity) {
    m_gravity = gravity;
    Logger::info("Gravity set: " + std::to_string(gravity.x) + ", " + 
                std::to_string(gravity.y) + ", " + std::to_string(gravity.z));
}

std::shared_ptr<Vehicle> GameWorld::spawnVehicle(const glm::vec3& position, const std::string& type) {
    auto vehicle = std::make_shared<Vehicle>();
    
    if (!vehicle->initialize()) {
        Logger::error("Failed to initialize spawned vehicle");
        return nullptr;
    }
    
    // Set position
    vehicle->setPosition(position);
    
    // Add vehicle to world
    m_vehicles.push_back(vehicle);
    
    Logger::info("Vehicle spawned at position: " + std::to_string(position.x) + ", " + 
                std::to_string(position.y) + ", " + std::to_string(position.z));
    return vehicle;
}

std::shared_ptr<Projectile> GameWorld::spawnProjectile(const glm::vec3& position, const glm::vec3& direction, 
                                                      float damage, float range, Vehicle* owner) {
    auto projectile = std::make_shared<Projectile>(position, direction, damage, range, owner);
    addProjectile(projectile);
    
    Logger::debug("Projectile spawned");
    return projectile;
}

void GameWorld::handleCollisions() {
    // Check projectile-vehicle collisions
    for (auto& projectile : m_projectiles) {
        if (!projectile || !projectile->isActive()) {
            continue;
        }
        
        for (auto& vehicle : m_vehicles) {
            if (!vehicle || vehicle->isDestroyed()) {
                continue;
            }
            
            // Simple distance-based collision check
            float distance = glm::length(projectile->getPosition() - vehicle->getPosition());
            if (distance < 3.0f) { // Approximate vehicle size
                onProjectileHit(projectile.get(), vehicle.get(), projectile->getPosition());
                break;
            }
        }
    }
    
    // Check vehicle-vehicle collisions
    // This would be handled by the physics world in a real implementation
}

void GameWorld::onProjectileHit(Projectile* projectile, Vehicle* vehicle, const glm::vec3& hitPoint) {
    if (!projectile || !vehicle) {
        return;
    }
    
    // Apply damage
    vehicle->applyDamage(projectile->getDamage());
    
    // Handle projectile collision
    projectile->onCollision(hitPoint, glm::vec3(0.0f, 1.0f, 0.0f), vehicle);
    
    // Create hit effect
    auto hitEffect = std::make_shared<ParticleSystem>();
    hitEffect->initialize(hitPoint, 20, 0.5f);
    hitEffect->setExplosionProperties();
    addParticleSystem(hitEffect);
    
    Logger::info("Projectile hit vehicle for " + std::to_string(projectile->getDamage()) + " damage");
}

void GameWorld::cleanupEntities() {
    // Remove inactive projectiles
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const std::shared_ptr<Projectile>& p) { return !p || !p->isActive(); }),
        m_projectiles.end()
    );
    
    // Remove completed particle systems
    m_particleSystems.erase(
        std::remove_if(m_particleSystems.begin(), m_particleSystems.end(),
            [](const std::shared_ptr<ParticleSystem>& ps) { return !ps || ps->isComplete(); }),
        m_particleSystems.end()
    );
    
    // Remove destroyed vehicles
    m_vehicles.erase(
        std::remove_if(m_vehicles.begin(), m_vehicles.end(),
            [](const std::shared_ptr<Vehicle>& v) { return !v || v->isDestroyed(); }),
        m_vehicles.end()
    );
}

void GameWorld::updateCamera(float deltaTime) {
    if (!m_playerVehicle) return;
    
    // Get vehicle position
    glm::vec3 vehiclePos = m_playerVehicle->getPosition();
    
    // Camera update - placeholder for now
    // In a real implementation, this would update the camera position to follow the vehicle
    Logger::debug("Updating camera to follow vehicle at position: " + 
                 std::to_string(vehiclePos.x) + ", " + 
                 std::to_string(vehiclePos.y) + ", " + 
                 std::to_string(vehiclePos.z));
}

void GameWorld::checkWorldBounds() {
    // Check vehicles
    for (auto& vehicle : m_vehicles) {
        if (!vehicle) continue;
        
        glm::vec3 pos = vehicle->getPosition();
        bool outOfBounds = false;
        
        if (pos.x < m_worldMin.x || pos.x > m_worldMax.x ||
            pos.y < m_worldMin.y || pos.y > m_worldMax.y ||
            pos.z < m_worldMin.z || pos.z > m_worldMax.z) {
            outOfBounds = true;
        }
        
        if (outOfBounds) {
            // Clamp position to bounds
            pos.x = glm::clamp(pos.x, m_worldMin.x, m_worldMax.x);
            pos.y = glm::clamp(pos.y, m_worldMin.y, m_worldMax.y);
            pos.z = glm::clamp(pos.z, m_worldMin.z, m_worldMax.z);
            vehicle->setPosition(pos);
            
            Logger::warn("Vehicle clamped to world bounds");
        }
    }
    
    // Remove projectiles that are out of bounds
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [this](const std::shared_ptr<Projectile>& p) {
                if (!p) return true;
                glm::vec3 pos = p->getPosition();
                return (pos.x < m_worldMin.x || pos.x > m_worldMax.x ||
                        pos.y < m_worldMin.y || pos.y > m_worldMax.y ||
                        pos.z < m_worldMin.z || pos.z > m_worldMax.z);
            }),
        m_projectiles.end()
    );
}

void GameWorld::updateProjectiles(float deltaTime) {
    for (auto& projectile : m_projectiles) {
        if (projectile && projectile->isActive()) {
            projectile->update(deltaTime);
        }
    }
}

void GameWorld::updateParticleSystems(float deltaTime) {
    for (auto& particleSystem : m_particleSystems) {
        if (particleSystem) {
            particleSystem->update(deltaTime);
        }
    }
}

const std::vector<std::shared_ptr<Projectile>>& GameWorld::getProjectiles() const {
    return m_projectiles;
}

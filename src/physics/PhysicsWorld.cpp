#include "physics/PhysicsWorld.hpp"
#include "core/Logger.hpp"
#include "game/Projectile.hpp"
#include "game/Vehicle.hpp"
#include "physics/RigidBody.hpp"
#include <btBulletDynamicsCommon.h>
#include <algorithm>

PhysicsWorld::PhysicsWorld() = default;

PhysicsWorld::~PhysicsWorld() {
    shutdown();
}

bool PhysicsWorld::initialize() {
    Logger::info("Initializing physics world...");
    
    // Create Bullet physics components
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver();
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);
    
    // Set default gravity (Earth-like)
    m_dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
    
    m_initialized = true;
    Logger::info("Physics world initialized successfully");
    return true;
}

void PhysicsWorld::shutdown() {
    Logger::info("Shutting down physics world...");
    
    // Clear all rigid bodies and vehicles
    m_rigidBodies.clear();
    m_vehicles.clear();
    m_projectiles.clear();
    
    // Clean up Bullet physics components
    if (m_dynamicsWorld) {
        delete m_dynamicsWorld;
        m_dynamicsWorld = nullptr;
    }
    if (m_solver) {
        delete m_solver;
        m_solver = nullptr;
    }
    if (m_overlappingPairCache) {
        delete m_overlappingPairCache;
        m_overlappingPairCache = nullptr;
    }
    if (m_dispatcher) {
        delete m_dispatcher;
        m_dispatcher = nullptr;
    }
    if (m_collisionConfiguration) {
        delete m_collisionConfiguration;
        m_collisionConfiguration = nullptr;
    }
    
    Logger::info("Physics world shutdown complete");
    m_initialized = false;
}

void PhysicsWorld::update(float deltaTime) {
    if (!m_initialized || !m_dynamicsWorld) {
        return;
    }
    
    // Use Bullet Physics simulation
    m_dynamicsWorld->stepSimulation(deltaTime, 10);
    
    // Update physics for all vehicles
    for (auto& vehicle : m_vehicles) {
        if (vehicle) {
            vehicle->updatePhysics(deltaTime);
        }
    }
    
    // Update physics for all projectiles
    for (auto& projectile : m_projectiles) {
        if (projectile) {
            projectile->updatePhysics(deltaTime);
        }
    }
}

void PhysicsWorld::setGravity(const glm::vec3& gravity) {
    if (m_dynamicsWorld) {
        m_dynamicsWorld->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
    }
    Logger::debug("Setting gravity to: " + std::to_string(gravity.x) + ", " + 
                 std::to_string(gravity.y) + ", " + std::to_string(gravity.z));
}

void PhysicsWorld::addRigidBody(const std::shared_ptr<RigidBody>& rigidBody) {
    if (m_dynamicsWorld && rigidBody) {
        m_dynamicsWorld->addRigidBody(rigidBody->getBulletBody());
    }
    if (rigidBody) {
        m_rigidBodies.push_back(rigidBody);
    }
}

void PhysicsWorld::removeRigidBody(const std::shared_ptr<RigidBody>& rigidBody) {
    if (!rigidBody) return;
    
    if (m_dynamicsWorld && rigidBody->getBulletBody()) {
        m_dynamicsWorld->removeRigidBody(rigidBody->getBulletBody());
    }
    
    auto it = std::remove_if(m_rigidBodies.begin(), m_rigidBodies.end(),
        [&rigidBody](const std::shared_ptr<RigidBody>& body) {
            return body == rigidBody;
        });
    m_rigidBodies.erase(it, m_rigidBodies.end());
}

void PhysicsWorld::addVehicle(const std::shared_ptr<Vehicle>& vehicle) {
    if (m_dynamicsWorld && vehicle) {
        vehicle->addToWorld(m_dynamicsWorld);
    }
    if (vehicle) {
        m_vehicles.push_back(vehicle);
    }
}

void PhysicsWorld::removeVehicle(const std::shared_ptr<Vehicle>& vehicle) {
    if (!vehicle) return;
    
    if (m_dynamicsWorld && vehicle) {
        vehicle->removeFromWorld(m_dynamicsWorld);
    }
    
    auto it = std::remove_if(m_vehicles.begin(), m_vehicles.end(),
        [&vehicle](const std::shared_ptr<Vehicle>& v) {
            return v == vehicle;
        });
    m_vehicles.erase(it, m_vehicles.end());
}

PhysicsWorld::RaycastResult PhysicsWorld::raycast(const glm::vec3& from, const glm::vec3& to) {
    RaycastResult result;
    
    if (!m_dynamicsWorld) {
        return result;
    }
    
    btVector3 fromBullet(from.x, from.y, from.z);
    btVector3 toBullet(to.x, to.y, to.z);
    btCollisionWorld::ClosestRayResultCallback rayCallback(fromBullet, toBullet);
    
    m_dynamicsWorld->rayTest(fromBullet, toBullet, rayCallback);
    
    if (rayCallback.hasHit()) {
        result.hit = true;
        result.hitPoint = glm::vec3(
            rayCallback.m_hitPointWorld.x(),
            rayCallback.m_hitPointWorld.y(),
            rayCallback.m_hitPointWorld.z()
        );
        result.hitNormal = glm::vec3(
            rayCallback.m_hitNormalWorld.x(),
            rayCallback.m_hitNormalWorld.y(),
            rayCallback.m_hitNormalWorld.z()
        );
        result.hitFraction = rayCallback.m_closestHitFraction;
        result.hitBody = static_cast<RigidBody*>(rayCallback.m_collisionObject->getUserPointer());
    }
    
    return result;
}

void PhysicsWorld::setCollisionCallback(std::function<void(void*, void*)> callback) {
    m_collisionCallback = callback;
}

void PhysicsWorld::addProjectile(const std::shared_ptr<Projectile>& projectile) {
    if (m_dynamicsWorld && projectile) {
        projectile->addToWorld(m_dynamicsWorld);
    }
    if (projectile) {
        m_projectiles.push_back(projectile);
    }
}

void PhysicsWorld::removeProjectile(const std::shared_ptr<Projectile>& projectile) {
    if (!projectile) return;
    
    if (m_dynamicsWorld && projectile) {
        projectile->removeFromWorld(m_dynamicsWorld);
    }
    
    auto it = std::remove_if(m_projectiles.begin(), m_projectiles.end(),
        [&projectile](const std::shared_ptr<Projectile>& p) {
            return p == projectile;
        });
    m_projectiles.erase(it, m_projectiles.end());
}

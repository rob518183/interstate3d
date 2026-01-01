#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <functional>

// Forward declarations for Bullet Physics
class btDiscreteDynamicsWorld;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btDbvtBroadphase;
class btSequentialImpulseConstraintSolver;
class btRigidBody;
class btCollisionShape;
class btBroadphaseInterface;

class RigidBody;
class CollisionShape;
class Vehicle;
class Projectile;

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();
    
    bool initialize();
    void shutdown();
    
    void update(float deltaTime);
    void setGravity(const glm::vec3& gravity);
    
    // Rigid body management
    void addRigidBody(const std::shared_ptr<RigidBody>& rigidBody);
    void removeRigidBody(const std::shared_ptr<RigidBody>& rigidBody);
    
    // Vehicle management
    void addVehicle(const std::shared_ptr<Vehicle>& vehicle);
    void removeVehicle(const std::shared_ptr<Vehicle>& vehicle);
    
    // Projectile management
    void addProjectile(const std::shared_ptr<Projectile>& projectile);
    void removeProjectile(const std::shared_ptr<Projectile>& projectile);
    
    // Ray casting
    struct RaycastResult {
        bool hit = false;
        glm::vec3 hitPoint;
        glm::vec3 hitNormal;
        float hitFraction = 0.0f;
        RigidBody* hitBody = nullptr;
    };
    
    RaycastResult raycast(const glm::vec3& from, const glm::vec3& to);
    
    // Collision detection
    void setCollisionCallback(std::function<void(void*, void*)> callback);
    
    // Getters
    btDiscreteDynamicsWorld* getWorld() const { return m_dynamicsWorld; }
    
private:
    // Bullet physics components
    btDefaultCollisionConfiguration* m_collisionConfiguration;
    btCollisionDispatcher* m_dispatcher;
    btBroadphaseInterface* m_overlappingPairCache;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_dynamicsWorld;
    
    std::vector<std::shared_ptr<RigidBody>> m_rigidBodies;
    std::vector<std::shared_ptr<Vehicle>> m_vehicles;
    std::vector<std::shared_ptr<Projectile>> m_projectiles;
    
    std::function<void(void*, void*)> m_collisionCallback;
    bool m_initialized = false;
};

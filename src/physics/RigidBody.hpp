#pragma once

#include <memory>
#include <glm/glm.hpp>

// Forward declarations for Bullet Physics
class btRigidBody;
class btMotionState;
class btCollisionShape;

class CollisionShape;

class RigidBody {
public:
    RigidBody();
    ~RigidBody();
    
    bool initialize(CollisionShape* shape, float mass = 1.0f);
    void shutdown();
    
    // Transform
    void setPosition(const glm::vec3& position);
    void setRotation(const glm::quat& rotation);
    void setTransform(const glm::vec3& position, const glm::quat& rotation);
    
    glm::vec3 getPosition() const;
    glm::quat getRotation() const;
    glm::mat4 getTransform() const;
    
    // Physics properties
    void setMass(float mass);
    void setFriction(float friction);
    void setRestitution(float restitution);
    void setLinearDamping(float damping);
    void setAngularDamping(float damping);
    
    // Forces and impulses
    void applyForce(const glm::vec3& force, const glm::vec3& relativePosition = glm::vec3(0.0f));
    void applyImpulse(const glm::vec3& impulse, const glm::vec3& relativePosition = glm::vec3(0.0f));
    void applyTorque(const glm::vec3& torque);
    void clearForces();
    
    // Velocity
    void setLinearVelocity(const glm::vec3& velocity);
    void setAngularVelocity(const glm::vec3& velocity);
    glm::vec3 getLinearVelocity() const;
    glm::vec3 getAngularVelocity() const;
    
    // Collision
    void setCollisionFilter(int group, int mask);
    void setUserPointer(void* pointer);
    void* getUserPointer() const;
    
    // Get Bullet body
    btRigidBody* getBulletBody() const { return m_body; }
    
    // Activation state
    void activate(bool force = false);
    void setDeactivationTime(float time);
    
private:
    btRigidBody* m_body = nullptr;
    btMotionState* m_motionState = nullptr;
    CollisionShape* m_shape = nullptr;
    
    float m_mass = 1.0f;
    bool m_initialized = false;
};

#include "physics/RigidBody.hpp"
#include "physics/CollisionShape.hpp"
#include "core/Logger.hpp"
#include <btBulletDynamicsCommon.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

RigidBody::RigidBody() = default;

RigidBody::~RigidBody() {
    shutdown();
}

bool RigidBody::initialize(CollisionShape* shape, float mass) {
    if (!shape) {
        Logger::error("Cannot initialize RigidBody with null shape");
        return false;
    }
    
    m_shape = shape;
    m_mass = mass;
    
    // Calculate inertia
    btVector3 localInertia(0, 0, 0);
    if (mass > 0.0f) {
        shape->getBulletShape()->calculateLocalInertia(mass, localInertia);
    }
    
    // Create motion state
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(0, 0, 0));
    m_motionState = new btDefaultMotionState(startTransform);
    
    // Create rigid body
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_motionState, shape->getBulletShape(), localInertia);
    m_body = new btRigidBody(rbInfo);
    
    m_initialized = true;
    Logger::debug("RigidBody initialized with mass: " + std::to_string(mass));
    return true;
}

void RigidBody::shutdown() {
    if (m_initialized) {
        // The body is owned by the physics world and will be deleted there
        // We only delete the motion state
        delete m_motionState;
        m_motionState = nullptr;
        
        m_body = nullptr;
        m_shape = nullptr;
        m_initialized = false;
    }
}

void RigidBody::setPosition(const glm::vec3& position) {
    if (!m_body) return;
    
    btTransform transform = m_body->getWorldTransform();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    m_body->setWorldTransform(transform);
    m_motionState->setWorldTransform(transform);
}

void RigidBody::setRotation(const glm::quat& rotation) {
    if (!m_body) return;
    
    btTransform transform = m_body->getWorldTransform();
    transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    m_body->setWorldTransform(transform);
    m_motionState->setWorldTransform(transform);
}

void RigidBody::setTransform(const glm::vec3& position, const glm::quat& rotation) {
    if (!m_body) return;
    
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    
    m_body->setWorldTransform(transform);
    m_motionState->setWorldTransform(transform);
}

glm::vec3 RigidBody::getPosition() const {
    if (!m_body) return glm::vec3(0.0f);
    
    btTransform transform = m_body->getWorldTransform();
    btVector3 pos = transform.getOrigin();
    return glm::vec3(pos.x(), pos.y(), pos.z());
}

glm::quat RigidBody::getRotation() const {
    if (!m_body) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    
    btTransform transform = m_body->getWorldTransform();
    btQuaternion rot = transform.getRotation();
    return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}

glm::mat4 RigidBody::getTransform() const {
    if (!m_body) return glm::mat4(1.0f);
    
    btTransform transform = m_body->getWorldTransform();
    btMatrix3x3 rotation = transform.getBasis();
    btVector3 translation = transform.getOrigin();
    
    glm::mat4 matrix(1.0f);
    matrix[0][0] = rotation[0][0];
    matrix[1][0] = rotation[0][1];
    matrix[2][0] = rotation[0][2];
    matrix[3][0] = translation.x();
    matrix[0][1] = rotation[1][0];
    matrix[1][1] = rotation[1][1];
    matrix[2][1] = rotation[1][2];
    matrix[3][1] = translation.y();
    matrix[0][2] = rotation[2][0];
    matrix[1][2] = rotation[2][1];
    matrix[2][2] = rotation[2][2];
    matrix[3][2] = translation.z();
    
    return matrix;
}

void RigidBody::setMass(float mass) {
    if (!m_body) return;
    
    m_mass = mass;
    
    btVector3 localInertia(0, 0, 0);
    if (mass > 0.0f) {
        m_shape->getBulletShape()->calculateLocalInertia(mass, localInertia);
    }
    
    m_body->setMassProps(mass, localInertia);
}

void RigidBody::setFriction(float friction) {
    if (!m_body) return;
    m_body->setFriction(friction);
}

void RigidBody::setRestitution(float restitution) {
    if (!m_body) return;
    m_body->setRestitution(restitution);
}

void RigidBody::setLinearDamping(float damping) {
    if (!m_body) return;
    m_body->setDamping(damping, m_body->getAngularDamping());
}

void RigidBody::setAngularDamping(float damping) {
    if (!m_body) return;
    m_body->setDamping(m_body->getLinearDamping(), damping);
}

void RigidBody::applyForce(const glm::vec3& force, const glm::vec3& relativePosition) {
    if (!m_body) return;
    m_body->applyForce(btVector3(force.x, force.y, force.z), 
                       btVector3(relativePosition.x, relativePosition.y, relativePosition.z));
}

void RigidBody::applyImpulse(const glm::vec3& impulse, const glm::vec3& relativePosition) {
    if (!m_body) return;
    m_body->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z), 
                         btVector3(relativePosition.x, relativePosition.y, relativePosition.z));
}

void RigidBody::applyTorque(const glm::vec3& torque) {
    if (!m_body) return;
    m_body->applyTorque(btVector3(torque.x, torque.y, torque.z));
}

void RigidBody::clearForces() {
    if (!m_body) return;
    m_body->clearForces();
}

void RigidBody::setLinearVelocity(const glm::vec3& velocity) {
    if (!m_body) return;
    m_body->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
}

void RigidBody::setAngularVelocity(const glm::vec3& velocity) {
    if (!m_body) return;
    m_body->setAngularVelocity(btVector3(velocity.x, velocity.y, velocity.z));
}

glm::vec3 RigidBody::getLinearVelocity() const {
    if (!m_body) return glm::vec3(0.0f);
    
    btVector3 vel = m_body->getLinearVelocity();
    return glm::vec3(vel.x(), vel.y(), vel.z());
}

glm::vec3 RigidBody::getAngularVelocity() const {
    if (!m_body) return glm::vec3(0.0f);
    
    btVector3 vel = m_body->getAngularVelocity();
    return glm::vec3(vel.x(), vel.y(), vel.z());
}

void RigidBody::setCollisionFilter(int group, int mask) {
    if (!m_body) return;
    
    // This would require custom collision filtering setup
    // For now, we just log it
    Logger::debug("Setting collision filter: group=" + std::to_string(group) + 
                  ", mask=" + std::to_string(mask));
}

void RigidBody::setUserPointer(void* pointer) {
    if (!m_body) return;
    m_body->setUserPointer(pointer);
}

void* RigidBody::getUserPointer() const {
    if (!m_body) return nullptr;
    return m_body->getUserPointer();
}

void RigidBody::activate(bool force) {
    if (!m_body) return;
    m_body->activate(force);
}

void RigidBody::setDeactivationTime(float time) {
    if (!m_body) return;
    m_body->setDeactivationTime(time);
}

#include "physics/Vehicle.hpp"
#include "core/Logger.hpp"

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

VehiclePhysics::VehiclePhysics() {
    // Initialize wheel configurations
    m_wheelDirections = {
        btVector3(0, -1, 0), // Front left
        btVector3(0, -1, 0), // Front right
        btVector3(0, -1, 0), // Rear left
        btVector3(0, -1, 0)  // Rear right
    };
    
    m_wheelAxes = {
        btVector3(-1, 0, 0), // Front left
        btVector3(1, 0, 0),  // Front right
        btVector3(-1, 0, 0), // Rear left
        btVector3(1, 0, 0)   // Rear right
    };
    
    m_wheelRadii = {0.4f, 0.4f, 0.4f, 0.4f};
    m_wheelWidths = {0.3f, 0.3f, 0.3f, 0.3f};
    
    // Wheel positions relative to chassis
    m_wheelPositions = {
        btVector3(-m_chassisDimensions.x() * 0.5f + m_wheelWidths[0] * 0.5f, -m_chassisDimensions.y() * 0.5f, m_chassisDimensions.z() * 0.3f),  // Front left
        btVector3(m_chassisDimensions.x() * 0.5f - m_wheelWidths[1] * 0.5f, -m_chassisDimensions.y() * 0.5f, m_chassisDimensions.z() * 0.3f),   // Front right
        btVector3(-m_chassisDimensions.x() * 0.5f + m_wheelWidths[2] * 0.5f, -m_chassisDimensions.y() * 0.5f, -m_chassisDimensions.z() * 0.3f), // Rear left
        btVector3(m_chassisDimensions.x() * 0.5f - m_wheelWidths[3] * 0.5f, -m_chassisDimensions.y() * 0.5f, -m_chassisDimensions.z() * 0.3f)   // Rear right
    };
}

VehiclePhysics::~VehiclePhysics() {
    shutdown();
}

bool VehiclePhysics::initialize() {
    Logger::info("Initializing vehicle physics with Bullet...");
    
    try {
        createChassis();
        configureTuning();
        createWheels();
        
        m_initialized = true;
        Logger::info("Vehicle physics initialized successfully");
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to initialize vehicle physics: " + std::string(e.what()));
        shutdown();
        return false;
    }
}

void VehiclePhysics::shutdown() {
    if (!m_initialized) return;
    
    Logger::info("Shutting down vehicle physics...");
    
    if (m_vehicle) {
        delete m_vehicle;
        m_vehicle = nullptr;
    }
    
    if (m_vehicleRaycaster) {
        delete m_vehicleRaycaster;
        m_vehicleRaycaster = nullptr;
    }
    
    if (m_chassisBody) {
        delete m_chassisBody;
        m_chassisBody = nullptr;
    }
    
    if (m_chassisShape) {
        delete m_chassisShape;
        m_chassisShape = nullptr;
    }
    
    m_initialized = false;
    Logger::info("Vehicle physics shutdown complete");
}

void VehiclePhysics::createChassis() {
    // Create chassis collision shape
    m_chassisShape = new btBoxShape(m_chassisDimensions * 0.5f);
    
    // Create chassis rigid body
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(0, 0, 0));
    
    btVector3 localInertia(0, 0, 0);
    m_chassisShape->calculateLocalInertia(m_chassisMass, localInertia);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(m_chassisMass, motionState, m_chassisShape, localInertia);
    m_chassisBody = new btRigidBody(rbInfo);
    
    // Set damping
    m_chassisBody->setDamping(0.2f, 0.2f);
    m_chassisBody->setActivationState(DISABLE_DEACTIVATION);
}

void VehiclePhysics::configureTuning() {
    m_tuning.m_suspensionStiffness = m_suspensionStiffness;
    m_tuning.m_suspensionCompression = m_suspensionCompression;
    m_tuning.m_suspensionDamping = m_suspensionDamping;
    m_tuning.m_maxSuspensionTravelCm = 500.0f;
    m_tuning.m_frictionSlip = m_frictionSlip;
    m_tuning.m_maxSuspensionForce = 6000.0f;
}

void VehiclePhysics::createWheels() {
    // This would be called after the vehicle is added to the world
    // The actual wheel creation happens in addToWorld
}

void VehiclePhysics::setPosition(const glm::vec3& position) {
    if (m_chassisBody) {
        btTransform transform = m_chassisBody->getWorldTransform();
        transform.setOrigin(btVector3(position.x, position.y, position.z));
        m_chassisBody->setWorldTransform(transform);
        m_chassisBody->getMotionState()->setWorldTransform(transform);
    }
}

glm::vec3 VehiclePhysics::getPosition() const {
    if (m_chassisBody) {
        btTransform transform = m_chassisBody->getWorldTransform();
        btVector3 origin = transform.getOrigin();
        return glm::vec3(origin.x(), origin.y(), origin.z());
    }
    return glm::vec3(0.0f);
}

void VehiclePhysics::setRotation(const glm::quat& rotation) {
    if (m_chassisBody) {
        btTransform transform = m_chassisBody->getWorldTransform();
        transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
        m_chassisBody->setWorldTransform(transform);
        m_chassisBody->getMotionState()->setWorldTransform(transform);
    }
}

glm::quat VehiclePhysics::getRotation() const {
    if (m_chassisBody) {
        btTransform transform = m_chassisBody->getWorldTransform();
        btQuaternion rotation = transform.getRotation();
        return glm::quat(rotation.w(), rotation.x(), rotation.y(), rotation.z());
    }
    return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

void VehiclePhysics::setThrottle(float throttle) {
    if (m_vehicle) {
        m_vehicle->applyEngineForce(throttle * m_engineForce, 2); // Rear wheels
        m_vehicle->applyEngineForce(throttle * m_engineForce, 3);
    }
}

void VehiclePhysics::setSteering(float steering) {
    if (m_vehicle) {
        m_vehicle->setSteeringValue(steering * m_maxSteeringAngle, 0); // Front wheels
        m_vehicle->setSteeringValue(steering * m_maxSteeringAngle, 1);
    }
}

void VehiclePhysics::setBrake(float brake) {
    if (m_vehicle) {
        for (int i = 0; i < m_vehicle->getNumWheels(); ++i) {
            m_vehicle->setBrake(brake * m_brakeForce, i);
        }
    }
}

void VehiclePhysics::setHandbrake(bool handbrake) {
    // Implement handbrake logic if needed
}

float VehiclePhysics::getSpeed() const {
    if (m_chassisBody) {
        btVector3 velocity = m_chassisBody->getLinearVelocity();
        return velocity.length() * 3.6f; // Convert to km/h
    }
    return 0.0f;
}

float VehiclePhysics::getRPM() const {
    // Simple RPM calculation based on speed
    float speed = getSpeed();
    return 800.0f + (speed / 200.0f) * 4000.0f; // Placeholder calculation
}

int VehiclePhysics::getCurrentGear() const {
    // Placeholder implementation - would normally determine current gear
    // if (m_speed < 10.0f) return 1;
    // if (m_speed < 30.0f) return 2;
    // if (m_speed < 50.0f) return 3;
    // if (m_speed < 70.0f) return 4;
    // if (m_speed < 90.0f) return 5;
    // return 6;
    return m_gear;
}

glm::mat4 VehiclePhysics::getChassisTransform() const {
    // Placeholder implementation - would normally get chassis transform from physics
    // if (m_chassisBody) {
    //     return m_chassisBody->getTransform();
    // }
    
    // Create transform matrix from position and rotation
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, m_position);
    transform *= glm::mat4_cast(m_rotation);
    return transform;
}

glm::mat4 VehiclePhysics::getWheelTransform(int wheelIndex) const {
    // Placeholder implementation - would normally get wheel transform from physics
    // if (m_vehicle && wheelIndex >= 0 && wheelIndex < 4) {
    //     btWheelInfo& wheelInfo = m_vehicle->getWheelInfo(wheelIndex);
    //     wheelInfo.m_worldTransform.getOpenGLMatrix(glm::value_ptr(transform));
    //     return transform;
    // }
    
    // Return a placeholder transform based on wheel index
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, m_position);
    
    // Offset wheels based on index
    switch (wheelIndex) {
        case 0: // Front left
            transform = glm::translate(transform, glm::vec3(-1.0f, 0.0f, 1.5f));
            break;
        case 1: // Front right
            transform = glm::translate(transform, glm::vec3(1.0f, 0.0f, 1.5f));
            break;
        case 2: // Rear left
            transform = glm::translate(transform, glm::vec3(-1.0f, 0.0f, -1.5f));
            break;
        case 3: // Rear right
            transform = glm::translate(transform, glm::vec3(1.0f, 0.0f, -1.5f));
            break;
    }
    
    transform *= glm::mat4_cast(m_rotation);
    return transform;
}

void VehiclePhysics::addToWorld(void* world) {
    // Placeholder implementation - would normally add to physics world
    // btDiscreteDynamicsWorld* dynamicsWorld = static_cast<btDiscreteDynamicsWorld*>(world);
    // if (dynamicsWorld && m_chassisBody) {
    //     dynamicsWorld->addRigidBody(m_chassisBody->getBulletBody());
    //     if (m_vehicle) {
    //         dynamicsWorld->addVehicle(m_vehicle.get());
    //     }
    // }
    
    Logger::info("Vehicle physics added to world");
}

void VehiclePhysics::removeFromWorld(void* world) {
    // Placeholder implementation - would normally remove from physics world
    // btDiscreteDynamicsWorld* dynamicsWorld = static_cast<btDiscreteDynamicsWorld*>(world);
    // if (dynamicsWorld && m_vehicle) {
    //     dynamicsWorld->removeVehicle(m_vehicle.get());
    // }
    // if (dynamicsWorld && m_chassisBody) {
    //     dynamicsWorld->removeRigidBody(m_chassisBody->getBulletBody());
    // }
    
    Logger::info("Vehicle physics removed from world");
}

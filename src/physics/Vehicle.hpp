#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>

class VehiclePhysics {
public:
    VehiclePhysics();
    ~VehiclePhysics();
    
    bool initialize();
    void shutdown();
    
    void setPosition(const glm::vec3& position);
    glm::vec3 getPosition() const;
    
    void setRotation(const glm::quat& rotation);
    glm::quat getRotation() const;
    
    void setThrottle(float throttle);
    void setSteering(float steering);
    void setBrake(float brake);
    void setHandbrake(bool handbrake);
    
    float getSpeed() const;
    float getRPM() const;
    int getCurrentGear() const;
    
    glm::mat4 getChassisTransform() const;
    glm::mat4 getWheelTransform(int wheelIndex) const;
    
    void addToWorld(btDiscreteDynamicsWorld* world);
    void removeFromWorld(btDiscreteDynamicsWorld* world);
    
    void updatePhysics(float deltaTime);
    
private:
    // Bullet Physics components
    btRigidBody* m_chassisBody = nullptr;
    btCollisionShape* m_chassisShape = nullptr;
    btRaycastVehicle* m_vehicle = nullptr;
    btVehicleRaycaster* m_vehicleRaycaster = nullptr;
    btRaycastVehicle::btVehicleTuning m_tuning;
    
    // Vehicle configuration
    btScalar m_chassisMass = 800.0f;
    btVector3 m_chassisDimensions = btVector3(2.0f, 1.0f, 5.0f);
    
    // Wheel configuration
    std::vector<btVector3> m_wheelDirections;
    std::vector<btVector3> m_wheelAxes;
    std::vector<btScalar> m_wheelRadii;
    std::vector<btScalar> m_wheelWidths;
    std::vector<btVector3> m_wheelPositions;
    
    // Engine parameters
    float m_engineForce = 2000.0f;
    float m_brakeForce = 1000.0f;
    float m_maxSteeringAngle = 0.5f;
    float m_suspensionStiffness = 20.0f;
    float m_suspensionDamping = 2.3f;
    float m_suspensionCompression = 4.4f;
    float m_rollInfluence = 0.1f;
    float m_frictionSlip = 1000.0f;
    
    // State
    bool m_initialized = false;
    float m_currentSpeed = 0.0f;
    float m_currentRPM = 800.0f;
    int m_currentGear = 1;
    
    // Helper methods
    void createChassis();
    void createWheels();
    void configureTuning();
};

#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

class Weapon;
class Mesh;

class Vehicle {
public:
    Vehicle();
    ~Vehicle();
    
    bool initialize();
    void update(float deltaTime);
    
    // Position and transform
    void setPosition(const glm::vec3& position);
    glm::vec3 getPosition() const;
    void setRotation(const glm::quat& rotation);
    glm::quat getRotation() const;
    glm::vec3 getRotationEuler() const;
    glm::mat4 getTransform() const;
    
    // Physics integration
    void setPhysicsVehicle(class VehiclePhysics* physicsVehicle);
    class VehiclePhysics* getPhysicsVehicle() const { return m_physicsVehicle; }
    
    // Weapons
    void addWeapon(const std::shared_ptr<Weapon>& weapon);
    void removeWeapon(const std::shared_ptr<Weapon>& weapon);
    void fireWeapon(int weaponIndex);
    void fireAllWeapons();
    void reloadWeapon(int weaponIndex);
    void reloadAllWeapons();
    
    // Vehicle control
    void setThrottle(float throttle);
    void setSteering(float steering);
    void setBrake(float brake);
    void setHandbrake(bool handbrake);
    
    // Vehicle state
    float getSpeed() const;
    float getRPM() const;
    int getCurrentGear() const;
    float getHealth() const { return m_health; }
    bool isDestroyed() const { return m_health <= 0.0f; }
    bool isActive() const { return m_active; }
    
    // Damage
    void applyDamage(float damage);
    void repair();
    void destroy();
    
    // Visual components
    void setChassisMesh(const std::shared_ptr<Mesh>& mesh) { m_chassisMesh = mesh; }
    std::shared_ptr<Mesh> getChassisMesh() const { return m_chassisMesh; }
    void setWheelMesh(const std::shared_ptr<Mesh>& mesh) { m_wheelMesh = mesh; }
    std::shared_ptr<Mesh> getWheelMesh() const { return m_wheelMesh; }
    
    // Transform matrices for rendering
    glm::mat4 getChassisTransform() const;
    glm::mat4 getWheelTransform(int wheelIndex) const;
    
    // Vehicle properties
    void setMaxHealth(float health) { m_maxHealth = health; m_health = health; }
    void setVehicleType(const std::string& type) { m_vehicleType = type; }
    const std::string& getVehicleType() const { return m_vehicleType; }
    
    // Visual properties
    void setColor(const glm::vec3& color) { m_color = color; }
    glm::vec3 getColor() const { return m_color; }
    
    // Physics integration
    void updatePhysics(float deltaTime);
    glm::vec3 getForwardVector() const;
    void addToWorld(void* world);
    void removeFromWorld(void* world);
    
    // Team/ownership
    void setTeam(int team) { m_team = team; }
    int getTeam() const { return m_team; }
    void setOwner(int owner) { m_owner = owner; }
    int getOwner() const { return m_owner; }
    
private:
    void updateWeapons(float deltaTime);
    void updateDamage(float deltaTime);
    void createDestroyedEffect();
    
    // Physics
    class VehiclePhysics* m_physicsVehicle = nullptr;
    
    // Weapons
    std::vector<std::shared_ptr<Weapon>> m_weapons;
    int m_currentWeapon = 0;
    
    // Visual components
    std::shared_ptr<Mesh> m_chassisMesh;
    std::shared_ptr<Mesh> m_wheelMesh;
    
    // Vehicle properties
    std::string m_vehicleType = "default";
    glm::vec3 m_color = glm::vec3(1.0f, 1.0f, 1.0f);
    float m_health = 100.0f;
    float m_maxHealth = 100.0f;
    bool m_active = true;
    bool m_isDestroyed = false;
    
    // Position and rotation
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    
    // Team/ownership
    int m_team = 0;
    int m_owner = -1;
    
    // Damage effects
    float m_damageTimer = 0.0f;
    bool m_onFire = false;
    
    // Vehicle control state
    float m_throttle = 0.0f;
    float m_steering = 0.0f;
    float m_brake = 0.0f;
    bool m_handbrake = false;
    float m_speed = 0.0f;
    float m_rpm = 800.0f;
    int m_gear = 1;
    
    bool m_initialized = false;
};

#include "game/Vehicle.hpp"
#include "game/Weapon.hpp"
#include "game/ParticleSystem.hpp"
#include "core/Logger.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>

Vehicle::Vehicle() = default;

Vehicle::~Vehicle() {
    // Weapon cleanup is handled by smart pointers
    m_weapons.clear();
}

bool Vehicle::initialize() {
    Logger::info("Initializing vehicle: " + m_vehicleType);
    
    // Add default weapons based on vehicle type
    if (m_vehicleType == "player") {
        m_weapons.push_back(std::make_shared<MachineGun>());
        m_weapons.push_back(std::make_shared<RocketLauncher>());
    } else if (m_vehicleType == "enemy") {
        m_weapons.push_back(std::make_shared<MachineGun>());
        m_weapons.push_back(std::make_shared<Cannon>());
    } else {
        // Default weapon
        m_weapons.push_back(std::make_shared<MachineGun>());
    }
    
    m_initialized = true;
    Logger::info("Vehicle initialized successfully");
    return true;
}

void Vehicle::update(float deltaTime) {
    if (!m_initialized || !m_active) {
        return;
    }
    
    // Update weapons
    updateWeapons(deltaTime);
    
    // Update damage effects
    updateDamage(deltaTime);
    
    // Check if vehicle should be destroyed
    if (m_health <= 0.0f && !m_isDestroyed) {
        destroy();
    }
}

void Vehicle::setPosition(const glm::vec3& position) {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     m_physicsVehicle->setPosition(position);
    // }
    m_position = position;
}

glm::vec3 Vehicle::getPosition() const {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     return m_physicsVehicle->getPosition();
    // }
    return m_position;
}

void Vehicle::setRotation(const glm::quat& rotation) {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     m_physicsVehicle->setRotation(rotation);
    // }
    m_rotation = rotation;
}

glm::quat Vehicle::getRotation() const {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     return m_physicsVehicle->getRotation();
    // }
    return m_rotation;
}

glm::vec3 Vehicle::getRotationEuler() const {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     return m_physicsVehicle->getRotation();
    // }
    return glm::eulerAngles(m_rotation);
}

glm::mat4 Vehicle::getTransform() const {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     return m_physicsVehicle->getChassisTransform();
    // }
    return glm::mat4(1.0f); // Identity matrix as placeholder
}

void Vehicle::setPhysicsVehicle(class VehiclePhysics* physicsVehicle) {
    m_physicsVehicle = physicsVehicle;
}

void Vehicle::addWeapon(const std::shared_ptr<Weapon>& weapon) {
    if (weapon) {
        m_weapons.push_back(weapon);
        Logger::info("Weapon added to vehicle: " + weapon->getName());
    }
}

void Vehicle::removeWeapon(const std::shared_ptr<Weapon>& weapon) {
    auto it = std::remove_if(m_weapons.begin(), m_weapons.end(),
        [&weapon](const std::shared_ptr<Weapon>& w) {
            return w == weapon;
        });
    m_weapons.erase(it, m_weapons.end());
    Logger::info("Weapon removed from vehicle");
}

void Vehicle::fireWeapon(int weaponIndex) {
    if (weaponIndex < 0 || weaponIndex >= m_weapons.size()) {
        return;
    }
    
    auto& weapon = m_weapons[weaponIndex];
    if (!weapon) {
        return;
    }
    
    // Calculate fire position and direction
    glm::vec3 position = getPosition();
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f); // Vehicle forward
    glm::quat rotation = getRotation();
    glm::vec3 direction = rotation * forward;
    
    // Offset position slightly in front of vehicle
    position += direction * 2.0f;
    
    // Fire weapon
    weapon->fire(position, direction, this);
}

void Vehicle::fireAllWeapons() {
    for (size_t i = 0; i < m_weapons.size(); ++i) {
        fireWeapon(static_cast<int>(i));
    }
}

void Vehicle::reloadWeapon(int weaponIndex) {
    if (weaponIndex < 0 || weaponIndex >= m_weapons.size()) {
        return;
    }
    
    auto& weapon = m_weapons[weaponIndex];
    if (weapon) {
        weapon->reload();
    }
}

void Vehicle::reloadAllWeapons() {
    for (auto& weapon : m_weapons) {
        if (weapon) {
            weapon->reload();
        }
    }
}

void Vehicle::setThrottle(float throttle) {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     m_physicsVehicle->setThrottle(throttle);
    // }
    m_throttle = throttle;
}

void Vehicle::setSteering(float steering) {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     m_physicsVehicle->setSteering(steering);
    // }
    m_steering = steering;
}

void Vehicle::setBrake(float brake) {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     m_physicsVehicle->setBrake(brake);
    // }
    m_brake = brake;
}

void Vehicle::setHandbrake(bool handbrake) {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     m_physicsVehicle->setHandbrake(handbrake);
    // }
    m_handbrake = handbrake;
}

float Vehicle::getSpeed() const {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     return m_physicsVehicle->getSpeed();
    // }
    return m_speed;
}

float Vehicle::getRPM() const {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     return m_physicsVehicle->getRPM();
    // }
    return m_rpm;
}

int Vehicle::getCurrentGear() const {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     return m_physicsVehicle->getCurrentGear();
    // }
    return m_gear;
}

void Vehicle::applyDamage(float damage) {
    if (!m_active || m_isDestroyed) {
        return;
    }
    
    m_health -= damage;
    
    if (m_health < 0.0f) {
        m_health = 0.0f;
    }
    
    Logger::info("Vehicle took " + std::to_string(damage) + " damage, health: " + 
                std::to_string(m_health) + "/" + std::to_string(m_maxHealth));
    
    // Start damage timer for effects
    m_damageTimer = 0.5f;
    
    // Check if vehicle should catch fire
    if (m_health < m_maxHealth * 0.3f && !m_onFire) {
        m_onFire = true;
        Logger::info("Vehicle caught fire!");
    }
}

void Vehicle::repair() {
    m_health = m_maxHealth;
    m_isDestroyed = false;
    m_active = true;
    m_onFire = false;
    m_damageTimer = 0.0f;
    
    Logger::info("Vehicle repaired");
}

void Vehicle::destroy() {
    m_isDestroyed = true;
    m_active = false;
    
    // Create explosion effect
    createDestroyedEffect();
    
    Logger::info("Vehicle destroyed");
}

glm::mat4 Vehicle::getChassisTransform() const {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle) {
    //     return m_physicsVehicle->getChassisTransform();
    // }
    return glm::mat4(1.0f); // Identity matrix as placeholder
}

glm::mat4 Vehicle::getWheelTransform(int wheelIndex) const {
    // Placeholder implementation - would normally delegate to physics vehicle
    // if (m_physicsVehicle && wheelIndex >= 0 && wheelIndex < 4) {
    //     return m_physicsVehicle->getWheelTransform(wheelIndex);
    // }
    return glm::mat4(1.0f); // Identity matrix as placeholder
}

void Vehicle::updateWeapons(float deltaTime) {
    for (auto& weapon : m_weapons) {
        if (weapon) {
            weapon->update(deltaTime);
        }
    }
}

void Vehicle::updateDamage(float deltaTime) {
    if (m_damageTimer > 0.0f) {
        m_damageTimer -= deltaTime;
    }
    
    // Apply fire damage
    if (m_onFire) {
        applyDamage(5.0f * deltaTime);
    }
}

void Vehicle::createDestroyedEffect() {
    // Create explosion particle system
    auto explosion = std::make_shared<ParticleSystem>();
    explosion->initialize(getPosition(), 100, 2.0f);
    explosion->setExplosionProperties();
    
    // This would be added to a particle system manager
    // For now, we just log it
    Logger::info("Vehicle explosion effect created");
}

void Vehicle::updatePhysics(float deltaTime) {
    if (m_physicsVehicle) {
        // TODO: Update position and rotation from physics when VehiclePhysics is fully implemented
        // For now, we'll skip physics updates to avoid compilation errors
        // m_position = m_physicsVehicle->getPosition();
        // m_rotation = m_physicsVehicle->getRotation();
        // m_speed = m_physicsVehicle->getSpeed();
        // m_rpm = m_physicsVehicle->getRPM();
        // m_gear = m_physicsVehicle->getCurrentGear();
        
        // Basic physics simulation for demo
        m_position += glm::vec3(0.0f, 0.0f, m_throttle * deltaTime * 10.0f);
        m_speed = m_throttle * 100.0f; // Simple speed calculation
        m_rpm = 800.0f + m_throttle * 6000.0f; // Simple RPM calculation
    }
    
    (void)deltaTime; // Suppress unused parameter warning
}

glm::vec3 Vehicle::getForwardVector() const {
    // Extract forward vector from rotation quaternion
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    forward = glm::rotate(m_rotation, forward);
    return glm::normalize(forward);
}

void Vehicle::addToWorld(void* world) {
    // TODO: Add vehicle to physics world when Bullet integration is complete
    (void)world; // Suppress unused parameter warning
}

void Vehicle::removeFromWorld(void* world) {
    // TODO: Remove vehicle from physics world when Bullet integration is complete
    (void)world; // Suppress unused parameter warning
}

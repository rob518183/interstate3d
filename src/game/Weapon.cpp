#include "game/Weapon.hpp"
#include "game/Vehicle.hpp"
#include "game/Projectile.hpp"
#include "core/Logger.hpp"
#include <algorithm>

// Base Weapon implementation
Weapon::Weapon(Type type, const std::string& name)
    : m_type(type), m_name(name), m_currentAmmo(100), m_maxAmmo(100),
      m_fireRate(1.0f), m_damage(10.0f), m_range(100.0f), m_fireMode(FireMode::FULL_AUTO),
      m_ready(true), m_reloading(false), m_reloadTime(2.0f), m_reloadProgress(0.0f),
      m_timeSinceLastFire(0.0f) {
}

void Weapon::update(float deltaTime) {
    // Update fire cooldown
    if (!m_ready) {
        m_timeSinceLastFire += deltaTime;
        if (m_timeSinceLastFire >= 1.0f / m_fireRate) {
            m_ready = true;
            m_timeSinceLastFire = 0.0f;
        }
    }
    
    // Update reload progress
    if (m_reloading) {
        m_reloadProgress += deltaTime / m_reloadTime;
        if (m_reloadProgress >= 1.0f) {
            finishReload();
        }
    }
    
    // Cool down overheat
    m_overheatLevel = std::max(0.0f, m_overheatLevel - m_coolDownRate * deltaTime);
}

void Weapon::reload() {
    if (!m_reloading && m_currentAmmo < m_maxAmmo) {
        startReload();
    }
}

void Weapon::startReload() {
    m_reloading = true;
    m_reloadProgress = 0.0f;
    m_ready = false;
    Logger::info("Weapon reloading: " + m_name);
}

void Weapon::finishReload() {
    m_reloading = false;
    m_currentAmmo = m_maxAmmo;
    m_ready = true;
    m_reloadProgress = 0.0f;
    Logger::info("Weapon reloaded: " + m_name);
}

bool Weapon::canFire() const {
    return m_ready && !m_reloading && m_currentAmmo > 0 && !isOverheated();
}

void Weapon::addAttachment(Attachment attachment) {
    if (std::find(m_attachments.begin(), m_attachments.end(), attachment) == m_attachments.end()) {
        m_attachments.push_back(attachment);
        
        // Apply attachment effects
        switch (attachment) {
            case Attachment::SCOPE:
                m_range *= 1.5f;
                break;
            case Attachment::SILENCER:
                m_damage *= 0.9f;
                break;
            case Attachment::GRIP:
                // Better stability - could affect accuracy
                break;
            case Attachment::MAGAZINE:
                m_maxAmmo = static_cast<int>(m_maxAmmo * 1.5f);
                break;
            case Attachment::STABILIZER:
                m_overheatRate *= 0.8f;
                break;
        }
        
        Logger::info("Attachment added to " + m_name + ": " + std::to_string(static_cast<int>(attachment)));
    }
}

void Weapon::removeAttachment(Attachment attachment) {
    auto it = std::find(m_attachments.begin(), m_attachments.end(), attachment);
    if (it != m_attachments.end()) {
        m_attachments.erase(it);
        
        // Revert attachment effects - simplified, would need better tracking
        switch (attachment) {
            case Attachment::SCOPE:
                m_range /= 1.5f;
                break;
            case Attachment::SILENCER:
                m_damage /= 0.9f;
                break;
            case Attachment::MAGAZINE:
                m_maxAmmo = static_cast<int>(m_maxAmmo / 1.5f);
                break;
            case Attachment::STABILIZER:
                m_overheatRate /= 0.8f;
                break;
        }
        
        Logger::info("Attachment removed from " + m_name + ": " + std::to_string(static_cast<int>(attachment)));
    }
}

bool Weapon::hasAttachment(Attachment attachment) const {
    return std::find(m_attachments.begin(), m_attachments.end(), attachment) != m_attachments.end();
}

// Machine Gun implementation
MachineGun::MachineGun() : Weapon(Weapon::Type::MACHINE_GUN, "Machine Gun") {
    m_fireRate = 10.0f;        // 10 rounds per second
    m_damage = 15.0f;
    m_range = 150.0f;
    m_maxAmmo = 200;
    m_currentAmmo = m_maxAmmo;
    m_reloadTime = 1.5f;
    m_fireMode = FireMode::FULL_AUTO;
}

bool MachineGun::fire(const glm::vec3& position, const glm::vec3& direction, Vehicle* owner) {
    if (!canFire()) {
        return false;
    }
    
    // Create bullet projectile
    auto projectile = std::make_shared<Projectile>(
        position,
        direction * 300.0f,  // High velocity
        m_damage,
        m_range,
        owner
    );
    
    // Apply ammo type effects
    switch (m_ammoType) {
        case AmmoType::STANDARD:
            // Default behavior
            break;
        case AmmoType::ARMOR_PIERCING:
            projectile->setPenetration(true);
            projectile->setDamage(m_damage * 1.2f); // Higher damage
            break;
        case AmmoType::EXPLOSIVE:
            projectile->setExplosionRadius(3.0f);
            projectile->setDamage(m_damage * 0.8f); // Less direct damage
            break;
        case AmmoType::INCENDIARY:
            projectile->setTrailEffect(true);
            // Could add fire damage over time
            break;
    }
    
    // Add projectile to game world
    // This would be handled by a projectile manager
    // For now, we'll just log the shot
    Logger::info("Machine gun fired from position: " + 
                std::to_string(position.x) + ", " + 
                std::to_string(position.y) + ", " + 
                std::to_string(position.z) + 
                " with ammo type: " + std::to_string(static_cast<int>(m_ammoType)));
    
    // Update state
    m_currentAmmo--;
    m_ready = false;
    
    // Increase overheat
    m_overheatLevel += m_overheatRate;
    
    // Auto-reload if empty
    if (m_currentAmmo == 0) {
        reload();
    }
    
    return true;
}

// Rocket Launcher implementation
RocketLauncher::RocketLauncher() : Weapon(Weapon::Type::ROCKET_LAUNCHER, "Rocket Launcher") {
    m_fireRate = 2.0f;         // 2 rockets per second
    m_damage = 100.0f;
    m_range = 200.0f;
    m_maxAmmo = 20;
    m_currentAmmo = m_maxAmmo;
    m_reloadTime = 3.0f;
    m_fireMode = FireMode::SEMI_AUTO;
}

bool RocketLauncher::fire(const glm::vec3& position, const glm::vec3& direction, Vehicle* owner) {
    if (!canFire()) {
        return false;
    }
    
    // Create rocket projectile
    auto projectile = std::make_shared<Projectile>(
        position,
        direction * 100.0f,  // Slower than bullets
        m_damage,
        m_range,
        owner
    );
    
    // Set rocket-specific properties
    projectile->setExplosionRadius(15.0f);
    projectile->setTrailEffect(true);
    
    Logger::info("Rocket launcher fired");
    
    // Update state
    m_currentAmmo--;
    m_ready = false;
    
    if (m_currentAmmo == 0) {
        reload();
    }
    
    return true;
}

// Cannon implementation
Cannon::Cannon() : Weapon(Weapon::Type::CANNON, "Cannon") {
    m_fireRate = 1.5f;         // 1.5 rounds per second
    m_damage = 75.0f;
    m_range = 175.0f;
    m_maxAmmo = 30;
    m_currentAmmo = m_maxAmmo;
    m_reloadTime = 2.5f;
    m_fireMode = FireMode::SEMI_AUTO;
}

bool Cannon::fire(const glm::vec3& position, const glm::vec3& direction, Vehicle* owner) {
    if (!canFire()) {
        return false;
    }
    
    // Create cannon shell projectile
    auto projectile = std::make_shared<Projectile>(
        position,
        direction * 200.0f,  // Medium velocity
        m_damage,
        m_range,
        owner
    );
    
    // Set cannon-specific properties
    projectile->setExplosionRadius(8.0f);
    projectile->setPenetration(true);
    
    Logger::info("Cannon fired");
    
    // Update state
    m_currentAmmo--;
    m_ready = false;
    
    if (m_currentAmmo == 0) {
        reload();
    }
    
    return true;
}

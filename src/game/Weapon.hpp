#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp>

class Projectile;
class Vehicle;

class Weapon {
public:
    enum class Type {
        MACHINE_GUN,
        ROCKET_LAUNCHER,
        CANNON,
        MISSILE,
        LASER,
        FLAMETHROWER
    };
    
    enum class FireMode {
        SEMI_AUTO,
        FULL_AUTO,
        BURST
    };
    
    Weapon(Type type, const std::string& name);
    virtual ~Weapon() = default;
    
    virtual bool fire(const glm::vec3& position, const glm::vec3& direction, Vehicle* owner) = 0;
    virtual void update(float deltaTime);
    virtual void reload();
    
    // Getters
    Type getType() const { return m_type; }
    const std::string& getName() const { return m_name; }
    int getCurrentAmmo() const { return m_currentAmmo; }
    int getMaxAmmo() const { return m_maxAmmo; }
    float getFireRate() const { return m_fireRate; }
    float getDamage() const { return m_damage; }
    float getRange() const { return m_range; }
    bool isReady() const { return m_ready; }
    bool isReloading() const { return m_reloading; }
    float getReloadProgress() const { return m_reloadProgress; }
    
    // Setters
    void setMaxAmmo(int maxAmmo) { m_maxAmmo = maxAmmo; }
    void setFireRate(float fireRate) { m_fireRate = fireRate; }
    void setDamage(float damage) { m_damage = damage; }
    // Overheat system
    float getOverheatLevel() const { return m_overheatLevel; }
    float getMaxOverheat() const { return m_maxOverheat; }
    bool isOverheated() const { return m_overheatLevel >= m_maxOverheat; }
    void setOverheatRate(float rate) { m_overheatRate = rate; }
    void setCoolDownRate(float rate) { m_coolDownRate = rate; }
    
    // Attachments system
    enum class Attachment {
        SCOPE,
        SILENCER,
        GRIP,
        MAGAZINE,
        STABILIZER
    };
    void addAttachment(Attachment attachment);
    void removeAttachment(Attachment attachment);
    bool hasAttachment(Attachment attachment) const;
    
    // Ammo types
    enum class AmmoType {
        STANDARD,
        ARMOR_PIERCING,
        EXPLOSIVE,
        INCENDIARY
    };
    void setAmmoType(AmmoType type) { m_ammoType = type; }
    AmmoType getAmmoType() const { return m_ammoType; }
    
protected:
    Type m_type;
    std::string m_name;
    
    // Ammo
    int m_currentAmmo;
    int m_maxAmmo;
    
    // Firing properties
    float m_fireRate;        // Rounds per second
    float m_damage;
    float m_range;
    FireMode m_fireMode;
    
    // State
    bool m_ready;
    bool m_reloading;
    float m_reloadTime;
    float m_reloadProgress;
    float m_timeSinceLastFire;
    
    // Overheat system
    float m_overheatLevel = 0.0f;
    float m_maxOverheat = 100.0f;
    float m_overheatRate = 10.0f;
    float m_coolDownRate = 5.0f;
    
    // Attachments and ammo
    std::vector<Attachment> m_attachments;
    AmmoType m_ammoType = AmmoType::STANDARD;
    
    void startReload();
    void finishReload();
    bool canFire() const;
};

class MachineGun : public Weapon {
public:
    MachineGun();
    bool fire(const glm::vec3& position, const glm::vec3& direction, Vehicle* owner) override;
};

class RocketLauncher : public Weapon {
public:
    RocketLauncher();
    bool fire(const glm::vec3& position, const glm::vec3& direction, Vehicle* owner) override;
};

class Cannon : public Weapon {
public:
    Cannon();
    bool fire(const glm::vec3& position, const glm::vec3& direction, Vehicle* owner) override;
};

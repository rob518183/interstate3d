#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

class PlayerProgress {
public:
    PlayerProgress();
    ~PlayerProgress();
    
    // Experience and leveling
    void addExperience(int xp);
    void setLevel(int level);
    int getLevel() const { return m_level; }
    int getExperience() const { return m_experience; }
    int getExperienceToNextLevel() const;
    float getLevelProgress() const;
    
    // Stats
    void addKill();
    void addDeath();
    void addMissionCompleted();
    void addTimePlayed(float seconds);
    
    int getTotalKills() const { return m_totalKills; }
    int getTotalDeaths() const { return m_totalDeaths; }
    int getMissionsCompleted() const { return m_missionsCompleted; }
    float getTotalPlayTime() const { return m_totalPlayTime; }
    
    // Unlocks and upgrades
    void unlockVehicle(const std::string& vehicleName);
    void unlockWeapon(const std::string& weaponName);
    void unlockUpgrade(const std::string& upgradeName);
    
    bool isVehicleUnlocked(const std::string& vehicleName) const;
    bool isWeaponUnlocked(const std::string& weaponName) const;
    bool isUpgradeUnlocked(const std::string& upgradeName) const;
    
    const std::vector<std::string>& getUnlockedVehicles() const { return m_unlockedVehicles; }
    const std::vector<std::string>& getUnlockedWeapons() const { return m_unlockedWeapons; }
    const std::vector<std::string>& getUnlockedUpgrades() const { return m_unlockedUpgrades; }
    
    // Customization
    void setVehicleColor(const std::string& vehicleName, const glm::vec4& color);
    glm::vec4 getVehicleColor(const std::string& vehicleName) const;
    
    // Persistence
    void saveProgress(const std::string& filename);
    void loadProgress(const std::string& filename);
    
private:
    int m_level = 1;
    int m_experience = 0;
    int m_experiencePerLevel = 1000; // Base XP per level
    
    // Stats
    int m_totalKills = 0;
    int m_totalDeaths = 0;
    int m_missionsCompleted = 0;
    float m_totalPlayTime = 0.0f;
    
    // Unlocks
    std::vector<std::string> m_unlockedVehicles;
    std::vector<std::string> m_unlockedWeapons;
    std::vector<std::string> m_unlockedUpgrades;
    
    // Customization
    std::unordered_map<std::string, glm::vec4> m_vehicleColors;
    
    // Level requirements
    int calculateXPForLevel(int level) const;
};

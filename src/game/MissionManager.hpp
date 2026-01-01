#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <string>

class Mission;
class GameWorld;
class AIManager;

class MissionManager {
public:
    MissionManager();
    ~MissionManager();
    
    bool initialize(std::shared_ptr<GameWorld> world, std::shared_ptr<AIManager> aiManager);
    void shutdown();
    
    void update(float deltaTime);
    
    // Mission management
    void addMission(std::shared_ptr<Mission> mission);
    void removeMission(const std::string& missionName);
    std::shared_ptr<Mission> getCurrentMission() const { return m_currentMission; }
    std::shared_ptr<Mission> getMission(const std::string& name) const;
    
    // Mission control
    bool startMission(const std::string& missionName);
    void pauseCurrentMission();
    void resumeCurrentMission();
    void completeCurrentMission();
    void failCurrentMission(const std::string& reason = "");
    
    // Mission progression
    void nextMission();
    void previousMission();
    void restartMission();
    
    // Mission types
    std::shared_ptr<Mission> createDeathmatchMission(const std::string& name, int targetKills = 10);
    std::shared_ptr<Mission> createTeamDeathmatchMission(const std::string& name, int targetKills = 15);
    std::shared_ptr<Mission> createCaptureTheFlagMission(const std::string& name);
    std::shared_ptr<Mission> createKingOfTheHillMission(const std::string& name, float holdTime = 60.0f);
    std::shared_ptr<Mission> createEscortMission(const std::string& name);
    std::shared_ptr<Mission> createAssassinationMission(const std::string& name, const std::string& target);
    std::shared_ptr<Mission> createRaceMission(const std::string& name, const std::vector<glm::vec3>& checkpoints);
    std::shared_ptr<Mission> createSurvivalMission(const std::string& name, float survivalTime = 300.0f);
    
    // Campaign management
    void loadCampaign(const std::string& campaignFile);
    void saveCampaign(const std::string& campaignFile);
    void startCampaign();
    void setCampaignMissions(const std::vector<std::string>& missionNames);
    
    // Events
    void onMissionCompleted(std::shared_ptr<Mission> mission);
    void onMissionFailed(std::shared_ptr<Mission> mission);
    void onVehicleDestroyed(std::shared_ptr<class Vehicle> vehicle);
    void onPlayerDeath();
    
    // Getters
    const std::vector<std::shared_ptr<Mission>>& getAllMissions() const { return m_missions; }
    const std::vector<std::string>& getCampaignMissions() const { return m_campaignMissions; }
    int getCurrentCampaignIndex() const { return m_campaignIndex; }
    bool isInCampaign() const { return m_inCampaign; }
    
    // Statistics
    int getMissionsCompleted() const { return m_missionsCompleted; }
    int getMissionsFailed() const { return m_missionsFailed; }
    float getTotalPlayTime() const { return m_totalPlayTime; }
    
private:
    void updateCurrentMission(float deltaTime);
    void checkMissionProgression();
    void resetWorld();
    
    std::vector<std::shared_ptr<Mission>> m_missions;
    std::unordered_map<std::string, std::shared_ptr<Mission>> m_missionMap;
    std::shared_ptr<Mission> m_currentMission;
    
    std::shared_ptr<GameWorld> m_world;
    std::shared_ptr<AIManager> m_aiManager;
    
    // Campaign
    std::vector<std::string> m_campaignMissions;
    int m_campaignIndex = 0;
    bool m_inCampaign = false;
    
    // Statistics
    int m_missionsCompleted = 0;
    int m_missionsFailed = 0;
    float m_totalPlayTime = 0.0f;
    
    bool m_initialized = false;
};

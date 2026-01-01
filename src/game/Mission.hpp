#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

class GameWorld;
class Vehicle;
class AIManager;

enum class MissionType {
    DEATHMATCH,
    TEAM_DEATHMATCH,
    CAPTURE_THE_FLAG,
    KING_OF_THE_HILL,
    ESCORT,
    ASSASSINATION,
    RACE,
    SURVIVAL
};

enum class MissionState {
    NOT_STARTED,
    IN_PROGRESS,
    COMPLETED,
    FAILED,
    PAUSED
};

struct MissionObjective {
    std::string description;
    bool completed = false;
    bool optional = false;
    float timeLimit = 0.0f; // 0 = no time limit
    std::function<bool()> completionCheck;
};

struct MissionReward {
    int experience = 0;
    int credits = 0;
    std::vector<std::string> unlockedVehicles;
    std::vector<std::string> unlockedWeapons;
};

class Mission {
public:
    Mission(const std::string& name, MissionType type);
    virtual ~Mission() = default;
    
    virtual bool initialize(std::shared_ptr<GameWorld> world, std::shared_ptr<AIManager> aiManager);
    virtual void shutdown();
    virtual void update(float deltaTime);
    virtual void render();
    
    // Mission control
    void start();
    void pause();
    void resume();
    void complete();
    void fail(const std::string& reason = "");
    
    // Objectives
    void addObjective(const MissionObjective& objective);
    void completeObjective(int index);
    bool areAllObjectivesCompleted() const;
    bool areRequiredObjectivesCompleted() const;
    
    // Spawning
    virtual void spawnVehicles();
    virtual void spawnWorldObjects();
    virtual void setupEnvironment();
    
    // Events
    virtual void onVehicleDestroyed(std::shared_ptr<Vehicle> vehicle);
    virtual void onPlayerDeath();
    virtual void onTimeExpired();
    
    // Getters
    const std::string& getName() const { return m_name; }
    MissionType getType() const { return m_type; }
    MissionState getState() const { return m_state; }
    float getTimeRemaining() const { return m_timeRemaining; }
    float getElapsedTime() const { return m_elapsedTime; }
    const std::vector<MissionObjective>& getObjectives() const { return m_objectives; }
    const MissionReward& getReward() const { return m_reward; }
    
    // Setters
    void setTimeLimit(float timeLimit) { m_timeLimit = timeLimit; }
    void setReward(const MissionReward& reward) { m_reward = reward; }
    void setDescription(const std::string& description) { m_description = description; }
    
protected:
    virtual void updateMission(float deltaTime);
    virtual void checkObjectives();
    virtual void updateTimer(float deltaTime);
    
    std::shared_ptr<GameWorld> m_world;
    std::shared_ptr<AIManager> m_aiManager;
    
    std::string m_name;
    std::string m_description;
    MissionType m_type;
    MissionState m_state = MissionState::NOT_STARTED;
    
    std::vector<MissionObjective> m_objectives;
    MissionReward m_reward;
    
    float m_timeLimit = 0.0f;
    float m_timeRemaining = 0.0f;
    float m_elapsedTime = 0.0f;
    
    bool m_initialized = false;
};

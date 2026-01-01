#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

class AIController;
class Vehicle;
class GameWorld;

class AIManager {
public:
    AIManager();
    ~AIManager();
    
    bool initialize(std::shared_ptr<GameWorld> world);
    void shutdown();
    
    void update(float deltaTime);
    
    // AI controller management
    std::shared_ptr<AIController> createAIController(std::shared_ptr<Vehicle> vehicle);
    void removeAIController(std::shared_ptr<Vehicle> vehicle);
    void removeAIController(std::shared_ptr<AIController> controller);
    
    // Getters
    std::shared_ptr<AIController> getAIController(std::shared_ptr<Vehicle> vehicle) const;
    const std::vector<std::shared_ptr<AIController>>& getAllControllers() const { return m_controllers; }
    size_t getControllerCount() const { return m_controllers.size(); }
    
    // Team management
    void setTeamRelationship(int team1, int team2, bool isEnemy);
    bool areTeamsEnemies(int team1, int team2) const;
    void clearTeamRelationships();
    
    // AI difficulty
    void setGlobalDifficulty(float difficulty); // 0.0 = easy, 1.0 = hard
    float getGlobalDifficulty() const { return m_globalDifficulty; }
    
    // Spawning
    std::shared_ptr<Vehicle> spawnEnemyVehicle(const glm::vec3& position, int team = 1);
    std::shared_ptr<Vehicle> spawnAllyVehicle(const glm::vec3& position, int team = 0);
    void spawnWave(int enemyCount, int allyCount);
    
    // Events
    void onVehicleDestroyed(std::shared_ptr<Vehicle> vehicle);
    void onVehicleDamaged(std::shared_ptr<Vehicle> vehicle, float damage);
    
    // Debug
    void enableDebugMode(bool enable) { m_debugMode = enable; }
    bool isDebugMode() const { return m_debugMode; }
    
private:
    void updateTeamCommunication(float deltaTime);
    void coordinateTeamAttacks();
    void balanceTeams();
    void cleanupDestroyedControllers();
    
    std::vector<std::shared_ptr<AIController>> m_controllers;
    std::unordered_map<Vehicle*, std::shared_ptr<AIController>> m_vehicleToController;
    std::shared_ptr<GameWorld> m_world;
    
    // Team relationships
    std::unordered_map<int, std::unordered_map<int, bool>> m_teamRelationships;
    
    // AI settings
    float m_globalDifficulty = 0.5f;
    float m_updateInterval = 0.1f;
    float m_updateTimer = 0.0f;
    
    // Spawning
    float m_respawnTime = 10.0f;
    std::vector<std::pair<std::shared_ptr<Vehicle>, float>> m_respawnQueue;
    
    // Debug
    bool m_debugMode = false;
    
    bool m_initialized = false;
};

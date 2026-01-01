#include "game/MissionManager.hpp"
#include "game/Mission.hpp"
#include "game/GameWorld.hpp"
#include "ai/AIManager.hpp"
#include "core/Logger.hpp"
#include <algorithm>

MissionManager::MissionManager() = default;

MissionManager::~MissionManager() {
    shutdown();
}

bool MissionManager::initialize(std::shared_ptr<GameWorld> world, std::shared_ptr<AIManager> aiManager) {
    if (!world || !aiManager) {
        Logger::error("MissionManager initialization failed: null world or AI manager");
        return false;
    }
    
    m_world = world;
    m_aiManager = aiManager;
    
    Logger::info("MissionManager initialized successfully");
    m_initialized = true;
    return true;
}

void MissionManager::shutdown() {
    Logger::info("Shutting down MissionManager");
    
    // Stop current mission
    if (m_currentMission) {
        m_currentMission->shutdown();
        m_currentMission.reset();
    }
    
    // Clear all missions
    m_missions.clear();
    m_missionMap.clear();
    m_campaignMissions.clear();
    
    m_world.reset();
    m_aiManager.reset();
    
    m_initialized = false;
}

void MissionManager::update(float deltaTime) {
    if (!m_initialized) return;
    
    updateCurrentMission(deltaTime);
    checkMissionProgression();
    
    m_totalPlayTime += deltaTime;
}

void MissionManager::addMission(std::shared_ptr<Mission> mission) {
    if (!mission) return;
    
    m_missions.push_back(mission);
    m_missionMap[mission->getName()] = mission;
    
    // Initialize mission if manager is initialized
    if (m_initialized) {
        mission->initialize(m_world, m_aiManager);
    }
    
    Logger::info("Mission added: " + mission->getName());
}

void MissionManager::removeMission(const std::string& name) {
    auto missionIt = std::find_if(m_missions.begin(), m_missions.end(),
        [&name](const std::shared_ptr<Mission>& mission) {
            return mission && mission->getName() == name;
        });
    
    if (missionIt != m_missions.end()) {
        m_missions.erase(missionIt);
        Logger::info("Mission removed: " + name);
    }
    
    // Stop if it's current mission
    if (m_currentMission && m_currentMission->getName() == name) {
        m_currentMission.reset();
    }
    
    // Remove from map
    auto it = m_missionMap.find(name);
    if (it != m_missionMap.end()) {
        m_missionMap.erase(it);
        Logger::info("Mission removed from map: " + name);
    }
}

std::shared_ptr<Mission> MissionManager::getMission(const std::string& name) const {
    auto it = m_missionMap.find(name);
    if (it != m_missionMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool MissionManager::startMission(const std::string& missionName) {
    auto mission = getMission(missionName);
    if (!mission) {
        Logger::error("Mission not found: " + missionName);
        return false;
    }
    
    // Stop current mission
    if (m_currentMission) {
        m_currentMission->shutdown();
    }
    
    // Start new mission
    m_currentMission = mission;
    m_currentMission->start();
    
    Logger::info("Mission started: " + missionName);
    return true;
}

void MissionManager::pauseCurrentMission() {
    if (m_currentMission) {
        m_currentMission->pause();
    }
}

void MissionManager::resumeCurrentMission() {
    if (m_currentMission) {
        m_currentMission->resume();
    }
}

void MissionManager::completeCurrentMission() {
    if (m_currentMission) {
        m_currentMission->complete();
        onMissionCompleted(m_currentMission);
    }
}

void MissionManager::failCurrentMission(const std::string& reason) {
    if (m_currentMission) {
        m_currentMission->fail(reason);
        onMissionFailed(m_currentMission);
    }
}

void MissionManager::nextMission() {
    if (!m_inCampaign || m_campaignIndex >= m_campaignMissions.size() - 1) {
        return;
    }
    
    m_campaignIndex++;
    startMission(m_campaignMissions[m_campaignIndex]);
}

void MissionManager::previousMission() {
    if (!m_inCampaign || m_campaignIndex <= 0) {
        return;
    }
    
    m_campaignIndex--;
    startMission(m_campaignMissions[m_campaignIndex]);
}

void MissionManager::restartMission() {
    if (m_currentMission) {
        std::string missionName = m_currentMission->getName();
        startMission(missionName);
    }
}

std::shared_ptr<Mission> MissionManager::createDeathmatchMission(const std::string& name, int targetKills) {
    auto mission = std::make_shared<Mission>(name, MissionType::DEATHMATCH);
    
    // Add kill objective
    MissionObjective objective;
    objective.description = "Eliminate " + std::to_string(targetKills) + " enemy vehicles";
    objective.completionCheck = [targetKills]() {
        // Check kill count
        // This would be implemented with a kill tracking system
        return false; // Placeholder
    };
    mission->addObjective(objective);
    
    return mission;
}

std::shared_ptr<Mission> MissionManager::createTeamDeathmatchMission(const std::string& name, int targetKills) {
    auto mission = std::make_shared<Mission>(name, MissionType::TEAM_DEATHMATCH);
    
    MissionObjective objective;
    objective.description = "Team must eliminate " + std::to_string(targetKills) + " enemy vehicles";
    objective.completionCheck = [targetKills]() {
        // Check team kill count
        return false; // Placeholder
    };
    mission->addObjective(objective);
    
    return mission;
}

std::shared_ptr<Mission> MissionManager::createCaptureTheFlagMission(const std::string& name) {
    auto mission = std::make_shared<Mission>(name, MissionType::CAPTURE_THE_FLAG);
    
    MissionObjective objective;
    objective.description = "Capture the enemy flag";
    objective.completionCheck = []() {
        // Check flag capture
        return false; // Placeholder
    };
    mission->addObjective(objective);
    
    return mission;
}

std::shared_ptr<Mission> MissionManager::createKingOfTheHillMission(const std::string& name, float holdTime) {
    auto mission = std::make_shared<Mission>(name, MissionType::KING_OF_THE_HILL);
    
    MissionObjective objective;
    objective.description = "Hold the hill for " + std::to_string(static_cast<int>(holdTime)) + " seconds";
    objective.completionCheck = [holdTime]() {
        // Check hill hold time
        return false; // Placeholder
    };
    mission->addObjective(objective);
    
    return mission;
}

std::shared_ptr<Mission> MissionManager::createEscortMission(const std::string& name) {
    auto mission = std::make_shared<Mission>(name, MissionType::ESCORT);
    
    MissionObjective objective;
    objective.description = "Escort the VIP vehicle to safety";
    objective.completionCheck = []() {
        // Check VIP reached destination
        return false; // Placeholder
    };
    mission->addObjective(objective);
    
    return mission;
}

std::shared_ptr<Mission> MissionManager::createAssassinationMission(const std::string& name, const std::string& target) {
    auto mission = std::make_shared<Mission>(name, MissionType::ASSASSINATION);
    
    MissionObjective objective;
    objective.description = "Eliminate target: " + target;
    objective.completionCheck = []() {
        // Check target eliminated
        return false; // Placeholder
    };
    mission->addObjective(objective);
    
    return mission;
}

std::shared_ptr<Mission> MissionManager::createRaceMission(const std::string& name, const std::vector<glm::vec3>& checkpoints) {
    auto mission = std::make_shared<Mission>(name, MissionType::RACE);
    
    for (size_t i = 0; i < checkpoints.size(); ++i) {
        MissionObjective objective;
        objective.description = "Reach checkpoint " + std::to_string(i + 1);
        objective.completionCheck = [i, checkpoints]() {
            // Check checkpoint reached
            return false; // Placeholder
        };
        mission->addObjective(objective);
    }
    
    return mission;
}

std::shared_ptr<Mission> MissionManager::createSurvivalMission(const std::string& name, float survivalTime) {
    auto mission = std::make_shared<Mission>(name, MissionType::SURVIVAL);
    
    mission->setTimeLimit(survivalTime);
    
    MissionObjective objective;
    objective.description = "Survive for " + std::to_string(static_cast<int>(survivalTime)) + " seconds";
    objective.completionCheck = []() {
        // Check survival time (time limit handles this)
        return false; // Placeholder
    };
    mission->addObjective(objective);
    
    return mission;
}

void MissionManager::loadCampaign(const std::string& campaignFile) {
    // This would load campaign data from file
    // For now, we'll create a default campaign
    
    m_campaignMissions.clear();
    m_campaignMissions.push_back("Deathmatch Basic");
    m_campaignMissions.push_back("Team Deathmatch");
    m_campaignMissions.push_back("Capture the Flag");
    m_campaignMissions.push_back("King of the Hill");
    
    Logger::info("Campaign loaded: " + std::to_string(m_campaignMissions.size()) + " missions");
}

void MissionManager::saveCampaign(const std::string& campaignFile) {
    // This would save campaign progress to file
    Logger::info("Campaign progress saved");
}

void MissionManager::startCampaign() {
    if (m_campaignMissions.empty()) {
        Logger::error("No missions in campaign");
        return;
    }
    
    m_inCampaign = true;
    m_campaignIndex = 0;
    startMission(m_campaignMissions[0]);
    
    Logger::info("Campaign started");
}

void MissionManager::setCampaignMissions(const std::vector<std::string>& missionNames) {
    m_campaignMissions = missionNames;
    m_campaignIndex = 0;
}

void MissionManager::onMissionCompleted(std::shared_ptr<Mission> mission) {
    if (!mission) return;
    
    m_missionsCompleted++;
    Logger::info("Mission completed: " + mission->getName());
    
    // Check campaign progression
    if (m_inCampaign) {
        if (m_campaignIndex < m_campaignMissions.size() - 1) {
            // Auto-advance to next mission
            nextMission();
        } else {
            // Campaign completed
            Logger::info("Campaign completed!");
            m_inCampaign = false;
        }
    }
}

void MissionManager::onMissionFailed(std::shared_ptr<Mission> mission) {
    if (!mission) return;
    
    m_missionsFailed++;
    Logger::info("Mission failed: " + mission->getName());
    
    // In campaign, offer retry
    if (m_inCampaign) {
        // Could show retry dialog here
        Logger::info("Campaign mission failed - retry available");
    }
}

void MissionManager::onVehicleDestroyed(std::shared_ptr<Vehicle> vehicle) {
    if (!vehicle) return;
    
    // Forward event to current mission
    if (m_currentMission) {
        m_currentMission->onVehicleDestroyed(vehicle);
    }
}

void MissionManager::onPlayerDeath() {
    // Forward event to current mission
    if (m_currentMission) {
        m_currentMission->onPlayerDeath();
    }
}

void MissionManager::updateCurrentMission(float deltaTime) {
    if (m_currentMission) {
        m_currentMission->update(deltaTime);
        
        // Check if mission state changed
        if (m_currentMission->getState() == MissionState::COMPLETED) {
            onMissionCompleted(m_currentMission);
        } else if (m_currentMission->getState() == MissionState::FAILED) {
            onMissionFailed(m_currentMission);
        }
    }
}

void MissionManager::checkMissionProgression() {
    // Check for automatic mission progression
    // This could handle difficulty scaling, dynamic mission selection, etc.
}

void MissionManager::resetWorld() {
    if (m_world) {
        // Clear all vehicles and projectiles
        // Reset world state
        Logger::debug("World reset for new mission");
    }
}

#include "ai/AIManager.hpp"
#include "ai/AIController.hpp"
#include "game/Vehicle.hpp"
#include "game/GameWorld.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/random.hpp>

AIManager::AIManager() = default;

AIManager::~AIManager() {
    shutdown();
}

bool AIManager::initialize(std::shared_ptr<GameWorld> world) {
    if (!world) {
        Logger::error("AIManager initialization failed: null world");
        return false;
    }
    
    m_world = world;
    
    // Setup default team relationships
    setTeamRelationship(0, 1, true); // Player vs Enemy
    setTeamRelationship(0, 2, false); // Player vs Ally
    setTeamRelationship(1, 2, true); // Enemy vs Ally
    
    Logger::info("AIManager initialized successfully");
    m_initialized = true;
    return true;
}

void AIManager::shutdown() {
    Logger::info("Shutting down AIManager");
    
    m_controllers.clear();
    m_vehicleToController.clear();
    m_teamRelationships.clear();
    m_respawnQueue.clear();
    
    m_world.reset();
    m_initialized = false;
}

void AIManager::update(float deltaTime) {
    if (!m_initialized) return;
    
    // Update all AI controllers
    for (auto& controller : m_controllers) {
        if (controller && controller->getVehicle() && !controller->getVehicle()->isDestroyed()) {
            controller->update(deltaTime);
        }
    }
    
    // Update team communication
    updateTeamCommunication(deltaTime);
    
    // Coordinate team attacks
    coordinateTeamAttacks();
    
    // Process respawn queue
    for (auto& respawn : m_respawnQueue) {
        respawn.second -= deltaTime;
    }
    
    // Remove respawned vehicles from queue
    m_respawnQueue.erase(
        std::remove_if(m_respawnQueue.begin(), m_respawnQueue.end(),
            [this](const auto& entry) { return entry.second <= 0.0f; }),
        m_respawnQueue.end()
    );
    
    // Cleanup destroyed controllers
    cleanupDestroyedControllers();
    
    // Balance teams if needed
    balanceTeams();
}

std::shared_ptr<AIController> AIManager::createAIController(std::shared_ptr<Vehicle> vehicle) {
    if (!vehicle || !m_world) {
        Logger::error("Failed to create AI controller: null vehicle or world");
        return nullptr;
    }
    
    auto controller = std::make_shared<AIController>();
    if (!controller->initialize(vehicle, m_world)) {
        Logger::error("Failed to initialize AI controller");
        return nullptr;
    }
    
    // Apply global difficulty settings
    float skill = 0.3f + m_globalDifficulty * 0.7f;
    controller->setSkill(skill);
    
    // Adjust aggressiveness based on difficulty
    float aggressiveness = 0.3f + m_globalDifficulty * 0.6f;
    controller->setAggressiveness(aggressiveness);
    
    m_controllers.push_back(controller);
    m_vehicleToController[vehicle.get()] = controller;
    
    Logger::info("AI controller created for vehicle: " + vehicle->getVehicleType());
    return controller;
}

void AIManager::removeAIController(std::shared_ptr<Vehicle> vehicle) {
    if (!vehicle) return;
    
    auto controllerIt = std::find_if(m_controllers.begin(), m_controllers.end(),
        [&vehicle](const std::shared_ptr<AIController>& controller) {
            return controller && controller->getVehicle() == vehicle;
        });
    
    if (controllerIt != m_controllers.end()) {
        (*controllerIt)->shutdown();
        m_controllers.erase(controllerIt);
        Logger::debug("AI controller removed for vehicle");
    }
}

void AIManager::removeAIController(std::shared_ptr<AIController> controller) {
    if (!controller) return;
    
    auto it = std::find(m_controllers.begin(), m_controllers.end(), controller);
    if (it != m_controllers.end()) {
        m_controllers.erase(it);
    }
    
    // Remove from vehicle mapping
    for (auto& pair : m_vehicleToController) {
        if (pair.second == controller) {
            m_vehicleToController.erase(pair.first);
            break;
        }
    }
    
    Logger::info("AI controller removed");
}

std::shared_ptr<AIController> AIManager::getAIController(std::shared_ptr<Vehicle> vehicle) const {
    auto it = m_vehicleToController.find(vehicle.get());
    if (it != m_vehicleToController.end()) {
        return it->second;
    }
    return nullptr;
}

void AIManager::setTeamRelationship(int team1, int team2, bool isEnemy) {
    m_teamRelationships[team1][team2] = isEnemy;
    m_teamRelationships[team2][team1] = isEnemy; // Symmetric relationship
    
    Logger::debug("Team relationship set: Team " + std::to_string(team1) + 
                " vs Team " + std::to_string(team2) + " = " + 
                (isEnemy ? "Enemy" : "Ally"));
}

bool AIManager::areTeamsEnemies(int team1, int team2) const {
    auto it1 = m_teamRelationships.find(team1);
    if (it1 != m_teamRelationships.end()) {
        auto it2 = it1->second.find(team2);
        if (it2 != it1->second.end()) {
            return it2->second;
        }
    }
    
    // Default to enemy if no relationship defined
    return team1 != team2;
}

void AIManager::clearTeamRelationships() {
    m_teamRelationships.clear();
    Logger::info("All team relationships cleared");
}

void AIManager::setGlobalDifficulty(float difficulty) {
    m_globalDifficulty = glm::clamp(difficulty, 0.0f, 1.0f);
    
    // Update existing controllers
    for (auto& controller : m_controllers) {
        if (controller) {
            float skill = 0.3f + m_globalDifficulty * 0.7f;
            controller->setSkill(skill);
            
            float aggressiveness = 0.3f + m_globalDifficulty * 0.6f;
            controller->setAggressiveness(aggressiveness);
        }
    }
    
    Logger::info("Global AI difficulty set to: " + std::to_string(m_globalDifficulty));
}

std::shared_ptr<Vehicle> AIManager::spawnEnemyVehicle(const glm::vec3& position, int team) {
    if (!m_world) return nullptr;
    
    auto vehicle = m_world->spawnVehicle(position, "enemy");
    if (vehicle) {
        vehicle->setTeam(team);
        vehicle->setOwner(-1); // AI owned
        
        // Create AI controller
        auto controller = createAIController(vehicle);
        if (controller) {
            controller->setTeam(team);
            controller->setBehavior(AIBehavior::AGGRESSIVE);
        }
        
        Logger::info("Enemy vehicle spawned at team " + std::to_string(team));
    }
    
    return vehicle;
}

std::shared_ptr<Vehicle> AIManager::spawnAllyVehicle(const glm::vec3& position, int team) {
    if (!m_world) return nullptr;
    
    auto vehicle = m_world->spawnVehicle(position, "ally");
    if (vehicle) {
        vehicle->setTeam(team);
        vehicle->setOwner(-1); // AI owned
        
        // Create AI controller
        auto controller = createAIController(vehicle);
        if (controller) {
            controller->setTeam(team);
            controller->setBehavior(AIBehavior::CAUTIOUS);
        }
        
        Logger::info("Ally vehicle spawned at team " + std::to_string(team));
    }
    
    return vehicle;
}

void AIManager::spawnWave(int enemyCount, int allyCount) {
    if (!m_world) return;
    
    Logger::info("Spawning wave: " + std::to_string(enemyCount) + " enemies, " + 
                std::to_string(allyCount) + " allies");
    
    // Spawn enemies
    for (int i = 0; i < enemyCount; ++i) {
        glm::vec3 position = glm::linearRand(
            glm::vec3(-200.0f, 10.0f, -200.0f),
            glm::vec3(200.0f, 10.0f, 200.0f)
        );
        
        // Adjust position to terrain height
        position.y = 5.0f; // Placeholder for terrain height
        
        spawnEnemyVehicle(position, 1);
    }
    
    // Spawn allies
    for (int i = 0; i < allyCount; ++i) {
        glm::vec3 position = glm::linearRand(
            glm::vec3(-150.0f, 10.0f, -150.0f),
            glm::vec3(150.0f, 10.0f, 150.0f)
        );
        
        // Adjust position to terrain height
        position.y = 5.0f; // Placeholder for terrain height
        
        spawnAllyVehicle(position, 2);
    }
}

void AIManager::onVehicleDestroyed(std::shared_ptr<Vehicle> vehicle) {
    if (!vehicle) return;
    
    Logger::info("Vehicle destroyed: " + vehicle->getVehicleType() + 
                " (Team " + std::to_string(vehicle->getTeam()) + ")");
    
    // Remove AI controller
    removeAIController(vehicle);
    
    // Add to respawn queue
    m_respawnQueue.push_back({vehicle, m_respawnTime});
    
    // Notify other AI controllers
    for (auto& controller : m_controllers) {
        if (controller && controller->getTarget() == vehicle) {
            controller->clearTarget();
        }
    }
}

void AIManager::onVehicleDamaged(std::shared_ptr<Vehicle> vehicle, float damage) {
    if (!vehicle) return;
    
    // Notify allied AI controllers about the attack
    for (auto& controller : m_controllers) {
        if (controller && controller->getVehicle() != vehicle) {
            int controllerTeam = controller->getTeam();
            int vehicleTeam = vehicle->getTeam();
            
            if (!areTeamsEnemies(controllerTeam, vehicleTeam)) {
                // Ally is under attack, provide support
                // This could trigger defensive behaviors
            }
        }
    }
}

void AIManager::updateTeamCommunication(float deltaTime) {
    // Share target information between allied AI controllers
    for (auto& controller1 : m_controllers) {
        if (!controller1 || !controller1->getTarget()) continue;
        
        auto target = controller1->getTarget();
        int team1 = controller1->getTeam();
        
        for (auto& controller2 : m_controllers) {
            if (!controller2 || controller2 == controller1) continue;
            
            int team2 = controller2->getTeam();
            
            if (!areTeamsEnemies(team1, team2) && !controller2->getTarget()) {
                // Share target with ally
                float distance = glm::length(
                    controller1->getVehicle()->getPosition() - 
                    controller2->getVehicle()->getPosition()
                );
                
                if (distance < 100.0f) { // Communication range
                    controller2->setTarget(target);
                }
            }
        }
    }
}

void AIManager::coordinateTeamAttacks() {
    // Coordinate attacks for better tactical positioning
    std::unordered_map<int, std::vector<std::shared_ptr<AIController>>> teamGroups;
    
    // Group controllers by team
    for (auto& controller : m_controllers) {
        if (controller && controller->getTarget()) {
            teamGroups[controller->getTeam()].push_back(controller);
        }
    }
    
    // Coordinate attacks within each team
    for (auto& teamPair : teamGroups) {
        auto& teamControllers = teamPair.second;
        
        if (teamControllers.size() > 1) {
            // Simple coordination: spread out around target
            for (size_t i = 0; i < teamControllers.size(); ++i) {
                float angle = (static_cast<float>(i) / teamControllers.size()) * 2.0f * 3.14159f; // glm::pi<float>()
                // This would set preferred attack positions
            }
        }
    }
}

void AIManager::balanceTeams() {
    if (!m_world) return;
    
    // Count vehicles per team
    std::unordered_map<int, int> teamCounts;
    const auto& vehicles = m_world->getVehicles();
    
    for (const auto& vehicle : vehicles) {
        if (vehicle && !vehicle->isDestroyed()) {
            teamCounts[vehicle->getTeam()]++;
        }
    }
    
    // Check if teams are unbalanced
    int playerCount = teamCounts[0];
    int enemyCount = teamCounts[1];
    int allyCount = teamCounts[2];
    
    // Spawn additional vehicles if needed
    if (enemyCount < playerCount + 1) {
        spawnEnemyVehicle(glm::vec3(100.0f, 10.0f, 0.0f), 1);
    }
    
    if (allyCount < playerCount / 2) {
        spawnAllyVehicle(glm::vec3(-100.0f, 10.0f, 0.0f), 2);
    }
}

void AIManager::cleanupDestroyedControllers() {
    // Remove controllers for destroyed vehicles
    auto it = m_controllers.begin();
    while (it != m_controllers.end()) {
        if (!(*it) || !(*it)->getVehicle() || (*it)->getVehicle()->isDestroyed()) {
            it = m_controllers.erase(it);
        } else {
            ++it;
        }
    }
    
    // Clean up vehicle mapping
    auto mapIt = m_vehicleToController.begin();
    while (mapIt != m_vehicleToController.end()) {
        if (!mapIt->second || !mapIt->first || 
            (mapIt->first && mapIt->first->isDestroyed())) {
            mapIt = m_vehicleToController.erase(mapIt);
        } else {
            ++mapIt;
        }
    }
}

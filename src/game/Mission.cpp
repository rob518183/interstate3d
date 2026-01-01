#include "game/Mission.hpp"
#include "game/GameWorld.hpp"
#include "ai/AIManager.hpp"
#include "game/Vehicle.hpp"
#include "core/Logger.hpp"

Mission::Mission(const std::string& name, MissionType type)
    : m_name(name), m_type(type) {
}

bool Mission::initialize(std::shared_ptr<GameWorld> world, std::shared_ptr<AIManager> aiManager) {
    if (!world || !aiManager) {
        Logger::error("Mission initialization failed: null world or AI manager");
        return false;
    }
    
    m_world = world;
    m_aiManager = aiManager;
    
    Logger::info("Initializing mission: " + m_name);
    
    // Setup environment
    setupEnvironment();
    
    // Spawn vehicles
    spawnVehicles();
    
    // Spawn world objects
    spawnWorldObjects();
    
    // Set time remaining
    m_timeRemaining = m_timeLimit;
    
    m_initialized = true;
    Logger::info("Mission initialized successfully: " + m_name);
    return true;
}

void Mission::shutdown() {
    Logger::info("Shutting down mission: " + m_name);
    
    m_world.reset();
    m_aiManager.reset();
    
    m_initialized = false;
}

void Mission::update(float deltaTime) {
    if (!m_initialized) return;
    
    if (m_state == MissionState::IN_PROGRESS) {
        updateMission(deltaTime);
        updateTimer(deltaTime);
        checkObjectives();
    }
}

void Mission::render() {
    // Render mission-specific UI elements
    // This would render objectives, timer, etc.
}

void Mission::start() {
    if (m_state == MissionState::NOT_STARTED) {
        m_state = MissionState::IN_PROGRESS;
        m_elapsedTime = 0.0f;
        m_timeRemaining = m_timeLimit;
        
        Logger::info("Mission started: " + m_name);
    }
}

void Mission::pause() {
    if (m_state == MissionState::IN_PROGRESS) {
        m_state = MissionState::PAUSED;
        Logger::info("Mission paused: " + m_name);
    }
}

void Mission::resume() {
    if (m_state == MissionState::PAUSED) {
        m_state = MissionState::IN_PROGRESS;
        Logger::info("Mission resumed: " + m_name);
    }
}

void Mission::complete() {
    if (m_state == MissionState::IN_PROGRESS) {
        m_state = MissionState::COMPLETED;
        Logger::info("Mission completed: " + m_name);
        
        // Apply rewards
        // This would be handled by a game manager or player system
    }
}

void Mission::fail(const std::string& reason) {
    if (m_state == MissionState::IN_PROGRESS) {
        m_state = MissionState::FAILED;
        Logger::info("Mission failed: " + m_name + " - " + reason);
    }
}

void Mission::addObjective(const MissionObjective& objective) {
    m_objectives.push_back(objective);
}

void Mission::completeObjective(int index) {
    if (index >= 0 && index < m_objectives.size()) {
        m_objectives[index].completed = true;
        Logger::info("Objective completed: " + m_objectives[index].description);
    }
}

bool Mission::areAllObjectivesCompleted() const {
    for (const auto& objective : m_objectives) {
        if (!objective.completed) {
            return false;
        }
    }
    return true;
}

bool Mission::areRequiredObjectivesCompleted() const {
    for (const auto& objective : m_objectives) {
        if (!objective.optional && !objective.completed) {
            return false;
        }
    }
    return true;
}

void Mission::spawnVehicles() {
    // Default vehicle spawning - can be overridden by specific missions
    if (m_aiManager) {
        m_aiManager->spawnWave(3, 1); // 3 enemies, 1 ally
    }
}

void Mission::spawnWorldObjects() {
    // Default world object spawning
    // This would spawn mission-specific objects like flags, capture points, etc.
}

void Mission::setupEnvironment() {
    // Default environment setup
    // This would set time of day, weather, etc.
}

void Mission::onVehicleDestroyed(std::shared_ptr<Vehicle> vehicle) {
    if (!vehicle) return;
    
    Logger::debug("Vehicle destroyed in mission: " + vehicle->getVehicleType());
    
    // Check if this affects mission objectives
    checkObjectives();
}

void Mission::onPlayerDeath() {
    Logger::info("Player died in mission: " + m_name);
    
    // Most missions fail on player death unless specified otherwise
    fail("Player vehicle destroyed");
}

void Mission::onTimeExpired() {
    Logger::info("Time expired in mission: " + m_name);
    
    // Check if time limit was critical
    bool timeWasCritical = false;
    for (const auto& objective : m_objectives) {
        if (objective.timeLimit > 0.0f && !objective.completed) {
            timeWasCritical = true;
            break;
        }
    }
    
    if (timeWasCritical) {
        fail("Time limit exceeded");
    }
}

void Mission::updateMission(float deltaTime) {
    m_elapsedTime += deltaTime;
    
    // Mission-specific update logic
    // This would be implemented by derived mission classes
}

void Mission::checkObjectives() {
    for (size_t i = 0; i < m_objectives.size(); ++i) {
        auto& objective = m_objectives[i];
        
        if (!objective.completed && objective.completionCheck) {
            if (objective.completionCheck()) {
                completeObjective(static_cast<int>(i));
            }
        }
    }
    
    // Check if mission should complete
    if (m_state == MissionState::IN_PROGRESS && areRequiredObjectivesCompleted()) {
        complete();
    }
}

void Mission::updateTimer(float deltaTime) {
    if (m_timeLimit > 0.0f && m_timeRemaining > 0.0f) {
        m_timeRemaining -= deltaTime;
        
        if (m_timeRemaining <= 0.0f) {
            m_timeRemaining = 0.0f;
            onTimeExpired();
        }
    }
}

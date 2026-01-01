#include "game/ParticleManager.hpp"
#include "game/ParticleSystem.hpp"
#include "core/Logger.hpp"
#include <algorithm>

ParticleManager::ParticleManager() = default;

ParticleManager::~ParticleManager() {
    shutdown();
}

bool ParticleManager::initialize() {
    Logger::info("Initializing Particle Manager");
    
    m_initialized = true;
    Logger::info("Particle Manager initialized successfully");
    return true;
}

void ParticleManager::shutdown() {
    Logger::info("Shutting down Particle Manager");
    
    clearAllSystems();
    m_initialized = false;
}

void ParticleManager::update(float deltaTime) {
    if (!m_initialized) return;
    
    // Update all particle systems
    for (auto& system : m_systems) {
        if (system) {
            system->update(deltaTime);
        }
    }
    
    // Remove inactive systems
    m_systems.erase(
        std::remove_if(m_systems.begin(), m_systems.end(),
            [](const std::shared_ptr<ParticleSystem>& system) {
                return !system || system->isComplete();
            }),
        m_systems.end()
    );
}

void ParticleManager::addParticleSystem(std::shared_ptr<ParticleSystem> system) {
    if (system) {
        m_systems.push_back(system);
        Logger::debug("Particle system added to manager");
    }
}

void ParticleManager::removeParticleSystem(std::shared_ptr<ParticleSystem> system) {
    auto it = std::find(m_systems.begin(), m_systems.end(), system);
    if (it != m_systems.end()) {
        m_systems.erase(it);
        Logger::debug("Particle system removed from manager");
    }
}

void ParticleManager::clearAllSystems() {
    m_systems.clear();
    Logger::debug("All particle systems cleared");
}

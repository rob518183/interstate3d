#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

class ParticleSystem;
class GameWorld;

class ParticleManager {
public:
    ParticleManager();
    ~ParticleManager();
    
    bool initialize();
    void shutdown();
    void update(float deltaTime);
    
    // Particle system management
    void addParticleSystem(std::shared_ptr<ParticleSystem> system);
    void removeParticleSystem(std::shared_ptr<ParticleSystem> system);
    void clearAllSystems();
    
    // Getters
    const std::vector<std::shared_ptr<ParticleSystem>>& getSystems() const { return m_systems; }
    size_t getSystemCount() const { return m_systems.size(); }
    
private:
    std::vector<std::shared_ptr<ParticleSystem>> m_systems;
    bool m_initialized = false;
};

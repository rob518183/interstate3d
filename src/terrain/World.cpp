#include "terrain/World.hpp"
#include "terrain/Terrain.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Light.hpp"
#include "rendering/Mesh.hpp"
#include "core/Logger.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <numbers>

// Define pi constant if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

World::World() = default;

World::~World() {
    shutdown();
}

bool World::initialize(float worldSize) {
    Logger::info("Initializing world with size: " + std::to_string(worldSize));
    
    m_worldSize = worldSize;
    
    // Create terrain
    m_terrain = std::make_shared<Terrain>();
    int terrainResolution = static_cast<int>(worldSize / 4.0f); // 4 units per vertex
    terrainResolution = glm::clamp(terrainResolution, 64, 512);
    
    if (!m_terrain->initialize(terrainResolution, terrainResolution, 4.0f)) {
        Logger::error("Failed to initialize terrain");
        return false;
    }
    
    // Generate terrain
    generateTerrain();
    
    // Setup lighting
    setupLighting();
    
    // Generate world objects
    generateVegetation();
    generateStructures();
    
    m_initialized = true;
    Logger::info("World initialized successfully");
    return true;
}

void World::shutdown() {
    Logger::info("Shutting down world...");
    
    clearObjects();
    m_terrain.reset();
    m_lights.clear();
    
    m_initialized = false;
}

void World::generateTerrain(int resolution) {
    Logger::info("Generating terrain with resolution: " + std::to_string(resolution));
    
    // Set random seed
    std::srand(m_terrainSeed);
    
    // Generate terrain using noise
    m_terrain->generateFromNoise(0.02f, 1.0f, m_terrainOctaves, m_terrainPersistence);
    
    Logger::info("Terrain generation complete");
}

void World::setTerrainSeed(unsigned int seed) {
    m_terrainSeed = seed;
    if (m_terrain) {
        generateTerrain();
    }
}

void World::setTerrainParameters(float minHeight, float maxHeight, int octaves, float persistence) {
    m_minTerrainHeight = minHeight;
    m_maxTerrainHeight = maxHeight;
    m_terrainOctaves = octaves;
    m_terrainPersistence = persistence;
    
    if (m_terrain) {
        generateTerrain();
    }
}

void World::setupLighting() {
    Logger::info("Setting up world lighting");
    
    m_lights.clear();
    
    // Main sun light (directional)
    auto sunLight = std::make_shared<Light>();
    sunLight->setType(Light::Type::DIRECTIONAL);
    sunLight->setDirection(glm::normalize(m_sunDirection));
    sunLight->setColor(m_sunColor);
    sunLight->setIntensity(1.5f);
    m_lights.push_back(sunLight);
    
    // Ambient light is handled in the shader
}

void World::setSkyColor(const glm::vec3& color) {
    m_skyColor = color;
}

void World::setSunDirection(const glm::vec3& direction) {
    m_sunDirection = glm::normalize(direction);
    if (!m_lights.empty()) {
        m_lights[0]->setDirection(m_sunDirection);
    }
}

void World::setSunColor(const glm::vec3& color) {
    m_sunColor = color;
    if (!m_lights.empty()) {
        m_lights[0]->setColor(color);
    }
}

void World::setAmbientColor(const glm::vec3& color) {
    m_ambientColor = color;
}

float World::getTerrainHeight(float x, float z) const {
    if (m_terrain) {
        return m_terrain->getHeightAt(x, z);
    }
    return 0.0f;
}

glm::vec3 World::getTerrainNormal(float x, float z) const {
    if (m_terrain) {
        return m_terrain->getNormalAt(x, z);
    }
    return glm::vec3(0.0f, 1.0f, 0.0f);
}

bool World::isPointOnTerrain(float x, float z) const {
    if (m_terrain) {
        return m_terrain->isPointOnTerrain(x, z);
    }
    return false;
}

void World::renderTerrain(Renderer* renderer) {
    if (!renderer || !m_terrain) {
        return;
    }
    
    // Add lights to renderer
    for (const auto& light : m_lights) {
        renderer->addLight(light);
    }
    
    // Render terrain
    m_terrain->render(nullptr); // Shader will be set by renderer
}

void World::renderSkybox(Renderer* renderer) {
    if (!renderer) {
        return;
    }
    
    // Set sky color as clear color
    renderer->setClearColor(m_skyColor);
    
    // Render skybox would go here
    // For now, we just set the clear color
}

void World::addTree(const glm::vec3& position) {
    // Create a simple tree mesh
    auto tree = std::make_shared<Mesh>();
    // This would load or generate a tree model
    
    m_trees.push_back(tree);
    
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
    m_treeTransforms.push_back(transform);
    
    Logger::debug("Tree added at position: " + std::to_string(position.x) + ", " + 
                std::to_string(position.y) + ", " + std::to_string(position.z));
}

void World::addRock(const glm::vec3& position) {
    // Create a simple rock mesh
    auto rock = std::make_shared<Mesh>();
    // This would load or generate a rock model
    
    m_rocks.push_back(rock);
    
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
    m_rockTransforms.push_back(transform);
    
    Logger::debug("Rock added at position: " + std::to_string(position.x) + ", " + 
                std::to_string(position.y) + ", " + std::to_string(position.z));
}

void World::addBuilding(const glm::vec3& position) {
    // Create a simple building mesh
    auto building = std::make_shared<Mesh>();
    // This would load or generate a building model
    
    m_buildings.push_back(building);
    
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
    m_buildingTransforms.push_back(transform);
    
    Logger::debug("Building added at position: " + std::to_string(position.x) + ", " + 
                std::to_string(position.y) + ", " + std::to_string(position.z));
}

void World::clearObjects() {
    m_trees.clear();
    m_rocks.clear();
    m_buildings.clear();
    m_treeTransforms.clear();
    m_rockTransforms.clear();
    m_buildingTransforms.clear();
}

void World::setTimeOfDay(float hours) {
    m_timeOfDay = glm::mod(hours, 24.0f);
    updateLighting();
}

void World::updateTime(float deltaTime) {
    // Update time of day
    float timeIncrement = (24.0f / m_dayDuration) * deltaTime;
    m_timeOfDay += timeIncrement;
    m_timeOfDay = glm::mod(m_timeOfDay, 24.0f);
    
    updateLighting();
}

void World::generateVegetation() {
    Logger::info("Generating vegetation");
    
    std::random_device rd;
    std::mt19937 gen(m_terrainSeed);
    std::uniform_real_distribution<float> xDist(-m_worldSize * 0.4f, m_worldSize * 0.4f);
    std::uniform_real_distribution<float> zDist(-m_worldSize * 0.4f, m_worldSize * 0.4f);
    std::uniform_real_distribution<float> densityDist(0.0f, 1.0f);
    
    // Generate trees
    int numTrees = static_cast<int>(m_worldSize * 0.1f); // Density based on world size
    for (int i = 0; i < numTrees; ++i) {
        float x = xDist(gen);
        float z = zDist(gen);
        float y = getTerrainHeight(x, z);
        
        // Only place trees on suitable terrain
        if (y > 5.0f && y < 40.0f && densityDist(gen) > 0.3f) {
            addTree(glm::vec3(x, y, z));
        }
    }
    
    // Generate rocks
    int numRocks = static_cast<int>(m_worldSize * 0.05f);
    for (int i = 0; i < numRocks; ++i) {
        float x = xDist(gen);
        float z = zDist(gen);
        float y = getTerrainHeight(x, z);
        
        // Place rocks on various terrain
        if (y > 0.0f && densityDist(gen) > 0.5f) {
            addRock(glm::vec3(x, y, z));
        }
    }
    
    Logger::info("Vegetation generation complete: " + std::to_string(m_trees.size()) + 
                " trees, " + std::to_string(m_rocks.size()) + " rocks");
}

void World::generateStructures() {
    Logger::info("Generating structures");
    
    std::random_device rd;
    std::mt19937 gen(m_terrainSeed + 1000);
    std::uniform_real_distribution<float> xDist(-m_worldSize * 0.3f, m_worldSize * 0.3f);
    std::uniform_real_distribution<float> zDist(-m_worldSize * 0.3f, m_worldSize * 0.3f);
    
    // Generate some buildings
    int numBuildings = static_cast<int>(m_worldSize * 0.02f);
    for (int i = 0; i < numBuildings; ++i) {
        float x = xDist(gen);
        float z = zDist(gen);
        float y = getTerrainHeight(x, z);
        
        // Place buildings on relatively flat terrain
        if (y > 0.0f && y < 30.0f) {
            addBuilding(glm::vec3(x, y, z));
        }
    }
    
    Logger::info("Structure generation complete: " + std::to_string(m_buildings.size()) + " buildings");
}

void World::updateLighting() {
    // Calculate sun position based on time of day
    float sunAngle = (m_timeOfDay / 24.0f) * 2.0f * M_PI - M_PI * 0.5f;
    
    glm::vec3 sunDirection(
        glm::cos(sunAngle),
        glm::sin(sunAngle),
        0.3f
    );
    
    setSunDirection(sunDirection);
    
    // Adjust sun color and intensity based on time
    glm::vec3 sunColor = m_sunColor;
    float sunIntensity = 1.5f;
    
    if (m_timeOfDay < 6.0f || m_timeOfDay > 20.0f) {
        // Night time
        sunColor = glm::vec3(0.2f, 0.2f, 0.4f);
        sunIntensity = 0.3f;
    } else if (m_timeOfDay < 8.0f || m_timeOfDay > 18.0f) {
        // Dawn/Dusk
        sunColor = glm::vec3(1.0f, 0.6f, 0.3f);
        sunIntensity = 0.8f;
    }
    
    setSunColor(sunColor);
    
    if (!m_lights.empty()) {
        m_lights[0]->setIntensity(sunIntensity);
    }
    
    // Update sky color based on time
    glm::vec3 skyColor = m_skyColor;
    if (m_timeOfDay < 6.0f || m_timeOfDay > 20.0f) {
        skyColor = glm::vec3(0.1f, 0.1f, 0.2f); // Night sky
    } else if (m_timeOfDay < 8.0f || m_timeOfDay > 18.0f) {
        skyColor = glm::vec3(0.8f, 0.4f, 0.2f); // Dawn/Dusk sky
    } else {
        skyColor = glm::vec3(0.5f, 0.7f, 1.0f); // Day sky
    }
    
    setSkyColor(skyColor);
}

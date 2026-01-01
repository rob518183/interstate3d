#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

class Terrain;
class Mesh;
class Texture;
class Light;

class World {
public:
    World();
    ~World();
    
    bool initialize(float worldSize = 1000.0f);
    void shutdown();
    
    // Terrain management
    void generateTerrain(int resolution = 256);
    void setTerrainSeed(unsigned int seed);
    void setTerrainParameters(float minHeight, float maxHeight, int octaves = 4, float persistence = 0.5f);
    
    // Environment
    void setupLighting();
    void setSkyColor(const glm::vec3& color);
    void setSunDirection(const glm::vec3& direction);
    void setSunColor(const glm::vec3& color);
    void setAmbientColor(const glm::vec3& color);
    
    // World properties
    void setWorldSize(float size) { m_worldSize = size; }
    float getWorldSize() const { return m_worldSize; }
    void setGravity(const glm::vec3& gravity) { m_gravity = gravity; }
    const glm::vec3& getGravity() const { return m_gravity; }
    
    // Height queries
    float getTerrainHeight(float x, float z) const;
    glm::vec3 getTerrainNormal(float x, float z) const;
    bool isPointOnTerrain(float x, float z) const;
    
    // Rendering
    void renderTerrain(class Renderer* renderer);
    void renderSkybox(class Renderer* renderer);
    
    // Environment objects
    void addTree(const glm::vec3& position);
    void addRock(const glm::vec3& position);
    void addBuilding(const glm::vec3& position);
    void clearObjects();
    
    // Getters
    std::shared_ptr<Terrain> getTerrain() const { return m_terrain; }
    const std::vector<std::shared_ptr<Light>>& getLights() const { return m_lights; }
    
    // Time of day
    void setTimeOfDay(float hours); // 0.0 to 24.0
    float getTimeOfDay() const { return m_timeOfDay; }
    void updateTime(float deltaTime);
    
private:
    void generateVegetation();
    void generateStructures();
    void updateLighting();
    
    std::shared_ptr<Terrain> m_terrain;
    std::vector<std::shared_ptr<Light>> m_lights;
    
    // Environment objects
    std::vector<std::shared_ptr<Mesh>> m_trees;
    std::vector<std::shared_ptr<Mesh>> m_rocks;
    std::vector<std::shared_ptr<Mesh>> m_buildings;
    std::vector<glm::mat4> m_treeTransforms;
    std::vector<glm::mat4> m_rockTransforms;
    std::vector<glm::mat4> m_buildingTransforms;
    
    // World properties
    float m_worldSize = 1000.0f;
    glm::vec3 m_gravity = glm::vec3(0.0f, -9.81f, 0.0f);
    
    // Environment
    glm::vec3 m_skyColor = glm::vec3(0.5f, 0.7f, 1.0f);
    glm::vec3 m_sunDirection = glm::vec3(0.5f, -0.8f, 0.3f);
    glm::vec3 m_sunColor = glm::vec3(1.0f, 0.9f, 0.7f);
    glm::vec3 m_ambientColor = glm::vec3(0.2f, 0.2f, 0.3f);
    
    // Time of day
    float m_timeOfDay = 12.0f; // Noon
    float m_dayDuration = 300.0f; // 5 minutes per day
    
    // Terrain parameters
    unsigned int m_terrainSeed = 12345;
    float m_minTerrainHeight = -20.0f;
    float m_maxTerrainHeight = 50.0f;
    int m_terrainOctaves = 4;
    float m_terrainPersistence = 0.5f;
    
    bool m_initialized = false;
};

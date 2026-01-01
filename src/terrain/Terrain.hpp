#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

class Mesh;
class Texture;
class Shader;

class Terrain {
public:
    Terrain();
    ~Terrain();
    
    bool initialize(int width, int height, float scale = 1.0f);
    void shutdown();
    
    // Height map generation
    void generateFlatTerrain(float height = 0.0f);
    void generateRandomTerrain(float minHeight, float maxHeight, int octaves = 4, float persistence = 0.5f);
    void generateFromHeightMap(const std::string& heightMapFile);
    void generateFromNoise(float frequency, float amplitude, int octaves = 4, float persistence = 0.5f);
    
    // Terrain modification
    void setHeight(int x, int y, float height);
    float getHeight(int x, int y) const;
    float getHeightAt(float worldX, float worldZ) const;
    void setHeightAt(float worldX, float worldZ, float height);
    
    // Terrain properties
    void setScale(float scale) { m_scale = scale; }
    float getScale() const { return m_scale; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    // Rendering
    void render(Shader* shader);
    void setTexture(const std::shared_ptr<Texture>& texture);
    void setMaterialProperties(const glm::vec3& color, float roughness = 0.8f, float metallic = 0.0f);
    
    // Physics
    std::shared_ptr<class CollisionShape> createCollisionShape() const;
    
    // Utility
    glm::vec3 getNormalAt(float worldX, float worldZ) const;
    bool isPointOnTerrain(float worldX, float worldZ) const;
    
private:
    void generateMesh();
    void calculateNormals();
    void calculateTangents();
    
    // Terrain data
    std::vector<float> m_heightMap;
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_texCoords;
    std::vector<glm::vec3> m_tangents;
    std::vector<glm::vec3> m_bitangents;
    std::vector<unsigned int> m_indices;
    
    // Terrain properties
    int m_width;
    int m_height;
    float m_scale;
    
    // Rendering
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Texture> m_texture;
    glm::vec3 m_materialColor = glm::vec3(0.5f, 0.8f, 0.3f);
    float m_materialRoughness = 0.8f;
    float m_materialMetallic = 0.0f;
    
    // Noise generation
    float perlinNoise(float x, float y, float frequency, float amplitude, int octaves, float persistence) const;
    float fade(float t) const;
    float lerp(float a, float b, float t) const;
    float grad(int hash, float x, float y) const;
    
    bool m_initialized = false;
};

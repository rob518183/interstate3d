#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include "core/AssetManager.hpp"

class Texture;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

class Mesh : public Asset {
public:
    Mesh(); // Default constructor for AssetManager
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, 
         const std::vector<std::shared_ptr<Texture>>& textures);
    ~Mesh();
    
    bool load(const std::string& path) override;
    void reload() override;
    AssetType getType() const override { return AssetType::Mesh; }
    
    void render() const;
    
    // Load mesh from raw data (for voxel terrain)
    bool loadFromData(const std::vector<float>& vertices, const std::vector<unsigned int>& indices,
                     const std::vector<float>& normals, const std::vector<float>& texCoords);
    
    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<unsigned int>& getIndices() const { return m_indices; }
    const std::vector<std::shared_ptr<Texture>>& getTextures() const { return m_textures; }
    
    void setMaterialProperties(const glm::vec3& color, float metallic, float roughness);
    
private:
    void setupMesh();
    
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<std::shared_ptr<Texture>> m_textures;
    
    // OpenGL buffers
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;
    
    // Material properties
    glm::vec3 m_materialColor = glm::vec3(1.0f);
    float m_metallic = 0.0f;
    float m_roughness = 0.5f;
};

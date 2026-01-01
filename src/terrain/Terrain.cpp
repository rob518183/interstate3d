#include "terrain/Terrain.hpp"
#include "rendering/Mesh.hpp"
#include "rendering/Texture.hpp"
#include "rendering/Shader.hpp"
#include "physics/CollisionShape.hpp"
#include "core/Logger.hpp"
#include <glm/gtc/noise.hpp>
#include <algorithm>
#include <random>

Terrain::Terrain() = default;

Terrain::~Terrain() {
    shutdown();
}

bool Terrain::initialize(int width, int height, float scale) {
    Logger::info("Initializing terrain: " + std::to_string(width) + "x" + std::to_string(height));
    
    m_width = width;
    m_height = height;
    m_scale = scale;
    
    // Resize height map
    m_heightMap.resize(width * height, 0.0f);
    
    m_initialized = true;
    Logger::info("Terrain initialized successfully");
    return true;
}

void Terrain::shutdown() {
    Logger::info("Shutting down terrain...");
    
    m_heightMap.clear();
    m_vertices.clear();
    m_normals.clear();
    m_texCoords.clear();
    m_tangents.clear();
    m_bitangents.clear();
    m_indices.clear();
    
    m_mesh.reset();
    m_texture.reset();
    
    m_initialized = false;
}

void Terrain::generateFlatTerrain(float height) {
    Logger::info("Generating flat terrain with height: " + std::to_string(height));
    
    std::fill(m_heightMap.begin(), m_heightMap.end(), height);
    generateMesh();
}

void Terrain::generateRandomTerrain(float minHeight, float maxHeight, int octaves, float persistence) {
    Logger::info("Generating random terrain");
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(minHeight, maxHeight);
    
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            float nx = static_cast<float>(x) / m_width;
            float ny = static_cast<float>(y) / m_height;
            
            float height = perlinNoise(nx, ny, 0.02f, 1.0f, octaves, persistence);
            height = (height + 1.0f) * 0.5f; // Normalize to [0, 1]
            height = minHeight + height * (maxHeight - minHeight);
            
            m_heightMap[y * m_width + x] = height;
        }
    }
    
    generateMesh();
}

void Terrain::generateFromHeightMap(const std::string& heightMapFile) {
    Logger::info("Generating terrain from height map: " + heightMapFile);
    
    // This would load an image file and use pixel values as heights
    // For now, we'll generate a simple pattern
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            float height = glm::sin(x * 0.1f) * glm::cos(y * 0.1f) * 5.0f;
            m_heightMap[y * m_width + x] = height;
        }
    }
    
    generateMesh();
}

void Terrain::generateFromNoise(float frequency, float amplitude, int octaves, float persistence) {
    Logger::info("Generating terrain from noise");
    
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            float nx = static_cast<float>(x) / m_width;
            float ny = static_cast<float>(y) / m_height;
            
            float height = perlinNoise(nx, ny, frequency, amplitude, octaves, persistence);
            m_heightMap[y * m_width + x] = height;
        }
    }
    
    generateMesh();
}

void Terrain::setHeight(int x, int y, float height) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_heightMap[y * m_width + x] = height;
        generateMesh(); // Regenerate mesh with new height
    }
}

float Terrain::getHeight(int x, int y) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        return m_heightMap[y * m_width + x];
    }
    return 0.0f;
}

float Terrain::getHeightAt(float worldX, float worldZ) const {
    // Convert world coordinates to terrain coordinates
    float x = worldX / m_scale;
    float z = worldZ / m_scale;
    
    // Get integer coordinates
    int ix = static_cast<int>(x);
    int iz = static_cast<int>(z);
    
    // Check bounds
    if (ix < 0 || ix >= m_width - 1 || iz < 0 || iz >= m_height - 1) {
        return 0.0f;
    }
    
    // Get fractional parts for interpolation
    float fx = x - ix;
    float fz = z - iz;
    
    // Get heights of four surrounding points
    float h00 = getHeight(ix, iz);
    float h10 = getHeight(ix + 1, iz);
    float h01 = getHeight(ix, iz + 1);
    float h11 = getHeight(ix + 1, iz + 1);
    
    // Bilinear interpolation
    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;
    float height = h0 * (1.0f - fz) + h1 * fz;
    
    return height;
}

void Terrain::setHeightAt(float worldX, float worldZ, float height) {
    float x = worldX / m_scale;
    float z = worldZ / m_scale;
    
    int ix = static_cast<int>(x);
    int iz = static_cast<int>(z);
    
    setHeight(ix, iz, height);
}

void Terrain::render(Shader* shader) {
    if (!m_mesh || !shader) {
        return;
    }
    
    // Set material properties
    shader->setVec3("materialColor", m_materialColor);
    shader->setFloat("materialRoughness", m_materialRoughness);
    shader->setFloat("materialMetallic", m_materialMetallic);
    
    // Bind texture if available
    if (m_texture) {
        m_texture->bind(0);
        shader->setInt("texture_diffuse1", 0);
    }
    
    // Render mesh
    m_mesh->render();
}

void Terrain::setTexture(const std::shared_ptr<Texture>& texture) {
    m_texture = texture;
}

void Terrain::setMaterialProperties(const glm::vec3& color, float roughness, float metallic) {
    m_materialColor = color;
    m_materialRoughness = roughness;
    m_materialMetallic = metallic;
}

std::shared_ptr<CollisionShape> Terrain::createCollisionShape() const {
    auto shape = std::make_shared<CollisionShape>();
    
    std::vector<glm::vec3> vertices;
    vertices.reserve(m_vertices.size());
    
    for (const auto& vertex : m_vertices) {
        vertices.push_back(vertex);
    }
    
    std::vector<int> indices;
    indices.reserve(m_indices.size());
    for (const auto& index : m_indices) {
        indices.push_back(static_cast<int>(index));
    }
    
    shape->createMeshShape(vertices, indices);
    return shape;
}

glm::vec3 Terrain::getNormalAt(float worldX, float worldZ) const {
    float epsilon = 0.1f;
    
    float hL = getHeightAt(worldX - epsilon, worldZ);
    float hR = getHeightAt(worldX + epsilon, worldZ);
    float hD = getHeightAt(worldX, worldZ - epsilon);
    float hU = getHeightAt(worldX, worldZ + epsilon);
    
    glm::vec3 normal(hL - hR, 2.0f * epsilon, hD - hU);
    return glm::normalize(normal);
}

bool Terrain::isPointOnTerrain(float worldX, float worldZ) const {
    float x = worldX / m_scale;
    float z = worldZ / m_scale;
    
    int ix = static_cast<int>(x);
    int iz = static_cast<int>(z);
    
    return ix >= 0 && ix < m_width && iz >= 0 && iz < m_height;
}

void Terrain::generateMesh() {
    Logger::debug("Generating terrain mesh");
    
    m_vertices.clear();
    m_normals.clear();
    m_texCoords.clear();
    m_tangents.clear();
    m_bitangents.clear();
    m_indices.clear();
    
    // Generate vertices and texture coordinates
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            float worldX = x * m_scale;
            float worldZ = y * m_scale;
            float worldY = m_heightMap[y * m_width + x];
            
            m_vertices.push_back(glm::vec3(worldX, worldY, worldZ));
            m_texCoords.push_back(glm::vec2(static_cast<float>(x) / (m_width - 1),
                                          static_cast<float>(y) / (m_height - 1)));
        }
    }
    
    // Generate indices for triangles
    for (int y = 0; y < m_height - 1; ++y) {
        for (int x = 0; x < m_width - 1; ++x) {
            int topLeft = y * m_width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (y + 1) * m_width + x;
            int bottomRight = bottomLeft + 1;
            
            // First triangle
            m_indices.push_back(topLeft);
            m_indices.push_back(bottomLeft);
            m_indices.push_back(topRight);
            
            // Second triangle
            m_indices.push_back(topRight);
            m_indices.push_back(bottomLeft);
            m_indices.push_back(bottomRight);
        }
    }
    
    // Calculate normals
    calculateNormals();
    
    // Create mesh
    std::vector<Vertex> vertices;
    vertices.reserve(m_vertices.size());
    
    for (size_t i = 0; i < m_vertices.size(); ++i) {
        Vertex vertex;
        vertex.position = m_vertices[i];
        vertex.normal = m_normals[i];
        vertex.texCoords = m_texCoords[i];
        
        if (i < m_tangents.size()) {
            vertex.tangent = m_tangents[i];
            vertex.bitangent = m_bitangents[i];
        } else {
            vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            vertex.bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        
        vertices.push_back(vertex);
    }
    
    m_mesh = std::make_shared<Mesh>(vertices, m_indices, std::vector<std::shared_ptr<Texture>>());
    
    Logger::debug("Terrain mesh generated: " + std::to_string(vertices.size()) + " vertices, " + 
                std::to_string(m_indices.size()) + " indices");
}

void Terrain::calculateNormals() {
    m_normals.resize(m_vertices.size(), glm::vec3(0.0f));
    
    // Calculate face normals and accumulate vertex normals
    for (size_t i = 0; i < m_indices.size(); i += 3) {
        unsigned int i0 = m_indices[i];
        unsigned int i1 = m_indices[i + 1];
        unsigned int i2 = m_indices[i + 2];
        
        glm::vec3 v0 = m_vertices[i0];
        glm::vec3 v1 = m_vertices[i1];
        glm::vec3 v2 = m_vertices[i2];
        
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        
        m_normals[i0] += normal;
        m_normals[i1] += normal;
        m_normals[i2] += normal;
    }
    
    // Normalize vertex normals
    for (auto& normal : m_normals) {
        normal = glm::normalize(normal);
    }
}

void Terrain::calculateTangents() {
    m_tangents.resize(m_vertices.size(), glm::vec3(0.0f));
    m_bitangents.resize(m_vertices.size(), glm::vec3(0.0f));
    
    for (size_t i = 0; i < m_indices.size(); i += 3) {
        unsigned int i0 = m_indices[i];
        unsigned int i1 = m_indices[i + 1];
        unsigned int i2 = m_indices[i + 2];
        
        glm::vec3 v0 = m_vertices[i0];
        glm::vec3 v1 = m_vertices[i1];
        glm::vec3 v2 = m_vertices[i2];
        
        glm::vec2 uv0 = m_texCoords[i0];
        glm::vec2 uv1 = m_texCoords[i1];
        glm::vec2 uv2 = m_texCoords[i2];
        
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;
        
        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * r;
        
        m_tangents[i0] += tangent;
        m_tangents[i1] += tangent;
        m_tangents[i2] += tangent;
        
        m_bitangents[i0] += bitangent;
        m_bitangents[i1] += bitangent;
        m_bitangents[i2] += bitangent;
    }
    
    // Normalize tangents and bitangents
    for (size_t i = 0; i < m_tangents.size(); ++i) {
        m_tangents[i] = glm::normalize(m_tangents[i]);
        m_bitangents[i] = glm::normalize(m_bitangents[i]);
    }
}

float Terrain::perlinNoise(float x, float y, float frequency, float amplitude, int octaves, float persistence) const {
    float total = 0.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; ++i) {
        total += glm::perlin(glm::vec2(x * frequency, y * frequency)) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

float Terrain::fade(float t) const {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Terrain::lerp(float a, float b, float t) const {
    return a + t * (b - a);
}

float Terrain::grad(int hash, float x, float y) const {
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

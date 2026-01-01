#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <functional>

// Forward declarations
class Renderer;
class Mesh;
class Texture;
class Shader;

// Voxel data structure
struct Voxel {
    glm::ivec3 position;
    glm::vec4 color;
    uint8_t type;
    uint8_t density;
    bool active;
    
    Voxel() : type(0), density(255), active(false) {}
    Voxel(const glm::ivec3& pos, const glm::vec4& col, uint8_t t = 1, uint8_t dens = 255)
        : position(pos), color(col), type(t), density(dens), active(true) {}
};

// Chunk structure for voxel management
class VoxelChunk {
public:
    static const int CHUNK_SIZE = 32;
    
    VoxelChunk(const glm::ivec3& chunkPos);
    ~VoxelChunk();
    
    // Voxel access
    Voxel& getVoxel(const glm::ivec3& localPos);
    const Voxel& getVoxel(const glm::ivec3& localPos) const;
    void setVoxel(const glm::ivec3& localPos, const Voxel& voxel);
    
    // Mesh generation
    void generateMesh();
    std::shared_ptr<Mesh> getMesh() const { return m_mesh; }
    bool needsUpdate() const { return m_needsUpdate; }
    
    // Chunk properties
    glm::ivec3 getPosition() const { return m_chunkPosition; }
    bool isEmpty() const { return m_voxelCount == 0; }
    
private:
    glm::ivec3 m_chunkPosition;
    std::vector<Voxel> m_voxels;
    std::shared_ptr<Mesh> m_mesh;
    uint32_t m_voxelCount;
    bool m_needsUpdate;
    
    int voxelToIndex(const glm::ivec3& localPos) const;
    void marchingCubes();
};

// Main voxel terrain system
class VoxelTerrain {
public:
    VoxelTerrain();
    ~VoxelTerrain();
    
    bool initialize();
    void shutdown();
    void update(float deltaTime);
    void render(Renderer* renderer);
    
    // Terrain generation
    void generateTerrain(const glm::vec3& worldCenter, float radius);
    void generateChunk(const glm::ivec3& chunkPos);
    
    // Voxel manipulation
    void setVoxel(const glm::vec3& worldPos, const Voxel& voxel);
    Voxel getVoxel(const glm::vec3& worldPos) const;
    void removeVoxel(const glm::vec3& worldPos);
    
    // Terrain modification
    void explode(const glm::vec3& center, float radius);
    void carveSphere(const glm::vec3& center, float radius);
    void fillSphere(const glm::vec3& center, float radius, const Voxel& voxel);
    
    // Physics integration
    std::vector<glm::vec3> getCollidableVoxels(const glm::vec3& center, float radius) const;
    bool checkCollision(const glm::vec3& point) const;
    
    // LOD and optimization
    void setLOD(const glm::vec3& cameraPos);
    void updateChunks();
    
private:
    std::vector<std::unique_ptr<VoxelChunk>> m_chunks;
    glm::ivec3 m_worldCenter;
    int m_renderDistance;
    int m_lodLevels;
    
    // Procedural generation
    float noise3D(const glm::vec3& pos) const;
    Voxel generateVoxel(const glm::vec3& worldPos) const;
    
    // Chunk management
    glm::ivec3 worldToChunk(const glm::vec3& worldPos) const;
    glm::ivec3 worldToLocalVoxel(const glm::vec3& worldPos) const;
    VoxelChunk* getChunk(const glm::ivec3& chunkPos);
    const VoxelChunk* getChunk(const glm::ivec3& chunkPos) const;
    
    bool m_initialized;
};

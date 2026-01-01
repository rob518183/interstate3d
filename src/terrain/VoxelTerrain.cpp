#include "VoxelTerrain.hpp"
#include "rendering/Mesh.hpp"
#include "rendering/Renderer.hpp"
#include "core/Logger.hpp"
#include <glm/gtc/noise.hpp>
#include <algorithm>

VoxelChunk::VoxelChunk(const glm::ivec3& chunkPos) 
    : m_chunkPosition(chunkPos), m_voxelCount(0), m_needsUpdate(false) {
    // Initialize voxel storage
    m_voxels.resize(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    std::fill(m_voxels.begin(), m_voxels.end(), Voxel());
    
    Logger::debug("Created VoxelChunk at position: " + 
                 std::to_string(chunkPos.x) + ", " + 
                 std::to_string(chunkPos.y) + ", " + 
                 std::to_string(chunkPos.z));
}

VoxelChunk::~VoxelChunk() = default;

Voxel& VoxelChunk::getVoxel(const glm::ivec3& localPos) {
    if (localPos.x < 0 || localPos.x >= CHUNK_SIZE ||
        localPos.y < 0 || localPos.y >= CHUNK_SIZE ||
        localPos.z < 0 || localPos.z >= CHUNK_SIZE) {
        static Voxel emptyVoxel;
        return emptyVoxel;
    }
    
    return m_voxels[voxelToIndex(localPos)];
}

const Voxel& VoxelChunk::getVoxel(const glm::ivec3& localPos) const {
    if (localPos.x < 0 || localPos.x >= CHUNK_SIZE ||
        localPos.y < 0 || localPos.y >= CHUNK_SIZE ||
        localPos.z < 0 || localPos.z >= CHUNK_SIZE) {
        static Voxel emptyVoxel;
        return emptyVoxel;
    }
    
    return m_voxels[voxelToIndex(localPos)];
}

void VoxelChunk::setVoxel(const glm::ivec3& localPos, const Voxel& voxel) {
    if (localPos.x < 0 || localPos.x >= CHUNK_SIZE ||
        localPos.y < 0 || localPos.y >= CHUNK_SIZE ||
        localPos.z < 0 || localPos.z >= CHUNK_SIZE) {
        return;
    }
    
    int index = voxelToIndex(localPos);
    bool wasActive = m_voxels[index].active;
    m_voxels[index] = voxel;
    
    if (voxel.active && !wasActive) {
        m_voxelCount++;
    } else if (!voxel.active && wasActive) {
        m_voxelCount--;
    }
    
    m_needsUpdate = true;
}

void VoxelChunk::generateMesh() {
    if (!m_needsUpdate) {
        return;
    }
    
    // Simple mesh generation - in a full implementation this would use marching cubes
    marchingCubes();
    m_needsUpdate = false;
    
    Logger::debug("Generated mesh for chunk at " + 
                 std::to_string(m_chunkPosition.x) + ", " + 
                 std::to_string(m_chunkPosition.y) + ", " + 
                 std::to_string(m_chunkPosition.z) + 
                 " with " + std::to_string(m_voxelCount) + " voxels");
}

int VoxelChunk::voxelToIndex(const glm::ivec3& localPos) const {
    return localPos.x + localPos.y * CHUNK_SIZE + localPos.z * CHUNK_SIZE * CHUNK_SIZE;
}

void VoxelChunk::marchingCubes() {
    // Simplified marching cubes implementation
    // In a full implementation, this would generate smooth mesh from voxel data
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    
    // Generate basic cube mesh for each active voxel
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                glm::ivec3 pos(x, y, z);
                const Voxel& voxel = getVoxel(pos);
                
                if (!voxel.active) {
                    continue;
                }
                
                // Check if voxel is on the surface (has at least one inactive neighbor)
                bool isSurface = false;
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dz = -1; dz <= 1; ++dz) {
                            if (dx == 0 && dy == 0 && dz == 0) continue;
                            
                            glm::ivec3 neighborPos = pos + glm::ivec3(dx, dy, dz);
                            const Voxel& neighbor = getVoxel(neighborPos);
                            
                            if (!neighbor.active) {
                                isSurface = true;
                                break;
                            }
                        }
                        if (isSurface) break;
                    }
                    if (isSurface) break;
                }
                
                if (!isSurface) continue;
                
                // Generate cube vertices for this voxel
                float voxelSize = 1.0f;
                glm::vec3 worldPos = glm::vec3(
                    m_chunkPosition.x * CHUNK_SIZE + x,
                    m_chunkPosition.y * CHUNK_SIZE + y,
                    m_chunkPosition.z * CHUNK_SIZE + z
                );
                
                // Simple cube generation (simplified)
                // In a full implementation, this would generate proper mesh data
                for (int i = 0; i < 8; ++i) {
                    vertices.push_back(worldPos.x + (i & 1) * voxelSize);
                    vertices.push_back(worldPos.y + ((i >> 1) & 1) * voxelSize);
                    vertices.push_back(worldPos.z + ((i >> 2) & 1) * voxelSize);
                    
                    normals.push_back(0.0f);
                    normals.push_back(1.0f);
                    normals.push_back(0.0f);
                    
                    texCoords.push_back((i & 1) * 1.0f);
                    texCoords.push_back(((i >> 1) & 1) * 1.0f);
                }
            }
        }
    }
    
    // Create mesh if we have vertices
    if (!vertices.empty()) {
        m_mesh = std::make_shared<Mesh>();
        m_mesh->loadFromData(vertices, indices, normals, texCoords);
    } else {
        m_mesh.reset();
    }
}

// VoxelTerrain implementation
VoxelTerrain::VoxelTerrain() : m_renderDistance(5), m_lodLevels(3), m_initialized(false) {
    Logger::info("VoxelTerrain created");
}

VoxelTerrain::~VoxelTerrain() {
    shutdown();
}

bool VoxelTerrain::initialize() {
    Logger::info("Initializing VoxelTerrain...");
    
    m_worldCenter = glm::ivec3(0, 0, 0);
    m_chunks.clear();
    
    m_initialized = true;
    Logger::info("VoxelTerrain initialized successfully");
    return true;
}

void VoxelTerrain::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    Logger::info("Shutting down VoxelTerrain...");
    m_chunks.clear();
    m_initialized = false;
}

void VoxelTerrain::update(float deltaTime) {
    if (!m_initialized) {
        return;
    }
    
    updateChunks();
}

void VoxelTerrain::render(Renderer* renderer) {
    if (!m_initialized || !renderer) {
        return;
    }
    
    std::vector<std::shared_ptr<Mesh>> terrainChunks;
    
    for (const auto& chunk : m_chunks) {
        if (chunk && chunk->getMesh()) {
            terrainChunks.push_back(chunk->getMesh());
        }
    }
    
    if (!terrainChunks.empty()) {
        renderer->renderTerrain(terrainChunks);
    }
}

void VoxelTerrain::generateTerrain(const glm::vec3& worldCenter, float radius) {
    Logger::info("Generating terrain around " + 
                std::to_string(worldCenter.x) + ", " + 
                std::to_string(worldCenter.y) + ", " + 
                std::to_string(worldCenter.z) + 
                " with radius " + std::to_string(radius));
    
    m_worldCenter = glm::ivec3(worldCenter);
    
    // Generate chunks within radius
    int chunkRadius = static_cast<int>(radius / VoxelChunk::CHUNK_SIZE) + 1;
    
    for (int x = -chunkRadius; x <= chunkRadius; ++x) {
        for (int y = -chunkRadius; y <= chunkRadius; ++y) {
            for (int z = -chunkRadius; z <= chunkRadius; ++z) {
                glm::ivec3 chunkPos = m_worldCenter + glm::ivec3(x, y, z);
                
                // Check if chunk is within radius
                glm::vec3 chunkCenter = glm::vec3(chunkPos) * static_cast<float>(VoxelChunk::CHUNK_SIZE);
                if (glm::distance(chunkCenter, worldCenter) <= radius) {
                    generateChunk(chunkPos);
                }
            }
        }
    }
    
    Logger::info("Generated " + std::to_string(m_chunks.size()) + " terrain chunks");
}

void VoxelTerrain::generateChunk(const glm::ivec3& chunkPos) {
    // Check if chunk already exists
    for (const auto& chunk : m_chunks) {
        if (chunk && chunk->getPosition() == chunkPos) {
            return;
        }
    }
    
    auto chunk = std::make_unique<VoxelChunk>(chunkPos);
    
    // Generate voxel data for this chunk
    for (int x = 0; x < VoxelChunk::CHUNK_SIZE; ++x) {
        for (int y = 0; y < VoxelChunk::CHUNK_SIZE; ++y) {
            for (int z = 0; z < VoxelChunk::CHUNK_SIZE; ++z) {
                glm::ivec3 localPos(x, y, z);
                glm::vec3 worldPos = glm::vec3(
                    chunkPos.x * VoxelChunk::CHUNK_SIZE + x,
                    chunkPos.y * VoxelChunk::CHUNK_SIZE + y,
                    chunkPos.z * VoxelChunk::CHUNK_SIZE + z
                );
                
                Voxel voxel = generateVoxel(worldPos);
                if (voxel.active) {
                    chunk->setVoxel(localPos, voxel);
                }
            }
        }
    }
    
    // Generate mesh for the chunk
    chunk->generateMesh();
    
    // Add to terrain if it's not empty
    if (!chunk->isEmpty()) {
        m_chunks.push_back(std::move(chunk));
    }
}

Voxel VoxelTerrain::generateVoxel(const glm::vec3& worldPos) const {
    // Simple terrain generation using noise
    float height = noise3D(worldPos * 0.05f) * 20.0f + 10.0f;
    
    if (worldPos.y < height) {
        // Underground voxel
        glm::vec4 color(0.6f, 0.4f, 0.2f, 1.0f); // Brown dirt
        
        if (worldPos.y < height - 5.0f) {
            color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray stone
        }
        
        return Voxel(glm::ivec3(worldPos), color, 1, 255);
    }
    
    return Voxel(); // Empty voxel
}

float VoxelTerrain::noise3D(const glm::vec3& pos) const {
    // Simple 3D noise function
    return glm::simplex(pos);
}

void VoxelTerrain::setVoxel(const glm::vec3& worldPos, const Voxel& voxel) {
    glm::ivec3 chunkPos = worldToChunk(worldPos);
    VoxelChunk* chunk = getChunk(chunkPos);
    
    if (chunk) {
        glm::ivec3 localPos = worldToLocalVoxel(worldPos);
        chunk->setVoxel(localPos, voxel);
    }
}

Voxel VoxelTerrain::getVoxel(const glm::vec3& worldPos) const {
    glm::ivec3 chunkPos = worldToChunk(worldPos);
    const VoxelChunk* chunk = getChunk(chunkPos);
    
    if (chunk) {
        glm::ivec3 localPos = worldToLocalVoxel(worldPos);
        return chunk->getVoxel(localPos);
    }
    
    return Voxel(); // Empty voxel
}

void VoxelTerrain::removeVoxel(const glm::vec3& worldPos) {
    Voxel emptyVoxel;
    setVoxel(worldPos, emptyVoxel);
}

void VoxelTerrain::explode(const glm::vec3& center, float radius) {
    // Simple explosion - remove voxels within radius
    for (auto& chunk : m_chunks) {
        if (!chunk) continue;
        
        glm::ivec3 chunkPos = chunk->getPosition();
        glm::vec3 chunkCenter = glm::vec3(chunkPos) * static_cast<float>(VoxelChunk::CHUNK_SIZE);
        
        if (glm::distance(chunkCenter, center) <= radius + VoxelChunk::CHUNK_SIZE) {
            // Check voxels in this chunk
            for (int x = 0; x < VoxelChunk::CHUNK_SIZE; ++x) {
                for (int y = 0; y < VoxelChunk::CHUNK_SIZE; ++y) {
                    for (int z = 0; z < VoxelChunk::CHUNK_SIZE; ++z) {
                        glm::ivec3 localPos(x, y, z);
                        glm::vec3 voxelWorldPos = glm::vec3(
                            chunkPos.x * VoxelChunk::CHUNK_SIZE + x,
                            chunkPos.y * VoxelChunk::CHUNK_SIZE + y,
                            chunkPos.z * VoxelChunk::CHUNK_SIZE + z
                        );
                        
                        if (glm::distance(voxelWorldPos, center) <= radius) {
                            chunk->setVoxel(localPos, Voxel());
                        }
                    }
                }
            }
            
            chunk->generateMesh();
        }
    }
}

void VoxelTerrain::updateChunks() {
    // Update chunk meshes if needed
    for (auto& chunk : m_chunks) {
        if (chunk && chunk->needsUpdate()) {
            chunk->generateMesh();
        }
    }
}

glm::ivec3 VoxelTerrain::worldToChunk(const glm::vec3& worldPos) const {
    return glm::ivec3(
        static_cast<int>(std::floor(worldPos.x / VoxelChunk::CHUNK_SIZE)),
        static_cast<int>(std::floor(worldPos.y / VoxelChunk::CHUNK_SIZE)),
        static_cast<int>(std::floor(worldPos.z / VoxelChunk::CHUNK_SIZE))
    );
}

glm::ivec3 VoxelTerrain::worldToLocalVoxel(const glm::vec3& worldPos) const {
    glm::ivec3 chunkPos = worldToChunk(worldPos);
    return glm::ivec3(
        static_cast<int>(std::floor(worldPos.x)) - chunkPos.x * VoxelChunk::CHUNK_SIZE,
        static_cast<int>(std::floor(worldPos.y)) - chunkPos.y * VoxelChunk::CHUNK_SIZE,
        static_cast<int>(std::floor(worldPos.z)) - chunkPos.z * VoxelChunk::CHUNK_SIZE
    );
}

VoxelChunk* VoxelTerrain::getChunk(const glm::ivec3& chunkPos) {
    for (auto& chunk : m_chunks) {
        if (chunk && chunk->getPosition() == chunkPos) {
            return chunk.get();
        }
    }
    return nullptr;
}

const VoxelChunk* VoxelTerrain::getChunk(const glm::ivec3& chunkPos) const {
    for (const auto& chunk : m_chunks) {
        if (chunk && chunk->getPosition() == chunkPos) {
            return chunk.get();
        }
    }
    return nullptr;
}

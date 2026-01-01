#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "core/Application.hpp"

// Forward declarations
class Shader;
class Mesh;
class Texture;
class Camera;
class Light;

// Abstract base class for rendering backends
class RenderBackend {
public:
    virtual ~RenderBackend() = default;
    
    // Core initialization and lifecycle
    virtual bool initialize(GLFWwindow* window) = 0;
    virtual void shutdown() = 0;
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    
    // Rendering operations
    virtual void setCamera(const std::shared_ptr<Camera>& camera) = 0;
    virtual void addLight(const std::shared_ptr<Light>& light) = 0;
    virtual void clearLights() = 0;
    
    virtual void renderMesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& modelMatrix) = 0;
    virtual void renderSkybox() = 0;
    virtual void renderTerrain(const std::vector<std::shared_ptr<Mesh>>& terrainChunks) = 0;
    
    // State management
    virtual void setClearColor(const glm::vec3& color) = 0;
    virtual void enableDepthTest(bool enable) = 0;
    virtual void enableCulling(bool enable) = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;
    
    // Advanced features for Interstate 3D
    virtual void enablePBR(bool enable) = 0;
    virtual void setEnvironmentMap(const std::shared_ptr<Texture>& cubemap) = 0;
    virtual void renderVoxels(const std::vector<glm::vec3>& voxelPositions, 
                             const std::vector<glm::vec4>& voxelColors) = 0;
    
    // Performance and debugging
    virtual void enableWireframe(bool enable) = 0;
    virtual void enableDebugInfo(bool enable) = 0;
    virtual uint32_t getDrawCalls() const = 0;
    virtual float getFrameTime() const = 0;
    
    // Resource management
    virtual void reloadShaders() = 0;
    virtual void garbageCollect() = 0;
    
    // Shader hot-reloading callback
    virtual void onShaderReloaded(const std::string& shaderName) = 0;
};

// Factory function for creating backends
std::unique_ptr<RenderBackend> createRenderBackend(GraphicsAPI api);

#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "RenderBackend.hpp"
#include "ShaderManager.hpp"

class Shader;
class Mesh;
class Texture;
class Camera;
class Light;

class Renderer {
public:
    Renderer(GraphicsAPI api = GraphicsAPI::OpenGL);
    ~Renderer();
    
    bool initialize(GLFWwindow* window);
    void shutdown();
    
    void beginFrame();
    void endFrame();
    
    void setCamera(const std::shared_ptr<Camera>& camera);
    void addLight(const std::shared_ptr<Light>& light);
    void clearLights();
    
    void renderMesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& modelMatrix);
    void renderSkybox();
    void renderTerrain(const std::vector<std::shared_ptr<Mesh>>& terrainChunks);
    
    void setClearColor(const glm::vec3& color);
    void enableDepthTest(bool enable);
    void enableCulling(bool enable);
    void setViewport(int x, int y, int width, int height);
    
    // Advanced Interstate 3D features
    void enablePBR(bool enable);
    void setEnvironmentMap(const std::shared_ptr<Texture>& cubemap);
    void renderVoxels(const std::vector<glm::vec3>& voxelPositions, 
                     const std::vector<glm::vec4>& voxelColors);
    
    // Post-processing effects
    void enableBloom(bool enable);
    void enableToneMapping(bool enable);
    void enableMotionBlur(bool enable);
    void enableDepthOfField(bool enable);
    void enableSSR(bool enable); // Screen-space reflections
    void enableSSAO(bool enable); // Screen-space ambient occlusion
    
    // Performance and debugging
    void enableWireframe(bool enable);
    void enableDebugInfo(bool enable);
    uint32_t getDrawCalls() const;
    float getFrameTime() const;
    
    // Resource management
    void reloadShaders();
    void garbageCollect();
    
    // Shader management
    ShaderManager& getShaderManager() { return ShaderManager::getInstance(); }
    std::shared_ptr<Shader> getShader(const std::string& name) { return getShaderManager().getShader(name); }
    void enableShaderHotReloading(bool enable) { getShaderManager().enableHotReloading(enable); }
    void setShaderReloadCallback(ShaderManager::ReloadCallback callback) { getShaderManager().setReloadCallback(callback); }
    
    // Backend information
    GraphicsAPI getGraphicsAPI() const { return m_graphicsAPI; }
    const char* getBackendName() const;
    
private:
    std::unique_ptr<RenderBackend> m_backend;
    GraphicsAPI m_graphicsAPI;
    bool m_initialized = false;
};

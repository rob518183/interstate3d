#pragma once

#include "RenderBackend.hpp"
#include <memory>
#include <vector>

class Shader;
class Mesh;
class Texture;
class Camera;
class Light;

class OpenGLBackend : public RenderBackend {
public:
    OpenGLBackend();
    ~OpenGLBackend() override;
    
    // Core initialization and lifecycle
    bool initialize(GLFWwindow* window) override;
    void shutdown() override;
    void beginFrame() override;
    void endFrame() override;
    
    // Rendering operations
    void setCamera(const std::shared_ptr<Camera>& camera) override;
    void addLight(const std::shared_ptr<Light>& light) override;
    void clearLights() override;
    
    void renderMesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& modelMatrix) override;
    void renderSkybox() override;
    void renderTerrain(const std::vector<std::shared_ptr<Mesh>>& terrainChunks) override;
    
    // State management
    void setClearColor(const glm::vec3& color) override;
    void enableDepthTest(bool enable) override;
    void enableCulling(bool enable) override;
    void setViewport(int x, int y, int width, int height) override;
    
    // Advanced features for Interstate 3D
    void enablePBR(bool enable) override;
    void setEnvironmentMap(const std::shared_ptr<Texture>& cubemap) override;
    void renderVoxels(const std::vector<glm::vec3>& voxelPositions, 
                     const std::vector<glm::vec4>& voxelColors) override;
    
    // Performance and debugging
    void enableWireframe(bool enable) override;
    void enableDebugInfo(bool enable) override;
    uint32_t getDrawCalls() const override;
    float getFrameTime() const override;
    
    // Resource management
    void reloadShaders() override;
    void garbageCollect() override;
    
    // Shader hot-reloading callback
    void onShaderReloaded(const std::string& shaderName) override;
    
private:
    void setupOpenGL();
    void createDefaultShaders();
    void createPBRShaders();
    void createVoxelShader();
    void updateLightingUniforms();
    
    GLFWwindow* m_window = nullptr;
    std::shared_ptr<Camera> m_camera;
    std::vector<std::shared_ptr<Light>> m_lights;
    
    // Shaders
    std::shared_ptr<Shader> m_defaultShader;
    std::shared_ptr<Shader> m_skyboxShader;
    std::shared_ptr<Shader> m_terrainShader;
    std::shared_ptr<Shader> m_pbrShader;
    std::shared_ptr<Shader> m_voxelShader;
    
    // OpenGL state
    glm::vec3 m_clearColor = glm::vec3(0.2f, 0.5f, 0.8f); // Light blue
    bool m_depthTestEnabled = true;
    bool m_cullingEnabled = true;
    bool m_wireframeEnabled = false;
    bool m_pbrEnabled = false;
    bool m_debugInfoEnabled = false;
    
    // Performance tracking
    uint32_t m_drawCalls = 0;
    float m_frameTime = 0.0f;
    double m_lastFrameTime = 0.0;
    
    // Voxel rendering
    GLuint m_voxelVAO = 0;
    GLuint m_voxelVBO = 0;
    
    bool m_initialized = false;
};

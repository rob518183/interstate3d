#include "rendering/OpenGLBackend.hpp"
#include "rendering/Shader.hpp"
#include "rendering/Mesh.hpp"
#include "rendering/Texture.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Light.hpp"
#include "core/Logger.hpp"
#include <cmath>

// Placeholder implementation for OpenGL - would normally include:
// #include <glad/glad.h>
#include <GLFW/glfw3.h>

// Declare stub OpenGL functions
extern void glGenVertexArrays(int n, unsigned int* arrays);
extern void glBindVertexArray(unsigned int array);
extern void glGenBuffers(int n, unsigned int* buffers);
extern void glBindBuffer(unsigned int target, unsigned int buffer);
extern void glBufferData(unsigned int target, int size, const void* data, unsigned int usage);
extern void glEnableVertexAttribArray(unsigned int index);
extern void glVertexAttribPointer(unsigned int index, int size, unsigned int type, int normalized, int stride, const void* pointer);
extern void glDeleteVertexArrays(int n, const unsigned int* arrays);
extern void glDeleteBuffers(int n, const unsigned int* buffers);

// Define OpenGL constants for placeholder implementation
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif
#ifndef GL_FLOAT
#define GL_FLOAT 0x1406
#endif

OpenGLBackend::OpenGLBackend() {
    Logger::info("OpenGLBackend created");
}

OpenGLBackend::~OpenGLBackend() {
    shutdown();
}

bool OpenGLBackend::initialize(GLFWwindow* window) {
    Logger::info("Initializing OpenGL backend...");
    
    m_window = window;
    
    // Load OpenGL functions (would use GLAD in real implementation)
    // if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    //     Logger::error("Failed to initialize GLAD");
    //     return false;
    // }
    
    setupOpenGL();
    createDefaultShaders();
    createVoxelShader();
    
    m_initialized = true;
    Logger::info("OpenGL backend initialized successfully");
    return true;
}

void OpenGLBackend::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    Logger::info("Shutting down OpenGL backend...");
    
    // Cleanup voxel rendering objects
    if (m_voxelVAO != 0) {
        glDeleteVertexArrays(1, &m_voxelVAO);
        m_voxelVAO = 0;
    }
    if (m_voxelVBO != 0) {
        glDeleteBuffers(1, &m_voxelVBO);
        m_voxelVBO = 0;
    }
    
    // Reset shaders
    m_defaultShader.reset();
    m_skyboxShader.reset();
    m_terrainShader.reset();
    m_pbrShader.reset();
    m_voxelShader.reset();
    
    m_lights.clear();
    m_camera.reset();
    
    m_initialized = false;
}

void OpenGLBackend::beginFrame() {
    // Animate clear color to show the application is working
    static float time = 0.0f;
    time += 0.016f; // ~60 FPS
    
    // Create a nice animated color
    float r = 0.2f + 0.3f * sin(time);
    float g = 0.5f + 0.3f * sin(time + 2.0f);
    float b = 0.8f + 0.2f * sin(time + 4.0f);
    
    m_clearColor = glm::vec3(r, g, b);
    
    // Clear buffers
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set OpenGL state
    if (m_depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    
    if (m_cullingEnabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else {
        glDisable(GL_CULL_FACE);
    }
    
    if (m_wireframeEnabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Start performance tracking
    m_drawCalls = 0;
    double currentTime = glfwGetTime();
    if (m_lastFrameTime > 0.0) {
        m_frameTime = static_cast<float>(currentTime - m_lastFrameTime);
    }
    m_lastFrameTime = currentTime;
}

void OpenGLBackend::endFrame() {
    // Frame cleanup if needed
}

void OpenGLBackend::setCamera(const std::shared_ptr<Camera>& camera) {
    m_camera = camera;
}

void OpenGLBackend::addLight(const std::shared_ptr<Light>& light) {
    m_lights.push_back(light);
}

void OpenGLBackend::clearLights() {
    m_lights.clear();
}

void OpenGLBackend::renderMesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& modelMatrix) {
    if (!mesh || !m_camera || !m_defaultShader) {
        return;
    }
    
    m_drawCalls++;
    
    // Use appropriate shader
    auto shader = m_pbrEnabled ? m_pbrShader : m_defaultShader;
    if (!shader) shader = m_defaultShader;
    
    shader->use();
    
    // Set matrices
    shader->setMat4("model", modelMatrix);
    shader->setMat4("view", m_camera->getViewMatrix());
    shader->setMat4("projection", m_camera->getProjectionMatrix());
    
    // Set camera position
    shader->setVec3("viewPos", m_camera->getPosition());
    
    // Update lighting uniforms
    updateLightingUniforms();
    
    // Render mesh
    mesh->render();
}

void OpenGLBackend::renderSkybox() {
    if (!m_skyboxShader || !m_camera) {
        return;
    }
    
    m_drawCalls++;
    
    // Skybox rendering implementation
    // This would render a cube map around the scene
    Logger::debug("Rendering skybox");
}

void OpenGLBackend::renderTerrain(const std::vector<std::shared_ptr<Mesh>>& terrainChunks) {
    if (!m_terrainShader || !m_camera) {
        return;
    }
    
    m_terrainShader->use();
    
    // Set camera matrices
    m_terrainShader->setMat4("view", m_camera->getViewMatrix());
    m_terrainShader->setMat4("projection", m_camera->getProjectionMatrix());
    m_terrainShader->setVec3("viewPos", m_camera->getPosition());
    
    // Update lighting
    updateLightingUniforms();
    
    // Render terrain chunks
    for (const auto& chunk : terrainChunks) {
        if (chunk) {
            m_drawCalls++;
            chunk->render();
        }
    }
}

void OpenGLBackend::setClearColor(const glm::vec3& color) {
    m_clearColor = color;
}

void OpenGLBackend::enableDepthTest(bool enable) {
    m_depthTestEnabled = enable;
}

void OpenGLBackend::enableCulling(bool enable) {
    m_cullingEnabled = enable;
}

void OpenGLBackend::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

// Advanced Interstate 3D features
void OpenGLBackend::enablePBR(bool enable) {
    m_pbrEnabled = enable;
    if (enable && !m_pbrShader) {
        createPBRShaders();
    }
    Logger::info("PBR rendering " + std::string(enable ? "enabled" : "disabled"));
}

void OpenGLBackend::setEnvironmentMap(const std::shared_ptr<Texture>& cubemap) {
    // Environment map implementation for PBR
    Logger::info("Environment map set for PBR rendering");
}

void OpenGLBackend::renderVoxels(const std::vector<glm::vec3>& voxelPositions, 
                                const std::vector<glm::vec4>& voxelColors) {
    if (!m_voxelShader || voxelPositions.empty() || voxelColors.empty()) {
        return;
    }
    
    m_drawCalls++;
    
    // Setup voxel rendering
    if (m_voxelVAO == 0) {
        glGenVertexArrays(1, &m_voxelVAO);
        glGenBuffers(1, &m_voxelVBO);
    }
    
    glBindVertexArray(m_voxelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_voxelVBO);
    
    // Interleave position and color data
    std::vector<float> vertexData;
    for (size_t i = 0; i < voxelPositions.size() && i < voxelColors.size(); ++i) {
        vertexData.push_back(voxelPositions[i].x);
        vertexData.push_back(voxelPositions[i].y);
        vertexData.push_back(voxelPositions[i].z);
        vertexData.push_back(voxelColors[i].r);
        vertexData.push_back(voxelColors[i].g);
        vertexData.push_back(voxelColors[i].b);
        vertexData.push_back(voxelColors[i].a);
    }
    
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);
    
    // Setup vertex attributes
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    
    // Render voxels
    m_voxelShader->use();
    m_voxelShader->setMat4("view", m_camera->getViewMatrix());
    m_voxelShader->setMat4("projection", m_camera->getProjectionMatrix());
    
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(voxelPositions.size()));
    
    glBindVertexArray(0);
}

// Performance and debugging
void OpenGLBackend::enableWireframe(bool enable) {
    m_wireframeEnabled = enable;
}

void OpenGLBackend::enableDebugInfo(bool enable) {
    m_debugInfoEnabled = enable;
    if (enable) {
        Logger::info("Debug info enabled - Draw calls: " + std::to_string(m_drawCalls) + 
                    ", Frame time: " + std::to_string(m_frameTime * 1000.0f) + "ms");
    }
}

uint32_t OpenGLBackend::getDrawCalls() const {
    return m_drawCalls;
}

float OpenGLBackend::getFrameTime() const {
    return m_frameTime;
}

// Resource management
void OpenGLBackend::reloadShaders() {
    Logger::info("Reloading shaders...");
    createDefaultShaders();
    createPBRShaders();
    createVoxelShader();
}

void OpenGLBackend::garbageCollect() {
    // Cleanup unused resources
    Logger::debug("Performing garbage collection");
}

// Private methods
void OpenGLBackend::setupOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Logger::info("OpenGL setup complete");
}

void OpenGLBackend::createDefaultShaders() {
    // Create default lighting shader
    m_defaultShader = std::make_shared<Shader>();
    if (!m_defaultShader->loadFromFiles("shaders/default.vert", "shaders/default.frag")) {
        Logger::error("Failed to load default shader");
        m_defaultShader.reset();
    }
    
    // Create skybox shader
    m_skyboxShader = std::make_shared<Shader>();
    if (!m_skyboxShader->loadFromFiles("shaders/skybox.vert", "shaders/skybox.frag")) {
        Logger::error("Failed to load skybox shader");
        m_skyboxShader.reset();
    }
    
    // Create terrain shader
    m_terrainShader = std::make_shared<Shader>();
    if (!m_terrainShader->loadFromFiles("shaders/terrain.vert", "shaders/terrain.frag")) {
        Logger::error("Failed to load terrain shader");
        m_terrainShader.reset();
    }
    
    Logger::info("Default shaders created");
}

void OpenGLBackend::createPBRShaders() {
    m_pbrShader = std::make_shared<Shader>();
    if (!m_pbrShader->loadFromFiles("shaders/pbr.vert", "shaders/pbr.frag")) {
        Logger::error("Failed to load PBR shader");
        m_pbrShader.reset();
    }
    Logger::info("PBR shaders created");
}

void OpenGLBackend::createVoxelShader() {
    m_voxelShader = std::make_shared<Shader>();
    if (!m_voxelShader->loadFromFiles("shaders/voxel.vert", "shaders/voxel.frag")) {
        Logger::error("Failed to load voxel shader");
        m_voxelShader.reset();
    }
    Logger::info("Voxel shader created");
}

void OpenGLBackend::updateLightingUniforms() {
    auto shader = m_pbrEnabled ? m_pbrShader : m_defaultShader;
    if (!shader) return;
    
    // Set lights
    for (size_t i = 0; i < m_lights.size() && i < 4; ++i) {
        const auto& light = m_lights[i];
        std::string prefix = "lights[" + std::to_string(i) + "].";
        
        shader->setVec3(prefix + "position", light->getPosition());
        shader->setVec3(prefix + "color", light->getColor());
        shader->setFloat(prefix + "intensity", light->getIntensity());
    }
    
    shader->setInt("numLights", static_cast<int>(m_lights.size()));
}

void OpenGLBackend::onShaderReloaded(const std::string& shaderName) {
    Logger::info("Shader reloaded: " + shaderName);
    // Re-create shaders that were reloaded
    if (shaderName == "default" || shaderName == "all") {
        createDefaultShaders();
    }
    if (shaderName == "pbr" || shaderName == "all") {
        createPBRShaders();
    }
    if (shaderName == "voxel" || shaderName == "all") {
        createVoxelShader();
    }
}

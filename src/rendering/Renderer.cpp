#include "Renderer.hpp"
#include "RenderBackend.hpp"
#include "ShaderManager.hpp"
#include "core/Logger.hpp"

Renderer::Renderer(GraphicsAPI api) : m_graphicsAPI(api) {
    std::string backendName = (api == GraphicsAPI::Vulkan ? "Vulkan" : "OpenGL");
    Logger::info("Creating renderer with backend: " + backendName);
}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize(GLFWwindow* window) {
    Logger::info("Initializing renderer...");
    
    // Create the appropriate backend
    m_backend = createRenderBackend(m_graphicsAPI);
    if (!m_backend) {
        Logger::error("Failed to create render backend");
        return false;
    }
    
    // Initialize the backend
    if (!m_backend->initialize(window)) {
        Logger::error("Failed to initialize render backend");
        return false;
    }
    
    std::string backendName = getBackendName();
    Logger::info("Renderer initialized successfully with " + backendName);
    
    // Initialize shader manager with default shaders
    auto& shaderManager = ShaderManager::getInstance();
    shaderManager.initializeDefaultShaders();
    shaderManager.enableHotReloading(true);
    
    // Set up shader reload callback
    shaderManager.setReloadCallback([this](const std::string& shaderName) {
        Logger::info("Shader reloaded: " + shaderName);
        // Notify backend of shader changes if needed
        if (m_backend) {
            m_backend->onShaderReloaded(shaderName);
        }
    });
    
    m_initialized = true;
    return true;
}

void Renderer::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    Logger::info("Shutting down renderer...");
    
    if (m_backend) {
        m_backend->shutdown();
        m_backend.reset();
    }
    
    m_initialized = false;
    Logger::info("Renderer shutdown complete");
}

void Renderer::beginFrame() {
    if (m_backend) {
        m_backend->beginFrame();
    }
}

void Renderer::endFrame() {
    if (m_backend) {
        m_backend->endFrame();
    }
}

void Renderer::setCamera(const std::shared_ptr<Camera>& camera) {
    if (m_backend) {
        m_backend->setCamera(camera);
    }
}

void Renderer::addLight(const std::shared_ptr<Light>& light) {
    if (m_backend) {
        m_backend->addLight(light);
    }
}

void Renderer::clearLights() {
    if (m_backend) {
        m_backend->clearLights();
    }
}

void Renderer::renderMesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& modelMatrix) {
    if (m_backend) {
        m_backend->renderMesh(mesh, modelMatrix);
    }
}

void Renderer::renderSkybox() {
    if (m_backend) {
        m_backend->renderSkybox();
    }
}

void Renderer::renderTerrain(const std::vector<std::shared_ptr<Mesh>>& terrainChunks) {
    if (m_backend) {
        m_backend->renderTerrain(terrainChunks);
    }
}

void Renderer::setClearColor(const glm::vec3& color) {
    if (m_backend) {
        m_backend->setClearColor(color);
    }
}

void Renderer::enableDepthTest(bool enable) {
    if (m_backend) {
        m_backend->enableDepthTest(enable);
    }
}

void Renderer::enableCulling(bool enable) {
    if (m_backend) {
        m_backend->enableCulling(enable);
    }
}

void Renderer::setViewport(int x, int y, int width, int height) {
    if (m_backend) {
        m_backend->setViewport(x, y, width, height);
    }
}

// Advanced Interstate 3D features
void Renderer::enablePBR(bool enable) {
    if (m_backend) {
        m_backend->enablePBR(enable);
    }
}

void Renderer::setEnvironmentMap(const std::shared_ptr<Texture>& cubemap) {
    if (m_backend) {
        m_backend->setEnvironmentMap(cubemap);
    }
}

void Renderer::renderVoxels(const std::vector<glm::vec3>& voxelPositions, 
                          const std::vector<glm::vec4>& voxelColors) {
    if (m_backend) {
        m_backend->renderVoxels(voxelPositions, voxelColors);
    }
}

// Performance and debugging
void Renderer::enableWireframe(bool enable) {
    if (m_backend) {
        m_backend->enableWireframe(enable);
    }
}

void Renderer::enableDebugInfo(bool enable) {
    if (m_backend) {
        m_backend->enableDebugInfo(enable);
    }
}

uint32_t Renderer::getDrawCalls() const {
    if (m_backend) {
        return m_backend->getDrawCalls();
    }
    return 0;
}

float Renderer::getFrameTime() const {
    if (m_backend) {
        return m_backend->getFrameTime();
    }
    return 0.0f;
}

// Resource management
void Renderer::reloadShaders() {
    auto& shaderManager = ShaderManager::getInstance();
    shaderManager.reloadAllShaders();
    
    if (m_backend) {
        m_backend->reloadShaders();
    }
}

void Renderer::garbageCollect() {
    if (m_backend) {
        m_backend->garbageCollect();
    }
}

// Backend information
const char* Renderer::getBackendName() const {
    switch (m_graphicsAPI) {
        case GraphicsAPI::OpenGL:
            return "OpenGL";
        case GraphicsAPI::Vulkan:
            return "Vulkan";
        default:
            return "Unknown";
    }
}

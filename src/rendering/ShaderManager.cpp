#include "rendering/ShaderManager.hpp"
#include "core/Logger.hpp"
#include <filesystem>
#include <fstream>

ShaderManager& ShaderManager::getInstance() {
    static ShaderManager instance;
    return instance;
}

ShaderManager::~ShaderManager() {
    m_running = false;
    if (m_fileWatcherThread.joinable()) {
        m_fileWatcherThread.join();
    }
}

std::shared_ptr<Shader> ShaderManager::loadShader(const std::string& name, 
                                                  const std::string& vertexPath, 
                                                  const std::string& fragmentPath) {
    auto shader = std::make_shared<Shader>();
    
    std::string fullVertexPath = m_shaderDirectory + vertexPath;
    std::string fullFragmentPath = m_shaderDirectory + fragmentPath;
    
    if (shader->loadFromFiles(fullVertexPath, fullFragmentPath)) {
        ShaderInfo info;
        info.shader = shader;
        info.vertexPath = fullVertexPath;
        info.fragmentPath = fullFragmentPath;
        info.hasGeometryShader = false;
        info.lastModified = std::chrono::system_clock::now();
        
        m_shaders[name] = info;
        Logger::info("Loaded shader: " + name);
        return shader;
    } else {
        Logger::error("Failed to load shader: " + name);
        return nullptr;
    }
}

std::shared_ptr<Shader> ShaderManager::loadShader(const std::string& name, 
                                                  const std::string& vertexPath, 
                                                  const std::string& fragmentPath,
                                                  const std::string& geometryPath) {
    auto shader = std::make_shared<Shader>();
    
    std::string fullVertexPath = m_shaderDirectory + vertexPath;
    std::string fullFragmentPath = m_shaderDirectory + fragmentPath;
    std::string fullGeometryPath = m_shaderDirectory + geometryPath;
    
    if (shader->loadFromFiles(fullVertexPath, fullFragmentPath, fullGeometryPath)) {
        ShaderInfo info;
        info.shader = shader;
        info.vertexPath = fullVertexPath;
        info.fragmentPath = fullFragmentPath;
        info.geometryPath = fullGeometryPath;
        info.hasGeometryShader = true;
        info.lastModified = std::chrono::system_clock::now();
        
        m_shaders[name] = info;
        Logger::info("Loaded shader with geometry: " + name);
        return shader;
    } else {
        Logger::error("Failed to load shader with geometry: " + name);
        return nullptr;
    }
}

std::shared_ptr<Shader> ShaderManager::getShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second.shader;
    }
    return nullptr;
}

void ShaderManager::unloadShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        Logger::info("Unloaded shader: " + name);
        m_shaders.erase(it);
    }
}

void ShaderManager::enableHotReloading(bool enable) {
    if (enable && !m_running) {
        m_running = true;
        m_hotReloadingEnabled = true;
        m_fileWatcherThread = std::thread(&ShaderManager::fileWatcherLoop, this);
        Logger::info("Shader hot-reloading enabled");
    } else if (!enable && m_running) {
        m_hotReloadingEnabled = false;
        m_running = false;
        if (m_fileWatcherThread.joinable()) {
            m_fileWatcherThread.join();
        }
        Logger::info("Shader hot-reloading disabled");
    }
}

void ShaderManager::setReloadCallback(ReloadCallback callback) {
    m_reloadCallback = callback;
}

void ShaderManager::checkForChanges() {
    for (auto& [name, info] : m_shaders) {
        bool changed = false;
        
        if (hasFileChanged(info.vertexPath, info.lastModified)) {
            changed = true;
        }
        if (hasFileChanged(info.fragmentPath, info.lastModified)) {
            changed = true;
        }
        if (info.hasGeometryShader && hasFileChanged(info.geometryPath, info.lastModified)) {
            changed = true;
        }
        
        if (changed) {
            Logger::info("Reloading shader due to file changes: " + name);
            
            // Reload the shader
            std::shared_ptr<Shader> newShader = std::make_shared<Shader>();
            bool success = false;
            
            if (info.hasGeometryShader) {
                success = newShader->loadFromFiles(info.vertexPath, info.fragmentPath, info.geometryPath);
            } else {
                success = newShader->loadFromFiles(info.vertexPath, info.fragmentPath);
            }
            
            if (success) {
                info.shader = newShader;
                updateShaderTimestamp(name);
                
                if (m_reloadCallback) {
                    m_reloadCallback(name);
                }
                
                Logger::info("Successfully reloaded shader: " + name);
            } else {
                Logger::error("Failed to reload shader: " + name);
            }
        }
    }
}

void ShaderManager::initializeDefaultShaders() {
    Logger::info("Initializing default shader library...");
    
    // Load standard shader set
    loadShader("default", "default.vert", "default.frag");
    loadShader("pbr", "pbr.vert", "pbr.frag");
    loadShader("terrain", "terrain.vert", "terrain.frag");
    loadShader("skybox", "skybox.vert", "skybox.frag");
    loadShader("voxel", "voxel.vert", "voxel.frag");
    
    Logger::info("Default shader library initialized with " + std::to_string(m_shaders.size()) + " shaders");
}

void ShaderManager::reloadAllShaders() {
    Logger::info("Reloading all shaders...");
    
    for (auto& [name, info] : m_shaders) {
        Logger::info("Reloading shader: " + name);
        
        std::shared_ptr<Shader> newShader = std::make_shared<Shader>();
        bool success = false;
        
        if (info.hasGeometryShader) {
            success = newShader->loadFromFiles(info.vertexPath, info.fragmentPath, info.geometryPath);
        } else {
            success = newShader->loadFromFiles(info.vertexPath, info.fragmentPath);
        }
        
        if (success) {
            info.shader = newShader;
            updateShaderTimestamp(name);
            
            if (m_reloadCallback) {
                m_reloadCallback(name);
            }
        } else {
            Logger::error("Failed to reload shader: " + name);
        }
    }
    
    Logger::info("Shader reload complete");
}

void ShaderManager::listLoadedShaders() const {
    Logger::info("Loaded shaders (" + std::to_string(m_shaders.size()) + "):");
    
    for (const auto& [name, info] : m_shaders) {
        std::string shaderType = info.hasGeometryShader ? " (Vertex+Fragment+Geometry)" : " (Vertex+Fragment)";
        Logger::info("  - " + name + shaderType);
    }
}

void ShaderManager::setShaderDirectory(const std::string& directory) {
    m_shaderDirectory = directory;
    if (m_shaderDirectory.back() != '/') {
        m_shaderDirectory += '/';
    }
    Logger::info("Shader directory set to: " + m_shaderDirectory);
}

std::chrono::system_clock::time_point ShaderManager::getFileLastModified(const std::string& filepath) {
    try {
        auto ftime = std::filesystem::last_write_time(filepath);
        return std::chrono::system_clock::from_time_t(std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count());
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::warn("Cannot get modification time for file: " + filepath);
        return std::chrono::system_clock::time_point{};
    }
}

bool ShaderManager::hasFileChanged(const std::string& filepath, const std::chrono::system_clock::time_point& lastCheck) {
    auto currentModified = getFileLastModified(filepath);
    return currentModified > lastCheck;
}

void ShaderManager::updateShaderTimestamp(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        auto& info = it->second;
        
        // Update to the latest timestamp among all files
        auto vertexTime = getFileLastModified(info.vertexPath);
        auto fragmentTime = getFileLastModified(info.fragmentPath);
        auto latest = std::max(vertexTime, fragmentTime);
        
        if (info.hasGeometryShader) {
            auto geometryTime = getFileLastModified(info.geometryPath);
            latest = std::max(latest, geometryTime);
        }
        
        info.lastModified = latest;
    }
}

void ShaderManager::fileWatcherLoop() {
    Logger::debug("Shader file watcher thread started");
    
    while (m_running) {
        if (m_hotReloadingEnabled) {
            checkForChanges();
        }
        
        // Check every 500ms
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    Logger::debug("Shader file watcher thread stopped");
}

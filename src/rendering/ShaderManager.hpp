#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include "Shader.hpp"

class ShaderManager {
public:
    using ReloadCallback = std::function<void(const std::string&)>;
    
    static ShaderManager& getInstance();
    
    // Shader loading and management
    std::shared_ptr<Shader> loadShader(const std::string& name, 
                                      const std::string& vertexPath, 
                                      const std::string& fragmentPath);
    std::shared_ptr<Shader> loadShader(const std::string& name, 
                                      const std::string& vertexPath, 
                                      const std::string& fragmentPath,
                                      const std::string& geometryPath);
    std::shared_ptr<Shader> getShader(const std::string& name);
    void unloadShader(const std::string& name);
    
    // Hot-reloading system
    void enableHotReloading(bool enable);
    void setReloadCallback(ReloadCallback callback);
    void checkForChanges();
    
    // Predefined shader library
    void initializeDefaultShaders();
    std::shared_ptr<Shader> getDefaultShader() { return getShader("default"); }
    std::shared_ptr<Shader> getPBRShader() { return getShader("pbr"); }
    std::shared_ptr<Shader> getTerrainShader() { return getShader("terrain"); }
    std::shared_ptr<Shader> getSkyboxShader() { return getShader("skybox"); }
    std::shared_ptr<Shader> getVoxelShader() { return getShader("voxel"); }
    
    // Utility functions
    void reloadAllShaders();
    void listLoadedShaders() const;
    size_t getShaderCount() const { return m_shaders.size(); }
    
    // Configuration
    void setShaderDirectory(const std::string& directory);
    const std::string& getShaderDirectory() const { return m_shaderDirectory; }
    
    ~ShaderManager();

private:
    ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;
    
    struct ShaderInfo {
        std::shared_ptr<Shader> shader;
        std::string vertexPath;
        std::string fragmentPath;
        std::string geometryPath;
        std::chrono::system_clock::time_point lastModified;
        bool hasGeometryShader = false;
    };
    
    std::unordered_map<std::string, ShaderInfo> m_shaders;
    std::string m_shaderDirectory = "shaders/";
    std::atomic<bool> m_hotReloadingEnabled{false};
    ReloadCallback m_reloadCallback;
    
    // File watching for hot-reloading
    std::chrono::system_clock::time_point getFileLastModified(const std::string& filepath);
    bool hasFileChanged(const std::string& filepath, const std::chrono::system_clock::time_point& lastCheck);
    void updateShaderTimestamp(const std::string& name);
    
    // Background thread for file watching
    std::thread m_fileWatcherThread;
    std::atomic<bool> m_running{false};
    void fileWatcherLoop();
};

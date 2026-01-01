#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

// Forward declarations
class Shader;
class Texture;
class Mesh;
class AudioBuffer;

// Asset types
enum class AssetType {
    Shader,
    Texture,
    Mesh,
    Audio,
    Material
};

// Asset base class
class Asset {
public:
    virtual ~Asset() = default;
    virtual bool load(const std::string& path) = 0;
    virtual void reload() = 0;
    virtual AssetType getType() const = 0;
    virtual const std::string& getPath() const { return m_path; }
    
protected:
    std::string m_path;
    std::atomic<bool> m_loaded{false};
};

// Asset loading callback type
using AssetCallback = std::function<void(const std::string&, std::shared_ptr<Asset>)>;

class AssetManager {
public:
    AssetManager();
    ~AssetManager();
    
    bool initialize();
    void shutdown();
    void update();
    
    // Asset loading and management
    template<typename T>
    std::shared_ptr<T> loadAsset(const std::string& path);
    
    template<typename T>
    std::shared_ptr<T> getAsset(const std::string& path);
    
    void unloadAsset(const std::string& path);
    void unloadAll();
    
    // Hot-reloading
    void enableHotReloading(bool enable);
    bool isHotReloadingEnabled() const { return m_hotReloadingEnabled; }
    
    // Asset callbacks
    void setAssetLoadedCallback(AssetCallback callback) { m_assetLoadedCallback = callback; }
    void setAssetReloadedCallback(AssetCallback callback) { m_assetReloadedCallback = callback; }
    
    // Asset paths and directories
    void addAssetDirectory(const std::string& directory);
    void removeAssetDirectory(const std::string& directory);
    
    // Asset information
    size_t getLoadedAssetCount() const;
    size_t getTotalMemoryUsage() const;
    std::vector<std::string> getLoadedAssets() const;
    
    // Preloading and caching
    bool preloadAsset(const std::string& path);
    void preloadDirectory(const std::string& directory, bool recursive = true);
    
    // Asset validation
    bool validateAsset(const std::string& path) const;
    void validateAllAssets();
    
private:
    // Internal asset management
    void registerAsset(const std::string& path, std::shared_ptr<Asset> asset);
    void unregisterAsset(const std::string& path);
    
    // File watching for hot-reloading
    void startFileWatcher();
    void stopFileWatcher();
    void fileWatcherThread();
    void checkFileChanges();
    
    // Asset creation
    std::shared_ptr<Asset> createAsset(AssetType type, const std::string& path);
    
    // Path utilities
    std::string resolveAssetPath(const std::string& path) const;
    std::string getAssetExtension(const std::string& path) const;
    AssetType getAssetTypeFromExtension(const std::string& extension) const;
    
    // Asset storage
    mutable std::mutex m_assetMutex;
    std::unordered_map<std::string, std::shared_ptr<Asset>> m_assets;
    std::vector<std::string> m_assetDirectories;
    
    // Hot-reloading
    std::atomic<bool> m_hotReloadingEnabled{true};
    std::atomic<bool> m_fileWatcherRunning{false};
    std::thread m_fileWatcherThread;
    std::unordered_map<std::string, std::time_t> m_fileTimestamps;
    
    // Callbacks
    AssetCallback m_assetLoadedCallback;
    AssetCallback m_assetReloadedCallback;
    
    // Statistics
    mutable std::mutex m_statsMutex;
    size_t m_totalMemoryUsage{0};
    
    bool m_initialized{false};
};

// Template implementations
template<typename T>
std::shared_ptr<T> AssetManager::loadAsset(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    
    // Check if asset is already loaded
    auto it = m_assets.find(path);
    if (it != m_assets.end()) {
        return std::dynamic_pointer_cast<T>(it->second);
    }
    
    // Create and load new asset
    auto asset = std::make_shared<T>();
    
    // For Mesh assets, we don't load from file directly
    if constexpr (std::is_same_v<T, Mesh>) {
        registerAsset(path, asset);
        return asset;
    }
    
    if (!asset->load(resolveAssetPath(path))) {
        return nullptr;
    }
    
    registerAsset(path, asset);
    
    // Call loaded callback
    if (m_assetLoadedCallback) {
        m_assetLoadedCallback(path, asset);
    }
    
    return asset;
}

template<typename T>
std::shared_ptr<T> AssetManager::getAsset(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    
    auto it = m_assets.find(path);
    if (it != m_assets.end()) {
        return std::dynamic_pointer_cast<T>(it->second);
    }
    
    return nullptr;
}

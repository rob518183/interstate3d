#include "AssetManager.hpp"
#include "rendering/Shader.hpp"
#include "rendering/Texture.hpp"
#include "rendering/Mesh.hpp"
#include "core/Logger.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>

AssetManager::AssetManager() {
    Logger::info("AssetManager created");
}

AssetManager::~AssetManager() {
    shutdown();
}

bool AssetManager::initialize() {
    Logger::info("Initializing AssetManager...");
    
    // Add default asset directories
    addAssetDirectory("assets");
    addAssetDirectory("shaders");
    addAssetDirectory("textures");
    addAssetDirectory("models");
    addAssetDirectory("audio");
    
    // Start file watcher for hot-reloading
    if (m_hotReloadingEnabled) {
        startFileWatcher();
    }
    
    m_initialized = true;
    Logger::info("AssetManager initialized successfully");
    return true;
}

void AssetManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    Logger::info("Shutting down AssetManager...");
    
    // Stop file watcher
    stopFileWatcher();
    
    // Unload all assets
    unloadAll();
    
    m_initialized = false;
    Logger::info("AssetManager shutdown complete");
}

void AssetManager::update() {
    if (m_hotReloadingEnabled && m_fileWatcherRunning) {
        checkFileChanges();
    }
}

void AssetManager::unloadAsset(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    unregisterAsset(path);
}

void AssetManager::unloadAll() {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    m_assets.clear();
    
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    m_totalMemoryUsage = 0;
    
    Logger::info("All assets unloaded");
}

void AssetManager::enableHotReloading(bool enable) {
    if (m_hotReloadingEnabled == enable) {
        return;
    }
    
    m_hotReloadingEnabled = enable;
    
    if (enable && m_initialized) {
        startFileWatcher();
    } else {
        stopFileWatcher();
    }
    
    Logger::info("Hot-reloading " + std::string(enable ? "enabled" : "disabled"));
}

void AssetManager::addAssetDirectory(const std::string& directory) {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    
    // Check if directory exists
    if (!std::filesystem::exists(directory)) {
        Logger::warn("Asset directory does not exist: " + directory);
        return;
    }
    
    // Check if already added
    for (const auto& dir : m_assetDirectories) {
        if (dir == directory) {
            return;
        }
    }
    
    m_assetDirectories.push_back(directory);
    Logger::info("Added asset directory: " + directory);
}

void AssetManager::removeAssetDirectory(const std::string& directory) {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    
    auto it = std::find(m_assetDirectories.begin(), m_assetDirectories.end(), directory);
    if (it != m_assetDirectories.end()) {
        m_assetDirectories.erase(it);
        Logger::info("Removed asset directory: " + directory);
    }
}

size_t AssetManager::getLoadedAssetCount() const {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    return m_assets.size();
}

size_t AssetManager::getTotalMemoryUsage() const {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    return m_totalMemoryUsage;
}

std::vector<std::string> AssetManager::getLoadedAssets() const {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    
    std::vector<std::string> assets;
    assets.reserve(m_assets.size());
    
    for (const auto& pair : m_assets) {
        assets.push_back(pair.first);
    }
    
    return assets;
}

bool AssetManager::preloadAsset(const std::string& path) {
    AssetType type = getAssetTypeFromExtension(getAssetExtension(path));
    auto asset = createAsset(type, path);
    
    if (!asset) {
        return false;
    }
    
    // For Mesh assets, we don't load from file
    if (type == AssetType::Mesh) {
        registerAsset(path, asset);
        return true;
    }
    
    if (!asset->load(resolveAssetPath(path))) {
        return false;
    }
    
    registerAsset(path, asset);
    return true;
}

void AssetManager::preloadDirectory(const std::string& directory, bool recursive) {
    Logger::info("Preloading directory: " + directory);
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                std::string relativePath = path.substr(directory.length() + 1);
                
                if (validateAsset(relativePath)) {
                    preloadAsset(relativePath);
                }
            } else if (recursive && entry.is_directory()) {
                preloadDirectory(entry.path().string(), recursive);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::error("Failed to preload directory: " + std::string(e.what()));
    }
}

bool AssetManager::validateAsset(const std::string& path) const {
    std::string fullPath = resolveAssetPath(path);
    return std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath);
}

void AssetManager::validateAllAssets() {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    
    std::vector<std::string> invalidAssets;
    
    for (const auto& pair : m_assets) {
        if (!validateAsset(pair.first)) {
            invalidAssets.push_back(pair.first);
        }
    }
    
    // Remove invalid assets
    for (const auto& path : invalidAssets) {
        Logger::warn("Removing invalid asset: " + path);
        m_assets.erase(path);
    }
    
    if (!invalidAssets.empty()) {
        Logger::info("Removed " + std::to_string(invalidAssets.size()) + " invalid assets");
    }
}

void AssetManager::registerAsset(const std::string& path, std::shared_ptr<Asset> asset) {
    m_assets[path] = asset;
    
    // Update file timestamp for hot-reloading
    if (m_hotReloadingEnabled) {
        std::string fullPath = resolveAssetPath(path);
        if (std::filesystem::exists(fullPath)) {
            m_fileTimestamps[path] = std::filesystem::last_write_time(fullPath).time_since_epoch().count();
        }
    }
    
    // Update memory usage (simplified)
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    m_totalMemoryUsage += 1024; // Placeholder calculation
}

void AssetManager::unregisterAsset(const std::string& path) {
    auto it = m_assets.find(path);
    if (it != m_assets.end()) {
        m_assets.erase(it);
        
        // Remove file timestamp
        m_fileTimestamps.erase(path);
        
        // Update memory usage
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_totalMemoryUsage = (m_totalMemoryUsage > 1024) ? m_totalMemoryUsage - 1024 : 0;
    }
}

void AssetManager::startFileWatcher() {
    if (m_fileWatcherRunning) {
        return;
    }
    
    m_fileWatcherRunning = true;
    m_fileWatcherThread = std::thread(&AssetManager::fileWatcherThread, this);
    
    Logger::info("File watcher started for hot-reloading");
}

void AssetManager::stopFileWatcher() {
    if (!m_fileWatcherRunning) {
        return;
    }
    
    m_fileWatcherRunning = false;
    if (m_fileWatcherThread.joinable()) {
        m_fileWatcherThread.join();
    }
    
    Logger::info("File watcher stopped");
}

void AssetManager::fileWatcherThread() {
    while (m_fileWatcherRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        checkFileChanges();
    }
}

void AssetManager::checkFileChanges() {
    std::lock_guard<std::mutex> lock(m_assetMutex);
    
    for (const auto& pair : m_assets) {
        const std::string& path = pair.first;
        std::string fullPath = resolveAssetPath(path);
        
        if (std::filesystem::exists(fullPath)) {
            auto currentTime = std::filesystem::last_write_time(fullPath).time_since_epoch().count();
            auto it = m_fileTimestamps.find(path);
            
            if (it == m_fileTimestamps.end() || it->second < currentTime) {
                // File has changed, reload asset
                Logger::info("Reloading asset: " + path);
                
                auto asset = pair.second;
                if (asset->load(fullPath)) {
                    m_fileTimestamps[path] = currentTime;
                    
                    // Call reloaded callback
                    if (m_assetReloadedCallback) {
                        m_assetReloadedCallback(path, asset);
                    }
                } else {
                    Logger::error("Failed to reload asset: " + path);
                }
            }
        }
    }
}

std::shared_ptr<Asset> AssetManager::createAsset(AssetType type, const std::string& path) {
    switch (type) {
        case AssetType::Shader:
            return std::static_pointer_cast<Asset>(std::make_shared<Shader>());
        case AssetType::Texture:
            return std::static_pointer_cast<Asset>(std::make_shared<Texture>());
        case AssetType::Mesh:
            {
                auto mesh = std::make_shared<Mesh>();
                // For Mesh assets, we don't load from file directly
                // They are typically created programmatically (like voxel terrain)
                return std::static_pointer_cast<Asset>(mesh);
            }
        case AssetType::Audio:
            // TODO: Implement AudioBuffer class
            Logger::warn("Audio assets not yet implemented");
            return nullptr;
        default:
            Logger::error("Unknown asset type for: " + path);
            return nullptr;
    }
}

std::string AssetManager::resolveAssetPath(const std::string& path) const {
    // If path is already absolute, return as is
    if (std::filesystem::path(path).is_absolute()) {
        return path;
    }
    
    // Try to find the asset in registered directories
    for (const auto& directory : m_assetDirectories) {
        std::string fullPath = directory + "/" + path;
        if (std::filesystem::exists(fullPath)) {
            return fullPath;
        }
    }
    
    // Return as is if not found (will be handled by validation)
    return path;
}

std::string AssetManager::getAssetExtension(const std::string& path) const {
    return std::filesystem::path(path).extension().string();
}

AssetType AssetManager::getAssetTypeFromExtension(const std::string& extension) const {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".vert" || ext == ".frag" || ext == ".geom" || ext == ".comp") {
        return AssetType::Shader;
    } else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || 
               ext == ".bmp" || ext == ".hdr" || ext == ".dds") {
        return AssetType::Texture;
    } else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb") {
        return AssetType::Mesh;
    } else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") {
        return AssetType::Audio;
    }
    
    return AssetType::Texture; // Default
}

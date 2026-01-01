#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <functional>
#include <memory>
#include <vector>

using ConfigValue = std::variant<bool, int, float, double, std::string>;

class ConfigManager {
public:
    static ConfigManager& getInstance();
    
    // Configuration loading and saving
    bool loadConfig(const std::string& filepath = "config.json");
    bool saveConfig(const std::string& filepath = "config.json");
    void setDefaults();
    
    // Value accessors
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T{}) const;
    
    template<typename T>
    void setValue(const std::string& key, const T& value);
    
    bool hasKey(const std::string& key) const;
    void removeKey(const std::string& key);
    
    // Category-based access
    std::vector<std::string> getKeysInCategory(const std::string& category) const;
    void removeCategory(const std::string& category);
    
    // Change callbacks
    using ChangeCallback = std::function<void(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue)>;
    void setChangeCallback(const std::string& key, ChangeCallback callback);
    void removeChangeCallback(const std::string& key);
    
    // Validation
    bool validateConfig() const;
    std::vector<std::string> getValidationErrors() const;
    
    // Utility functions
    void printConfig() const;
    void resetToDefaults();
    size_t size() const { return m_configValues.size(); }
    
    // Specific configuration sections
    struct GraphicsConfig {
        int windowWidth = 1920;
        int windowHeight = 1080;
        bool fullscreen = false;
        bool vsync = true;
        int maxFPS = 60;
        float renderScale = 1.0f;
        bool enableShadows = true;
        int shadowQuality = 2; // 0=low, 1=medium, 2=high, 3=ultra
        bool enablePBR = true;
        int textureQuality = 2; // 0=low, 1=medium, 2=high
        float fov = 75.0f;
        float renderDistance = 1000.0f;
    };
    
    struct AudioConfig {
        float masterVolume = 1.0f;
        float musicVolume = 0.8f;
        float sfxVolume = 0.9f;
        float voiceVolume = 1.0f;
        bool enable3DAudio = true;
        int audioDevice = -1; // -1 = default
        int maxChannels = 32;
    };
    
    struct InputConfig {
        float mouseSensitivity = 1.0f;
        bool invertMouseY = false;
        float gamepadSensitivity = 1.0f;
        bool vibrationEnabled = true;
        // Key bindings would go here
    };
    
    struct GameplayConfig {
        float difficulty = 1.0f; // 0.5=easy, 1.0=normal, 1.5=hard, 2.0=very hard
        bool autoAim = false;
        float crosshairSize = 1.0f;
        bool showFPS = true;
        bool showDebugInfo = false;
        std::string language = "en";
    };
    
    // Getters for structured config
    GraphicsConfig getGraphicsConfig() const;
    AudioConfig getAudioConfig() const;
    InputConfig getInputConfig() const;
    GameplayConfig getGameplayConfig() const;
    
    // Setters for structured config
    void setGraphicsConfig(const GraphicsConfig& config);
    void setAudioConfig(const AudioConfig& config);
    void setInputConfig(const InputConfig& config);
    void setGameplayConfig(const GameplayConfig& config);

private:
    ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    std::unordered_map<std::string, ConfigValue> m_configValues;
    std::unordered_map<std::string, ChangeCallback> m_changeCallbacks;
    std::string m_configFilepath;
    
    // Internal helpers
    std::string valueToString(const ConfigValue& value) const;
    ConfigValue stringToValue(const std::string& str, const std::string& type) const;
    void notifyChange(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue);
    void setStructuredDefaults();
    
    // JSON parsing helpers
    bool parseJSON(const std::string& json);
    std::string serializeToJSON() const;
};

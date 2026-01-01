#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

// Simple JSON parser implementation
class SimpleJSONParser {
public:
    static std::unordered_map<std::string, std::string> parse(const std::string& json) {
        std::unordered_map<std::string, std::string> result;
        
        std::string jsonStr = json;
        // Remove whitespace
        jsonStr.erase(std::remove_if(jsonStr.begin(), jsonStr.end(), ::isspace), jsonStr.end());
        
        if (jsonStr.empty() || jsonStr[0] != '{' || jsonStr.back() != '}') {
            return result;
        }
        
        // Remove braces
        jsonStr = jsonStr.substr(1, jsonStr.length() - 2);
        
        size_t pos = 0;
        while (pos < jsonStr.length()) {
            // Find key
            size_t keyStart = jsonStr.find('"', pos);
            if (keyStart == std::string::npos) break;
            
            size_t keyEnd = jsonStr.find('"', keyStart + 1);
            if (keyEnd == std::string::npos) break;
            
            std::string key = jsonStr.substr(keyStart + 1, keyEnd - keyStart - 1);
            
            // Find colon
            size_t colonPos = jsonStr.find(':', keyEnd);
            if (colonPos == std::string::npos) break;
            
            // Find value
            size_t valueStart = colonPos + 1;
            size_t valueEnd = jsonStr.find(',', valueStart);
            if (valueEnd == std::string::npos) {
                valueEnd = jsonStr.length();
            }
            
            std::string value = jsonStr.substr(valueStart, valueEnd - valueStart);
            
            // Remove quotes from string values
            if (!value.empty() && value[0] == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            
            result[key] = value;
            pos = valueEnd + 1;
        }
        
        return result;
    }
    
    static std::string serialize(const std::unordered_map<std::string, std::string>& data) {
        std::ostringstream oss;
        oss << "{\n";
        
        bool first = true;
        for (const auto& [key, value] : data) {
            if (!first) {
                oss << ",\n";
            }
            first = false;
            
            // Determine if value needs quotes
            bool needsQuotes = true;
            if (value == "true" || value == "false") {
                needsQuotes = false;
            } else if (!value.empty() && (std::isdigit(value[0]) || value[0] == '-')) {
                bool isNumber = true;
                for (char c : value) {
                    if (!std::isdigit(c) && c != '.' && c != '-') {
                        isNumber = false;
                        break;
                    }
                }
                if (isNumber) needsQuotes = false;
            }
            
            if (needsQuotes) {
                oss << "  \"" << key << "\": \"" << value << "\"";
            } else {
                oss << "  \"" << key << "\": " << value;
            }
        }
        
        oss << "\n}";
        return oss.str();
    }
};

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig(const std::string& filepath) {
    m_configFilepath = filepath;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::warn("Config file not found: " + filepath + ". Using defaults.");
        setDefaults();
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    bool success = parseJSON(content);
    if (success) {
        Logger::info("Configuration loaded from: " + filepath);
        setStructuredDefaults(); // Fill in any missing values
    } else {
        Logger::error("Failed to parse config file: " + filepath);
        setDefaults();
    }
    
    return success;
}

bool ConfigManager::saveConfig(const std::string& filepath) {
    std::string savePath = filepath.empty() ? m_configFilepath : filepath;
    
    std::ofstream file(savePath);
    if (!file.is_open()) {
        Logger::error("Cannot create config file: " + savePath);
        return false;
    }
    
    std::string json = serializeToJSON();
    file << json;
    file.close();
    
    Logger::info("Configuration saved to: " + savePath);
    return true;
}

void ConfigManager::setDefaults() {
    setStructuredDefaults();
    Logger::info("Default configuration applied");
}

template<typename T>
T ConfigManager::getValue(const std::string& key, const T& defaultValue) const {
    auto it = m_configValues.find(key);
    if (it == m_configValues.end()) {
        return defaultValue;
    }
    
    try {
        return std::get<T>(it->second);
    } catch (const std::bad_variant_access&) {
        Logger::warn("Config value type mismatch for key: " + key);
        return defaultValue;
    }
}

template<typename T>
void ConfigManager::setValue(const std::string& key, const T& value) {
    auto it = m_configValues.find(key);
    ConfigValue oldValue = (it != m_configValues.end()) ? it->second : ConfigValue{};
    
    m_configValues[key] = value;
    
    if (it != m_configValues.end()) {
        notifyChange(key, oldValue, value);
    }
}

bool ConfigManager::hasKey(const std::string& key) const {
    return m_configValues.find(key) != m_configValues.end();
}

void ConfigManager::removeKey(const std::string& key) {
    auto it = m_configValues.find(key);
    if (it != m_configValues.end()) {
        ConfigValue oldValue = it->second;
        m_configValues.erase(it);
        notifyChange(key, oldValue, ConfigValue{});
    }
}

std::vector<std::string> ConfigManager::getKeysInCategory(const std::string& category) const {
    std::vector<std::string> keys;
    std::string prefix = category + ".";
    
    for (const auto& [key, value] : m_configValues) {
        if (key.substr(0, prefix.length()) == prefix) {
            keys.push_back(key.substr(prefix.length()));
        }
    }
    
    return keys;
}

void ConfigManager::removeCategory(const std::string& category) {
    std::vector<std::string> keysToRemove;
    std::string prefix = category + ".";
    
    for (const auto& [key, value] : m_configValues) {
        if (key.substr(0, prefix.length()) == prefix) {
            keysToRemove.push_back(key);
        }
    }
    
    for (const auto& key : keysToRemove) {
        removeKey(key);
    }
}

void ConfigManager::setChangeCallback(const std::string& key, ChangeCallback callback) {
    m_changeCallbacks[key] = callback;
}

void ConfigManager::removeChangeCallback(const std::string& key) {
    m_changeCallbacks.erase(key);
}

bool ConfigManager::validateConfig() const {
    return getValidationErrors().empty();
}

std::vector<std::string> ConfigManager::getValidationErrors() const {
    std::vector<std::string> errors;
    
    // Validate graphics settings
    int width = getValue<int>("graphics.windowWidth", 1920);
    int height = getValue<int>("graphics.windowHeight", 1080);
    if (width < 640 || height < 480) {
        errors.push_back("Invalid resolution: must be at least 640x480");
    }
    
    float fov = getValue<float>("graphics.fov", 75.0f);
    if (fov < 30.0f || fov > 120.0f) {
        errors.push_back("Invalid FOV: must be between 30 and 120 degrees");
    }
    
    // Validate audio settings
    float masterVolume = getValue<float>("audio.masterVolume", 1.0f);
    if (masterVolume < 0.0f || masterVolume > 2.0f) {
        errors.push_back("Invalid master volume: must be between 0.0 and 2.0");
    }
    
    return errors;
}

void ConfigManager::printConfig() const {
    Logger::info("Current configuration:");
    for (const auto& [key, value] : m_configValues) {
        Logger::info("  " + key + " = " + valueToString(value));
    }
}

void ConfigManager::resetToDefaults() {
    m_configValues.clear();
    setDefaults();
}

ConfigManager::GraphicsConfig ConfigManager::getGraphicsConfig() const {
    GraphicsConfig config;
    config.windowWidth = getValue<int>("graphics.windowWidth", 1920);
    config.windowHeight = getValue<int>("graphics.windowHeight", 1080);
    config.fullscreen = getValue<bool>("graphics.fullscreen", false);
    config.vsync = getValue<bool>("graphics.vsync", true);
    config.maxFPS = getValue<int>("graphics.maxFPS", 60);
    config.renderScale = getValue<float>("graphics.renderScale", 1.0f);
    config.enableShadows = getValue<bool>("graphics.enableShadows", true);
    config.shadowQuality = getValue<int>("graphics.shadowQuality", 2);
    config.enablePBR = getValue<bool>("graphics.enablePBR", true);
    config.textureQuality = getValue<int>("graphics.textureQuality", 2);
    config.fov = getValue<float>("graphics.fov", 75.0f);
    config.renderDistance = getValue<float>("graphics.renderDistance", 1000.0f);
    return config;
}

ConfigManager::AudioConfig ConfigManager::getAudioConfig() const {
    AudioConfig config;
    config.masterVolume = getValue<float>("audio.masterVolume", 1.0f);
    config.musicVolume = getValue<float>("audio.musicVolume", 0.8f);
    config.sfxVolume = getValue<float>("audio.sfxVolume", 0.9f);
    config.voiceVolume = getValue<float>("audio.voiceVolume", 1.0f);
    config.enable3DAudio = getValue<bool>("audio.enable3DAudio", true);
    config.audioDevice = getValue<int>("audio.audioDevice", -1);
    config.maxChannels = getValue<int>("audio.maxChannels", 32);
    return config;
}

ConfigManager::InputConfig ConfigManager::getInputConfig() const {
    InputConfig config;
    config.mouseSensitivity = getValue<float>("input.mouseSensitivity", 1.0f);
    config.invertMouseY = getValue<bool>("input.invertMouseY", false);
    config.gamepadSensitivity = getValue<float>("input.gamepadSensitivity", 1.0f);
    config.vibrationEnabled = getValue<bool>("input.vibrationEnabled", true);
    return config;
}

ConfigManager::GameplayConfig ConfigManager::getGameplayConfig() const {
    GameplayConfig config;
    config.difficulty = getValue<float>("gameplay.difficulty", 1.0f);
    config.autoAim = getValue<bool>("gameplay.autoAim", false);
    config.crosshairSize = getValue<float>("gameplay.crosshairSize", 1.0f);
    config.showFPS = getValue<bool>("gameplay.showFPS", true);
    config.showDebugInfo = getValue<bool>("gameplay.showDebugInfo", false);
    config.language = getValue<std::string>("gameplay.language", "en");
    return config;
}

void ConfigManager::setGraphicsConfig(const GraphicsConfig& config) {
    setValue("graphics.windowWidth", config.windowWidth);
    setValue("graphics.windowHeight", config.windowHeight);
    setValue("graphics.fullscreen", config.fullscreen);
    setValue("graphics.vsync", config.vsync);
    setValue("graphics.maxFPS", config.maxFPS);
    setValue("graphics.renderScale", config.renderScale);
    setValue("graphics.enableShadows", config.enableShadows);
    setValue("graphics.shadowQuality", config.shadowQuality);
    setValue("graphics.enablePBR", config.enablePBR);
    setValue("graphics.textureQuality", config.textureQuality);
    setValue("graphics.fov", config.fov);
    setValue("graphics.renderDistance", config.renderDistance);
}

void ConfigManager::setAudioConfig(const AudioConfig& config) {
    setValue("audio.masterVolume", config.masterVolume);
    setValue("audio.musicVolume", config.musicVolume);
    setValue("audio.sfxVolume", config.sfxVolume);
    setValue("audio.voiceVolume", config.voiceVolume);
    setValue("audio.enable3DAudio", config.enable3DAudio);
    setValue("audio.audioDevice", config.audioDevice);
    setValue("audio.maxChannels", config.maxChannels);
}

void ConfigManager::setInputConfig(const InputConfig& config) {
    setValue("input.mouseSensitivity", config.mouseSensitivity);
    setValue("input.invertMouseY", config.invertMouseY);
    setValue("input.gamepadSensitivity", config.gamepadSensitivity);
    setValue("input.vibrationEnabled", config.vibrationEnabled);
}

void ConfigManager::setGameplayConfig(const GameplayConfig& config) {
    setValue("gameplay.difficulty", config.difficulty);
    setValue("gameplay.autoAim", config.autoAim);
    setValue("gameplay.crosshairSize", config.crosshairSize);
    setValue("gameplay.showFPS", config.showFPS);
    setValue("gameplay.showDebugInfo", config.showDebugInfo);
    setValue("gameplay.language", config.language);
}

std::string ConfigManager::valueToString(const ConfigValue& value) const {
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    } else if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<float>(value)) {
        return std::to_string(std::get<float>(value));
    } else if (std::holds_alternative<double>(value)) {
        return std::to_string(std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    return "unknown";
}

void ConfigManager::notifyChange(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue) {
    auto it = m_changeCallbacks.find(key);
    if (it != m_changeCallbacks.end()) {
        it->second(key, oldValue, newValue);
    }
}

void ConfigManager::setStructuredDefaults() {
    GraphicsConfig graphics;
    AudioConfig audio;
    InputConfig input;
    GameplayConfig gameplay;
    
    setGraphicsConfig(graphics);
    setAudioConfig(audio);
    setInputConfig(input);
    setGameplayConfig(gameplay);
}

bool ConfigManager::parseJSON(const std::string& json) {
    auto parsed = SimpleJSONParser::parse(json);
    
    for (const auto& [key, value] : parsed) {
        // Try to determine the type and convert
        if (value == "true" || value == "false") {
            m_configValues[key] = (value == "true");
        } else if (!value.empty() && (std::isdigit(value[0]) || value[0] == '-')) {
            // Check if it's a float or int
            bool isFloat = false;
            for (char c : value) {
                if (c == '.') {
                    isFloat = true;
                    break;
                }
            }
            
            if (isFloat) {
                try {
                    m_configValues[key] = std::stof(value);
                } catch (...) {
                    m_configValues[key] = value;
                }
            } else {
                try {
                    m_configValues[key] = std::stoi(value);
                } catch (...) {
                    m_configValues[key] = value;
                }
            }
        } else {
            m_configValues[key] = value;
        }
    }
    
    return true;
}

std::string ConfigManager::serializeToJSON() const {
    std::unordered_map<std::string, std::string> data;
    
    for (const auto& [key, value] : m_configValues) {
        data[key] = valueToString(value);
    }
    
    return SimpleJSONParser::serialize(data);
}

// Explicit template instantiations
template bool ConfigManager::getValue<bool>(const std::string&, const bool&) const;
template int ConfigManager::getValue<int>(const std::string&, const int&) const;
template float ConfigManager::getValue<float>(const std::string&, const float&) const;
template double ConfigManager::getValue<double>(const std::string&, const double&) const;
template std::string ConfigManager::getValue<std::string>(const std::string&, const std::string&) const;

template void ConfigManager::setValue<bool>(const std::string&, const bool&);
template void ConfigManager::setValue<int>(const std::string&, const int&);
template void ConfigManager::setValue<float>(const std::string&, const float&);
template void ConfigManager::setValue<double>(const std::string&, const double&);
template void ConfigManager::setValue<std::string>(const std::string&, const std::string&);

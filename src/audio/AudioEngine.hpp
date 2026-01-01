#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

class Sound;
class Music;

enum class SoundCategory {
    ENGINE,
    WEAPON,
    EXPLOSION,
    IMPACT,
    AMBIENT,
    UI,
    VOICE
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();
    
    bool initialize();
    void shutdown();
    void update(float deltaTime);
    
    // Sound management
    std::shared_ptr<Sound> loadSound(const std::string& filepath);
    void playSound(const std::shared_ptr<Sound>& sound, const glm::vec3& position = glm::vec3(0.0f));
    void playSound2D(const std::shared_ptr<Sound>& sound, float volume = 1.0f);
    void stopSound(const std::shared_ptr<Sound>& sound);
    void pauseSound(const std::shared_ptr<Sound>& sound);
    void resumeSound(const std::shared_ptr<Sound>& sound);
    
    // Music management
    std::shared_ptr<Music> loadMusic(const std::string& filepath);
    void playMusic(const std::shared_ptr<Music>& music, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void setMusicVolume(float volume);
    float getMusicVolume() const { return m_musicVolume; }
    
    // Audio settings
    void setMasterVolume(float volume);
    float getMasterVolume() const { return m_masterVolume; }
    void setSoundVolume(float volume);
    float getSoundVolume() const { return m_soundVolume; }
    void setListenerPosition(const glm::vec3& position);
    void setListenerOrientation(const glm::vec3& forward, const glm::vec3& up);
    void setListenerVelocity(const glm::vec3& velocity);
    
    // 3D audio
    void setDistanceModel(bool inverse = true);
    void setDopplerFactor(float factor);
    void setSpeedOfSound(float speed);
    
    // Audio categories
    void playSoundCategory(SoundCategory category, const std::shared_ptr<Sound>& sound, 
                         const glm::vec3& position = glm::vec3(0.0f));
    void setCategoryVolume(SoundCategory category, float volume);
    
    // Getters
    bool isInitialized() const { return m_initialized; }
    float getCategoryVolume(SoundCategory category) const;
    
private:
    void updateListener();
    void updateSounds(float deltaTime);
    void cleanupStoppedSounds();
    
    std::vector<std::shared_ptr<Sound>> m_sounds;
    std::shared_ptr<Music> m_currentMusic;
    
    // Audio settings
    float m_masterVolume = 1.0f;
    float m_soundVolume = 1.0f;
    float m_musicVolume = 1.0f;
    
    // Category volumes
    std::unordered_map<SoundCategory, float> m_categoryVolumes;
    
    // Listener properties
    glm::vec3 m_listenerPosition = glm::vec3(0.0f);
    glm::vec3 m_listenerForward = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_listenerUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_listenerVelocity = glm::vec3(0.0f);
    
    // 3D audio settings
    bool m_inverseDistanceModel = true;
    float m_dopplerFactor = 1.0f;
    float m_speedOfSound = 343.3f;
    
    bool m_initialized = false;
};

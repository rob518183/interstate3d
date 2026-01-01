#pragma once

#include <string>
#include <glm/glm.hpp>

// Forward declare SoundCategory enum to avoid multiple definitions
enum class SoundCategory;

class AudioEngine;

class Sound {
public:
    enum class State {
        STOPPED,
        PLAYING,
        PAUSED
    };
    
    Sound();
    ~Sound();
    
    bool loadFromFile(const std::string& filepath);
    void play();
    void pause();
    void stop();
    void resume();
    
    // Properties
    void setVolume(float volume);
    float getVolume() const { return m_volume; }
    float getBaseVolume() const { return m_baseVolume; }
    
    void setPitch(float pitch);
    float getPitch() const { return m_pitch; }
    float getBasePitch() const { return m_basePitch; }
    
    void setLoop(bool loop);
    bool isLooping() const { return m_loop; }
    
    void setPosition(const glm::vec3& position);
    const glm::vec3& getPosition() const { return m_position; }
    
    void setVelocity(const glm::vec3& velocity);
    const glm::vec3& getVelocity() const { return m_velocity; }
    
    void set2D(bool is2D);
    bool is2D() const { return m_is2D; }
    
    // Audio category
    void setCategory(SoundCategory category);
    SoundCategory getCategory() const { return m_category; }
    
    // State
    State getState() const { return m_state; }
    bool isPlaying() const { return m_state == State::PLAYING; }
    bool isPaused() const { return m_state == State::PAUSED; }
    bool isStopped() const { return m_state == State::STOPPED; }
    
    // Duration
    float getDuration() const { return m_duration; }
    float getCurrentTime() const { return m_currentTime; }
    
private:
    void updateState();
    
    // Audio data
    unsigned int m_bufferId = 0;
    unsigned int m_sourceId = 0;
    
    // Properties
    float m_volume = 1.0f;
    float m_baseVolume = 1.0f;
    float m_pitch = 1.0f;
    float m_basePitch = 1.0f;
    bool m_loop = false;
    
    // 3D audio
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_velocity = glm::vec3(0.0f);
    bool m_is2D = false;
    
    // Category
    SoundCategory m_category = static_cast<SoundCategory>(1); // WEAPON
    
    // State
    State m_state = State::STOPPED;
    float m_duration = 0.0f;
    float m_currentTime = 0.0f;
    
    std::string m_filepath;
    bool m_loaded = false;
};

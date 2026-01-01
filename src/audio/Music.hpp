#pragma once

#include <string>

class Music {
public:
    enum class State {
        STOPPED,
        PLAYING,
        PAUSED
    };
    
    Music();
    ~Music();
    
    bool loadFromFile(const std::string& filepath);
    void play();
    void pause();
    void stop();
    void resume();
    
    // Properties
    void setVolume(float volume);
    float getVolume() const { return m_volume; }
    
    void setLoop(bool loop);
    bool isLooping() const { return m_loop; }
    
    // State
    State getState() const { return m_state; }
    bool isPlaying() const { return m_state == State::PLAYING; }
    bool isPaused() const { return m_state == State::PAUSED; }
    bool isStopped() const { return m_state == State::STOPPED; }
    
    // Duration
    float getDuration() const { return m_duration; }
    float getCurrentTime() const { return m_currentTime; }
    void setCurrentTime(float time);
    
    // Fading
    void fadeIn(float duration);
    void fadeOut(float duration);
    bool isFading() const { return m_fadeActive; }
    
private:
    void updateFade(float deltaTime);
    
    // Audio data
    unsigned int m_musicId = 0;
    
    // Properties
    float m_volume = 1.0f;
    bool m_loop = true;
    
    // State
    State m_state = State::STOPPED;
    float m_duration = 0.0f;
    float m_currentTime = 0.0f;
    
    // Fading
    bool m_fadeActive = false;
    float m_fadeDuration = 0.0f;
    float m_fadeTime = 0.0f;
    float m_fadeStartVolume = 0.0f;
    float m_fadeTargetVolume = 0.0f;
    
    std::string m_filepath;
    bool m_loaded = false;
};

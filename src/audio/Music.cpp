#include "audio/Music.hpp"
#include "core/Logger.hpp"
#include <glm/glm.hpp>

Music::Music() = default;

Music::~Music() {
    if (m_loaded) {
        stop();
        // Clean up music resources
        if (m_musicId != 0) {
            // Delete music resource
        }
    }
}

bool Music::loadFromFile(const std::string& filepath) {
    m_filepath = filepath;
    
    // This would load music file using a library like OpenAL, FMOD, or similar
    // For now, we'll simulate loading
    
    Logger::debug("Loading music: " + filepath);
    
    // Simulate music loading
    m_duration = 180.0f; // Default 3 minutes
    m_loaded = true;
    
    Logger::debug("Music loaded successfully: " + filepath);
    return true;
}

void Music::play() {
    if (!m_loaded) {
        Logger::error("Cannot play unloaded music: " + m_filepath);
        return;
    }
    
    if (m_state == State::PLAYING) {
        // Already playing
        return;
    }
    
    // Start playback
    m_state = State::PLAYING;
    if (m_currentTime >= m_duration) {
        m_currentTime = 0.0f;
    }
    
    Logger::debug("Playing music: " + m_filepath);
}

void Music::pause() {
    if (m_state == State::PLAYING) {
        m_state = State::PAUSED;
        Logger::debug("Paused music: " + m_filepath);
    }
}

void Music::stop() {
    if (m_state != State::STOPPED) {
        m_state = State::STOPPED;
        m_currentTime = 0.0f;
        m_fadeActive = false;
        Logger::debug("Stopped music: " + m_filepath);
    }
}

void Music::resume() {
    if (m_state == State::PAUSED) {
        m_state = State::PLAYING;
        Logger::debug("Resumed music: " + m_filepath);
    }
}

void Music::setVolume(float volume) {
    m_volume = glm::clamp(volume, 0.0f, 1.0f);
    
    // Apply volume to music
    if (m_loaded && m_musicId != 0) {
        // Set music volume
    }
}

void Music::setLoop(bool loop) {
    m_loop = loop;
    
    // Apply loop setting to music
    if (m_loaded && m_musicId != 0) {
        // Set music loop
    }
}

void Music::setCurrentTime(float time) {
    m_currentTime = glm::clamp(time, 0.0f, m_duration);
    
    // Seek to new position
    if (m_loaded && m_musicId != 0) {
        // Set music position
    }
}

void Music::fadeIn(float duration) {
    if (!m_loaded) return;
    
    m_fadeActive = true;
    m_fadeDuration = duration;
    m_fadeTime = 0.0f;
    m_fadeStartVolume = 0.0f;
    m_fadeTargetVolume = m_volume;
    
    // Start playing if not already
    if (m_state == State::STOPPED) {
        m_state = State::PLAYING;
        setVolume(0.0f);
    }
    
    Logger::debug("Music fade in started: " + m_filepath);
}

void Music::fadeOut(float duration) {
    if (!m_loaded) return;
    
    m_fadeActive = true;
    m_fadeDuration = duration;
    m_fadeTime = 0.0f;
    m_fadeStartVolume = m_volume;
    m_fadeTargetVolume = 0.0f;
    
    Logger::debug("Music fade out started: " + m_filepath);
}

void Music::updateFade(float deltaTime) {
    if (!m_fadeActive) return;
    
    m_fadeTime += deltaTime;
    float fadeProgress = m_fadeTime / m_fadeDuration;
    
    if (fadeProgress >= 1.0f) {
        // Fade complete
        setVolume(m_fadeTargetVolume);
        m_fadeActive = false;
        
        if (m_fadeTargetVolume <= 0.0f) {
            // Fade out complete, stop music
            stop();
        }
        
        return;
    }
    
    // Update volume during fade
    float currentVolume = glm::mix(m_fadeStartVolume, m_fadeTargetVolume, fadeProgress);
    setVolume(currentVolume);
}

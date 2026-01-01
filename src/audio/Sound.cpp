#include "audio/Sound.hpp"
#include "audio/AudioEngine.hpp"
#include "core/Logger.hpp"

Sound::Sound() = default;

Sound::~Sound() {
    if (m_loaded) {
        stop();
        // Clean up audio resources
        if (m_sourceId != 0) {
            // Delete audio source
        }
        if (m_bufferId != 0) {
            // Delete audio buffer
        }
    }
}

bool Sound::loadFromFile(const std::string& filepath) {
    m_filepath = filepath;
    
    // This would load audio file using a library like OpenAL, FMOD, or similar
    // For now, we'll simulate loading
    
    Logger::debug("Loading sound: " + filepath);
    
    // Simulate audio loading
    m_duration = 2.0f; // Default 2 seconds
    m_loaded = true;
    
    Logger::debug("Sound loaded successfully: " + filepath);
    return true;
}

void Sound::play() {
    if (!m_loaded) {
        Logger::error("Cannot play unloaded sound: " + m_filepath);
        return;
    }
    
    if (m_state == State::PLAYING) {
        // Already playing
        return;
    }
    
    // Start playback
    m_state = State::PLAYING;
    m_currentTime = 0.0f;
    
    Logger::debug("Playing sound: " + m_filepath);
}

void Sound::pause() {
    if (m_state == State::PLAYING) {
        m_state = State::PAUSED;
        Logger::debug("Paused sound: " + m_filepath);
    }
}

void Sound::stop() {
    if (m_state != State::STOPPED) {
        m_state = State::STOPPED;
        m_currentTime = 0.0f;
        Logger::debug("Stopped sound: " + m_filepath);
    }
}

void Sound::resume() {
    if (m_state == State::PAUSED) {
        m_state = State::PLAYING;
        Logger::debug("Resumed sound: " + m_filepath);
    }
}

void Sound::setVolume(float volume) {
    m_volume = glm::clamp(volume, 0.0f, 1.0f);
    
    // Apply volume to audio source
    if (m_loaded && m_sourceId != 0) {
        // Set source volume
    }
}

void Sound::setPitch(float pitch) {
    m_pitch = glm::clamp(pitch, 0.1f, 3.0f);
    
    // Apply pitch to audio source
    if (m_loaded && m_sourceId != 0) {
        // Set source pitch
    }
}

void Sound::setLoop(bool loop) {
    m_loop = loop;
    
    // Apply loop setting to audio source
    if (m_loaded && m_sourceId != 0) {
        // Set source loop
    }
}

void Sound::setPosition(const glm::vec3& position) {
    m_position = position;
    
    // Apply position to audio source
    if (m_loaded && m_sourceId != 0 && !m_is2D) {
        // Set source position
    }
}

void Sound::setVelocity(const glm::vec3& velocity) {
    m_velocity = velocity;
    
    // Apply velocity to audio source
    if (m_loaded && m_sourceId != 0 && !m_is2D) {
        // Set source velocity
    }
}

void Sound::set2D(bool is2D) {
    m_is2D = is2D;
    
    // Update source settings
    if (m_loaded && m_sourceId != 0) {
        if (m_is2D) {
            // Set source as 2D (no spatial audio)
        } else {
            // Set source as 3D (spatial audio)
        }
    }
}

void Sound::setCategory(SoundCategory category) {
    m_category = category;
}

void Sound::updateState() {
    if (m_state == State::PLAYING) {
        m_currentTime += 0.016f; // Assuming 60 FPS
        
        // Check if sound has finished
        if (m_currentTime >= m_duration && !m_loop) {
            m_state = State::STOPPED;
            m_currentTime = 0.0f;
        }
    }
}

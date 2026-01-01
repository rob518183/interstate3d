#include "audio/AudioEngine.hpp"
#include "audio/Sound.hpp"
#include "audio/Music.hpp"
#include "core/Logger.hpp"
#include <unordered_map>
#include <algorithm>

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::initialize() {
    Logger::info("Initializing Audio Engine");
    
    // Initialize category volumes
    m_categoryVolumes[SoundCategory::ENGINE] = 0.8f;
    m_categoryVolumes[SoundCategory::WEAPON] = 1.0f;
    m_categoryVolumes[SoundCategory::EXPLOSION] = 0.9f;
    m_categoryVolumes[SoundCategory::IMPACT] = 0.7f;
    m_categoryVolumes[SoundCategory::AMBIENT] = 0.6f;
    m_categoryVolumes[SoundCategory::UI] = 0.5f;
    m_categoryVolumes[SoundCategory::VOICE] = 1.0f;
    
    m_initialized = true;
    Logger::info("Audio Engine initialized successfully");
    return true;
}

void AudioEngine::shutdown() {
    Logger::info("Shutting down Audio Engine");
    
    // Stop all sounds
    for (auto& sound : m_sounds) {
        if (sound) {
            sound->stop();
        }
    }
    
    // Stop music
    if (m_currentMusic) {
        m_currentMusic->stop();
    }
    
    // Clear all audio resources
    m_sounds.clear();
    m_currentMusic.reset();
    
    m_initialized = false;
}

void AudioEngine::update(float deltaTime) {
    if (!m_initialized) return;
    
    // Update listener
    updateListener();
    
    // Update sounds
    updateSounds(deltaTime);
    
    // Cleanup stopped sounds
    cleanupStoppedSounds();
}

std::shared_ptr<Sound> AudioEngine::loadSound(const std::string& filepath) {
    auto sound = std::make_shared<Sound>();
    if (!sound->loadFromFile(filepath)) {
        Logger::error("Failed to load sound: " + filepath);
        return nullptr;
    }
    
    Logger::debug("Sound loaded: " + filepath);
    return sound;
}

void AudioEngine::playSound(const std::shared_ptr<Sound>& sound, const glm::vec3& position) {
    if (!sound || !m_initialized) return;
    
    sound->setPosition(position);
    sound->play();
    
    // Add to active sounds list
    m_sounds.push_back(sound);
}

void AudioEngine::playSound2D(const std::shared_ptr<Sound>& sound, float volume) {
    if (!sound || !m_initialized) return;
    
    sound->set2D(true);
    sound->setVolume(volume * m_soundVolume * m_masterVolume);
    sound->play();
    
    m_sounds.push_back(sound);
}

void AudioEngine::stopSound(const std::shared_ptr<Sound>& sound) {
    if (sound) {
        sound->stop();
    }
}

void AudioEngine::pauseSound(const std::shared_ptr<Sound>& sound) {
    if (sound) {
        sound->pause();
    }
}

void AudioEngine::resumeSound(const std::shared_ptr<Sound>& sound) {
    if (sound) {
        sound->resume();
    }
}

std::shared_ptr<Music> AudioEngine::loadMusic(const std::string& filepath) {
    auto music = std::make_shared<Music>();
    if (!music->loadFromFile(filepath)) {
        Logger::error("Failed to load music: " + filepath);
        return nullptr;
    }
    
    Logger::debug("Music loaded: " + filepath);
    return music;
}

void AudioEngine::playMusic(const std::shared_ptr<Music>& music, bool loop) {
    if (!music || !m_initialized) return;
    
    // Stop current music
    if (m_currentMusic) {
        m_currentMusic->stop();
    }
    
    m_currentMusic = music;
    m_currentMusic->setLoop(loop);
    m_currentMusic->setVolume(m_musicVolume * m_masterVolume);
    m_currentMusic->play();
}

void AudioEngine::stopMusic() {
    if (m_currentMusic) {
        m_currentMusic->stop();
        m_currentMusic.reset();
    }
}

void AudioEngine::pauseMusic() {
    if (m_currentMusic) {
        m_currentMusic->pause();
    }
}

void AudioEngine::resumeMusic() {
    if (m_currentMusic) {
        m_currentMusic->resume();
    }
}

void AudioEngine::setMusicVolume(float volume) {
    m_musicVolume = glm::clamp(volume, 0.0f, 1.0f);
    
    if (m_currentMusic) {
        m_currentMusic->setVolume(m_musicVolume * m_masterVolume);
    }
}

void AudioEngine::setMasterVolume(float volume) {
    m_masterVolume = glm::clamp(volume, 0.0f, 1.0f);
    
    // Update all sounds
    for (auto& sound : m_sounds) {
        if (sound && sound->isPlaying()) {
            float categoryVolume = getCategoryVolume(sound->getCategory());
            sound->setVolume(sound->getBaseVolume() * categoryVolume * m_soundVolume * m_masterVolume);
        }
    }
    
    // Update music
    if (m_currentMusic) {
        m_currentMusic->setVolume(m_musicVolume * m_masterVolume);
    }
}

void AudioEngine::setSoundVolume(float volume) {
    m_soundVolume = glm::clamp(volume, 0.0f, 1.0f);
    
    // Update all sounds
    for (auto& sound : m_sounds) {
        if (sound && sound->isPlaying()) {
            float categoryVolume = getCategoryVolume(sound->getCategory());
            sound->setVolume(sound->getBaseVolume() * categoryVolume * m_soundVolume * m_masterVolume);
        }
    }
}

void AudioEngine::setListenerPosition(const glm::vec3& position) {
    m_listenerPosition = position;
}

void AudioEngine::setListenerOrientation(const glm::vec3& forward, const glm::vec3& up) {
    m_listenerForward = glm::normalize(forward);
    m_listenerUp = glm::normalize(up);
}

void AudioEngine::setListenerVelocity(const glm::vec3& velocity) {
    m_listenerVelocity = velocity;
}

void AudioEngine::setDistanceModel(bool inverse) {
    m_inverseDistanceModel = inverse;
}

void AudioEngine::setDopplerFactor(float factor) {
    m_dopplerFactor = glm::clamp(factor, 0.0f, 10.0f);
}

void AudioEngine::setSpeedOfSound(float speed) {
    m_speedOfSound = glm::clamp(speed, 1.0f, 1000.0f);
}

void AudioEngine::playSoundCategory(SoundCategory category, const std::shared_ptr<Sound>& sound, 
                                  const glm::vec3& position) {
    if (!sound) return;
    
    sound->setCategory(category);
    float categoryVolume = getCategoryVolume(category);
    sound->setVolume(sound->getBaseVolume() * categoryVolume * m_soundVolume * m_masterVolume);
    
    playSound(sound, position);
}

void AudioEngine::setCategoryVolume(SoundCategory category, float volume) {
    m_categoryVolumes[category] = glm::clamp(volume, 0.0f, 1.0f);
    
    // Update existing sounds in this category
    for (auto& sound : m_sounds) {
        if (sound && sound->getCategory() == category && sound->isPlaying()) {
            sound->setVolume(sound->getBaseVolume() * volume * m_soundVolume * m_masterVolume);
        }
    }
}

float AudioEngine::getCategoryVolume(SoundCategory category) const {
    auto it = m_categoryVolumes.find(category);
    if (it != m_categoryVolumes.end()) {
        return it->second;
    }
    return 1.0f;
}

void AudioEngine::updateListener() {
    // This would update the audio system's listener properties
    // In a real implementation, this would call OpenAL or similar API functions
}

void AudioEngine::updateSounds(float deltaTime) {
    // Update 3D sound positions and apply effects
    for (auto& sound : m_sounds) {
        if (sound && sound->isPlaying() && !sound->is2D()) {
            // Calculate distance attenuation
            float distance = glm::length(sound->getPosition() - m_listenerPosition);
            float attenuation = 1.0f;
            
            if (m_inverseDistanceModel) {
                attenuation = 1.0f / (1.0f + distance * 0.01f);
            } else {
                attenuation = glm::max(0.0f, 1.0f - distance / 100.0f);
            }
            
            // Apply Doppler effect
            if (m_dopplerFactor > 0.0f) {
                glm::vec3 relativeVelocity = sound->getVelocity() - m_listenerVelocity;
                float relativeSpeed = glm::length(relativeVelocity);
                float dopplerShift = 1.0f + (relativeSpeed / m_speedOfSound) * m_dopplerFactor;
                sound->setPitch(sound->getBasePitch() * dopplerShift);
            }
            
            // Apply volume attenuation
            float categoryVolume = getCategoryVolume(sound->getCategory());
            float finalVolume = sound->getBaseVolume() * categoryVolume * m_soundVolume * m_masterVolume * attenuation;
            sound->setVolume(finalVolume);
        }
    }
}

void AudioEngine::cleanupStoppedSounds() {
    // Remove sounds that are no longer playing
    m_sounds.erase(
        std::remove_if(m_sounds.begin(), m_sounds.end(),
            [](const std::shared_ptr<Sound>& sound) {
                return !sound || !sound->isPlaying();
            }),
        m_sounds.end()
    );
}

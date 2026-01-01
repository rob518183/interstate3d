#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <string>

class Vehicle;
class Weapon;
class Renderer;
class Shader;
class Texture;

class HUD {
public:
    HUD();
    ~HUD();
    
    bool initialize();
    void shutdown();
    
    void update(float deltaTime);
    void render(Renderer* renderer);
    
    // Vehicle information
    void setPlayerVehicle(std::shared_ptr<Vehicle> vehicle);
    std::shared_ptr<Vehicle> getPlayerVehicle() const { return m_playerVehicle; }
    
    // HUD elements
    void showSpeedometer(bool show) { m_showSpeedometer = show; }
    void showRPMGauge(bool show) { m_showRPMGauge = show; }
    void showHealthBar(bool show) { m_showHealthBar = show; }
    void showWeaponStatus(bool show) { m_showWeaponStatus = show; }
    void showMinimap(bool show) { m_showMinimap = show; }
    void showCrosshair(bool show) { m_showCrosshair = show; }
    
    // HUD positioning
    void setScreenSize(int width, int height);
    void setScale(float scale) { m_scale = scale; }
    
    // Colors and styling
    void setPrimaryColor(const glm::vec3& color) { m_primaryColor = color; }
    void setSecondaryColor(const glm::vec3& color) { m_secondaryColor = color; }
    void setWarningColor(const glm::vec3& color) { m_warningColor = color; }
    
    // Messages
    void showMessage(const std::string& message, float duration = 3.0f);
    void addDamageIndicator(const glm::vec3& direction, float damage);
    void addKillNotification(const std::string& victim, const std::string& weapon);
    
    // Minimap
    void updateMinimap();
    void setMinimapBounds(const glm::vec3& min, const glm::vec3& max);
    
private:
    void renderSpeedometer();
    void renderRPMGauge();
    void renderHealthBar();
    void renderWeaponStatus();
    void renderMinimap();
    void renderCrosshair();
    void renderMessages();
    void renderDamageIndicators();
    
    void drawText(const std::string& text, const glm::vec2& position, float size, const glm::vec3& color);
    void drawProgressBar(const glm::vec2& position, const glm::vec2& size, float progress, const glm::vec3& color);
    void drawCircle(const glm::vec2& center, float radius, const glm::vec3& color, float thickness = 2.0f);
    void drawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color, float thickness = 2.0f);
    
    struct Message {
        std::string text;
        float timeRemaining;
        glm::vec3 color;
    };
    
    struct DamageIndicator {
        glm::vec3 direction;
        float damage;
        float timeRemaining;
    };
    
    struct KillNotification {
        std::string victim;
        std::string weapon;
        float timeRemaining;
    };
    
    std::shared_ptr<Vehicle> m_playerVehicle;
    
    // Screen properties
    int m_screenWidth = 1920;
    int m_screenHeight = 1080;
    float m_scale = 1.0f;
    
    // Colors
    glm::vec3 m_primaryColor = glm::vec3(0.2f, 0.8f, 0.2f);
    glm::vec3 m_secondaryColor = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 m_warningColor = glm::vec3(1.0f, 0.2f, 0.2f);
    
    // HUD elements visibility
    bool m_showSpeedometer = true;
    bool m_showRPMGauge = true;
    bool m_showHealthBar = true;
    bool m_showWeaponStatus = true;
    bool m_showMinimap = true;
    bool m_showCrosshair = true;
    
    // Messages and notifications
    std::vector<Message> m_messages;
    std::vector<DamageIndicator> m_damageIndicators;
    std::vector<KillNotification> m_killNotifications;
    
    // Minimap
    glm::vec3 m_minimapCenter = glm::vec3(0.0f);
    glm::vec3 m_minimapSize = glm::vec3(500.0f);
    
    // Rendering resources
    std::shared_ptr<Shader> m_hudShader;
    std::shared_ptr<Texture> m_fontTexture;
    std::shared_ptr<Texture> m_hudTexture;
    
    bool m_initialized = false;
};

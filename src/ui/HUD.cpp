#include "ui/HUD.hpp"
#include "game/Vehicle.hpp"
#include "game/Weapon.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Shader.hpp"
#include "rendering/Texture.hpp"
#include "core/Logger.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

HUD::HUD() = default;

HUD::~HUD() {
    shutdown();
}

bool HUD::initialize() {
    Logger::info("Initializing HUD");
    
    // Create HUD shader (simple 2D rendering)
    m_hudShader = std::make_shared<Shader>();
    // This would load a simple 2D shader for UI rendering
    
    // Load font texture
    m_fontTexture = std::make_shared<Texture>();
    m_hudTexture = std::make_shared<Texture>();
    
    m_initialized = true;
    Logger::info("HUD initialized successfully");
    return true;
}

void HUD::shutdown() {
    Logger::info("Shutting down HUD");
    
    m_messages.clear();
    m_damageIndicators.clear();
    m_killNotifications.clear();
    
    m_hudShader.reset();
    m_fontTexture.reset();
    m_hudTexture.reset();
    
    m_playerVehicle.reset();
    
    m_initialized = false;
}

void HUD::update(float deltaTime) {
    if (!m_initialized) return;
    
    // Update messages
    for (auto& message : m_messages) {
        message.timeRemaining -= deltaTime;
    }
    m_messages.erase(
        std::remove_if(m_messages.begin(), m_messages.end(),
            [](const Message& msg) { return msg.timeRemaining <= 0.0f; }),
        m_messages.end()
    );
    
    // Update damage indicators
    for (auto& indicator : m_damageIndicators) {
        indicator.timeRemaining -= deltaTime;
    }
    m_damageIndicators.erase(
        std::remove_if(m_damageIndicators.begin(), m_damageIndicators.end(),
            [](const DamageIndicator& ind) { return ind.timeRemaining <= 0.0f; }),
        m_damageIndicators.end()
    );
    
    // Update kill notifications
    for (auto& notification : m_killNotifications) {
        notification.timeRemaining -= deltaTime;
    }
    m_killNotifications.erase(
        std::remove_if(m_killNotifications.begin(), m_killNotifications.end(),
            [](const KillNotification& notif) { return notif.timeRemaining <= 0.0f; }),
        m_killNotifications.end()
    );
    
    // Update minimap
    if (m_showMinimap) {
        updateMinimap();
    }
}

void HUD::render(Renderer* renderer) {
    if (!m_initialized || !renderer) return;
    
    // Switch to 2D rendering mode
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Render HUD elements
    if (m_showSpeedometer) renderSpeedometer();
    if (m_showRPMGauge) renderRPMGauge();
    if (m_showHealthBar) renderHealthBar();
    if (m_showWeaponStatus) renderWeaponStatus();
    if (m_showMinimap) renderMinimap();
    if (m_showCrosshair) renderCrosshair();
    
    // Render overlays
    renderMessages();
    renderDamageIndicators();
    
    // Restore 3D rendering mode
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void HUD::setPlayerVehicle(std::shared_ptr<Vehicle> vehicle) {
    m_playerVehicle = vehicle;
    Logger::info("Player vehicle set for HUD");
}

void HUD::setScreenSize(int width, int height) {
    m_screenWidth = width;
    m_screenHeight = height;
}

void HUD::showMessage(const std::string& message, float duration) {
    Message msg;
    msg.text = message;
    msg.timeRemaining = duration;
    msg.color = m_secondaryColor;
    
    m_messages.push_back(msg);
    Logger::debug("HUD message: " + message);
}

void HUD::addDamageIndicator(const glm::vec3& direction, float damage) {
    DamageIndicator indicator;
    indicator.direction = glm::normalize(direction);
    indicator.damage = damage;
    indicator.timeRemaining = 2.0f;
    
    m_damageIndicators.push_back(indicator);
}

void HUD::addKillNotification(const std::string& victim, const std::string& weapon) {
    KillNotification notification;
    notification.victim = victim;
    notification.weapon = weapon;
    notification.timeRemaining = 5.0f;
    
    m_killNotifications.push_back(notification);
}

void HUD::updateMinimap() {
    if (!m_playerVehicle) return;
    
    // Update minimap center to follow player
    m_minimapCenter = m_playerVehicle->getPosition();
}

void HUD::setMinimapBounds(const glm::vec3& min, const glm::vec3& max) {
    m_minimapSize = max - min;
}

void HUD::renderSpeedometer() {
    if (!m_playerVehicle) return;
    
    float speed = m_playerVehicle->getSpeed();
    float maxSpeed = 50.0f; // m/s
    
    // Position and size
    glm::vec2 position(m_screenWidth - 200 * m_scale, m_screenHeight - 100 * m_scale);
    glm::vec2 size(150 * m_scale, 20 * m_scale);
    
    // Draw background
    drawProgressBar(position, size, 1.0f, glm::vec3(0.1f, 0.1f, 0.1f));
    
    // Draw speed bar
    float speedRatio = glm::clamp(speed / maxSpeed, 0.0f, 1.0f);
    glm::vec3 speedColor = speedRatio > 0.8f ? m_warningColor : m_primaryColor;
    drawProgressBar(position, size, speedRatio, speedColor);
    
    // Draw text
    drawText("Speed: " + std::to_string(static_cast<int>(speed * 3.6f)) + " km/h", 
             position + glm::vec2(0, -25 * m_scale), 0.8f * m_scale, m_secondaryColor);
}

void HUD::renderRPMGauge() {
    if (!m_playerVehicle) return;
    
    float rpm = m_playerVehicle->getRPM();
    float maxRPM = 7000.0f;
    
    // Position and size
    glm::vec2 position(m_screenWidth - 200 * m_scale, m_screenHeight - 150 * m_scale);
    glm::vec2 size(150 * m_scale, 20 * m_scale);
    
    // Draw background
    drawProgressBar(position, size, 1.0f, glm::vec3(0.1f, 0.1f, 0.1f));
    
    // Draw RPM bar
    float rpmRatio = glm::clamp(rpm / maxRPM, 0.0f, 1.0f);
    glm::vec3 rpmColor = rpmRatio > 0.9f ? m_warningColor : m_primaryColor;
    drawProgressBar(position, size, rpmRatio, rpmColor);
    
    // Draw text
    drawText("RPM: " + std::to_string(static_cast<int>(rpm)), 
             position + glm::vec2(0, -25 * m_scale), 0.8f * m_scale, m_secondaryColor);
}

void HUD::renderHealthBar() {
    if (!m_playerVehicle) return;
    
    float health = m_playerVehicle->getHealth();
    float maxHealth = 100.0f;
    
    // Position and size
    glm::vec2 position(50 * m_scale, m_screenHeight - 100 * m_scale);
    glm::vec2 size(200 * m_scale, 25 * m_scale);
    
    // Draw background
    drawProgressBar(position, size, 1.0f, glm::vec3(0.1f, 0.1f, 0.1f));
    
    // Draw health bar
    float healthRatio = glm::clamp(health / maxHealth, 0.0f, 1.0f);
    glm::vec3 healthColor = healthRatio > 0.5f ? m_primaryColor : m_warningColor;
    drawProgressBar(position, size, healthRatio, healthColor);
    
    // Draw text
    drawText("Health: " + std::to_string(static_cast<int>(health)) + "%", 
             position + glm::vec2(0, -25 * m_scale), 0.8f * m_scale, m_secondaryColor);
    
    // Draw gear indicator
    int gear = m_playerVehicle->getCurrentGear();
    drawText("Gear: " + std::to_string(gear), 
             position + glm::vec2(0, 25 * m_scale), 0.8f * m_scale, m_secondaryColor);
}

void HUD::renderWeaponStatus() {
    if (!m_playerVehicle) return;
    
    // Position
    glm::vec2 position(50 * m_scale, 50 * m_scale);
    
    // This would render weapon information
    // For now, we'll just draw placeholder text
    drawText("Weapons", position, 1.0f * m_scale, m_primaryColor);
    
    // Draw ammo status for each weapon
    position.y += 30 * m_scale;
    drawText("Machine Gun: 200/200", position, 0.8f * m_scale, m_secondaryColor);
    
    position.y += 25 * m_scale;
    drawText("Rockets: 20/20", position, 0.8f * m_scale, m_secondaryColor);
}

void HUD::renderMinimap() {
    // Position and size
    glm::vec2 position(m_screenWidth - 250 * m_scale, 50 * m_scale);
    glm::vec2 size(200 * m_scale, 200 * m_scale);
    
    // Draw minimap background
    drawProgressBar(position, size, 1.0f, glm::vec3(0.1f, 0.1f, 0.2f));
    
    // Draw border
    drawLine(position, position + glm::vec2(size.x, 0), m_secondaryColor, 2.0f * m_scale);
    drawLine(position + glm::vec2(size.x, 0), position + size, m_secondaryColor, 2.0f * m_scale);
    drawLine(position + size, position + glm::vec2(0, size.y), m_secondaryColor, 2.0f * m_scale);
    drawLine(position + glm::vec2(0, size.y), position, m_secondaryColor, 2.0f * m_scale);
    
    // Draw player position
    if (m_playerVehicle) {
        glm::vec2 playerPos = position + size * 0.5f; // Center of minimap
        drawCircle(playerPos, 3.0f * m_scale, m_primaryColor);
    }
    
    // Draw minimap title
    drawText("TACTICAL MAP", position + glm::vec2(0, -25 * m_scale), 0.6f * m_scale, m_secondaryColor);
}

void HUD::renderCrosshair() {
    glm::vec2 center(m_screenWidth * 0.5f, m_screenHeight * 0.5f);
    float size = 20.0f * m_scale;
    float thickness = 2.0f * m_scale;
    
    // Draw crosshair lines
    drawLine(center - glm::vec2(size, 0), center + glm::vec2(size, 0), m_secondaryColor, thickness);
    drawLine(center - glm::vec2(0, size), center + glm::vec2(0, size), m_secondaryColor, thickness);
    
    // Draw center dot
    drawCircle(center, 2.0f * m_scale, m_warningColor);
}

void HUD::renderMessages() {
    glm::vec2 position(m_screenWidth * 0.5f, m_screenHeight * 0.8f);
    
    for (const auto& message : m_messages) {
        // Center text
        glm::vec2 textSize = glm::vec2(message.text.length() * 10 * m_scale, 20 * m_scale);
        glm::vec2 textPos = position - textSize * 0.5f;
        
        drawText(message.text, textPos, 1.0f * m_scale, message.color);
        
        position.y += 30 * m_scale;
    }
}

void HUD::renderDamageIndicators() {
    glm::vec2 center(m_screenWidth * 0.5f, m_screenHeight * 0.5f);
    
    for (const auto& indicator : m_damageIndicators) {
        // Convert 3D direction to 2D screen position
        glm::vec2 screenDirection = glm::vec2(indicator.direction.x, indicator.direction.z);
        screenDirection = glm::normalize(screenDirection);
        
        // Position indicator around crosshair
        glm::vec2 indicatorPos = center + screenDirection * 100.0f * m_scale;
        
        // Draw damage indicator
        float alpha = indicator.timeRemaining / 2.0f; // Fade out
        glm::vec3 color = m_warningColor * alpha;
        
        drawCircle(indicatorPos, 5.0f * m_scale, color);
        drawText(std::to_string(static_cast<int>(indicator.damage)), 
                indicatorPos + glm::vec2(10 * m_scale, 0), 0.8f * m_scale, color);
    }
}

void HUD::drawText(const std::string& text, const glm::vec2& position, float size, const glm::vec3& color) {
    // This would render text using the font texture
    // For now, we'll just log the text rendering
    // In a real implementation, this would use a text rendering library or custom font rendering
    
    // Placeholder: draw text as simple rectangles for each character
    for (size_t i = 0; i < text.length(); ++i) {
        glm::vec2 charPos = position + glm::vec2(i * size * 10.0f, 0);
        drawProgressBar(charPos, glm::vec2(size * 8.0f, size * 12.0f), 1.0f, color);
    }
}

void HUD::drawProgressBar(const glm::vec2& position, const glm::vec2& size, float progress, const glm::vec3& color) {
    // Draw background
    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.1f, 0.1f);
    glVertex2f(position.x, position.y);
    glVertex2f(position.x + size.x, position.y);
    glVertex2f(position.x + size.x, position.y + size.y);
    glVertex2f(position.x, position.y + size.y);
    glEnd();
    
    // Draw progress bar
    if (progress > 0.0f) {
        glBegin(GL_QUADS);
        glColor3f(color.r, color.g, color.b);
        glVertex2f(position.x, position.y);
        glVertex2f(position.x + size.x * progress, position.y);
        glVertex2f(position.x + size.x * progress, position.y + size.y);
        glVertex2f(position.x, position.y + size.y);
        glEnd();
    }
}

void HUD::drawCircle(const glm::vec2& center, float radius, const glm::vec3& color, float thickness) {
    glBegin(GL_LINE_LOOP);
    glColor3f(color.r, color.g, color.b);
    
    int segments = 32;
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        glm::vec2 point = center + glm::vec2(glm::cos(angle), glm::sin(angle)) * radius;
        glVertex2f(point.x, point.y);
    }
    
    glEnd();
}

void HUD::drawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color, float thickness) {
    glBegin(GL_LINES);
    glColor3f(color.r, color.g, color.b);
    glVertex2f(start.x, start.y);
    glVertex2f(end.x, end.y);
    glEnd();
}

#pragma once

#include <memory>
#include <string>

// Forward declarations for graphics API abstraction
enum class GraphicsAPI {
    OpenGL,
    Vulkan
};

class AssetManager;
class Window;
class Renderer;
class PhysicsWorld;
class GameWorld;
class MissionManager;
class AIManager;
class AudioEngine;
class HUD;
class World;
class InputManager;
class ConfigManager;
class VehicleDemo;

class Application {
public:
    Application(GraphicsAPI api = GraphicsAPI::OpenGL);
    ~Application();
    
    bool initialize();
    void run();
    void shutdown();
    
    // Graphics API management
    GraphicsAPI getGraphicsAPI() const { return m_graphicsAPI; }
    void setGraphicsAPI(GraphicsAPI api);
    
    // Asset management
    AssetManager* getAssetManager() const { return m_assetManager.get(); }
    
    // Configuration management
    ConfigManager* getConfigManager() const { return m_configManager; }
    
    // Demo management
    VehicleDemo* getVehicleDemo() const { return m_vehicleDemo.get(); }
    
private:
    void update(float deltaTime);
    void render();
    void handleInput();
    void setupGame();
    void setupWindowCallbacks();
    bool initializeRenderer();
    
    std::unique_ptr<AssetManager> m_assetManager;
    ConfigManager* m_configManager = nullptr;
    std::unique_ptr<VehicleDemo> m_vehicleDemo;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<PhysicsWorld> m_physicsWorld;
    std::shared_ptr<GameWorld> m_gameWorld;
    std::unique_ptr<MissionManager> m_missionManager;
    std::shared_ptr<AIManager> m_aiManager;
    std::unique_ptr<AudioEngine> m_audioEngine;
    std::unique_ptr<HUD> m_hud;
    std::unique_ptr<World> m_world;
    std::unique_ptr<InputManager> m_inputManager;
    
    GraphicsAPI m_graphicsAPI;
    bool m_running = false;
    double m_lastFrameTime = 0.0;
};

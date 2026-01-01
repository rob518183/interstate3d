#include "core/Application.hpp"
#include "core/Logger.hpp"
#include "core/AssetManager.hpp"
#include "core/ConfigManager.hpp"
#include "input/InputManager.hpp"
#include <iostream>
#include "game/GameWorld.hpp"
#include "game/MissionManager.hpp"
#include "game/VehicleDemo.hpp"
#include "ai/AIManager.hpp"
#include "audio/AudioEngine.hpp"
#include "ui/HUD.hpp"
#include "terrain/World.hpp"
#include "windowing/Window.hpp"
#include "rendering/Renderer.hpp"
#include "physics/PhysicsWorld.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/glm.hpp>

Application::Application(GraphicsAPI api) : m_graphicsAPI(api) {
    std::string apiName = (api == GraphicsAPI::Vulkan ? "Vulkan" : "OpenGL");
    Logger::info("Creating Interstate 3D application with " + apiName + " renderer");
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    Logger::info("Initializing application...");
    
    // Initialize configuration manager first
    m_configManager = &ConfigManager::getInstance();
    if (!m_configManager->loadConfig()) {
        Logger::warn("Failed to load config, using defaults");
    }
    
    // Validate configuration
    if (!m_configManager->validateConfig()) {
        auto errors = m_configManager->getValidationErrors();
        for (const auto& error : errors) {
            Logger::error("Config validation error: " + error);
        }
    }
    
    // Get graphics configuration
    auto graphicsConfig = m_configManager->getGraphicsConfig();
    
    // Initialize asset manager first
    m_assetManager = std::make_unique<AssetManager>();
    if (!m_assetManager->initialize()) {
        Logger::error("Failed to initialize asset manager");
        return false;
    }
    
    // Initialize window with config settings
    m_window = std::make_unique<Window>(graphicsConfig.windowWidth, graphicsConfig.windowHeight, "Interstate 3D");
    if (!m_window->initialize()) {
        Logger::error("Failed to initialize window");
        return false;
    }
    
    // Apply window settings from config
    if (graphicsConfig.fullscreen) {
        m_window->setFullscreen(true);
    }
    
    // Set up window event callbacks
    setupWindowCallbacks();
    
    // Initialize input manager
    m_inputManager = std::make_unique<InputManager>();
    if (!m_inputManager->initialize(m_window->getHandle())) {
        Logger::error("Failed to initialize input manager");
        return false;
    }
    
    // Initialize renderer with API abstraction and config settings
    if (!initializeRenderer()) {
        Logger::error("Failed to initialize renderer");
        return false;
    }
    
    // Apply graphics settings from config
    m_renderer->enablePBR(graphicsConfig.enablePBR);
    m_renderer->setViewport(0, 0, graphicsConfig.windowWidth, graphicsConfig.windowHeight);
    
    // Initialize physics world
    m_physicsWorld = std::make_unique<PhysicsWorld>();
    if (!m_physicsWorld->initialize()) {
        Logger::error("Failed to initialize physics world");
        return false;
    }
    
    // Initialize game world
    m_gameWorld = std::make_shared<GameWorld>();
    if (!m_gameWorld->initialize()) {
        Logger::error("Failed to initialize game world");
        return false;
    }
    
    // Initialize AI manager
    m_aiManager = std::make_shared<AIManager>();
    if (!m_aiManager->initialize(m_gameWorld)) {
        Logger::error("Failed to initialize AI manager");
        return false;
    }
    
    // Initialize mission manager
    m_missionManager = std::make_unique<MissionManager>();
    if (!m_missionManager->initialize(m_gameWorld, m_aiManager)) {
        Logger::error("Failed to initialize mission manager");
        return false;
    }
    
    // Initialize world/terrain
    m_world = std::make_unique<World>();
    if (!m_world->initialize()) {
        Logger::error("Failed to initialize world");
        return false;
    }
    
    // Initialize HUD
    m_hud = std::make_unique<HUD>();
    if (!m_hud->initialize()) {
        Logger::error("Failed to initialize HUD");
        return false;
    }
    
    // Initialize audio engine with config settings
    auto audioConfig = m_configManager->getAudioConfig();
    m_audioEngine = std::make_unique<AudioEngine>();
    if (!m_audioEngine->initialize()) {
        Logger::warn("Audio engine failed to initialize - continuing without sound");
    } else {
        // Apply audio settings
        m_audioEngine->setMasterVolume(audioConfig.masterVolume);
        m_audioEngine->setMusicVolume(audioConfig.musicVolume);
        m_audioEngine->setSoundVolume(audioConfig.sfxVolume);
    }
    
    // Setup initial game state
    setupGame();
    
    // Initialize vehicle demo
    m_vehicleDemo = std::make_unique<VehicleDemo>();
    if (!m_vehicleDemo->initialize(m_renderer.get(), m_physicsWorld.get(), m_gameWorld.get())) {
        Logger::error("Failed to initialize vehicle demo");
        return false;
    }
    
    m_running = true;
    m_lastFrameTime = glfwGetTime();
    
    Logger::info("Application initialized successfully");
    return true;
}

void Application::run() {
    Logger::info("Entering main game loop");
    
    // Temporarily disable game loop to isolate crash
    Logger::warn("Game loop temporarily disabled for debugging");
    return;
    
    // Simple text demo to show game is working
    int frameCount = 0;
    double m_deltaTime = 0.016; // 60 FPS
    
    m_running = true;
    while (m_running) {
        // Handle input
        handleInput();
        
        // Update game state
        update(m_deltaTime);
        
        // Render
        render();
        
        // Poll events
        m_window->handleInput();
        
        // Check if window should close
        if (m_window->shouldClose()) {
            m_running = false;
        }
        
        // Swap buffers
        glfwPollEvents();
        m_window->swapBuffers();
        
        frameCount++;
    }
    
    Logger::info("Exiting main game loop");
}

void Application::shutdown() {
    Logger::info("Shutting down application...");
    
    // Save configuration before shutdown
    if (m_configManager) {
        m_configManager->saveConfig();
    }
    
    m_vehicleDemo.reset();
    m_assetManager.reset();
    m_audioEngine.reset();
    m_hud.reset();
    m_world.reset();
    m_missionManager.reset();
    m_aiManager.reset();
    m_gameWorld.reset();
    m_physicsWorld.reset();
    m_renderer.reset();
    m_window.reset();
    
    Logger::info("Application shutdown complete");
}

void Application::update(float deltaTime) {
    // Update asset manager (for hot-reloading)
    m_assetManager->update();
    
    m_physicsWorld->update(deltaTime);
    m_gameWorld->update(deltaTime);
    m_aiManager->update(deltaTime);
    m_missionManager->update(deltaTime);
    m_audioEngine->update(deltaTime);
    m_hud->update(deltaTime);
    m_world->updateTime(deltaTime);
    
    // Update vehicle demo
    if (m_vehicleDemo) {
        m_vehicleDemo->update(deltaTime);
    }
}

void Application::render() {
    m_renderer->beginFrame();
    
    // Render vehicle demo
    if (m_vehicleDemo) {
        m_vehicleDemo->render();
    }
    
    // Render world and terrain
    m_world->renderTerrain(m_renderer.get());
    m_world->renderSkybox(m_renderer.get());
    
    // Render game world (vehicles, projectiles, etc.)
    m_gameWorld->render(m_renderer.get());
    
    // Render HUD
    m_hud->render(m_renderer.get());
    
    m_renderer->endFrame();
}

void Application::handleInput() {
    if (!m_inputManager) return;
    
    // Update input manager
    m_inputManager->update();
    
    // Handle basic input
    if (m_inputManager->isKeyJustPressed(KeyCode::Escape)) {
        m_running = false;
    }
    
    // Handle gamepad input
    if (m_inputManager->isGamepadConnected()) {
        if (m_inputManager->isGamepadButtonJustPressed(GamepadButton::Start)) {
            m_running = false;
        }
    }
    
    // Handle mouse input
    if (m_inputManager->isMouseButtonJustPressed(MouseButton::Left)) {
        glm::vec2 mousePos = m_inputManager->getMousePosition();
        (void)mousePos; // Suppress unused variable warning
    }
}

void Application::setupWindowCallbacks() {
// Window callbacks - temporarily disabled due to crash
// m_window->setFramebufferSizeCallback([this](int width, int height) {
//     Logger::info("Framebuffer resized: " + std::to_string(width) + "x" + std::to_string(height));
//     if (m_renderer) {
//         m_renderer->setViewport(0, 0, width, height);
//     }
// });
    
// Window resize callback
// m_window->setWindowSizeCallback([this](int width, int height) {
//     Logger::info("Window resized: " + std::to_string(width) + "x" + std::to_string(height));
// });
    
// Window position callback - temporarily disabled due to crash
// m_window->setWindowPosCallback([this](int xpos, int ypos) {
//     Logger::debug("Window moved to: " + std::to_string(xpos) + ", " + std::to_string(ypos));
// });
    
// Window close callback
// m_window->setWindowCloseCallback([this]() {
//     Logger::info("Window close requested");
//     m_running = false;
// });
    
// Window focus callback
// m_window->setWindowFocusCallback([this](bool focused) {
//     Logger::info("Window " + std::string(focused ? "focused" : "unfocused"));
// });    // Could pause game when unfocused
}

void Application::setupGame() {
Logger::info("Setting up initial game state");
    
// Create player vehicle
auto playerVehicle = m_gameWorld->spawnVehicle(glm::vec3(0.0f, 10.0f, 0.0f), "player");
if (playerVehicle) {
// Set team - placeholder for Vehicle::setTeam method
// playerVehicle->setTeam(0); // Player team
m_gameWorld->setPlayerVehicle(playerVehicle);
        
// Set HUD player vehicle - placeholder for HUD::setPlayerVehicle method
// m_hud->setPlayerVehicle(playerVehicle);
        
Logger::info("Player vehicle created");
}
    
// Add some initial missions
auto deathmatchMission = m_missionManager->createDeathmatchMission("Training Mission", 5);
m_missionManager->addMission(deathmatchMission);
    
auto teamBattleMission = m_missionManager->createDeathmatchMission("Team Battle", 10);
m_missionManager->addMission(teamBattleMission);
    
// Start the first mission
m_missionManager->startMission("Training Mission");
    
// Spawn some enemy vehicles for testing
m_aiManager->spawnEnemyVehicle(glm::vec3(50.0f, 10.0f, 50.0f), 1);
m_aiManager->spawnEnemyVehicle(glm::vec3(-50.0f, 10.0f, 50.0f), 1);
m_aiManager->spawnAllyVehicle(glm::vec3(0.0f, 10.0f, 50.0f), 2);
    
Logger::info("Game setup complete");
}

bool Application::initializeRenderer() {
    std::string apiName = (m_graphicsAPI == GraphicsAPI::Vulkan ? "Vulkan" : "OpenGL");
    Logger::info("Initializing renderer with " + apiName + " backend");
    
    // Temporarily disable renderer to isolate crash
    Logger::warn("Renderer temporarily disabled for debugging");
    return true;
    
    m_renderer = std::make_unique<Renderer>(m_graphicsAPI);
    if (!m_renderer->initialize(m_window->getHandle())) {
        Logger::error("Failed to initialize renderer");
        return false;
    }
    
    Logger::info("Renderer initialized successfully");
    return true;
}

void Application::setGraphicsAPI(GraphicsAPI api) {
    if (m_graphicsAPI == api) {
        return; // No change needed
    }
    
    std::string apiName = (api == GraphicsAPI::Vulkan ? "Vulkan" : "OpenGL");
    Logger::info("Switching graphics API to " + apiName);
    
    // Note: In a full implementation, this would require recreating the renderer
    // and all graphics resources. For now, this is a placeholder.
    m_graphicsAPI = api;
}

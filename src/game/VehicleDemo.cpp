#include "game/VehicleDemo.hpp"
#include "rendering/Renderer.hpp"
#include "physics/PhysicsWorld.hpp"
#include "game/GameWorld.hpp"
#include "game/Vehicle.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Shader.hpp"
#include "rendering/Mesh.hpp"
#include "rendering/Texture.hpp"
#include "core/Logger.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <algorithm>

VehicleDemo::VehicleDemo() {
    Logger::info("Creating Vehicle Demo");
}

VehicleDemo::~VehicleDemo() {
    shutdown();
}

bool VehicleDemo::initialize(Renderer* renderer, PhysicsWorld* physics, GameWorld* gameWorld) {
    if (!renderer || !physics || !gameWorld) {
        Logger::error("VehicleDemo: Invalid system pointers");
        return false;
    }
    
    m_renderer = renderer;
    m_physics = physics;
    m_gameWorld = gameWorld;
    
    Logger::info("Initializing Vehicle Demo...");
    
    if (!loadAssets()) {
        Logger::error("Failed to load demo assets");
        return false;
    }
    
    if (!createTerrain()) {
        Logger::error("Failed to create demo terrain");
        return false;
    }
    
    if (!createObstacles()) {
        Logger::error("Failed to create demo obstacles");
        return false;
    }
    
    if (!setupCamera()) {
        Logger::error("Failed to setup demo camera");
        return false;
    }
    
    // Create initial demo vehicles
    spawnTestVehicles();
    
    // Create physics playground
    createPhysicsPlayground();
    
    m_initialized = true;
    Logger::info("Vehicle Demo initialized successfully");
    return true;
}

void VehicleDemo::shutdown() {
    if (!m_initialized) return;
    
    Logger::info("Shutting down Vehicle Demo...");
    
    clearAllVehicles();
    
    m_terrainMeshes.clear();
    m_obstacleMeshes.clear();
    m_terrainTexture.reset();
    m_vehicleTexture.reset();
    m_camera.reset();
    
    m_initialized = false;
    Logger::info("Vehicle Demo shutdown complete");
}

void VehicleDemo::update(float deltaTime) {
    if (!m_initialized) return;
    
    m_demoTime += deltaTime;
    
    updateVehicles(deltaTime);
    updateCamera(deltaTime);
    
    // Run demo scenarios
    if (m_demoTime < 10.0f) {
        runVehicleSpawningDemo();
    } else if (m_demoTime < 20.0f) {
        runPhysicsCollisionDemo();
    } else if (m_demoTime < 30.0f) {
        runVehicleCombatDemo();
    } else {
        runTerrainNavigationDemo();
    }
}

void VehicleDemo::render() {
    if (!m_initialized) return;
    
    // Render terrain
    for (const auto& mesh : m_terrainMeshes) {
        if (mesh) {
            m_renderer->renderMesh(mesh, glm::mat4(1.0f));
        }
    }
    
    // Render obstacles
    for (const auto& mesh : m_obstacleMeshes) {
        if (mesh) {
            m_renderer->renderMesh(mesh, glm::mat4(1.0f));
        }
    }
    
    // Render vehicles
    renderVehicles();
}

void VehicleDemo::spawnPlayerVehicle() {
    glm::vec3 spawnPos(0.0f, 2.0f, 0.0f);
    createDemoVehicle(spawnPos, glm::vec3(0.2f, 0.4f, 0.8f), "Player", true);
}

void VehicleDemo::spawnEnemyVehicle(const glm::vec3& position) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> colorDist(0.3f, 1.0f);
    
    glm::vec3 color(colorDist(gen), colorDist(gen), colorDist(gen));
    createDemoVehicle(position, color, "Enemy_" + std::to_string(m_vehicles.size()), false);
}

void VehicleDemo::spawnTestVehicles() {
    Logger::info("Spawning test vehicles for demo");
    
    // Spawn player vehicle
    spawnPlayerVehicle();
    
    // Spawn enemy vehicles in various positions
    spawnEnemyVehicle(glm::vec3(10.0f, 2.0f, 0.0f));
    spawnEnemyVehicle(glm::vec3(-10.0f, 2.0f, 0.0f));
    spawnEnemyVehicle(glm::vec3(0.0f, 2.0f, 10.0f));
    spawnEnemyVehicle(glm::vec3(0.0f, 2.0f, -10.0f));
    
    // Spawn vehicles at different heights for physics testing
    spawnEnemyVehicle(glm::vec3(20.0f, 10.0f, 20.0f));
    spawnEnemyVehicle(glm::vec3(-20.0f, 15.0f, -20.0f));
    
    Logger::info("Spawned " + std::to_string(m_vehicles.size()) + " vehicles");
}

void VehicleDemo::clearAllVehicles() {
    Logger::info("Clearing all demo vehicles");
    
    for (auto& demoVehicle : m_vehicles) {
        if (demoVehicle.vehicle) {
            m_physics->removeVehicle(demoVehicle.vehicle);
            m_gameWorld->removeVehicle(demoVehicle.vehicle);
        }
    }
    
    m_vehicles.clear();
    m_playerVehicle.reset();
}

void VehicleDemo::setCameraMode(int mode) {
    m_cameraMode = glm::clamp(mode, 0, 2);
    Logger::info("Camera mode set to: " + std::to_string(m_cameraMode));
}

void VehicleDemo::updateCamera(float deltaTime) {
    switch (m_cameraMode) {
        case 0:
            updateFollowCamera(deltaTime);
            break;
        case 1:
            updateOrbitCamera(deltaTime);
            break;
        case 2:
            updateFixedCamera();
            break;
    }
    
    if (m_camera) {
        m_renderer->setCamera(m_camera);
    }
}

void VehicleDemo::runPhysicsTests() {
    Logger::info("Running physics tests");
    m_physicsTestRunning = true;
    m_physicsTestTime = 0.0f;
}

void VehicleDemo::createPhysicsPlayground() {
    Logger::info("Creating physics playground");
    
    createRamp();
    createJump();
    createTunnel();
    createDestructibleWall();
}

bool VehicleDemo::loadAssets() {
    Logger::info("Loading demo assets");
    
    // Load or create textures
    // For demo purposes, we'll create simple colored textures
    // In a real implementation, these would be loaded from files
    
    Logger::info("Demo assets loaded");
    return true;
}

bool VehicleDemo::createTerrain() {
    Logger::info("Creating demo terrain");
    
    // Create a large flat ground plane
    // In a real implementation, this would use proper terrain generation
    
    Logger::info("Demo terrain created");
    return true;
}

bool VehicleDemo::createObstacles() {
    Logger::info("Creating demo obstacles");
    
    // Create various obstacles for testing
    // In a real implementation, these would be proper 3D meshes
    
    Logger::info("Demo obstacles created");
    return true;
}

bool VehicleDemo::setupCamera() {
    Logger::info("Setting up demo camera");
    
    // Create camera with good initial position
    m_camera = std::make_shared<Camera>();
    m_camera->setPosition(glm::vec3(0.0f, 10.0f, 20.0f));
    m_camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    m_camera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
    
    Logger::info("Demo camera setup complete");
    return true;
}

void VehicleDemo::createDemoVehicle(const glm::vec3& position, const glm::vec3& color, 
                                 const std::string& name, bool isPlayer) {
    auto vehicle = m_gameWorld->spawnVehicle(position, name);
    if (!vehicle) {
        Logger::error("Failed to spawn demo vehicle: " + name);
        return;
    }
    
    // Set vehicle properties
    vehicle->setColor(color);
    
    // Add to physics world
    m_physics->addVehicle(vehicle);
    
    // Create demo vehicle record
    DemoVehicle demoVehicle;
    demoVehicle.vehicle = vehicle;
    demoVehicle.color = color;
    demoVehicle.name = name;
    demoVehicle.isPlayer = isPlayer;
    demoVehicle.spawnTime = m_demoTime;
    
    m_vehicles.push_back(demoVehicle);
    
    if (isPlayer) {
        m_playerVehicle = vehicle;
        m_gameWorld->setPlayerVehicle(vehicle);
    }
    
    Logger::info("Created demo vehicle: " + name + " at position (" + 
                 std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + 
                 std::to_string(position.z) + ")");
}

void VehicleDemo::updateVehicles(float deltaTime) {
    for (auto& demoVehicle : m_vehicles) {
        if (!demoVehicle.vehicle) continue;
        
        // Update vehicle physics
        demoVehicle.vehicle->updatePhysics(deltaTime);
        
        // Apply some demo behaviors
        if (!demoVehicle.isPlayer) {
            // Simple AI behavior for demo vehicles
            float time = m_demoTime - demoVehicle.spawnTime;
            
            // Make vehicles move in patterns
            if (time < 5.0f) {
                // Move forward
                demoVehicle.vehicle->setThrottle(0.5f);
            } else if (time < 10.0f) {
                // Turn and move
                demoVehicle.vehicle->setSteering(0.3f);
                demoVehicle.vehicle->setThrottle(0.3f);
            } else {
                // Random movement
                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_real_distribution<float> steerDist(-0.5f, 0.5f);
                static std::uniform_real_distribution<float> throttleDist(0.0f, 0.7f);
                
                demoVehicle.vehicle->setSteering(steerDist(gen));
                demoVehicle.vehicle->setThrottle(throttleDist(gen));
            }
        }
    }
}

void VehicleDemo::renderVehicles() {
    for (const auto& demoVehicle : m_vehicles) {
        if (!demoVehicle.vehicle) continue;
        
        // Get vehicle transform and render
        glm::mat4 transform = demoVehicle.vehicle->getTransform();
        
        // In a real implementation, this would render the vehicle mesh
        // For now, we'll use placeholder rendering
        // m_renderer->renderMesh(demoVehicle.vehicle->getMesh(), transform);
    }
}

void VehicleDemo::createRamp() {
    Logger::info("Creating physics ramp");
    
    // Create a ramp for vehicles to drive up
    // In a real implementation, this would create a proper collision shape
}

void VehicleDemo::createJump() {
    Logger::info("Creating physics jump");
    
    // Create a jump for vehicles to test physics
    // In a real implementation, this would create a proper collision shape
}

void VehicleDemo::createTunnel() {
    Logger::info("Creating physics tunnel");
    
    // Create a tunnel for vehicles to drive through
    // In a real implementation, this would create proper collision shapes
}

void VehicleDemo::createDestructibleWall() {
    Logger::info("Creating destructible wall");
    
    // Create a wall that can be destroyed
    // In a real implementation, this would create destructible physics objects
}

void VehicleDemo::updateFollowCamera(float deltaTime) {
    if (!m_playerVehicle) return;
    
    glm::vec3 vehiclePos = m_playerVehicle->getPosition();
    glm::vec3 vehicleForward = m_playerVehicle->getForwardVector();
    
    // Position camera behind and above vehicle
    glm::vec3 desiredPos = vehiclePos - vehicleForward * m_cameraDistance + glm::vec3(0, m_cameraHeight, 0);
    
    // Smooth camera movement
    glm::vec3 currentPos = m_camera->getPosition();
    glm::vec3 newPos = glm::mix(currentPos, desiredPos, deltaTime * 2.0f);
    
    m_camera->setPosition(newPos);
    m_camera->setTarget(vehiclePos);
}

void VehicleDemo::updateOrbitCamera(float deltaTime) {
    if (!m_playerVehicle) return;
    
    glm::vec3 vehiclePos = m_playerVehicle->getPosition();
    
    // Rotate camera around vehicle
    m_cameraAngle += deltaTime * 0.5f; // 0.5 radians per second
    
    glm::vec3 cameraPos;
    cameraPos.x = vehiclePos.x + cos(m_cameraAngle) * m_cameraDistance;
    cameraPos.y = vehiclePos.y + m_cameraHeight;
    cameraPos.z = vehiclePos.z + sin(m_cameraAngle) * m_cameraDistance;
    
    m_camera->setPosition(cameraPos);
    m_camera->setTarget(vehiclePos);
}

void VehicleDemo::updateFixedCamera() {
    // Fixed camera position
    glm::vec3 fixedPos(25.0f, 15.0f, 25.0f);
    glm::vec3 target(0.0f, 0.0f, 0.0f);
    
    m_camera->setPosition(fixedPos);
    m_camera->setTarget(target);
}

void VehicleDemo::runVehicleSpawningDemo() {
    // Spawn new vehicles periodically during this phase
    static float lastSpawnTime = 0.0f;
    
    if (m_demoTime - lastSpawnTime > 2.0f && m_vehicles.size() < 10) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> xDist(-30.0f, 30.0f);
        std::uniform_real_distribution<float> zDist(-30.0f, 30.0f);
        
        glm::vec3 spawnPos(xDist(gen), 5.0f, zDist(gen));
        spawnEnemyVehicle(spawnPos);
        
        lastSpawnTime = m_demoTime;
    }
}

void VehicleDemo::runPhysicsCollisionDemo() {
    // Test physics collisions
    static bool collisionTestStarted = false;
    
    if (!collisionTestStarted) {
        Logger::info("Starting collision demo");
        
        // Make vehicles drive towards each other
        for (auto& demoVehicle : m_vehicles) {
            if (demoVehicle.vehicle) {
                demoVehicle.vehicle->setThrottle(0.8f);
            }
        }
        
        collisionTestStarted = true;
    }
}

void VehicleDemo::runVehicleCombatDemo() {
    // Test vehicle combat
    Logger::info("Starting combat demo");
    
    // Make vehicles shoot at each other
    for (auto& demoVehicle : m_vehicles) {
        if (demoVehicle.vehicle && !demoVehicle.isPlayer) {
            // Simple combat AI
            demoVehicle.vehicle->setThrottle(0.3f);
            // In a real implementation, this would handle weapons
        }
    }
}

void VehicleDemo::runTerrainNavigationDemo() {
    // Test terrain navigation
    Logger::info("Starting terrain navigation demo");
    
    // Make vehicles navigate the terrain
    for (auto& demoVehicle : m_vehicles) {
        if (demoVehicle.vehicle) {
            // Navigate to different points on the terrain
            demoVehicle.vehicle->setThrottle(0.6f);
        }
    }
}

#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Renderer;
class Vehicle;
class PhysicsWorld;
class GameWorld;
class Camera;
class Shader;
class Mesh;
class Texture;

class VehicleDemo {
public:
    VehicleDemo();
    ~VehicleDemo();
    
    bool initialize(Renderer* renderer, PhysicsWorld* physics, GameWorld* gameWorld);
    void shutdown();
    
    void update(float deltaTime);
    void render();
    
    // Demo controls
    void spawnPlayerVehicle();
    void spawnEnemyVehicle(const glm::vec3& position);
    void spawnTestVehicles();
    void clearAllVehicles();
    
    // Camera controls
    void setCameraMode(int mode); // 0=follow, 1=orbit, 2=fixed
    void updateCamera(float deltaTime);
    
    // Physics testing
    void runPhysicsTests();
    void createPhysicsPlayground();
    
    // Getters
    bool isInitialized() const { return m_initialized; }
    size_t getVehicleCount() const { return m_vehicles.size(); }
    
private:
    struct DemoVehicle {
        std::shared_ptr<Vehicle> vehicle;
        glm::vec3 color;
        std::string name;
        bool isPlayer;
        float spawnTime;
    };
    
    // Core systems
    Renderer* m_renderer = nullptr;
    PhysicsWorld* m_physics = nullptr;
    GameWorld* m_gameWorld = nullptr;
    
    // Demo vehicles
    std::vector<DemoVehicle> m_vehicles;
    std::shared_ptr<Vehicle> m_playerVehicle;
    
    // Camera
    std::shared_ptr<Camera> m_camera;
    int m_cameraMode = 0; // follow mode
    float m_cameraDistance = 10.0f;
    float m_cameraHeight = 5.0f;
    float m_cameraAngle = 0.0f;
    
    // Demo environment
    std::vector<std::shared_ptr<Mesh>> m_terrainMeshes;
    std::vector<std::shared_ptr<Mesh>> m_obstacleMeshes;
    std::shared_ptr<Texture> m_terrainTexture;
    std::shared_ptr<Texture> m_vehicleTexture;
    
    // Demo state
    bool m_initialized = false;
    float m_demoTime = 0.0f;
    bool m_physicsTestRunning = false;
    float m_physicsTestTime = 0.0f;
    
    // Demo settings
    glm::vec3 m_spawnAreaMin = glm::vec3(-50.0f, 0.0f, -50.0f);
    glm::vec3 m_spawnAreaMax = glm::vec3(50.0f, 0.0f, 50.0f);
    
    // Initialization helpers
    bool loadAssets();
    bool createTerrain();
    bool createObstacles();
    bool setupCamera();
    
    // Vehicle helpers
    void createDemoVehicle(const glm::vec3& position, const glm::vec3& color, 
                         const std::string& name, bool isPlayer = false);
    void updateVehicles(float deltaTime);
    void renderVehicles();
    
    // Physics helpers
    void createRamp();
    void createJump();
    void createTunnel();
    void createDestructibleWall();
    
    // Camera helpers
    void updateFollowCamera(float deltaTime);
    void updateOrbitCamera(float deltaTime);
    void updateFixedCamera();
    
    // Demo scenarios
    void runVehicleSpawningDemo();
    void runPhysicsCollisionDemo();
    void runVehicleCombatDemo();
    void runTerrainNavigationDemo();
};

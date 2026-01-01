#pragma once

#include "RenderBackend.hpp"
#include <memory>
#include <vector>
#include <optional>
#include <vulkan/vulkan.h>

class Shader;
class Mesh;
class Texture;
class Camera;
class Light;

// Helper structs for Vulkan backend
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanBackend : public RenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;
    
    // Core initialization and lifecycle
    bool initialize(GLFWwindow* window) override;
    void shutdown() override;
    void beginFrame() override;
    void endFrame() override;
    
    // Rendering operations
    void setCamera(const std::shared_ptr<Camera>& camera) override;
    void addLight(const std::shared_ptr<Light>& light) override;
    void clearLights() override;
    
    void renderMesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& modelMatrix) override;
    void renderSkybox() override;
    void renderTerrain(const std::vector<std::shared_ptr<Mesh>>& terrainChunks) override;
    
    // State management
    void setClearColor(const glm::vec3& color) override;
    void enableDepthTest(bool enable) override;
    void enableCulling(bool enable) override;
    void setViewport(int x, int y, int width, int height) override;
    
    // Advanced features for Interstate 3D
    void enablePBR(bool enable) override;
    void setEnvironmentMap(const std::shared_ptr<Texture>& cubemap) override;
    void renderVoxels(const std::vector<glm::vec3>& voxelPositions, 
                     const std::vector<glm::vec4>& voxelColors) override;
    
    // Performance and debugging
    void enableWireframe(bool enable) override;
    void enableDebugInfo(bool enable) override;
    uint32_t getDrawCalls() const override;
    float getFrameTime() const override;
    
    // Resource management
    void reloadShaders() override;
    void garbageCollect() override;
    
    // Shader hot-reloading callback
    void onShaderReloaded(const std::string& shaderName) override;
    
private:
    // Vulkan initialization
    bool createInstance();
    bool setupDebugMessenger();
    bool createSurface();
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createSwapChain();
    bool createImageViews();
    bool createRenderPass();
    bool createDescriptorSetLayout();
    bool createGraphicsPipeline();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createSyncObjects();
    
    // Vulkan helper functions
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    
    // Cleanup helpers
    void cleanupSwapChain();
    void cleanup();
    
    // Vulkan core objects
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;
    
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    size_t m_currentFrame = 0;
    
    // Application state
    GLFWwindow* m_window = nullptr;
    std::shared_ptr<Camera> m_camera;
    std::vector<std::shared_ptr<Light>> m_lights;
    
    // Rendering state
    glm::vec3 m_clearColor = glm::vec3(0.2f, 0.5f, 0.8f);
    bool m_depthTestEnabled = true;
    bool m_cullingEnabled = true;
    bool m_wireframeEnabled = false;
    bool m_pbrEnabled = false;
    bool m_debugInfoEnabled = false;
    
    // Performance tracking
    uint32_t m_drawCalls = 0;
    float m_frameTime = 0.0f;
    double m_lastFrameTime = 0.0;
    
    // Constants
    const int MAX_FRAMES_IN_FLIGHT = 2;
    const std::vector<const char*> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    bool m_initialized = false;
    bool m_enableValidationLayers = false;
};

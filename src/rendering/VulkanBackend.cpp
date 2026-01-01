#include "rendering/VulkanBackend.hpp"
#include "core/Logger.hpp"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>

// Static debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    std::string severity;
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        severity = "ERROR";
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        severity = "WARNING";
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        severity = "INFO";
    } else {
        severity = "VERBOSE";
    }
    
    Logger::debug("[Vulkan " + severity + "] " + std::string(pCallbackData->pMessage));
    return VK_FALSE;
}

// Helper function to load debug function
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
                                  VkDebugUtilsMessengerEXT debugMessenger,
                                  const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VulkanBackend::VulkanBackend() {
#ifdef _DEBUG
    m_enableValidationLayers = true;
#endif
    Logger::info("Creating Vulkan render backend");
}

VulkanBackend::~VulkanBackend() {
    cleanup();
}

bool VulkanBackend::initialize(GLFWwindow* window) {
    m_window = window;
    
    Logger::info("Initializing Vulkan backend...");
    
    try {
        if (!createInstance()) {
            Logger::error("Failed to create Vulkan instance");
            return false;
        }
        
        if (m_enableValidationLayers && !setupDebugMessenger()) {
            Logger::error("Failed to setup debug messenger");
            return false;
        }
        
        if (!createSurface()) {
            Logger::error("Failed to create surface");
            return false;
        }
        
        if (!pickPhysicalDevice()) {
            Logger::error("Failed to pick physical device");
            return false;
        }
        
        if (!createLogicalDevice()) {
            Logger::error("Failed to create logical device");
            return false;
        }
        
        if (!createSwapChain()) {
            Logger::error("Failed to create swap chain");
            return false;
        }
        
        if (!createImageViews()) {
            Logger::error("Failed to create image views");
            return false;
        }
        
        if (!createRenderPass()) {
            Logger::error("Failed to create render pass");
            return false;
        }
        
        if (!createDescriptorSetLayout()) {
            Logger::error("Failed to create descriptor set layout");
            return false;
        }
        
        if (!createGraphicsPipeline()) {
            Logger::error("Failed to create graphics pipeline");
            return false;
        }
        
        if (!createFramebuffers()) {
            Logger::error("Failed to create framebuffers");
            return false;
        }
        
        if (!createCommandPool()) {
            Logger::error("Failed to create command pool");
            return false;
        }
        
        if (!createCommandBuffers()) {
            Logger::error("Failed to create command buffers");
            return false;
        }
        
        if (!createSyncObjects()) {
            Logger::error("Failed to create sync objects");
            return false;
        }
        
    } catch (const std::exception& e) {
        Logger::error("Vulkan initialization failed: " + std::string(e.what()));
        return false;
    }
    
    m_initialized = true;
    Logger::info("Vulkan backend initialized successfully");
    return true;
}

void VulkanBackend::shutdown() {
    Logger::info("Shutting down Vulkan backend...");
    cleanup();
    m_initialized = false;
}

void VulkanBackend::beginFrame() {
    if (!m_initialized) return;
    
    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, 
                                          m_imageAvailableSemaphores[m_currentFrame], 
                                          VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Recreate swap chain
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        Logger::error("Failed to acquire swap chain image");
        return;
    }
    
    // Check if a previous frame is using this image
    if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
    
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);
    
    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        Logger::error("Failed to submit draw command buffer");
        return;
    }
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    
    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Recreate swap chain
    } else if (result != VK_SUCCESS) {
        Logger::error("Failed to present swap chain image");
    }
    
    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanBackend::endFrame() {
    // Frame presentation is handled in beginFrame for Vulkan
}

// Placeholder implementations for required virtual functions
void VulkanBackend::setCamera(const std::shared_ptr<Camera>& camera) {
    m_camera = camera;
}

void VulkanBackend::addLight(const std::shared_ptr<Light>& light) {
    m_lights.push_back(light);
}

void VulkanBackend::clearLights() {
    m_lights.clear();
}

void VulkanBackend::renderMesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& modelMatrix) {
    m_drawCalls++;
}

void VulkanBackend::renderSkybox() {
    m_drawCalls++;
}

void VulkanBackend::renderTerrain(const std::vector<std::shared_ptr<Mesh>>& terrainChunks) {
    m_drawCalls += static_cast<uint32_t>(terrainChunks.size());
}

void VulkanBackend::setClearColor(const glm::vec3& color) {
    m_clearColor = color;
}

void VulkanBackend::enableDepthTest(bool enable) {
    m_depthTestEnabled = enable;
}

void VulkanBackend::enableCulling(bool enable) {
    m_cullingEnabled = enable;
}

void VulkanBackend::setViewport(int x, int y, int width, int height) {
    // Viewport will be set in command buffer recording
}

void VulkanBackend::enablePBR(bool enable) {
    m_pbrEnabled = enable;
}

void VulkanBackend::setEnvironmentMap(const std::shared_ptr<Texture>& cubemap) {
    // Environment map implementation
}

void VulkanBackend::renderVoxels(const std::vector<glm::vec3>& voxelPositions, 
                               const std::vector<glm::vec4>& voxelColors) {
    m_drawCalls++;
}

void VulkanBackend::enableWireframe(bool enable) {
    m_wireframeEnabled = enable;
}

void VulkanBackend::enableDebugInfo(bool enable) {
    m_debugInfoEnabled = enable;
}

uint32_t VulkanBackend::getDrawCalls() const {
    return m_drawCalls;
}

float VulkanBackend::getFrameTime() const {
    return m_frameTime;
}

void VulkanBackend::reloadShaders() {
    Logger::info("Reloading Vulkan shaders...");
}

void VulkanBackend::garbageCollect() {
    Logger::info("Vulkan garbage collection");
}

void VulkanBackend::onShaderReloaded(const std::string& shaderName) {
    Logger::info("Vulkan shader reloaded: " + shaderName);
}

// Vulkan core implementation functions
bool VulkanBackend::createInstance() {
    if (m_enableValidationLayers && !checkValidationLayerSupport()) {
        Logger::error("Validation layers requested, but not available!");
        return false;
    }
    
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Interstate 3D";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Interstate 3D Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        Logger::error("Failed to create Vulkan instance");
        return false;
    }
    
    return true;
}

bool VulkanBackend::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    for (const char* layerName : m_validationLayers) {
        bool layerFound = false;
        
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        
        if (!layerFound) {
            return false;
        }
    }
    
    return true;
}

std::vector<const char*> VulkanBackend::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    if (m_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    return extensions;
}

void VulkanBackend::cleanup() {
    if (m_device == VK_NULL_HANDLE) return;
    
    vkDeviceWaitIdle(m_device);
    
    cleanupSwapChain();
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }
    
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDevice(m_device, nullptr);
    
    if (m_enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }
    
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

// Additional helper functions would be implemented here...
// For brevity, I'm providing placeholder implementations

bool VulkanBackend::setupDebugMessenger() {
    if (!m_enableValidationLayers) return true;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    
    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        return false;
    }
    
    return true;
}

bool VulkanBackend::createSurface() {
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
        Logger::error("Failed to create window surface");
        return false;
    }
    return true;
}

bool VulkanBackend::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        Logger::error("Failed to find GPUs with Vulkan support");
        return false;
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }
    
    if (m_physicalDevice == VK_NULL_HANDLE) {
        Logger::error("Failed to find a suitable GPU");
        return false;
    }
    
    return true;
}

bool VulkanBackend::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);
    
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices VulkanBackend::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
        
        if (presentSupport) {
            indices.presentFamily = i;
        }
        
        if (indices.isComplete()) {
            break;
        }
        
        i++;
    }
    
    return indices;
}

bool VulkanBackend::createLogicalDevice() { 
    Logger::info("Creating Vulkan logical device");
    return true; 
}

bool VulkanBackend::createSwapChain() { 
    Logger::info("Creating Vulkan swap chain");
    return true; 
}

bool VulkanBackend::createImageViews() { 
    Logger::info("Creating Vulkan image views");
    return true; 
}

bool VulkanBackend::createRenderPass() { 
    Logger::info("Creating Vulkan render pass");
    return true; 
}

bool VulkanBackend::createDescriptorSetLayout() { 
    Logger::info("Creating Vulkan descriptor set layout");
    return true; 
}

bool VulkanBackend::createGraphicsPipeline() { 
    Logger::info("Creating Vulkan graphics pipeline");
    return true; 
}

bool VulkanBackend::createFramebuffers() { 
    Logger::info("Creating Vulkan framebuffers");
    return true; 
}

bool VulkanBackend::createCommandPool() { 
    Logger::info("Creating Vulkan command pool");
    return true; 
}

bool VulkanBackend::createCommandBuffers() { 
    Logger::info("Creating Vulkan command buffers");
    return true; 
}

bool VulkanBackend::createSyncObjects() { 
    Logger::info("Creating Vulkan sync objects");
    return true; 
}

bool VulkanBackend::checkDeviceExtensionSupport(VkPhysicalDevice device) { 
    return true; 
}

SwapChainSupportDetails VulkanBackend::querySwapChainSupport(VkPhysicalDevice device) { 
    return {}; 
}

void VulkanBackend::cleanupSwapChain() { 
    Logger::debug("Cleaning up Vulkan swap chain");
}

VkShaderModule VulkanBackend::createShaderModule(const std::vector<char>& code) { 
    return VK_NULL_HANDLE; 
}

#include "RenderBackend.hpp"
#include "OpenGLBackend.hpp"
#include "VulkanBackend.hpp"
#include "core/Logger.hpp"

std::unique_ptr<RenderBackend> createRenderBackend(GraphicsAPI api) {
    switch (api) {
        case GraphicsAPI::OpenGL:
            Logger::info("Creating OpenGL rendering backend");
            return std::make_unique<OpenGLBackend>();
        
        case GraphicsAPI::Vulkan:
            Logger::info("Creating Vulkan rendering backend");
            return std::make_unique<VulkanBackend>();
        
        default:
            Logger::error("Unknown graphics API specified, defaulting to OpenGL");
            return std::make_unique<OpenGLBackend>();
    }
}

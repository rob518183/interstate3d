#include "windowing/Window.hpp"
#include "core/Logger.hpp"
#include <iostream>

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title) {
}

Window::~Window() {
    shutdown();
}

bool Window::initialize() {
    Logger::info("Initializing window: " + std::to_string(m_width) + "x" + std::to_string(m_height));
    
    // Initialize GLFW if not already done
    if (!glfwInit()) {
        Logger::error("Failed to initialize GLFW");
        return false;
    }
    
    // Configure GLFW for visible window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE); // Visible window
    
    // Create window (hidden for headless)
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        Logger::error("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }
    
    // Set OpenGL context
    glfwMakeContextCurrent(m_window);
    
    // Set window user pointer for callbacks
    glfwSetWindowUserPointer(m_window, this);
    
    // Register callbacks
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetWindowPosCallback(m_window, windowPosCallback);
    glfwSetWindowSizeCallback(m_window, windowSizeCallback);
    glfwSetWindowCloseCallback(m_window, windowCloseCallback);
    glfwSetWindowFocusCallback(m_window, windowFocusCallback);
    
    // Enable VSync
    glfwSwapInterval(1);
    
    Logger::info("Window initialized successfully");
    return true;
}

void Window::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::handleInput() {
    // Input handling is done through GLFW callbacks
    // This method can be used for polling-based input if needed
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void Window::getCursorPos(double& x, double& y) const {
    glfwGetCursorPos(m_window, &x, &y);
}

void Window::setFullscreen(bool fullscreen) {
    if (fullscreen) {
        // Get the primary monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        // Set window to fullscreen
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        // Set window to windowed mode
        glfwSetWindowMonitor(m_window, nullptr, 100, 100, m_width, m_height, GLFW_DONT_CARE);
    }
}

void Window::setFramebufferSizeCallback(std::function<void(int, int)> callback) {
    m_framebufferSizeCallback = callback;
}

void Window::setWindowPosCallback(std::function<void(int, int)> callback) {
    m_windowPosCallback = callback;
}

void Window::setWindowSizeCallback(std::function<void(int, int)> callback) {
    m_windowSizeCallback = callback;
}

void Window::setWindowCloseCallback(std::function<void()> callback) {
    m_windowCloseCallback = callback;
}

void Window::setWindowFocusCallback(std::function<void(bool)> callback) {
    m_windowFocusCallback = callback;
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->m_width = width;
        win->m_height = height;
        if (win->m_framebufferSizeCallback) {
            try {
                win->m_framebufferSizeCallback(width, height);
            } catch (const std::exception& e) {
                Logger::error("Exception in framebuffer size callback: " + std::string(e.what()));
            } catch (...) {
                Logger::error("Unknown exception in framebuffer size callback");
            }
        }
    }
    
    // Update viewport
    glViewport(0, 0, width, height);
}

void Window::windowPosCallback(GLFWwindow* window, int xpos, int ypos) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_windowPosCallback) {
        try {
            win->m_windowPosCallback(xpos, ypos);
        } catch (const std::exception& e) {
            Logger::error("Exception in window position callback: " + std::string(e.what()));
        } catch (...) {
            Logger::error("Unknown exception in window position callback");
        }
    }
}

void Window::windowSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->m_width = width;
        win->m_height = height;
        if (win->m_windowSizeCallback) {
            try {
                win->m_windowSizeCallback(width, height);
            } catch (const std::exception& e) {
                Logger::error("Exception in window size callback: " + std::string(e.what()));
            } catch (...) {
                Logger::error("Unknown exception in window size callback");
            }
        }
    }
}

void Window::windowCloseCallback(GLFWwindow* window) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_windowCloseCallback) {
        try {
            win->m_windowCloseCallback();
        } catch (const std::exception& e) {
            Logger::error("Exception in window close callback: " + std::string(e.what()));
        } catch (...) {
            Logger::error("Unknown exception in window close callback");
        }
    }
}

void Window::windowFocusCallback(GLFWwindow* window, int focused) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_windowFocusCallback) {
        try {
            win->m_windowFocusCallback(focused == GLFW_TRUE);
        } catch (const std::exception& e) {
            Logger::error("Exception in window focus callback: " + std::string(e.what()));
        } catch (...) {
            Logger::error("Unknown exception in window focus callback");
        }
    }
}

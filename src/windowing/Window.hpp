#pragma once

#include <string>
#include <functional>
#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();
    
    bool initialize();
    void shutdown();
    
    GLFWwindow* getHandle() const { return m_window; }
    
    bool shouldClose() const;
    void swapBuffers();
    
    void handleInput();
    
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    void getCursorPos(double& x, double& y) const;
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    void setFullscreen(bool fullscreen);
    
    void setFramebufferSizeCallback(std::function<void(int, int)> callback);
    void setWindowPosCallback(std::function<void(int, int)> callback);
    void setWindowSizeCallback(std::function<void(int, int)> callback);
    void setWindowCloseCallback(std::function<void()> callback);
    void setWindowFocusCallback(std::function<void(bool)> callback);
    
private:
    GLFWwindow* m_window = nullptr;
    int m_width;
    int m_height;
    std::string m_title;
    
    std::function<void(int, int)> m_framebufferSizeCallback;
    std::function<void(int, int)> m_windowPosCallback;
    std::function<void(int, int)> m_windowSizeCallback;
    std::function<void()> m_windowCloseCallback;
    std::function<void(bool)> m_windowFocusCallback;
    
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void windowPosCallback(GLFWwindow* window, int xpos, int ypos);
    static void windowSizeCallback(GLFWwindow* window, int width, int height);
    static void windowCloseCallback(GLFWwindow* window);
    static void windowFocusCallback(GLFWwindow* window, int focused);
};

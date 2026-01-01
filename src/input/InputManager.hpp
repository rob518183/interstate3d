#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

enum class KeyCode {
    // Keyboard keys
    Unknown = -1,
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Num0 = 48, Num1 = 49, Num2 = 50, Num3 = 51, Num4 = 52,
    Num5 = 53, Num6 = 54, Num7 = 55, Num8 = 56, Num9 = 57,
    Semicolon = 59,
    Equal = 61,
    A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71,
    H = 72, I = 73, J = 74, K = 75, L = 76, M = 77, N = 78,
    O = 79, P = 80, Q = 81, R = 82, S = 83, T = 84, U = 85,
    V = 86, W = 87, X = 88, Y = 89, Z = 90,
    LeftBracket = 91, Backslash = 92, RightBracket = 93,
    GraveAccent = 96, World1 = 161, World2 = 162,
    
    // Function keys
    Escape = 256, Enter, Tab, Backspace, Insert, Delete,
    Right = 262, Left, Down, Up,
    PageUp = 266, PageDown, Home, End,
    CapsLock = 280, ScrollLock, NumLock, PrintScreen,
    Pause = 284, F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10,
    F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,
    KP0 = 320, KP1, KP2, KP3, KP4, KP5, KP6, KP7, KP8, KP9,
    KPDecimal, KPDivide, KPMultiply, KPSubtract, KPAdd, KPEnter,
    KPEqual = 336,
    LeftShift = 340, LeftControl, LeftAlt, LeftSuper, RightShift,
    RightControl, RightAlt, RightSuper, Menu
};

enum class MouseButton {
    Left = 0, Right = 1, Middle = 2, Button4 = 3, Button5 = 4,
    Button6 = 5, Button7 = 6, Button8 = 7
};

enum class GamepadButton {
    A = 0, B, X, Y,
    LeftBumper, RightBumper,
    Back, Start, Guide,
    LeftThumb, RightThumb,
    DPadUp, DPadRight, DPadDown, DPadLeft
};

enum class GamepadAxis {
    LeftX = 0, LeftY, RightX, RightY,
    LeftTrigger, RightTrigger
};

struct InputEvent {
    enum Type { KeyPressed, KeyReleased, MouseButtonPressed, MouseButtonReleased, 
                MouseMoved, MouseScrolled, GamepadButtonPressed, GamepadButtonReleased,
                GamepadAxisMoved } type;
    
    union {
        KeyCode key;
        MouseButton mouseButton;
        GamepadButton gamepadButton;
        GamepadAxis gamepadAxis;
    };
    
    glm::vec2 mousePosition;
    glm::vec2 mouseDelta;
    glm::vec2 mouseScroll;
    float axisValue;
    bool pressed = false;
};

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    bool initialize(GLFWwindow* window);
    void shutdown();
    void update();
    
    // Keyboard input
    bool isKeyPressed(KeyCode key) const;
    bool isKeyJustPressed(KeyCode key) const;
    bool isKeyJustReleased(KeyCode key) const;
    
    // Mouse input
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonJustPressed(MouseButton button) const;
    bool isMouseButtonJustReleased(MouseButton button) const;
    glm::vec2 getMousePosition() const { return m_mousePosition; }
    glm::vec2 getMouseDelta() const { return m_mouseDelta; }
    glm::vec2 getMouseScroll() const { return m_mouseScroll; }
    
    // Gamepad input
    bool isGamepadConnected(int gamepadId = 0) const;
    bool isGamepadButtonPressed(GamepadButton button, int gamepadId = 0) const;
    bool isGamepadButtonJustPressed(GamepadButton button, int gamepadId = 0) const;
    bool isGamepadButtonJustReleased(GamepadButton button, int gamepadId = 0) const;
    float getGamepadAxis(GamepadAxis axis, int gamepadId = 0) const;
    
    // Event system
    void addEventCallback(std::function<void(const InputEvent&)> callback);
    void removeEventCallback(std::function<void(const InputEvent&)> callback);
    
    // Configuration
    void setMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }
    float getMouseSensitivity() const { return m_mouseSensitivity; }
    
    void setGamepadDeadzone(float deadzone) { m_gamepadDeadzone = deadzone; }
    float getGamepadDeadzone() const { return m_gamepadDeadzone; }
    
private:
    // GLFW callbacks
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // Internal state
    GLFWwindow* m_window = nullptr;
    
    // Keyboard state
    std::unordered_map<int, bool> m_currentKeyState;
    std::unordered_map<int, bool> m_previousKeyState;
    
    // Mouse state
    std::unordered_map<int, bool> m_currentMouseButtonState;
    std::unordered_map<int, bool> m_previousMouseButtonState;
    glm::vec2 m_mousePosition{0.0f};
    glm::vec2 m_previousMousePosition{0.0f};
    glm::vec2 m_mouseDelta{0.0f};
    glm::vec2 m_mouseScroll{0.0f};
    
    // Gamepad state
    std::unordered_map<int, std::unordered_map<int, bool>> m_currentGamepadButtonState;
    std::unordered_map<int, std::unordered_map<int, bool>> m_previousGamepadButtonState;
    std::unordered_map<int, std::unordered_map<int, float>> m_gamepadAxisState;
    
    // Configuration
    float m_mouseSensitivity = 1.0f;
    float m_gamepadDeadzone = 0.1f;
    
    // Event system
    std::vector<std::function<void(const InputEvent&)>> m_eventCallbacks;
    
    // Helper methods
    void pollGamepadEvents();
    void dispatchEvent(const InputEvent& event);
    static KeyCode glfwToKeyCode(int glfwKey);
    static MouseButton glfwToMouseButton(int glfwButton);
    static GamepadButton glfwToGamepadButton(int glfwButton);
    static GamepadAxis glfwToGamepadAxis(int glfwAxis);
};

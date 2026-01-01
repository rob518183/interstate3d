#include "input/InputManager.hpp"
#include "core/Logger.hpp"
#include <algorithm>

InputManager::InputManager() {
    Logger::info("InputManager created");
}

InputManager::~InputManager() {
    shutdown();
}

bool InputManager::initialize(GLFWwindow* window) {
    if (!window) {
        Logger::error("InputManager: Invalid window provided");
        return false;
    }
    
    m_window = window;
    
    // Set GLFW callbacks
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    
    // Set window user pointer to this instance
    glfwSetWindowUserPointer(m_window, this);
    
    Logger::info("InputManager initialized successfully");
    return true;
}

void InputManager::shutdown() {
    if (m_window) {
        glfwSetKeyCallback(m_window, nullptr);
        glfwSetMouseButtonCallback(m_window, nullptr);
        glfwSetCursorPosCallback(m_window, nullptr);
        glfwSetScrollCallback(m_window, nullptr);
        m_window = nullptr;
    }
    
    m_currentKeyState.clear();
    m_previousKeyState.clear();
    m_currentMouseButtonState.clear();
    m_previousMouseButtonState.clear();
    m_currentGamepadButtonState.clear();
    m_previousGamepadButtonState.clear();
    m_gamepadAxisState.clear();
    m_eventCallbacks.clear();
    
    Logger::info("InputManager shutdown complete");
}

void InputManager::update() {
    // Store previous states
    m_previousKeyState = m_currentKeyState;
    m_previousMouseButtonState = m_currentMouseButtonState;
    m_previousGamepadButtonState = m_currentGamepadButtonState;
    
    // Reset mouse delta and scroll
    m_mouseDelta = glm::vec2(0.0f);
    m_mouseScroll = glm::vec2(0.0f);
    
    // Poll gamepad events
    pollGamepadEvents();
    
    // Update mouse delta
    m_mouseDelta = m_mousePosition - m_previousMousePosition;
    m_previousMousePosition = m_mousePosition;
}

// Keyboard input
bool InputManager::isKeyPressed(KeyCode key) const {
    int glfwKey = static_cast<int>(key);
    auto it = m_currentKeyState.find(glfwKey);
    return it != m_currentKeyState.end() && it->second;
}

bool InputManager::isKeyJustPressed(KeyCode key) const {
    int glfwKey = static_cast<int>(key);
    auto currentIt = m_currentKeyState.find(glfwKey);
    auto previousIt = m_previousKeyState.find(glfwKey);
    
    bool current = currentIt != m_currentKeyState.end() && currentIt->second;
    bool previous = previousIt != m_previousKeyState.end() && previousIt->second;
    
    return current && !previous;
}

bool InputManager::isKeyJustReleased(KeyCode key) const {
    int glfwKey = static_cast<int>(key);
    auto currentIt = m_currentKeyState.find(glfwKey);
    auto previousIt = m_previousKeyState.find(glfwKey);
    
    bool current = currentIt != m_currentKeyState.end() && currentIt->second;
    bool previous = previousIt != m_previousKeyState.end() && previousIt->second;
    
    return !current && previous;
}

// Mouse input
bool InputManager::isMouseButtonPressed(MouseButton button) const {
    int glfwButton = static_cast<int>(button);
    auto it = m_currentMouseButtonState.find(glfwButton);
    return it != m_currentMouseButtonState.end() && it->second;
}

bool InputManager::isMouseButtonJustPressed(MouseButton button) const {
    int glfwButton = static_cast<int>(button);
    auto currentIt = m_currentMouseButtonState.find(glfwButton);
    auto previousIt = m_previousMouseButtonState.find(glfwButton);
    
    bool current = currentIt != m_currentMouseButtonState.end() && currentIt->second;
    bool previous = previousIt != m_previousMouseButtonState.end() && previousIt->second;
    
    return current && !previous;
}

bool InputManager::isMouseButtonJustReleased(MouseButton button) const {
    int glfwButton = static_cast<int>(button);
    auto currentIt = m_currentMouseButtonState.find(glfwButton);
    auto previousIt = m_previousMouseButtonState.find(glfwButton);
    
    bool current = currentIt != m_currentMouseButtonState.end() && currentIt->second;
    bool previous = previousIt != m_previousMouseButtonState.end() && previousIt->second;
    
    return !current && previous;
}

// Gamepad input
bool InputManager::isGamepadConnected(int gamepadId) const {
    return glfwJoystickIsGamepad(GLFW_JOYSTICK_1 + gamepadId) == GLFW_TRUE;
}

bool InputManager::isGamepadButtonPressed(GamepadButton button, int gamepadId) const {
    auto gamepadIt = m_currentGamepadButtonState.find(gamepadId);
    if (gamepadIt == m_currentGamepadButtonState.end()) return false;
    
    int glfwButton = static_cast<int>(button);
    auto buttonIt = gamepadIt->second.find(glfwButton);
    return buttonIt != gamepadIt->second.end() && buttonIt->second;
}

bool InputManager::isGamepadButtonJustPressed(GamepadButton button, int gamepadId) const {
    int glfwButton = static_cast<int>(button);
    
    auto currentIt = m_currentGamepadButtonState.find(gamepadId);
    auto previousIt = m_previousGamepadButtonState.find(gamepadId);
    
    if (currentIt == m_currentGamepadButtonState.end() || previousIt == m_previousGamepadButtonState.end()) return false;
    
    bool current = currentIt->second.find(glfwButton) != currentIt->second.end() && currentIt->second.at(glfwButton);
    bool previous = previousIt->second.find(glfwButton) != previousIt->second.end() && previousIt->second.at(glfwButton);
    
    return current && !previous;
}

bool InputManager::isGamepadButtonJustReleased(GamepadButton button, int gamepadId) const {
    int glfwButton = static_cast<int>(button);
    auto currentIt = m_currentGamepadButtonState.find(gamepadId);
    auto previousIt = m_previousGamepadButtonState.find(gamepadId);
    
    if (currentIt == m_currentGamepadButtonState.end() || previousIt == m_previousGamepadButtonState.end()) return false;
    
    bool current = currentIt->second.find(glfwButton) != currentIt->second.end() && currentIt->second.at(glfwButton);
    bool previous = previousIt->second.find(glfwButton) != previousIt->second.end() && previousIt->second.at(glfwButton);
    
    return !current && previous;
}

float InputManager::getGamepadAxis(GamepadAxis axis, int gamepadId) const {
    auto gamepadIt = m_gamepadAxisState.find(gamepadId);
    if (gamepadIt == m_gamepadAxisState.end()) return 0.0f;
    
    int glfwAxis = static_cast<int>(axis);
    auto axisIt = gamepadIt->second.find(glfwAxis);
    if (axisIt == gamepadIt->second.end()) return 0.0f;
    
    // Apply deadzone
    float value = axisIt->second;
    if (std::abs(value) < m_gamepadDeadzone) return 0.0f;
    return value;
}

// Event system
void InputManager::addEventCallback(std::function<void(const InputEvent&)> callback) {
    m_eventCallbacks.push_back(callback);
}

void InputManager::removeEventCallback(std::function<void(const InputEvent&)> callback) {
    // Note: This is a simplified implementation. In practice, you'd want to store
    // callback IDs or use a more sophisticated removal mechanism.
    m_eventCallbacks.erase(
        std::remove_if(m_eventCallbacks.begin(), m_eventCallbacks.end(),
            [&callback](const std::function<void(const InputEvent&)>& stored) {
                return stored.target<void(const InputEvent&)>() == callback.target<void(const InputEvent&)>();
            }),
        m_eventCallbacks.end()
    );
}

// GLFW callbacks
void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!input) return;
    
    KeyCode keyCode = glfwToKeyCode(key);
    if (keyCode == KeyCode::Unknown) return;
    
    int keyInt = static_cast<int>(keyCode);
    
    if (action == GLFW_PRESS) {
        input->m_currentKeyState[keyInt] = true;
        
        InputEvent event;
        event.type = InputEvent::KeyPressed;
        event.key = keyCode;
        event.pressed = true;
        input->dispatchEvent(event);
    } else if (action == GLFW_RELEASE) {
        input->m_currentKeyState[keyInt] = false;
        
        InputEvent event;
        event.type = InputEvent::KeyReleased;
        event.key = keyCode;
        event.pressed = false;
        input->dispatchEvent(event);
    }
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!input) return;
    
    MouseButton mouseButton = glfwToMouseButton(button);
    int buttonInt = static_cast<int>(mouseButton);
    
    if (action == GLFW_PRESS) {
        input->m_currentMouseButtonState[buttonInt] = true;
        
        InputEvent event;
        event.type = InputEvent::MouseButtonPressed;
        event.mouseButton = mouseButton;
        event.pressed = true;
        event.mousePosition = input->m_mousePosition;
        input->dispatchEvent(event);
    } else if (action == GLFW_RELEASE) {
        input->m_currentMouseButtonState[buttonInt] = false;
        
        InputEvent event;
        event.type = InputEvent::MouseButtonReleased;
        event.mouseButton = mouseButton;
        event.pressed = false;
        event.mousePosition = input->m_mousePosition;
        input->dispatchEvent(event);
    }
}

void InputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!input) return;
    
    glm::vec2 newPosition(static_cast<float>(xpos), static_cast<float>(ypos));
    
    InputEvent event;
    event.type = InputEvent::MouseMoved;
    event.mousePosition = newPosition;
    event.mouseDelta = newPosition - input->m_mousePosition;
    input->dispatchEvent(event);
    
    input->m_mousePosition = newPosition;
}

void InputManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!input) return;
    
    glm::vec2 scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
    
    InputEvent event;
    event.type = InputEvent::MouseScrolled;
    event.mouseScroll = scroll;
    input->dispatchEvent(event);
    
    input->m_mouseScroll += scroll;
}

// Helper methods
void InputManager::pollGamepadEvents() {
    for (int i = 0; i <= GLFW_JOYSTICK_LAST; ++i) {
        if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1 + i) == GLFW_TRUE) {
            GLFWgamepadstate state;
            if (glfwGetGamepadState(GLFW_JOYSTICK_1 + i, &state) == GLFW_TRUE) {
                // Update button states
                for (int button = 0; button <= GLFW_GAMEPAD_BUTTON_LAST; ++button) {
                    bool pressed = state.buttons[button] == GLFW_PRESS;
                    m_currentGamepadButtonState[i][button] = pressed;
                    
                    // Dispatch events
                    GamepadButton gamepadButton = glfwToGamepadButton(button);
                    if (gamepadButton != GamepadButton::A) { // Skip invalid mappings
                        InputEvent event;
                        event.type = pressed ? InputEvent::GamepadButtonPressed : InputEvent::GamepadButtonReleased;
                        event.gamepadButton = gamepadButton;
                        event.pressed = pressed;
                        dispatchEvent(event);
                    }
                }
                
                // Update axis states
                for (int axis = 0; axis <= GLFW_GAMEPAD_AXIS_LAST; ++axis) {
                    float value = state.axes[axis];
                    m_gamepadAxisState[i][axis] = value;
                    
                    // Dispatch axis events for significant changes
                    if (std::abs(value) > m_gamepadDeadzone) {
                        GamepadAxis gamepadAxis = glfwToGamepadAxis(axis);
                        if (gamepadAxis != GamepadAxis::LeftX) { // Skip invalid mappings
                            InputEvent event;
                            event.type = InputEvent::GamepadAxisMoved;
                            event.gamepadAxis = gamepadAxis;
                            event.axisValue = value;
                            dispatchEvent(event);
                        }
                    }
                }
            }
        }
    }
}

void InputManager::dispatchEvent(const InputEvent& event) {
    for (const auto& callback : m_eventCallbacks) {
        callback(event);
    }
}

KeyCode InputManager::glfwToKeyCode(int glfwKey) {
    return static_cast<KeyCode>(glfwKey);
}

MouseButton InputManager::glfwToMouseButton(int glfwButton) {
    return static_cast<MouseButton>(glfwButton);
}

GamepadButton InputManager::glfwToGamepadButton(int glfwButton) {
    switch (glfwButton) {
        case GLFW_GAMEPAD_BUTTON_A: return GamepadButton::A;
        case GLFW_GAMEPAD_BUTTON_B: return GamepadButton::B;
        case GLFW_GAMEPAD_BUTTON_X: return GamepadButton::X;
        case GLFW_GAMEPAD_BUTTON_Y: return GamepadButton::Y;
        case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return GamepadButton::LeftBumper;
        case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return GamepadButton::RightBumper;
        case GLFW_GAMEPAD_BUTTON_BACK: return GamepadButton::Back;
        case GLFW_GAMEPAD_BUTTON_START: return GamepadButton::Start;
        case GLFW_GAMEPAD_BUTTON_GUIDE: return GamepadButton::Guide;
        case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return GamepadButton::LeftThumb;
        case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return GamepadButton::RightThumb;
        case GLFW_GAMEPAD_BUTTON_DPAD_UP: return GamepadButton::DPadUp;
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return GamepadButton::DPadRight;
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return GamepadButton::DPadDown;
        case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return GamepadButton::DPadLeft;
        default: return GamepadButton::A; // Fallback
    }
}

GamepadAxis InputManager::glfwToGamepadAxis(int glfwAxis) {
    switch (glfwAxis) {
        case GLFW_GAMEPAD_AXIS_LEFT_X: return GamepadAxis::LeftX;
        case GLFW_GAMEPAD_AXIS_LEFT_Y: return GamepadAxis::LeftY;
        case GLFW_GAMEPAD_AXIS_RIGHT_X: return GamepadAxis::RightX;
        case GLFW_GAMEPAD_AXIS_RIGHT_Y: return GamepadAxis::RightY;
        case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER: return GamepadAxis::LeftTrigger;
        case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER: return GamepadAxis::RightTrigger;
        default: return GamepadAxis::LeftX; // Fallback
    }
}

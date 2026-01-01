#include "rendering/Shader.hpp"
#include "core/Logger.hpp"

// Placeholder implementation for OpenGL - would normally include:
// #include <glad/glad.h>
#include <sstream>

// Declare stub OpenGL functions
extern unsigned int glCreateShader(unsigned int type);
extern void glShaderSource(unsigned int shader, int count, const char** string, const int* length);
extern void glCompileShader(unsigned int shader);
extern void glGetShaderiv(unsigned int shader, unsigned int pname, int* params);
extern void glGetShaderInfoLog(unsigned int shader, int bufSize, int* length, char* infoLog);
extern void glDeleteShader(unsigned int shader);
extern unsigned int glCreateProgram();
extern void glAttachShader(unsigned int program, unsigned int shader);
extern void glLinkProgram(unsigned int program);
extern void glGetProgramiv(unsigned int program, unsigned int pname, int* params);
extern void glGetProgramInfoLog(unsigned int program, int bufSize, int* length, char* infoLog);
extern void glDeleteProgram(unsigned int program);
extern void glUseProgram(unsigned int program);
extern int glGetUniformLocation(unsigned int program, const char* name);
extern void glUniform1i(int location, int value);
extern void glUniform1f(int location, float value);
extern void glUniform2fv(int location, int count, const float* value);
extern void glUniform3fv(int location, int count, const float* value);
extern void glUniform4fv(int location, int count, const float* value);
extern void glUniformMatrix2fv(int location, int count, int transpose, const float* value);
extern void glUniformMatrix3fv(int location, int count, int transpose, const float* value);
extern void glUniformMatrix4fv(int location, int count, int transpose, const float* value);

// Define OpenGL constants for placeholder implementation
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_GEOMETRY_SHADER
#define GL_GEOMETRY_SHADER 0x8DD9
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_FALSE
#define GL_FALSE 0
#endif

Shader::Shader() {
    Logger::debug("Created Shader asset");
}

Shader::~Shader() {
    if (m_loaded && m_programID != 0) {
        glDeleteProgram(m_programID);
    }
}

bool Shader::load(const std::string& path) {
    // For shaders, we expect path to be a base name without extension
    // We'll try to load vertex and fragment shaders with .vert and .frag extensions
    std::string vertexPath = path + ".vert";
    std::string fragmentPath = path + ".frag";
    
    bool success = loadFromFiles(vertexPath, fragmentPath);
    if (success) {
        m_path = path;
        m_loaded = true;
    }
    return success;
}

void Shader::reload() {
    if (!m_path.empty()) {
        Logger::info("Reloading shader: " + m_path);
        load(m_path);
    }
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        Logger::error("Failed to read shader files: " + vertexPath + ", " + fragmentPath);
        return false;
    }
    
    return loadFromSource(vertexSource, fragmentSource);
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);
    std::string geometrySource = readFile(geometryPath);
    
    if (vertexSource.empty() || fragmentSource.empty() || geometrySource.empty()) {
        Logger::error("Failed to read shader files: " + vertexPath + ", " + fragmentPath + ", " + geometryPath);
        return false;
    }
    
    // Compile shaders
    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    unsigned int geometryShader = compileShader(geometrySource, GL_GEOMETRY_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0 || geometryShader == 0) {
        Logger::error("Failed to compile shaders");
        return false;
    }
    
    // Create program
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glAttachShader(m_programID, geometryShader);
    glLinkProgram(m_programID);
    
    // Check linking errors
    int success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, 512, nullptr, infoLog);
        Logger::error("Shader program linking failed: " + std::string(infoLog));
        return false;
    }
    
    // Clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(geometryShader);
    
    m_loaded = true;
    m_uniformLocations.clear();
    
    Logger::info("Shader program loaded successfully with geometry shader");
    return true;
}

bool Shader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    // Compile shaders
    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        Logger::error("Failed to compile shaders");
        return false;
    }
    
    // Create program
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);
    
    // Check linking errors
    int success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, 512, nullptr, infoLog);
        Logger::error("Shader program linking failed: " + std::string(infoLog));
        return false;
    }
    
    // Clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    m_loaded = true;
    m_uniformLocations.clear();
    
    Logger::info("Shader program loaded successfully");
    return true;
}

void Shader::use() const {
    if (m_loaded) {
        glUseProgram(m_programID);
    }
}

void Shader::unuse() const {
    glUseProgram(0);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setMat2(const std::string& name, const glm::mat2& value) const {
    glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) const {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

unsigned int Shader::compileShader(const std::string& source, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    // Check compilation errors
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        
        std::string shaderType = (type == GL_VERTEX_SHADER) ? "VERTEX" : 
                                 (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "GEOMETRY";
        Logger::error("Shader compilation failed (" + shaderType + "): " + std::string(infoLog));
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

std::string Shader::readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::error("Cannot open file: " + filepath);
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int Shader::getUniformLocation(const std::string& name) const {
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }
    
    int location = glGetUniformLocation(m_programID, name.c_str());
    m_uniformLocations[name] = location;
    return location;
}

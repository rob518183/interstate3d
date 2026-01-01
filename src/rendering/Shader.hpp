#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "core/AssetManager.hpp"

class Shader : public Asset {
public:
    Shader();
    ~Shader();
    
    bool load(const std::string& path) override;
    void reload() override;
    AssetType getType() const override { return AssetType::Shader; }
    
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath);
    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    
    void use() const;
    void unuse() const;
    
    // Uniform setters
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat2(const std::string& name, const glm::mat2& value) const;
    void setMat3(const std::string& name, const glm::mat3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;
    
    unsigned int getID() const { return m_programID; }
    bool isLoaded() const { return m_loaded; }
    
private:
    unsigned int m_programID = 0;
    bool m_loaded = false;
    mutable std::unordered_map<std::string, int> m_uniformLocations;
    
    unsigned int compileShader(const std::string& source, unsigned int type);
    std::string readFile(const std::string& filepath);
    int getUniformLocation(const std::string& name) const;
};

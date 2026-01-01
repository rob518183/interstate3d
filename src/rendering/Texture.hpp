#pragma once

#include <string>
#include <memory>
#include "core/AssetManager.hpp"

class Texture : public Asset {
public:
    enum class Format {
        RGB,
        RGBA,
        BGR,
        BGRA
    };
    
    enum class Filter {
        NEAREST,
        LINEAR,
        LINEAR_MIPMAP
    };
    
    enum class Wrap {
        REPEAT,
        CLAMP_TO_EDGE,
        MIRRORED_REPEAT
    };
    
    Texture();
    ~Texture();
    
    bool load(const std::string& path) override;
    void reload() override;
    AssetType getType() const override { return AssetType::Texture; }
    
    bool loadFromFile(const std::string& filepath);
    bool loadFromMemory(const unsigned char* data, int width, int height, Format format);
    
    void bind(unsigned int unit = 0) const;
    void unbind() const;
    
    void setFilter(Filter minFilter, Filter magFilter);
    void setWrap(Wrap wrapS, Wrap wrapT);
    
    unsigned int getId() const { return m_textureId; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    bool isLoaded() const { return m_loaded; }
    
private:
    unsigned int m_textureId = 0;
    int m_width = 0;
    int m_height = 0;
    Format m_format = Format::RGBA;
    bool m_loaded = false;
    
    void cleanup();
};

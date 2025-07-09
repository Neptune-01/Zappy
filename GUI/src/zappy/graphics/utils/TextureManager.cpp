/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Texture manager
*/

#include <iostream>
#include "zappy/graphics/utils/TextureManager.hpp"

zappy::graphics::utils::TextureManager::TextureManager()
{
    //std::cout << "🖼️ TextureManager created" << std::endl;
}

zappy::graphics::utils::TextureManager::~TextureManager()
{
    deleteAllTextures();
}

void zappy::graphics::utils::TextureManager::loadDefaultTextures()
{
    //std::cout << "🔍 Loading default textures..." << std::endl;
    
    // Load grass texture
    if (loadTexture(TextureType::GRASS, DEFAULT_GRASS_PATH)) {
        //std::cout << "✅ Grass texture loaded!" << std::endl;
    } else {
        //std::cout << "❌ Could not find " << DEFAULT_GRASS_PATH << std::endl;
    }
    
    // Load dirt texture
    if (loadTexture(TextureType::DIRT, DEFAULT_DIRT_PATH)) {
        //std::cout << "✅ Dirt texture loaded!" << std::endl;
    } else {
        //std::cout << "❌ Could not find " << DEFAULT_DIRT_PATH << std::endl;
    }
}

bool zappy::graphics::utils::TextureManager::loadTexture(TextureType type, const std::string& filepath)
{
    TextureConfig default_config;
    return loadTexture(type, filepath, default_config);
}

bool zappy::graphics::utils::TextureManager::loadTexture(TextureType type, const std::string& filepath, 
    const TextureConfig& config)
{
    sf::Image image;
    if (!image.loadFromFile(filepath)) {
        //std::cout << "❌ Failed to load texture: " << filepath << std::endl;
        return false;
    }
    
    // Delete existing texture if it exists
    deleteTexture(type);
    
    // Create OpenGL texture
    GLuint texture_id = createTextureFromImage(image, config);
    if (texture_id == 0) {
        //std::cout << "❌ Failed to create OpenGL texture for: " << filepath << std::endl;
        return false;
    }
    
    // Store texture
    _textures[type] = texture_id;
    _textures_loaded++;
    _memory_usage += image.getSize().x * image.getSize().y * 4; // Approximate RGBA
    
    //std::cout << "✅ Loaded " << textureTypeToString(type) << " texture (" 
              //<< image.getSize().x << "x" << image.getSize().y << ")" << std::endl;
    
    return true;
}

bool zappy::graphics::utils::TextureManager::loadCustomTexture(const std::string& name, const std::string& filepath)
{
    // Use default configuration
    TextureConfig default_config;
    return loadCustomTexture(name, filepath, default_config);
}

bool zappy::graphics::utils::TextureManager::loadCustomTexture(const std::string& name, const std::string& filepath,
    const TextureConfig& config)
{
    sf::Image image;
    if (!image.loadFromFile(filepath)) {
        //std::cout << "❌ Failed to load custom texture: " << filepath << std::endl;
        return false;
    }
    
    // Delete existing texture if it exists
    deleteCustomTexture(name);
    
    // Create OpenGL texture
    GLuint texture_id = createTextureFromImage(image, config);
    if (texture_id == 0) {
        //std::cout << "❌ Failed to create OpenGL texture for: " << filepath << std::endl;
        return false;
    }
    
    // Store texture
    _custom_textures[name] = texture_id;
    _textures_loaded++;
    _memory_usage += image.getSize().x * image.getSize().y * 4;
    
    //std::cout << "✅ Loaded custom texture '" << name << "' (" 
    //          << image.getSize().x << "x" << image.getSize().y << ")" << std::endl;
    
    return true;
}

GLuint zappy::graphics::utils::TextureManager::createTextureFromImage(const sf::Image& image, const TextureConfig& config)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    
    if (texture_id == 0) {
        return 0;
    }
    
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap_t);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
        image.getSize().x, image.getSize().y,
        0, GL_RGBA, GL_UNSIGNED_BYTE, 
        image.getPixelsPtr());
    
    if (config.generate_mipmaps) {
    }
    
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind
    
    return texture_id;
}

GLuint zappy::graphics::utils::TextureManager::getTexture(TextureType type) const
{
    auto it = _textures.find(type);
    return (it != _textures.end()) ? it->second : 0;
}

GLuint zappy::graphics::utils::TextureManager::getCustomTexture(const std::string& name) const
{
    auto it = _custom_textures.find(name);
    return (it != _custom_textures.end()) ? it->second : 0;
}

bool zappy::graphics::utils::TextureManager::hasTexture(TextureType type) const
{
    return _textures.find(type) != _textures.end();
}

bool zappy::graphics::utils::TextureManager::hasCustomTexture(const std::string& name) const
{
    return _custom_textures.find(name) != _custom_textures.end();
}

void zappy::graphics::utils::TextureManager::bindTexture(TextureType type)
{
    GLuint texture_id = getTexture(type);
    if (texture_id != 0) {
        glBindTexture(GL_TEXTURE_2D, texture_id);
        _currently_bound = texture_id;
    }
}

void zappy::graphics::utils::TextureManager::bindCustomTexture(const std::string& name)
{
    GLuint texture_id = getCustomTexture(name);
    if (texture_id != 0) {
        glBindTexture(GL_TEXTURE_2D, texture_id);
        _currently_bound = texture_id;
    }
}

void zappy::graphics::utils::TextureManager::unbindTexture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    _currently_bound = 0;
}

void zappy::graphics::utils::TextureManager::enableTexturing()
{
    glEnable(GL_TEXTURE_2D);
    _texturing_enabled = true;
}

void zappy::graphics::utils::TextureManager::disableTexturing()
{
    glDisable(GL_TEXTURE_2D);
    _texturing_enabled = false;
}

void zappy::graphics::utils::TextureManager::deleteTexture(TextureType type)
{
    auto it = _textures.find(type);
    if (it != _textures.end()) {
        glDeleteTextures(1, &it->second);
        _textures.erase(it);
    }
}

void zappy::graphics::utils::TextureManager::deleteCustomTexture(const std::string& name)
{
    auto it = _custom_textures.find(name);
    if (it != _custom_textures.end()) {
        glDeleteTextures(1, &it->second);
        _custom_textures.erase(it);
    }
}

void zappy::graphics::utils::TextureManager::deleteAllTextures()
{
    // Delete standard textures
    for (auto& pair : _textures) {
        glDeleteTextures(1, &pair.second);
    }
    _textures.clear();
    
    // Delete custom textures
    for (auto& pair : _custom_textures) {
        glDeleteTextures(1, &pair.second);
    }
    _custom_textures.clear();
    
    _textures_loaded = 0;
    _memory_usage = 0;
    _currently_bound = 0;
}

void zappy::graphics::utils::TextureManager::printTextureInfo() const
{
    //std::cout << "📊 Texture Manager Statistics:" << std::endl;
    //std::cout << "  Textures loaded: " << _textures_loaded << std::endl;
    //std::cout << "  Memory usage: ~" << (_memory_usage / 1024) << " KB" << std::endl;
    //std::cout << "  Standard textures: " << _textures.size() << std::endl;
    //std::cout << "  Custom textures: " << _custom_textures.size() << std::endl;
}

std::string zappy::graphics::utils::TextureManager::textureTypeToString(TextureType type) const
{
    switch (type) {
        case TextureType::GRASS: return "grass";
        case TextureType::DIRT: return "dirt";
        case TextureType::STONE: return "stone";
        case TextureType::WATER: return "water";
        case TextureType::CUSTOM: return "custom";
        default: return "unknown";
    }
}
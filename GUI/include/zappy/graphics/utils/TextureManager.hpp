/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Texture loading and management system
*/

#ifndef TEXTUREMANAGER_HPP
#define TEXTUREMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <string>
#include <unordered_map>


namespace zappy::graphics::utils{

class TextureManager {
public:
    enum class TextureType {
        GRASS,
        DIRT,
        STONE,
        WATER,
        CUSTOM
    };
    
    struct TextureConfig {
        GLenum min_filter = GL_LINEAR;
        GLenum mag_filter = GL_LINEAR;
        GLenum wrap_s = GL_REPEAT;
        GLenum wrap_t = GL_REPEAT;
        bool generate_mipmaps = false;
    };
    
public:
    TextureManager();
    ~TextureManager();
    
    bool loadTexture(TextureType type, const std::string& filepath);
    bool loadTexture(TextureType type, const std::string& filepath, 
                    const TextureConfig& config);
    bool loadCustomTexture(const std::string& name, const std::string& filepath);
    bool loadCustomTexture(const std::string& name, const std::string& filepath,
                          const TextureConfig& config);
    void loadDefaultTextures();
    
    GLuint getTexture(TextureType type) const;
    GLuint getCustomTexture(const std::string& name) const;
    bool hasTexture(TextureType type) const;
    bool hasCustomTexture(const std::string& name) const;
    
    void bindTexture(TextureType type);
    void bindCustomTexture(const std::string& name);
    void unbindTexture();
    
    void enableTexturing();
    void disableTexturing();
    bool isTexturingEnabled() const { return _texturing_enabled; }

    void deleteTexture(TextureType type);
    void deleteCustomTexture(const std::string& name);
    void deleteAllTextures();
    void printTextureInfo() const;
    
    static constexpr const char* DEFAULT_GRASS_PATH = "assets/textures/grass.png";
    static constexpr const char* DEFAULT_DIRT_PATH = "assets/textures/grass.png";
    
private:
    GLuint createTextureFromImage(const sf::Image& image, const TextureConfig& config);
    void applyTextureConfig(const TextureConfig& config);
    std::string textureTypeToString(TextureType type) const;
    
    std::unordered_map<TextureType, GLuint> _textures;
    std::unordered_map<std::string, GLuint> _custom_textures;
    
    bool _texturing_enabled = false;
    GLuint _currently_bound = 0;
    
    size_t _textures_loaded = 0;
    size_t _memory_usage = 0;
};
}
#endif /* !TEXTUREMANAGER_HPP */
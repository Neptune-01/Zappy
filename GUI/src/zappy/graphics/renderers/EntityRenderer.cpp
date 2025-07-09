/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Entity rendering
*/

#include "zappy/graphics/renderers/EntityRenderer.hpp"
#include "zappy/game/entities/CreeperModel.hpp"
#include "zappy/graphics/utils/OpenGLUtils.hpp"
#include <iostream>

zappy::graphics::renderers::EntityRenderer::EntityRenderer()
{
    //std::cout << "🎮 EntityRenderer created with Creeper models" << std::endl;
    _creeper_model = nullptr;
    _initialized = false;
    _players_rendered = 0;
}

zappy::graphics::renderers::EntityRenderer::~EntityRenderer()
{
    if (_creeper_model) {
        delete _creeper_model;
        _creeper_model = nullptr;
    }
    //std::cout << "🎮 EntityRenderer destroyed" << std::endl;
}

void zappy::graphics::renderers::EntityRenderer::initialize(zappy::graphics::utils::TextureManager* texture_manager)
{
    if (_initialized) return;
    
    //std::cout << "🎮 Initializing EntityRenderer with Creeper models..." << std::endl;
    
    // Initialize Creeper model avec TextureManager
    _creeper_model = new zappy::game::entities::CreeperModel();
    if (!_creeper_model->initialize("assets/models/Creeper.obj", texture_manager)) {
        #ifndef NDEBUG
        std::cout << "⚠️ Creeper model initialization failed, using fallback cubes" << std::endl;
        #endif
    } else {
        //std::cout << "✅ Creeper model initialized with texture support!" << std::endl;
    }
    
    _initialized = true;
    
    //std::cout << "🎮 EntityRenderer initialized!" << std::endl;
    //std::cout << "🐸 Creeper models: " << (_creeper_model && _creeper_model->isEnabled() ? "ENABLED" : "FALLBACK CUBES") << std::endl;
}

void zappy::graphics::renderers::EntityRenderer::renderPlayer(const zappy::game::entities::ZappyPlayer& player)
{
    if (!_initialized) return;
    
    if (_creeper_model && _creeper_model->isEnabled()) {
        _creeper_model->renderPlayer(player, _last_delta_time);
    } else {
        renderPlayerTraditional(player);
    }
    
    _players_rendered++;
}

void zappy::graphics::renderers::EntityRenderer::renderDirectionIndicator(const zappy::game::entities::ZappyPlayer& player)
{
    if (_creeper_model && _creeper_model->isEnabled()) {
        return;
    }
    
    sf::Vector2f dir = player.getDirectionVector();
    MathUtils::Vector3f player_pos = getPlayerPosition(player);
    
    float indicator_x = player_pos.x + dir.x * (_config.player_size/2 + _config.indicator_distance);
    float indicator_z = player_pos.z + dir.y * (_config.player_size/2 + _config.indicator_distance);
    float indicator_y = player_pos.y + _config.player_height * 0.5f;
    
    renderIndicatorGeometry(indicator_x, indicator_y, indicator_z,
        _config.indicator_size, _config.indicator_height,
        _config.indicator_color);
}

void zappy::graphics::renderers::EntityRenderer::renderPlayers(const std::map<int, zappy::game::entities::ZappyPlayer>& players)
{
    for (const auto& pair : players) {
        renderPlayer(pair.second);
        renderDirectionIndicator(pair.second);
    }
}

void zappy::graphics::renderers::EntityRenderer::renderPlayersRange(const std::map<int, zappy::game::entities::ZappyPlayer>& players, 
                                       int start_id, int end_id)
{
    for (const auto& pair : players) {
        if (pair.first >= start_id && pair.first <= end_id) {
            renderPlayer(pair.second);
            renderDirectionIndicator(pair.second);
        }
    }
}

void zappy::graphics::renderers::EntityRenderer::renderCube(float x, float y, float z, float size, float height, 
                               const sf::Color& color)
{
    renderPlayerGeometry(x, y, z, size, height, color);
}

void zappy::graphics::renderers::EntityRenderer::renderColoredCube(const MathUtils::Vector3f& position, 
    const MathUtils::Vector3f& size,
    const sf::Color& color)
{
    renderPlayerGeometry(position.x, position.y, position.z, 
        size.x, size.y, color);
}

bool zappy::graphics::renderers::EntityRenderer::isCreeperEnabled() const
{
    return _creeper_model && _creeper_model->isEnabled();
}

bool zappy::graphics::renderers::EntityRenderer::isCreeperTextureEnabled() const
{
    return _creeper_model && _creeper_model->isTextureEnabled();
}

void zappy::graphics::renderers::EntityRenderer::setDeltaTime(float delta_time)
{
    _last_delta_time = delta_time;
}

size_t zappy::graphics::renderers::EntityRenderer::getPlayersRendered() const
{
    return _players_rendered;
}

void zappy::graphics::renderers::EntityRenderer::resetStatistics()
{
    _players_rendered = 0;
}

void zappy::graphics::renderers::EntityRenderer::renderPlayerTraditional(const zappy::game::entities::ZappyPlayer& player)
{
    MathUtils::Vector3f pos = getPlayerPosition(player);
    sf::Color color = player.color;
    
    applyBrightness(color);
    
    float size = _config.player_size;
    float height = _config.player_height;
    
    if (_config.level_scaling) {
        float scale_factor = 1.0f + (player.level * 0.1f);
        size *= scale_factor;
        height *= scale_factor;
    }
    
    renderPlayerGeometry(pos.x, pos.y, pos.z, size, height, color);
}

void zappy::graphics::renderers::EntityRenderer::renderPlayerGeometry(float x, float y, float z, 
    float size, float height, 
    const sf::Color& color)
{
    OpenGLUtils::setColor(color);
    
    glPushMatrix();
    glTranslatef(x, y + height/2, z);
    glScalef(size, height, size);
    
    // Render cube
    glBegin(GL_QUADS);
    
    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    
    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    
    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    
    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    
    glEnd();
    glPopMatrix();
}

void zappy::graphics::renderers::EntityRenderer::renderIndicatorGeometry(float x, float y, float z,
    float size, float height,
    const sf::Color& color)
{
    OpenGLUtils::setColor(color);
    
    glPushMatrix();
    glTranslatef(x, y + height/2, z);
    glScalef(size, height, size);
    
    glBegin(GL_TRIANGLES);
    
    // Front
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f,  0.5f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    
    // Back
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f,  0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    
    // Right
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f,  0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    
    // Left
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f,  0.5f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    
    glEnd();
    glPopMatrix();
}

MathUtils::Vector3f zappy::graphics::renderers::EntityRenderer::getPlayerPosition(const zappy::game::entities::ZappyPlayer& player) const
{
    return MathUtils::Vector3f(
        static_cast<float>(player.x),
        _config.player_y_offset,
        static_cast<float>(player.y)
    );
}

MathUtils::Vector3f zappy::graphics::renderers::EntityRenderer::getIndicatorPosition(const zappy::game::entities::ZappyPlayer& player) const
{
    sf::Vector2f dir = player.getDirectionVector();
    MathUtils::Vector3f player_pos = getPlayerPosition(player);
    
    return MathUtils::Vector3f(
        player_pos.x + dir.x * (_config.player_size/2 + _config.indicator_distance),
        player_pos.y + _config.player_height * 0.5f,
        player_pos.z + dir.y * (_config.player_size/2 + _config.indicator_distance)
    );
}

void zappy::graphics::renderers::EntityRenderer::applyBrightness(sf::Color& color) const
{
    if (_config.brightness_factor != 1.0f) {
        color.r = static_cast<sf::Uint8>(std::min(255.0f, color.r * _config.brightness_factor));
        color.g = static_cast<sf::Uint8>(std::min(255.0f, color.g * _config.brightness_factor));
        color.b = static_cast<sf::Uint8>(std::min(255.0f, color.b * _config.brightness_factor));
    }
}
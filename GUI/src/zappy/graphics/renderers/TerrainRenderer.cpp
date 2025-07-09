/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Terrain rendering
*/

#include "zappy/graphics/renderers/TerrainRenderer.hpp"
#include "zappy/graphics/utils/OpenGLUtils.hpp"
#include <iostream>
#include <cmath>

zappy::graphics::renderers::TerrainRenderer::TerrainRenderer()
{
    //std::cout << "🌍 TerrainRenderer created" << std::endl;
    
    // Configure for production: no coordinate display by default
    _config.show_tile_coordinates = false;
    _config.show_grid = true;
    _config.show_tile_outlines = true;
    _config.show_resource_stacks = true;
}

void zappy::graphics::renderers::TerrainRenderer::initialize(zappy::graphics::utils::TextureManager* texture_manager)
{
    if (_initialized) return;
    
    _texture_manager = texture_manager;
    _initialized = true;
    
    //std::cout << "🌍 TerrainRenderer initialized!" << std::endl;
}

void zappy::graphics::renderers::TerrainRenderer::renderTile(int x, int z, const zappy::game::world::ZappyTile& tile)
{
    if (!_initialized) return;
    
    float world_x = static_cast<float>(x);
    float world_z = static_cast<float>(z);
    float world_y = 0.0f;
    
    // Always render the base empty tile first
    renderEmptyTile(world_x, world_y, world_z);
    
    // Render tile outline if enabled
    if (_config.show_tile_outlines) {
        renderTileOutline(world_x, world_y, world_z, 
        _config.tile_size, _config.tile_height);
    }
    
    // Render coordinate display only if explicitly enabled (disabled by default)
    if (_config.show_tile_coordinates) {
        renderTileCoordinates(x, z);
    }
    
    // Render resource stacks on top if enabled
    if (_config.show_resource_stacks && tile.getTotalResources() > 0) {
        renderResourceStacks(world_x, world_y, world_z, tile);
    }
    
    _tiles_rendered++;
}

void zappy::graphics::renderers::TerrainRenderer::renderTileMap(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map)
{
    resetStatistics();
    
    // Render grid lines first if enabled and not on top
    if (_config.show_grid && !_config.grid_on_top) {
        renderGridLines(world_map);
    }
    
    // Render all tiles
    for (int y = 0; y < static_cast<int>(world_map.size()); y++) {
        for (int x = 0; x < static_cast<int>(world_map[y].size()); x++) {
            renderTile(x, y, world_map[y][x]);
        }
    }
    
    // Render grid lines on top if enabled
    if (_config.show_grid && _config.grid_on_top) {
        renderGridLines(world_map);
    }
}

void zappy::graphics::renderers::TerrainRenderer::renderTileRange(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map,
    int start_x, int start_z, int end_x, int end_z)
{
    for (int z = start_z; z <= end_z && z < static_cast<int>(world_map.size()); z++) {
        for (int x = start_x; x <= end_x && x < static_cast<int>(world_map[z].size()); x++) {
            renderTile(x, z, world_map[z][x]);
        }
    }
}

void zappy::graphics::renderers::TerrainRenderer::renderEmptyTile(float x, float y, float z)
{
    if (_config.use_textures && _texture_manager) {
        renderTexturedCube(x, y, z, _config.tile_size, _config.tile_height);
    } else {
        // Fallback to colored cube
        sf::Color grass_color(100, 200, 100);
        applyBrightnessBoost(grass_color);
        renderColoredCube(x, y, z, _config.tile_size, _config.tile_height, grass_color);
    }
}

void zappy::graphics::renderers::TerrainRenderer::renderResourceTile(float x, float y, float z, const sf::Color& color)
{
    renderColoredCube(x, y, z, _config.tile_size, _config.tile_height, color);
}

void zappy::graphics::renderers::TerrainRenderer::renderResourceStacks(float x, float y, float z, const zappy::game::world::ZappyTile& tile)
{
    float cube_size = _config.tile_size * _config.resource_cube_size_factor;
    float cube_height = _config.resource_cube_height;
    
    // Define resource information
    struct ResourceInfo {
        int count;
        sf::Color color;
    };
    
    ResourceInfo resources[] = {
        {tile.food, sf::Color(255, 255, 0)},           // Yellow
        {tile.linemate, sf::Color(192, 192, 192)},     // Silver
        {tile.deraumere, sf::Color(139, 69, 19)},      // Brown
        {tile.sibur, sf::Color(0, 100, 255)},          // Blue
        {tile.mendiane, sf::Color(255, 0, 255)},       // Magenta
        {tile.phiras, sf::Color(255, 0, 0)},           // Red
        {tile.thystame, sf::Color(128, 0, 128)}        // Purple
    };
    
    float offset_angle = 0.0f;
    const float angle_step = 2.0f * 3.14159f / 7.0f; // Distribute around center
    
    for (const auto& resource : resources) {
        if (resource.count > 0) {
            // Use the height of the terrain + tile height
            float base_y = y + _config.tile_height; // On top of the tile
            renderResourceStack(x, base_y, z, resource.count, resource.color, 
                cube_size, cube_height, offset_angle);
            offset_angle += angle_step;
        }
    }
}

void zappy::graphics::renderers::TerrainRenderer::renderResourceStack(float center_x, float center_y, float center_z,
    int count, const sf::Color& color,
    float cube_size, float cube_height, float angle_offset)
{
    // Calculate position offset from center for this resource type
    float radius = _config.tile_size * 0.25f;
    float pos_x = center_x + std::cos(angle_offset) * radius;
    float pos_z = center_z + std::sin(angle_offset) * radius;
    
    // Calculate number of cubes to stack (every 5 resources = 1 cube, minimum 1)
    int cubes_to_render = std::max(1, (count + 4) / 5); // Round up division by 5
    cubes_to_render = std::min(cubes_to_render, _config.max_stack_height);
    
    sf::Color final_color = color;
    applyBrightnessBoost(final_color);
    
    // Render stack of cubes - position Y relative to terrain
    for (int i = 0; i < cubes_to_render; i++) {
        // Position cubes directly on the terrain base
        float stack_y = center_y + (i * cube_height) + (cube_height / 2.0f); // Center of cube
        renderResourceCube(pos_x, stack_y, pos_z, cube_size, cube_height, final_color);
    }
}

void zappy::graphics::renderers::TerrainRenderer::renderResourceCube(float x, float y, float z, float size, float height, 
    const sf::Color& color)
{
    float half_size = size / 2.0f;
    
    OpenGLUtils::disableTexturing();
    OpenGLUtils::setColor(color);
    
    // Render cube with proper normals
    glBegin(GL_QUADS);
    
    // Top face
    OpenGLUtils::setNormal(0.0f, 1.0f, 0.0f);
    OpenGLUtils::setVertex(x - half_size, y + height, z - half_size);
    OpenGLUtils::setVertex(x + half_size, y + height, z - half_size);
    OpenGLUtils::setVertex(x + half_size, y + height, z + half_size);
    OpenGLUtils::setVertex(x - half_size, y + height, z + half_size);
    
    // Bottom face
    OpenGLUtils::setNormal(0.0f, -1.0f, 0.0f);
    OpenGLUtils::setVertex(x - half_size, y, z - half_size);
    OpenGLUtils::setVertex(x + half_size, y, z - half_size);
    OpenGLUtils::setVertex(x + half_size, y, z + half_size);
    OpenGLUtils::setVertex(x - half_size, y, z + half_size);
    
    // Front face
    OpenGLUtils::setNormal(0.0f, 0.0f, 1.0f);
    OpenGLUtils::setVertex(x - half_size, y, z + half_size);
    OpenGLUtils::setVertex(x + half_size, y, z + half_size);
    OpenGLUtils::setVertex(x + half_size, y + height, z + half_size);
    OpenGLUtils::setVertex(x - half_size, y + height, z + half_size);
    
    // Back face
    OpenGLUtils::setNormal(0.0f, 0.0f, -1.0f);
    OpenGLUtils::setVertex(x + half_size, y, z - half_size);
    OpenGLUtils::setVertex(x - half_size, y, z - half_size);
    OpenGLUtils::setVertex(x - half_size, y + height, z - half_size);
    OpenGLUtils::setVertex(x + half_size, y + height, z - half_size);
    
    // Left face
    OpenGLUtils::setNormal(-1.0f, 0.0f, 0.0f);
    OpenGLUtils::setVertex(x - half_size, y, z - half_size);
    OpenGLUtils::setVertex(x - half_size, y, z + half_size);
    OpenGLUtils::setVertex(x - half_size, y + height, z + half_size);
    OpenGLUtils::setVertex(x - half_size, y + height, z - half_size);
    
    // Right face
    OpenGLUtils::setNormal(1.0f, 0.0f, 0.0f);
    OpenGLUtils::setVertex(x + half_size, y, z + half_size);
    OpenGLUtils::setVertex(x + half_size, y, z - half_size);
    OpenGLUtils::setVertex(x + half_size, y + height, z - half_size);
    OpenGLUtils::setVertex(x + half_size, y + height, z + half_size);
    
    glEnd();
}

void zappy::graphics::renderers::TerrainRenderer::renderTexturedCube(float x, float y, float z, float size, float height)
{
    if (!_texture_manager) {
        renderColoredCube(x, y, z, size, height, sf::Color(100, 200, 100));
        return;
    }
    
    if (_texture_manager->hasTexture(zappy::graphics::utils::TextureManager::TextureType::GRASS) && 
        _texture_manager->hasTexture(zappy::graphics::utils::TextureManager::TextureType::DIRT)) {
        
        setupTexturedRendering();
        OpenGLUtils::setColor(sf::Color::White); // Let default handle shading
        
        // TOP FACE - Grass texture
        _texture_manager->bindTexture(zappy::graphics::utils::TextureManager::TextureType::GRASS);
        renderCubeFace(x, y, z, size, height, FACE_TOP, true);
        
        // SIDE FACES - Dirt texture
        _texture_manager->bindTexture(zappy::graphics::utils::TextureManager::TextureType::DIRT);
        renderCubeFace(x, y, z, size, height, FACE_FRONT, true);
        renderCubeFace(x, y, z, size, height, FACE_BACK, true);
        renderCubeFace(x, y, z, size, height, FACE_LEFT, true);
        renderCubeFace(x, y, z, size, height, FACE_RIGHT, true);
        renderCubeFace(x, y, z, size, height, FACE_BOTTOM, true);
        
        _texture_manager->unbindTexture();
    } else {
        // Fallback to colored rendering
        sf::Color grass_color(100, 200, 100);
        sf::Color dirt_color(150, 100, 50);
        applyBrightnessBoost(grass_color);
        applyBrightnessBoost(dirt_color);
        
        setupColoredRendering();
        
        // Top face - grass color
        OpenGLUtils::setColor(grass_color);
        renderCubeFace(x, y, z, size, height, FACE_TOP, false);
        
        // Side faces - dirt color
        OpenGLUtils::setColor(dirt_color);
        renderCubeFace(x, y, z, size, height, FACE_FRONT, false);
        renderCubeFace(x, y, z, size, height, FACE_BACK, false);
        renderCubeFace(x, y, z, size, height, FACE_LEFT, false);
        renderCubeFace(x, y, z, size, height, FACE_RIGHT, false);
        renderCubeFace(x, y, z, size, height, FACE_BOTTOM, false);
    }
}

void zappy::graphics::renderers::TerrainRenderer::renderColoredCube(float x, float y, float z, float size, float height, 
const sf::Color& color)
{
    setupColoredRendering();
    OpenGLUtils::setColor(color);
    renderCubeGeometry(x, y, z, size, height);
}

void zappy::graphics::renderers::TerrainRenderer::renderCubeFace(float x, float y, float z, float size, float height,
    int face, bool textured)
{
    float half_size = size / 2.0f;
    
    glBegin(GL_QUADS);
    
    switch (face) {
        case FACE_TOP:
            OpenGLUtils::setNormal(0.0f, 1.0f, 0.0f);
            if (textured) {
                OpenGLUtils::setTexCoord(0.0f, 0.0f); OpenGLUtils::setVertex(x - half_size, y + height, z - half_size);
                OpenGLUtils::setTexCoord(1.0f, 0.0f); OpenGLUtils::setVertex(x + half_size, y + height, z - half_size);
                OpenGLUtils::setTexCoord(1.0f, 1.0f); OpenGLUtils::setVertex(x + half_size, y + height, z + half_size);
                OpenGLUtils::setTexCoord(0.0f, 1.0f); OpenGLUtils::setVertex(x - half_size, y + height, z + half_size);
            } else {
                OpenGLUtils::setVertex(x - half_size, y + height, z - half_size);
                OpenGLUtils::setVertex(x + half_size, y + height, z - half_size);
                OpenGLUtils::setVertex(x + half_size, y + height, z + half_size);
                OpenGLUtils::setVertex(x - half_size, y + height, z + half_size);
            }
            break;
            
        case FACE_FRONT:
            OpenGLUtils::setNormal(0.0f, 0.0f, 1.0f);
            if (textured) {
                OpenGLUtils::setTexCoord(0.0f, 0.0f); OpenGLUtils::setVertex(x - half_size, y, z + half_size);
                OpenGLUtils::setTexCoord(1.0f, 0.0f); OpenGLUtils::setVertex(x + half_size, y, z + half_size);
                OpenGLUtils::setTexCoord(1.0f, 1.0f); OpenGLUtils::setVertex(x + half_size, y + height, z + half_size);
                OpenGLUtils::setTexCoord(0.0f, 1.0f); OpenGLUtils::setVertex(x - half_size, y + height, z + half_size);
            } else {
                OpenGLUtils::setVertex(x - half_size, y, z + half_size);
                OpenGLUtils::setVertex(x + half_size, y, z + half_size);
                OpenGLUtils::setVertex(x + half_size, y + height, z + half_size);
                OpenGLUtils::setVertex(x - half_size, y + height, z + half_size);
            }
            break;
            
        case FACE_BACK:
            OpenGLUtils::setNormal(0.0f, 0.0f, -1.0f);
            if (textured) {
                OpenGLUtils::setTexCoord(1.0f, 0.0f); OpenGLUtils::setVertex(x + half_size, y, z - half_size);
                OpenGLUtils::setTexCoord(0.0f, 0.0f); OpenGLUtils::setVertex(x - half_size, y, z - half_size);
                OpenGLUtils::setTexCoord(0.0f, 1.0f); OpenGLUtils::setVertex(x - half_size, y + height, z - half_size);
                OpenGLUtils::setTexCoord(1.0f, 1.0f); OpenGLUtils::setVertex(x + half_size, y + height, z - half_size);
            } else {
                OpenGLUtils::setVertex(x + half_size, y, z - half_size);
                OpenGLUtils::setVertex(x - half_size, y, z - half_size);
                OpenGLUtils::setVertex(x - half_size, y + height, z - half_size);
                OpenGLUtils::setVertex(x + half_size, y + height, z - half_size);
            }
            break;
            
        case FACE_LEFT:
            OpenGLUtils::setNormal(-1.0f, 0.0f, 0.0f);
            if (textured) {
                OpenGLUtils::setTexCoord(1.0f, 0.0f); OpenGLUtils::setVertex(x - half_size, y, z - half_size);
                OpenGLUtils::setTexCoord(0.0f, 0.0f); OpenGLUtils::setVertex(x - half_size, y, z + half_size);
                OpenGLUtils::setTexCoord(0.0f, 1.0f); OpenGLUtils::setVertex(x - half_size, y + height, z + half_size);
                OpenGLUtils::setTexCoord(1.0f, 1.0f); OpenGLUtils::setVertex(x - half_size, y + height, z - half_size);
            } else {
                OpenGLUtils::setVertex(x - half_size, y, z - half_size);
                OpenGLUtils::setVertex(x - half_size, y, z + half_size);
                OpenGLUtils::setVertex(x - half_size, y + height, z + half_size);
                OpenGLUtils::setVertex(x - half_size, y + height, z - half_size);
            }
            break;
            
        case FACE_RIGHT:
            OpenGLUtils::setNormal(1.0f, 0.0f, 0.0f);
            if (textured) {
                OpenGLUtils::setTexCoord(0.0f, 0.0f); OpenGLUtils::setVertex(x + half_size, y, z + half_size);
                OpenGLUtils::setTexCoord(1.0f, 0.0f); OpenGLUtils::setVertex(x + half_size, y, z - half_size);
                OpenGLUtils::setTexCoord(1.0f, 1.0f); OpenGLUtils::setVertex(x + half_size, y + height, z - half_size);
                OpenGLUtils::setTexCoord(0.0f, 1.0f); OpenGLUtils::setVertex(x + half_size, y + height, z + half_size);
            } else {
                OpenGLUtils::setVertex(x + half_size, y, z + half_size);
                OpenGLUtils::setVertex(x + half_size, y, z - half_size);
                OpenGLUtils::setVertex(x + half_size, y + height, z - half_size);
                OpenGLUtils::setVertex(x + half_size, y + height, z + half_size);
            }
            break;
            
        case FACE_BOTTOM:
            OpenGLUtils::setNormal(0.0f, -1.0f, 0.0f);
            if (textured) {
                OpenGLUtils::setTexCoord(0.0f, 0.0f); OpenGLUtils::setVertex(x - half_size, y, z - half_size);
                OpenGLUtils::setTexCoord(1.0f, 0.0f); OpenGLUtils::setVertex(x + half_size, y, z - half_size);
                OpenGLUtils::setTexCoord(1.0f, 1.0f); OpenGLUtils::setVertex(x + half_size, y, z + half_size);
                OpenGLUtils::setTexCoord(0.0f, 1.0f); OpenGLUtils::setVertex(x - half_size, y, z + half_size);
            } else {
                OpenGLUtils::setVertex(x - half_size, y, z - half_size);
                OpenGLUtils::setVertex(x + half_size, y, z - half_size);
                OpenGLUtils::setVertex(x + half_size, y, z + half_size);
                OpenGLUtils::setVertex(x - half_size, y, z + half_size);
            }
            break;
    }
    
    glEnd();
}

void zappy::graphics::renderers::TerrainRenderer::renderCubeGeometry(float x, float y, float z, float size, float height)
{
    renderCubeFace(x, y, z, size, height, FACE_TOP, false);
    renderCubeFace(x, y, z, size, height, FACE_FRONT, false);
    renderCubeFace(x, y, z, size, height, FACE_BACK, false);
    renderCubeFace(x, y, z, size, height, FACE_LEFT, false);
    renderCubeFace(x, y, z, size, height, FACE_RIGHT, false);
    renderCubeFace(x, y, z, size, height, FACE_BOTTOM, false);
}

void zappy::graphics::renderers::TerrainRenderer::setupTexturedRendering()
{
    if (_texture_manager) {
        _texture_manager->enableTexturing();
    }
}

void zappy::graphics::renderers::TerrainRenderer::setupColoredRendering()
{
    if (_texture_manager) {
        _texture_manager->disableTexturing();
    }
}

void zappy::graphics::renderers::TerrainRenderer::setColor(const sf::Color& color)
{
    OpenGLUtils::setColor(color);
}

void zappy::graphics::renderers::TerrainRenderer::applyBrightnessBoost(sf::Color& color) const
{
    if (_config.brightness_boost != 1.0f) {
        color.r = static_cast<sf::Uint8>(std::min(255.0f, color.r * _config.brightness_boost));
        color.g = static_cast<sf::Uint8>(std::min(255.0f, color.g * _config.brightness_boost));
        color.b = static_cast<sf::Uint8>(std::min(255.0f, color.b * _config.brightness_boost));
    }
}

void zappy::graphics::renderers::TerrainRenderer::resetStatistics()
{
    _tiles_rendered = 0;
}

void zappy::graphics::renderers::TerrainRenderer::renderTileOutline(float x, float y, float z, float size, float height)
{
    float half_size = size / 2.0f;
    
    // Setup for line rendering
    OpenGLUtils::disableTexturing();
    OpenGLUtils::setColor(_config.outline_color);
    
    glLineWidth(_config.outline_width);
    
    // Draw outline of the cube
    glBegin(GL_LINES);
    
    // Bottom face outline
    glVertex3f(x - half_size, y, z - half_size);
    glVertex3f(x + half_size, y, z - half_size);
    
    glVertex3f(x + half_size, y, z - half_size);
    glVertex3f(x + half_size, y, z + half_size);
    
    glVertex3f(x + half_size, y, z + half_size);
    glVertex3f(x - half_size, y, z + half_size);
    
    glVertex3f(x - half_size, y, z + half_size);
    glVertex3f(x - half_size, y, z - half_size);
    
    // Top face outline
    glVertex3f(x - half_size, y + height, z - half_size);
    glVertex3f(x + half_size, y + height, z - half_size);
    
    glVertex3f(x + half_size, y + height, z - half_size);
    glVertex3f(x + half_size, y + height, z + half_size);
    
    glVertex3f(x + half_size, y + height, z + half_size);
    glVertex3f(x - half_size, y + height, z + half_size);
    
    glVertex3f(x - half_size, y + height, z + half_size);
    glVertex3f(x - half_size, y + height, z - half_size);
    
    // Vertical edges
    glVertex3f(x - half_size, y, z - half_size);
    glVertex3f(x - half_size, y + height, z - half_size);
    
    glVertex3f(x + half_size, y, z - half_size);
    glVertex3f(x + half_size, y + height, z - half_size);
    
    glVertex3f(x + half_size, y, z + half_size);
    glVertex3f(x + half_size, y + height, z + half_size);
    
    glVertex3f(x - half_size, y, z + half_size);
    glVertex3f(x - half_size, y + height, z + half_size);
    
    glEnd();
    
    // Reset line width
    glLineWidth(1.0f);
    
    // Re-enable texturing
    OpenGLUtils::enableTexturing();
}

void zappy::graphics::renderers::TerrainRenderer::renderGridLines(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map)
{
    if (world_map.empty()) return;
    
    int map_height = static_cast<int>(world_map.size());
    int map_width = static_cast<int>(world_map[0].size());
    
    // Setup for grid rendering
    OpenGLUtils::disableTexturing();
    OpenGLUtils::setColor(_config.grid_color);
    
    glLineWidth(_config.grid_line_width);
    
    float grid_y = _config.tile_height + 0.01f;
    float tile_size = _config.tile_size;
    
    glBegin(GL_LINES);
    
    // Horizontal lines (along X axis)
    for (int z = 0; z <= map_height; z++) {
        float world_z = static_cast<float>(z) - 0.5f;
        renderGridLine(-0.5f * tile_size, grid_y, world_z,
            (map_width - 0.5f) * tile_size, grid_y, world_z);
    }
    
    // Vertical lines (along Z axis)
    for (int x = 0; x <= map_width; x++) {
        float world_x = static_cast<float>(x) - 0.5f;
        renderGridLine(world_x, grid_y, -0.5f * tile_size,
            world_x, grid_y, (map_height - 0.5f) * tile_size);
    }
    
    glEnd();
    
    // Reset line width
    glLineWidth(1.0f);
    
    // Re-enable texturing
    OpenGLUtils::enableTexturing();
}

void zappy::graphics::renderers::TerrainRenderer::renderGridLine(float x1, float y1, float z1, float x2, float y2, float z2)
{
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
}

void zappy::graphics::renderers::TerrainRenderer::renderTileCoordinates(int x, int z)
{
    if (!_config.show_tile_coordinates) {
        return;
    }
    
    float world_x = static_cast<float>(x);
    float world_z = static_cast<float>(z);
    float world_y = _config.tile_height + _config.coordinate_height_offset;
    
    // Create coordinate text
    std::string coord_text = "(" + std::to_string(x) + "," + std::to_string(z) + ")";
    
    // Render the text in 3D space
    renderText3D(coord_text, world_x, world_y, world_z, 
        _config.coordinate_color, _config.coordinate_scale);
}

void zappy::graphics::renderers::TerrainRenderer::renderText3D(const std::string& text, float x, float y, float z, 
const sf::Color& color, float scale)
{
    OpenGLUtils::disableTexturing();
    OpenGLUtils::setColor(color);
    
    glLineWidth(2.0f);
    
    float char_spacing = scale * 0.8f;
    float start_x = x - (text.length() * char_spacing) * 0.5f; // Center the text
    
    for (size_t i = 0; i < text.length(); ++i) {
        float char_x = start_x + i * char_spacing;
        renderCharacter(text[i], char_x, y, z, scale, color);
    }
    
    glLineWidth(1.0f);
    OpenGLUtils::enableTexturing();
}

void zappy::graphics::renderers::TerrainRenderer::renderCharacter(char c, float x, float y, float z, float scale, const sf::Color& color)
{
    (void)color;
    
    glBegin(GL_LINES);
    
    float s = scale;
    
    switch (c) {
        case '0':
            // Rectangle
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z - s*0.4f); // Top
            glVertex3f(x + s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f); // Right
            glVertex3f(x + s*0.3f, y, z + s*0.4f); glVertex3f(x - s*0.3f, y, z + s*0.4f); // Bottom
            glVertex3f(x - s*0.3f, y, z + s*0.4f); glVertex3f(x - s*0.3f, y, z - s*0.4f); // Left
            break;
            
        case '1':
            // Vertical line
            glVertex3f(x, y, z - s*0.4f); glVertex3f(x, y, z + s*0.4f);
            break;
            
        case '2':
            // Top horizontal
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z - s*0.4f);
            // Right vertical (top half)
            glVertex3f(x + s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z);
            // Middle horizontal
            glVertex3f(x + s*0.3f, y, z); glVertex3f(x - s*0.3f, y, z);
            // Left vertical (bottom half)
            glVertex3f(x - s*0.3f, y, z); glVertex3f(x - s*0.3f, y, z + s*0.4f);
            // Bottom horizontal
            glVertex3f(x - s*0.3f, y, z + s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            break;
            
        case '3':
            // Top horizontal
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z - s*0.4f);
            // Right vertical
            glVertex3f(x + s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            // Middle horizontal
            glVertex3f(x - s*0.1f, y, z); glVertex3f(x + s*0.3f, y, z);
            // Bottom horizontal
            glVertex3f(x - s*0.3f, y, z + s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            break;
            
        case '4':
            // Left vertical (top half)
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x - s*0.3f, y, z);
            // Right vertical (full)
            glVertex3f(x + s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            // Middle horizontal
            glVertex3f(x - s*0.3f, y, z); glVertex3f(x + s*0.3f, y, z);
            break;
            
        case '5':
            // Top horizontal
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z - s*0.4f);
            // Left vertical (top half)
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x - s*0.3f, y, z);
            // Middle horizontal
            glVertex3f(x - s*0.3f, y, z); glVertex3f(x + s*0.3f, y, z);
            // Right vertical (bottom half)
            glVertex3f(x + s*0.3f, y, z); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            // Bottom horizontal
            glVertex3f(x - s*0.3f, y, z + s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            break;
            
        case '6':
            // Full left vertical
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x - s*0.3f, y, z + s*0.4f);
            // Top horizontal
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z - s*0.4f);
            // Middle horizontal
            glVertex3f(x - s*0.3f, y, z); glVertex3f(x + s*0.3f, y, z);
            // Right vertical (bottom half)
            glVertex3f(x + s*0.3f, y, z); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            // Bottom horizontal
            glVertex3f(x - s*0.3f, y, z + s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            break;
            
        case '7':
            // Top horizontal
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z - s*0.4f);
            // Right diagonal
            glVertex3f(x + s*0.3f, y, z - s*0.4f); glVertex3f(x - s*0.1f, y, z + s*0.4f);
            break;
            
        case '8':
            // Rectangle (like 0)
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z - s*0.4f);
            glVertex3f(x + s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            glVertex3f(x + s*0.3f, y, z + s*0.4f); glVertex3f(x - s*0.3f, y, z + s*0.4f);
            glVertex3f(x - s*0.3f, y, z + s*0.4f); glVertex3f(x - s*0.3f, y, z - s*0.4f);
            // Middle horizontal
            glVertex3f(x - s*0.3f, y, z); glVertex3f(x + s*0.3f, y, z);
            break;
            
        case '9':
            // Top part (like 6 flipped)
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z - s*0.4f);
            glVertex3f(x + s*0.3f, y, z - s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            glVertex3f(x - s*0.3f, y, z - s*0.4f); glVertex3f(x - s*0.3f, y, z);
            glVertex3f(x - s*0.3f, y, z); glVertex3f(x + s*0.3f, y, z);
            // Bottom horizontal
            glVertex3f(x - s*0.3f, y, z + s*0.4f); glVertex3f(x + s*0.3f, y, z + s*0.4f);
            break;
            
        case '(':
            // Left parenthesis
            glVertex3f(x + s*0.1f, y, z - s*0.4f); glVertex3f(x - s*0.1f, y, z - s*0.2f);
            glVertex3f(x - s*0.1f, y, z - s*0.2f); glVertex3f(x - s*0.1f, y, z + s*0.2f);
            glVertex3f(x - s*0.1f, y, z + s*0.2f); glVertex3f(x + s*0.1f, y, z + s*0.4f);
            break;
            
        case ')':
            // Right parenthesis
            glVertex3f(x - s*0.1f, y, z - s*0.4f); glVertex3f(x + s*0.1f, y, z - s*0.2f);
            glVertex3f(x + s*0.1f, y, z - s*0.2f); glVertex3f(x + s*0.1f, y, z + s*0.2f);
            glVertex3f(x + s*0.1f, y, z + s*0.2f); glVertex3f(x - s*0.1f, y, z + s*0.4f);
            break;
            
        case ',':
            // Comma
            glVertex3f(x, y, z + s*0.3f); glVertex3f(x - s*0.1f, y, z + s*0.4f);
            break;
            
        case ' ':
            break;
            
        default:
            // Unknown character
            glVertex3f(x - s*0.1f, y, z - s*0.1f); glVertex3f(x + s*0.1f, y, z - s*0.1f);
            glVertex3f(x + s*0.1f, y, z - s*0.1f); glVertex3f(x + s*0.1f, y, z + s*0.1f);
            glVertex3f(x + s*0.1f, y, z + s*0.1f); glVertex3f(x - s*0.1f, y, z + s*0.1f);
            glVertex3f(x - s*0.1f, y, z + s*0.1f); glVertex3f(x - s*0.1f, y, z - s*0.1f);
            break;
    }
    
    glEnd();
}

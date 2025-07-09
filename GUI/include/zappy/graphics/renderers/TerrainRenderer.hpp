/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Terrain and tile rendering system - Clean final version
*/

#ifndef TERRAINRENDERER_HPP
#define TERRAINRENDERER_HPP

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include "zappy/game/world/Tile.hpp"
#include "zappy/graphics/utils/TextureManager.hpp"

namespace zappy::graphics::renderers {
class TerrainRenderer {
public:
    struct TerrainConfig {
        float tile_size = 1.05f;
        float tile_height = 1.0f;
        bool smooth_edges = true;
        
        bool use_textures = true;
        bool texture_filtering = true;
        
        bool frustum_culling = false;
        bool batched_rendering = false;
        float brightness_boost = 1.0f;
        bool color_variation = false;
        
        // Resource rendering settings
        bool show_resource_stacks = true;
        float resource_cube_size_factor = 0.15f;
        float resource_cube_height = 0.15f;
        int max_stack_height = 10;
        
        // Grid settings
        bool show_grid = true;
        sf::Color grid_color = sf::Color(80, 80, 80, 120);
        float grid_line_width = 1.0f;
        bool grid_on_top = false;
        
        // Tile outline settings
        bool show_tile_outlines = true;
        sf::Color outline_color = sf::Color(60, 60, 60, 100);
        float outline_width = 1.0f;
        
        // Coordinate display settings (disabled by default)
        bool show_tile_coordinates = false;
        sf::Color coordinate_color = sf::Color::White;
        float coordinate_scale = 0.1f;
        float coordinate_height_offset = 0.1f;
    };

public:
    TerrainRenderer();
    ~TerrainRenderer() = default;
    
    void initialize(zappy::graphics::utils::TextureManager* texture_manager);
    void setConfig(const TerrainConfig& config) { _config = config; }
    TerrainConfig getConfig() const { return _config; }
    
    void renderTile(int x, int z, const zappy::game::world::ZappyTile& tile);
    void renderEmptyTile(float x, float y, float z);
    void renderResourceTile(float x, float y, float z, const sf::Color& color);
    
    // Resource rendering methods
    void renderResourceStacks(float x, float y, float z, const zappy::game::world::ZappyTile& tile);
    void renderResourceStack(float center_x, float center_y, float center_z,
                           int count, const sf::Color& color,
                           float cube_size, float cube_height, float angle_offset);
    void renderResourceCube(float x, float y, float z, float size, float height, 
                           const sf::Color& color);
    
    void renderTileMap(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map);
    void renderTileRange(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map,
                        int start_x, int start_z, int end_x, int end_z);
    
    void renderTexturedCube(float x, float y, float z, float size, float height);
    void renderColoredCube(float x, float y, float z, float size, float height, 
                          const sf::Color& color);
    
    // Grid and outline methods
    void renderGridLines(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map);
    void renderGridLine(float x1, float y1, float z1, float x2, float y2, float z2);
    void renderTileOutline(float x, float y, float z, float size, float height);
    
    // Coordinate display methods (disabled by default)
    void renderTileCoordinates(int x, int z);
    void renderText3D(const std::string& text, float x, float y, float z, 
                     const sf::Color& color, float scale);
    void renderCharacter(char c, float x, float y, float z, float scale, const sf::Color& color);
    
    void enableWireframe() { _wireframe_mode = true; }
    void disableWireframe() { _wireframe_mode = false; }
    bool isWireframeEnabled() const { return _wireframe_mode; }
    
    // Getters pour les états
    bool areResourceStacksEnabled() const { return _config.show_resource_stacks; }
    bool isGridEnabled() const { return _config.show_grid; }
    
    size_t getTilesRendered() const { return _tiles_rendered; }
    void resetStatistics();

private:
    void renderCubeFace(float x, float y, float z, float size, float height,
                       int face, bool textured = false);
    void setColor(const sf::Color& color);
    void applyBrightnessBoost(sf::Color& color) const;
    void renderCubeGeometry(float x, float y, float z, float size, float height);
    void setupTexturedRendering();
    void setupColoredRendering();
    
    TerrainConfig _config;
    
    zappy::graphics::utils::TextureManager* _texture_manager = nullptr;
    
    bool _initialized = false;
    bool _wireframe_mode = false;
    
    size_t _tiles_rendered = 0;
    
    enum CubeFace {
        FACE_TOP = 0,
        FACE_BOTTOM,
        FACE_FRONT, 
        FACE_BACK,
        FACE_LEFT,
        FACE_RIGHT
    };
};
}

#endif /* !TERRAINRENDERER_HPP */
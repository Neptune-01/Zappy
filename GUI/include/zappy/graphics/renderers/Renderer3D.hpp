/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Core 3D Renderer - coordinates all rendering systems (No Lighting/Shadows)
*/

#ifndef RENDERER3D_HPP
#define RENDERER3D_HPP

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <vector>
#include <map>
#include "zappy/game/world/Tile.hpp"
#include "zappy/game/entities/Player.hpp"
#include "zappy/graphics/utils/TextureManager.hpp"
#include "zappy/graphics/renderers/TerrainRenderer.hpp"
#include "zappy/graphics/renderers/EntityRenderer.hpp"

namespace zappy::graphics::renderers {
class Renderer3D {
public:
    struct RenderConfig {
        bool textures_enabled = true;
        
        bool smooth_shading = true;
        bool depth_testing = true;
        bool face_culling = false;
        
        bool frustum_culling = false;
        bool batch_rendering = false;
        
        bool wireframe_mode = false;
        bool show_normals = false;
    };

public:
    Renderer3D();
    ~Renderer3D();
    
    bool initialize();
    void shutdown();
    bool isInitialized() const { return _initialized; }
    
    void setConfig(const RenderConfig& config);
    RenderConfig getConfig() const { return _config; }
    
    void initOpenGL();
    void renderTileHighlight(int x, int z);
    void renderTile(int x, int z, const zappy::game::world::ZappyTile& tile);
    void renderPlayer(const zappy::game::entities::ZappyPlayer& player);
    void renderTileMap(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map);
    void renderPlayers(const std::map<int, zappy::game::entities::ZappyPlayer>& players);
    
    zappy::graphics::utils::TextureManager* getTextureManager() { return &_texture_manager; }
    zappy::graphics::renderers::TerrainRenderer* getTerrainRenderer() { return &_terrain_renderer; }
    zappy::graphics::renderers::EntityRenderer* getEntityRenderer() { return &_entity_renderer; }
    
    void renderText(const sf::Font& font, const std::string& text, 
                   float x, float y, const sf::Color& color);
    
    void enableWireframe() { _config.wireframe_mode = true; }
    void disableWireframe() { _config.wireframe_mode = false; }
    void toggleWireframe() { _config.wireframe_mode = !_config.wireframe_mode; }
    
    void printRenderingStatistics() const;

private:
    void setupSystems();
    void applyConfig();
    
    RenderConfig _config;
    
    zappy::graphics::utils::TextureManager _texture_manager;
    zappy::graphics::renderers::TerrainRenderer _terrain_renderer;
    zappy::graphics::renderers::EntityRenderer _entity_renderer;
    
    bool _initialized = false;
};
}
#endif /* !RENDERER3D_HPP */
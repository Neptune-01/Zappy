/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** UI rendering - Clean final version (minimal interface)
*/

#ifndef UIRENDERER_HPP
#define UIRENDERER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>
#include "zappy/game/world/Tile.hpp"
#include "zappy/game/entities/Player.hpp"

namespace zappy::graphics::renderers {
class UIRenderer {
public:
    struct UIConfig {
        // Stats panel - minimal
        sf::Vector2f stats_panel_position = sf::Vector2f(10, 10);
        sf::Vector2f stats_panel_size = sf::Vector2f(300, 150);
        sf::Color stats_panel_color = sf::Color(0, 0, 0, 120);
        sf::Color stats_panel_outline = sf::Color::White;
        float stats_panel_outline_thickness = 1.0f;
        
        // Tile info panel
        sf::Vector2f tile_info_panel_position = sf::Vector2f(10, 180);
        sf::Vector2f tile_info_panel_size = sf::Vector2f(280, 300);
        sf::Color tile_info_panel_color = sf::Color(0, 0, 50, 140);
        sf::Color tile_info_panel_outline = sf::Color::Cyan;
        float tile_info_panel_outline_thickness = 1.0f;
        
        // Player info panel
        sf::Vector2f player_info_panel_position = sf::Vector2f(1100, 10);
        sf::Vector2f player_info_panel_size = sf::Vector2f(280, 250);
        sf::Color player_info_panel_color = sf::Color(50, 0, 50, 140);
        sf::Color player_info_panel_outline = sf::Color::Magenta;
        float player_info_panel_outline_thickness = 1.0f;
        
        unsigned int stats_text_size = 13;
        unsigned int tile_info_text_size = 12;
        unsigned int player_info_text_size = 12;
        sf::Color stats_text_color = sf::Color::White;
        sf::Color tile_info_text_color = sf::Color::White;
        sf::Color player_info_text_color = sf::Color::White;
        
        bool show_stats_panel = true;
        bool show_tile_info_panel = true;
        bool show_player_info_panel = true;
        bool show_fps = true;
    };
    
    struct GameStats {
        int map_width = 10;
        int map_height = 10;
        int player_count = 0;
        float current_fps = 60.0f;
        
        float zoom_level = 1.0f;
        float rotation_degrees = 0.0f;
        
        int total_resources = 0; // Simplifié
    };
    
    struct TileInfo {
        bool has_selection = false;
        int x = -1;
        int y = -1;
        zappy::game::world::ZappyTile tile_data;
        std::vector<zappy::game::entities::ZappyPlayer> players_on_tile;
    };
    
    struct PlayerInfo {
        bool has_selection = false;
        int player_id = -1;
        zappy::game::entities::ZappyPlayer player_data;
        // Supprimé: distance_from_click, current_tile, nearby_players
    };

public:
    UIRenderer();
    ~UIRenderer() = default;
    
    void initialize(sf::RenderWindow* window);
    bool loadFont(const std::string& font_path = "");
    void setConfig(const UIConfig& config) { _config = config; }
    UIConfig getConfig() const { return _config; }
    
    void renderUI(const GameStats& stats);
    void renderUI(const GameStats& stats, const TileInfo& tile_info);
    void renderUI(const GameStats& stats, const TileInfo& tile_info, const PlayerInfo& player_info);
    
    void renderStatsPanel(const GameStats& stats);
    void renderTileInfoPanel(const TileInfo& tile_info);
    void renderPlayerInfoPanel(const PlayerInfo& player_info);
    
    GameStats calculateGameStats(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map,
                                const std::map<int, zappy::game::entities::ZappyPlayer>& players,
                                float fps, float zoom, float rotation) const;
    
    TileInfo createTileInfo(int x, int y, 
                           const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map,
                           const std::map<int, zappy::game::entities::ZappyPlayer>& players) const;
    
    PlayerInfo createPlayerInfo(int player_id, 
                               const std::map<int, zappy::game::entities::ZappyPlayer>& players) const;
    
    bool isFontLoaded() const { return _font_loaded; }

private:
    void renderPanel(const sf::Vector2f& position, const sf::Vector2f& size,
                    const sf::Color& fill_color, const sf::Color& outline_color,
                    float outline_thickness);
    void renderText(const std::string& text, const sf::Vector2f& position,
                   unsigned int size, const sf::Color& color);
    std::string formatStatsText(const GameStats& stats) const;
    std::string formatTileInfoText(const TileInfo& tile_info) const;
    std::string formatPlayerInfoText(const PlayerInfo& player_info) const;
    
    UIConfig _config;
    
    sf::RenderWindow* _window = nullptr;
    
    sf::Font _font;
    bool _font_loaded = false;
    
    bool _initialized = false;
};
}
#endif /* !UIRENDERER_HPP */
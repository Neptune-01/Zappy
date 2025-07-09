/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** UI rendering
*/

#include "zappy/graphics/renderers/UIRenderer.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

zappy::graphics::renderers::UIRenderer::UIRenderer()
{
    //std::cout << "🎨 UIRenderer created" << std::endl;
}

void zappy::graphics::renderers::UIRenderer::initialize(sf::RenderWindow* window)
{
    if (_initialized) return;
    
    _window = window;
    
    loadFont();
    
    _initialized = true;
    //std::cout << "🎨 UIRenderer initialized!" << std::endl;
}

bool zappy::graphics::renderers::UIRenderer::loadFont(const std::string& font_path)
{
    std::vector<std::string> font_paths = {
        font_path,
        "MineFont.otf",
        "./MineFont.otf", 
        "MineFont.otf",
        "assets/fonts/MineFont.otf"
    };
    
    for (const auto& path : font_paths) {
        if (path.empty()) continue;
        
        if (_font.loadFromFile(path)) {
            _font_loaded = true;
            //std::cout << "✅ Font loaded: " << path << std::endl;
            return true;
        }
    }
    
    //std::cout << "⚠️ Could not load any font, UI text may not display correctly" << std::endl;
    return false;
}

void zappy::graphics::renderers::UIRenderer::renderUI(const GameStats& stats)
{
    if (!_initialized || !_window) return;
    
    if (_config.show_stats_panel) {
        renderStatsPanel(stats);
    }
    
}

void zappy::graphics::renderers::UIRenderer::renderUI(const GameStats& stats, const TileInfo& tile_info)
{
    // Render standard UI
    renderUI(stats);
    
    // Render tile info panel if enabled and there's a selection
    if (_config.show_tile_info_panel && tile_info.has_selection) {
        renderTileInfoPanel(tile_info);
    }
}

void zappy::graphics::renderers::UIRenderer::renderUI(const GameStats& stats, const TileInfo& tile_info, const PlayerInfo& player_info)
{
    // Render standard UI
    renderUI(stats);
    
    // Render tile info panel if enabled and there's a selection
    if (_config.show_tile_info_panel && tile_info.has_selection) {
        renderTileInfoPanel(tile_info);
    }
    
    // Render player info panel if enabled and there's a selection
    if (_config.show_player_info_panel && player_info.has_selection) {
        renderPlayerInfoPanel(player_info);
    }
}

void zappy::graphics::renderers::UIRenderer::renderStatsPanel(const GameStats& stats)
{
    renderPanel(_config.stats_panel_position, _config.stats_panel_size,
        _config.stats_panel_color, _config.stats_panel_outline,
        _config.stats_panel_outline_thickness);
    
    std::string stats_text = formatStatsText(stats);
    sf::Vector2f text_position = _config.stats_panel_position + sf::Vector2f(10, 10);
    renderText(stats_text, text_position, _config.stats_text_size, _config.stats_text_color);
}

void zappy::graphics::renderers::UIRenderer::renderTileInfoPanel(const TileInfo& tile_info)
{
    if (!tile_info.has_selection) return;
    
    // Render panel background
    renderPanel(_config.tile_info_panel_position, _config.tile_info_panel_size,
        _config.tile_info_panel_color, _config.tile_info_panel_outline,
        _config.tile_info_panel_outline_thickness);
    
    // Render tile information text
    std::string tile_text = formatTileInfoText(tile_info);
    sf::Vector2f text_position = _config.tile_info_panel_position + sf::Vector2f(10, 10);
    renderText(tile_text, text_position, _config.tile_info_text_size, _config.tile_info_text_color);
}

void zappy::graphics::renderers::UIRenderer::renderPlayerInfoPanel(const PlayerInfo& player_info)
{
    if (!player_info.has_selection) return;
    
    // Render panel background
    renderPanel(_config.player_info_panel_position, _config.player_info_panel_size,
        _config.player_info_panel_color, _config.player_info_panel_outline,
        _config.player_info_panel_outline_thickness);
    
    // Render player information text
    std::string player_text = formatPlayerInfoText(player_info);
    sf::Vector2f text_position = _config.player_info_panel_position + sf::Vector2f(10, 10);
    renderText(player_text, text_position, _config.player_info_text_size, _config.player_info_text_color);
}

void zappy::graphics::renderers::UIRenderer::renderPanel(const sf::Vector2f& position, const sf::Vector2f& size,
    const sf::Color& fill_color, const sf::Color& outline_color,
    float outline_thickness)
{
    sf::RectangleShape panel(size);
    panel.setPosition(position);
    panel.setFillColor(fill_color);
    panel.setOutlineThickness(outline_thickness);
    panel.setOutlineColor(outline_color);
    
    _window->pushGLStates();
    _window->draw(panel);
    _window->popGLStates();
}

void zappy::graphics::renderers::UIRenderer::renderText(const std::string& text, const sf::Vector2f& position,
    unsigned int size, const sf::Color& color)
{
    if (!_font_loaded) return;
    
    sf::Text sf_text;
    sf_text.setFont(_font);
    sf_text.setString(text);
    sf_text.setCharacterSize(size);
    sf_text.setFillColor(color);
    sf_text.setPosition(position);
    
    _window->pushGLStates();
    _window->draw(sf_text);
    _window->popGLStates();
}

std::string zappy::graphics::renderers::UIRenderer::formatStatsText(const GameStats& stats) const
{
    std::ostringstream ss;
    
    ss << "ZAPPY 3D VISUALIZER\n\n";
    ss << "Map: " << stats.map_width << "x" << stats.map_height << "\n";
    ss << "Players: " << stats.player_count << "\n";
    ss << "Zoom: " << std::fixed << std::setprecision(1) << stats.zoom_level << "x\n";
    ss << "Rotation: " << std::fixed << std::setprecision(0) << stats.rotation_degrees << "°\n";
    
    return ss.str();
}

std::string zappy::graphics::renderers::UIRenderer::formatTileInfoText(const TileInfo& tile_info) const
{
    std::ostringstream ss;
    
    //ss << "🎯 SELECTED TILE\n\n";
    //ss << "📍 Position: (" << tile_info.x << ", " << tile_info.y << ")\n\n";
    
    // Resources on this tile
    ss << "💎 Resources:\n";
    if (tile_info.tile_data.getTotalResources() == 0) {
        ss << "  ∅ No resources\n";
    } else {
        if (tile_info.tile_data.food > 0)
            ss << "  🌾 Food: " << tile_info.tile_data.food << "\n";
        if (tile_info.tile_data.linemate > 0)
            ss << "  ⚪ Linemate: " << tile_info.tile_data.linemate << "\n";
        if (tile_info.tile_data.deraumere > 0)
            ss << "  🟤 Deraumere: " << tile_info.tile_data.deraumere << "\n";
        if (tile_info.tile_data.sibur > 0)
            ss << "  🔵 Sibur: " << tile_info.tile_data.sibur << "\n";
        if (tile_info.tile_data.mendiane > 0)
            ss << "  🟣 Mendiane: " << tile_info.tile_data.mendiane << "\n";
        if (tile_info.tile_data.phiras > 0)
            ss << "  🔴 Phiras: " << tile_info.tile_data.phiras << "\n";
        if (tile_info.tile_data.thystame > 0)
            ss << "  💜 Thystame: " << tile_info.tile_data.thystame << "\n";
            
        ss << "  📊 Total: " << tile_info.tile_data.getTotalResources() << "\n";
    }
    
    ss << "\n";
    
    // Players on this tile
    ss << "👥 Players:\n";
    if (tile_info.players_on_tile.empty()) {
        ss << "  ∅ No players here\n";
    } else {
        for (const auto& player : tile_info.players_on_tile) {
            ss << "  👤 Player " << player.id << "\n";
            ss << "     Team: " << player.team << "\n";
            ss << "     Level: " << player.level << "\n";
            ss << "     Facing: " << player.getOrientationName() << "\n";
            if (tile_info.players_on_tile.size() > 1) {
                ss << "\n";
            }
        }
    }
    
    return ss.str();
}

std::string zappy::graphics::renderers::UIRenderer::formatPlayerInfoText(const PlayerInfo& player_info) const
{
    std::ostringstream ss;
    
    ss << "👤 SELECTED PLAYER\n\n";
    ss << "🆔 Player ID: " << player_info.player_data.id << "\n";
    ss << "👥 Team: " << player_info.player_data.team << "\n";
    ss << "⭐ Level: " << player_info.player_data.level << "\n";
    ss << "📍 Position: (" << player_info.player_data.x << ", " << player_info.player_data.y << ")\n";
    ss << "🧭 Facing: " << player_info.player_data.getOrientationName() << "\n\n";
    
    // Inventaire du joueur
    ss << "🎒 PLAYER INVENTORY:\n";
    if (!player_info.player_data.hasInventoryItems()) {
        ss << "  ∅ Inventory is empty\n";
    } else {
        if (player_info.player_data.inventory_food > 0)
            ss << "  🌾 Food: " << player_info.player_data.inventory_food << "\n";
        if (player_info.player_data.inventory_linemate > 0)
            ss << "  ⚪ Linemate: " << player_info.player_data.inventory_linemate << "\n";
        if (player_info.player_data.inventory_deraumere > 0)
            ss << "  🟤 Deraumere: " << player_info.player_data.inventory_deraumere << "\n";
        if (player_info.player_data.inventory_sibur > 0)
            ss << "  🔵 Sibur: " << player_info.player_data.inventory_sibur << "\n";
        if (player_info.player_data.inventory_mendiane > 0)
            ss << "  🟣 Mendiane: " << player_info.player_data.inventory_mendiane << "\n";
        if (player_info.player_data.inventory_phiras > 0)
            ss << "  🔴 Phiras: " << player_info.player_data.inventory_phiras << "\n";
        if (player_info.player_data.inventory_thystame > 0)
            ss << "  💜 Thystame: " << player_info.player_data.inventory_thystame << "\n";
            
        ss << "  📊 Total items: " << player_info.player_data.getTotalInventoryItems() << "\n";
    }
    
    return ss.str();
}

zappy::graphics::renderers::UIRenderer::GameStats zappy::graphics::renderers::UIRenderer::calculateGameStats(
    const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map,
    const std::map<int, zappy::game::entities::ZappyPlayer>& players,
    float fps, float zoom, float rotation) const
{
    GameStats stats;
    
    stats.map_width = static_cast<int>(world_map.empty() ? 0 : world_map[0].size());
    stats.map_height = static_cast<int>(world_map.size());
    stats.player_count = static_cast<int>(players.size());
    stats.current_fps = fps;
    stats.zoom_level = zoom;
    stats.rotation_degrees = rotation * 180.0f / 3.14159f;
    return stats;
}

zappy::graphics::renderers::UIRenderer::PlayerInfo zappy::graphics::renderers::UIRenderer::createPlayerInfo(int player_id, 
    const std::map<int, zappy::game::entities::ZappyPlayer>& players) const
{
    PlayerInfo info;
    
    // Find the player
    auto player_it = players.find(player_id);
    if (player_it == players.end()) {
        info.has_selection = false;
        return info;
    }
    
    info.has_selection = true;
    info.player_id = player_id;
    info.player_data = player_it->second;
    
    return info;
}

zappy::graphics::renderers::UIRenderer::TileInfo zappy::graphics::renderers::UIRenderer::createTileInfo(int x, int y, 
    const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map,
    const std::map<int, zappy::game::entities::ZappyPlayer>& players) const
{
    TileInfo info;
    
    // Check if coordinates are valid
    if (x < 0 || y < 0 || 
        y >= static_cast<int>(world_map.size()) || 
        x >= static_cast<int>(world_map[0].size())) {
        info.has_selection = false;
        return info;
    }
    
    info.has_selection = true;
    info.x = x;
    info.y = y;
    info.tile_data = world_map[y][x];
    
    // Find players on this tile
    info.players_on_tile.clear();
    for (const auto& player_pair : players) {
        const zappy::game::entities::ZappyPlayer& player = player_pair.second;
        if (player.x == x && player.y == y) {
            info.players_on_tile.push_back(player);
        }
    }
    
    return info;
}
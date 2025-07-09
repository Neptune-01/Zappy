/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Map generation with setup system implementation
*/

#include "zappy/game/world/MapGenerator.hpp"
#include <iostream>
#include <algorithm>

// Constantes par défaut pour la compatibilité
namespace {
    constexpr int DEFAULT_MAP_WIDTH = 10;
    constexpr int DEFAULT_MAP_HEIGHT = 10;
}

const std::vector<sf::Color> zappy::game::world::MapGenerator::TEAM_COLORS = {
    sf::Color::Red,
    sf::Color::Blue,
    sf::Color::Green,
    sf::Color::Yellow,
    sf::Color::Magenta,
    sf::Color::Cyan,
    sf::Color(255, 165, 0), // Orange
    sf::Color(128, 0, 128)  // Purple
};

zappy::game::world::MapGenerator::MapGenerator() : _gen(_rd()) {}

void zappy::game::world::MapGenerator::setupMap(std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map, const MapConfig& config)
{
    //std::cout << "🗺️ Setting up map " << config.width << "x" << config.height << std::endl;
    
    if (config.clear_existing) {
        world_map.clear();
    }
    
    ensureMapSize(world_map, config.width, config.height);
    
    // Initialize all tiles as empty
    for (int y = 0; y < config.height; y++) {
        for (int x = 0; x < config.width; x++) {
            world_map[y][x] = zappy::game::world::ZappyTile(x, y);
            // Ensure all resources start at zero
            world_map[y][x].food = 0;
            world_map[y][x].linemate = 0;
            world_map[y][x].deraumere = 0;
            world_map[y][x].sibur = 0;
            world_map[y][x].mendiane = 0;
            world_map[y][x].phiras = 0;
            world_map[y][x].thystame = 0;
        }
    }
    
    //std::cout << "✅ Map setup complete" << std::endl;
}

void zappy::game::world::MapGenerator::setupPlayer(std::map<int, zappy::game::entities::ZappyPlayer>& players, const PlayerConfig& config)
{
    if (!isValidOrientation(config.orientation)) {
        std::cerr << "❌ Invalid orientation " << config.orientation << " for player " << config.id << std::endl;
        return;
    }
    
    zappy::game::entities::ZappyPlayer player(config.id);
    player.x = config.x;
    player.y = config.y;
    player.orientation = config.orientation;
    player.level = config.level;
    player.team = config.team;
    player.color = config.color.a > 0 ? config.color : getTeamColor(config.team);
    
    players[config.id] = player;
    
    //std::cout << "👤 Player " << config.id << " (" << config.team << ") added at (" 
              //<< config.x << "," << config.y << ") facing " << config.orientation << std::endl;
}

void zappy::game::world::MapGenerator::setupPlayers(std::map<int, zappy::game::entities::ZappyPlayer>& players, const std::vector<PlayerConfig>& configs)
{
    //std::cout << "👥 Setting up " << configs.size() << " players..." << std::endl;
    
    for (const auto& config : configs) {
        setupPlayer(players, config);
    }
    
    //std::cout << "✅ All players setup complete" << std::endl;
}

void zappy::game::world::MapGenerator::setupResources(std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map, const ResourceConfig& config)
{
    if (!isValidPosition(world_map, config.x, config.y)) {
        std::cerr << "❌ Invalid position (" << config.x << "," << config.y << ") for resources" << std::endl;
        return;
    }
    
    zappy::game::world::ZappyTile& tile = world_map[config.y][config.x];
    tile.food += config.food;
    tile.linemate += config.linemate;
    tile.deraumere += config.deraumere;
    tile.sibur += config.sibur;
    tile.mendiane += config.mendiane;
    tile.phiras += config.phiras;
    tile.thystame += config.thystame;
    
    int total = config.food + config.linemate + config.deraumere + config.sibur + 
                config.mendiane + config.phiras + config.thystame;
    
    if (total > 0) {
        //std::cout << "💎 Added " << total << " resources at (" << config.x << "," << config.y << ")" << std::endl;
    }
}

void zappy::game::world::MapGenerator::setupResources(std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map, const std::vector<ResourceConfig>& configs)
{
    //std::cout << "💎 Setting up resources at " << configs.size() << " locations..." << std::endl;
    
    for (const auto& config : configs) {
        setupResources(world_map, config);
    }
    
    //std::cout << "✅ All resources setup complete" << std::endl;
}

void zappy::game::world::MapGenerator::clearMap(std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map)
{
    world_map.clear();
    //std::cout << "🧹 Map cleared" << std::endl;
}

void zappy::game::world::MapGenerator::clearPlayers(std::map<int, zappy::game::entities::ZappyPlayer>& players)
{
    players.clear();
    //std::cout << "🧹 Players cleared" << std::endl;
}

void zappy::game::world::MapGenerator::clearResources(std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map)
{
    for (auto& row : world_map) {
        for (auto& tile : row) {
            tile.food = 0;
            tile.linemate = 0;
            tile.deraumere = 0;
            tile.sibur = 0;
            tile.mendiane = 0;
            tile.phiras = 0;
            tile.thystame = 0;
        }
    }
    //std::cout << "🧹 Resources cleared" << std::endl;
}

void zappy::game::world::MapGenerator::distributeResourcesByDensity(std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map)
{
    if (world_map.empty()) {
        std::cerr << "❌ Cannot distribute resources on empty map" << std::endl;
        return;
    }
    
    int width = static_cast<int>(world_map[0].size());
    int height = static_cast<int>(world_map.size());
    int total_tiles = width * height;
    
    // Resource densities from Zappy specification
    struct ResourceDensity {
        const char* name;
        float density;
        int total_count;
    };
    
    ResourceDensity densities[] = {
        {"food", 0.5f, static_cast<int>(total_tiles * 0.5f)},
        {"linemate", 0.3f, static_cast<int>(total_tiles * 0.3f)},
        {"deraumere", 0.15f, static_cast<int>(total_tiles * 0.15f)},
        {"sibur", 0.1f, static_cast<int>(total_tiles * 0.1f)},
        {"mendiane", 0.1f, static_cast<int>(total_tiles * 0.1f)},
        {"phiras", 0.08f, static_cast<int>(total_tiles * 0.08f)},
        {"thystame", 0.05f, static_cast<int>(total_tiles * 0.05f)}
    };
    
    ////std::cout << "🌍 Distributing resources by density on " << width << "x" << height << " map..." << std::endl;
    
    // Clear existing resources
    clearResources(world_map);
    
    std::uniform_int_distribution<> x_dist(0, width - 1);
    std::uniform_int_distribution<> y_dist(0, height - 1);
    
    for (int resource_type = 0; resource_type < 7; resource_type++) {
        const auto& density = densities[resource_type];
        
        for (int i = 0; i < density.total_count; i++) {
            int x = x_dist(_gen);
            int y = y_dist(_gen);
            
            switch (resource_type) {
                case 0: world_map[y][x].food++; break;
                case 1: world_map[y][x].linemate++; break;
                case 2: world_map[y][x].deraumere++; break;
                case 3: world_map[y][x].sibur++; break;
                case 4: world_map[y][x].mendiane++; break;
                case 5: world_map[y][x].phiras++; break;
                case 6: world_map[y][x].thystame++; break;
            }
        }
        
        //std::cout << "  " << density.name << ": " << density.total_count << " units" << std::endl;
    }
    
    //std::cout << "✅ Resource distribution complete" << std::endl;
}

bool zappy::game::world::MapGenerator::isValidPosition(const std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map, int x, int y) const
{
    if (world_map.empty()) return false;
    
    int height = static_cast<int>(world_map.size());
    int width = static_cast<int>(world_map[0].size());
    
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool zappy::game::world::MapGenerator::isValidOrientation(int orientation) const
{
    return orientation >= 1 && orientation <= 4;
}

void zappy::game::world::MapGenerator::ensureMapSize(std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map, int width, int height)
{
    world_map.resize(height);
    for (int y = 0; y < height; y++) {
        world_map[y].resize(width);
    }
}

sf::Color zappy::game::world::MapGenerator::getTeamColor(const std::string& team) const
{
    // Simple hash of team name to pick a color
    size_t hash = 0;
    for (char c : team) {
        hash = hash * 31 + c;
    }
    
    return TEAM_COLORS[hash % TEAM_COLORS.size()];
}

// Legacy methods for backward compatibility
void zappy::game::world::MapGenerator::generateTestMap(std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map)
{
    //std::cout << "🗺️ Generating test map (legacy method)..." << std::endl;
    
    MapConfig config;
    // CORRECTION: Utiliser les constantes par défaut au lieu de Camera3D::MAP_WIDTH/HEIGHT
    config.width = DEFAULT_MAP_WIDTH;
    config.height = DEFAULT_MAP_HEIGHT;
    
    setupMap(world_map, config);
    
    // Add some test resources
    std::vector<ResourceConfig> test_resources = {
        {1, 1, 3, 0, 0, 0, 0, 0, 0},    // Food at (1,1)
        {2, 2, 7, 0, 0, 0, 0, 0, 0},    // Food at (2,2)
        {8, 8, 12, 0, 0, 0, 0, 0, 0},   // Food at (8,8)
        {3, 1, 0, 2, 0, 0, 0, 0, 0},    // Linemate at (3,1)
        {4, 4, 0, 6, 0, 0, 0, 0, 0},    // Linemate at (4,4)
        {7, 2, 0, 15, 0, 0, 0, 0, 0},   // Linemate at (7,2)
        {1, 3, 0, 0, 4, 0, 0, 0, 0},    // Deraumere at (1,3)
        {5, 5, 0, 0, 8, 0, 0, 0, 0},    // Deraumere at (5,5)
        {0, 0, 4, 3, 2, 0, 0, 0, 0},    // Mixed at (0,0)
        {9, 9, 0, 0, 5, 0, 0, 8, 3}     // Mixed at (9,9)
    };
    
    setupResources(world_map, test_resources);
    
    //std::cout << "✅ Test map generated!" << std::endl;
}

void zappy::game::world::MapGenerator::addTestPlayers(std::map<int, zappy::game::entities::ZappyPlayer>& players)
{
    //std::cout << "👥 Adding test players (legacy method)..." << std::endl;
    
    std::vector<PlayerConfig> test_players = {
        {1, 2, 2, 1, 1, "Red", sf::Color::Red},
        {2, 7, 7, 3, 1, "Blue", sf::Color::Blue},
        {3, 4, 5, 2, 1, "Green", sf::Color::Green}
    };
    
    setupPlayers(players, test_players);
    
    //std::cout << "✅ Test players added!" << std::endl;
}

void zappy::game::world::MapGenerator::addRandomResources(std::vector<std::vector<zappy::game::world::ZappyTile>>& world_map)
{
    //std::cout << "🎲 Adding random resources (legacy method)..." << std::endl;
    
    if (world_map.empty()) {
        std::cerr << "❌ Cannot add resources to empty map" << std::endl;
        return;
    }
    
    int width = static_cast<int>(world_map[0].size());
    int height = static_cast<int>(world_map.size());
    
    std::uniform_int_distribution<> resource_chance(0, 100);
    std::uniform_int_distribution<> resource_amount(1, 15);
    std::uniform_int_distribution<> x_pos(0, width - 1);
    std::uniform_int_distribution<> y_pos(0, height - 1);
    
    // Add 20 random resource patches
    for (int i = 0; i < 20; i++) {
        ResourceConfig config;
        config.x = x_pos(_gen);
        config.y = y_pos(_gen);
        
        if (resource_chance(_gen) < 30) config.food = resource_amount(_gen);
        if (resource_chance(_gen) < 25) config.linemate = resource_amount(_gen);
        if (resource_chance(_gen) < 20) config.deraumere = resource_amount(_gen);
        if (resource_chance(_gen) < 15) config.sibur = resource_amount(_gen);
        if (resource_chance(_gen) < 10) config.mendiane = resource_amount(_gen);
        if (resource_chance(_gen) < 8) config.phiras = resource_amount(_gen);
        if (resource_chance(_gen) < 5) config.thystame = resource_amount(_gen);
        
        setupResources(world_map, config);
    }
    
    //std::cout << "✅ Random resources added!" << std::endl;
}
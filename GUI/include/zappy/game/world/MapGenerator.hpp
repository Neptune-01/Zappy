/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Map generation and resource management with setup system
*/

#ifndef MAPGENERATOR_HPP
#define MAPGENERATOR_HPP

#include <vector>
#include <map>
#include <random>
#include "zappy/game/world/Tile.hpp"
#include "zappy/game/entities/Player.hpp"


namespace zappy::game::world {
class MapGenerator {
public:
    struct MapConfig {
        int width = 10;
        int height = 10;
        bool clear_existing = true;
    };

    struct PlayerConfig {
        int id;
        int x;
        int y;
        int orientation; // 1=North, 2=East, 3=South, 4=West
        int level;
        std::string team;
        sf::Color color;
    };

    struct ResourceConfig {
        int x;
        int y;
        int food = 0;
        int linemate = 0;
        int deraumere = 0;
        int sibur = 0;
        int mendiane = 0;
        int phiras = 0;
        int thystame = 0;
    };

public:
    MapGenerator();
    ~MapGenerator() = default;

    // Setup methods
    void setupMap(std::vector<std::vector<ZappyTile>>& world_map, const MapConfig& config);
    void setupPlayer(std::map<int, zappy::game::entities::ZappyPlayer>& players, const PlayerConfig& config);
    void setupPlayers(std::map<int, zappy::game::entities::ZappyPlayer>& players, const std::vector<PlayerConfig>& configs);
    void setupResources(std::vector<std::vector<ZappyTile>>& world_map, const ResourceConfig& config);
    void setupResources(std::vector<std::vector<ZappyTile>>& world_map, const std::vector<ResourceConfig>& configs);

    // Utility methods
    void clearMap(std::vector<std::vector<ZappyTile>>& world_map);
    void clearPlayers(std::map<int, zappy::game::entities::ZappyPlayer>& players);
    void clearResources(std::vector<std::vector<ZappyTile>>& world_map);

    // Resource distribution by density (following Zappy protocol)
    void distributeResourcesByDensity(std::vector<std::vector<ZappyTile>>& world_map);

    // Legacy methods for backward compatibility
    void generateTestMap(std::vector<std::vector<ZappyTile>>& world_map);
    void addTestPlayers(std::map<int, zappy::game::entities::ZappyPlayer>& players);
    void addRandomResources(std::vector<std::vector<ZappyTile>>& world_map);

    // Validation methods
    bool isValidPosition(const std::vector<std::vector<ZappyTile>>& world_map, int x, int y) const;
    bool isValidOrientation(int orientation) const;

private:
    void ensureMapSize(std::vector<std::vector<ZappyTile>>& world_map, int width, int height);
    sf::Color getTeamColor(const std::string& team) const;

    std::random_device _rd;
    std::mt19937 _gen;
    
    static const std::vector<sf::Color> TEAM_COLORS;
};
}

#endif /* !MAPGENERATOR_HPP */
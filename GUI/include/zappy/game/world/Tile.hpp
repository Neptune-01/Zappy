/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Tile data structure and methods
*/

#ifndef TILE_HPP
#define TILE_HPP

#include <SFML/Graphics.hpp>

namespace zappy::game::world {
struct ZappyTile {
    int x, y;
    int food = 0;
    int linemate = 0;
    int deraumere = 0;
    int sibur = 0;
    int mendiane = 0;
    int phiras = 0;
    int thystame = 0;
    
    ZappyTile(int px = 0, int py = 0);
    
    sf::Color getColor() const;
    
    float getLightFactor() const;
    
    int getTotalResources() const;

    bool hasRareResources() const;
};
}

#endif /* !TILE_HPP */
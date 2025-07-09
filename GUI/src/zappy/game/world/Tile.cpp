/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** Tile
*/


#include "zappy/game/world/Tile.hpp"
#include <cmath>

zappy::game::world::ZappyTile::ZappyTile(int px, int py) : x(px), y(py) {}

sf::Color zappy::game::world::ZappyTile::getColor() const {
    if (thystame > 0) return sf::Color(128, 0, 128);
    if (phiras > 0) return sf::Color(255, 0, 0);
    if (mendiane > 0) return sf::Color(255, 0, 255);
    if (sibur > 0) return sf::Color(0, 100, 255);
    if (deraumere > 0) return sf::Color(139, 69, 19);
    if (linemate > 0) return sf::Color(192, 192, 192);
    if (food > 0) return sf::Color(255, 255, 0);
    return sf::Color(34, 139, 34);
}

float zappy::game::world::ZappyTile::getLightFactor() const {
    float light = 0.6f;
    
    if (thystame > 0) light += 0.4f;
    else if (phiras > 0) light += 0.3f;
    else if (mendiane > 0) light += 0.25f;
    else if (sibur > 0) light += 0.2f;
    else if (deraumere > 0) light += 0.15f;
    else if (linemate > 0) light += 0.1f;
    else if (food > 0) light += 0.05f;
    
    light += 0.2f * std::sin(static_cast<float>(x + y) * 0.5f) * 0.1f;
    
    return std::min(1.0f, std::max(0.3f, light));
}

int zappy::game::world::ZappyTile::getTotalResources() const {
    return food + linemate + deraumere + sibur + mendiane + phiras + thystame;
}

bool zappy::game::world::ZappyTile::hasRareResources() const {
    return thystame > 0 || phiras > 0 || mendiane > 0;
}
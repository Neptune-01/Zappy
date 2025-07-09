/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** Player
*/

#include "zappy/game/entities/Player.hpp"

zappy::game::entities::ZappyPlayer::ZappyPlayer(int pid) : id(pid), x(0), y(0), orientation(1), level(1), team(""), color(sf::Color::White) {}

sf::Vector2f zappy::game::entities::ZappyPlayer::getDirectionVector() const {
    switch (orientation) {
        case 1: return sf::Vector2f(0, -1); // Nord
        case 2: return sf::Vector2f(1, 0);  // Est
        case 3: return sf::Vector2f(0, 1);  // Sud
        case 4: return sf::Vector2f(-1, 0); // Ouest
        default: return sf::Vector2f(0, -1);
    }
}

std::string zappy::game::entities::ZappyPlayer::getOrientationName() const {
    switch (orientation) {
        case 1: return "North";
        case 2: return "East";
        case 3: return "South";
        case 4: return "West";
        default: return "Unknown";
    }
}
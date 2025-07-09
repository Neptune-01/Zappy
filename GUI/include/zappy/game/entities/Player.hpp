/*
** EPITECH PROJECT, 2024
** Zappy
** File description:
** Player data structure and methods
*/

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include <string>

namespace zappy {
    namespace game {
        namespace entities {
            struct ZappyPlayer {
                int id;
                int x, y;
                int orientation;
                int level;
                std::string team;
                sf::Color color;
                
                // Inventaire du joueur
                int inventory_food = 0;
                int inventory_linemate = 0;
                int inventory_deraumere = 0;
                int inventory_sibur = 0;
                int inventory_mendiane = 0;
                int inventory_phiras = 0;
                int inventory_thystame = 0;
                
                ZappyPlayer(int pid = 0);
                
                sf::Vector2f getDirectionVector() const;
                std::string getOrientationName() const;
                
                // Méthodes pour l'inventaire
                int getTotalInventoryItems() const {
                    return inventory_food + inventory_linemate + inventory_deraumere + 
                           inventory_sibur + inventory_mendiane + inventory_phiras + inventory_thystame;
                }
                
                bool hasInventoryItems() const {
                    return getTotalInventoryItems() > 0;
                }
            };
        }
    }
}

#endif /* !PLAYER_HPP */
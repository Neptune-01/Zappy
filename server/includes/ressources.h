/*
** EPITECH PROJECT, 2025
** ressources.h
** File description:
** all struct and fct that manage ressources in the game
*/

#ifndef RESSOURCES_H_
    #define RESSOURCES_H_

typedef struct inventory_s {
    int food;
    int linemate;
    int deraumere;
    int sibur;
    int mendiane;
    int phiras;
    int thystame;
} inventory_t;

typedef enum ressource_s {
    FOOD,
    LINEMATE,
    DERAUMERE,
    SIBUR,
    MENDIANE,
    PHIRAS,
    THYSTAME,
    ERROR
} ressource_t;

void init_inventory(inventory_t *inv);

#endif /* !RESSOURCES_H_ */

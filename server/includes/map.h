/*
** EPITECH PROJECT, 2025
** map.h
** File description:
** header of the struct and fct that manage the map during the game
*/

#ifndef MAP_H_
    #define MAP_H_
    #include "ressources.h"

typedef struct map_s {
    inventory_t **block;
    int width;
    int height;
} map_t;

#endif /* !MAP_H_ */

/*
** EPITECH PROJECT, 2025
** eject_utils.c
** File description:
** fct utils for eject cmd
*/

#include "commands.h"

coords_t get_ejection_target(coords_t current, direction_t dir)
{
    coords_t target = current;

    if (dir == UP)
        target.y++;
    if (dir == DOWN)
        target.y--;
    if (dir == RIGHT)
        target.x++;
    if (dir == LEFT)
        target.x--;
    return target;
}

int get_ejection_direction(player_t *ejector, player_t *ejected)
{
    if (ejector->coords.x == ejected->coords.x &&
        ejector->coords.y == ejected->coords.y)
        return 0;
    if (ejector->direction == UP)
        return 1;
    if (ejector->direction == RIGHT)
        return 3;
    if (ejector->direction == DOWN)
        return 5;
    if (ejector->direction == LEFT)
        return 7;
    return 0;
}

void move_ejected_player(player_t *player, coords_t target,
    game_info_t *game_info)
{
    player->coords = target;
    verif_limits(player, game_info);
}

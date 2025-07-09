/*
** EPITECH PROJECT, 2025
** look_utils.c
** File description:
** fct utils for look cmd
*/

#include "commands.h"

static void look_coord_up(look_coords_t *vecs)
{
    vecs->fx = 0;
    vecs->fy = 1;
    vecs->rx = 1;
    vecs->ry = 0;
}

static void look_coord_right(look_coords_t *vecs)
{
    vecs->fx = 1;
    vecs->fy = 0;
    vecs->rx = 0;
    vecs->ry = -1;
}

static void look_coord_down(look_coords_t *vecs)
{
    vecs->fx = 0;
    vecs->fy = -1;
    vecs->rx = -1;
    vecs->ry = 0;
}

static void look_coord_left(look_coords_t *vecs)
{
    vecs->fx = -1;
    vecs->fy = 0;
    vecs->rx = 0;
    vecs->ry = 1;
}

void get_direction_vectors(direction_t dir, look_coords_t *coords)
{
    switch (dir) {
        case UP:
            look_coord_up(coords);
            break;
        case RIGHT:
            look_coord_right(coords);
            break;
        case DOWN:
            look_coord_down(coords);
            break;
        case LEFT:
        default:
            look_coord_left(coords);
            break;
    }
}

void get_tile_coords(player_t *player, int line, int pos, coords_t *coords)
{
    look_coords_t vecs;

    get_direction_vectors(player->direction, &vecs);
    vecs.relative_x = vecs.fx * line + vecs.rx * (pos - line);
    vecs.relative_y = vecs.fy * line + vecs.ry * (pos - line);
    coords->x = player->coords.x + vecs.relative_x;
    coords->y = player->coords.y + vecs.relative_y;
}

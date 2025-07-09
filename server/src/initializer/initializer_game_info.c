/*
** EPITECH PROJECT, 2025
** initializer_game_info.c
** File description:
** fct that init the struct game_info to start the game and prepare the player
*/

#include "game_info.h"
#include "player.h"

void init_inventory(inventory_t *inv)
{
    inv->food = 10;
    inv->deraumere = 0;
    inv->linemate = 0;
    inv->mendiane = 0;
    inv->phiras = 0;
    inv->sibur = 0;
    inv->thystame = 0;
}

static void init_inventory_empty(inventory_t *inv)
{
    inv->food = 0;
    inv->deraumere = 0;
    inv->linemate = 0;
    inv->mendiane = 0;
    inv->phiras = 0;
    inv->sibur = 0;
    inv->thystame = 0;
}

static void init_map(map_t *map, int width, int height)
{
    map->width = width;
    map->height = height;
    map->block = NULL;
    map->block = malloc(sizeof(inventory_t *) * width + 1);
    if (!map->block) {
        fprintf(stderr, "Error: Memory allocation failed for map blocks.\n");
        return;
    }
    for (int i = 0; i != width; i++) {
        map->block[i] = malloc(sizeof(inventory_t) * height);
        if (!map->block[i]) {
            fprintf(stderr, "Error: Memory allocation for map row %d.\n", i);
            return;
        }
        for (int j = 0; j != height; j++) {
            init_inventory_empty(&map->block[i][j]);
            map->block[i][j].food = 0;
        }
    }
}

game_info_t *initialize_game_info(args_t *args)
{
    game_info_t *game_info;

    if (args == NULL)
        return NULL;
    game_info = malloc(sizeof(game_info_t));
    if (!game_info) {
        fprintf(stderr, "Error: Memory allocation failed for game_info.\n");
        return NULL;
    }
    game_info->freq = args->frequency;
    game_info->nb_teams = args->team_count;
    game_info->teams = args->team_names;
    game_info->next_id = 0;
    init_map(&game_info->map, args->width, args->height);
    init_inventory(&game_info->global_inv);
    generate_res_map(game_info);
    get_current_time(&game_info->time_gen);
    return game_info;
}

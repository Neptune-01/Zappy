/*
** EPITECH PROJECT, 2025
** generate_ressources.c
** File description:
** fct that generate randomly the ressource in the map
*/

#include "game_info.h"
#include <time.h>

static const double DENSITIES[] = {
    0.5,
    0.3,
    0.15,
    0.1,
    0.1,
    0.08,
    0.05
};

static int calculate_quantity(int width, int height, double density)
{
    int quantity = (int)(width * height * density);

    return ((quantity < 1) ? 1 : quantity);
}

static void add_basic_resources(inventory_t *tile, ressource_t type)
{
    switch (type) {
        case FOOD:
            tile->food++;
            break;
        case LINEMATE:
            tile->linemate++;
            break;
        case DERAUMERE:
            tile->deraumere++;
            break;
        case SIBUR:
            tile->sibur++;
            break;
        default:
            break;
    }
}

static void add_rare_resources(inventory_t *tile, ressource_t type)
{
    switch (type) {
        case MENDIANE:
            tile->mendiane++;
            break;
        case PHIRAS:
            tile->phiras++;
            break;
        case THYSTAME:
            tile->thystame++;
            break;
        default:
            break;
    }
}

static void add_resource_to_tile(inventory_t *tile, ressource_t type)
{
    add_basic_resources(tile, type);
    add_rare_resources(tile, type);
}

static void place_resource_randomly(map_t *map, ressource_t type)
{
    int x = rand() % map->width;
    int y = rand() % map->height;

    add_resource_to_tile(&map->block[x][y], type);
}

static void generate_resource_type_limited(map_t *map, ressource_t type,
    int max_per_spawn)
{
    for (int i = 0; i < max_per_spawn; i++) {
        place_resource_randomly(map, type);
    }
}

void generate_initial_resources(game_info_t *game_info)
{
    int quantity = 0;
    int batch_size;

    if (!game_info || !game_info->map.block) {
        fprintf(stderr, "Error: Invalid game_info or map.\n");
        return;
    }
    srand(time(NULL));
    for (int type = FOOD; type <= THYSTAME; type++) {
        quantity = calculate_quantity(game_info->map.width,
        game_info->map.height, DENSITIES[type]);
        batch_size = (quantity > 20) ? quantity / 4 : quantity;
        generate_resource_type_limited(&game_info->map, (ressource_t)type,
            batch_size);
    }
}

void regenerate_resources(game_info_t *game_info)
{
    int regen_quantity;
    int base_quantity;

    if (!game_info || !game_info->map.block) {
        fprintf(stderr, "Error: Invalid game_info or map.\n");
        return;
    }
    for (int type = FOOD; type <= THYSTAME; type++) {
        base_quantity = calculate_quantity(game_info->map.width,
        game_info->map.height, DENSITIES[type]);
        regen_quantity = base_quantity / 10;
        if (regen_quantity < 1)
            regen_quantity = 1;
        generate_resource_type_limited(&game_info->map, (ressource_t)type,
        regen_quantity);
    }
}

void generate_res_map(game_info_t *game_info)
{
    generate_initial_resources(game_info);
}

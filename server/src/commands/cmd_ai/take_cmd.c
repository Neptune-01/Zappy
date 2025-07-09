/*
** EPITECH PROJECT, 2025
** take_cmd.c
** File description:
** fct that handle take cmd of ai
*/

#include "commands.h"

static ressource_t get_resource_type(char *resource_name)
{
    if (!resource_name)
        return ERROR;
    if (strcmp(resource_name, "food") == 0)
        return FOOD;
    if (strcmp(resource_name, "linemate") == 0)
        return LINEMATE;
    if (strcmp(resource_name, "deraumere") == 0)
        return DERAUMERE;
    if (strcmp(resource_name, "sibur") == 0)
        return SIBUR;
    if (strcmp(resource_name, "mendiane") == 0)
        return MENDIANE;
    if (strcmp(resource_name, "phiras") == 0)
        return PHIRAS;
    if (strcmp(resource_name, "thystame") == 0)
        return THYSTAME;
    return ERROR;
}

static bool check_resource_availability(inventory_t *block, ressource_t type)
{
    switch (type) {
        case FOOD:
            return block->food > 0;
        case LINEMATE:
            return block->linemate > 0;
        case DERAUMERE:
            return block->deraumere > 0;
        case SIBUR:
            return block->sibur > 0;
        case MENDIANE:
            return block->mendiane > 0;
        case PHIRAS:
            return block->phiras > 0;
        case THYSTAME:
            return block->thystame > 0;
        default:
            return false;
    }
}

static int *get_resource_pointer(inventory_t *inv, ressource_t type)
{
    int *resources[] = {
        &inv->food,
        &inv->linemate,
        &inv->deraumere,
        &inv->sibur,
        &inv->mendiane,
        &inv->phiras,
        &inv->thystame
    };

    if (type < 0 || type >= ERROR)
        return NULL;
    return resources[type];
}

static void remove_resource_from_block(inventory_t *block, ressource_t type)
{
    int *res = get_resource_pointer(block, type);

    if (res)
        (*res)--;
}

static void add_resource_to_player(inventory_t *inventory, ressource_t type)
{
    int *res = get_resource_pointer(inventory, type);

    if (res)
        (*res)++;
}

static void print_take(linked_client_t *player, char *args, inventory_t *block)
{
    ressource_t resource_type = get_resource_type(args);
    coords_t pos = player->player->coords;

    printf("type : %d\n", resource_type);
    printf("Player fd=%d is taking resource %s\n",
        player->client->client_fd, args);
    printf("for map block at (%d, %d) nb food : %d\n",
        pos.x, pos.y, block->linemate);
}

static bool validate_take_request(linked_client_t *player, inventory_t *block,
    ressource_t resource_type)
{
    if (resource_type == ERROR) {
        send_responce(player->client->client_fd, "ko\n");
        return false;
    }
    if (!check_resource_availability(block, resource_type)) {
        send_responce(player->client->client_fd, "ko\n");
        return false;
    }
    return true;
}

static void send_take_gui_info(server_t *serv, int player_id,
    ressource_t resource_type)
{
    char response[4098];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected, cannot send take info\n");
        return;
    }
    snprintf(response, sizeof(response), "pgt #%d %d\n", player_id,
        resource_type);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void take(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    coords_t pos = player->player->coords;
    inventory_t *block = &game_info->map.block[pos.y][pos.x];
    ressource_t resource_type;

    if (!args || strlen(args) == 0) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    resource_type = get_resource_type(args);
    print_take(player, args, block);
    if (!validate_take_request(player, block, resource_type))
        return;
    remove_resource_from_block(block, resource_type);
    add_resource_to_player(&player->player->inventory, resource_type);
    send_responce(player->client->client_fd, "ok\n");
    send_take_gui_info(serv, player->player->id, resource_type);
}

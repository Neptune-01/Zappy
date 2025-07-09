/*
** EPITECH PROJECT, 2025
** drop_cmd.c
** File description:
** fct that handle drop cmd
*/

#include "commands.h"
#include "ressources.h"
#include <string.h>

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

static bool check_player_has_resource(inventory_t *inventory, ressource_t type)
{
    switch (type) {
        case FOOD:
            return inventory->food > 0;
        case LINEMATE:
            return inventory->linemate > 0;
        case DERAUMERE:
            return inventory->deraumere > 0;
        case SIBUR:
            return inventory->sibur > 0;
        case MENDIANE:
            return inventory->mendiane > 0;
        case PHIRAS:
            return inventory->phiras > 0;
        case THYSTAME:
            return inventory->thystame > 0;
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

static void remove_resource_from_player(inventory_t *inventory,
    ressource_t type)
{
    int *res = get_resource_pointer(inventory, type);

    if (res)
        (*res)--;
}

static void add_resource_to_block(inventory_t *block, ressource_t type)
{
    int *res = get_resource_pointer(block, type);

    if (res)
        (*res)++;
}

static void send_gui_info_drop(server_t *serv, int id,
    ressource_t resource_type)
{
    char response[64];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected, cannot send drop info\n");
        return;
    }
    snprintf(response, sizeof(response), "pdr #%d %d\n",
        id, resource_type);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void drop(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    coords_t pos = player->player->coords;
    inventory_t *block = &game_info->map.block[pos.y][pos.x];
    ressource_t resource_type;

    printf("[DEBUG] Player fd=%d is dropping resource: %s\n",
        player->client->client_fd, args);
    if (!args || strlen(args) == 0 || get_resource_type(args) == ERROR) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    resource_type = get_resource_type(args);
    if (!check_player_has_resource(&player->player->inventory,
        resource_type)) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    remove_resource_from_player(&player->player->inventory, resource_type);
    add_resource_to_block(block, resource_type);
    send_responce(player->client->client_fd, "ok\n");
    send_gui_info_drop(serv, player->player->id, resource_type);
}

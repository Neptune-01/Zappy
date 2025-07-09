/*
** EPITECH PROJECT, 2025
** inventory_cmd.c
** File description:
** fct that manage the inventory cmd of ai
*/

#include "commands.h"
#include <stdio.h>

void inventory(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    char response[4096];

    (void)args;
    (void)game_info;
    (void)serv;
    printf("[DEBUG] Player fd=%d requested inventory\n",
        player->client->client_fd);
    snprintf(response, sizeof(response), "[food %d, linemate %d, "
    "deraumere %d, sibur %d, mendiane %d, phiras %d, thystame %d]\n",
    player->player->inventory.food,
    player->player->inventory.linemate,
    player->player->inventory.deraumere,
    player->player->inventory.sibur,
    player->player->inventory.mendiane,
    player->player->inventory.phiras,
    player->player->inventory.thystame);
    send_responce(player->client->client_fd, response);
}

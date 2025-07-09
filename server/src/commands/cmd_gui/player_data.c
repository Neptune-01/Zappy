/*
** EPITECH PROJECT, 2025
** player_data.c
** File description:
** player data : inventory, position  level
*/

#include "commands.h"
#include "player.h"

void player_pos(game_info_t *game_info, server_t *serv, char **args)
{
    int player_id;
    player_t *player;
    char response[128];

    (void)game_info;
    if (!args[0] || sscanf(args[0], "#%d", &player_id) != 1 || player_id < 0) {
        send(serv->gui_client->client->client_fd, "ppo\n", 4, 0);
        return;
    }
    player = get_player_by_id(serv->head_team, player_id);
    if (!player) {
        send(serv->gui_client->client->client_fd, "ppo\n", 4, 0);
        return;
    }
    snprintf(response, sizeof(response), "ppo #%d %d %d %d\n",
        player_id, player->coords.x, player->coords.y, player->direction);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void player_level(game_info_t *game_info, server_t *serv, char **args)
{
    int player_id;
    player_t *player;
    char response[64];

    (void)game_info;
    if (!args[0] || sscanf(args[0], "#%d", &player_id) != 1 || player_id < 0) {
        send(serv->gui_client->client->client_fd, "plv\n", 4, 0);
        return;
    }
    player = get_player_by_id(serv->head_team, player_id);
    if (!player) {
        send(serv->gui_client->client->client_fd, "plv\n", 4, 0);
        return;
    }
    snprintf(response, sizeof(response), "plv #%d %d\n", player_id,
        player->level);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

static void send_player_inventory_response(server_t *serv, int player_id,
    player_t *player)
{
    char response[256];

    if (!serv->gui_client || !serv->gui_client->client ||
        serv->gui_client->client->client_fd == -1) {
        return;
    }
    snprintf(response, sizeof(response),
    "pin #%d %d %d %d %d %d %d %d %d %d\n", player_id,
        player->coords.x,
        player->coords.y,
        player->inventory.food,
        player->inventory.linemate,
        player->inventory.deraumere,
        player->inventory.sibur,
        player->inventory.mendiane,
        player->inventory.phiras,
        player->inventory.thystame);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void player_inventory(game_info_t *game_info, server_t *serv, char **args)
{
    int player_id;
    player_t *player;

    if (!serv->gui_client || !serv->gui_client->client ||
        serv->gui_client->client->client_fd == -1) {
        return;
    }
    if (!args || !args[0] || sscanf(args[0], "#%d", &player_id) != 1 ||
        player_id < 0 ||
        player_id >= game_info->map.width * game_info->map.height) {
        send(serv->gui_client->client->client_fd, "pin\n", 4, 0);
        return;
    }
    player = get_player_by_id(serv->head_team, player_id);
    if (!player) {
        send(serv->gui_client->client->client_fd, "pin\n", 4, 0);
        return;
    }
    send_player_inventory_response(serv, player_id, player);
}

void sgt_time(game_info_t *game_info, server_t *serv, char **args)
{
    char response[64];

    (void)game_info;
    (void)args;
    snprintf(response, sizeof(response), "sgt %d\n", 4);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

/*
** EPITECH PROJECT, 2025
** incantation.c
** File description:
** Implementation of the incantation command for level up
*/

#include "commands.h"
#include "game_info.h"
#include "player.h"
#include "socket.h"
#include <stdio.h>

static const int elevation_reqs[7][7] = {
    {1, 1, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 0, 0, 0},
    {2, 2, 0, 1, 0, 2, 0},
    {4, 1, 1, 2, 0, 1, 0},
    {4, 1, 2, 1, 3, 0, 0},
    {6, 1, 2, 3, 0, 1, 0},
    {6, 2, 2, 2, 2, 2, 1}
};

static int count_players_coordo(linked_client_t *client, coords_t coords,
    int level)
{
    int count = 0;

    printf("[DEBUG] Counting players at coords\n");
    while (client != NULL) {
        if (client->player != NULL &&
            client->player->coords.x == coords.x &&
            client->player->coords.y == coords.y &&
            client->player->level == level &&
            client->player->state == ALIVE) {
            count++;
        }
        client = client->next;
    }
    return count;
}

static int count_players_same_level(server_t *serv, coords_t coords, int level)
{
    linked_teams_t *team = serv->head_team;
    int count = 0;

    while (team != NULL) {
        if (strcmp(team->name_team, "waiting_clients") != 0) {
            count += count_players_coordo(team->head_client, coords, level);
        }
        team = team->next;
    }
    return count;
}

static bool check_tile_resources(inventory_t *tile_inv, int level)
{
    const int *reqs = elevation_reqs[level - 1];

    if (tile_inv->linemate < reqs[1])
        return false;
    if (tile_inv->deraumere < reqs[2])
        return false;
    if (tile_inv->sibur < reqs[3])
        return false;
    if (tile_inv->mendiane < reqs[4])
        return false;
    if (tile_inv->phiras < reqs[5])
        return false;
    if (tile_inv->thystame < reqs[6])
        return false;
    return true;
}

static bool check_elevation_requirements(game_info_t *game_info,
    server_t *serv, linked_client_t *player)
{
    int level = player->player->level;
    coords_t coords = player->player->coords;
    int player_count;
    inventory_t *tile_inv;

    if (level >= 8)
        return false;
    player_count = count_players_same_level(serv, coords, level);
    if (player_count < elevation_reqs[level - 1][0])
        return false;
    tile_inv = &game_info->map.block[coords.y][coords.x];
    return check_tile_resources(tile_inv, level);
}

static void consume_elevation_resources(game_info_t *game_info,
    coords_t coords, int level)
{
    const int *reqs = elevation_reqs[level - 1];
    inventory_t *tile_inv = &game_info->map.block[coords.y][coords.x];

    tile_inv->linemate -= reqs[1];
    tile_inv->deraumere -= reqs[2];
    tile_inv->sibur -= reqs[3];
    tile_inv->mendiane -= reqs[4];
    tile_inv->phiras -= reqs[5];
    tile_inv->thystame -= reqs[6];
}

static void send_current_level(int client_fd, int level)
{
    char response[64];

    snprintf(response, sizeof(response), "Current level: %d\n", level);
    send_responce(client_fd, response);
}

static void send_to_gui_player_lvlup(server_t *serv, player_t *player)
{
    char response[64];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected, cannot send level up\n");
        return;
    }
    snprintf(response, sizeof(response), "plv #%d %d\n", player->id,
        player->level);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

static void elevate_team_players_same_level(linked_client_t *client,
    coords_t coords, int level, server_t *serv)
{
    printf("[DEBUG] Elevating players at coords\n");
    while (client != NULL) {
        if (client->player != NULL &&
            client->player->coords.x == coords.x &&
            client->player->coords.y == coords.y &&
            client->player->level == level &&
            client->player->state == ALIVE) {
            client->player->level++;
            client->player->is_incantation = false;
            send_current_level(client->client->client_fd,
                client->player->level);
            send_to_gui_player_lvlup(serv, client->player);
        }
        client = client->next;
    }
}

static void elevate_players_same_level(server_t *serv, coords_t coords,
    int level)
{
    linked_teams_t *team = serv->head_team;

    while (team != NULL) {
        if (strcmp(team->name_team, "waiting_clients") != 0) {
            elevate_team_players_same_level(team->head_client, coords,
            level, serv);
        }
        team = team->next;
    }
}

void incantation(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    char response[64];

    (void)args;
    printf("[DEBUG] Player fd=%d is attempting to incantate\n",
        player->client->client_fd);
    if (!check_elevation_requirements(game_info, serv, player)) {
        send_responce(player->client->client_fd, "ko\n");
        send_lvlup_failed_gui(serv, player->player);
        return;
    }
    consume_elevation_resources(game_info, player->player->coords,
        player->player->level);
    elevate_players_same_level(serv, player->player->coords,
        player->player->level);
    snprintf(response, sizeof(response),
        "Elevation underway\n");
    send_responce(player->client->client_fd, response);
    send_lvlup_finish_gui(serv, player->player);
}

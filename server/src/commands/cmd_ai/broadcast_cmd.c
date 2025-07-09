/*
** EPITECH PROJECT, 2025
** broadcast_cmd.c
** File description:
** fct that handle game logic of broadcast cmd
*/

#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static void send_broadcast_to_client(linked_client_t *client,
    coords_t sender_coords, char *message, game_info_t *game_info)
{
    char response[4096];
    int direction;

    if (client->player == NULL || client->player->state != ALIVE)
        return;
    direction = get_direction_number(client->player->coords,
        sender_coords, client->player->direction, game_info);
    snprintf(response, sizeof(response), "message %d, %s\n",
        direction, message);
    send_responce(client->client->client_fd, response);
}

static void broadcast_to_team_clients(linked_client_t *client,
    coords_t sender_coords, char *message, game_info_t *game_info)
{
    while (client != NULL) {
        send_broadcast_to_client(client, sender_coords, message, game_info);
        client = client->next;
    }
}

static void broadcast_to_all_teams(server_t *serv, coords_t sender_coords,
    char *message, game_info_t *game_info)
{
    linked_teams_t *team = serv->head_team;

    while (team != NULL) {
        if (strcmp(team->name_team, "waiting_clients") != 0) {
            broadcast_to_team_clients(team->head_client, sender_coords,
                message, game_info);
        }
        team = team->next;
    }
}

static void send_pbc_to_gui(server_t *serv, int player_id, char *message)
{
    char response[4096];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected, cannot send PBC\n");
        return;
    }
    snprintf(response, sizeof(response), "pbc #%d %s\n", player_id, message);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void broadcast(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    char *message;

    if (args == NULL || strlen(args) == 0) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    message = args;
    while (*message == ' ')
        message++;
    if (strlen(message) == 0) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    broadcast_to_all_teams(serv, player->player->coords, message, game_info);
    send_responce(player->client->client_fd, "ok\n");
    send_pbc_to_gui(serv, player->player->id, message);
}

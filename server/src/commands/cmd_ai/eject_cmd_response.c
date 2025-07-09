/*
** EPITECH PROJECT, 2025
** eject_cmd_response.c
** File description:
** Implementation of the eject command's response
*/

#include "commands.h"
#include "game_info.h"
#include "player.h"
#include "socket.h"
#include <stdio.h>
#include <sys/socket.h>

bool has_players_to_eject(game_info_t *game_info, linked_client_t *ejector,
    server_t *serv)
{
    linked_teams_t *team = serv->head_team;

    (void)game_info;
    while (team) {
        if (strcmp(team->name_team, "waiting_clients") != 0 &&
            team_has_ejectables(team, ejector))
            return true;
        team = team->next;
    }
    return false;
}

void eject_responce_gui(server_t *serv, linked_client_t *player)
{
    char response[4096];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected,");
        printf(" cannot send eject response\n");
        return;
    }
    snprintf(response, sizeof(response), "pex %d\n", player->player->id);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

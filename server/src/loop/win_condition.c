/*
** EPITECH PROJECT, 2025
** win_condition.c
** File description:
** fct thta handle the win condition of the game and say if a player is lvl 8
*/

#include "commands.h"
#include "game_info.h"
#include "player.h"
#include "socket.h"
#include <string.h>

static bool verif_win_team(linked_teams_t *teams, game_info_t *game_info)
{
    linked_client_t *client = teams->head_client;

    (void)game_info;
    while (client) {
        if (client->player != NULL && client->player->state != UNUSED &&
            client->player->level == 8)
            return true;
        client = client->next;
    }
    return false;
}

bool win_condition(server_t *serv, game_info_t *game_info)
{
    linked_teams_t *teams = serv->head_team;
    bool ret = false;

    while (teams) {
        if (strcmp(teams->name_team, "waiting_clients") != 0 &&
            teams->head_client != NULL) {
            ret = verif_win_team(teams, game_info);
        }
        teams = teams->next;
    }
    return ret;
}

/*
** EPITECH PROJECT, 2025
** nb_unuse_cmd.c
** File description:
** fct that handle the cmd to see the number of slot not use
*/

#include "commands.h"
#include "player.h"
#include "socket.h"
#include <stdio.h>

static int count_remaining_slots(linked_teams_t *team)
{
    linked_client_t *tmp = team->head_client;
    int count = 0;

    while (tmp) {
        if (tmp->player->state == UNUSED || tmp->player->state == EGG)
            count++;
        tmp = tmp->next;
    }
    return count;
}

int get_nb_slot_team(char *team, server_t *serv)
{
    linked_teams_t *tmp = serv->head_team;

    while (tmp != NULL) {
        if (strcmp(tmp->name_team, team) == 0) {
            return count_remaining_slots(tmp);
        }
        tmp = tmp->next;
    }
    return -1;
}

void nb_unuse_slot(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    char responce[2048];
    int nb_slot = get_nb_slot_team(player->player->team_name, serv);

    (void)game_info;
    (void)args;
    snprintf(responce, sizeof(responce), "%d\n", nb_slot);
    send_responce(player->client->client_fd, responce);
}

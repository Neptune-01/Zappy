/*
** EPITECH PROJECT, 2025
** manage_death.c
** File description:
** manage the death of player
*/

#include "game_info.h"
#include "ressources.h"
#include "socket.h"

static void free_player(player_t *player)
{
    if (!player)
        return;
    if (player->command)
        free(player->command);
    if (player->team_name)
        free(player->team_name);
    free(player);
}

static void free_client(linked_client_t *node)
{
    if (!node)
        return;
    if (node->client) {
        close_client_connection(node->client);
        free(node->client);
    }
    free_player(node->player);
    free(node);
}

static void unlink_client(linked_teams_t *team,
    linked_client_t *prev, linked_client_t *target)
{
    if (prev)
        prev->next = target->next;
    else
        team->head_client = target->next;
}

static void remove_client_from_team(linked_teams_t *team,
    linked_client_t *target)
{
    linked_client_t *current = team->head_client;
    linked_client_t *prev = NULL;

    while (current) {
        if (current == target) {
            unlink_client(team, prev, current);
            free_client(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

static void kill_player_from_egg(linked_teams_t *team, linked_client_t *client)
{
    if (client->client && client->client->client_fd > 0) {
        send_responce(client->client->client_fd, "dead\n");
    }
    remove_client_from_team(team, client);
    if (team->nbr_max_player > 0) {
        team->nbr_max_player--;
    }
}

static void how_kill_player(linked_teams_t *team, linked_client_t *client)
{
    if (client->player->was_a_egg) {
        kill_player_from_egg(team, client);
    } else {
        kill_player_from_egg(team, client);
    }
}

static void send_death_to_gui(int id, server_t *serv)
{
    char response[64];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected, cannot send take info\n");
        return;
    }
    snprintf(response, sizeof(response), "pdi #%d\n", id);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

static void verif_death_team(linked_teams_t *team, game_info_t *game_info,
    server_t *serv)
{
    linked_client_t *client = team->head_client;
    linked_client_t *next = NULL;

    (void)game_info;
    while (client != NULL) {
        next = client->next;
        if (client->client != NULL && client->player != NULL &&
            client->player->state == DEAD) {
            send_death_to_gui(client->player->id, serv);
            how_kill_player(team, client);
            client = next;
        } else {
            client = client->next;
        }
    }
}

void manage_death(server_t *serv, game_info_t *game_info)
{
    linked_teams_t *team = serv->head_team;

    while (team != NULL) {
        if (strcmp(team->name_team, "waiting_clients") == 0) {
            team = team->next;
            continue;
        }
        verif_death_team(team, game_info, serv);
        team = team->next;
    }
}

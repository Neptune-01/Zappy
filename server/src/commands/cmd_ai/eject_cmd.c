/*
** EPITECH PROJECT, 2025
** eject.c
** File description:
** Implementation of the eject command for Zappy server
*/

#include "commands.h"
#include "game_info.h"
#include "player.h"
#include "socket.h"
#include <stdio.h>
#include <sys/socket.h>

static void notify_ejected_player(linked_client_t *player, server_t *serv)
{
    char response[4096];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected,");
        printf(" cannot notify ejected player\n");
        return;
    }
    snprintf(response, sizeof(response), "ppo %d %d %d %d\n",
        player->player->id,
        player->player->coords.x,
        player->player->coords.y,
        player->player->direction);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

static void notify_kill_egg_gui(linked_client_t *client, server_t *serv)
{
    char response[4096];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected, cannot notify egg kill\n");
        return;
    }
    snprintf(response, sizeof(response), "edi %d\n", client->player->id);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

static void destroy_team_eggs(linked_teams_t *team, coords_t coords,
    server_t *serv)
{
    linked_client_t *client = team->head_client;

    while (client) {
        if (client->player->state == EGG &&
            client->player->coords.x == coords.x &&
            client->player->coords.y == coords.y) {
            client->player->state = DEAD;
            notify_kill_egg_gui(client, serv);
        }
        client = client->next;
    }
}

static bool should_eject_player(linked_client_t *client,
    linked_client_t *ejector)
{
    if (client == ejector)
        return false;
    if (client->player->state != ALIVE)
        return false;
    return (client->player->coords.x == ejector->player->coords.x &&
            client->player->coords.y == ejector->player->coords.y);
}

static void eject_team_players(linked_teams_t *team,
    linked_client_t *ejector, tdg_t *tdg, server_t *serv)
{
    linked_client_t *client = team->head_client;

    while (client) {
        if (should_eject_player(client, ejector)) {
            move_ejected_player(client->player, tdg->target, tdg->game_info);
            notify_ejected_player(client, serv);
        }
        client = client->next;
    }
}

static void eject_players_from_tile(game_info_t *game_info,
    linked_client_t *ejector, server_t *serv)
{
    linked_teams_t *team = serv->head_team;
    tdg_t tdg;

    tdg.target = get_ejection_target(ejector->player->coords,
        ejector->player->direction);
    tdg.direction = get_ejection_direction(ejector->player, NULL);
    tdg.game_info = game_info;
    while (team) {
        eject_team_players(team, ejector, &tdg, serv);
        team = team->next;
    }
}

static void destroy_eggs_on_tile(game_info_t *game_info,
    server_t *serv, coords_t coords)
{
    linked_teams_t *team = serv->head_team;

    (void)game_info;
    while (team) {
        destroy_team_eggs(team, coords, serv);
        team = team->next;
    }
}

static bool is_egg_on_tile(linked_client_t *client,
    linked_client_t *ejector)
{
    return (client->player->state == EGG &&
            client->player->coords.x == ejector->player->coords.x &&
            client->player->coords.y == ejector->player->coords.y);
}

bool team_has_ejectables(linked_teams_t *team,
    linked_client_t *ejector)
{
    linked_client_t *client = team->head_client;

    while (client) {
        if (should_eject_player(client, ejector) ||
            is_egg_on_tile(client, ejector))
            return true;
        client = client->next;
    }
    return false;
}

void eject(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    (void)args;
    printf("[DEBUG] Player fd=%d is ejecting players from tile\n",
        player->client->client_fd);
    if (!has_players_to_eject(game_info, player, serv)) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    eject_responce_gui(serv, player);
    eject_players_from_tile(game_info, player, serv);
    destroy_eggs_on_tile(game_info, serv, player->player->coords);
    send_responce(player->client->client_fd, "ok\n");
}

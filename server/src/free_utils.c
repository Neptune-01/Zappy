/*
** EPITECH PROJECT, 2025
** free_utils.c
** File description:
** free_utils
*/

#include "game_info.h"
#include "player.h"
#include "socket.h"

static void free_team_elements(char **team_names)
{
    if (!team_names)
        return;
    for (size_t i = 0; team_names[i] != NULL; i++) {
        if (team_names[i]) {
            free(team_names[i]);
            team_names[i] = NULL;
        }
    }
}

static void destroy_team_names(char **team_names)
{
    free_team_elements(team_names);
    free(team_names);
}

static void destroy_player(player_t *player)
{
    if (player) {
        if (player->team_name) {
            free(player->team_name);
            player->team_name = NULL;
        }
        if (player->command) {
            free(player->command);
            player->command = NULL;
        }
    }
}

void destroy_args(args_t *args)
{
    if (args) {
        if (args->team_names) {
            destroy_team_names(args->team_names);
            args->team_names = NULL;
        }
        free(args);
    }
}

void destroy_map(map_t *map)
{
    if (!map || !map->block)
        return;
    for (int i = 0; i < map->height; i++) {
        if (map->block[i]) {
            free(map->block[i]);
            map->block[i] = NULL;
        }
    }
    free(map->block);
    map->block = NULL;
}

void destroy_game_info(game_info_t *game_info)
{
    if (!game_info)
        return;
    if (game_info->teams) {
        destroy_team_names(game_info->teams);
        game_info->teams = NULL;
    }
    destroy_map(&game_info->map);
    free(game_info);
}

static void destroy_clients(linked_client_t *head_client)
{
    linked_client_t *current = head_client;
    linked_client_t *next;

    while (current != NULL) {
        next = current->next;
        if (current->player) {
            destroy_player(current->player);
            free(current->player);
            current->player = NULL;
        }
        if (current->client) {
            free(current->client);
            current->client = NULL;
        }
        free(current);
        current = next;
    }
}

void destroy_teams(linked_teams_t *head_team)
{
    linked_teams_t *current = head_team;
    linked_teams_t *next;

    while (current != NULL) {
        next = current->next;
        if (current->name_team) {
            free(current->name_team);
            current->name_team = NULL;
        }
        destroy_clients(current->head_client);
        current->head_client = NULL;
        free(current);
        current = next;
    }
}

void destroy_gui(gui_t *gui)
{
    if (!gui)
        return;
    if (gui->queue_gui) {
        free(gui->queue_gui);
        gui->queue_gui = NULL;
    }
    if (gui->client) {
        free(gui->client);
        gui->client = NULL;
    }
    free(gui);
}

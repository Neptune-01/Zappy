/*
** EPITECH PROJECT, 2025
** init_player.c
** File description:
** init the player info to launch the server
*/

#include "args.h"
#include "game_info.h"
#include "player.h"
#include "socket.h"
#include <string.h>
#include <time.h>

void create_player(player_t *player, const char *team_name,
    int *id)
{
    player->id = *id;
    (*id)++;
    player->team_name = strdup(team_name);
    if (!player->team_name) {
        fprintf(stderr, "Error: Memory allocation for team name.\n");
        return;
    }
    player->coords.x = 0;
    player->coords.y = 0;
    player->direction = UP;
    init_inventory(&player->inventory);
    player->state = UNUSED;
    player->command = malloc(sizeof(queue_command_ai_t));
    initialize_queue_ai(player->command);
    player->is_waiting_start = false;
    memset(&player->waiting_start, 0, sizeof(struct timespec));
    memset(&player->time_eat, 0, sizeof(struct timespec));
    player->was_a_egg = false;
    player->level = 1;
    player->is_incantation = false;
}

void init_clients_list(int team_count, linked_client_t **head_client,
    char *team_name, int *id)
{
    linked_client_t *tmp_client;

    for (int i = 0; i < team_count; i++) {
        tmp_client = malloc(sizeof(linked_client_t));
        tmp_client->client = NULL;
        tmp_client->player = malloc(sizeof(player_t));
        tmp_client->after_connect = false;
        create_player(tmp_client->player, team_name, id);
        tmp_client->next = *head_client;
        *head_client = tmp_client;
    }
}

static void init_team_waiting_clients(linked_teams_t **head_team)
{
    linked_teams_t *tmp_team;

    tmp_team = malloc(sizeof(linked_teams_t));
    tmp_team->name_team = strdup("waiting_clients");
    tmp_team->nbr_max_player = 0;
    tmp_team->head_client = NULL;
    tmp_team->next = *head_team;
    *head_team = tmp_team;
}

void init_linked_teams(args_t *args, linked_teams_t **head_team, int *id)
{
    linked_teams_t *tmp_team;

    *head_team = NULL;
    for (size_t i = 0; args->team_names[i] != NULL; i++) {
        tmp_team = malloc(sizeof(linked_teams_t));
        tmp_team->name_team = strdup(args->team_names[i]);
        tmp_team->nbr_max_player = args->team_count;
        tmp_team->head_client = NULL;
        init_clients_list(args->team_count, &tmp_team->head_client,
            tmp_team->name_team, id);
        tmp_team->next = *head_team;
        *head_team = tmp_team;
    }
    init_team_waiting_clients(head_team);
}

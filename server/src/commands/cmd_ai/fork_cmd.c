/*
** EPITECH PROJECT, 2025
** fork_command.c
** File description:
** Implementation of the fork command for zappy server
*/

#include "commands.h"
#include "game_info.h"
#include "player.h"
#include "ressources.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

static linked_teams_t *find_player_team(server_t *serv, char *team_name)
{
    linked_teams_t *current_team = serv->head_team;

    while (current_team) {
        if (strcmp(current_team->name_team, team_name) == 0)
            return current_team;
        current_team = current_team->next;
    }
    return NULL;
}

static direction_t random_direction(void)
{
    static direction_t directions[] = {UP, RIGHT, DOWN, LEFT};

    return directions[rand() % 4];
}

static player_t *create_egg_player(linked_client_t *parent, int *id)
{
    player_t *egg = malloc(sizeof(player_t));

    if (!egg)
        return NULL;
    egg->coords = parent->player->coords;
    egg->direction = random_direction();
    egg->team_name = strdup(parent->player->team_name);
    egg->state = EGG;
    egg->was_a_egg = true;
    init_inventory(&egg->inventory);
    egg->command = malloc(sizeof(queue_command_ai_t));
    initialize_queue_ai(egg->command);
    egg->level = 1;
    egg->id = *id;
    egg->is_waiting_start = false;
    (*id)++;
    memset(&egg->waiting_start, 0, sizeof(struct timespec));
    memset(&egg->time_eat, 0, sizeof(struct timespec));
    return egg;
}

static linked_client_t *create_egg_client(player_t *egg_player)
{
    linked_client_t *egg_client = malloc(sizeof(linked_client_t));

    if (!egg_client) {
        free(egg_player);
        return NULL;
    }
    egg_client->player = egg_player;
    egg_client->client = NULL;
    egg_client->next = NULL;
    egg_client->after_connect = false;
    return egg_client;
}

static void add_egg_to_team(linked_teams_t *team, linked_client_t *egg)
{
    linked_client_t *current;

    if (!team->head_client) {
        team->head_client = egg;
        return;
    }
    current = team->head_client;
    while (current->next)
        current = current->next;
    current->next = egg;
}

static void fork_player_gui_responce(server_t *serv, linked_client_t *player,
    linked_client_t *egg_client)
{
    char response[4096];

    (void)egg_client;
    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected, cannot send fork response\n");
        return;
    }
    snprintf(response, sizeof(response), "pfk #%d\n", player->player->id);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void fork_player(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    linked_teams_t *team = find_player_team(serv, player->player->team_name);
    player_t *egg_player = create_egg_player(player, &game_info->next_id);
    linked_client_t *egg_client;

    (void)game_info;
    (void)args;
    if (!team || !egg_player) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    egg_client = create_egg_client(egg_player);
    if (!egg_client) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    add_egg_to_team(team, egg_client);
    team->nbr_max_player++;
    send_responce(player->client->client_fd, "ok\n");
    fork_player_gui_responce(serv, player, egg_client);
}

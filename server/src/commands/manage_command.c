/*
** EPITECH PROJECT, 2025
** manage_command.c
** File description:
** fct that manage the command of the player
*/

#include "game_info.h"
#include "player.h"
#include "commands.h"
#include "socket.h"
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

static void search_cmd_ai(linked_client_t *player, game_info_t *game_info,
    command_ai_t *cmd, server_t *serv)
{
    for (size_t i = 0; tab_exec_ai[i].name != NULL; i++) {
        if (strcmp(cmd->name, tab_exec_ai[i].name) == 0 &&
            tab_exec_ai[i].exec_fct != NULL) {
            tab_exec_ai[i].exec_fct(game_info, player, serv, cmd->args);
            return;
        }
    }
}

static void exec_last_cmd(linked_client_t *player, game_info_t *game_info,
    command_ai_t *cmd, server_t *serv)
{
    struct timespec current_time;
    double wt;

    get_current_time(&current_time);
    wt = timespec_diff(&player->player->waiting_start, &current_time);
    if (wt >= ((double)cmd->time / game_info->freq) &&
        player->player->is_incantation == false) {
        search_cmd_ai(player, game_info, cmd, serv);
        if (cmd->name != NULL)
            free(cmd->name);
        if (cmd->args != NULL)
            free(cmd->args);
        dequeue(player->player->command);
        memset(&player->player->waiting_start, 0, sizeof(struct timespec));
        player->player->is_waiting_start = false;
    }
}

static void notify_start_incantation(linked_client_t *player,
    game_info_t *game_info, server_t *serv)
{
    char response[100];

    (void)game_info;
    if (!serv->gui_client || !serv->gui_client->client) {
        return;
    }
    snprintf(response, sizeof(response), "pic %d %d #%d\n",
        player->player->coords.x,
        player->player->coords.y,
        player->player->level);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void verif_and_lock_player(linked_client_t *head_client,
    int x, int y, int level)
{
    linked_client_t *tmp = head_client;

    while (tmp != NULL) {
        if (tmp->player != NULL && tmp->player->coords.x == x &&
            tmp->player->coords.y == y && tmp->player->level == level &&
            tmp->player->state == ALIVE) {
            tmp->player->is_incantation = true;
        }
        tmp = tmp->next;
    }
}

void froze_player(linked_client_t *player,
    game_info_t *game_info, server_t *serv)
{
    linked_teams_t *team = serv->head_team;

    (void)game_info;
    while (team != NULL) {
        if (strcmp(team->name_team, "waiting_clients") != 0 &&
            team->head_client != NULL) {
            verif_and_lock_player(team->head_client, player->player->coords.x,
                player->player->coords.y, player->player->level);
        }
        team = team->next;
    }
    player->player->is_incantation = false;
}

static void handle_command_execution(linked_client_t *player,
    game_info_t *game_info, server_t *serv, command_ai_t *cmd)
{
    if (!player->player->is_waiting_start &&
        strcmp(cmd->name, "Incantation") != 0) {
        player->player->is_waiting_start = true;
        get_current_time(&player->player->waiting_start);
    } else if (!player->player->is_waiting_start &&
        strcmp(cmd->name, "Incantation") == 0) {
        player->player->is_waiting_start = true;
        get_current_time(&player->player->waiting_start);
        notify_start_incantation(player, game_info, serv);
        froze_player(player, game_info, serv);
    } else {
        exec_last_cmd(player, game_info, cmd, serv);
    }
}

void verif_and_exec_cmd(linked_client_t *player, game_info_t *game_info,
    server_t *serv)
{
    command_ai_t *cmd = peek(player->player->command);

    if (player->player->state != DEAD && player->player->state != EGG) {
        handle_command_execution(player, game_info, serv, cmd);
    }
}

void exec_commands(server_t *serv, game_info_t *game_info,
    linked_client_t *client)
{
    if (!is_empty_ai(client->player->command))
        verif_and_exec_cmd(client, game_info, serv);
    if (client->player->state == ALIVE)
        eat_food(client, game_info);
}

static void process_clients_commands(server_t *serv, game_info_t *game_info,
    linked_teams_t *tmp_team)
{
    linked_client_t *tmp_client = tmp_team->head_client;

    while (tmp_client != NULL) {
        if (tmp_client->client != NULL && tmp_client->player != NULL &&
                tmp_client->player->command != NULL) {
            exec_commands(serv, game_info, tmp_client);
        }
        tmp_client = tmp_client->next;
    }
}

void manage_command(server_t *serv, game_info_t *game_info)
{
    linked_teams_t *tmp_team = serv->head_team;

    verif_regen_ressources(game_info);
    while (tmp_team != NULL) {
        if (strcmp(tmp_team->name_team, "waiting_clients") != 0) {
            process_clients_commands(serv, game_info, tmp_team);
        }
        tmp_team = tmp_team->next;
    }
    manage_cmd_gui(serv, game_info);
}

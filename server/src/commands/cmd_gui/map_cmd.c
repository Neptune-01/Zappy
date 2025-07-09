/*
** EPITECH PROJECT, 2025
** map_cmd.c
** File description:
** fct that handle cmd that manage map info
*/

#include "commands.h"
#include "player.h"

void map_size(game_info_t *game_info, server_t *serv, char **args)
{
    char response[64];

    (void)args;
    snprintf(response, sizeof(response), "msz %d %d\n",
        game_info->map.width, game_info->map.height);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

static void send_block_response(server_t *serv, int x, int y,
    inventory_t *block)
{
    char response[256];

    snprintf(response, sizeof(response), "bct %d %d %d %d %d %d %d %d %d\n",
        x, y,
        block->food,
        block->linemate,
        block->deraumere,
        block->sibur,
        block->mendiane,
        block->phiras,
        block->thystame);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void block_content(game_info_t *game_info, server_t *serv, char **args)
{
    int x;
    int y;

    if (!args[0] || !args[1] ||
        sscanf(args[0], "%d", &x) != 1 || sscanf(args[1], "%d", &y) != 1 ||
        x < 0 || x >= game_info->map.width || y < 0 ||
        y >= game_info->map.height) {
        send(serv->gui_client->client->client_fd, "bct\n", 4, 0);
        return;
    }
    send_block_response(serv, x, y, &game_info->map.block[y][x]);
}

static void format_block_data(game_info_t *game_info, int x, int y,
    char *buffer)
{
    snprintf(buffer, 256,
        "bct %d %d %d %d %d %d %d %d %d\n",
        x, y,
        game_info->map.block[y][x].food,
        game_info->map.block[y][x].linemate,
        game_info->map.block[y][x].deraumere,
        game_info->map.block[y][x].sibur,
        game_info->map.block[y][x].mendiane,
        game_info->map.block[y][x].phiras,
        game_info->map.block[y][x].thystame);
}

static void process_map_row(game_info_t *game_info, server_t *serv, int y,
    char *response)
{
    size_t len;
    size_t remaining;
    int written;
    char block_data[256];

    len = strlen(response);
    for (int x = 0; x < game_info->map.width; x++) {
        remaining = 16826 - len;
        if (remaining < 50) {
            send(serv->gui_client->client->client_fd, response, len, 0);
            response[0] = '\0';
            len = 0;
            remaining = 16826;
        }
        format_block_data(game_info, x, y, block_data);
        written = snprintf(response + len, remaining, "%s", block_data);
        if (written > 0 && written < (int)remaining) {
            len += written;
        }
    }
}

void map_content(game_info_t *game_info, server_t *serv, char **args)
{
    char response[16826];
    size_t len;

    (void)args;
    response[0] = '\0';
    for (int y = 0; y < game_info->map.height; y++) {
        process_map_row(game_info, serv, y, response);
    }
    len = strlen(response);
    if (len > 0) {
        send(serv->gui_client->client->client_fd, response, len, 0);
    }
}

static void add_team_to_response(server_t *serv, linked_teams_t *team,
    char *response, size_t *len)
{
    size_t remaining;

    remaining = 256 - *len;
    if (remaining < strlen(team->name_team) + 10) {
        send(serv->gui_client->client->client_fd, response, *len, 0);
        *len = 0;
    }
    *len += snprintf(response + *len, 256 - *len,
        "tna %s\n", team->name_team);
}

void team_names(game_info_t *game_info, server_t *serv, char **args)
{
    char response[256];
    size_t len = 0;
    linked_teams_t *current_team = serv->head_team;

    (void)game_info;
    (void)args;
    while (current_team) {
        if (strcmp(current_team->name_team, "waiting_clients") == 0) {
            current_team = current_team->next;
            continue;
        }
        add_team_to_response(serv, current_team, response, &len);
        current_team = current_team->next;
    }
    if (len > 0)
        send(serv->gui_client->client->client_fd, response, len, 0);
}

static player_t *search_player_in_team(linked_client_t *client, int player_id)
{
    while (client) {
        if (client->player && client->player->id == player_id) {
            return client->player;
        }
        client = client->next;
    }
    return NULL;
}

player_t *get_player_by_id(linked_teams_t *head_team, int player_id)
{
    linked_teams_t *team = head_team;
    player_t *found_player;

    while (team) {
        if (strcmp(team->name_team, "waiting_clients") == 0) {
            team = team->next;
            continue;
        }
        found_player = search_player_in_team(team->head_client, player_id);
        if (found_player) {
            return found_player;
        }
        team = team->next;
    }
    return NULL;
}

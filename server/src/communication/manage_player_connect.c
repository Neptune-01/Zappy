/*
** EPITECH PROJECT, 2025
** manage_player_connect.c
** File description:
** manage_player_connect
*/

#include "game_info.h"
#include "socket.h"
#include "player.h"
#include "errno.h"
#include <time.h>

bool is_valid_team(server_t *serv, const char *team_name)
{
    linked_teams_t *tmp = serv->head_team;

    while (tmp) {
        if (strcmp(tmp->name_team, "waiting_clients") != 0 &&
            strcmp(tmp->name_team, team_name) == 0 &&
            tmp->nbr_max_player > 0)
            return true;
        tmp = tmp->next;
    }
    return false;
}

linked_teams_t *get_team(server_t *serv, const char *team_name)
{
    linked_teams_t *tmp = serv->head_team;

    while (tmp) {
        if (strcmp(tmp->name_team, team_name) == 0)
            return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

static char *extract_team_name(const char *buf)
{
    char *team = strdup(buf);
    size_t i = 0;

    if (!team)
        return NULL;
    while (team[i]) {
        if (team[i] == '\r' || team[i] == '\n') {
            team[i] = '\0';
            break;
        }
        i++;
    }
    return team;
}

char *get_client_message(linked_client_t *client)
{
    char buf[1024] = {0};
    int ret;

    if (!client || !client->client || client->client->client_fd < 0)
        return NULL;
    ret = recv(client->client->client_fd, buf, sizeof(buf) - 1, MSG_DONTWAIT);
    if (ret <= 0)
        return NULL;
    return extract_team_name(buf);
}

void setup_player_position(player_t *player, game_info_t *game_info)
{
    if (player->state != EGG) {
        player->coords.x = rand() % game_info->map.width;
        player->coords.y = rand() % game_info->map.height;
    }
    player->state = ALIVE;
    get_current_time(&player->time_eat);
    printf("[DEBUG] Player %d positioned at (%d, %d) and set to ALIVE\n",
        player->id, player->coords.x, player->coords.y);
}

void manage_player_connect(server_t *serv, game_info_t *game_info)
{
    if (!serv || !game_info)
        return;
    process_all_waiting_clients(serv, game_info);
}

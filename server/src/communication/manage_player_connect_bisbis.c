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

void assign_client_to_slot(linked_client_t *slot,
    linked_client_t *waiting_client, game_info_t *game_info)
{
    slot->client = waiting_client->client;
    setup_player_position(slot->player, game_info);
    slot->after_connect = true;
    slot->player->is_incantation = false;
    printf("[DEBUG] Client fd=%d assigned to player %d\n",
        slot->client->client_fd, slot->player->id);
}

void new_connection_player_gui(player_t *player, server_t *serv)
{
    char response[64];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected,");
        printf(" cannot send new player response\n");
        return;
    }
    snprintf(response, sizeof(response), "pnw #%d %d %d %d %d %s\n",
        player->id, player->coords.x, player->coords.y,
        player->direction, player->level, player->team_name);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
    printf("[DEBUG] New player GUI response sent for player %d\n", player->id);
}

void send_connection_response(int fd, bool success, int remaining_slots,
    player_t *player)
{
    char response[64];

    if (success && player) {
        snprintf(response, sizeof(response), "%d\n%d %d\n",
            remaining_slots, player->coords.x, player->coords.y);
        send(fd, response, strlen(response), 0);
    } else {
        send(fd, "ko\n", 3, 0);
    }
}

void remove_client_from_list(linked_teams_t *waiting,
    linked_client_t *target, linked_client_t *prev)
{
    if (prev)
        prev->next = target->next;
    else
        waiting->head_client = target->next;
    free(target);
}

void process_all_waiting_clients(server_t *serv, game_info_t *game_info)
{
    linked_teams_t *waiting = get_team(serv, "waiting_clients");
    linked_client_t *curr = waiting->head_client;
    linked_client_t *next;

    while (curr) {
        next = curr->next;
        process_waiting_client(serv, curr, game_info);
        curr = next;
    }
}

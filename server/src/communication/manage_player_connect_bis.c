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

static void remove_from_waiting_list(server_t *serv, linked_client_t *target)
{
    linked_teams_t *waiting = get_team(serv, "waiting_clients");
    linked_client_t *prev = NULL;
    linked_client_t *curr = waiting->head_client;

    while (curr) {
        if (curr == target) {
            remove_client_from_list(waiting, curr, prev);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

static int count_remaining_slot(linked_teams_t *team)
{
    linked_client_t *tmp = team->head_client;
    int count = 0;

    while (tmp) {
        if (tmp->player->state == UNUSED || tmp->player->state == EGG)
            count++;
        tmp = tmp->next;
    }
    printf("count free slot : %d\n", count);
    return count;
}

static bool process_team_assignment(server_t *serv, linked_client_t *client,
    const char *team_name, game_info_t *game_info)
{
    linked_teams_t *team;
    linked_client_t *free_slot;
    int remaining_slots;

    if (!is_valid_team(serv, team_name))
        return false;
    team = get_team(serv, team_name);
    free_slot = find_free_slot(team);
    if (!free_slot)
        return false;
    assign_client_to_slot(free_slot, client, game_info);
    remaining_slots = count_remaining_slot(team);
    send_connection_response(client->client->client_fd, true, remaining_slots,
        free_slot->player);
    new_connection_player_gui(free_slot->player, serv);
    return true;
}

bool is_client_disconnected(linked_client_t *client)
{
    char test_buf[1];
    int ret;

    if (!client || !client->client || client->client->client_fd < 0)
        return true;
    ret = recv(client->client->client_fd, test_buf, 1,
        MSG_PEEK | MSG_DONTWAIT);
    return (ret == 0 || (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK));
}

void handle_disconnected_client(server_t *serv, linked_client_t *client)
{
    printf("[DEBUG] Client fd=%d disconnected before team assignment\n",
        client->client ? client->client->client_fd : -1);
    if (client->client) {
        close(client->client->client_fd);
        free(client->client);
    }
    remove_from_waiting_list(serv, client);
}

bool assign_graphic_client(server_t *serv, linked_client_t *client)
{
    if (serv->gui_client && serv->gui_client->client) {
        send_connection_response(client->client->client_fd, false, 0, NULL);
        return false;
    }
    if (!serv->gui_client) {
        if (!serv->gui_client)
            return false;
    }
    serv->gui_client->client = client->client;
    serv->gui_client->after_connect = true;
    printf("[DEBUG] GRAPHIC client connected with fd=%d\n",
        client->client->client_fd);
    return true;
}

static void handle_client_assignment(server_t *serv, linked_client_t *client,
    char *team_name, game_info_t *game_info)
{
    bool assignment_success = false;

    if (strcmp(team_name, "GRAPHIC") == 0) {
        assignment_success = assign_graphic_client(serv, client);
        if (assignment_success) {
            remove_from_waiting_list(serv, client);
        }
    } else {
        assignment_success = process_team_assignment(serv, client, team_name,
            game_info);
        if (assignment_success) {
            remove_from_waiting_list(serv, client);
        } else {
            send_connection_response(client->client->client_fd, false, 0,
                NULL);
        }
    }
}

void process_waiting_client(server_t *serv, linked_client_t *client,
    game_info_t *game_info)
{
    char *team_name;

    if (is_client_disconnected(client)) {
        handle_disconnected_client(serv, client);
        return;
    }
    team_name = get_client_message(client);
    if (!team_name)
        return;
    handle_client_assignment(serv, client, team_name, game_info);
    free(team_name);
}

linked_client_t *find_free_slot(linked_teams_t *team)
{
    linked_client_t *eggs[100] = {0};
    linked_client_t *tmp = team->head_client;
    int egg_count = 0;

    while (tmp) {
        if (tmp->player->state == EGG && egg_count < 100) {
            eggs[egg_count] = tmp;
            egg_count++;
        }
        tmp = tmp->next;
    }
    if (egg_count > 0)
        return eggs[rand() % egg_count];
    tmp = team->head_client;
    while (tmp) {
        if (tmp->player->state == UNUSED)
            return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

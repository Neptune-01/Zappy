/*
** EPITECH PROJECT, 2025
** server_loop.c
** File description:
** server event loop handling client connections and messages
*/

#include "game_info.h"
#include "socket.h"
#include "server_state.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

static void error_poll(int ready)
{
    if (ready < 0) {
        if (errno == EINTR)
            return;
        perror("poll");
        exit(EXIT_FAILURE);
    }
}

static void verif_new_connection(server_t *serv, int ready)
{
    if (serv->pollfds[0].revents & POLLIN) {
        manage_client_connect(serv);
        ready--;
    }
}

static void signal_handler(int sig)
{
    if (sig == SIGINT) {
        printf("\nServer is closing by signal...\n");
        set_exit_server(true);
    }
}

static void handle_client_messages(linked_client_t *client, server_t *serv)
{
    while (client != NULL) {
        if (client->client != NULL && client->client->pollfd != NULL &&
            client->client->client_fd != -1 &&
            (client->client->pollfd->revents & POLLIN)
            && client->after_connect != true) {
                handle_client_message(serv, client);
            }
        client = client->next;
    }
}

static void change_after_connection(linked_client_t *teams)
{
    linked_client_t *tmp = teams;

    while (tmp != NULL) {
        if (tmp->after_connect == true) {
            tmp->after_connect = false;
            printf("Client fd=%d is now ready for commands\n",
                tmp->client->client_fd);
        }
        tmp = tmp->next;
    }
}

static void verif_new_message_teams(server_t *serv, game_info_t *game_info)
{
    linked_teams_t *tmp = serv->head_team;
    gui_t *gui = serv->gui_client;

    while (tmp != NULL) {
        if (strcmp(tmp->name_team, "waiting_clients") != 0) {
            handle_client_messages(tmp->head_client, serv);
            change_after_connection(tmp->head_client);
        } else {
            manage_player_connect(serv, game_info);
        }
        tmp = tmp->next;
    }
    if (gui != NULL && gui->client != NULL && gui->after_connect == true) {
        gui->after_connect = false;
        return;
    }
    if (gui != NULL && gui->client != NULL && gui->client->pollfd != NULL &&
        gui->client->client_fd != -1 && (gui->client->pollfd->revents & POLLIN)
        && gui->after_connect == false)
            handle_gui_message(serv, gui);
}

static void init_server_pollfds(server_t *serv)
{
    serv->pollfds[0].fd = serv->sock.socket_fd;
    serv->pollfds[0].events = POLLIN;
    serv->pollfds[0].revents = 0;
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        serv->pollfds[i].fd = -1;
        serv->pollfds[i].events = 0;
        serv->pollfds[i].revents = 0;
    }
}

static void reset_pollfds(server_t *serv)
{
    for (int i = 0; i <= MAX_CLIENTS; i++) {
        if (serv->pollfds[i].fd != -1) {
            serv->pollfds[i].revents = 0;
        }
    }
}

void server_event_loop(server_t *serv, game_info_t *game_info)
{
    int ready;

    signal(SIGINT, signal_handler);
    init_server_state();
    init_server_pollfds(serv);
    while (should_exit_server() == false &&
        win_condition(serv, game_info) == false) {
        reset_pollfds(serv);
        ready = poll(serv->pollfds, MAX_CLIENTS + 1, 10);
        error_poll(ready);
        if (ready > 0) {
            verif_new_connection(serv, ready);
            verif_new_message_teams(serv, game_info);
        }
        manage_command(serv, game_info);
        manage_death(serv, game_info);
    }
    cleanup_server_state();
}

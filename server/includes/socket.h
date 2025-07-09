/*
** EPITECH PROJECT, 2025
** socket.h
** File description:
** header file for the network structures of the zappy server
*/

#ifndef SOCKET_H_
    #define SOCKET_H_
    #include "args.h"
    #include "player.h"
    #include <arpa/inet.h>
    #include <sys/poll.h>
    #include <sys/socket.h>
    #include <stdbool.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>

    #define BACKLOG 10
    #define MAX_CLIENTS 1000
    #define CIRCBUF_SIZE 4096

typedef struct circbuf_s {
    char data[CIRCBUF_SIZE];
    size_t head;
    size_t tail;
    size_t len;
} circbuf_t;

typedef struct client_s {
    int client_fd;
    struct sockaddr_in client_adr;
    struct pollfd *pollfd;
    circbuf_t circbuf;
} client_t;

typedef struct linked_client_s {
    client_t *client;
    player_t *player;
    struct linked_client_s *next;
    bool after_connect;
} linked_client_t;

typedef struct linked_teams_s {
    char *name_team;
    int nbr_max_player;
    linked_client_t *head_client;
    struct linked_teams_s *next;
} linked_teams_t;

typedef struct socket_s {
    int port;
    int socket_fd;
    struct sockaddr_in server_adr;
    bool disconect;
} socket_t;

typedef struct gui_s {
    client_t *client;
    queue_command_gui_t *queue_gui;
    bool after_connect;;
} gui_t;

typedef struct server_s {
    socket_t sock;
    linked_teams_t *head_team;
    gui_t *gui_client;
    struct pollfd pollfds[MAX_CLIENTS + 1];
    int nb_clients;
} server_t;

extern bool exit_server;

int init_socket(socket_t *sock);

server_t *init_server(args_t *args, int *id);

void manage_client_connect(server_t *serv);

void handle_client_message(server_t *serv, linked_client_t *client);

void handle_gui_message(server_t *serv, gui_t *gui);

void init_linked_teams(args_t *args, linked_teams_t **head_team, int *id);

void send_responce(int fd, char *str);

void parse_client_command(server_t *serv, linked_client_t *client);

char **my_str_to_word_array(char *str, char *separator);

int receive_into_circbuf(client_t *client);

void remove_client_struct(client_t *client);

bool has_complete_command(circbuf_t *cb);

int read_command_from_buffer(circbuf_t *cb, char *dest, size_t max_len);

void handle_parse_result(int result,
    linked_client_t *client, const char *command);

void parse_line_command_gui(gui_t *client, char *command_line);

int find_cmd_in_table(char *command_name);

void free_word_arr(char **array);

command_gui_t create_command_gui(int cmd_index, char **parsed_command);

char **validate_and_parse_command(gui_t *client, char *command_line);

void process_command_to_queue(gui_t *client, char **parsed_command);

double timespec_diff(struct timespec *start, struct timespec *end);

void get_current_time(struct timespec *ts);

#endif /* !SOCKET_H_ */

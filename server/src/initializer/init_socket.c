/*
** EPITECH PROJECT, 2025
** init_socket.c
** File description:
** init socket
*/

#include "args.h"
#include "socket.h"
#include <fcntl.h>

static int create_socket_fd(socket_t *sock)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        perror("socket");
        return 84;
    }
    sock->socket_fd = fd;
    return 0;
}

static int set_reuse_addr(socket_t *sock)
{
    int opt = 1;

    if (setsockopt(sock->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
        sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sock->socket_fd);
        return 84;
    }
    return 0;
}

static void prepare_server_addr(socket_t *sock)
{
    memset(&sock->server_adr, 0, sizeof(sock->server_adr));
    sock->server_adr.sin_family = AF_INET;
    sock->server_adr.sin_addr.s_addr = INADDR_ANY;
    sock->server_adr.sin_port = htons((uint16_t)sock->port);
}

static int do_bind(socket_t *sock)
{
    if (bind(sock->socket_fd, (const struct sockaddr *)&sock->server_adr,
        sizeof(sock->server_adr)) < 0) {
        perror("bind");
        close(sock->socket_fd);
        return 84;
    }
    return 0;
}

static int do_listen(socket_t *sock)
{
    if (listen(sock->socket_fd, BACKLOG) < 0) {
        perror("listen");
        close(sock->socket_fd);
        return 84;
    }
    return 0;
}

int init_socket(socket_t *sock)
{
    if (create_socket_fd(sock) == 84)
        return 84;
    if (set_reuse_addr(sock) == 84)
        return 84;
    prepare_server_addr(sock);
    if (do_bind(sock) == 84)
        return 84;
    if (do_listen(sock) == 84)
        return 84;
    printf("Server listening on port %d (fd %d)\n", sock->port,
        sock->socket_fd);
    printf("Server address: %s:%d\n", inet_ntoa(sock->server_adr.sin_addr),
        ntohs(sock->server_adr.sin_port));
    return 0;
}

static void init_gui(server_t *serv)
{
    serv->gui_client = malloc(sizeof(gui_t));
    if (!serv->gui_client) {
        perror("malloc");
        return;
    }
    serv->gui_client->client = NULL;
    serv->gui_client->queue_gui = malloc(sizeof(queue_command_gui_t));
    if (!serv->gui_client->queue_gui) {
        perror("malloc");
        free(serv->gui_client);
        return;
    }
    initialize_queue_gui(serv->gui_client->queue_gui);
}

server_t *init_server(args_t *args, int *id)
{
    server_t *serv = malloc(sizeof(server_t));

    if (!serv) {
        perror("malloc");
        return NULL;
    }
    serv->sock.port = args->port;
    serv->sock.disconect = false;
    if (init_socket(&serv->sock) == 84) {
        free(serv);
        return NULL;
    }
    serv->pollfds[0].fd = serv->sock.socket_fd;
    serv->pollfds[0].events = POLLIN;
    serv->head_team = NULL;
    serv->nb_clients = 0;
    init_linked_teams(args, &serv->head_team, id);
    init_gui(serv);
    return serv;
}

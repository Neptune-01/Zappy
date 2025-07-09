/*
** EPITECH PROJECT, 2025
** handle_new_connection.c
** File description:
** fct that handle new connection of client
*/

#include "socket.h"
#include <stdlib.h>
#include <fcntl.h>

static linked_client_t *create_linked_client(client_t *new_client)
{
    linked_client_t *new_linked_client = malloc(sizeof(linked_client_t));

    if (new_linked_client == NULL)
        return NULL;
    new_linked_client->client = new_client;
    new_linked_client->player = NULL;
    new_linked_client->next = NULL;
    return new_linked_client;
}

static int add_client_to_team(linked_teams_t *team, client_t *new_client)
{
    linked_client_t *new_linked_client = create_linked_client(new_client);

    if (new_linked_client == NULL)
        return 84;
    new_linked_client->after_connect = false;
    new_linked_client->next = team->head_client;
    team->head_client = new_linked_client;
    return 0;
}

static int register_client(server_t *serv, client_t *new_client)
{
    linked_teams_t *tmp = serv->head_team;

    while (tmp != NULL) {
        if (strcmp(tmp->name_team, "waiting_clients") == 0) {
            return add_client_to_team(tmp, new_client);
        }
        tmp = tmp->next;
    }
    return 84;
}

static int find_free_pollfd_index(server_t *serv)
{
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (serv->pollfds[i].fd == -1 || serv->pollfds[i].fd == 0) {
            return i;
        }
    }
    return -1;
}

static void init_client(server_t *serv, client_t *client, int fd,
    struct sockaddr_in *addr)
{
    int pollfd_index = find_free_pollfd_index(serv);

    if (pollfd_index == -1) {
        fprintf(stderr, "Error: No free pollfd slot available\n");
        close(fd);
        free(client);
        return;
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);
    client->client_fd = fd;
    client->client_adr = *addr;
    serv->pollfds[pollfd_index].fd = fd;
    serv->pollfds[pollfd_index].events = POLLIN;
    serv->pollfds[pollfd_index].revents = 0;
    client->pollfd = &serv->pollfds[pollfd_index];
    client->circbuf.head = 0;
    client->circbuf.tail = 0;
    client->circbuf.len = 0;
    serv->nb_clients++;
}

static void send_welcome_message(int client_fd)
{
    if (send(client_fd, "WELCOME\n", 8, 0) < 0) {
        perror("send welcome");
    }
}

static int try_accept_client(server_t *serv, struct sockaddr_in *addr,
    socklen_t *len)
{
    int fd = accept(serv->sock.socket_fd, (struct sockaddr *)addr,
        len);

    if (fd < 0)
        perror("accept");
    return fd;
}

void manage_client_connect(server_t *serv)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    client_t *new_client;
    int clifd;

    if (serv->nb_clients >= MAX_CLIENTS) {
        printf("Maximum clients reached, rejecting connection\n");
        return;
    }
    new_client = malloc(sizeof(client_t));
    clifd = try_accept_client(serv, &client_addr, &addr_len);
    if (clifd < 0) {
        free(new_client);
        return;
    }
    send_welcome_message(clifd);
    init_client(serv, new_client, clifd, &client_addr);
    register_client(serv, new_client);
}

/*
** EPITECH PROJECT, 2025
** handle_client_message.c
** File description:
** fct that manage client message
*/

#include "socket.h"
#include "errno.h"

static int handle_recv_error(int ret, client_t *client)
{
    if (ret < 0)
        perror("recv");
    else
        printf("Client fd=%d closed the connection\n", client->client_fd);
    return ret;
}

int receive_into_circbuf(client_t *client)
{
    circbuf_t *cb;
    size_t space;
    size_t end_space;
    size_t to_read;
    int ret;

    cb = &client->circbuf;
    space = CIRCBUF_SIZE - cb->len;
    if (space == 0)
        return 84;
    end_space = CIRCBUF_SIZE - cb->head;
    to_read = space;
    if (to_read > end_space)
        to_read = end_space;
    ret = recv(client->client_fd, &cb->data[cb->head], to_read, 0);
    if (ret <= 0)
        return handle_recv_error(ret, client);
    cb->head = (cb->head + ret) % CIRCBUF_SIZE;
    cb->len += ret;
    return ret;
}

void remove_client_struct(client_t *client)
{
    if (client) {
        if (client->pollfd) {
            client->pollfd->fd = -1;
            client->pollfd->events = 0;
            client->pollfd->revents = 0;
        }
        if (client->client_fd != -1) {
            close(client->client_fd);
            client->client_fd = -1;
        }
        memset(&client->circbuf, 0, sizeof(circbuf_t));
    }
}

static void reset_player(player_t *player)
{
    if (player) {
        player->id = -1;
        free(player->team_name);
        player->team_name = NULL;
        player->coords.x = 0;
        player->coords.y = 0;
        player->direction = UP;
        init_inventory(&player->inventory);
        player->state = UNUSED;
        player->is_waiting_start = false;
    }
}

void remove_linked_client(linked_client_t *head)
{
    remove_client_struct(head->client);
    reset_player(head->player);
    head->client = NULL;
}

static bool is_client_disconnected(client_t *client)
{
    if (!client || !client->pollfd || client->client_fd == -1)
        return true;
    if (client->pollfd->revents & (POLLHUP | POLLERR | POLLNVAL))
        return true;
    return false;
}

void handle_client_message(server_t *serv, linked_client_t *client)
{
    int ret;

    printf("Handling message from client fd=%d\n", client->client->client_fd);
    if (is_client_disconnected(client->client)) {
        printf("Client fd=%d is already disconnected\n",
        client->client->client_fd);
        remove_linked_client(client);
        return;
    }
    ret = receive_into_circbuf(client->client);
    if (ret <= 0) {
        remove_linked_client(client);
        return;
    }
    printf("Received %d bytes from fd=%d\n", ret, client->client->client_fd);
    parse_client_command(serv, client);
}

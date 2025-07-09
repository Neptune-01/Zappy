/*
** EPITECH PROJECT, 2025
** manage_death_utils.c
** File description:
** fct that utils for the fct manage death
*/

#include "commands.h"

void reset_player(player_t *player)
{
    player->coords.x = 0;
    player->coords.y = 0;
    player->direction = UP;
    player->level = 1;
    player->state = UNUSED;
    player->is_waiting_start = false;
    player->was_a_egg = false;
    init_inventory(&player->inventory);
    if (player->command) {
        initialize_queue_ai(player->command);
    }
}

void close_client_connection(client_t *client)
{
    if (!client)
        return;
    if (client->client_fd > 0) {
        close(client->client_fd);
        client->client_fd = -1;
    }
    if (client->pollfd) {
        client->pollfd->fd = -1;
        client->pollfd->events = 0;
        client->pollfd->revents = 0;
    }
}

/*
** EPITECH PROJECT, 2025
** mouvement.c
** File description:
** alll fct that handle the mouvement of a ai in zappy
*/

#include "commands.h"
#include "game_info.h"
#include "player.h"
#include <sys/socket.h>

void move_player(player_t *player, direction_t dir)
{
    if (dir == UP)
        player->coords.y++;
    if (dir == DOWN)
        player->coords.y--;
    if (dir == RIGHT)
        player->coords.x++;
    if (dir == LEFT)
        player->coords.x--;
}

void verif_limits(player_t *player, game_info_t *game_info)
{
    if (player->coords.x < 0)
        player->coords.x = game_info->map.width - 1;
    if (player->coords.x == game_info->map.width)
        player->coords.x = 0;
    if (player->coords.y < 0)
        player->coords.y = game_info->map.height - 1;
    if (player->coords.y == game_info->map.height)
        player->coords.y = 0;
}

static void forward_gui_responce(server_t *serv, linked_client_t *player)
{
    char response[2098];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected,");
        printf(" cannot send forward response\n");
        return;
    }
    snprintf(response, sizeof(response), "ppo #%d %d %d %d\n",
        player->player->id,
        player->player->coords.x,
        player->player->coords.y,
        player->player->direction);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void forward(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    (void)args;
    printf("[DEBUG] Player fd=%d is moving forward\n",
        player->client->client_fd);
    move_player(player->player, player->player->direction);
    verif_limits(player->player, game_info);
    send_responce(player->client->client_fd, "ok\n");
    forward_gui_responce(serv, player);
}

static direction_t change_dir_right(direction_t direction)
{
    if (direction == UP)
        return RIGHT;
    if (direction == DOWN)
        return LEFT;
    if (direction == RIGHT)
        return DOWN;
    if (direction == LEFT)
        return UP;
    return UP;
}

void right(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    (void)game_info;
    (void)serv;
    (void)args;
    printf("[DEBUG] Player fd=%d is turning right\n",
        player->client->client_fd);
    player->player->direction = change_dir_right(player->player->direction);
    send_responce(player->client->client_fd, "ok\n");
    forward_gui_responce(serv, player);
}

static direction_t change_dir_left(direction_t direction)
{
    if (direction == UP)
        return LEFT;
    if (direction == DOWN)
        return RIGHT;
    if (direction == RIGHT)
        return UP;
    if (direction == LEFT)
        return DOWN;
    return UP;
}

void left(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    (void)game_info;
    (void)serv;
    (void)args;
    printf("[DEBUG] Player fd=%d is turning left\n",
        player->client->client_fd);
    player->player->direction = change_dir_left(player->player->direction);
    send_responce(player->client->client_fd, "ok\n");
    forward_gui_responce(serv, player);
}

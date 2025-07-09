/*
** EPITECH PROJECT, 2025
** incantation_lvlup.c
** File description:
** Implementation of the incantation level up
*/

#include "commands.h"
#include "game_info.h"
#include "player.h"
#include "socket.h"
#include <stdio.h>

void send_lvlup_finish_gui(server_t *serv, player_t *player)
{
    char response[64];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected,");
        printf(" cannot send level up finish\n");
        return;
    }
    snprintf(response, sizeof(response), "pie %d %d 1\n", player->coords.x,
        player->coords.y);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

void send_lvlup_failed_gui(server_t *serv, player_t *player)
{
    char response[64];

    if (!serv->gui_client || !serv->gui_client->client) {
        printf("[DEBUG] No GUI client connected,");
        printf(" cannot send level up finish\n");
        return;
    }
    snprintf(response, sizeof(response), "pie %d %d 0\n", player->coords.x,
        player->coords.y);
    send(serv->gui_client->client->client_fd, response, strlen(response), 0);
}

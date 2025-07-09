/*
** EPITECH PROJECT, 2025
** look_command.c
** File description:
** Implementation of the look command for zappy server - SECURED VERSION
*/

#include "commands.h"
#include "game_info.h"
#include "player.h"
#include "ressources.h"
#include "socket.h"
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void apply_world_limits(coords_t *coords, game_info_t *game_info)
{
    if (coords->x < 0)
        coords->x = game_info->map.width - 1;
    if (coords->x >= game_info->map.width)
        coords->x = 0;
    if (coords->y < 0)
        coords->y = game_info->map.height - 1;
    if (coords->y >= game_info->map.height)
        coords->y = 0;
}

static int process_single_tile(game_info_t *game_info,
    server_t *serv, player_t *player, look_utils_bis_t *lsr)
{
    coords_t tile_coords;
    char tile_content[MAX_TILE_CONTENT];
    int line = lsr->line;
    int side = lsr->side;
    char *response = lsr->response;

    get_tile_coords(player, line, side + line, &tile_coords);
    apply_world_limits(&tile_coords, game_info);
    tile_content[0] = '\0';
    if (get_tile_content_secure(game_info, serv, &tile_coords,
        tile_content) != 0)
        return -1;
    if (add_tile_to_response_secure(response, tile_content) != 0)
        return -1;
    return 0;
}

static int process_vision_line(game_info_t *game_info,
    server_t *serv, player_t *player, look_utils_t *lrp)
{
    int line = lrp->line;
    char *response = lrp->response;
    int *pos = lrp->pos;
    look_utils_bis_t lsr = {line, 0, response};

    for (int side = -line; side <= line; side++) {
        if (*pos > 1000)
            return -1;
        lsr.side = side;
        if (process_single_tile(game_info, serv, player, &lsr) != 0)
            return -1;
        (*pos)++;
    }
    return 0;
}

static int build_look_response_secure(game_info_t *game_info,
    server_t *serv, player_t *player, char *response)
{
    int vision_range = player->level;
    int pos = 0;
    look_utils_t lrp = {0, response, &pos};

    strcpy(response, "[");
    for (int line = 0; line <= vision_range; line++) {
        lrp.line = line;
        lrp.response = response;
        lrp.pos = &pos;
        if (process_vision_line(game_info, serv, player, &lrp) != 0)
            return -1;
    }
    if (safe_strcat(response, "]\n", MAX_RESPONSE_SIZE) != 0)
        return -1;
    return 0;
}

void look(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args)
{
    char response[MAX_RESPONSE_SIZE] = {0};

    (void)args;
    if (!game_info || !player || !player->player || !serv) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    if (build_look_response_secure(game_info, serv,
        player->player, response) != 0) {
        send_responce(player->client->client_fd, "ko\n");
        return;
    }
    send_responce(player->client->client_fd, response);
}

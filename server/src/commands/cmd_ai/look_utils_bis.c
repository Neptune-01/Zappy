/*
** EPITECH PROJECT, 2025
** look_utils_bis.c
** File description:
** fct that handle look utils
*/

#include "commands.h"

int safe_strcat(char *dest, const char *src, size_t dest_size)
{
    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);

    if (dest_len + src_len + 1 >= dest_size) {
        printf("[WARNING] Buffer overflow prevented in safe_strcat\n");
        return -1;
    }
    strcat(dest, src);
    return 0;
}

static void counter_players_on_tile(linked_client_t *client,
    coords_t *coords, int *count)
{
    if (client->player && client->player->coords.x == coords->x
        && client->player->coords.y == coords->y
        && client->player->state == ALIVE)
        (*count)++;
}

static int count_players_on_tile(server_t *serv, coords_t *coords)
{
    linked_teams_t *team = serv->head_team;
    linked_client_t *client;
    int count = 0;

    while (team) {
        if (strcmp(team->name_team, "waiting_clients") == 0) {
            team = team->next;
            continue;
        }
        client = team->head_client;
        while (client) {
            counter_players_on_tile(client, coords, &count);
            client = client->next;
        }
        team = team->next;
    }
    return count;
}

static int append_object_to_tile(char *tile_content, const char *object)
{
    if (strlen(tile_content) > 0) {
        if (safe_strcat(tile_content, " ", MAX_TILE_CONTENT) != 0)
            return -1;
    }
    if (safe_strcat(tile_content, object, MAX_TILE_CONTENT) != 0)
        return -1;
    return 0;
}

static int add_objects_str_secure(char *tile_content, inventory_t *inv, int i)
{
    int quantities[] = {inv->food, inv->linemate, inv->deraumere,
        inv->sibur, inv->mendiane, inv->phiras, inv->thystame};
    char *objects[] = {"food", "linemate", "deraumere", "sibur",
        "mendiane", "phiras", "thystame"};

    for (int j = 0; j < quantities[i]; j++) {
        if (append_object_to_tile(tile_content, objects[i]) != 0)
            return -1;
    }
    return 0;
}

static int add_tile_objects_secure(char *tile_content, inventory_t *inv)
{
    for (int i = 0; i < 7; i++) {
        if (add_objects_str_secure(tile_content, inv, i) != 0) {
            return -1;
        }
    }
    return 0;
}

static int append_player_to_tile(char *content)
{
    if (strlen(content) > 0) {
        if (safe_strcat(content, " ", MAX_TILE_CONTENT) != 0)
            return -1;
    }
    if (safe_strcat(content, "player", MAX_TILE_CONTENT) != 0)
        return -1;
    return 0;
}

int get_tile_content_secure(game_info_t *game_info, server_t *serv,
    coords_t *coords, char *content)
{
    int player_count;
    inventory_t *tile_inv;

    if (coords->x < 0 || coords->x >= game_info->map.width ||
        coords->y < 0 || coords->y >= game_info->map.height) {
        content[0] = '\0';
        return -1;
    }
    content[0] = '\0';
    player_count = count_players_on_tile(serv, coords);
    for (int i = 0; i < player_count; i++) {
        if (append_player_to_tile(content) != 0)
            return -1;
    }
    tile_inv = &game_info->map.block[coords->y][coords->x];
    if (add_tile_objects_secure(content, tile_inv) != 0)
        return -1;
    return 0;
}

int add_tile_to_response_secure(char *response, char *tile_content)
{
    if (strlen(response) > 1) {
        if (safe_strcat(response, ",", MAX_RESPONSE_SIZE) != 0)
            return -1;
    }
    if (safe_strcat(response, tile_content, MAX_RESPONSE_SIZE) != 0)
        return -1;
    return 0;
}

int calculate_total_tiles(int vision_range)
{
    int total_tiles = 0;

    for (int line = 0; line <= vision_range; line++) {
        total_tiles += (2 * line + 1);
    }
    return total_tiles;
}

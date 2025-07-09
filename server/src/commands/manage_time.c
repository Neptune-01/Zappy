/*
** EPITECH PROJECT, 2025
** manage_time.c
** File description:
** fct that manage the time of game logic
*/

#include "game_info.h"
#include "player.h"
#include "commands.h"
#include "socket.h"

double timespec_diff(struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) +
        (end->tv_nsec - start->tv_nsec) / 1000000000.0;
}

void get_current_time(struct timespec *ts)
{
    clock_gettime(CLOCK_MONOTONIC, ts);
}

void eat_food(linked_client_t *player, game_info_t *game_info)
{
    struct timespec current_time;
    double wt;

    get_current_time(&current_time);
    wt = timespec_diff(&player->player->time_eat, &current_time);
    if (wt >= (126.0 / game_info->freq)) {
        printf("[DEBUG] Player fd=%d and wt : %f is eating food\n",
            player->client->client_fd, wt);
        player->player->inventory.food--;
        get_current_time(&player->player->time_eat);
        if (player->player->inventory.food < 0) {
            player->player->state = DEAD;
        }
    }
}

void verif_regen_ressources(game_info_t *game_info)
{
    struct timespec current_time;
    double waiting;

    get_current_time(&current_time);
    waiting = timespec_diff(&game_info->time_gen, &current_time);
    if (waiting >= (20.0 / game_info->freq)) {
        printf("[DEBUG] Regenerating resources on the map\n");
        generate_res_map(game_info);
        get_current_time(&game_info->time_gen);
    }
}

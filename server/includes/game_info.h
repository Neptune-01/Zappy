/*
** EPITECH PROJECT, 2025
** game_info.h
** File description:
** struct game info
*/

#ifndef GAME_INFO_H_
    #define GAME_INFO_H_
    #include "args.h"
    #include "map.h"
    #include "socket.h"
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <time.h>

typedef struct game_info_s {
    int freq;                  // Frequency of game ticks
    int next_id;              // Next player ID to assign
    int nb_teams;              // Number of teams in the game
    char **teams;              // Array of team names
    map_t map;                 // Game map structure
    inventory_t global_inv;    // Global inventory for the game
    struct timespec time_gen;         // Time for resource generation
} game_info_t;

void init_players(game_info_t *game_info);

game_info_t *initialize_game_info(args_t *args);

void generate_res_map(game_info_t *game_info);

void manage_command(server_t *serv, game_info_t *game_info);

void manage_cmd_gui(server_t *serv, game_info_t *game_info);

void manage_death(server_t *serv, game_info_t *game_info);

bool win_condition(server_t *serv, game_info_t *game_info);

void close_client_connection(client_t *client);

void reset_player(player_t *player);

void manage_player_connect(server_t *serv, game_info_t *game_info);

linked_teams_t *get_team(server_t *serv, const char *team_name);

bool is_valid_team(server_t *serv, const char *team_name);

char *get_client_message(linked_client_t *client);

void setup_player_position(player_t *player, game_info_t *game_info);

bool is_client_disconnected(linked_client_t *client);

void handle_disconnected_client(server_t *serv, linked_client_t *client);

bool assign_graphic_client(server_t *serv, linked_client_t *client);

void process_waiting_client(server_t *serv, linked_client_t *client,
    game_info_t *game_info);

void assign_client_to_slot(linked_client_t *slot,
    linked_client_t *waiting_client, game_info_t *game_info);

void new_connection_player_gui(player_t *player, server_t *serv);

void send_connection_response(int fd, bool success, int remaining_slots,
    player_t *player);

void remove_client_from_list(linked_teams_t *waiting,
    linked_client_t *target, linked_client_t *prev);

void process_all_waiting_clients(server_t *serv, game_info_t *game_info);

linked_client_t *find_free_slot(linked_teams_t *team);

void verif_regen_ressources(game_info_t *game_info);

void eat_food(linked_client_t *player, game_info_t *game_info);

#endif /* !GAME_INFO_H_ */

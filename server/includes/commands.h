/*
** EPITECH PROJECT, 2025
** commands.h
** File description:
** header of struct and fct that manage the command send by ai and gui
*/

#ifndef COMMANDS_H_
    #define COMMANDS_H_
    #include "player.h"
    #include "game_info.h"
    #include "socket.h"
    #include <stdbool.h>
    #define MAX_RESPONSE_SIZE 16384
    #define MAX_TILE_CONTENT 2048
    #define MAX_SINGLE_OBJECT 64

static const command_ai_t tab_command_ai[] = {
    {"Forward", 7, NULL},
    {"Right", 7, NULL},
    {"Left", 7, NULL},
    {"Look", 7, NULL},
    {"Inventory", 1, NULL},
    {"Broadcast", 7, NULL},
    {"Connect_nbr", 1, NULL},
    {"Fork", 42, NULL},
    {"Eject", 7, NULL},
    {"Take", 7, NULL},
    {"Set", 7, NULL},
    {"Incantation", 300, NULL},
    {NULL, 0, NULL}
};

static const command_gui_t tab_command_gui[] = {
    {"msz", NULL}, // Map size
    {"bct", NULL}, // Block content
    {"mct", NULL}, // Map content
    {"tna", NULL}, // Team names
    {"ppo", NULL}, // Player position
    {"plv", NULL}, // Player level
    {"pin", NULL}, // Player inventory
    {"sgt", NULL}, // Server time
    {NULL, NULL}  // End of commands
};

typedef struct exec_cmd_ai_s {
    char *name;
    void (*exec_fct)(game_info_t *, linked_client_t *, server_t *, char *);
} exec_cmd_ai_t;

typedef struct look_coord_s {
    int x;
    int y;
    int pos;
    int px;
    int py;
} look_coord_t;

void verif_limits(player_t *player, game_info_t *game_info);

void forward(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void right(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void left(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void inventory(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void nb_unuse_slot(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void incantation(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void send_lvlup_finish_gui(server_t *serv, player_t *player);

void send_lvlup_failed_gui(server_t *serv, player_t *player);

void fork_player(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void eject(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

bool has_players_to_eject(game_info_t *game_info, linked_client_t *ejector,
    server_t *serv);

void eject_responce_gui(server_t *serv, linked_client_t *player);

bool team_has_ejectables(linked_teams_t *team, linked_client_t *ejector);

void drop(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void take(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void look(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

void broadcast(game_info_t *game_info, linked_client_t *player,
    server_t *serv, char *args);

static const exec_cmd_ai_t tab_exec_ai[] = {
    {"Forward", forward},
    {"Right", right},
    {"Left", left},
    {"Look", look},
    {"Inventory", inventory},
    {"Broadcast", broadcast},
    {"Connect_nbr", nb_unuse_slot},
    {"Fork", fork_player},
    {"Eject", eject},
    {"Take", take},
    {"Set", drop},
    {"Incantation", incantation},
    {NULL, NULL}
};

typedef struct exec_cmd_gui_s {
    char *name;
    void (*exec_fct)(game_info_t *, server_t *, char **);
} exec_cmd_gui_t;

void map_size(game_info_t *game_info, server_t *serv, char **args);

void block_content(game_info_t *game_info, server_t *serv, char **args);

void map_content(game_info_t *game_info, server_t *serv, char **args);

void team_names(game_info_t *game_info, server_t *serv, char **args);

player_t *get_player_by_id(linked_teams_t *head_team, int player_id);

void player_pos(game_info_t *game_info, server_t *serv, char **args);

void player_level(game_info_t *game_info, server_t *serv, char **args);

void player_inventory(game_info_t *game_info, server_t *serv, char **args);

void sgt_time(game_info_t *game_info, server_t *serv, char **args);

static const exec_cmd_gui_t tab_exec_gui[] = {
    {"msz", map_size}, // Map size
    {"bct", block_content}, // Block content
    {"mct", map_content}, // Map content
    {"tna", team_names}, // Team names
    {"ppo", player_pos}, // Player position
    {"plv", player_level}, // Player level
    {"pin", player_inventory}, // Player inventory
    {"sgt", sgt_time}, // Server time
    {NULL, NULL}  // End of commands
};

typedef struct look_coords_s {
    int fx;
    int fy;
    int rx;
    int ry;
    int relative_x;
    int relative_y;
} look_coords_t;

typedef struct tdg_s {
    coords_t target;
    int direction;
    game_info_t *game_info;
} tdg_t;

void get_tile_coords(player_t *player, int line, int pos,
    coords_t *coords);

coords_t get_ejection_target(coords_t current, direction_t dir);

int get_ejection_direction(player_t *ejector, player_t *ejected);

void move_ejected_player(player_t *player, coords_t target,
    game_info_t *game_info);

void parse_line_command(linked_client_t *client, char *command_line);

int get_direction_number(coords_t from, coords_t to,
    direction_t receiver_direction, game_info_t *game_info);

int calculate_total_tiles(int vision_range);

int add_tile_to_response_secure(char *response, char *tile_content);

int get_tile_content_secure(game_info_t *game_info, server_t *serv,
    coords_t *coords, char *content);

int safe_strcat(char *dest, const char *src, size_t dest_size);

typedef struct look_utils_s {
    int line;
    char *response;
    int *pos;
} look_utils_t;

typedef struct look_utils_bis_s {
    int line;
    int side;
    char *response;
} look_utils_bis_t;

#endif /* !COMMANDS_H_ */

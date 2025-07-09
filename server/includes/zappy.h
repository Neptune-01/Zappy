/*
** EPITECH PROJECT, 2025
** zapy.h
** File description:
** main header file for the zappy server
*/

#ifndef ZAPPY_SERVER_H_
    #define ZAPPY_SERVER_H_

    #include "game_info.h"
    #include "socket.h"

    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>


void server_event_loop(server_t *serv, game_info_t *game_info);

void destroy_args(args_t *args);

void destroy_all(args_t *args, game_info_t *game_info, server_t *server);

void destroy_teams(linked_teams_t *head_team);

void destroy_gui(gui_t *gui);

void destroy_game_info(game_info_t *game_info);

#endif /* !ZAPPY_H_ */

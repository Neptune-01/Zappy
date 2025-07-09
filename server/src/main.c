/*
** EPITECH PROJECT, 2025
** main.c
** File description:
** main function of the server of zappy
*/

#include "args.h"
#include "game_info.h"
#include "zappy.h"
#include "socket.h"

void destroy_server(server_t *server)
{
    if (!server)
        return;
    destroy_teams(server->head_team);
    server->head_team = NULL;
    destroy_gui(server->gui_client);
    server->gui_client = NULL;
    if (server->sock.socket_fd > 0) {
        close(server->sock.socket_fd);
        server->sock.socket_fd = -1;
    }
}

void destroy_all(args_t *args, game_info_t *game_info, server_t *server)
{
    if (server) {
        destroy_server(server);
        free(server);
    }
    if (game_info && args && game_info->teams == args->team_names) {
        args->team_names = NULL;
    }
    if (game_info) {
        destroy_game_info(game_info);
    }
    if (args) {
        destroy_args(args);
    }
}

int main(int argc, char **argv)
{
    args_t *args = parser_arguments(argc, argv);
    game_info_t *game_info = initialize_game_info(args);
    server_t *server;

    if (args == NULL || game_info == NULL) {
        fprintf(stderr,
            "Error: Invalid arguments or game info initialization failed.\n");
        destroy_args(args);
        return 84;
    }
    server = init_server(args, &game_info->next_id);
    if (server == NULL) {
        fprintf(stderr, "Error: Server initialization failed.\n");
        destroy_all(args, game_info, server);
        return 84;
    }
    printf("freq : %d\n", game_info->freq);
    server_event_loop(server, game_info);
    destroy_all(args, game_info, server);
    return 0;
}

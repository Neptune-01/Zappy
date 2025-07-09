/*
** EPITECH PROJECT, 2025
** manage_cmd_gi.c
** File description:
** fct that manage the call of right fct that gui want
*/

#include "game_info.h"
#include "player.h"
#include "commands.h"
#include "socket.h"
#include <stddef.h>
#include <time.h>

static void exec_cmd_gui(server_t *serv, game_info_t *game_info,
    command_gui_t *cmd)
{
    for (size_t i = 0; tab_exec_gui[i].name != NULL; i++) {
        if (strcmp(cmd->name, tab_exec_gui[i].name) == 0 &&
            tab_exec_gui[i].exec_fct != NULL) {
            tab_exec_gui[i].exec_fct(game_info, serv, cmd->args);
        }
    }
}

void manage_cmd_gui(server_t *serv, game_info_t *game_info)
{
    command_gui_t *cmd;

    if (serv->gui_client == NULL || serv->gui_client->client == NULL ||
        serv->gui_client->queue_gui == NULL)
        return;
    while (!is_empty_gui(serv->gui_client->queue_gui)) {
        cmd = peek_gui(serv->gui_client->queue_gui);
        exec_cmd_gui(serv, game_info, cmd);
        dequeue_gui(serv->gui_client->queue_gui);
    }
}

/*
** EPITECH PROJECT, 2025
** handle_gui_message.c
** File description:
** fct that handle message receive from gui and transform to struct cmd_gui_t
*/

#include "player.h"
#include "socket.h"

void handle_parse_result_gui(int result,
    gui_t *client, const char *command)
{
    if (result > 0) {
        printf("[DEBUG] Command from fd=%d: \"%s\"\n",
            client->client->client_fd, command);
    } else if (result == -2) {
        printf("[WARNING] Command too long from fd=%d, discarded\n",
            client->client->client_fd);
    } else {
        printf("[ERROR] Inconsistent state: "
            "has_complete_command true but read failed\n");
    }
}

void parse_gui_command(server_t *serv, gui_t *client)
{
    char command[1024] = {0};
    circbuf_t *cb = &client->client->circbuf;
    int result = 0;

    (void)serv;
    while (has_complete_command(cb)) {
        result = read_command_from_buffer(cb, command, sizeof(command));
        handle_parse_result_gui(result, client, command);
        if (result <= 0)
            break;
        parse_line_command_gui(client, command);
    }
}

static bool is_client_disconnected(client_t *client)
{
    if (!client || !client->pollfd || client->client_fd == -1)
        return true;
    if (client->pollfd->revents & (POLLHUP | POLLERR | POLLNVAL))
        return true;
    return false;
}

void handle_gui_message(server_t *serv, gui_t *gui)
{
    int ret;

    if (is_client_disconnected(gui->client)) {
        printf("GUI is already disconnected\n");
        return;
    }
    ret = receive_into_circbuf(gui->client);
    if (ret <= 0) {
        remove_client_struct(gui->client);
        gui->client = NULL;
        while (!is_empty_gui(gui->queue_gui)) {
            dequeue_gui(gui->queue_gui);
        }
        return;
    }
    parse_gui_command(serv, gui);
}

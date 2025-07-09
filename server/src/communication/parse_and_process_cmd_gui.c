/*
** EPITECH PROJECT, 2025
** parse_line_cmd_gui.c
** File description:
** fct that process and validate a command line received from the GUI
*/

#include "player.h"
#include "socket.h"
#include "commands.h"

char **validate_and_parse_command(gui_t *client, char *command_line)
{
    char **parsed_command = NULL;

    if (client == NULL || command_line == NULL) {
        printf("[ERROR] Invalid parameters in parse_line_command\n");
        return NULL;
    }
    if (client->queue_gui == NULL) {
        printf("[ERROR] Player command queue is NULL\n");
        return NULL;
    }
    parsed_command = my_str_to_word_array(command_line, " \t");
    if (parsed_command == NULL || parsed_command[0] == NULL) {
        printf("[WARNING] Empty or invalid command from fd=%d\n",
            client->client->client_fd);
        if (parsed_command != NULL)
            free_word_arr(parsed_command);
        return NULL;
    }
    return parsed_command;
}

void process_command_to_queue(gui_t *client, char **parsed_command)
{
    int cmd_index = -1;
    command_gui_t new_command;

    cmd_index = find_cmd_in_table(parsed_command[0]);
    if (cmd_index == -1) {
        printf("[WARNING] Unknown command '%s' from fd=%d\n",
            parsed_command[0], client->client->client_fd);
        send_responce(client->client->client_fd, "suc\n");
        return;
    }
    if (is_full_gui(client->queue_gui)) {
        return;
    }
    new_command = create_command_gui(cmd_index, parsed_command);
    enqueue_gui(client->queue_gui, new_command);
    printf("[DEBUG] Command '%s' added to queue for GUI\n", new_command.name);
}

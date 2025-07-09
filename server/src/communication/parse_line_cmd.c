/*
** EPITECH PROJECT, 2025
** parse_line_command.c
** File description:
** function that parse a command line and add it to player queue
*/

#include "socket.h"
#include "commands.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int find_command_in_table(char *command_name)
{
    int i = 0;

    while (tab_command_ai[i].name != NULL) {
        if (strcmp(tab_command_ai[i].name, command_name) == 0) {
            return i;
        }
        i++;
    }
    return -1;
}

static char *duplicate_string(char *src)
{
    char *dest = NULL;
    int len = 0;
    int i = 0;

    if (src == NULL)
        return NULL;
    len = strlen(src);
    dest = malloc((len + 1) * sizeof(char));
    if (dest == NULL)
        return NULL;
    while (i < len) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

static void free_word_array(char **array)
{
    int i = 0;

    if (array == NULL)
        return;
    while (array[i] != NULL) {
        free(array[i]);
        i++;
    }
    free(array);
}

static command_ai_t create_command(int cmd_index, char **parsed_command)
{
    command_ai_t new_command;

    new_command.name = duplicate_string(tab_command_ai[cmd_index].name);
    new_command.time = tab_command_ai[cmd_index].time;
    if (parsed_command[1] != NULL) {
        new_command.args = duplicate_string(parsed_command[1]);
    } else {
        new_command.args = NULL;
    }
    return new_command;
}

static int validate_parse_parameters(linked_client_t *client,
    char *command_line)
{
    if (client == NULL || command_line == NULL || client->player == NULL) {
        printf("[ERROR] Invalid parameters in parse_line_command\n");
        return -1;
    }
    if (client->player->command == NULL) {
        printf("[ERROR] Player command queue is NULL\n");
        return -1;
    }
    return 0;
}

static char **parse_and_validate_command(char *command_line, int client_fd)
{
    char **parsed_command = NULL;

    parsed_command = my_str_to_word_array(command_line, " \t");
    if (parsed_command == NULL || parsed_command[0] == NULL) {
        printf("[WARNING] Empty or invalid command from fd=%d\n", client_fd);
        if (parsed_command != NULL)
            free_word_array(parsed_command);
        return NULL;
    }
    return parsed_command;
}

static int check_command_exists(char **parsed_command, int client_fd)
{
    int cmd_index = find_command_in_table(parsed_command[0]);

    if (cmd_index == -1) {
        printf("[WARNING] Unknown command '%s' from fd=%d\n",
            parsed_command[0], client_fd);
        send_responce(client_fd, "ko\n");
        return -1;
    }
    return cmd_index;
}

static int add_command_to_queue(linked_client_t *client, int cmd_index,
    char **parsed_command)
{
    command_ai_t new_command;

    if (is_full_ai(client->player->command)) {
        printf("[WARNING] Command queue full for player %d\n",
            client->player->id);
        return -1;
    }
    new_command = create_command(cmd_index, parsed_command);
    enqueue(client->player->command, new_command);
    printf("[DEBUG] Command '%s' added to queue for player %d\n",
        new_command.name, client->player->id);
    return 0;
}

void parse_line_command(linked_client_t *client, char *command_line)
{
    char **parsed_command = NULL;
    int cmd_index = -1;

    if (validate_parse_parameters(client, command_line) == -1)
        return;
    parsed_command = parse_and_validate_command(command_line,
        client->client->client_fd);
    if (parsed_command == NULL)
        return;
    cmd_index = check_command_exists(parsed_command,
        client->client->client_fd);
    if (cmd_index == -1) {
        free_word_array(parsed_command);
        return;
    }
    if (add_command_to_queue(client, cmd_index, parsed_command) == -1) {
        free_word_array(parsed_command);
        return;
    }
    free_word_array(parsed_command);
}

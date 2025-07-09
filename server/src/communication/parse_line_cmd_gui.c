/*
** EPITECH PROJECT, 2025
** parse_line_cmd_gui.c
** File description:
** fct that parse a string received from the gui to transform to struct cmd
*/

#include "player.h"
#include "socket.h"
#include "commands.h"

int find_cmd_in_table(char *command_name)
{
    int i = 0;

    while (tab_command_gui[i].name != NULL) {
        if (strcmp(tab_command_gui[i].name, command_name) == 0) {
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

void free_word_arr(char **array)
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

static int count_elements(char **parsed_command)
{
    int count = 0;

    while (parsed_command && parsed_command[count])
        count++;
    return count;
}

static char **allocate_args(int args_count)
{
    return malloc((args_count + 1) * sizeof(char *));
}

static void free_args(char **args, int count)
{
    for (int i = 0; i < count; i++)
        free(args[i]);
    free(args);
}

static char **copy_args(char **src, int count)
{
    char **args = allocate_args(count);

    if (!args)
        return NULL;
    for (int i = 0; i < count; i++) {
        args[i] = duplicate_string(src[i + 1]);
        if (!args[i]) {
            free_args(args, i);
            return NULL;
        }
    }
    args[count] = NULL;
    return args;
}

static char **get_args_gui(char **parsed_command)
{
    int total = 0;

    if (!parsed_command || !parsed_command[0])
        return NULL;
    total = count_elements(parsed_command);
    if (total <= 1)
        return NULL;
    return copy_args(parsed_command, total - 1);
}

command_gui_t create_command_gui(int cmd_index, char **parsed_command)
{
    command_gui_t new_command;

    new_command.name = duplicate_string(tab_command_gui[cmd_index].name);
    printf("Command received from GUI : %s\n", new_command.name);
    if (parsed_command[1] != NULL) {
        new_command.args = get_args_gui(parsed_command);
    } else {
        new_command.args = NULL;
    }
    return new_command;
}

void parse_line_command_gui(gui_t *client, char *command_line)
{
    char **parsed_command = NULL;

    parsed_command = validate_and_parse_command(client, command_line);
    if (parsed_command == NULL)
        return;
    process_command_to_queue(client, parsed_command);
    free_word_arr(parsed_command);
}

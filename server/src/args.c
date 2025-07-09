/*
** EPITECH PROJECT, 2025
** args.c
** File description:
** fct that manage the argument in the launch of the binary server zappy
*/

#include "args.h"
#include <stddef.h>

static args_t *init_arguments(void)
{
    args_t *args = malloc(sizeof(args_t));

    if (!args)
        return NULL;
    args->port = -1;
    args->width = -1;
    args->height = -1;
    args->frequency = -1;
    args->team_names = NULL;
    args->team_count = -1;
    return args;
}

int handle_arg(args_t *args, size_t ac, char **av, size_t *i)
{
    if (av[*i][0] != '-' || av[*i][1] == '\0')
        return -1;
    for (size_t j = 0; tab_arg[j].name != NULL; j++) {
        if (strcmp(av[*i], tab_arg[j].name) == 0) {
            return tab_arg[j].fct(args, ac, av, i);
        }
    }
    return -1;
}

int verif_all_args_are_init(args_t *args)
{
    if (args->port == -1 || args->width == -1 || args->height == -1 ||
        args->frequency == -1 || args->team_names == NULL ||
        args->team_count == -1) {
        return -1;
    }
    return 0;
}

args_t *parser_arguments(int argc, char **av)
{
    args_t *args = init_arguments();
    size_t ac = (size_t)argc;

    if (!args || ac < 12)
        return NULL;
    for (size_t i = 1; i < ac; i++) {
        if (handle_arg(args, ac, av, &i) == -1) {
            free(args);
            return NULL;
        }
    }
    if (verif_all_args_are_init(args) == -1) {
        free(args);
        return NULL;
    }
    return args;
}

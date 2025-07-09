/*
** EPITECH PROJECT, 2025
** width&height_parser.c
** File description:
** parsing for the -x and -y arguments that are the width and height of the map
*/

#include "args.h"
#include <stddef.h>

int width_parser(args_t *args, size_t ac, char **av, size_t *i)
{
    if (*i + 1 >= ac || av[*i + 1][0] == '-' || args->width != -1) {
        return -1;
    }
    args->width = atoi(av[*i + 1]);
    if (args->width <= 9 || args->width > 42) {
        return -1;
    }
    *i += 1;
    return 0;
}

int height_parser(args_t *args, size_t ac, char **av, size_t *i)
{
    if (*i + 1 >= ac || av[*i + 1][0] == '-' || args->height != -1) {
        return -1;
    }
    args->height = atoi(av[*i + 1]);
    if (args->height <= 9 || args->height > 42) {
        return -1;
    }
    *i += 1;
    return 0;
}

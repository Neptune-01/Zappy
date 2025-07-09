/*
** EPITECH PROJECT, 2025
** port_parser.c
** File description:
** parsing for the -p argument that is the port of the server
*/

#include "args.h"
#include <stddef.h>
#include <stdio.h>

int port_parser(args_t *args, size_t ac, char **av, size_t *i)
{
    if (*i + 1 >= ac || av[*i + 1][0] == '-' || args->port != -1) {
        return -1;
    }
    args->port = atoi(av[*i + 1]);
    if (args->port <= 0 || args->port > 65535) {
        return -1;
    }
    *i += 1;
    return 0;
}

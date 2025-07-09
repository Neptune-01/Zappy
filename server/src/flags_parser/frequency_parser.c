/*
** EPITECH PROJECT, 2025
** frequency_parser.c
** File description:
** parsing for the -f argument that is the frequency of the server
*/

#include "args.h"
#include <stddef.h>

int frequency_parser(args_t *args, size_t ac, char **av, size_t *i)
{
    if (*i + 1 >= ac || av[*i + 1][0] == '-' || args->frequency != -1) {
        return -1;
    }
    args->frequency = atoi(av[*i + 1]);
    if (args->frequency <= 0) {
        return -1;
    }
    *i += 1;
    return 0;
}

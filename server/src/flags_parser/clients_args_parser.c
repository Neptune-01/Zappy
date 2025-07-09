/*
** EPITECH PROJECT, 2025
** clients_args_parser.c
** File description:
** parsing for the -c and -n arguments that are the team names and team count
*/

#include "args.h"
#include <stddef.h>

static size_t count_team_names(size_t ac, char **av, size_t start)
{
    size_t count = 0;

    for (size_t j = start; j < ac && av[j][0] != '-'; j++)
        count++;
    return count;
}

static int allocate_and_store_team_names(args_t *args, char **av,
    size_t start, size_t count)
{
    size_t index = 0;

    args->team_names = malloc(sizeof(char *) * (count + 1));
    if (!args->team_names)
        return -1;
    for (size_t j = start; j < start + count; j++) {
        args->team_names[index] = strdup(av[j]);
        index++;
    }
    args->team_names[index] = NULL;
    return 0;
}

int team_names_parser(args_t *args, size_t ac, char **av, size_t *i)
{
    size_t count;

    if (*i + 1 >= ac || av[*i + 1][0] == '-' || args->team_names != NULL)
        return -1;
    count = count_team_names(ac, av, *i + 1);
    if (count == 0)
        return -1;
    if (allocate_and_store_team_names(args, av, *i + 1, count) == -1)
        return -1;
    *i += count;
    return 0;
}

int team_count_parser(args_t *args, size_t ac, char **av, size_t *i)
{
    if (*i + 1 >= ac || av[*i + 1][0] == '-' || args->team_count != -1) {
        return -1;
    }
    args->team_count = atoi(av[*i + 1]);
    if (args->team_count <= 0) {
        return -1;
    }
    *i += 1;
    return 0;
}

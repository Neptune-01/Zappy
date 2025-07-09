/*
** EPITECH PROJECT, 2025
** args.h
** File description:
** header file for the argument parser of the zappy server
*/

#ifndef ARGS_H_
    #define ARGS_H_
    #include <stddef.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>

typedef struct args_s {
    int port;         // Port number for the server
    int width;        // Width of the map
    int height;       // Height of the map
    int frequency;    // Frequency of actions
    char **team_names; // Comma-separated list of team names
    int team_count;   // Number of teams
} args_t;

typedef struct tab_args_fct_s {
    char *name;
    int (*fct)(args_t *, size_t, char **, size_t *);
} tab_args_fct_t;

int port_parser(args_t *args, size_t ac, char **av, size_t *i);
int height_parser(args_t *args, size_t ac, char **av, size_t *i);
int width_parser(args_t *args, size_t ac, char **av, size_t *i);
int team_names_parser(args_t *args, size_t ac, char **av, size_t *i);
int team_count_parser(args_t *args, size_t ac, char **av, size_t *i);
int frequency_parser(args_t *args, size_t ac, char **av, size_t *i);

static const tab_args_fct_t tab_arg[] = {
    {"-p", *port_parser}, // Port
    {"-x", *width_parser}, // Width
    {"-y", *height_parser}, // Height
    {"-f", *frequency_parser}, // Frequency
    {"-n", *team_names_parser}, // Team names
    {"-c", *team_count_parser}, // Team count
    {NULL, NULL}  // End of table
};

args_t *parser_arguments(int ac, char **av);

#endif /* !ARGS_H_ */

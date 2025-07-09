/*
** EPITECH PROJECT, 2025
** server_state.h
** File description:
** Singleton for server state management
*/

#ifndef SERVER_STATE_H_
    #define SERVER_STATE_H_
    #include <stdbool.h>
    #include <stdio.h>
    #include <stdlib.h>

typedef struct server_state_s {
    bool exit_server;
} server_state_t;

server_state_t *get_server_state(void);

void set_exit_server(bool value);

bool should_exit_server(void);

void init_server_state(void);

void cleanup_server_state(void);

#endif /* !SERVER_STATE_H_ */

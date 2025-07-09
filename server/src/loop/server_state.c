/*
** EPITECH PROJECT, 2025
** server_state.c
** File description:
** Singleton implementation for server state management
*/

#include "server_state.h"

static server_state_t **get_instance_ptr(void)
{
    static server_state_t *instance = NULL;

    return &instance;
}

server_state_t *get_server_state(void)
{
    server_state_t **instance_ptr = get_instance_ptr();

    if (*instance_ptr == NULL) {
        *instance_ptr = malloc(sizeof(server_state_t));
        if (*instance_ptr == NULL) {
            return NULL;
        }
        (*instance_ptr)->exit_server = false;
    }
    return *instance_ptr;
}

void init_server_state(void)
{
    server_state_t *state = get_server_state();

    if (state != NULL) {
        state->exit_server = false;
    }
}

void set_exit_server(bool value)
{
    server_state_t *state = get_server_state();

    if (state != NULL) {
        state->exit_server = value;
    }
}

bool should_exit_server(void)
{
    server_state_t *state = get_server_state();

    if (state != NULL) {
        return state->exit_server;
    }
    return false;
}

void cleanup_server_state(void)
{
    server_state_t **instance_ptr = get_instance_ptr();

    if (*instance_ptr != NULL) {
        free(*instance_ptr);
        *instance_ptr = NULL;
    }
}

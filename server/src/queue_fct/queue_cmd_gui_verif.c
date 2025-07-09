/*
** EPITECH PROJECT, 2025
** queue_cmd_gui.c
** File description:
** fct that handle gui command
*/

#include "commands.h"

bool is_empty_gui(queue_command_gui_t *q)
{
    return q->front == q->rear;
}

bool is_full_gui(queue_command_gui_t *q)
{
    return (q->rear + 1) % 10 == q->front;
}

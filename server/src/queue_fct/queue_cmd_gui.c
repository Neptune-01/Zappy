/*
** EPITECH PROJECT, 2025
** queue_cmd_gui.c
** File description:
** fct that handle gui command
*/

#include "commands.h"

void initialize_queue_gui(queue_command_gui_t *q)
{
    q->front = 0;
    q->rear = 0;
}

void enqueue_gui(queue_command_gui_t *q, command_gui_t value)
{
    if (is_full_gui(q)) {
        return;
    }
    q->command[q->rear] = value;
    q->rear = (q->rear + 1) % 10;
}

void dequeue_gui(queue_command_gui_t *q)
{
    if (is_empty_gui(q)) {
        return;
    }
    q->front = (q->front + 1) % 10;
}

command_gui_t *peek_gui(queue_command_gui_t *q)
{
    if (is_empty_gui(q)) {
        return NULL;
    }
    return &q->command[q->front];
}

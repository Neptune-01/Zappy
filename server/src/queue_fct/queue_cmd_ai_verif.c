/*
** EPITECH PROJECT, 2025
** queue_cmd_ai_verif.c
** File description:
** verif queue is full or empty
*/

#include "commands.h"

bool is_empty_ai(queue_command_ai_t *q)
{
    return (q->front == q->rear);
}

bool is_full_ai(queue_command_ai_t *q)
{
    return ((q->rear + 1) % 10 == q->front);
}

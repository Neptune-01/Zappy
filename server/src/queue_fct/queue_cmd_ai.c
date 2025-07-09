/*
** EPITECH PROJECT, 2025
** queue_cmd_ai.c
** File description:
** fct that manage the queue of command send by ai
*/

#include "commands.h"

void initialize_queue_ai(queue_command_ai_t *q)
{
    q->front = 0;
    q->rear = 0;
}

void enqueue(queue_command_ai_t *q, command_ai_t value)
{
    if (is_full_ai(q)) {
        printf("Queue is full\n");
        return;
    }
    q->command[q->rear] = value;
    q->rear = (q->rear + 1) % 10;
}

void dequeue(queue_command_ai_t *q)
{
    if (is_empty_ai(q)) {
        printf("Queue is empty\n");
        return;
    }
    q->front = (q->front + 1) % 10;
}

command_ai_t *peek(queue_command_ai_t *q)
{
    if (is_empty_ai(q)) {
        printf("Queue is empty\n");
        return NULL;
    }
    return &q->command[q->front];
}

/*
** EPITECH PROJECT, 2025
** send_responce.c
** File description:
** fct that handle send fct
*/

#include "socket.h"

void send_responce(int fd, char *str)
{
    send(fd, str, strlen(str), 0);
}

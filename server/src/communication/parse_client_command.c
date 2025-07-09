/*
** EPITECH PROJECT, 2025
** parse_client_command.c
** File description:
** functions that parse commands
*/

#include "socket.h"
#include "commands.h"
#include <string.h>
#include <stdio.h>

bool has_complete_command(circbuf_t *cb)
{
    size_t i = 0;
    char c = 0;

    while (i < cb->len) {
        c = cb->data[(cb->tail + i) % CIRCBUF_SIZE];
        if (c == '\n')
            return true;
        i++;
    }
    return false;
}

static void discard_long_command(circbuf_t *cb, size_t newline_pos)
{
    size_t i = 0;

    while (i <= newline_pos) {
        cb->tail = (cb->tail + 1) % CIRCBUF_SIZE;
        cb->len--;
        i++;
    }
}

static size_t get_command_len(circbuf_t *cb, size_t *newline_pos)
{
    size_t i = 0;

    while (i < cb->len) {
        if (cb->data[(cb->tail + i) % CIRCBUF_SIZE] == '\n') {
            *newline_pos = i;
            return i;
        }
        i++;
    }
    return 0;
}

static void consume_crlf(circbuf_t *cb)
{
    if (cb->len > 0 && cb->data[cb->tail] == '\r') {
        cb->tail = (cb->tail + 1) % CIRCBUF_SIZE;
        cb->len--;
    }
    if (cb->len > 0 && cb->data[cb->tail] == '\n') {
        cb->tail = (cb->tail + 1) % CIRCBUF_SIZE;
        cb->len--;
    }
}

static int extract_command_from_buffer(circbuf_t *cb, char *dest,
    size_t cmd_len)
{
    size_t i = 0;

    while (i < cmd_len) {
        dest[i] = cb->data[cb->tail];
        i++;
        cb->tail = (cb->tail + 1) % CIRCBUF_SIZE;
        cb->len--;
    }
    dest[i] = '\0';
    consume_crlf(cb);
    return i;
}

int read_command_from_buffer(circbuf_t *cb, char *dest, size_t max_len)
{
    size_t newline_pos = 0;
    size_t cmd_len = 0;

    if (cb->len == 0)
        return -1;
    cmd_len = get_command_len(cb, &newline_pos);
    if (cmd_len == 0)
        return -1;
    if (cmd_len > 0 &&
        cb->data[(cb->tail + cmd_len - 1) % CIRCBUF_SIZE] == '\r')
        cmd_len--;
    if (cmd_len >= max_len) {
        discard_long_command(cb, newline_pos);
        return -2;
    }
    return extract_command_from_buffer(cb, dest, cmd_len);
}

void handle_parse_result(int result,
    linked_client_t *client, const char *command)
{
    if (result > 0) {
        printf("[DEBUG] Command from fd=%d: \"%s\"\n",
            client->client->client_fd, command);
    } else if (result == -2) {
        printf("[WARNING] Command too long from fd=%d, discarded\n",
            client->client->client_fd);
    } else {
        printf("[ERROR] Inconsistent state: "
            "has_complete_command true but read failed\n");
    }
}

void parse_client_command(server_t *serv, linked_client_t *client)
{
    char command[1024] = {0};
    circbuf_t *cb = &client->client->circbuf;
    int result = 0;

    (void)serv;
    while (has_complete_command(cb)) {
        result = read_command_from_buffer(cb, command, sizeof(command));
        handle_parse_result(result, client, command);
        if (result <= 0)
            break;
        parse_line_command(client, command);
    }
}

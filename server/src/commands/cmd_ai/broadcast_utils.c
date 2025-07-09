/*
** EPITECH PROJECT, 2025
** broadcast_utils.c
** File description:
** fct that handle calcul
*/

#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static double normalize_angle(double angle)
{
    while (angle > M_PI)
        angle -= 2 * M_PI;
    while (angle < - M_PI)
        angle += 2 * M_PI;
    return angle;
}

static double direction_to_angle(direction_t dir)
{
    switch (dir) {
        case UP:
            return M_PI / 2;
        case RIGHT:
            return 0;
        case DOWN:
            return - M_PI / 2;
        case LEFT:
            return M_PI;
        default:
            return 0;
    }
}

static int calculate_dx_wrapped(int from_x, int to_x, int width)
{
    int dx = to_x - from_x;

    if (abs(dx) > width / 2)
        dx = (dx > 0) ? dx - width : dx + width;
    return dx;
}

static int calculate_dy_wrapped(int from_y, int to_y, int height)
{
    int dy = to_y - from_y;

    if (abs(dy) > height / 2)
        dy = (dy > 0) ? dy - height : dy + height;
    return dy;
}

static double calculate_angle_to_target(int dx, int dy)
{
    return atan2(-dy, dx);
}

static int angle_direction_half(double angle)
{
    if (angle >= - M_PI / 8 && angle < M_PI / 8)
        return 1;
    if (angle >= M_PI / 8 && angle < 3 * M_PI / 8)
        return 2;
    if (angle >= 3 * M_PI / 8 && angle < 5 * M_PI / 8)
        return 3;
    if (angle >= 5 * M_PI / 8 && angle < 7 * M_PI / 8)
        return 4;
    return 0;
}

static int angle_direction_halfbis(double angle)
{
    if (angle >= 7 * M_PI / 8 || angle < -7 * M_PI / 8)
        return 5;
    if (angle >= -7 * M_PI / 8 && angle < -5 * M_PI / 8)
        return 6;
    if (angle >= -5 * M_PI / 8 && angle < -3 * M_PI / 8)
        return 7;
    if (angle >= -3 * M_PI / 8 && angle < - M_PI / 8)
        return 8;
    return 1;
}

static int angle_to_direction_number(double relative_angle)
{
    int result = angle_direction_half(relative_angle);

    if (result != 0)
        return result;
    return angle_direction_halfbis(relative_angle);
}

int get_direction_number(coords_t from, coords_t to,
    direction_t receiver_direction, game_info_t *game_info)
{
    int width = game_info->map.width;
    int height = game_info->map.height;
    int dx;
    int dy;
    double angle;
    double relative_angle;
    double receiver_angle;

    if (from.x == to.x && from.y == to.y)
        return 0;
    dx = calculate_dx_wrapped(from.x, to.x, width);
    dy = calculate_dy_wrapped(from.y, to.y, height);
    angle = calculate_angle_to_target(dx, dy);
    receiver_angle = direction_to_angle(receiver_direction);
    relative_angle = normalize_angle(angle - receiver_angle);
    return angle_to_direction_number(relative_angle);
}

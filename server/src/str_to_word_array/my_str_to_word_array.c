/*
** EPITECH PROJECT, 2023
** my_str_to_word_array
** File description:
** SettingUp
*/

#include <stdlib.h>
#include <string.h>
#include "socket.h"

int verif_separator(char c, char *separator)
{
    int i = 0;

    if (c == '\0') {
        return 1;
    }
    while (separator[i] != '\0') {
        if (separator[i] == c) {
            return 1;
        }
        i++;
    }
    return 0;
}

int nbr_mot(char *str, char *separator)
{
    int count = 0;
    int i = 0;

    while (str[i] != '\0') {
        if (verif_separator(str[i], separator) == 0 &&
        verif_separator(str[i + 1], separator) == 1) {
            count++;
        }
        i++;
    }
    return count;
}

int verif_end_file(char *str, char *separator, int index)
{
    while (str[index] != '\0') {
        if (verif_separator(str[index], separator) == 0) {
            return 0;
        }
        index++;
    }
    return 1;
}

char **my_str_to_word_array(char *str, char *separator)
{
    char **arr = malloc((nbr_mot(str, separator) + 1) * sizeof(char *));
    int i = 0;
    int y = 0;
    int k = 0;

    while (verif_end_file(str, separator, k) == 0) {
        arr[i] = malloc((strlen(str) + 1) * sizeof(char));
        while (verif_separator(str[k], separator) == 1)
            k++;
        y = 0;
        while (verif_separator(str[k], separator) == 0) {
            arr[i][y] = str[k];
            k++;
            y++;
        }
        arr[i][y] = '\0';
        i++;
    }
    arr[i] = NULL;
    return (arr);
}

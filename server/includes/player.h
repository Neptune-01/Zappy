/*
** EPITECH PROJECT, 2025
** player.h
** File description:
** header of the struct and fct that manage player info
*/

#ifndef PLAYER_H_
    #define PLAYER_H_
    #include "ressources.h"
    #include <stdbool.h>
    #include <time.h>

typedef struct coords_s {
    int x;
    int y;
} coords_t;

typedef struct command_ai_s {
    char *name;                // Command name
    int time;                // Time required to execute the command
    char *args;
} command_ai_t;

typedef struct queue_command_ai_s {
    command_ai_t command[10]; // Array of commands
    int front;                // Index of the front command
    int rear;                 // Index of the rear command
} queue_command_ai_t;

typedef enum direction_s {
    UP = 1,
    RIGHT = 2,
    DOWN = 3,
    LEFT = 4
} direction_t;

typedef enum state_s {
    UNUSED,
    ALIVE,
    DEAD,
    EGG
} state_t;

typedef struct player_s {
    int id;                    // Player ID
    char *team_name;           // Team name of the player
    coords_t coords;           // Player coordinates on the map
    direction_t direction;     // Current direction of the player
    inventory_t inventory;     // Inventory of the player
    int level;                 // Player level
    state_t state;             // Current state of the player
    struct timespec waiting_start;//Whether the player is waiting for a command
    bool is_waiting_start;     // verif if start fct was already used
    struct timespec time_eat;             // time to know when eat
    bool was_a_egg;            // If the player was an egg
    bool is_incantation;      // If the player is in incantation
    queue_command_ai_t *command;
} player_t;

typedef struct command_gui_s {
    char *name;
    char **args;
} command_gui_t;

typedef struct queue_command_gui_s {
    command_gui_t command[25];         // Array of commands
    int front;                // Index of the front command
    int rear;                 // Index of the rear command
} queue_command_gui_t;

bool is_empty_gui(queue_command_gui_t *q);

bool is_full_gui(queue_command_gui_t *q);

void initialize_queue_gui(queue_command_gui_t *q);

void enqueue_gui(queue_command_gui_t *q, command_gui_t value);

command_gui_t *peek_gui(queue_command_gui_t *q);

void dequeue_gui(queue_command_gui_t *q);

bool is_empty_ai(queue_command_ai_t *q);

bool is_full_ai(queue_command_ai_t *q);

command_ai_t *peek(queue_command_ai_t *q);

void dequeue(queue_command_ai_t *q);

void enqueue(queue_command_ai_t *q, command_ai_t value);

void initialize_queue_ai(queue_command_ai_t *q);

void get_current_time(struct timespec *ts);

typedef struct linked_player_s {
    player_t player;            // Player data
    struct linked_player_s *next; // Pointer to the next player in the list
} linked_player_t;

#endif /* !PLAYER_H_ */

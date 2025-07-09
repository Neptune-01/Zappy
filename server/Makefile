##
## EPITECH PROJECT, 2025
## Makefile
## File description:
## Makefile for the server part of the zappy project years end project
##

SRC	=	./src/main.c	\
		./src/args.c	\
		./src/free_utils.c	\
		./src/flags_parser/port_parser.c	\
		./src/flags_parser/width_height_parser.c	\
		./src/flags_parser/frequency_parser.c	\
		./src/flags_parser/clients_args_parser.c	\
		./src/initializer/initializer_game_info.c	\
		./src/initializer/init_player.c	\
		./src/initializer/generate_ressources.c	\
		./src/initializer/init_socket.c	\
		./src/loop/server_loop.c	\
		./src/loop/server_state.c	\
		./src/loop/manage_death.c	\
		./src/loop/manage_death_utils.c	\
		./src/loop/win_condition.c	\
		./src/communication/handle_new_connection.c	\
		./src/communication/handle_client_message.c	\
		./src/communication/send_responce.c		\
		./src/communication/parse_line_cmd.c	\
		./src/communication/handle_gui_message.c	\
		./src/communication/parse_line_cmd_gui.c	\
		./src/communication/parse_and_process_cmd_gui.c	\
		./src/queue_fct/queue_cmd_ai.c	\
		./src/queue_fct/queue_cmd_ai_verif.c	\
		./src/queue_fct/queue_cmd_gui.c		\
		./src/queue_fct/queue_cmd_gui_verif.c	\
		./src/commands/manage_command.c	\
		./src/commands/manage_cmd_gui.c	\
		./src/commands/manage_time.c	\
		./src/commands/cmd_ai/mouvement.c	\
		./src/commands/cmd_ai/nb_unuse_cmd.c	\
		./src/commands/cmd_ai/inventory_cmd.c	\
		./src/commands/cmd_ai/look_cmd.c	\
		./src/commands/cmd_ai/take_cmd.c	\
		./src/commands/cmd_ai/drop_cmd.c	\
		./src/commands/cmd_ai/eject_cmd.c	\
		./src/commands/cmd_ai/eject_cmd_response.c	\
		./src/commands/cmd_ai/fork_cmd.c	\
		./src/commands/cmd_ai/incantation_cmd.c		\
		./src/commands/cmd_ai/incantation_lvlup.c	\
		./src/commands/cmd_ai/look_utils.c	\
		./src/commands/cmd_ai/look_utils_bis.c	\
		./src/commands/cmd_ai/eject_utils.c	\
		./src/commands/cmd_ai/broadcast_cmd.c	\
		./src/commands/cmd_ai/broadcast_utils.c	\
		./src/commands/cmd_gui/map_cmd.c	\
		./src/commands/cmd_gui/player_data.c	\
		./src/communication/manage_player_connect.c	\
		./src/communication/manage_player_connect_bis.c	\
		./src/communication/manage_player_connect_bisbis.c	\
		./src/communication/parse_client_command.c \
		./src/str_to_word_array/my_str_to_word_array.c	\

OBJ     =       $(SRC:.c=.o)

NAME    =       ../zappy_server

CFLAGS	=	-Wall -Wextra -g3

CPPFLAGS	=	-Iincludes/

CC	=	gcc

all:    $(NAME)

$(NAME):        $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(CFLAGS) -lm

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re:     fclean all
		rm $(OBJ)

.PHONY : all  clean fclean re

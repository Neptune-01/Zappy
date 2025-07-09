##
## EPITECH PROJECT, 2025
## B-YEP-400-PAR-4-1-zappy-pierre-etienne.henry
## File description:
## Makefile
##

NAME		= zappy_ai

RM			= rm -f

all: $(NAME)

$(NAME): $(OBJ)
	ln -s ai/main.py $(NAME)
	mv $(NAME) ../$(NAME)
	chmod +x main.py

clean:

fclean: clean
	$(RM) ../$(NAME)

re: fclean all

.PHONY: all clean fclean re
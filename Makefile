##
## Makefile général pour zappy (AI, GUI, Server)
##

.PHONY: all ai gui server clean fclean re

all: ai gui server

ai:
	@echo "=== Build AI ==="
	-$(MAKE) -C ai all

gui:
	@echo "=== Build GUI ==="
	-$(MAKE) -j -C GUI all

server:
	@echo "=== Build Server ==="
	$(MAKE) -j -C server all

clean:
	@echo "=== Clean AI ==="
	-$(MAKE) -C ai clean
	@echo "=== Clean GUI ==="
	-$(MAKE) -C GUI clean
	@echo "=== Clean Server ==="
	$(MAKE) -C server clean

fclean: clean
	@echo "=== Full Clean AI ==="
	-$(MAKE) -C ai fclean
	@echo "=== Full Clean GUI ==="
	-$(MAKE) -C GUI fclean
	@echo "=== Full Clean Server ==="
	$(MAKE) -C server fclean

re: fclean all

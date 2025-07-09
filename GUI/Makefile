##
## EPITECH PROJECT, 2025
## Zappy [WSL: Ubuntu]
## File description:
## Makefile
##

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lGL -lGLU

NAME = zappy_gui

SRC_DIR = src/zappy
OBJ_DIR = obj
INCLUDE_DIR = include

VPATH = \
	$(SRC_DIR)/core \
	$(SRC_DIR)/game/world \
	$(SRC_DIR)/game/entities \
	$(SRC_DIR)/graphics/utils \
	$(SRC_DIR)/graphics/renderers \
	$(SRC_DIR)/graphics/legacy \
	$(SRC_DIR)/input \
	$(SRC_DIR)/network/utils \
	$(SRC_DIR)/network/protocol

SRC_FILES = \
	Application.cpp \
	Tile.cpp MapGenerator.cpp \
	Player.cpp CreeperModel.cpp \
	OpenGLUtils.cpp MathUtils.cpp Camera3D.cpp ModelLoader.cpp TextureManager.cpp \
	TerrainRenderer.cpp EntityRenderer.cpp Renderer3D.cpp UIRenderer.cpp \
	main3D.cpp \
	InputHandler.cpp \
	NetworkManager.cpp NetworkThread.cpp \
	CommandParser.cpp SharedGameState.cpp

OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.cpp=.o))

GREEN = \033[32m
BLUE = \033[34m
RED = \033[31m
YELLOW = \033[33m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJ_FILES)
	@echo "$(BLUE)🔗 Linking $(NAME)...$(RESET)"
	@$(CXX) $(OBJ_FILES) -o $(NAME) $(LDFLAGS)
	@echo "$(GREEN) $(NAME) built successfully!$(RESET)"
	@echo "#!/bin/bash" > ../$(NAME)
	@echo "cd \$$(dirname \$$0)/GUI" >> ../$(NAME)
	@echo "exec ./$(NAME) \"\$$@\"" >> ../$(NAME)
	@chmod +x ../$(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(YELLOW)Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(YELLOW)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)
	@rm -f ../$(NAME)

re: fclean all

install-deps:
	@echo "$(BLUE)Installing dependencies...$(RESET)"
	@if command -v apt >/dev/null 2>&1; then \
		echo "Detected Debian/Ubuntu system"; \
		sudo apt update && \
		sudo apt install -y libsfml-dev libgl1-mesa-dev libglu1-mesa-dev; \
	elif command -v dnf >/dev/null 2>&1; then \
		echo "Detected Fedora system (dnf)"; \
		sudo dnf install -y SFML-devel mesa-libGL-devel mesa-libGLU-devel; \
	elif command -v yum >/dev/null 2>&1; then \
		echo "Detected RHEL/CentOS/older Fedora system (yum)"; \
		sudo yum install -y SFML-devel mesa-libGL-devel mesa-libGLU-devel; \
	else \
		echo "$(RED)Unsupported package manager. Please install dependencies manually:$(RESET)"; \
		echo "  - SFML development libraries"; \
		echo "  - OpenGL Mesa development libraries"; \
		echo "  - OpenGL Utility library Mesa development libraries"; \
		exit 1; \
	fi

debug: CXXFLAGS += -g -DDEBUG
debug: $(NAME)

release: CXXFLAGS += -O3 -DNDEBUG
release: $(NAME)

help:
	@echo "$(GREEN)Zappy - Makefile Help$(RESET)"
	@echo "$(BLUE)Targets:$(RESET)"
	@echo "  $(YELLOW)all$(RESET)        - Build the application"
	@echo "  $(YELLOW)clean$(RESET)      - Remove object files"
	@echo "  $(YELLOW)fclean$(RESET)     - Remove all generated files"
	@echo "  $(YELLOW)re$(RESET)         - Rebuild everything"
	@echo "  $(YELLOW)debug$(RESET)      - Build with debug symbols"
	@echo "  $(YELLOW)release$(RESET)    - Build optimized release"
	@echo "  $(YELLOW)install-deps$(RESET) - Install required dependencies"
	@echo "  $(YELLOW)help$(RESET)       - Show this help message"

.PHONY: all clean fclean re debug release install-deps help
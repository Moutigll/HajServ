# ------------------------
# Variables
# ------------------------
NAME = hajserv
LIB_NAME = libhajserv.a

CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude
AR = ar rcs
OBJ_DIR = build

# Include source lists
include src/sources.mk

# Generate object paths
OBJ_CORE = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC_CORE))
OBJ_SERVER = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC_SERVER))
OBJ_MODULES = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC_MODULES))

# ------------------------
# Rules
# ------------------------
all: $(LIB_NAME) $(NAME)

# Build static library
$(LIB_NAME): $(OBJ_CORE)
	$(AR) $@ $^

# Build executable
$(NAME): $(OBJ_SERVER) $(LIB_NAME) $(OBJ_MODULES)
	$(CC) $(CFLAGS) $^ -o $@

# Compile .c to .o in build/ with mirrored structure
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files
clean:
	rm -rf $(OBJ_DIR)

# Fclean = clean + binaries
fclean: clean
	rm -f $(LIB_NAME) $(NAME)

# Rebuild
re: fclean all

# Run server
run: $(NAME)
	./$(NAME)

.PHONY: all clean fclean re run

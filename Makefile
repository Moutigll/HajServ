# ------------------------
# Variables
# ------------------------
NAME = hajserv
LIB_NAME = libhajserv.a
TEST_BIN = $(OBJ_DIR)/tests/unit.out

CC = gcc
STRICT_FLAGS = -Wall -Wextra -Werror
CFLAGS = -Iinclude
COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage -O0
TEST_FLAGS = -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Iinclude

AR = ar rcs
OBJ_DIR = build

COVERAGE_DIR = $(OBJ_DIR)/coverage
COVERAGE_INFO = $(COVERAGE_DIR)/coverage.info
COVERAGE_HTML = $(COVERAGE_DIR)/html

# Include source lists
include src/sources.mk

# Generate object paths
OBJ_CORE = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC_CORE))
OBJ_SERVER = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC_SERVER))
OBJ_MODULES = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC_MODULES))
OBJ_UNIT_TESTS = $(patsubst tests/unit/%.c,$(OBJ_DIR)/tests/unit/%.o,$(SRC_UNIT_TESTS))

# ------------------------
# Rules
# ------------------------
all: $(LIB_NAME) $(NAME)

# Build static library (archive)
$(LIB_NAME): $(OBJ_CORE)
	$(AR) $@ $^

# Build server executable
# Note: link with COVERAGE_FLAGS because object files were compiled with them
$(NAME): $(OBJ_SERVER) $(LIB_NAME) $(OBJ_MODULES)
	$(CC) $(STRICT_FLAGS) $(CFLAGS) $(COVERAGE_FLAGS) $^ -o $@

# Build tests binary (objects first, then lib)
# Link with COVERAGE_FLAGS because lib was built with them
$(TEST_BIN): $(OBJ_UNIT_TESTS) $(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(TEST_FLAGS) $(COVERAGE_FLAGS) $(CFLAGS) $(OBJ_UNIT_TESTS) $(LIB_NAME) -o $@

# Run tests
test: $(TEST_BIN)
	@./$(TEST_BIN)

# Compile .c to .o for core/server/modules (with coverage)
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(STRICT_FLAGS) $(CFLAGS) $(COVERAGE_FLAGS) -c $< -o $@

# Compile .c to .o for unit tests (test sources do not need COVERAGE_FLAGS)
$(OBJ_DIR)/tests/unit/%.o: tests/unit/%.c
	@mkdir -p $(dir $@)
	$(CC) $(TEST_FLAGS) -c $< -o $@

# Generate coverage report using lcov + genhtml
coverage: re test
	@echo "Generating coverage..."
	@mkdir -p $(COVERAGE_DIR)
	lcov --directory $(OBJ_DIR) --capture --output-file $(COVERAGE_INFO)
	@mkdir -p $(COVERAGE_HTML)
	genhtml $(COVERAGE_INFO) --output-directory $(COVERAGE_HTML)
	@echo "Coverage HTML report generated in $(COVERAGE_HTML)"

# Clean object files and coverage data
clean:
	rm -rf $(OBJ_DIR)

# Fclean = clean + binaries + coverage artifacts
fclean: clean
	rm -f $(LIB_NAME) $(NAME) $(TEST_BIN)
	rm -f *.gcda *.gcno *.gcov

# Rebuild
re: fclean all

# Run server
run: $(NAME)
	./$(NAME)

.PHONY: all clean fclean re run test coverage

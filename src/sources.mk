SRC_DIR		= src/
CORE_DIR	= $(SRC_DIR)core/
MODULES_DIR	= $(SRC_DIR)modules/
SERVER_DIR	= $(SRC_DIR)server/

# Core library sources
SRC_CORE	= \
	$(CORE_DIR)sockets/create.c \
	$(CORE_DIR)server.c

# Modules (optional)
SRC_MODULES	= 

# Server executable sources
SRC_SERVER	= \
	$(SERVER_DIR)main.c



TEST_DIR	= tests/
INTEG_DIR	= $(TEST_DIR)integration/
UNIT_DIR	= $(TEST_DIR)unit/

SRC_INTEG_TESTS	= 

SRC_UNIT_TESTS	= \
	$(UNIT_DIR)main.c
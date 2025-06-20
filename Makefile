NAME		=	webserv
EXECUTABLE	=	$PWD/$(NAME)

RED			=	$(shell tput bold setaf 1)
GREEN		=	$(shell tput setaf 2)
PURPLE		=	$(shell tput setaf 5)
BLUE		=	$(shell tput setaf 6)
LIGHTBLUE	=	$(shell tput -Txterm setaf 4)
BOLD		=	$(shell tput bold)
RESET		=	$(shell tput -Txterm sgr0)

CC			=	c++
CFLAGS		=	-Wall -Werror -Wextra -std=c++98 -I includes -g

include Files.mk

SRCS		= 	$(addsuffix .cpp,$(addprefix $(SRC_DIR)/,$(FILES)))
OBJS		= 	$(addsuffix .o,$(addprefix $(OBJ_DIR)/,$(FILES)))
OBJ_DIR		= 	obj

NB			=	$(shell echo $(SRCS) | wc -w)
NUMB2		=	0
NUMB3		=	0
NUMB4		=	0
PERCENT		=	0

all: $(NAME)
	@if [ $(shell echo $(NUMB3)) -eq 0 ]; then echo "$(BOLD)$(RED)Nothing to be made.$(RESET)"; fi

$(NAME): $(OBJS)
	@if [ -f ./$(NAME) ]; then echo "$(BOLD)$(BLUE)Executable already exists.$(RESET)"; else echo "$(BOLD)$(BLUE)Created the executable : $(NAME)$(RESET)"; fi
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(BOLD)$(PURPLE)Finished the compilation of the Makefile$(RESET)"
	@$(eval NUMB3=$(shell echo $$(($(NUMB3)+1))))

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(eval NUMB2=$(shell echo $$(($(NUMB2)+1))))
	$(eval PERCENT=$(shell awk "BEGIN { printf(\"%.1f\", $(NUMB2) * 100 / $(NB)) }"))
	@$(CC) -c $< -o $@ $(CFLAGS)
	@$(eval NUMB4=$(shell echo $@ / | tr -cd '/' | wc -c))
	@if [ $(shell uname -a | grep arch | wc -l) -gt 0 ] || [ $(shell cat /etc/*-release | grep fedora | wc -l) -gt 0 ]; then echo -e "$(BOLD)$(PURPLE)[Percent : "$(PERCENT)%"] $(BOLD)$(GREEN) \t~Compiling $(shell echo $< | cut -d'/' -f 2) : $(shell echo $@ | cut -d'/' -f $(NUMB4))$(RESET)"; else echo "$(BOLD)$(PURPLE)[Percent : "$(PERCENT)%"] $(BOLD)$(GREEN) \t~Compiling $(shell echo $< | cut -d'/' -f 2) : $(shell echo $@ | cut -d'/' -f $(NUMB4))$(RESET)"; fi

clean:
	@if [ $(shell find -name "*.o" | wc -l) -gt 0 ]; then echo "$(BOLD)$(RED)Removed all objs$(RESET)"; else echo "$(BOLD)$(PURPLE)All objs were already removed.$(RESET)"; fi
	@rm -rf $(OBJ_DIR)
	@echo "$(BOLD)$(BLUE)Finished cleaning all$(RESET)"

fclean: clean
	@if [ -f ./$(NAME) ]; then echo "$(BOLD)$(RED)Removed the executable : $(NAME).$(RESET)"; else echo "$(BOLD)$(PURPLE)The executable $(NAME) was already removed.$(RESET)"; fi
	@rm -f $(NAME)
	@echo "$(BOLD)$(LIGHTBLUE)Finished fcleaning all$(RESET)"

re: fclean all

.PHONY: all clean fclean re

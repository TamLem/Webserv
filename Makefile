NAME		=	webserv

CXX			=	c++

UNAME		:=	$(shell uname)

ifeq ($(UNAME), Darwin)
CXXFLAGS	=	-std=c++98 -Wall -Wextra -Werror
else
CXXFLAGS	=	-std=c++98 -Wall -Wextra #-Werror // please fix this Tam
endif

CXXFLAGS	+=	-g -fsanitize=address -fsanitize=alignment -fsanitize=unreachable -fsanitize=bounds

CXXFLAGS	+=	-D SHOW_LOG #enables the printing of surface-level logs, server-side only !!! make re is needed if you just enabled this !!!

# CXXFLAGS	+=	-D SHOW_LOG_2 #enables the printing of deep-level logs, server-side only !!! make re is needed if you just enabled this !!!

#directories
PWD			=	$(shell pwd)
SRC_DIR		=	server/src/
OBJ_DIR		=	server/obj/
INC_DIR		=	server/include/

#controll codes
RESET		=	\033[0m
GREEN		=	\033[32m
YELLOW		=	\033[33m
BLUE		=	\033[34m
RED			=	\033[31m
UP			=	\033[A
CUT			=	\033[K

#source files
#				automaticly decides which Sources to use depending on the OS
ifeq ($(UNAME), Darwin) #(Darwin is the used OS on school Macs, might be different on other Macs)
SRC_FILES	=	main.cpp \
				SocketHandler.cpp \
				Server.cpp \
				Message.cpp \
				Response.cpp \
				Cgi.cpp		\
				Request.cpp \
				Config.cpp \
				SingleServerConfig.cpp
else
#(blocking server for testing on Linux)
SRC_FILES	=	Linux_server.cpp \
				Message.cpp \
				Response.cpp \
				Cgi.cpp		\
				CgiResoponse.cpp \
				Request.cpp \
				Config.cpp \
				SingleServerConfig.cpp
endif

OBJ_FILES	=	$(SRC_FILES:.cpp=.o)

#paths
SRC			=	$(addprefix $(SRC_DIR), $(SRC_FILES))
OBJ			=	$(addprefix $(OBJ_DIR), $(OBJ_FILES))

#all rule
all: $(NAME)
#ascii art can be found at: https://patorjk.com/software/taag/#p=display&f=Calvin%20S&t=ft%20containers
	@printf "\n"
	@printf "$(GREEN)┬ ┬┌─┐┌┐ ┌─┐┌─┐┬─┐┬  ┬ \n$(RESET)"
	@printf "$(GREEN)│││├┤ ├┴┐└─┐├┤ ├┬┘└┐┌┘ \n$(RESET)"
	@printf "$(GREEN)└┴┘└─┘└─┘└─┘└─┘┴└─ └┘  \n$(RESET)"
	@printf "\n"

#compile the executable
$(NAME): $(OBJ)
	@echo "$(YELLOW)Compiling [$(NAME)]...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)Finished [$(NAME)]$(RESET)"

#compile objects
$(OBJ_DIR)%.o:$(SRC_DIR)%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "$(YELLOW)Compiling [$@]...$(RESET)"
	@$(CXX) $(CXXFLAGS) -I $(INC_DIR) -o $@ -c $<
	@printf "$(UP)$(CUT)"
	@echo "$(GREEN)Finished [$@]$(RESET)"
	@printf "$(UP)$(CUT)"

#clean rule
clean:
	@if [ -d "$(OBJ_DIR)" ]; then \
	rm -rf $(OBJ_DIR); \
	echo "$(BLUE)Deleting all objects from $(PWD)...$(RESET)"; else \
	echo "No objects to remove from $(PWD)."; \
	fi;

#fclean rule
fclean: clean
	@if [ -f "$(NAME)" ]; then \
	rm -f $(NAME); \
	echo "$(BLUE)Deleting $(NAME) from $(PWD)...$(RESET)"; else \
	echo "No Executable to remove from $(PWD)."; \
	fi;

#re rule
re: fclean all

#run rule
run: all
	./$(NAME) server/config/ae.conf

#phony
.PHONY: all clean fclean re run
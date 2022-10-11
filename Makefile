NAME		=	webserv

CXX			=	c++

UNAME		:=	$(shell uname)

CXXFLAGS	=	-std=c++98 -Wall -Wextra -Werror

# CXXFLAGS	+=	-g -fsanitize=address -fsanitize=alignment -fsanitize=unreachable -fsanitize=bounds # can be used to check for any memory faults

# CXXFLAGS	+=	-D FORTYTWO_TESTER

# CXXFLAGS	+=	-D SHOW_CONSTRUCTION # enables the printing of constructor/destructor messages

# CXXFLAGS	+=	-D SHOW_LOG_SOCKET

# CXXFLAGS	+=	-D SHOW_LOG #enables the printing of surface-level logs, server-side only

# CXXFLAGS	+=	-D SHOW_LOG_2 #enables the printing of deep-level logs, server-side only

# CXXFLAGS	+=	-D SHOW_LOG_REQUEST

# CXXFLAGS	+=	-D SHOW_LOG_RESPONSE

# CXXFLAGS	+=	-D SHOW_LOG_EXCEPTION

# CXXFLAGS	+=	-D SHOW_LOG_ROUTING

# CXXFLAGS	+=	-D SHOW_LOG_CGI

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
SRC_FILES	=	main.cpp \
				SocketHandler.cpp \
				Server.cpp \
				ServerExceptions.cpp \
				Routing.cpp	\
				ServerUtils.cpp \
				Message.cpp \
				Response.cpp \
				Send.cpp \
				Receive.cpp \
				Cgi.cpp	\
				CgiResponse.cpp \
				Request.cpp \
				Config.cpp \
				SingleServerConfig.cpp

OBJ_FILES	=	$(SRC_FILES:.cpp=.o)

DEP			=	$(wildcard $(INC_DIR)*.hpp) $(wildcard $(INC_DIR)*/*.hpp) Makefile

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
$(OBJ_DIR)%.o:$(SRC_DIR)%.cpp $(DEP)
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
	./$(NAME) server/config/test.conf

ae: all
	./$(NAME) server/config/ae.conf

site: all
	./$(NAME) server/config/site.conf

trans: all
	./$(NAME) server/config/trans.conf

42: all
	./$(NAME) server/config/42tester.conf

our: all
	./$(NAME) server/config/ourTester.conf

tester:
	@make -C ./ourTester

rmtester:
	@make clean -C ./ourTester

rm42tester:
	-rm 42tester/file_should_exist_after
	-rm 42tester/multiple_same
	-rm 42tester/post_body

#phony
.PHONY: all clean fclean re run tester

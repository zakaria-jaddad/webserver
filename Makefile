NAME = ft_webserver

CFLAGS = -Wall -Werror -Wextra -std=c++98 -g -fsanitize=address
# CFLAGS = -Wall -Werror -Wextra -std=c++17 -g

SRC_DIR		= src
CONF_DIR	= config
SRV_DIR		= server
REQ_DIR		= request
RSP_DIR		= response
CGI_DIR		= cgi
OBJ_DIR		= obj
UTI_DIR 	= utils

## src files (just add their plain names don't write their paths)
MAIN_SRC	= ft_webserver.cpp
CONF_SRC 	= Config.cpp ServerConfig.cpp LocationConfig.cpp parseHelpers.cpp
SRV_SRC		= Server.cpp
REQ_SRC		= Request.cpp
RSP_SRC		= ResolvedConfig.cpp Response.cpp ResponseCrafting.cpp staticMethods.cpp \
			  cgiExecution.cpp ResponseUtilities.cpp mime.cpp
CGI_SRC		= CgiEnv.cpp Cgi.cpp envUtils.cpp
UTI_SRC = isCgi.cpp getExtension.cpp

## project srcs : here you add the prefix and boom compiled ;p
PROJECT_SRC	=	$(addprefix $(SRC_DIR)/$(SRV_DIR)/, $(SRV_SRC)) \
				$(addprefix $(SRC_DIR)/$(CONF_DIR)/, $(CONF_SRC)) \
				$(addprefix $(SRC_DIR)/$(REQ_DIR)/, $(REQ_SRC)) \
				$(addprefix $(SRC_DIR)/$(CGI_DIR)/, $(CGI_SRC)) \
				$(addprefix $(SRC_DIR)/$(UTI_DIR)/, $(UTI_SRC)) \
				$(addprefix $(SRC_DIR)/$(RSP_DIR)/, $(RSP_SRC))

SRC			= $(addprefix $(SRC_DIR)/, $(MAIN_SRC)) $(PROJECT_SRC)

OBJ			= $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
PROJECT_OBJ	= $(PROJECT_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CXX = c++

all: $(NAME)
#
$(NAME): $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o $(NAME)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -MMD -c $< -o $@


clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME) $(TEST_NAME)

re: fclean all

.PHONY: all clean fclean re test

test:
	./run_tests.sh

-include $(OBJ:.o=.d)

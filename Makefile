NAME		= ft_webserver

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 

INCLUDES	= -I includes

SRC_DIR		= src
SRC			= $(SRC_DIR)/ft_webserver.cpp $(SRC_DIR)/Server.cpp

OBJ_DIR		= obj
OBJ			= $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

test: $(SRC_DIR)/minimal_test_server.cpp
	$(CXX) $(CXXFLAGS) $< -o $(NAME)
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

NAME = ft_webserver
CFLAGS = -Wall -Werror -Wextra -std=c++98

SRC_DIR		= src
SRC			= $(SRC_DIR)/ft_webserver.cpp \
					$(SRC_DIR)/server/server.cpp \
OBJ = $(SRC:.cpp=.o)

CXX = c++

all: $(NAME)

test: $(SRC_DIR)/minimal_test_server.cpp
	$(CXX) $(CXXFLAGS) $< -o $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o  $@ -MMD

re: fclean all

clean: 
	rm -rf $(OBJ) $(OBJ:.o=.d)

fclean: clean
	rm -rf $(NAME)

.PHONY: clean
-include $(OBJ:.o=.d)

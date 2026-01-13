NAME = ft_webserver
CFLAGS = -Wall -Werror -Wextra -std=c++98

SRC_DIR		= src
SRC		= 	$(SRC_DIR)/ft_webserver.cpp \
			$(SRC_DIR)/server/Server.cpp 
OBJ = $(SRC:.cpp=.o)

CXX = c++

all: $(NAME)

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


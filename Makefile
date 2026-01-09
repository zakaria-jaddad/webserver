NAME = ft_webserver
TEST_NAME = ft_webserver_test

CFLAGS = -Wall -Werror -Wextra -std=c++17

SRC_DIR		= src
OBJ_DIR		= obj
TEST_DIR	= test

MAIN_SRC      = $(SRC_DIR)/ft_webserver.cpp
PROJECT_SRC   = $(SRC_DIR)/server/server.cpp
SRC           = $(MAIN_SRC) $(PROJECT_SRC)

OBJ 					= $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
PROJECT_OBJ   = $(PROJECT_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

TEST_SRC      = $(TEST_DIR)/request_test.cpp
TEST_OBJ      = $(TEST_SRC:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

GTEST_DIR     = external/googletest
GTEST_LIBS    = $(GTEST_DIR)/build/lib/libgtest.a $(GTEST_DIR)/build/lib/libgtest_main.a
LDFLAGS       = -lpthread
GTEST_INCLUDE = -I $(GTEST_DIR)/googletest/include

CXX = c++

all: $(NAME)
#
$(NAME): $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o $(NAME)

test: $(PROJECT_OBJ) $(TEST_OBJ) 
	$(CXX) $(CFLAGS) $(PROJECT_OBJ) $(TEST_OBJ) $(GTEST_LIBS) $(LDFLAGS) -o $(TEST_NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(GTEST_INCLUDE) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME) $(TEST_NAME)

re: fclean all

.PHONY: all clean fclean re test

-include $(OBJ:.o=.d)
-include $(TEST_OBJ:.o=.d)

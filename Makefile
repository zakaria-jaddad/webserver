NAME = ft_webserver
TEST_NAME = ft_webserver_test

CFLAGS = -Wall -Werror -Wextra -std=c++98 -g -fsanitize=address
#CFLAGS = -Wall -Werror -Wextra -std=c++17 -g

SRC_DIR		= src
CONF_DIR	= config
SRV_DIR		= server
REQ_DIR		= request
RSP_DIR		= response
CGI_DIR		= cgi
OBJ_DIR		= obj
TEST_DIR	= test

## src files (just add their plain names don't write their paths)
MAIN_SRC	= ft_webserver.cpp
CONF_SRC 	= Config.cpp ServerConfig.cpp LocationConfig.cpp parseHelpers.cpp
SRV_SRC		= Server.cpp
REQ_SRC		= Request.cpp
RSP_SRC		= ResolvedConfig.cpp Response.cpp ResponseCrafting.cpp staticMethods.cpp \
			  cgiExecution.cpp ResponseUtilities.cpp
CGI_SRC		= CgiEnv.cpp Cgi.cpp envUtils.cpp

## project srcs : here you add the prefix and boom compiled ;p
PROJECT_SRC	=	$(addprefix $(SRC_DIR)/$(SRV_DIR)/, $(SRV_SRC)) \
				$(addprefix $(SRC_DIR)/$(CONF_DIR)/, $(CONF_SRC)) \
				$(addprefix $(SRC_DIR)/$(REQ_DIR)/, $(REQ_SRC)) \
				$(addprefix $(SRC_DIR)/$(CGI_DIR)/, $(CGI_SRC)) \
				$(addprefix $(SRC_DIR)/$(RSP_DIR)/, $(RSP_SRC))

SRC			= $(addprefix $(SRC_DIR)/, $(MAIN_SRC)) $(PROJECT_SRC)

OBJ			= $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
PROJECT_OBJ	= $(PROJECT_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

TEST_SRC_FILES = $(SRC_DIR)/request/Request.cpp

TEST_TEST_FILES = $(TEST_DIR)/request_line_test.cpp \
									$(TEST_DIR)/request_uri.cpp \
									$(TEST_DIR)/request_header_test.cpp \
									$(TEST_DIR)/test.cpp 

TEST_OBJ = $(TEST_SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) \
           $(TEST_TEST_FILES:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

GTEST_DIR     = external/googletest
GTEST_LIBS    = $(GTEST_DIR)/build/lib/libgtest.a $(GTEST_DIR)/build/lib/libgtest_main.a
LDFLAGS       = -lpthread
GTEST_INCLUDE = -I $(GTEST_DIR)/googletest/include

CXX = c++

all: $(NAME)
#
$(NAME): $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o $(NAME)

test: $(TEST_OBJ) 
	$(CXX) $(CFLAGS) $(TEST_OBJ) $(GTEST_LIBS) $(LDFLAGS) -o $(TEST_NAME)

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

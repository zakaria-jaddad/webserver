cpp := c++

NAME := config

flags := -Wall -Werror -Wextra

SRCS := test.cpp Config.cpp

OBJS := $(addprefix objs/, $(SRCS:.cpp=.o))

all: $(NAME)

$(NAME) : $(OBJS)
	$(cpp) $(flags) $(OBJS) -o $(NAME) -MMD

objs/%.o: %.cpp
	mkdir -p objs/
	$(cpp) -c $< -o $@ -MMD

clean: 
	rm -rf objs/

fclean: clean
	rm -rf $(NAME)

re: fclean all

-include $(OBJS:.o=.d)


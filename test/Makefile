CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98
SRCS := main.cpp parse_path.cpp send.cpp
OBJS := $(addprefix obj/,$(SRCS:.cpp=.o))
NAME := miniwebserv
all : $(NAME)
$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
obj/%.o : %.cpp | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@
obj :
	mkdir -p obj
clean :
	rm -rf obj
fclean : clean
	rm -f miniwebserv
re : fclean all
.PHONY : all clean fclean re
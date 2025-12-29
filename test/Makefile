CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -I include
CXXFLAGS += -MMD -MP -pedantic
NAME := webserv
SRCS := src/main.cpp \
		src/parse_path.cpp \
		src/send.cpp \
		src/server_accept.cpp \
		src/net_utils.cpp \
		src/server.cpp \
		src/server_write.cpp \
		src/server_read.cpp \
		src/server_close.cpp
OBJDIR := obj
OBJS := $(SRCS:src/%.cpp=$(OBJDIR)/%.o)
DEPS := $(OBJS:.o=.d)
all : $(NAME)
$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
$(OBJDIR)/%.o : src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean :
	rm -rf $(OBJDIR)
fclean : clean
	rm -f $(NAME)
re : fclean all
.PHONY : all clean fclean re
-include $(DEPS)
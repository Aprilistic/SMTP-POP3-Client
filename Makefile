NAME = SMTP_POP3_client
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -MMD
LDFLAGS = -lssl -lcrypto

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    OPENSSL_DIR=$(shell brew --prefix openssl@3)
    CXXFLAGS += -I$(OPENSSL_DIR)/include
    LDFLAGS += -L$(OPENSSL_DIR)/lib
endif

INCLUDES = -I ./includes 
OBJ_DIR = objs

SRCS := $(wildcard srcs/core/*.cpp) \
        $(wildcard srcs/POP3/*.cpp) \
        $(wildcard srcs/*.cpp)
        
SRCS_DIR := $(dir $(SRCS))

OBJS := $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.cpp=.o)))

DEPS = $(OBJS:.o=.d)

vpath %.cpp $(SRCS_DIR)

all : $(OBJ_DIR) $(NAME)

$(NAME) : $(OBJS)
    $(CXX) $(OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR) :
    mkdir -p $@

$(OBJ_DIR)/%.o : %.cpp
    $(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean :
    rm -rf $(OBJ_DIR)
    rm -rf ./$(NAME).dSYM
    rm -f $(NAME)

fclean : clean
    rm -f $(NAME)
    rm -f leaks.txt

re :
    make fclean
    make all -j4

leaks :
    valgrind --leak-check=full --show-leak-kinds=all --log-file=leaks.txt $(PWD)/$(NAME) $(PWD)/configs/example.conf

.PHONY : all clean fclean re leaks

-include $(DEPS)

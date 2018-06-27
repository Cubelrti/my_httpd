CC = g++ -std=c++11
CC_FLAGS = -g -W -Wall -Werror -I./
EXEC = my_http
OBJS = main.o core.o

$(EXEC): $(OBJS)
	$(CC) $(CC_FLAGS) $(OBJS) -o $@

main.o: main.cpp
	$(CC) $(CC_FLAGS) -c -o $@ main.cpp

core.o: core.cpp core.h connection.hpp
	$(CC) $(CC_FLAGS) -c -o $@ core.cpp


all : $(EXEC)

clean:
	rm -rf $(OBJS) $(EXEC)

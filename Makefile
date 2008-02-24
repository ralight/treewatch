CC=gcc
CXX=g++
NAME=treewatch
CFLAGS=-Wall -ggdb
LDFLAGS=-nopie

all : $(NAME)

$(NAME).o : $(NAME).c
	$(CC) -c $(CFLAGS) -o $@ $<

$(NAME) : $(NAME).o
	$(CC) ${LDFLAGS} -o $@ $^

memtest : $(NAME)
	valgrind -v --show-reachable=yes --leak-check=full ./$^

clean :
	-rm -f $(NAME) *.o


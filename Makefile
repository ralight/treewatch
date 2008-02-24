CC=gcc
CXX=g++
NAME=treewatch
CFLAGS=-Wall -ggdb
LDFLAGS=-nopie

all : treewatch

treewatch.o : treewatch.c
	$(CC) -c $(CFLAGS) -o $@ $<

treewatch : treewatch.o
	$(CC) ${LDFLAGS} -o $@ $^

install : treewatch
	install treewatch /usr/local/bin/

memtest : treewatch
	valgrind -v --show-reachable=yes --leak-check=full ./$^

clean :
	-rm -f treewatch *.o


prefix=/usr/local

VERSION=\"1.1.0\"
BUILDDATE=\"20080302\"

CC=gcc
CXX=g++
NAME=treewatch
CFLAGS=-Wall -ggdb -DVERSION="${VERSION}" -DBUILDDATE="${BUILDDATE}"
LDFLAGS=


all : treewatch

treewatch.o : treewatch.c
	$(CC) -c $(CFLAGS) -o $@ $<

treewatch : treewatch.o
	$(CC) ${LDFLAGS} -o $@ $^

install : treewatch
	install treewatch ${prefix}/bin/
	install treewatch.1 ${prefix}/man/man1/

uninstall :
	rm -f ${prefix}/bin/treewatch
	rm -f ${prefix}/man/man1/treewatch.1

memtest : treewatch
	valgrind -v --show-reachable=yes --leak-check=full ./$^

clean :
	-rm -f treewatch *.o


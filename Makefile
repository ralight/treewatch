INSTALLPATH=/usr/local/bin

VERSION=\"1.0.1\"
BUILDDATE=\"20080224\"

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
	install treewatch ${INSTALLPATH}

uninstall :
	rm -f ${INSTALLPATH}/treewatch

memtest : treewatch
	valgrind -v --show-reachable=yes --leak-check=full ./$^

clean :
	-rm -f treewatch *.o


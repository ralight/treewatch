prefix=/usr/local
mandir=${prefix}/man

VERSION=\"1.1.0\"
BUILDDATE=\"20080302\"

CC=gcc
CXX=g++
CFLAGS=-Wall -ggdb -DVERSION="${VERSION}" -DBUILDDATE="${BUILDDATE}"
LDFLAGS=

.PHONY : all install uninstall memtest clean

all : treewatch

treewatch.o : treewatch.c
	$(CC) -c $(CFLAGS) -o $@ $<

treewatch : treewatch.o
	$(CC) ${LDFLAGS} -o $@ $^

install : treewatch
	install treewatch ${DESTDIR}${prefix}/bin/
	install treewatch.1 ${DESTDIR}${mandir}/man/man1/

uninstall :
	rm -f ${DESTDIR}${prefix}/bin/treewatch
	rm -f ${DESTDIR}${mandir}/man1/treewatch.1

memtest : treewatch
	valgrind -v --show-reachable=yes --leak-check=full ./$^

clean :
	-rm -f treewatch *.o


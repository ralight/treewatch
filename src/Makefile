include ../config.mk

treewatch_SOURCES= treewatch.c
treewatch_OBJS=$(treewatch_SOURCES:.c=.o)

.PHONY : all clean install uninstall

all : treewatch

treewatch : $(treewatch_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o : %.c
	$(CC) -DVERSION=\"${VERSION}\" -DBUILDDATE=\"${BUILDDATE}\" ${CFLAGS} -c $^

clean :
	-rm -f treewatch *.o

install : treewatch
	$(INSTALL) -d ${DESTDIR}${prefix}/bin/
	$(INSTALL) -s treewatch ${DESTDIR}${prefix}/bin/treewatch

uninstall :
	-rm -f ${DESTDIR}${prefix}/bin/treewatch

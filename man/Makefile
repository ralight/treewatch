include ../config.mk

.PHONY : all clean install uninstall

all : 

clean :

install :
	$(INSTALL) -d ${DESTDIR}$(mandir)/man1
	$(INSTALL) treewatch.1 ${DESTDIR}${mandir}/man1/treewatch.1

uninstall :
	-rm -f ${DESTDIR}${mandir}/man1/treewatch.1

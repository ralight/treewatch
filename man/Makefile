include ../config.mk

.PHONY : all clean install uninstall dist dist-clean

all : treewatch.1

treewatch.1 : treewatch.1.xml
	$(XSLTPROC) $^

clean :

install :
	$(INSTALL) -d ${DESTDIR}$(mandir)/man1
	$(INSTALL) treewatch.1 ${DESTDIR}${mandir}/man1/treewatch.1

uninstall :
	-rm -f ${DESTDIR}${mandir}/man1/treewatch.1

dist-clean :
	-rm -f treewatch.1

html : *.xml
	set -e; for m in *.xml; \
		do \
		hfile=$$(echo $${m} | sed -e 's#\(.*\)\.xml#\1#' | sed -e 's/\./-/g'); \
		$(XSLTPROC) html.xsl $${m} > $${hfile}.html; \
	done


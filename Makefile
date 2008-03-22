include config.mk

DIRS=man po src

.PHONY : all treewatch clean install uninstall dist dist-clean sign copy

all : treewatch

treewatch :
	for d in ${DIRS}; do $(MAKE) -C $${d}; done

clean :
	for d in ${DIRS}; do $(MAKE) -C $${d} clean; done

install : treewatch
	@for d in ${DIRS}; do $(MAKE) -C $${d} install; done

uninstall :
	@for d in ${DIRS}; do $(MAKE) -C $${d} uninstall; done

dist : clean
	mkdir -p treewatch-${VERSION}
	cp -r images man src ChangeLog COPYING Makefile README TODO config.mk treewatch-${VERSION}/
	tar -jcf treewatch-${VERSION}.tar.bz2 treewatch-${VERSION}/

dist-clean : clean
	-rm -rf treewatch-${VERSION}*

sign : dist
	gpg --detach-sign -a treewatch-${VERSION}.tar.bz2

copy : sign
	scp treewatch-${VERSION}.tar.bz2 treewatch-${VERSION}.tar.bz2.asc atchoo:atchoo.org/tools/treewatch/files/

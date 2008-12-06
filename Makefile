include config.mk

DIRS=man po src
DISTDIRS=man

.PHONY : all treewatch clean install uninstall dist dist-clean sign copy

all : treewatch

treewatch :
	@for d in ${DIRS}; do $(MAKE) -C $${d}; done

clean :
	@for d in ${DIRS}; do $(MAKE) -C $${d} $@; done

install : treewatch
	@for d in ${DIRS}; do $(MAKE) -C $${d} $@; done

uninstall :
	@for d in ${DIRS}; do $(MAKE) -C $${d} $@; done

dist : dist-clean
	@for d in ${DISTDIRS}; do $(MAKE) -C $${d} $@; done
	mkdir -p treewatch-${VERSION}
	cp -r images man po src ChangeLog COPYING Makefile README TODO config.mk treewatch-${VERSION}/
	tar -jcf treewatch-${VERSION}.tar.bz2 treewatch-${VERSION}/

dist-clean : clean
	@for d in ${DISTDIRS}; do $(MAKE) -C $${d} $@; done
	-rm -rf treewatch-${VERSION}*

sign : dist
	gpg --detach-sign -a treewatch-${VERSION}.tar.bz2

copy : sign
	scp treewatch-${VERSION}.tar.bz2 treewatch-${VERSION}.tar.bz2.asc atchoo:atchoo.org/tools/treewatch/files/

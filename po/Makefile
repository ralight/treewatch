include ../config.mk

POFILES=$(wildcard *.po)
MOFILES=$(POFILES:.po=.mo)
POSOURCES=$(wildcard ../src/*.c)

.PHONY : all clean install uninstall

all : treewatch.pot $(POFILES) $(MOFILES)

treewatch.pot : $(POSOURCES)
	$(XGETTEXT) -k_ $(POSOURCES) -o tmp.po
	$(MSGMERGE) -U $@ tmp.po 2>/dev/null
	-rm -f tmp.po

%.po : treewatch.pot
	$(MSGMERGE) -U $@ $<

%.mo : %.po
	$(MSGFMT) -o $@ $<

install : $(MOFILES)
	@for j in $(MOFILES); do \
		k=`basename $$j`; \
		country=`echo $$j | cut -d "." -f 1`; \
		dir=$(DESTDIR)$(prefix)/share/locale/$$country/LC_MESSAGES; \
		echo "installing $$dir/treewatch.mo ($$k)"; \
		install -d $$dir; \
		cp $$j $$dir/treewatch.mo; \
	done

uninstall :
	@for j in $(MOFILES); do \
		k=`basename $$j`; \
		country=`echo $$j | cut -d "." -f 1`; \
		dir=$(DESTDIR)$(prefix)/share/locale/$$country/LC_MESSAGES; \
		rm -f $$dir/treewatch.mo; \
	done

clean :
	-rm  -f $(MOFILES) *~


PREFIX=/usr/local
DATADIR=$(PREFIX)/share
LOCALEDIR=$(DATADIR)/locale
mkdir_p= mkdir -p
MSGFMT=msgfmt

OBJECTS=de/HeeksCAD.mo it/HeeksCAD.mo sk/HeeksCAD.mo
all: ${OBJECTS}

de/HeeksCAD.mo: de/HeeksCAD.po
	$(MSGFMT) -o $@ $?

it/HeeksCAD.mo: it/HeeksCAD.po
	$(MSGFMT) -o $@ $?

sk/HeeksCAD.mo: sk/HeeksCAD.po
	$(MSGFMT) -o $@ $?

clean:
	-rm ${OBJECTS}

install:
	$(mkdir_p) "$(DESTDIR)$(LOCALEDIR)/de/LC_MESSAGES"
	$(mkdir_p) "$(DESTDIR)$(LOCALEDIR)/it/LC_MESSAGES"
	cp de/HeeksCAD.mo "$(DESTDIR)$(LOCALEDIR)/de/LC_MESSAGES/"
	cp it/HeeksCAD.mo "$(DESTDIR)$(LOCALEDIR)/it/LC_MESSAGES/"

uninstall:
	-rm "$(DESTDIR)$(LOCALEDIR)/*/LC_MESSAGES/HeeksCAD.mo"

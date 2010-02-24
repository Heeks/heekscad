PREFIX=/usr/local
DATADIR=$(PREFIX)/share
LOCALEDIR=$(DATADIR)/locale
mkdir_p= mkdir -p
MSGFMT=msgfmt

#italian translation is not ready, because no pot file was available it used german file
#as template, so if you use it.po you'll have almost in german
#OBJECTS=de/HeeksCAD.mo it/HeeksCAD.mo
OBJECTS=de/HeeksCAD.mo

all: ${OBJECTS}

de/HeeksCAD.mo: de/HeeksCAD.po
	$(MSGFMT) -o $@ $?

#it/HeeksCAD.mo: it/HeeksCAD.po
#	$(MSGFMT) -o $@ $?

clean:
	-rm ${OBJECTS}

install:
	$(mkdir_p) "$(DESTDIR)$(LOCALEDIR)/de/LC_MESSAGES"
#	$(mkdir_p) "$(DESTDIR)$(LOCALEDIR)/it/LC_MESSAGES"
	cp de/HeeksCAD.mo "$(DESTDIR)$(LOCALEDIR)/de/LC_MESSAGES/"
#	cp it/HeeksCAD.mo "$(DESTDIR)$(LOCALEDIR)/it/LC_MESSAGES/"

uninstall:
	-rm "$(DESTDIR)$(LOCALEDIR)/*/LC_MESSAGES/HeeksCAD.mo"

# Just typing "make" will build the web page; typing "make web" will
# actually copy it into our Rhodes Mill content directory.

WEB = ./www

all: pyephem.html pyephem-manual.html
all: $(WEB)/index.html $(WEB)/tutorial.html $(WEB)/CHANGELOG

$(WEB)/index.html: pyephem.html
	umask 022; tidy -q pyephem.html > $@

$(WEB)/tutorial.html: pyephem-manual.html
	umask 022; cp $< $@

pyephem-manual.html: premanual.html format.py
	cat premanual.html | python ./format.py > $@

#/var/www/manual.html: premanual.html format.py
#	umask 022; cat $< | python ./format.py > $@

$(WEB)/CHANGELOG: ../CHANGELOG
	umask 022; cat $< > $@

$(WEB)/pyephem-$(VERSION).tar.gz: ../dist/pyephem-$(VERSION).tar.gz
	umask 022; cat ../dist/pyephem-$(VERSION).tar.gz > $@

RSTS = $(filter-out quick.rst, $(wildcard *.rst))
PAGES = $(patsubst %.rst, $(WEB)/%.html, $(RSTS))

all: rst
rst: $(PAGES) $(WEB)/quick.html
$(PAGES): $(WEB)/%.html: %.rst style.css
	umask 022; rst2html --stylesheet-path=style.css < $< > $@
$(WEB)/quick.html: quick.rst quick.css
	umask 022; rst2html --stylesheet-path=quick.css < $< > $@

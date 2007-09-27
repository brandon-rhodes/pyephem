# Just typing "make" will build the web page; typing "make web" will
# actually copy it into our Rhodes Mill content directory.

WEB = ./www

all: pyephem.html pyephem-manual.html
web: $(WEB)/pyephem.html $(WEB)/pyephem-manual.html $(WEB)/pyephem-CHANGELOG

$(WEB)/pyephem.html: pyephem.html
	umask 022; tidy -q pyephem.html > $@

$(WEB)/pyephem-manual.html: pyephem-manual.html
	umask 022; cp $< $@

pyephem-manual.html: premanual.html format.py
	cat premanual.html | python ./format.py > $@

#/var/www/manual.html: premanual.html format.py
#	umask 022; cat $< | python ./format.py > $@

$(WEB)/pyephem-CHANGELOG: ../CHANGELOG
	umask 022; cat $< > $@

$(WEB)/pyephem-$(VERSION).tar.gz: ../dist/pyephem-$(VERSION).tar.gz
	umask 022; cat ../dist/pyephem-$(VERSION).tar.gz > $@

RSTS = $(wildcard *.rst)
PAGES = $(patsubst %.rst, $(WEB)/%.html, $(RSTS))

rst: $(PAGES)
$(PAGES): $(WEB)/%.html: %.rst style.css
	umask 022; rst2html --stylesheet-path=style.css < $< > $@

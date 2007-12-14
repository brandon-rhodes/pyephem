# Just typing "make" will build the web page; typing "make web" will
# actually copy it into our Rhodes Mill content directory.

WEB = ./www

all: pyephem.html $(WEB)/index.html $(WEB)/CHANGELOG

$(WEB)/index.html: pyephem.html
	umask 022; tidy -q pyephem.html > $@

$(WEB)/CHANGELOG: ../CHANGELOG
	umask 022; cat $< > $@

RSTS = $(filter-out quick.rst, $(wildcard *.rst))
PAGES = $(patsubst %.rst, $(WEB)/%.html, $(RSTS))

all: rst
rst: $(PAGES) $(WEB)/quick.html
$(PAGES): $(WEB)/%.html: %.rst style.css
	umask 022; rst2html --stylesheet-path=style.css < $< > $@
$(WEB)/quick.html: quick.rst quick.css
	umask 022; rst2html --stylesheet-path=quick.css < $< > $@

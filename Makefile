# Just typing "make" will build the web page; typing "make web" will
# actually copy it into our Rhodes Mill content directory.

WEB = ./www
DOC = ./src/ephem/doc

all: $(WEB)/index.html $(WEB)/CHANGELOG

$(WEB)/index.html: $(DOC)/pyephem.html
	umask 022; tidy -q $< > $@

$(WEB)/CHANGELOG: CHANGELOG
	umask 022; cat $< > $@

RSTS = $(filter-out $(DOC)/quick.rst, $(wildcard $(DOC)/*.rst))
PAGES = $(patsubst $(DOC)/%.rst, $(WEB)/%.html, $(RSTS))

all: rst
rst: $(PAGES) $(WEB)/quick.html
$(PAGES): $(WEB)/%.html: $(DOC)/%.rst $(DOC)/style.css
	umask 022; rst2html --stylesheet-path=$(DOC)/style.css < $< > $@
$(WEB)/quick.html: $(DOC)/quick.rst $(DOC)/quick.css
	umask 022; rst2html --stylesheet-path=$(DOC)/quick.css < $< > $@

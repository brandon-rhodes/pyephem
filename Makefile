#
# Building HTML documentation
#

WEB = $(shell pwd)/www
DOC = ./src/ephem/doc
RST2HTML = rst2html --initial-header-level=2 -g -d --strip-comments

all: $(WEB)/index.html $(WEB)/CHANGELOG

$(WEB)/index.html: $(DOC)/pyephem.html
	umask 022; tidy -q $< > $@

$(WEB)/CHANGELOG: CHANGELOG
	umask 022; cat $< > $@

RSTS = $(filter-out $(DOC)/quick.rst $(DOC)/nav.rst, $(wildcard $(DOC)/*.rst))
PAGES = $(patsubst $(DOC)/%.rst, $(WEB)/%.html, $(RSTS))

all: rst
rst: $(PAGES) $(WEB)/quick.html
$(PAGES): $(WEB)/%.html: $(DOC)/%.rst $(DOC)/style.css
	umask 022; cd $(DOC); $(RST2HTML) --stylesheet-path=style.css < $*.rst > $@
$(WEB)/quick.html: $(DOC)/quick.rst $(DOC)/quick.css
	umask 022; cd $(DOC); $(RST2HTML) --stylesheet-path=quick.css < quick.rst > $@

#
# Building data sets
#

D=./extensions/data
G=./generate
BDLS=$(patsubst $G/%, %, $(wildcard $G/*.9910 $G/*.1020))
BDLSRCS=$(patsubst %, $D/%.c, $(BDLS))

data: $(BDLSRCS)

$(BDLSRCS): $D/%.c: $G/% $G/satxyz.py
	python $G/satxyz.py $G/$* > $@

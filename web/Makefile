WEB = /home/brandon/sites/rhodesmill/html/brandon/projects
VERSION = 3.5.2a

all: $(WEB)/pyephem.html $(WEB)/pyephem-manual.html \
     $(WEB)/pyephem-$(VERSION).tar.gz

$(WEB)/pyephem.html: pyephem.html
	umask 022; cat pyephem.html > $@

$(WEB)/pyephem-$(VERSION).tar.gz: ../dist/pyephem-$(VERSION).tar.gz
	umask 022; cat ../dist/pyephem-$(VERSION).tar.gz > $@

$(WEB)/pyephem-README: ../README
	umask 022; cat ../README > $@

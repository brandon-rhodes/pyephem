#/var/www/manual.html: premanual.html format.py
#	umask 022; cat $< | python ./format.py > $@

WEB = /home/brandon/sites/rhodesmill/html/brandon/projects
VERSION = 3.5.2a

all: $(WEB)/pyephem.html $(WEB)/pyephem-manual.html $(WEB)/pyephem.jpg \
   $(WEB)/pyephem-$(VERSION).tar.gz

$(WEB)/pyephem.html: pyephem.html
	umask 022; cat pyephem.html > $@

$(WEB)/pyephem-manual.html: premanual.html format.py
	umask 022; cat premanual.html | python ./format.py > $@

#/var/www/manual.html: premanual.html format.py
#	umask 022; cat $< | python ./format.py > $@

$(WEB)/pyephem.jpg: pyephem.jpg
	umask 022; cat $< > $@

$(WEB)/pyephem-$(VERSION).tar.gz: ../dist/pyephem-$(VERSION).tar.gz
	umask 022; cat ../dist/pyephem-$(VERSION).tar.gz > $@


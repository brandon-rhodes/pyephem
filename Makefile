# Copyright 1998 Brandon Craig Rhodes
# Licensed under the GNU General Public License

#
# Configuration: before installing, make this variable point to where
# you have Python 1.5 modules installed.
#

PYTHON = /usr/lib/python1.5/

#
# Basic compliation options.
#

CFLAGS = -g
IDIR = ./libastro
LDIR = ./libastro
SWIG = swig

#
# Convenient targets.
#

all: docs module
docs: pyephem.html
module: ephemcmodule.so

#
# Installation
#

install: ephemcmodule.so
	install -m 644 ephem.py $(PYTHON)
	install -m 755 ephemcmodule.so $(PYTHON)/lib-dynload

#
# Creation of a dynamically linked interface with Swig.
#
# The swig command that creates `ephemcmodule.so' also creates an
# `ephem.py' class interface; both files should be made available to
# Python.
#

ephemcmodule.so: ephem_wrap.o $(LDIR)/libastro.a
	$(CC) -L$(LDIR) -shared -o $@ $< -lastro

$(LDIR)/libastro.a:
	$(MAKE) -C $(LDIR)

ephem_wrap.o: ephem_wrap.c
	$(CC) -O2 -I$(IDIR) -c $< -DHAVE_CONFIG_H -I/usr/include/python1.5

ephem_wrap.c: ephem.i
	$(SWIG) -dnone -python -globals preference -shadow $<

ephem.i: format_swig.m4 ephem.m4
	m4 --prefix-builtins format_swig.m4 ephem.m4 > $@

#
# Creation of documentation.
#

pyephem.html: format_html.m4 ephem.m4
	m4 --prefix-builtins format_html.m4 ephem.m4 > $@

#
# Cleanup.
#

clean:
	rm -f ephemcmodule.so \
		ephem_wrap.o ephem_wrap.c ephem.py ephem.pyc \
		ephem.i pyephem.html

# Makefile for image processing routines, libip.
# (C) 2001 Elwood Charles Downey

# gcc
CC = gcc
CFLAGS= -I../libastro -O2 -Wall

# macosx universal binary
# CFLAGS= -I../libastro -O2 -Wall -arch i386 -arch ppc

# solaris
# CC = cc
# CFLAGS= -I../libastro -O

# AIX
# CC = xlc
# CFLAGS= -I../libastro -O2 -qlanglvl=ansi -qarch=com -qmaxmem=16384

# HP-UX
# CC = cc
# CFLAGS= -I../libastro -Aa -fast

OBJS =	\
	explodegif.o	\
	fits.o		\
	fsmatch.o	\
	gaussfit.o	\
	lstsqr.o	\
	median.o	\
	sqr.o		\
	stars.o		\
	stats.o		\
	walk.o		\
	wcs.o

HS = ip.h fsmatch.h

libip.a: $(HS)	$(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@

libip.so:    $(OBJS)
	gcc -shared -o $@ $(OBJS)

clobber:
	rm -f *.o libip.a

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: Makefile,v $ $Date: 2006/02/27 13:24:29 $ $Revision: 1.11 $ $Name:  $

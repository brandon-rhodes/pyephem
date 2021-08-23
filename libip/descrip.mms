# Makefile for image processing routines, libip.
# (C) 2001 Elwood Charles Downey

# gcc
CC = cc
CFLAGS= /INCLUDE=[-.libastro]

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
	explodegif.obj,	\
	fits.obj,		\
	fsmatch.obj,	\
	gaussfit.obj,	\
	lstsqr.obj,	\
	median.obj,	\
	sqr.obj,		\
	stars.obj,		\
	stats.obj,		\
	walk.obj,		\
	wcs.obj

HS = ip.h fsmatch.h

libip.olb :	$(OBJS)
	lib/crea $@ $?

#libip.so:    $(OBJS)
#	gcc -shared -o $@ $(OBJS)

clean :
	del *.o;* 


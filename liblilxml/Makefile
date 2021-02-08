# Makefile for the basic XML routines.
# The idea is to compile and archive them into liblilxml.a

# compiler and flags

# gcc
CC = gcc
CFLAGS= -O2 -Wall

# macosx universal binary
# CFLAGS= -O2 -Wall -arch i386 -arch ppc

# solaris
# CC = cc
# CFLAGS= -O

# AIX
# CC = xlc
# CFLAGS= -O2 -qlanglvl=ansi -qarch=com -qmaxmem=16384

# HP-UX
# CC = cc
# CFLAGS= -Aa -fast

HS = lilxml.h

OBJS = lilxml.o base64.o

liblilxml.a: $(HS) $(OBJS)
	ar r $@ $(OBJS)
	ranlib $@

liltest: liltest.o liblilxml.a
	$(CC) $(LDFLAGS) -o liltest liltest.o liblilxml.a

liltest.o: $(HS) lilxml.c
	$(CC) -DMAIN_TST $(CFLAGS) -c -o liltest.o lilxml.c

clobber:
	touch x.o x.a
	rm -f *.o *.a core liltest

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: Makefile,v $ $Date: 2006/02/24 03:37:19 $ $Revision: 1.11 $ $Name:  $

# Makefile for the basic XML routines.
# The idea is to compile and archive them into liblilxml.a

# compiler and flags
# Adopted by Alex Chupahin alex@rostov.ultrastar.ru

# gcc
CC = cc
CFLAGS= 

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

OBJS = lilxml.obj, BASE64.OBJ

liblilxml.olb : $(HS) $(OBJS)
	lib/crea $@ $(OBJS)


liltest : liltest.obj liblilxml.olb
	link/exe=liltest $(LDFLAGS)  liltest.obj,liblilxml/lib

liltest.obj : $(HS) lilxml.c
	$(CC)/define=MAIN_TST $(CFLAGS) lilxml.c

clean :
	del *.o;*, *.a;* core;*, liltest;*

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: descrip.mms,v $ $Date: 2005/09/01 02:26:35 $ $Revision: 1.2 $ $Name:  $

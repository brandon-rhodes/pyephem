# Makefile for the decompress-only jpeg library, libjpegd.a.

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

HS =  \
	jerror.h \
	jmorecfg.h \
	jpegint.h \
	jpeglib.h \
	jconfig.h \
	jinclude.h \
	jmemsys.h \
	jdct.h \
	jversion.h \
	jdhuff.h

OBJS = \
	jcomapi.o \
	jdapimin.o \
	jdapistd.o \
	jdatasrc.o \
	jdcoefct.o \
	jdcolor.o \
	jddctmgr.o \
	jdhuff.o \
	jdinput.o \
	jdmainct.o \
	jdmarker.o \
	jdmaster.o \
	jdmerge.o \
	jdphuff.o \
	jdpostct.o \
	jdsample.o \
	jerror.o \
	jidctflt.o \
	jidctfst.o \
	jidctint.o \
	jidctred.o \
	jmemmgr.o \
	jmemnobs.o \
	jquant1.o \
	jquant2.o \
	jutils.o

libjpegd.a: $(HS) $(OBJS)
	ar r $@ $(OBJS)
	ranlib $@

clobber:
	touch x.o x.a
	rm -f *.o *.a

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: Makefile,v $ $Date: 2006/02/24 03:37:19 $ $Revision: 1.5 $ $Name:  $

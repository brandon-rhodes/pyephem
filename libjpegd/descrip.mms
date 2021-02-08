# Makefile for the decompress-only jpeg library, libjpegd.a.

# compiler and flags

# gcc
CC = cc
CFLAGS= 
#-O2 -ffast-math -Wall

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
	jcomapi.obj, \
	jdapimin.obj, \
	jdapistd.obj, \
	jdatasrc.obj, \
	jdcoefct.obj, \
	jdcolor.obj, \
	jddctmgr.obj, \
	jdhuff.obj, \
	jdinput.obj, \
	jdmainct.obj, \
	jdmarker.obj, \
	jdmaster.obj, \
	jdmerge.obj, \
	jdphuff.obj, \
	jdpostct.obj, \
	jdsample.obj, \
	jerror.obj, \
	jidctflt.obj, \
	jidctfst.obj, \
	jidctint.obj, \
	jidctred.obj, \
	jmemmgr.obj, \
	jmemnobs.obj, \
	jquant1.obj, \
	jquant2.obj, \
	jutils.obj

libjpegd.olb : $(HS) $(OBJS)
	lib/cre $@ $(OBJS)


clean :
	 del *.o;*
	del *.olb;*

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: descrip.mms,v $ $Date: 2005/07/27 21:29:35 $ $Revision: 1.1 $ $Name:  $

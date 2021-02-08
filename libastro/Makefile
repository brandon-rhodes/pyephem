# Makefile for the basic astronomy routines.
# The idea is to compile and archive them into libastro.a

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

HS = astro.h bdl.h chap95.h deepconst.h preferences.h satlib.h satspec.h \
	sattypes.h vector.h vsop87.h

OBJS =			\
	aa_hadec.o	\
	aberration.o	\
	actan.o		\
	airmass.o	\
	anomaly.o	\
	ap_as.o		\
	atlas.o		\
	auxil.o		\
	bdl.o		\
	chap95.o	\
	chap95_data.o	\
	circum.o	\
	comet.o		\
	constel.o	\
	dbfmt.o		\
	deep.o		\
	deltat.o	\
	earthsat.o	\
	eq_ecl.o	\
	eq_gal.o	\
	formats.o	\
	helio.o		\
	jupmoon.o	\
	libration.o	\
	magdecl.o	\
	marsmoon.o	\
	misc.o		\
	mjd.o		\
	moon.o		\
	mooncolong.o	\
	moonnf.o	\
	nutation.o	\
	obliq.o		\
	parallax.o	\
	parallactic.o	\
	plans.o		\
	plmoon.o	\
	plshadow.o	\
	precess.o	\
	reduce.o	\
	refract.o	\
	rings.o		\
	riset.o		\
	riset_cir.o	\
	satmoon.o	\
	sdp4.o		\
	sgp4.o		\
	sphcart.o	\
	sun.o		\
	thetag.o	\
	utc_gst.o	\
	umoon.o		\
	twobody.o	\
	vsop87.o	\
	vsop87_data.o

libastro.a: $(HS) $(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@

libastro.so: $(HS) $(OBJS)
	$(CC) -shared -o $@ $(OBJS)

clobber:
	rm -f *.o libastro.a

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: Makefile,v $ $Date: 2006/02/24 03:37:19 $ $Revision: 1.30 $ $Name:  $

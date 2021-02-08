# Makefile for the basic astronomy routines.
# The idea is to compile and archive them into libastro.a

# compiler and flags

# gcc
CC = cc
#CFLAGS= -O2 -ffast-math -Wall

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
	aa_hadec.obj,	\
	aberration.obj,	\
	actan.obj,		\
	airmass.obj,	\
	anomaly.obj,	\
	ap_as.obj,		\
	atlas.obj,		\
	auxil.obj,		\
	bdl.obj,		\
	chap95.obj,	\
	chap95_data.obj,	\
	circum.obj,	\
	comet.obj,		\
	constel.obj,	\
	dbfmt.obj,		\
	deep.obj,		\
	deltat.obj,	\
	earthsat.obj,	\
	eq_ecl.obj,	\
	eq_gal.obj,	\
	formats.obj,	\
	helio.obj,		\
	jupmoon.obj,	\
	libration.obj,	\
	magdecl.obj,	\
	marsmoon.obj,	\
	misc.obj,		\
	mjd.obj,		\
	moon.obj,		\
	mooncolong.obj,	\
	moonnf.obj,	\
	nutation.obj,	\
	obliq.obj,		\
	parallax.obj,	\
	parallactic.obj,	\
	plans.obj,		\
	plmoon.obj,	\
	plshadow.obj,	\
	precess.obj,	\
	reduce.obj,	\
	refract.obj,	\
	rings.obj,		\
	riset.obj,		\
	riset_cir.obj,	\
	satmoon.obj,	\
	sdp4.obj,		\
	sgp4.obj,		\
	sphcart.obj,	\
	sun.obj,		\
	thetag.obj,	\
	utc_gst.obj,	\
	umoon.obj,		\
	twobody.obj,	\
	vsop87.obj,	\
	vsop87_data.obj

libastro.olb : $(OBJS)
	lib/crea $@ $(OBJS)

#libastro.so: $(HS) $(OBJS)
#	$(CC) -shared -o $@ $(OBJS)

clean :
	del *.o;*

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: descrip.mms,v $ $Date: 2005/07/27 21:28:40 $ $Revision: 1.1 $ $Name:  $

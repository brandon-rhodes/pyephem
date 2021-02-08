# Simple Makefile for xephem v3.6

# Sample compile and link flags for a few systems. Default is Linux or
# probably any system using gcc with X11 and Motif stuff in /usr/X11R6.
# Otherwise find yours or one that looks like the way Motif applications are
# built on your system. Remove the leading # to uncomment lines. The basic
# idea to build xephem is to first go compile the libraries in ../../libastro,
# ../../libip, ../../liblilxml and ../../libjpegd then come back here and
# compile and link all the *.c files into one executable, xephem. 
# Note: some motif libraries now use the printing frame buffer and need -lXp

# -I and -L flags to find required supporting libraries

# Adopted by Alex Chupahin alex@rostov.ultrastar.ru

LIBINC = /INCLUDE=([-.-.libastro],[-.-.libip],[-.-.liblilxml],[-.-.libjpegd],[-.-.libpng],[-.-.libz])
LIBLIB = [-.-.libastro]libastro/lib, [-.-.libip]libip/lib, [-.-.liblilxml]liblilxml/lib, [-.-.libjpegd]libjpegd/lib, [-.-.libpng]libpng/lib, [-.-.libz]libz/lib, 




CC = cc
#MOTIFI = -I/usr/X11R6/include
#MOTIFL = -L/usr/X11R6/lib
#CLDFLAGS = -ffast-math
CFLAGS = $(LIBINC) 
LDFLAGS = 
#XLIBS = -lXm -lXp -lXt -lXext -lSM -lICE -lXmu -lX11
#LIBS = $(XLIBS) $(LIBLIB) -lm

# MKS Enterprise toolkit
# CC = cc
# CLDFLAGS =
# CFLAGS = $(LIBINC) $(CLDFLAGS) -O 
# LDFLAGS = $(LIBLNK) $(CLDFLAGS) -W/subsystem:windows -s
# XLIBS = -lXm -lXt -lX11
# LIBS = $(XLIBS) $(LIBLIB) -lm

# SVR4 derivatives:
# CC = cc
# CLDFLAGS = 
# CFLAGS = $(LIBINC) $(CLDFLAGS) -O
# LDFLAGS = $(LIBLNK) $(CLDFLAGS)
# XLIBS = -lXm -lXt -lX11
# LIBS = $(XLIBS) $(LIBLIB) -lsocket -lnsl -lc -lm /usr/ucblib/libucb.a
# Note: if you get regex undefined, add -lgen to the end of LIBS.

# Solaris:
# Motif stuff might also be in /usr/dt/share/{include,lib}.
# be sure /opt/SUNWspro/bin and /usr/ccs/bin are in your PATH ahead of /usr/ucb.
# CC = cc
# CLDFLAGS =
# MOTIFI = -I/usr/dt/include
# MOTIFL = -L/usr/dt/lib -R/usr/dt/lib
# CFLAGS = $(LIBINC) $(CLDFLAGS) -O $(MOTIFI) -I/usr/openwin/include
# LDFLAGS = $(LIBLNK) $(CLDFLAGS) $(MOTIFL) -L/usr/openwin/lib
# XLIBS = -lXm -lXt -lX11
# LIBS = $(XLIBS) $(LIBLIB) -lm -lsocket -lnsl

# HP-UX
# CC = cc
# CLDFLAGS =
# CFLAGS = $(LIBINC) $(CLDFLAGS) -Aa -fast $(MOTIFI)
# LDFLAGS = $(LIBLNK) $(CLDFLAGS) $(MOTIFL)

# AIX
# CC = xlc
# CLDFLAGS =
# CFLAGS = $(LIBINC) $(CLDFLAGS) -O2 -qlanglvl=ansi -qarch=com -qmaxmem=16384 $(MOTIFI)
# LDFLAGS = $(LIBLNK) $(CLDFLAGS) $(MOTIFL)


INCS =	db.h dm.h indiapi.h map.h net.h patchlevel.h plot.h ps.h \
	rotated.h sites.h skyeyep.h skyhist.h skyip.h skylist.h skytoolbar.h \
	trails.h xephem.h

OBJS =	\
aavso.obj,\
annotmenu.obj,\
broadcast.obj,\
calmenu.obj,\
closemenu.obj,\
compiler.obj,\
coordsmenu.obj,\
datamenu.obj,\
db.obj,\
dbmenu.obj,\
earthmap.obj,\
earthmenu.obj,\
fallbacks.obj,\
favmenu.obj,\
formats.obj,\
fsmenu.obj,\
gallerymenu.obj,\
glance.obj,\
gsc.obj,\
gscnet.obj,\
helpmenu.obj,\
homeio.obj,\
hznmenu.obj,\
imregmenu.obj,\
indimenu.obj,\
jpeg2pm.obj,\
jupmenu.obj,\
listmenu.obj,\
mainmenu.obj,\
marsmenu.obj,\
marsmmenu.obj,\
moonmenu.obj,\
moviemenu.obj,\
msgmenu.obj,\
netmenu.obj,\
objmenu.obj,\
obslog.obj,\
patchlevel.obj,\
plot_aux.obj,\
plotmenu.obj,\
preferences.obj,\
progress.obj,\
ps.obj,\
query.obj,\
rotated.obj,\
satmenu.obj,\
saveres.obj,\
scope.obj,\
sites.obj,\
skybinary.obj,\
skyeyep.obj,\
skyfifos.obj,\
skyfiltmenu.obj,\
skyfits.obj,\
skyhist.obj,\
skyip.obj,\
skylist.obj,\
skytoolbar.obj,\
skyviewmenu.obj,\
solsysmenu.obj,\
splash.obj,\
srchmenu.obj,\
sunmenu.obj,\
time.obj,\
tips.obj,\
trailmenu.obj,\
uranusmenu.obj,\
ucac.obj,\
usno.obj,\
versionmenu.obj,\
webdbmenu.obj,\
xe2.obj,\
xe3.obj,\
xephem.obj,\
xmisc.obj

all :  xephem.exe 

xephem.exe : $(INCS) $(OBJS)
	link/exe=$@ $(LDFLAGS) $(OBJS),options.opt/opt 

#xephem.1: xephem.man
#	nroff -man $? > $@

#libs:
#	cd ../../libastro; make
#	cd ../../libip; make
#	cd ../../liblilxml; make
#	cd ../../libjpegd; make

clean :
	del *.obj;*


# For RCS Only -- Do Not Edit
# @(#) $RCSfile: descrip.mms,v $ $Date: 2006/02/08 09:12:15 $ $Revision: 1.5 $ $Name:  $

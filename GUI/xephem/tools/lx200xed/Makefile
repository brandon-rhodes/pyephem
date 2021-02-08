LIBASTRO = ../../../../libastro
CLDFLAGS =
CFLAGS = -Wall -O2 $(CLDFLAGS) -I$(LIBASTRO)
LDFLAGS = $(CLDFLAGS) -L$(LIBASTRO)
CC = gcc

LIBS = -lastro -lm

OBJS = 			\
	liblx200.o	\
	lx200xed.o

TARS = Makefile GPL README liblx200.c liblx200.h lx200xed.c

lx200xed: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

lx200xed.tar.gz: $(TARS)
	tar cfz lx200xed.tar.gz $(TARS)

clobber:
	rm -f $(OBJS) lx200xed

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: Makefile,v $ $Date: 2000/11/27 21:55:46 $ $Revision: 1.1 $ $Name:  $

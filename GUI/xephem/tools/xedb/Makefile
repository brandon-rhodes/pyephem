# Makefile for xedb.
# assumes gcc, adjust to suit.

ASLIBDIR = ../../../../libastro
CLDFLAGS =
GCCFLAGS = -O2 -ffast-math -Wall
CFLAGS = $(CLDFLAGS) $(GCCFLAGS) -I$(ASLIBDIR)
LDFLAGS = $(CLDFLAGS) -L$(ASLIBDIR)
LIBS = -lastro -lm

OBJS =	xedb.o	\
	print.o

xedb: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

TARFILES = Makefile README print.c sample.res xedb.c

xedb.tar.gz: $(TARFILES)
	cd ..; tar cvfz xedb/$@ `cd xedb; ls $(TARFILES) | sed -e s,^,xedb/,`

clean:
	touch x.o
	rm -f *.o xedb


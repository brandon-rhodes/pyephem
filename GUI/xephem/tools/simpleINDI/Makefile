CC = gcc

LILXML = ../../../../liblilxml
ASTRO = ../../../../libastro

LILXMLA = $(LILXML)/liblilxml.a
ASTROA = $(ASTRO)/libastro.a

CLDFLAGS = -g
CFLAGS = $(CLDFLAGS) -O2 -Wall -I$(LILXML) -I$(ASTRO)
LDFLAGS = $(CLDFLAGS)
LIBS = $(LILXMLA) $(ASTROA) -lm

simpleINDI: simpleINDI.o $(LILXMLA) $(ASTROA)
	$(CC) $(LDFLAGS) simpleINDI.o -o $@ $(LIBS)

$(LILXMLA):
	cd $(LILXML); make

$(ASTROA):
	cd $(ASTRO); make

clean:
	rm -f simpleINDI simpleINDI.o 

CLDFLAGS =
CFLAGS = $(CLDFLAGS) -Wall -O2
LDFLAGS =

# macosx universal binary
# CFLAGS= $(CLDFLAGS) -O2 -Wall -arch i386 -arch ppc

OBJS =	adler32.o	\
	compress.o	\
	crc32.o		\
	uncompr.o	\
	deflate.o	\
	trees.o		\
	zutil.o		\
	inflate.o	\
	inftrees.o	\
	inffast.o

libz.a: $(OBJS)
	ar rc libz.a $(OBJS)
	-ranlib libz.a

testzlib: testzlib.o libz.a
	cc $(LDFLAGS) -o testzlib testzlib.o libz.a

clean:
	rm -f *.o *.a testzlib

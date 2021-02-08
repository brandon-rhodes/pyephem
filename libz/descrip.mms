CC = cc

OBJS =	adler32.obj,	\
	compress.obj,	\
	crc32.obj,		\
	uncompr.obj,	\
	deflate.obj,	\
	trees.obj,		\
	zutil.obj,		\
	inflate.obj,	\
	inftrees.obj,	\
	inffast.obj

libz.olb : $(OBJS)
	lib/crea $@ $?

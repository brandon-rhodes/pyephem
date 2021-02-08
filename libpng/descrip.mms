# makefile for libpng.olb

CC = cc
CFLAGS = /INCLUDE=[-.libz]

OBJS = png.obj, pngerror.obj, pngget.obj, pngmem.obj, pngpread.obj, \
	pngread.obj, pngrio.obj, pngrtran.obj, pngrutil.obj, pngset.obj, \
	pngtrans.obj, pngwio.obj, pngwrite.obj, pngwtran.obj, pngwutil.obj

libpng.olb : $(OBJS)
	lib/crea $@ $?

#ifndef _XmPngI_h
#define _XmPngI_h

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

int _XmPngGetImage(Screen * screen, FILE * infile, Pixel background,
                   XImage ** ximage);

#endif

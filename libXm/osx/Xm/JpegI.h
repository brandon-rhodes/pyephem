#ifndef _XmJpegI_h
#define _XmJpegI_h

#include <stdio.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <X11/Xlib.h>

typedef struct _XmJpegErrorMgrRec {
    struct  jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
} XmJpegErrorMgrRec, *XmJpegErrorMgr;

typedef struct { JSAMPLE red, green, blue; } CTable;

void _XmJpegErrorExit(j_common_ptr cinfo);
int _XmJpegGetImage(Screen *screen, FILE *infile, XImage **ximage);

#endif

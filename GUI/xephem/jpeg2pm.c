#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include <Xm/Xm.h>

#include "jpeglib.h"
#include "xephem.h"

/* read an open jpeg file and return a new pixmap and its size.
 * return 0 if ok, else fill why[] and return -1.
 */
int
jpeg2pm (Display *dsp,
Colormap cm,
FILE *jpegfp,
int *wp, int *hp,
Pixmap *pmp,
XColor xcols[256],
char why[])
{
	Window win = RootWindow(dsp, DefaultScreen(dsp));
	unsigned char *jpegpix;
	unsigned char r[256], g[256], b[256];
	XImage *xip;
	Pixmap pm;
	GC gc;
	int x, y;
	int w, h;
	int i;

	/* read the image */
	jpegpix = jpegRead (jpegfp, &w, &h, r, g, b, why);
	if (!jpegpix) {
	    strcpy (why, "Could not read jpeg image\n");
	    return (-1);
	}

	/* allocate colors -- don't be too fussy */
	for (i = 0; i < 256; i++) {
	    XColor *xcp = xcols+i;
	    xcp->red   = ((short)(r[i] & 0xf8) << 8) | 0x7ff;
	    xcp->green = ((short)(g[i] & 0xf8) << 8) | 0x7ff;
	    xcp->blue  = ((short)(b[i] & 0xf8) << 8) | 0x7ff;
	    if (!XAllocColor (dsp, cm, xcp)) {
		strcpy (why, "Can not get all image map colors");
		free ((void *)(jpegpix));
		if (i > 0)
		    freeXColors (dsp, cm, xcols, i);
		return (-1);
	    }
	}

	/* create XImage */
	xip = create_xim (w, h);
	if (!xip) {
	    freeXColors (dsp, cm, xcols, 256);
	    free ((void *)jpegpix);
	    strcpy (why, "No memory for image");
	    return (-1);
	}

	/* N.B. now obligued to free xip */

	/* fill XImage with image */
	for (y = 0; y < h; y++) {
	    int yrow = y*w;
	    for (x = 0; x < w; x++) {
		int gp = (int)jpegpix[x + yrow];
		unsigned long p = xcols[gp].pixel;
		XPutPixel (xip, x, y, p);
	    }
	}

	/* create pixmap and fill with image */
	pm = XCreatePixmap (dsp, win, w, h, xip->depth);
	gc = DefaultGC (dsp, DefaultScreen(dsp));
	XPutImage (dsp, pm, gc, xip, 0, 0, 0, 0, w, h);

	/* free jpegpix and xip */
	free ((void *)jpegpix);
	free ((void *)xip->data);
	xip->data = NULL;
	XDestroyImage (xip);

	/* that's it! */
	*wp = w;
	*hp = h;
	*pmp = pm;
	return (0);
}

/* lib jpeg's error scheme */
struct my_error_mgr {
    struct jpeg_error_mgr pub;	/* "public" fields */
    jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr *my_error_ptr;
static void
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	longjmp(myerr->setjmp_buffer, 1);
}

unsigned char *
jpegRead(FILE *infile, int *width, int *height, unsigned char r[256],
unsigned char g[256], unsigned char b[256], char why[])
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	unsigned char *retBuffer=NULL;
	unsigned char *rb;
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;			/* physical row in output buffer */
	int i;

	/* set up the normal JPEG error routines, then override error_exit */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
	    /* If we get here, the JPEG code has signaled an error. */
	    jpeg_destroy_decompress(&cinfo);
	    if (retBuffer)
		free(retBuffer);
	    strcpy (why, "jpeg reports error during decompression");
	    return (NULL);
	}

	/* init decompression.
	 * We can ignore the return value from jpeg_read_header since
	 *   (a) suspension is not possible with the stdio data source, and
	 *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	 * See libjpeg.doc for more info.
	 */
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	(void) jpeg_read_header(&cinfo, TRUE);

	/* start decompression, 1-byte pixels, 256 colors */
  	cinfo.quantize_colors = TRUE; 
	cinfo.desired_number_of_colors = 256;
	cinfo.two_pass_quantize = TRUE;
	cinfo.out_color_space = JCS_RGB;
	jpeg_start_decompress(&cinfo);

	/* get image memory */
	if (!(retBuffer = (unsigned char *) malloc(cinfo.output_width 
			* cinfo.output_height * cinfo.output_components))) {
	    jpeg_destroy_decompress(&cinfo);
	    strcpy (why, "Couldn't create space for JPEG read");
	    return(NULL);
	}

	/* Make a one-row-high sample array that will go away when done */
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
			    ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* loop through file, creating image array */
	rb = retBuffer;
	row_stride = cinfo.output_width * cinfo.output_components;
	while (cinfo.output_scanline < cinfo.output_height) {
	    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
	    memcpy(rb,buffer[0],row_stride);
	    rb += row_stride;
	}

	/* report back size */
	*width =  cinfo.output_width;
	*height =  cinfo.output_height;

	/* set up X colormap */
	if (cinfo.out_color_components  == 3) {
	    for (i=0; i < cinfo.actual_number_of_colors; i++) {
		r[i] = (unsigned char)cinfo.colormap[0][i];
		g[i] = (unsigned char)cinfo.colormap[1][i];
		b[i] = (unsigned char)cinfo.colormap[2][i];
	    }
	} else {
	    for (i=0; i < cinfo.actual_number_of_colors; i++) {
		r[i] = g[i] = b[i] = (unsigned char)cinfo.colormap[0][i];
	    }
	}

	/* clean up and done */
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	return (retBuffer);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: jpeg2pm.c,v $ $Date: 2004/01/10 05:33:23 $ $Revision: 1.3 $ $Name:  $"};

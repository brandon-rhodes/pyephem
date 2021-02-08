/* code to control image processing operations.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/ArrowB.h>
#include <Xm/PushB.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>


#include "xephem.h"
#include "fsmatch.h"

void si_off (void);

static Pixel gray_pixel (int fp);
static Widget addPB (Widget rc_w, char *label, int align);
static Widget addTF (Widget rc_w, char *name, int rw);
static void addLabel (Widget rc_w, char *label, int align);
static void build_colormap (void);
static void build_histo (void);
static void chooseMag (int sw, int sh, int lr, int tb, ZM_Undo *zp, int nz);
static void glassSetup (void);
static void glassSize (void);
static void si_ne (void);
static void si_createdialog (void);
static void si_createwcsdialog (void);
static void sectionCtrl (Widget par_w, char *hlptag, Widget *tb,
    char *tbname, char *tblabel, char *tbtip);
static void si_wcsuse_cb (Widget w, XtPointer client, XtPointer call);
static void si_markstars_cb (Widget w, XtPointer client, XtPointer call);
static void si_lohi_cb (Widget w, XtPointer client, XtPointer call);
static void si_wcsgo_cb (Widget w, XtPointer client, XtPointer call);
static void si_wcsclose_cb (Widget w, XtPointer client, XtPointer call);
static void si_close_cb (Widget w, XtPointer client, XtPointer call);
static void si_exp_cb (Widget w, XtPointer client, XtPointer call);
static void si_drawHistogram (void);
static void si_gamma_cb (Widget w, XtPointer client, XtPointer call);
static void si_help_cb (Widget w, XtPointer client, XtPointer call);
static void si_wcshelp_cb (Widget w, XtPointer client, XtPointer call);
static void si_inv_cb (Widget w, XtPointer client, XtPointer call);
static void si_contrast_cb (Widget w, XtPointer client, XtPointer call);
static void si_markrefstar_cb (Widget w, XtPointer client, XtPointer call);
static void si_newrefstar_cb (Widget w, XtPointer client, XtPointer call);
static void si_newref_cb (Widget w, XtPointer client, XtPointer call);
static void si_managetb_cb (Widget w, XtPointer client, XtPointer call);
static void si_newImage (char *name, int autocon);
static void printGlassStats (ImRegion *rp, double cimx, double cimy);
static void printImageStats (char *name);
static void drawSlice (int x0, int y0, int ww, int wh, int x1, int y1,
    int lr, int tb);
static void drawGraphGrid (Display *dsp, Window win, int ww, int wh,
    double xmin, double max, double ymin, double ymax);
static void drawGlassRow (Widget w, ImRegion *rp, int rx, int rw);
static void drawGlassCol (Widget w, ImRegion *rp, int ry, int rh);
static void defContrast (void);
static void si_wide (void);
static void si_narrow (void);
static void si_full (void);
static void si_duller (void);
static void si_sharper (void);
static void si_brighter (void);
static void si_darker (void);
static void si_mcontrast (void);
static void si_motion_eh (Widget w, XtPointer client, XEvent *ev,
    Boolean *continue_to_dispatch);
static void mkGCs (void);
static void wcsMatch (void);
static int initSolverFields (void);
static int glimsz (void);
static void findWholeImageStats (void);
static void matchStats (ObjF *fsp, double *sx, double *sy, int nfs);
static void imPhotom (double ix, double iy);

static void makeGlassImage (void);
static void fillGlass (int wx, int wy);

static Widget si_w;		/* main imaging tools dialog */
static Widget wcs_w;		/* main wcs dialog */
static Widget fda_w;		/* Drawing area for the contrast map */
static Widget sda_w;		/* Drawing area for slice cross sections */
static Widget glrda_w;		/* Drawing area for glass row plot */
static Widget glcda_w;		/* Drawing area for glass column plot */
static Widget gamma_w;		/* gamma scale */
static Widget ctb_w;		/* TB for contrast section */
static Widget ptb_w;		/* TB for gaussian section */
static Widget gtb_w;		/* TB for glass Stats section */
static Widget stb_w;		/* TB for Slice section */
static Widget rtb_w;		/* TB for ROI section */
static Widget ostxt_w;		/* R/O Text for overall image stats */
static Widget phtxt_w;		/* R/O Text for photometric stats */
static Widget phmagref_w;	/* R/W TF for photometric magnitude reference */
static Widget glcol_w;		/* TB whether to draw vertical cross section */
static Widget glrow_w;		/* TB whether to draw horizontl cross section */
static Widget lo_w;		/* lower contrast setting */
static Widget hi_w;		/* higher contrast setting */
static Widget rstxt_w;		/* Text for ROI stats */
static Widget burnt_w;		/* TF for specifying burned out pix */
static Widget snr_w;		/* TF for specifying star snr */
static Widget bsep_w;		/* TF for best wcs separation */
static Widget wsep_w;		/* TF for worst wcs separation */
static Widget inv_w;		/* TB for reverse vid */
static Widget slmax_w;		/* TB slice auto contrast */

typedef enum {
    WCS_RA, WCS_DEC, WCS_RSCALE, WCS_USCALE, WCS_ROT,
    WCS_N
} WCSFName;			/* callback code */
typedef struct {
    WCSFName which;		/* field name code to allow for random access */
    char *label;		/* field label */
    char *kwtip;		/* help tip for keyword TF */
    char *vtip;			/* help tip for value TF */
    char *res;			/* instance name of TF with FITS fields */
    Widget kw_w;		/* FITS keyword TF */
    Widget v_w;			/* value TF */
    double v;			/* handy value */
} WCSSeed;
static WCSSeed wcsseed[WCS_N] = {
    {WCS_RA, "RA, H:M:S", "FITS field for Center RA", "Center RA", "RA"},
    {WCS_DEC, "Dec, D:M:S", "FITS field for Center Dec","Center Dec","Dec"},
    {WCS_RSCALE, "°/Pixel right", "FITS field for sky step right",
			    "Sky angle right, degrees per pixel", "StepRight"},
    {WCS_USCALE, "°/Pixel up", "FITS field for sky step up",
			    "Sky angle up, degrees per pixel", "StepUp"},
    {WCS_ROT, "Rotate °EofN", "FITS field for rotation", "Image rotation",
								"Rotation"},
};
static int setWCSField (WCSSeed *wsp, char msg[]);
static int getWCSSeed (WCSSeed *wsp, double *vp);

static FImage fim;		/* current FITS image */
static Pixmap fpm;		/* current pixmap with image */
static int fpmw, fpmh;		/* dimensions of fpm */
static int fimok;		/* 1 if fim is ok, else 0 */
static Pixel *gray;		/* gray-scale ramp for drawing image */
static int ngray;		/* number of pixels usable in gray[] */
static int fdepth;		/* depth of image, in bits */
static int fbpp;		/* bits per pixel in image: 1, 8, 16 or 32 */
static double fmag;		/* screen pix / image pix */
static double fximoff, fyimoff;	/* image offsets that put center on screen */
static int lopix, hipix;	/* current LUT lo and hi image pixel values */
static int *histo;		/* malloced histogram of current image */
static int h_mini, h_maxi;	/* histo[] index to first and last pixel used */
static int h_peaki;		/* index of largest value in histo[] */
static char *colormap;		/* malloced gray[] indeces for current image */
static int want_inv;		/* set when want inverse video effect */
static int nup;			/* set if fim has north up */
static int elf;			/* set if fim has east left */
static ImStats wims;		/* handy stats for whole image */

static Pixel imbg_p;		/* grid background color */
static Pixel img_p;		/* grid color 1 */
static Pixel imc1_p;		/* handy color 1 */
static Pixel imc2_p;		/* handy color 2 */
static Pixel imc3_p;		/* handy color 3 */
static GC imgc;			/* handy GC */

#define	MMH		10	/* map marker height */
#define	MMW		5	/* map marker half-width */
#define	MONUHI		30	/* height of tallest monument, pixels */
#define	GAM_MAX		3.0	/* gamma ranges from 1/GAM_MAX .. GAM_MAX */
#define	DEFGAMMA	75	/* default gamma (in scale's range) */
#define	NTICKS		5	/* rough number of tick marks in plots */
#define	SINDENT		15	/* main section indent */
#define	FOVMORE		1.1	/* size factor over diagonal for wcs fit */
#define	BORDER		8	/* edge to ignore looking for stars */

static XImage *glass_xim;       /* glass XImage -- 0 means new or can't */
static GC glassGC;              /* GC for glass border */
static Widget glstxt_w;		/* Text for glass stats */

typedef struct {
    char *name;			/* TB name for resource */
    char *label;		/* toggle button label */
    int value;			/* numeric value to use when selected */
    Widget w;			/* TB widget in radio box */
} RadioSet;
static RadioSet glmag[] = {
    {"1x", " 1x", 1},
    {"2x", " 2x", 2},
    {"4x", " 4x", 4},
    {"8x", " 8x", 8},
};
static RadioSet glsz[] = {
    {"16x16", " 16", 16},
    {"32x32", " 32", 32},
    {"64x64", " 64", 64},
    {"128x128", "128", 128},
};

static char lopixkw[] = "XELOGLUT";	/* our FITS hw for lopix */
static char hipixkw[] = "XEHIGLUT";	/* our FITS hw for hipix */
static char gammakw[] = "XEGAMMA";	/* our FITS hw for gamma */


#define	MINGLSZ		3	/* anything smaller causes several grief */
static int glassmag;		/* glass mag factor, any integer > 0 */
static int glasssz;		/* glass edge, screen pixels, int X of mag */

static Star newstar;		/* current photometric computation */
static Star refstar;		/* photometric reference */
static int refstarok;		/* set when refstar is valid */
static int newstarok;		/* set when newstar is valid */

static char skyipcategory[] = "Sky View -- Image tools";/* Save category */

/* called to create but not manage the imaging dialog */
void
si_create()
{
	if (!si_w) {
	    si_createdialog();
	    si_createwcsdialog();
	    mkGCs();
	    ngray = gray_ramp (XtD, xe_cm, &gray);
	    fdepth = DefaultDepth (XtD, DefaultScreen(XtD));
	    fbpp = (fdepth == 1 || ngray == 2) ? 1 :
				(fdepth>=17 ? 32 : (fdepth >= 9 ? 16 : 8));
	    glassSetup();		/* insure glassmag/sz set */
	}
}

/* called to bring up the imaging dialog */
void
si_manage()
{
	si_create();
	XtManageChild(si_w);
}

/* called to find whether Imaging window is up */
int
si_isup()
{
	return (isUp (si_w));
}

/* called to unmanage the imaging dialog.
 * N.B. do not try to reclaim memory so we can leave FITS watch on while down.
 */
void
si_unmanage()
{
	if (!si_w)
	    return;
	XtUnmanageChild (si_w);
}

/* return 1 if dialog is up, else 0.
 */
int
si_ismanaged()
{
	return (si_w && XtIsManaged(si_w));
}

/* return 1 if there is currently a valid FITS image available, else 0 */
int
si_ison()
{
	return (fimok);
}

/* discard any current FITS image */
void
si_off()
{
	if (fimok) {
	    resetFImage (&fim);
	    fimok = 0;
	}
	if (fpm) {
	    XFreePixmap (XtD, fpm);
	    fpm = (Pixmap)0;
	}
}

/* set the object at the given image coords as the new reference object,
 * assigned with magnitude mag.
 */
void
si_setPhotomRef (ix, iy, mag)
double ix, iy, mag;
{
	char magstr[32];

	/* install new ref mag number */
	sprintf (magstr, "%.2f", mag);
	XmTextFieldSetString (phmagref_w, magstr);

	/* force new refstar, compute and mark */
	refstarok = 0;
	imPhotom (ix, iy);
	sv_all(NULL);	/* erase */
	sv_drawimdot (ix, iy, 5, 0);
}

/* called when the Gauss toolbar TB is used.
 * N.B. can be called before we are created
 */
void
si_updateGauss (on)
int on;
{
	if (fimok && on)
	    si_manage();
	else
	    si_create();
	XmToggleButtonSetState (ptb_w, on, True);
}

/* called when the Glass toolbar TB is used.
 * N.B. can be called before we are created
 */
void
si_updateGlass (on)
int on;
{
	/* always close, open only if window already up */
	si_create();
	if (!on || si_isup())
	    XmToggleButtonSetState (gtb_w, on, True);
}

/* called when the ROI toolbar TB is used.
 * N.B. can be called before we are created
 */
void
si_updateROI (on)
int on;
{
	/* always close, open only if window already up */
	si_create();
	if (!on || (si_isup() && si_ison()))
	    XmToggleButtonSetState (rtb_w, on, True);
}

/* called when the Contrast toolbar TB is used.
 * N.B. can be called before we are created
 */
void
si_updateContrast (on)
int on;
{
	if (fimok && on)
	    si_manage();
	else
	    si_create();
	XmToggleButtonSetState (ctb_w, on, True);
}

/* called when the Slice toolbar TB is used.
 * N.B. can be called before we are created
 */
void
si_updateSlice (on)
int on;
{
	if (fimok && on)
	    si_manage();
	else
	    si_create();
	XmToggleButtonSetState (stb_w, on, True);
}

/* called to put up or remove the watch cursor.
 * we grab the WCS window while we're at it.
 */
void
si_cursor (c)
Cursor c;
{
	Window win;

	if (si_w && (win = XtWindow(si_w)) != 0) {
	    Display *dsp = XtDisplay(si_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}

	if (wcs_w && (win = XtWindow(wcs_w)) != 0) {
	    Display *dsp = XtDisplay(wcs_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* return pointer to the current FImage.
 * if no current FImage, return NULL.
 */
FImage *
si_getFImage ()
{
	return (fimok ? &fim : NULL);
}

/* set the contrast fields in the given FITS header */
void
si_setContrast (fip)
FImage *fip;
{
	int lpix, hpix;
	int gamma;

	/* lo>hi will imply reverse video */
	if (want_inv) {
	    lpix = hipix;
	    hpix = lopix;
	} else {
	    lpix = lopix;
	    hpix = hipix;
	}

	setIntFITS (fip, lopixkw, lpix, "XEphem: Pixel at low LUT end");
	setIntFITS (fip, hipixkw, hpix, "XEphem: Pixel at hi LUT end");
	XmScaleGetValue (gamma_w, &gamma);
	setRealFITS (fip, gammakw, gamma/100., 4, "XEphem: LUT gamma value");
	sf_showHeader(fip);
}

/* return the current Pixmap */
Pixmap
si_getPixmap ()
{
	return (fpm);
}

/* send the current image to the postscript machine */
void
si_ps ()
{
	XPSPixmap (fpm, fpmw, fpmh, xe_cm, 0, 0);
}

/* build a pixmap, fpm, from the XImage fim with the given size, flipping and
 * zoom inf, using the current colormap.
 */
void
si_newPixmap (w, h, lr, tb, zp, nz)
int w, h;		/* screen size */
int lr, tb;		/* display's flipping */
ZM_Undo *zp;		/* entire current zoom stack */
int nz;			/* items on zoom stack */
{
	Display *dsp = XtDisplay (toplevel_w);
	Window win = XtWindow (toplevel_w);
	GC gc = DefaultGC(dsp, DefaultScreen(dsp));
	CamPix *ip = (CamPix *)fim.image;	/* just handy */
	XImage *f_xim;		/* XImage: adjusted for flipping and lo/hi */
	char *data;		/* pixel data for f_xim */
	int nbytes;		/* total bytes in data[] */
	int x, y;		/* screen coords */
	int wantmonu;		/* whether we want monument mode */
	int dpix;		/* h_maxi - h_mini */

	if (!fimok)
	    return;

	watch_cursor (1);

	/* create a temp XImage in which to build image for pixmap */
	/* get memory for image pixels */
	nbytes = (h+7)*(w+7)*fbpp/8;
	data = (char *) malloc (nbytes);
	if (!data) {
	    xe_msg (1, "Can not get %d bytes for FITS pixels", nbytes);
	    watch_cursor (0);
	    return;
	}

	/* create the XImage */
	f_xim = XCreateImage (dsp, XDefaultVisual (dsp, DefaultScreen(dsp)),
	    /* depth */         fbpp == 1 ? 1 : fdepth,
	    /* format */        fbpp == 1 ? XYBitmap : ZPixmap,
	    /* offset */        0,
	    /* data */          data,
	    /* width */         w,
	    /* height */        h,
	    /* pad */           fbpp < 8 ? 8 : fbpp,
	    /* bpl */           0);
	if (!f_xim) {
	    xe_msg (1, "Can not create shadow XImage");
	    free ((void *)data);
	    watch_cursor (0);
	    return;
	}
        f_xim->bitmap_bit_order = LSBFirst;
	f_xim->byte_order = LSBFirst;

	/* decide magnification and offsets */
	chooseMag (w, h, lr, tb, zp, nz);

	/* fill image via gray map and allow for flipping, centering, monument.
	 * use fast/clean nearest-neighbor interpolation.
	 */
	if (!elf)
	    lr = !lr;
	if (!nup)
	    tb = !tb;
	dpix = h_maxi - h_mini;
	wantmonu = svtb_monumentIsOn() && dpix > 0;
	for (y = 0; y < h; y++) {
	    int flipy = tb ? h-y-1 : y;
	    CamPix *rowp = &ip[fim.sw*((int)(flipy/fmag+fyimoff+.5))];
	    for (x = 0; x < w; x++) {
		int flipx = lr ? w-x-1 : x;
		int p = rowp[(int)(flipx/fmag+fximoff+.5)];
		Pixel xp = gray_pixel (p);
		XPutPixel (f_xim, x, y, xp);
		if (wantmonu) {
		    int z, zmax = (int)(MONUHI*fmag*(p-h_mini)/dpix);
		    for (z = 1; z < zmax && z <= y; z++)
			XPutPixel (f_xim, x, y-z, xp);
		}
	    }
	}

	/* (re)create the pixmap if first or new size */
	if (!fpm || w != fpmw || h != fpmh) {
	    fpm = XCreatePixmap (dsp, win, w, h, fdepth);
	    fpmw = w;
	    fpmh = h;
	}

	/* copy the image to the pixmap */
	XPutImage (dsp, fpm, gc, f_xim, 0, 0, 0, 0, w, h);

	/* finished with the image */
	free ((void *)f_xim->data);
	f_xim->data = NULL;
	XDestroyImage (f_xim);

	watch_cursor (0);
}

/* install fip as the new fim and display.
 * N.B. malloced fields in fip are just re-used, so do not reset it on return.
 * name might include leading path, if so we strip off.
 * last argument determines whether contrast and WCS are set automatically.
 */
void
si_newfim (fip, name, autocon)
FImage *fip;
char *name;
int autocon;
{
	double eq;

	/* we support 8 bit images by converting them to 16 */
	if (fip->bitpix == 8) {
	    /* convert to 16 bits.
	     * thanks to Egil Kvaleberg <egil@kvaleberg.no
	     */
	    int n;
	    int new_bytes = fip->totbytes * 2;
	    unsigned char *new_image = (unsigned char *) realloc (fip->image,
								new_bytes);
	    if (!new_image) {
		xe_msg (1, "Could not realloc %d for pixels", new_bytes);
		resetFImage (fip);
		return;
	    }

	    /* convert from BITPIX 8 to BITPIX 16 */
	    for (n=fip->totbytes-1; n>=0; --n)
		((unsigned short *)new_image)[n] = 256*new_image[n];

	    /* update FITS header to reflect BITPIX 16 */
	    fip->totbytes = new_bytes;
	    fip->image = (char *) new_image;
	    fip->bitpix = 16;
	    setIntFITS (fip, "BITPIX", 16, "Bits per pixel");
	}

	/* EQUINOX is preferred, but we'll accept EPOCH in a pinch */
	if (getRealFITS (fip, "EQUINOX", &eq) < 0) {
	    if (getRealFITS (fip, "EPOCH", &eq) < 0) {
		setRealFITS (fip, "EQUINOX", 2000.0, 10, "Faked");
	    } else {
		setRealFITS (fip, "EQUINOX", eq, 10, "Copied from EPOCH");
	    }
	}

	resetFImage (&fim);
	(void) memcpy ((void *)&fim, (void *)fip, sizeof(fim));
	fimok = 1;

	si_newImage(name, autocon);
}

/* convert FITS x/y (well, 0-based "FITS") to X Windows x/y.
 * this always assumes you want east-left and north-up
 */
void
si_im2win (
double imx, double imy,		/* FITS image coords */
int winw, int winh,		/* X window w and h */
int *winxp, int *winyp)		/* return X window x and y */
{
	*winxp = (int)floor(fmag*(imx-fximoff) + .5);
	*winyp = (int)floor(fmag*(imy-fyimoff) + .5);
	if (!elf)
	    *winxp = winw - 1 - *winxp;
	if (!nup)
	    *winyp = winh - 1 - *winyp;
}

/* convert X Windows x/y to FITS x/y (well, 0-based "FITS").
 * this always assumes you want east-left and north-up
 */
void
si_win2im (
int winx, int winy,			/* X window x and y */
int winw, int winh,			/* X window w and h */
double *imxp, double *imyp)		/* return FITS image coords */
{
	if (!elf)
	    winx = winw - 1 - winx;
	if (!nup)
	    winy = winh - 1 - winy;
	*imxp = winx/fmag+fximoff;
	*imyp = winy/fmag+fyimoff;
}

/* get the given WCS seed value from the GUI.
 * return 0 if ok, else fuss with xe_msg and return -1
 */
static int
getWCSSeed (wsp, vp)
WCSSeed *wsp;
double *vp;
{
	double v;
	char *str;
	int ok;

	/* read TF */
	str = XmTextFieldGetString (wsp->v_w);
	ok = f_scansexa (str, &v);
	XtFree (str);
	if (ok < 0) {
	    xe_msg (1, "Missing or ill-formed format for %s", wsp->label);
	    return (-1);
	}

	/* a few special cases */
	switch (wsp->which) {
	case WCS_RA:
	    *vp = hrrad(v);
	    return(0);
	case WCS_RSCALE:	/* FALLTHRU */
	case WCS_USCALE:
	    if (v == 0) {
		xe_msg (1, "Missing or 0 for %s", wsp->label);
		return (-1);
	    }
	    break;
	default:
	    break;
	}
	*vp = degrad(v);
	return (0);
}

/* set global fmag/fximoff/fyimoff for given window and image portion */
static void
setMag (ww, wh, imx, imy, imw, imh)
int ww, wh;
double imx, imy, imw, imh;
{
	/* set mag and offsets to display desired portion */
	fmag = wh/imh;
	if (fmag*imw < ww) {
	    /* full width, center in height */
	    fmag = ww/imw;
	    fximoff = imx - 0.5;
	    fyimoff = imy + (imh - wh/fmag)/2 - 0.5;
	} else {
	    /* full height, center in width */
	    fximoff = imx + (imw - ww/fmag)/2 - 0.5;
	    fyimoff = imy - 0.5;
	}
}


/* find a magnification and image offsets to center image such that we always
 * fill the screen, centering and cropping if necessary.
 * in order to establish zoom current context, go through all entries in order.
 */
static void
chooseMag (ww, wh, lr, tb, zp, nz)
int ww, wh;	/* window w/h */
int lr, tb;	/* user's notion of what is flipped */
ZM_Undo *zp;	/* zoom info in screen coords, else NULL */
int nz;		/* n zoom entries */
{
	double imx, imy;	/* corner of image to display, im pixels */
	double imw, imh;	/* size of image to display, im pixels */
	int fx, fy;		/* flipped coords */

	/* start with no zoom, then work through stack */
	imw = fim.sw;
	imh = fim.sh;
	imx = 0;
	imy = 0;
	setMag (ww, wh, imx, imy, imw, imh);

	for (; nz--; zp++) {
	    /* display cropped portion */
	    double ix0, iy0;	/* one corner of image */
	    double ix1, iy1;	/* other corner of image */

	    fx = lr ? ww - 1 - zp->x0 : zp->x0;
	    fy = tb ? wh - 1 - zp->y0 : zp->y0;
	    si_win2im (fx, fy, ww, wh, &ix0, &iy0);
	    fx = lr ? ww - 1 - zp->x1 : zp->x1;
	    fy = tb ? wh - 1 - zp->y1 : zp->y1;
	    si_win2im (fx, fy, ww, wh, &ix1, &iy1);
	    imw = fabs(ix1-ix0);
	    imh = fabs(iy1-iy0);
	    imx = ix0 < ix1 ? ix0 : ix1;
	    imy = iy0 < iy1 ? iy0 : iy1;
	    setMag (ww, wh, imx, imy, imw, imh);
	}
}

/* compute the histogram for fim and put it in histo[].
 */
static void
build_histo()
{
	FImage *fip = &fim;
	CamPix *ip = (CamPix *)fip->image;
	int npix = fip->sw*fip->sh;
	int i;

	/* insure memory */
	if (!histo) {
	    histo = (int *) malloc (MAXCAMPIX * sizeof(histo[0]));
	    if (!histo) {
		printf ("No memory for histogram\n");
		abort();
	    }
	}

	/* zero the histogram */
	memset ((void *)histo, 0, MAXCAMPIX*sizeof(histo[0]));

	/* compute histogram */
	for (i = 0; i < npix; i++)
	    histo[(int)(*ip++)]++;

	/* find boundary stats */
	for (h_mini = 0; histo[h_mini] == 0; h_mini++)
	    continue;
	for (h_maxi = MAXCAMPIX; histo[--h_maxi] == 0; )
	    continue;
	if (h_mini == h_maxi) {
	    if (h_mini > 0)
		h_mini -= 1;
	    else 
		h_maxi += 1;
	}
	h_peaki = 0;
	for (i = h_mini; i <= h_maxi; i++)
	    if (histo[i] > histo[h_peaki])
		h_peaki = i;
}

/* compute the colormap for fim and put in colormap, allowing for method
 * and lo/hi.
 * N.B. if we need the histogram, we assume it's already built.
 */
static void
build_colormap()
{
	int range = hipix - lopix;
	int g, i;

	/* insure memory */
	if (!colormap) {
	    colormap = malloc (MAXCAMPIX*sizeof(colormap[0]));
	    if (!colormap) {
		printf ("No memory for colormap\n");
		abort();
	    }
	}

	/* build ramp for current gamma setting */
	for (i = 0; i < lopix; i++)
	    colormap[i] = 0;
	XmScaleGetValue (gamma_w, &g);
	for (; i < hipix; i++)
	    colormap[i] = (int)(pow((double)(i-lopix)/range, g/100.0)*ngray);
	for (; i < MAXCAMPIX; i++)
	    colormap[i] = ngray-1;
}

/* given a FITS pixel, return an X pixel */
static Pixel
gray_pixel (fp)
int fp;
{
	int gp = (int)colormap[fp];

	if (want_inv)
	    gp = (ngray-1) - gp;

	return (gray[gp]);

}

/* walk gradient to brightest pixel.
 * no use using true max because of errors introduced changing from win, to im
 * to win to im coords (here and in, say, computing the glass stats).
 */
void
si_findSnap (
int ww, int wh,		/* size of X Window */
int wx, int wy,		/* X window coords of cursor */
int lr, int tb,		/* user's flip settings */
int *sxp, int *syp)	/* snap coords */
{
	ImRegion imr;
	double ix, iy;
	int fx, fy;
	int bx, by;

	/* null case if no image */
	if (!fimok) {
	    *sxp = wx;
	    *syp = wy;
	    return;
	}

	/* find image coord of cursor */
	fx = lr ? ww - 1 - wx : wx;
	fy = tb ? wh - 1 - wy : wy;
	si_win2im (fx, fy, ww, wh, &ix, &iy);

	/* walk up the hill */
	imr.im = (CamPix *) fim.image;
	imr.iw = fim.sw;
	imr.ih = fim.sh;
	imr.rx = (int)(ix + .5);
	imr.ry = (int)(iy + .5);
	imr.rw = 1;
	imr.rh = 1;
	brightWalk (&imr, NULL, &bx, &by);

	/* turn back into X cursor coords */
	si_im2win ((double)bx, (double)by, ww, wh, &fx, &fy);
	*sxp = lr ? ww - 1 - fx : fx;
	*syp = tb ? wh - 1 - fy : fy;
}

/* handle the operation of the magnifying glass.
 */
void
si_doGlass (
Display *dsp,
Window win,
int b1p, int m1,	/* button/motion state */
int ww, int wh,		/* size of X Window */
int wx, int wy,		/* true X window coords of cursor */
int sx, int sy,		/* snap-to-max X window coords */
int lr, int tb)		/* user's flip settings */
{
	static int lastsx, lastsy;	/* last windos x/y */
	int rx, ry, rw, rh;		/* region */

	if (!fimok)
	    return;

	if (b1p)
	    glassSetup();

	if (m1) {

	    /* motion: put back old pixels that won't just be covered again */

	    /* first the vertical strip that is uncovered */

	    rh = glasssz;
	    ry = lastsy - (glasssz/2);
	    if (ry < 0) {
		rh += ry;
		ry = 0;
	    }
	    if (sx < lastsx) {
		rw = lastsx - sx;	/* cursor moved left */
		rx = sx + (glasssz/2);
	    } else {
		rw = sx - lastsx;	/* cursor moved right */
		rx = lastsx - (glasssz/2);
	    }
	    if (rx < 0) {
		rw += rx;
		rx = 0;
	    }

	    if (rw > 0 && rh > 0)
		XCopyArea (dsp, fpm, win, glassGC, rx, ry, rw, rh, rx, ry);

	    /* then the horizontal strip that is uncovered */

	    rw = glasssz;
	    rx = lastsx - (glasssz/2);
	    if (rx < 0) {
		rw += rx;
		rx = 0;
	    }
	    if (sy < lastsy) {
		rh = lastsy - sy;	/* cursor moved up */
		ry = sy + (glasssz/2);
	    } else {
		rh = sy - lastsy;	/* cursor moved dosn */
		ry = lastsy - (glasssz/2);
	    }
	    if (ry < 0) {
		rh += ry;
		ry = 0;
	    }

	    if (rw > 0 && rh > 0)
		XCopyArea (dsp, fpm, win, glassGC, rx, ry, rw, rh, rx, ry);
	}

	if (b1p || m1) {

	    /* start or new location: show glass and save new location */

	    fillGlass (sx, sy);
	    XPutImage (dsp, win, glassGC, glass_xim, 0, 0,
			sx-(glasssz/2), sy-(glasssz/2),
			glasssz, glasssz);
	    lastsx = sx;
	    lastsy = sy;

	    /* kinda hard to tell boundry of glass so draw a line around it */
	    XDrawRectangle (dsp, win, glassGC, sx-(glasssz/2), sy-(glasssz/2),
							glasssz-1, glasssz-1);

	    /* show stats and graphs as desired */
	    if (svtb_glassIsOn() && gtb_w && XmToggleButtonGetState(gtb_w)) {
		int fx = lr ? ww - 1 - sx : sx;
		int fy = tb ? wh - 1 - sy : sy;
		int gsz = glimsz();
		ImRegion imr;
		int cx, cy;
		double gix, giy;

		/* find glass region */
		si_win2im (fx, fy, ww, wh, &gix, &giy);
		cx = (int)(gix + .5);
		cy = (int)(giy + .5);
		imr.im = (CamPix *) fim.image;
		imr.iw = fim.sw;
		imr.ih = fim.sh;
		imr.rx = cx - gsz/2;
		imr.ry = cy - gsz/2;
		imr.rw = gsz;
		imr.rh = gsz;
		if (clampRegion (&imr) < 0)
		    return;

		/* print stats */
		printGlassStats(&imr, gix, giy);

		/* plot desired crosssection through center of glass */
		if (XmToggleButtonGetState (glrow_w))
		    drawGlassRow (glrda_w, &imr, cx - gsz/2, gsz);
		if (XmToggleButtonGetState (glcol_w))
		    drawGlassCol (glcda_w, &imr, cy - gsz/2, gsz);
	    }
	}
}

/* print stats for glass given image location of center */
static void
printGlassStats(rp, ix, iy)
ImRegion *rp;
double ix, iy;
{
	ImStats ims;
	int cx, cy;
	char buf[1024];

	regionStats (rp, &ims);

	cx = (int)(ix + .5);
	cy = (int)(iy + .5);
	sprintf (buf, "        %3dW x %dH\nCenter: %5d @:%6.1f,%6.1f\n   Max: %5d @:%4d.0,%4d.0\n   Min: %5d  Mean: %8.1f\nMedian: %5d StDev: %8.1f",
	    rp->rw, rp->rh,
	    rp->im[cy*rp->iw + cx], ix, iy,
	    ims.max, ims.maxatx, ims.maxaty,
	    ims.min, ims.mean,
	    ims.median, ims.std);
	XmTextSetString (glstxt_w, buf);
}

/* plot central row of region rp.
 * rp is already clamped, rx and rw are the original rp->rx and rp->rw.
 * N.B. may be called before w is realized, eg, glass row set in app defaults.
 */
static void
drawGlassRow (w, rp, rx, rw)
Widget w;
ImRegion *rp;
int rx, rw;
{
	Display *dsp = XtDisplay (w);
	Window win = XtWindow(w);
	XPoint xpcache[256];
	XPoint *xps, *xp;
	Dimension daw, dah;
	int divs = rw-1;		/* fence posts -> rails */
	double bot, top;
	CamPix *im;
	int i;

	/* benign if called before being realized */
	if (!win)
	    return;

	/* use stack and avoid malloc if possible */
	xps = 2*rw<=XtNumber(xpcache) ? xpcache
				      : (XPoint*)XtMalloc(2*rw*sizeof(XPoint));
	xp = xps;

	/* get size of drawing area */
	get_something (w, XmNwidth, (XtArgVal)&daw);
	get_something (w, XmNheight, (XtArgVal)&dah);

	/* draw background box and labels first */
	bot = h_mini;
	top = h_maxi;
	drawGraphGrid (dsp, win, daw, dah, (double)rx, (double)rx+divs,bot,top);

	/* pixels */
	XSetForeground (dsp, imgc, imc1_p);
	im = &rp->im[(rp->ry+rp->rh/2)*rp->iw + rx];
	for (i = 0; i < rw; i++) {
	    if (i + rx >= 0 && rx + i < rp->iw) {
		CamPix p = im[i];
		xp->x = i*(int)daw/divs - (int)daw/divs/2;	/* center */
		xp->y = (short)((int)dah - (int)dah*(p-bot)/(top-bot));
		xp++;
		xp->x = (i+1)*(int)daw/divs - (int)daw/divs/2; /* exact match */
		xp->y = xp[-1].y;
		xp++;
	    }
	}
	XDrawLines (dsp, win, imgc, xps, xp-xps, CoordModeOrigin);

	if (xps != xpcache)
	    XtFree ((char *)xps);
}

/* plot central column of region rp.
 * rp is already clamped, ry and rh are the original rp->ry and rp->rh.
 * N.B. may be called before w is realized, eg, glass row set in app defaults.
 */
static void
drawGlassCol (w, rp, ry, rh)
Widget w;
ImRegion *rp;
int ry, rh;
{
	Display *dsp = XtDisplay (w);
	Window win = XtWindow(w);
	XPoint xpcache[256];
	XPoint *xps, *xp;
	Dimension daw, dah;
	int divs = rh-1;
	double bot, top;
	CamPix *im;
	int i;

	/* benign if called before being realized */
	if (!win)
	    return;

	/* use stack and avoid malloc if possible */
	xps = 2*rh<=XtNumber(xpcache) ? xpcache
				      : (XPoint*)XtMalloc(2*rh*sizeof(XPoint));
	xp = xps;

	/* get size of drawing area */
	get_something (w, XmNwidth, (XtArgVal)&daw);
	get_something (w, XmNheight, (XtArgVal)&dah);

	/* draw background box and labels first */
	bot = h_mini;
	top = h_maxi;
	drawGraphGrid (dsp, win, daw, dah, (double)ry, (double)ry+divs,bot,top);

	/* pixels */
	XSetForeground (dsp, imgc, imc1_p);
	im = &rp->im[rp->ry*rp->iw + rp->rx + rp->rw/2];
	for (i = 0; i < rh; i++) {
	    if (i + ry >= 0 && ry + i < rp->ih) {
		CamPix p = im[i*rp->iw];
		xp->x = i*(int)daw/divs - (int)daw/divs/2;	/* center */
		xp->y = (short)((int)dah - (int)dah*(p-bot)/(top-bot));
		xp++;
		xp->x = (i+1)*(int)daw/divs - (int)daw/divs/2; /* exact match */
		xp->y = xp[-1].y;
		xp++;
	    }
	}
	XDrawLines (dsp, win, imgc, xps, xp-xps, CoordModeOrigin);

	if (xps != xpcache)
	    XtFree ((char *)xps);
}

/* print overall image stats */
static void
printImageStats(name)
char *name;
{
	char buf[1024];
	char center[1024];
	char *base;
	short columns;
	int centerl;

	/* strip any leading path portion */
	if ((base = strrchr (name, '/')) || (base = strrchr (name, '\\')))
	    name = base+1;

	/* prepare to center name and size */
	centerl = sprintf (center, "%s %dW x %dH", name, fim.sw, fim.sh);
	get_something (ostxt_w, XmNcolumns, (XtArgVal)&columns);

	sprintf (buf, "%*s%s\n   Max: %5d    at:%4d,%4d\n   Min: %5d  Mean: %8.1f\nMedian: %5d StDev: %8.1f",
	    (columns-centerl)/2, "", center,
	    wims.max, wims.maxatx, wims.maxaty,
	    wims.min, wims.mean,
	    wims.median, wims.std);

	XmTextSetString (ostxt_w, buf);
}

/* compute wims based on fim */
static void
findWholeImageStats()
{
	ImRegion imr;

	imr.im = (CamPix *) fim.image;
	imr.iw = fim.sw;
	imr.ih = fim.sh;
	imr.rx = BORDER;
	imr.ry = BORDER;
	imr.rw = fim.sw - 2*BORDER;
	imr.rh = fim.sh - 2*BORDER;
	regionStats (&imr, &wims);
}

/* handle the operation of the cross-sectional slice.
 */
void
si_doSlice (
Display *dsp,
Window win,
int state,	/* -1: new 0: erase 1: draw 2: void next erase */
int ww, int wh,	/* size of image drawing area */
int wx, int wy,	/* cursor coords on drawing area */
int lr, int tb)	/* user's flip settings */
{
	static int startwx, startwy;	/* session starting point */
	static int lastwx, lastwy;	/* last window endpoint x/y */
	static GC xogc;			/* GC for XORing slice */

	if (!fimok)
	    return;
	if (!xogc) {
	    unsigned long mask;
	    XGCValues gcv;

	    mask = GCFunction | GCForeground;
	    gcv.function = GXxor;
	    gcv.foreground = imc2_p ^ BlackPixel(dsp,DefaultScreen(dsp));
	    xogc = XCreateGC (dsp, win, mask, &gcv);
	}

	switch (state) {
	case -1:
	    /* new starting pos */
	    startwx = lastwx = wx;
	    startwy = lastwy = wy;
	    break;

	case 0:
	    XDrawLine (dsp, win, xogc, startwx, startwy, lastwx, lastwy);
	    break;

	case 1:
	    XDrawLine (dsp, win, xogc, startwx, startwy, wx, wy);
	    lastwx = wx;
	    lastwy = wy;
	    if (si_ismanaged())
		drawSlice (startwx, startwy, ww, wh, wx, wy, lr, tb);
	    break;

	case 2:
	    lastwx = startwx;
	    lastwy = startwy;
	    break;

	default:
	    printf ("bad doSlice state: %d\n", state);
	    abort();
	    break;
	}
}

/* draw pixels along line from [x0,y0] to [x1,y1] in window of size wwXwh */
static void
drawSlice (x0, y0, ww, wh, x1, y1, lr, tb)
int x0, y0, ww, wh, x1, y1, lr, tb;
{
	Display *dsp = XtDisplay (sda_w);
	Window win = XtWindow (sda_w);
	CamPix *ip = (CamPix *)fim.image;	/* just handy */
	Dimension sdaw, sdah;			/* sda size */
	XPoint xpcache[1024], *xp;
	int l, h;
	double len;
	int sdax;

	/* length if slice */
	len = sqrt(sqr((double)(x1-x0))+sqr((double)(y1-y0)))/fmag;
	if (len == 0)
	    return;

	/* get size of drawing area */
	get_something (sda_w, XmNwidth, (XtArgVal)&sdaw);
	get_something (sda_w, XmNheight, (XtArgVal)&sdah);

	/* use stack if possible to avoid malloc */
	xp = sdaw <= XtNumber(xpcache) ? xpcache
				       : (XPoint*)XtMalloc(sdaw*sizeof(XPoint));

	/* account for user's flipping */
	if (lr) {
	    x0 = ww - 1 - x0;
	    x1 = ww - 1 - x1;
	}
	if (tb) {
	    y0 = wh - 1 - y0;
	    y1 = wh - 1 - y1;
	}

	/* range */
	if (XmToggleButtonGetState (slmax_w)) {
	    h = h_maxi;
	    l = h_mini;
	} else {
	    h = hipix;
	    l = lopix;
	}

	/* walk along */
	for (sdax = 0; sdax < sdaw; sdax++) {
	    double ix, iy;
	    int p, wx, wy;
	    wx = x0 + sdax*(x1-x0)/(int)sdaw;
	    wy = y0 + sdax*(y1-y0)/(int)sdaw;
	    si_win2im (wx, wy, ww, wh, &ix, &iy);
	    p = ip[(int)(fim.sw*(int)(iy + .5) + ix + .5)];
	    xp[sdax].x = sdax;
	    xp[sdax].y = sdah-sdah*(p-l)/(h-l); /* +y is down */
	}

	/* grid */
	XSetForeground (dsp, imgc, img_p);
	drawGraphGrid (dsp, win, sdaw, sdah, 0.0, len, (double)l, (double)h);

	/* graph */
	XSetForeground (dsp, imgc, imc1_p);
	XDrawLines (dsp, win, imgc, xp, sdaw, CoordModeOrigin);

	if (xp != xpcache)
	    XtFree ((char *)xp);
}

/* handle the operation of gaussian measurement.
 */
void
si_doGauss (
Display *dsp,
int ww, int wh,	/* size of X Window */
int wx, int wy,	/* X window coords of cursor */
int lr, int tb)	/* user's flip settings */
{

	int fx, fy;			/* flipped X coords */
	double ix, iy;			/* image coords under cursor */

	/* find image coords under cursor */
	fx = lr ? ww - 1 - wx : wx;
	fy = tb ? wh - 1 - wy : wy;
	si_win2im (fx, fy, ww, wh, &ix, &iy);
	imPhotom (ix, iy);
}

/* compute and display photometry of star in glass-size region around image
 * coords [ix,iy].
 * N.B. must be called after doGlass for gaussian overlays to work.
 */
static void
imPhotom (ix, iy)
double ix, iy;
{
	Display *dsp = XtD;
	char buf[256], rastr[32], decstr[32], vfwhmstr[32], hfwhmstr[32];
	char magstr[64];
	ImRegion imr;			/* region to gauss fit */
	int len;			/* size of region to fit */
	int back;			/* handy len/2 */
	double ra, dec;			/* ra/dec of gaussian peak */
	double mag, merr;		/* magnitude and error */
	double vfwhm, hfwhm;		/* v and h fwhm, in pixels */
	int x, y;			/* integral image coords of peak */

	/* just paranoid */
	if (!fimok)
	    return;

	/* get star here */
	x = (int)(ix + .5);
	y = (int)(iy + .5);
	len = glimsz();
	back = len/2;
	imr.im = (CamPix *) fim.image;
	imr.iw = fim.sw;
	imr.ih = fim.sh;
	imr.rx = x - back;
	imr.ry = y - back;
	imr.rw = len;
	imr.rh = len;
	if (clampRegion (&imr) < 0)
	    return;
	if (getStar (&imr, &newstar) < 0) {
	    /* blank out if trouble */
	    if (XmToggleButtonGetState (glrow_w))
		XClearWindow (dsp, XtWindow(glrda_w));
	    if (XmToggleButtonGetState (glcol_w))
		XClearWindow (dsp, XtWindow(glcda_w));
	    XmTextSetString (phtxt_w, "");
	    return;
	}
	newstarok = 1;
	sv_drawimdot (newstar.x, newstar.y, 5, 1);

	/* find magnitude compared to reference.
	 * if no current reference, then use this one.
	 */
	if (!refstarok) {
	    refstar = newstar;
	    refstarok = 1;
	}
	if (cmpStars (&refstar, &newstar, &mag, &merr) < 0) {
	    magstr[0] = '\0';
	} else {
	    /* add user's reference value */
	    char *txt = XmTextFieldGetString (phmagref_w);
	    mag += atof (txt);
	    XtFree (txt);
	    sprintf (magstr, "%6.2f ±%6.2f", mag, merr);
	}

	/* print stats of peak */
	hfwhm = newstar.hg.s*FWHMSIG;
	vfwhm = newstar.vg.s*FWHMSIG;
	if (xy2RADec (&fim, newstar.x, newstar.y, &ra, &dec) == 0) {
	    fs_sexa (rastr, radhr(ra), 2, 360000);
	    fs_sexa (decstr, raddeg(dec), 3, 36000);
	    sprintf (hfwhmstr, "%5.1f\"", fabs(hfwhm*fim.xinc*3600.));
	    sprintf (vfwhmstr, "%5.1f\"", fabs(vfwhm*fim.yinc*3600.));
	} else {
	    hfwhmstr[0] = vfwhmstr[0] = decstr[0] = '\0';
	    strcpy (rastr, " (No WCS)");
	}
	sprintf (buf, "Mag: %s\n  X: %6.1f  RA:  %s\n  Y: %6.1f Dec: %s\nXFWHM:%4.1fp%6s Base:%6.0f\nYFWHM:%4.1fp%6s Peak:%6.0f",
		magstr,
		newstar.x, rastr,
		newstar.y, decstr,
		hfwhm, hfwhmstr, newstar.hg.B,
		vfwhm, vfwhmstr, newstar.hg.B+newstar.hg.A);

	XmTextSetString (phtxt_w, buf);
}

/* draw stats for current zoom.
 */
void
si_doROI (
Display *dsp,
int ww, int wh,	/* size of X Window */
int lr, int tb,	/* user's flip settings */
ZM_Undo *zp)
{
	int fx, fy;			/* flipped screen coords */
	double ix, iy;			/* image coords */
	ImRegion imr;			/* ROI region */
	int cx, cy;			/* cursor image coords */
	char buf[1024];			/* info */

	/* quiet noop w/o and image */
	if (!fimok)
	    return;

	/* start defining region */
	imr.im = (CamPix *) fim.image;
	imr.iw = fim.sw;
	imr.ih = fim.sh;

	/* find image coords at each corner */
	fx = lr ? ww - 1 - zp->x0 : zp->x0;
	fy = tb ? wh - 1 - zp->y0 : zp->y0;
	si_win2im (fx, fy, ww, wh, &ix, &iy);
	imr.rx = (int)(ix + .5);
	imr.ry = (int)(iy + .5);
	fx = lr ? ww - 1 - zp->x1 : zp->x1;
	fy = tb ? wh - 1 - zp->y1 : zp->y1;
	si_win2im (fx, fy, ww, wh, &ix, &iy);
	cx = (int)(ix + .5);
	cy = (int)(iy + .5);
	imr.rw = cx - imr.rx;
	imr.rh = cy - imr.ry;

	/* straight if necessary */
	if (imr.rw < 0) {
	    imr.rw = -imr.rw;
	    imr.rx -= imr.rw;
	}
	if (imr.rh < 0) {
	    imr.rh = -imr.rh;
	    imr.ry -= imr.rh;
	}

	/* get stats .. need more than 1 pixel to work with */
	if (imr.rw * imr.rh >= 2) {
	    ImStats ims;

	    regionStats (&imr, &ims);
	    sprintf (buf, "  Diag:%6.1f   WxH:%4d,%4d\nCursor: %5d    at:%4d,%4d\n   Max: %5d    at:%4d,%4d\n   Min: %5d  Mean: %8.1f\nMedian: %5d StDev: %8.1f",
		sqrt(sqr((double)imr.rw) + sqr((double)imr.rh)),
		imr.rw, imr.rh, 
		imr.im[cy*imr.iw + cx], cx, cy,
		ims.max, ims.maxatx, ims.maxaty,
		ims.min, ims.mean,
		ims.median, ims.std);
	} else
	    buf[0] = '\0';

	/* display */
	XmTextSetString (rstxt_w, buf);
}

/* draw a nice labeled graph in the given window */
static void
drawGraphGrid (dsp, win, ww, wh, xmin, xmax, ymin, ymax)
Display *dsp;
Window win;
int ww, wh;		/* size of win */
double xmin, xmax;	/* x range */
double ymin, ymax;	/* y range */
{
	double ticks[NTICKS+2];		/* see tickmarks() */
	XSegment xs[2*(NTICKS+2)];	/* room for both */
	int nticks, nxs;
	char buf[32];
	int i, l, x, y;

	/* erase to background */
	XSetForeground (dsp, imgc, imbg_p);
	XFillRectangle (dsp, win, imgc, 0, 0, ww, wh);

	/* init one set of segments for both directions */
	nxs = 0;

	/* horizontal */
	XSetForeground (dsp, imgc, img_p);
	nticks = tickmarks (ymin, ymax, NTICKS, ticks);
	for (i = 0; i < nticks; i++) {
	    y = wh - (int)(wh*(ticks[i]-ymin)/(ymax-ymin));
	    if (y < 0 || y > wh)
		continue;
	    xs[nxs].x1 = 0;
	    xs[nxs].y1 = y;
	    xs[nxs].x2 = ww;
	    xs[nxs].y2 = y;
	    nxs++;
	    l = sprintf (buf, "%.0f", ticks[i]);
	    XDrawString (dsp, win, imgc, 2, y-1, buf, l);
	}

	/* vertical */
	nticks = tickmarks (xmin, xmax, NTICKS, ticks);
	for (i = 0; i < nticks; i++) {
	    x = (int)(ww*(ticks[i]-xmin)/(xmax-xmin));
	    if (x < 0 || x > ww)
		continue;
	    xs[nxs].x1 = x;
	    xs[nxs].y1 = 0;
	    xs[nxs].x2 = x;
	    xs[nxs].y2 = wh;
	    nxs++;
	    l = sprintf (buf, "%.0f", ticks[i]);
	    XDrawString (dsp, win, imgc, x+1, wh-1, buf, l);
	}

	/* draw grid in one blast */
	if (nxs > XtNumber(xs)) {
	    printf ("drawGraphGrid stack corrupted: %d\n", nxs);
	    abort();
	}
	XDrawSegments (dsp, win, imgc, xs, nxs);
}

/* make some GCs and get some pixels we use in several places */
static void
mkGCs()
{
	Display *dsp = XtD;
	Widget tlw = toplevel_w;
	Window win = RootWindow(dsp, DefaultScreen(dsp));
	Pixel p;

	imgc = XCreateGC (dsp, win, 0L, NULL);
	if (get_color_resource (tlw, "GraphBGColor", &imbg_p) < 0)
	    imbg_p = BlackPixel(dsp,DefaultScreen(dsp));
	if (get_color_resource (tlw, "ImGridColor", &img_p) < 0)
	    img_p = WhitePixel(dsp,DefaultScreen(dsp));
	if (get_color_resource (tlw, "ImPlotColor1", &imc1_p) < 0)
	    imc1_p = WhitePixel(dsp,DefaultScreen(dsp));
	if (get_color_resource (tlw, "ImPlotColor2", &imc2_p) < 0)
	    imc2_p = WhitePixel(dsp,DefaultScreen(dsp));
	if (get_color_resource (tlw, "ImPlotColor3", &imc3_p) < 0)
	    imc3_p = WhitePixel(dsp,DefaultScreen(dsp));

	if (get_color_resource (tlw, "GlassBorderColor", &p) < 0)
	    p = WhitePixel (dsp, 0);
	glassGC = XCreateGC (dsp, win, 0L, NULL);
	XSetForeground (dsp, glassGC, p);
}

/* pick a good initial contrast for the new fim.
 * use our keywords if present and reasonable.
 */
static void
defContrast()
{
	int lpix, hpix;
	double gamma;

	if (!getIntFITS (&fim, lopixkw, &lpix) && lpix >= h_mini &&
		!getIntFITS (&fim, hipixkw, &hpix) && hpix <= h_maxi &&
		!getRealFITS (&fim, gammakw, &gamma) && gamma >= 1./GAM_MAX
							&& gamma <= GAM_MAX) {
	    if (lpix > hpix) {
		/* implies inverse video */
		lopix = hpix;
		hipix = lpix;
		want_inv = 1;
	    } else {
		/* normal video */
		lopix = lpix;
		hipix = hpix;
		want_inv = 0;
	    }
	    XmToggleButtonSetState (inv_w, want_inv, False);

	    XmScaleSetValue (gamma_w, (int)(gamma*100));
	} else {
	    si_wide();
	}
}

/* set up lopix and hipix and a resonable gamma for medium contrast setting */
static void
si_mcontrast()
{
	lopix = h_peaki - (h_peaki - h_mini)/2;
	hipix = h_peaki + (h_maxi - h_peaki)/2;
	XmScaleSetValue (gamma_w, DEFGAMMA);
}

/* increase contrast */
static void
si_sharper()
{
	lopix += (h_peaki-lopix)/3;
	hipix -= (hipix-h_peaki)/3;
}

/* decrease contrast */
static void
si_duller()
{
	lopix = (3*lopix - h_peaki)/(3-1);
	if (lopix < h_mini)
	    lopix = h_mini;

	hipix = (3*hipix - h_peaki)/(3-1);
	if (hipix > h_maxi)
	    hipix = h_maxi;

	if (lopix == h_mini && hipix == h_maxi) {
	    int g;
	    XmScaleGetValue (gamma_w, &g);
	    g -= 10;
	    if (g >= (int)(1./GAM_MAX*100))
		XmScaleSetValue (gamma_w, g);
	}
}

/* set to narrow statistical contrast */
static void
si_narrow()
{
	lopix = (int)(wims.mean - wims.std/3);
	if (lopix < h_mini)
	    lopix = h_mini;
	hipix = (int)(wims.mean + wims.std/3);
	if (hipix > h_maxi)
	    hipix = h_maxi;
	XmScaleSetValue (gamma_w, DEFGAMMA);
}

/* set to wide statistical contrast */
static void
si_wide()
{
	lopix = (int)(wims.mean - wims.std);
	if (lopix < h_mini)
	    lopix = h_mini;
	hipix = (int)(wims.mean + 2*wims.std);
	if (hipix > h_maxi)
	    hipix = h_maxi;
	XmScaleSetValue (gamma_w, DEFGAMMA);
}

/* set to full pixel contrast */
static void
si_full()
{
	lopix = h_mini;
	hipix = h_maxi;
	XmScaleSetValue (gamma_w, DEFGAMMA);
}

/* increase brightness */
static void
si_brighter()
{
	lopix = (3*lopix - h_peaki)/(3-1);
	if (lopix < h_mini)
	    lopix = h_mini;
	hipix -= (hipix-h_peaki)/3;
}

/* decrease brightness */
static void
si_darker()
{
	lopix += (h_peaki-lopix)/3;
	hipix = (3*hipix - h_peaki)/(3-1);
	if (hipix > h_maxi) {
	    int g;
	    hipix = h_maxi;
	    XmScaleGetValue (gamma_w, &g);
	    g += 10;
	    if (g < (int)(GAM_MAX*100))
		XmScaleSetValue (gamma_w, g);
	}
}

/* add a new section control */
static void
sectionCtrl (Widget par_w, char *hlptag, Widget *tb, char *tbname,
    char *tblabel, char *tbtip)
{
	Arg args[20];
	Widget w, tb_w, f_w;
	int n;

	n = 0;
	XtSetArg (args[n], XmNspacing, 10); n++;
	f_w = XmCreateForm (par_w, "SF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    tb_w = XmCreateToggleButton (f_w, tbname, args, n);
	    sr_reg (tb_w, NULL, skyipcategory, 0);
	    set_xmstring (tb_w, XmNlabelString, tblabel);
	    wtip (tb_w, tbtip);
	    XtManageChild (tb_w);
	    *tb = tb_w;

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    w = XmCreatePushButton (f_w, "Help", args, n);
	    XtAddCallback (w, XmNactivateCallback, si_help_cb, hlptag);
	    wtip (w, "Get detailed help on this section");
	    XtManageChild (w);
}

/* create the imaging dialog */
static void
si_createdialog()
{
	Widget mrc_w, rc_w, rc2_w;
	Widget f_w;
	Widget ml_w, fr_w;
	Widget rb_w;
	Widget w;
	Arg args[20];
	long mask;
	int i;
	int n;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNallowResize, True); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	si_w = XmCreateFormDialog (svshell_w, "SkyIP", args, n);
	set_something (si_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (si_w, XmNhelpCallback, si_help_cb, "SkyIP");
	sr_reg (XtParent(si_w), "XEphem*SkyIP.x", skyipcategory, 0);
	sr_reg (XtParent(si_w), "XEphem*SkyIP.y", skyipcategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Sky Image Tools"); n++;
	XtSetValues (XtParent(si_w), args, n);

	/* each separately controllable chunk in main RC */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNspacing, 4); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNadjustMargin, False); n++;
	XtSetArg (args[n], XmNadjustLast, True); n++;
	mrc_w = XmCreateRowColumn (si_w, "SIMRC", args, n);
	XtManageChild (mrc_w);

	/* overall info -- always on */

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, SINDENT); n++;
	f_w = XmCreateForm (mrc_w, "Image", args, n);
	XtManageChild (f_w);

	    /* info Text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNeditable, False); n++;
	    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	    XtSetArg (args[n], XmNblinkRate, 0); n++;
	    XtSetArg (args[n], XmNrows, 4); n++;
	    XtSetArg (args[n], XmNcolumns, 29); n++;
	    ostxt_w = XmCreateText (f_w, "GM", args, n);
	    wtip (ostxt_w, "Overall image statistics");
	    XtManageChild (ostxt_w);

	/* contrast controls */

	sectionCtrl (mrc_w, "SkyIP_Contrast", &ctb_w, "ContrastSection",
	    "Brightness and Contrast", "Whether to show the Contrast section");

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, SINDENT); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	f_w = XmCreateForm (mrc_w, "CF", args, n);
	if (XmToggleButtonGetState(ctb_w))
	    XtManageChild (f_w);

	XtAddCallback (ctb_w, XmNvalueChangedCallback, si_managetb_cb,
							    (XtPointer)f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	    XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	    XtSetArg (args[n], XmNnumColumns, 3); n++;
	    XtSetArg (args[n], XmNspacing, 3); n++;
	    XtSetArg (args[n], XmNisAligned, False); n++;
	    XtSetArg (args[n], XmNadjustMargin, False); n++;
	    XtSetArg (args[n], XmNadjustLast, False); n++;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    rc_w = XmCreateRowColumn (f_w, "Contrast", args, n);
	    XtManageChild (rc_w);

		n = 0;
		inv_w = XmCreateToggleButton (rc_w, "Reverse", args, n);
		XtAddCallback (inv_w, XmNvalueChangedCallback, si_inv_cb, 0);
		wtip (inv_w, "Whether image is displayed black-on-white");
		XtManageChild (inv_w);
		want_inv = XmToggleButtonGetState (inv_w);
		sr_reg (inv_w, NULL, skyipcategory, 1);

		n = 0;
		w = XmCreatePushButton (rc_w, "Nominal", args, n);
		XtAddCallback (w, XmNactivateCallback, si_contrast_cb,
						    (XtPointer)si_mcontrast);
		wtip (w, "Set nominal contrast");
		XtManageChild (w);

		n = 0;
		w = XmCreatePushButton (rc_w, "Narrow", args, n);
		XtAddCallback (w, XmNactivateCallback, si_contrast_cb,
							(XtPointer)si_narrow);
		wtip (w, "Set contrast range to -StDev/3 .. +StDev/3");
		XtManageChild (w);

		n = 0;
		w = XmCreatePushButton (rc_w, "Sharper", args, n);
		XtAddCallback (w, XmNactivateCallback, si_contrast_cb,
							(XtPointer)si_sharper);
		wtip (w, "Increase contrast");
		XtManageChild (w);

		n = 0;
		w = XmCreatePushButton (rc_w, "Duller", args, n);
		XtAddCallback (w, XmNactivateCallback, si_contrast_cb,
							(XtPointer)si_duller);
		wtip (w, "Decrease contrast");
		XtManageChild (w);

		n = 0;
		w = XmCreatePushButton (rc_w, "Wide", args, n);
		XtAddCallback (w, XmNactivateCallback, si_contrast_cb,
							(XtPointer)si_wide);
		wtip (w, "Set contrast range to -StDev .. +2*StDev");
		XtManageChild (w);

		n = 0;
		w = XmCreatePushButton (rc_w, "Brighter", args, n);
		XtAddCallback (w, XmNactivateCallback, si_contrast_cb,
							(XtPointer)si_brighter);
		wtip (w, "Increase brightness");
		XtManageChild (w);

		n = 0;
		w = XmCreatePushButton (rc_w, "Darker", args, n);
		XtAddCallback (w, XmNactivateCallback, si_contrast_cb,
							(XtPointer)si_darker);
		wtip (w, "Decrease brightness");
		XtManageChild (w);

		n = 0;
		w = XmCreatePushButton (rc_w, "Full", args, n);
		XtAddCallback (w, XmNactivateCallback, si_contrast_cb,
							(XtPointer)si_full);
		wtip (w, "Set contrast range to show all pixels");
		XtManageChild (w);

	    /* min/max fields */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 4); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (f_w, "Low ", args, n);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 3); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNcolumns, 5); n++;
	    lo_w = XmCreateTextField (f_w, "Low", args, n);
	    XtAddCallback (lo_w, XmNactivateCallback, si_lohi_cb, NULL);
	    XtManageChild (lo_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 4); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, lo_w); n++;
	    w = XmCreateLabel (f_w, " ... ", args, n);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 3); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNcolumns, 5); n++;
	    hi_w = XmCreateTextField (f_w, "Hi", args, n);
	    XtAddCallback (hi_w, XmNactivateCallback, si_lohi_cb, NULL);
	    XtManageChild (hi_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 4); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, hi_w); n++;
	    w = XmCreateLabel (f_w, "Hi ", args, n);
	    XtManageChild (w);

	    /* gamma scale */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, lo_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    XtSetArg (args[n], XmNminimum, (int)(1./GAM_MAX*100)); n++;
	    XtSetArg (args[n], XmNmaximum, (int)(GAM_MAX*100)); n++;
	    XtSetArg (args[n], XmNscaleMultiple, 20); n++;
	    XtSetArg (args[n], XmNdecimalPoints, 2); n++;
	    XtSetArg (args[n], XmNshowValue, True); n++;
	    gamma_w = XmCreateScale (f_w, "GammaScale", args, n);
	    XtAddCallback (gamma_w, XmNdragCallback, si_gamma_cb, NULL);
	    XtAddCallback (gamma_w, XmNvalueChangedCallback, si_gamma_cb, NULL);
	    wtip (gamma_w, "Select new Gamma factor: brightness = pixel^Gamma");
	    XtManageChild (gamma_w);
	    sr_reg (gamma_w, NULL, skyipcategory, 0);

	    /* drawing area in a frame for the contrast map */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, gamma_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 6); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    fr_w = XmCreateFrame (f_w, "HF", args, n);
	    XtManageChild (fr_w);

		n = 0;
		fda_w = XmCreateDrawingArea (fr_w, "Histogram", args, n);
		XtAddCallback (fda_w, XmNexposeCallback, si_exp_cb, NULL);
		mask = Button1MotionMask | ButtonPressMask | ButtonReleaseMask
						       | PointerMotionHintMask;
		XtAddEventHandler (fda_w, mask, False, si_motion_eh, NULL);
		XtManageChild (fda_w);
		wtip (fda_w,
		    "Histogram and colormap.. slide points to change limits");

	/* cross section slice */

	sectionCtrl (mrc_w, "SkyIP_Slice", &stb_w, "SliceSection",
			"Cross section Slice",
			"Whether to show the Cross-section Slice section");

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, SINDENT); n++;
	f_w = XmCreateForm (mrc_w, "Slice", args, n);
	if (XmToggleButtonGetState(stb_w)) {
	    XtManageChild (f_w);
	    svtb_updateSlice (1);
	}

	XtAddCallback (stb_w, XmNvalueChangedCallback, si_managetb_cb,
							    (XtPointer)f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    slmax_w = XmCreateToggleButton (f_w, "FullScale", args, n);
	    set_xmstring (slmax_w, XmNlabelString, "Full pixel range");
	    wtip (slmax_w, "Whether scale is Full or from Bright/Contrast");
	    sr_reg (slmax_w, NULL, skyipcategory, 1);
	    XtManageChild (slmax_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, slmax_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    fr_w = XmCreateFrame (f_w, "SF", args, n);
	    XtManageChild (fr_w);

		n = 0;
		sda_w = XmCreateDrawingArea (fr_w, "SlicePlot", args, n);
		XtManageChild (sda_w);
		wtip (sda_w, "Cross-sectional slice");

	/* glass section */

	sectionCtrl (mrc_w, "SkyIP_Glass", &gtb_w, "GlassSection",
	    "Magnifying Glass", "Whether to show the glass statistics section");

	n = 0;
	XtSetArg (args[n], XmNspacing, 2); n++;
	XtSetArg (args[n], XmNmarginWidth, SINDENT); n++;
	rc_w = XmCreateRowColumn (mrc_w, "Glass", args, n);
	if (XmToggleButtonGetState(gtb_w)) {
	    XtManageChild (rc_w);
	    svtb_updateGlass(1);
	}

	XtAddCallback (gtb_w, XmNvalueChangedCallback, si_managetb_cb,
							    (XtPointer)rc_w);

	    /* size RB */

	    n = 0;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    f_w = XmCreateForm (rc_w, "Size", args, n);
	    XtManageChild (f_w);

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNtopOffset, 3); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
		ml_w = XmCreateLabel (f_w, "SL", args, n);
		set_xmstring (ml_w, XmNlabelString, "Size:");
		XtManageChild (ml_w);

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
		XtSetArg (args[n], XmNleftWidget, ml_w); n++;
		XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		rb_w = XmCreateRadioBox (f_w, "GS", args, n);
		wtip (rb_w, "Glass' final size on screen");
		XtManageChild (rb_w);

		for (i = 0; i < XtNumber(glsz); i++) {
		    RadioSet *rp = &glsz[i];

		    n = 0;
		    rp->w = XmCreateToggleButton (rb_w, rp->name, args, n);
		    sr_reg (rp->w, NULL, skyipcategory, 1);
		    set_xmstring (rp->w, XmNlabelString, rp->label);
		    XtManageChild (rp->w);
		}

	    /* mag RB */

	    n = 0;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    f_w = XmCreateForm (rc_w, "Mag", args, n);
	    XtManageChild (f_w);

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNtopOffset, 3); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
		ml_w = XmCreateLabel (f_w, "ML", args, n);
		set_xmstring (ml_w, XmNlabelString, " Mag:");
		XtManageChild (ml_w);

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
		XtSetArg (args[n], XmNleftWidget, ml_w); n++;
		XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		rb_w = XmCreateRadioBox (f_w, "GM", args, n);
		wtip (rb_w, "Glass' magnification factor");
		XtManageChild (rb_w);

		for (i = 0; i < XtNumber(glmag); i++) {
		    RadioSet *rp = &glmag[i];

		    n = 0;
		    rp->w = XmCreateToggleButton (rb_w, rp->name, args, n);
		    sr_reg (rp->w, NULL, skyipcategory, 1);
		    set_xmstring (rp->w, XmNlabelString, rp->label);
		    XtManageChild (rp->w);
		}

	    /* info Text */

	    n = 0;
	    XtSetArg (args[n], XmNeditable, False); n++;
	    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	    XtSetArg (args[n], XmNblinkRate, 0); n++;
	    XtSetArg (args[n], XmNrows, 5); n++;
	    XtSetArg (args[n], XmNcolumns, 29); n++;
	    glstxt_w = XmCreateText (rc_w, "GM", args, n);
	    wtip (glstxt_w, "Glass' statistics");
	    XtManageChild (glstxt_w);

	    /* row/col plot controls */

	    n = 0;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    rc2_w = XmCreateRowColumn (rc_w, "Plot", args, n);
	    XtManageChild (rc2_w);

		n = 0;
		w = XmCreateLabel (rc2_w, "PCSL", args, n);
		set_xmstring (w, XmNlabelString, "Plot center: ");
		XtManageChild (w);

		n = 0;
		XtSetArg (args[n], XmNspacing, 3); n++;
		glrow_w = XmCreateToggleButton (rc2_w, "Row", args, n);
		sr_reg (glrow_w, NULL, skyipcategory, 0);
		wtip (glrow_w,
		   "Whether to draw a horizontal cross section");
		XtManageChild (glrow_w);

		n = 0;
		XtSetArg (args[n], XmNspacing, 3); n++;
		glcol_w = XmCreateToggleButton (rc2_w, "Column", args, n);
		sr_reg (glcol_w, NULL, skyipcategory, 0);
		wtip (glcol_w,
		    "Whether to draw a vertical cross section");
		XtManageChild (glcol_w);

	    /* drawing areas for each direction in frames */

	    n = 0;
	    fr_w = XmCreateFrame (rc_w, "GRF", args, n);
	    XtAddCallback (glrow_w, XmNvalueChangedCallback, si_managetb_cb,
							    (XtPointer)fr_w);
	    if (XmToggleButtonGetState(glrow_w))
		XtManageChild (fr_w);

		/* N.B. since this DA may not be mapped (if the frame is not
		 * managed) take care it is not used before it is really up.
		 */

		n = 0;
		XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
		glrda_w = XmCreateDrawingArea (fr_w, "RowPlot", args, n);
		XtManageChild (glrda_w);
		wtip (glrda_w, "Plot of central row under glass");

	    n = 0;
	    fr_w = XmCreateFrame (rc_w, "GCF", args, n);
	    XtAddCallback (glcol_w, XmNvalueChangedCallback, si_managetb_cb,
							    (XtPointer)fr_w);
	    if (XmToggleButtonGetState(glcol_w))
		XtManageChild (fr_w);

		n = 0;
		XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
		glcda_w = XmCreateDrawingArea (fr_w, "ColPlot", args, n);
		XtManageChild (glcda_w);
		wtip (glcda_w, "Plot of central column under glass");

	/* ROI section */

	sectionCtrl (mrc_w, "SkyIP_ROI", &rtb_w, "ROISection",
		"Region of Interest", "Whether to draw region of interest");

	n = 0;
	XtSetArg (args[n], XmNspacing, 2); n++;
	XtSetArg (args[n], XmNmarginWidth, SINDENT); n++;
	rc_w = XmCreateRowColumn (mrc_w, "ROI", args, n);
	if (XmToggleButtonGetState(rtb_w)) {
	    XtManageChild (rc_w);
	    svtb_updateROI(1);
	}

	XtAddCallback (rtb_w, XmNvalueChangedCallback, si_managetb_cb,
							    (XtPointer)rc_w);

	    /* info Text */

	    n = 0;
	    XtSetArg (args[n], XmNeditable, False); n++;
	    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	    XtSetArg (args[n], XmNblinkRate, 0); n++;
	    XtSetArg (args[n], XmNrows, 5); n++;
	    XtSetArg (args[n], XmNcolumns, 29); n++;
	    rstxt_w = XmCreateText (rc_w, "GM", args, n);
	    wtip (rstxt_w, "ROI statistics");
	    XtManageChild (rstxt_w);

	/* gaussian section */

	sectionCtrl (mrc_w, "SkyIP_2D_Gaussian", &ptb_w, "GaussianSection",
	    "2D Gaussian Measurements",
	    "Whether to show the Gaussian Photometry/Astrometry section");

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, SINDENT); n++;
	XtSetArg (args[n], XmNverticalSpacing, 4); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 4); n++;
	f_w = XmCreateForm (mrc_w, "Gaussian", args, n);
	if (XmToggleButtonGetState(ptb_w))
	    XtManageChild (f_w);

	XtAddCallback (ptb_w, XmNvalueChangedCallback, si_managetb_cb,
							    (XtPointer)f_w);

	    /* reference */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    ml_w = XmCreateLabel (f_w, "PP", args, n);
	    set_xmstring (ml_w, XmNlabelString, "Reference Mag:");
	    XtManageChild (ml_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    w = XmCreatePushButton (f_w, "Mark", args, n);
	    XtAddCallback (w, XmNactivateCallback, si_markrefstar_cb, NULL);
	    wtip (w, "Mark the current reference star on the image");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, w); n++;
	    w = XmCreatePushButton (f_w, "New", args, n);
	    XtAddCallback (w, XmNactivateCallback, si_newrefstar_cb, NULL);
	    wtip (w, "Assign the last star as the new reference star");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, ml_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, w); n++;
	    XtSetArg (args[n], XmNcolumns, 5); n++;
	    phmagref_w = XmCreateTextField (f_w, "RefMag", args, n);
	    XtAddCallback (phmagref_w, XmNactivateCallback, si_newref_cb, NULL);
	    sr_reg (phmagref_w, NULL, skyipcategory, 0);
	    wtip (phmagref_w, "Magnitude to use as reference to other stars");
	    XtManageChild (phmagref_w);

	    /* info Text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, phmagref_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 3); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNeditable, False); n++;
	    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	    XtSetArg (args[n], XmNblinkRate, 0); n++;
	    XtSetArg (args[n], XmNrows, 5); n++;
	    XtSetArg (args[n], XmNcolumns, 29); n++;
	    phtxt_w = XmCreateText (f_w, "GM", args, n);
	    XtAddCallback (phtxt_w, XmNactivateCallback, si_newrefstar_cb,NULL);
	    wtip (phtxt_w, "Gaussian solution details");
	    XtManageChild (phtxt_w);

	/* bottom PBs */

	n = 0;
	w = XmCreateSeparator (mrc_w, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	f_w = XmCreateForm (mrc_w, "F", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 20); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 40); n++;
	    w = XmCreatePushButton (f_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, si_close_cb, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 60); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 80); n++;
	    w = XmCreatePushButton (f_w, "Help", args, n);
	    XtAddCallback (w, XmNactivateCallback, si_help_cb, "SkyIP");
	    XtManageChild (w);
}

/* we just read a fits image into fim -- do everything necessary to display it.
 * last argument determines whether contrast and WCS are set automatically.
 */
static void
si_newImage(name, autocon)
char *name;
int autocon;
{
	double ra, dec;

	watch_cursor(1);

	/* compute overall image stats */
	findWholeImageStats();

	/* put the fits fields into the scrolled text */
	sf_showHeader(&fim);

	/* build the histogram */
	build_histo();

	/* pick a good initial lopix/hipix */
	if (autocon)
	    defContrast();

	/* assign the colors */
	build_colormap();

	/* draw the histogram */
	si_drawHistogram();

	/* establish north and east */
	si_ne();

	/* show basic stats */
	printImageStats(name);

	/* if find seed values but no WCS run solver, else just display*/
	if (autocon && initSolverFields() && xy2RADec(&fim,0,0,&ra,&dec) < 0)
	    wcsMatch ();
	else
	    sv_newFITS();

	watch_cursor(0);
}

/* set nup and elf from fim */
static void
si_ne()
{
	FImage *fip = &fim;
	double ra1, dec1, ra2, dec2;

	/* compare upper left corner to center */
	if (xy2RADec (fip, fip->sw/2.0, fip->sh/2.0, &ra1, &dec1) < 0) {
	    nup = elf = 1;	/* good as any */
	    return;
	}
	(void) xy2RADec (fip, 0.0, 0.0, &ra2, &dec2);

	nup = (dec2 > dec1);
	elf = (ra2 > ra1);
}

/* mark refstar on the image */
/* ARGSUSED */
static void
si_markrefstar_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (!fimok)
	    xe_msg (1, "First open an image");
	else if (!refstarok)
	    xe_msg (1, "Reference star not defined");
	else
	    sv_drawimdot (refstar.x, refstar.y, 5, 0);
}

/* make newstar the new refstar.
 * N.B. do not use call: used by PB and TF
 */
/* ARGSUSED */
static void
si_newrefstar_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (!fimok)
	    xe_msg (1, "First open an image");
	else if (newstarok) {
	    refstar = newstar;
	    refstarok = 1;
	    imPhotom (refstar.x, refstar.y);		/* show stats */
	    sv_all(NULL);				/* erase */
	    sv_drawimdot (refstar.x, refstar.y, 5, 0);	/* mark */
	} else {
	    xe_msg (1, "No star is defined as a photometric reference.\nPlease click on a star to set one.");
	}
}

/* user typed ENTER with a new reference value.
 */
/* ARGSUSED */
static void
si_newref_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (!fimok)
	    xe_msg (1, "First open an image");
	else if (newstarok) {
	    imPhotom (newstar.x, newstar.y);  		/* reshow stats */
	} else {
	    xe_msg (1, "No star is defined as a photometric reference.\nPlease click on a star to set one.");
	}
}

/* toggle a manager, in client, on or off.
 * some also mirror to the toolbar.
 */
/* ARGSUSED */
static void
si_managetb_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int state = XmToggleButtonGetState(w);
	Widget sw = (Widget)client;

	if (state)
	    XtManageChild (sw);
	else
	    XtUnmanageChild(sw);

	if (w == stb_w)
	    svtb_updateSlice(state);
	else if (w == ptb_w)
	    svtb_updateGauss(state);
	else if (w == rtb_w)
	    svtb_updateROI(state);
	else if (w == ctb_w)
	    svtb_updateContrast(state);
	else if (w == gtb_w)
	    svtb_updateGlass(state);
}

/* ARGSUSED */
static void
si_lohi_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *lostr = XmTextFieldGetString (lo_w);
	char *histr = XmTextFieldGetString (hi_w);
	int lo, hi;

	lo = atoi(lostr);
	hi = atoi(histr);
	if (lo<0 || lo>MAXCAMPIX || hi<0 || hi>MAXCAMPIX || hi<lo)
	    xe_msg (1, "Range must satisfy:\n0 <= Low <= Hi <= 65535");
	else {
	    lopix = lo;
	    hipix = hi;

	    watch_cursor(1);

	    build_colormap();
	    si_drawHistogram();
	    sv_dspFITS();

	    watch_cursor(0);
	}
	XtFree (lostr);
	XtFree (histr);
}

/* ARGSUSED */
static void
si_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (si_w);
}

/* called to put up help.
 * client is the help file tag.
 */
/* ARGSUSED */
static void
si_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *tag = (char *)client;

	if (!tag) {
	    printf ("SkyIP Help with missing tag\n");
	    abort();
	}
	hlp_dialog (tag, 0, 0);
}

/* get name of FITS field and load its value for given solver entry into GUI.
 * return 0 if ok, else -1 with excuse in msg[]
 */
static int
setWCSField (wsp, msg)
WCSSeed *wsp;
char msg[];
{
	char *kw, *bp, valu[100];
	double dvalu;
	int ret = 0;

	/* get name of field to use */
	kw = XmTextFieldGetString (wsp->kw_w);
	if (!*kw || ((bp = strrchr(kw,' ')) && bp[1] == '\0')) {
	    sprintf (msg, "Please specify %s", wsp->kwtip);
	    ret = -1;
	    goto done;
	}

	/* relax need for user to type in upper case */
	for (bp = kw; *bp; bp++)
	    if (islower(*bp))
		*bp = toupper(*bp);

	/* get from FITS header and copy to our value field */
	if (getStringFITS (&fim, kw, valu) == 0) {
	    XmTextFieldSetString (wsp->v_w, valu);
	} else if (getRealFITS (&fim, kw, &dvalu) == 0) {
	    sprintf (valu, "%g", dvalu);
	    XmTextFieldSetString (wsp->v_w, valu);
	} else {
	    sprintf (msg, "Header does not contain a field named '%s'", kw);
	    ret = -1;
	    goto done;
	}

    done:
	XtFree (kw);
	return (ret);
}

/* button motion and press/release event handler for the gray map */
/* ARGSUSED */
static void
si_motion_eh (w, client, ev, continue_to_dispatch)
Widget w;
XtPointer client;
XEvent *ev;
Boolean *continue_to_dispatch;
{
	static int moving_lopix;
	Display *dsp = XtDisplay(w);
	Window win = XtWindow(w);
	Dimension wid, hei;
	Window root, child;
	int rx, ry, x, y;
	unsigned mask;
	int evt = ev->type;
	int m1, b1p, b1r;
	int newpix;

	/* do nothing if no current image */
	if (!fimok)
	    return;

	/* what happened? */
	b1p = evt == ButtonPress   && ev->xbutton.button == Button1;
	b1r = evt == ButtonRelease && ev->xbutton.button == Button1;
	m1  = evt == MotionNotify  && ev->xmotion.state   & Button1Mask;

	/* ignore everything else */
	if (!b1p && !m1)
	    return;

	watch_cursor(1);

	/* where are we? */
	XQueryPointer (dsp, win, &root, &child, &rx, &ry, &x, &y, &mask);
	get_something (w, XmNwidth, (XtArgVal)&wid);
	get_something (w, XmNheight, (XtArgVal)&hei);
	if (x < 0)         x = 0;
	if (x >= (int)wid) x = wid-1;
	if (y < 0)         y = 0;
	if (y >= (int)hei) y = hei-1;

	/* scale x to pixel value */
	newpix = h_mini + x*(h_maxi - h_mini)/(int)wid;

	/* if button was just pressed, choose which end to track */
	if (b1p)
	    moving_lopix = newpix < (lopix + hipix)/2;

	/* track the current end -- but never cross over */
	if (moving_lopix && newpix < hipix)
	    lopix = newpix;
	else if (!moving_lopix && newpix > lopix)
	    hipix = newpix;

	/* rebuild colormap and redraw histogram to show new markers */
	build_colormap();
	si_drawHistogram();

	/* display net result */
	sv_dspFITS();

	watch_cursor(0);
}

/* expose callback for the gray map */
/* ARGSUSED */
static void
si_exp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
	Display *dsp = XtDisplay(fda_w);
	Window win = XtWindow(fda_w);

	switch (c->reason) {
	case XmCR_EXPOSE: {
	    static int before;
	    XExposeEvent *e = &c->event->xexpose;

	    if (!before) {
		XSetWindowAttributes swa;
		unsigned long mask = CWBitGravity;

		swa.bit_gravity = ForgetGravity;
		XChangeWindowAttributes (dsp, win, mask, &swa);
		before = 1;
	    }

	    /* wait for the last in the series */
	    if (e->count != 0)
		return;
	    break;
	    }
	default:
	    printf ("Unexpected fda_w event. type=%d\n", c->reason);
	    abort();
	}

	si_drawHistogram();
}

/* callback to set a preset contrast level.
 * client is pointer to function that sets lo/hipix (and maybe gamma)
 */
/* ARGSUSED */
static void
si_contrast_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	void (*pfv)() = (void (*)())client;

	if (!fimok)
	    return;

	/* implement desired contrast */
	(*pfv)();

	/* reassign the colors */
	build_colormap();

	/* redraw histogram to show new markers */
	si_drawHistogram();

	/* tell skyview to redraw */
	sv_dspFITS();
}

/* callback from Drag or ValueChanged on the gamma scale */
/* ARGSUSED */
static void
si_gamma_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (!fimok)
	    return;

	watch_cursor(1);

	build_colormap();
	si_drawHistogram();
	sv_dspFITS();

	watch_cursor(0);
}


/* callback for the gray map Inverse Vid TB */
/* ARGSUSED */
static void
si_inv_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	want_inv = XmToggleButtonGetState(w);

	if (!fimok)
	    return;

	sv_dspFITS();
}

/* draw histgram, if any, in the drawing area, and the lo/hi markers.
 * we stretch and only show from h_mini .. h_maxi
 * N.B. we might be called before the first expose
 */
static void
si_drawHistogram()
{
#define	MAXPTS	50
	Display *dsp = XtDisplay(fda_w);
	Window win = XtWindow(fda_w);
	int h_peak;
	XPoint pts[MAXPTS];
	Dimension wid, hei;
	char buf[32];
	int np;
	int x;

	/* might not be far enough */
	if (!win || !histo)
	    return;
	h_peak = histo[h_peaki];

	/* nothing to do if no image yet */
	if (!fimok)
	    return;

	/* get window size */
	get_something (fda_w, XmNwidth, (XtArgVal)&wid);
	get_something (fda_w, XmNheight, (XtArgVal)&hei);

	/* clear */
	XSetForeground (dsp, imgc, imbg_p);
	XFillRectangle (dsp, win, imgc, 0, 0, wid, hei);

	/* draw the histogram all across each x of the window */
	XSetForeground (dsp, imgc, imc1_p);
	for (np = x = 0; x < (int)wid; x++) {
	    int i, p, y;
	    int l, h;

	    /* find lo and hi pixel values binned into this x */
	    l = h_mini + x*(h_maxi - h_mini)/(int)wid;
	    h = h_mini + (x+1)*(h_maxi - h_mini)/(int)wid;

	    /* find peak in histogram portion shown at this x */
	    p = 0;
	    h = h < MAXCAMPIX ? h : MAXCAMPIX;
	    for (i = l; i < h; i++)
		if (histo[i] > p)
		    p = histo[i];

	    /* scale to window y value */
	    y = (int)hei-1 - p*((int)hei-1)/h_peak;

	    /* add another line segment if its y changes */
	    if (np == 0 || y != pts[np-1].y || x == (int)wid-1) {
		XPoint *xp = &pts[np++];
		xp->x = x;
		xp->y = y;
	    }
	    if (np == MAXPTS) {
		XDrawLines (dsp, win, imgc, pts, np, CoordModeOrigin);
		pts[0].x = pts[np-1].x;
		pts[0].y = pts[np-1].y;
		np = 1;
	    }
	}
	if (np > 1)
	    XDrawLines (dsp, win, imgc, pts, np, CoordModeOrigin);

	/* then overlay with the colormap within the lo/hi */
	XSetForeground (dsp, imgc, imc2_p);
	for (np = x = 0; x < (int)wid; x++) {
	    int i, p, y;
	    int l, h;

	    /* find lo and hi pixel values binned into this x */
	    l = h_mini + x*(h_maxi - h_mini)/(int)wid;
	    h = h_mini + (x+1)*(h_maxi - h_mini)/(int)wid;

	    /* only draw inside the indicators */
	    if (h < lopix || l > hipix)
		continue;

	    /* find peak in colormap portion shown at this x */
	    p = 0;
	    h = h < MAXCAMPIX ? h : MAXCAMPIX;
	    for (i = l; i < h; i++)
		if (colormap[i] > p)
		    p = colormap[i];

	    /* scale to window y value */
	    y = (int)hei-1 - p*((int)hei-1)/(ngray-1);

	    /* add another line segment if its y changes */
	    if (np == 0 || y != pts[np-1].y || x == (int)wid-1) {
		XPoint *xp = &pts[np++];
		xp->x = x;
		xp->y = y;
	    }
	    if (np == MAXPTS) {
		XDrawLines (dsp, win, imgc, pts, np, CoordModeOrigin);
		pts[0].x = pts[np-1].x;
		pts[0].y = pts[np-1].y;
		np = 1;
	    }
	}
	if (np > 1)
	    XDrawLines (dsp, win, imgc, pts, np, CoordModeOrigin);

	/* add the indicators */
	XSetForeground (dsp, imgc, imc3_p);
	x = (lopix - h_mini)*(int)wid/(h_maxi - h_mini);
	pts[0].x = x-MMW; pts[0].y = 0;
	pts[1].x = x;     pts[1].y = MMH;
	pts[2].x = x+MMW; pts[2].y = 0;
	XFillPolygon (dsp, win, imgc, pts, 3, Convex, CoordModeOrigin);

	x = (hipix - h_mini)*(int)wid/(h_maxi - h_mini);
	pts[0].x = x-MMW; pts[0].y = 0;
	pts[1].x = x;     pts[1].y = MMH;
	pts[2].x = x+MMW; pts[2].y = 0;
	XFillPolygon (dsp, win, imgc, pts, 3, Convex, CoordModeOrigin);

	/* add the numeric values too */
	sprintf (buf, "%5d", lopix);
	XmTextFieldSetString (lo_w, buf);
	sprintf (buf, "%5d", hipix);
	XmTextFieldSetString (hi_w, buf);

}

/* set up glass' resources according to current size and mag selections.
 */
static void
glassSetup ()
{

	glassSize();
	makeGlassImage ();
}

/* set glassmag/sz from widgets */
static void
glassSize()
{
	int i;

	/* get current mag and and size */
	glassmag = 0;
	for (i = 0; i < XtNumber(glmag); i++) {
	    if (XmToggleButtonGetState(glmag[i].w)) {
		glassmag = glmag[i].value;
		break;
	    }
	}
	if (!glassmag) {
	    printf ("Glass mag radiobox broke!\n");
	    abort();
	}
	glasssz = 0;
	for (i = 0; i < XtNumber(glsz); i++) {
	    if (XmToggleButtonGetState(glsz[i].w)) {
		glasssz = glsz[i].value;
		break;
	    }
	}
	if (!glasssz) {
	    printf ("Glass size radiobox broke!\n");
	    abort();
	}
}

/* (re)make glass_xim of size glasssz and same genre as fim.
 */
static void
makeGlassImage ()
{
	Display *dsp = XtD;
	char *glassdata;
	int nbytes;

	/* free any old then create with new size */
	if (glass_xim) {
	    free((char *)glass_xim->data);
	    glass_xim->data = NULL;
	    XDestroyImage(glass_xim);
	    glass_xim = 0;
	}

	nbytes = (glasssz+7) * (glasssz+7) * fbpp/8;
	glassdata = XtMalloc (nbytes);
	if (!glassdata) {
	    xe_msg (0, "Can not malloc %d for Glass pixels", nbytes);
	    return;
	}

	glass_xim = XCreateImage (dsp, XDefaultVisual (dsp, DefaultScreen(dsp)),
	    /* depth */         fbpp == 1 ? 1 : fdepth,
	    /* format */        fbpp == 1 ? XYBitmap : ZPixmap,
	    /* offset */        0,
	    /* data */          glassdata,
	    /* width */         glasssz,
	    /* height */        glasssz,
	    /* pad */           fbpp < 8 ? 8 : fbpp,
	    /* bpl */           0);

	if (!glass_xim) {
	    free ((void *)glassdata);
	    xe_msg (0, "Can not make Glass XImage");
	    return;
	}

        glass_xim->bitmap_bit_order = LSBFirst;
	glass_xim->byte_order = LSBFirst;
}

/* fill glass_xim with glasssz (X pixels) view of fim centered at X
 * coords xc,yc. take care at the edges.
 */
/* ARGSUSED */
static void
fillGlass (xc, yc)
int xc, yc;	/* center of glass, in X window coords */
{
	int isz = glasssz/glassmag;	/* size of patch image */
	int x, y, w, h;			/* pixmap patch to extract */
	int gx, gy;			/* glass coords */
	int px, py;			/* patch coords */
	Pixel p;

	/* get mag patch from pixmap and save in LR corner of glass image */
	px = py = glasssz-isz;
	x = xc-isz/2;
	if (x < 0) {
	    px -= x;
	    x = 0;
	}
	y = yc-isz/2;
	if (y < 0) {
	    py -= y;
	    y = 0;
	}
	w = h = isz;
	if (x + w > fpmw)
	    w = fpmw - x;
	if (y + h > fpmh)
	    h = fpmh - y;
	XGetSubImage (XtD, fpm, x, y, w, h, AllPlanes,
			fbpp == 1 ? XYBitmap : ZPixmap, glass_xim, px, py);

	/* spread patch out into entire glass in place */
	px = py = glasssz-isz;
	for (gy = 0; gy < glasssz; gy += glassmag) {
	    for (gx = 0; gx < glasssz; gx += glassmag) {
		p = XGetPixel (glass_xim, px+gx/glassmag, py+gy/glassmag);
		for (y = 0; y < glassmag; y++) {
		    for (x = 0; x < glassmag; x++) {
			XPutPixel (glass_xim, gx+x, gy+y, p);
		    }
		}
	    }
	}
}

/* compare two ObjF wrt f_mag, qsort style */
static int
objf_qsortf (const void *f1p, const void *f2p)
{
	Obj *o1p = (Obj *)f1p;
	Obj *o2p = (Obj *)f2p;
	double magdiff = get_mag(o1p) - get_mag(o2p);
	return (magdiff == 0 ? 0 : (magdiff < 0 ? -1 : 1));
}

/* find WCS solution for current image.
 */
static void
wcsMatch ()
{
	Now *np = mm_get_now();
	double mag;
	double ra0, dec0, right0, up0, rot0;
	double bestsep, worstsep;
	double *sx, *sy;
	int oldnodups;
	FImage tmpfi;
	int burnt;
	double std;
	double fov;
	char msg[1024];
	char *txt;
	ObjF *fsp;
	int nis, nfs;		/* n image, n field stars */

	/* get starting conditions */
	if (!fimok) {
	    xe_msg (1, "First open an image");
	    return;
	}
	if (getWCSSeed (&wcsseed[WCS_RA], &ra0) < 0)
	    return;
	if (getWCSSeed (&wcsseed[WCS_DEC], &dec0) < 0)
	    return;
	if (getWCSSeed (&wcsseed[WCS_USCALE], &up0) < 0)
	    return;
	if (getWCSSeed (&wcsseed[WCS_RSCALE], &right0) < 0)
	    return;
	if (getWCSSeed (&wcsseed[WCS_ROT], &rot0) < 0)
	    return;
	txt = XmTextFieldGetString (burnt_w);
	burnt = atoi (txt);
	XtFree (txt);
	txt = XmTextFieldGetString (snr_w);
	std = atof (txt);
	XtFree (txt);
	txt = XmTextFieldGetString (bsep_w);
	bestsep = atof (txt);
	XtFree (txt);
	txt = XmTextFieldGetString (wsep_w);
	worstsep = atof (txt);
	XtFree (txt);

	sv_dspFITS();	/* clean */

	/* get image stars, sorted by brightness */
	nis = quickStars (&fim, &wims, BORDER, burnt, std, &sx, &sy);
	if (nis < 0) {
	    xe_msg (1, "No memory to find stars");
	    return;
	}
	xe_msg (0,"Using %d image stars @ burnt = %d, SNR = %g", nis,burnt,std);

	/* get field stars in vicinity, sort by brightness.
	 * N.B. turn off nodups so we are imune to loaded catalogs
	 */
	fov = FOVMORE*sqrt (sqr(right0*fim.sw) + sqr(up0*fim.sh));
	mag = fov > degrad(10) ? 15 : 20;	/* no USNO if really big */
	oldnodups = fs_setnodups (0);
	nfs = fs_fetch (np, ra0, dec0, fov, mag, &fsp);
	fs_setnodups (oldnodups);
	if (nfs <= 0) {
	    free ((char *)sx);
	    free ((char *)sy);
	    xe_msg (1, "Field stars are required for pattern matching");
	    return;
	}
	qsort ((void*)fsp, nfs, sizeof(*fsp), objf_qsortf);
	xe_msg (0, "Using %d field stars, magnitude range %g .. %g", nfs,
			get_mag ((Obj*)&fsp[0]), get_mag((Obj*)&fsp[nfs-1]));

	/* initial rough coord system.
	 * N.B. take care with tmpfi since this copy shares the same pointers
	 */
	tmpfi = fim;
	tmpfi.xref = raddeg(ra0);
	tmpfi.yref = raddeg(dec0);
	tmpfi.xrefpix = fim.sw/2;
	tmpfi.yrefpix = fim.sh/2;
	tmpfi.xinc = raddeg(right0);
	tmpfi.yinc = raddeg(up0);
	tmpfi.rot = raddeg(rot0);
	strcpy (tmpfi.type, "-TAN");
	tmpfi.wcsset = 1;

	/* here we go */
	nfs = fsmatch (&tmpfi, sv_drawimdot, fsp, nfs, sx, sy, nis, bestsep,
								worstsep, msg);
	if (nfs < 0) {
	    free ((char *)fsp);
	    free ((char *)sx);
	    free ((char *)sy);
	    xe_msg (1, "%s", msg);
	    return;
	}

	/* ok! install new result */
	fim = tmpfi;

	/* show with its proud new WCS solution */
	sf_showHeader (&fim);
	(void) initSolverFields();
	si_ne();
	sv_newFITS();

	/* mark and report stats of solution */
	matchStats (fsp, sx, sy, nfs);

	/* clean up */
	free ((char *)fsp);
	free ((char *)sx);
	free ((char *)sy);
}

/* report just how well fim supposedly maps these stars to these image coords.
 */
static void
matchStats (fsp, sx, sy, nfs)
ObjF *fsp;
double *sx, *sy;
int nfs;
{
	double m, *med;
	double sum;
	Obj *bestop, *worstop;
	double best, worst;
	double asppix;
	int i;

	/* sanity check */
	if (nfs < 1)
	    return;

	/* get an array for finding median err */
	med = (double*)XtMalloc(nfs*sizeof(double));

	/* scan to build stats */
	sum = 0;
	worst = 0;
	best = 1e9;
	bestop = worstop = NULL;
	for (i = 0; i < nfs; i++) {
	    Obj *op = (Obj *)fsp++;
	    double x, y, err;

	    RADec2xy (&fim, op->f_RA, op->f_dec, &x, &y);
	    err = sqrt(sqr(x-sx[i]) + sqr(y-sy[i]));
	    if (err > worst) {
		worst = err;
		worstop = op;
	    }
	    if (err < best) {
		best = err;
		bestop = op;
	    }
	    sum += err;
	    med[i] = err;

	    sv_drawimdot (sx[i], sy[i], 5, 0);
	    sv_drawimdot (x, y, 7, 1);
	}
	m = dmedian (med, nfs);

	/* report info */
	asppix = 3600*sqrt(sqr((double)fim.xinc) + sqr((double)fim.yinc));
	xe_msg (confirm(), "Found WCS Solution using %d star pairs\n%5.2fp = %5.2f\" minimum error at %s\n%5.2fp = %5.2f\" maximum error at %s\n%5.2fp = %5.2f\" mean error\n%5.2fp = %5.2f\" median error",
		nfs,
		best, best*asppix, bestop->o_name,
		worst, worst*asppix, worstop->o_name,
		sum/nfs, sum/nfs*asppix,
		m, m*asppix);

	/* cleanup */
	XtFree ((char *)med);
}

/* return size of glass in whole image pixels, rounded up */
static int
glimsz()
{
	int proper = (int)ceil(glasssz/glassmag/fmag);
	return (proper >= MINGLSZ ? proper : MINGLSZ);
}


/* WCS dialog stuff
 */

/* called to bring up the wcs solver dialog */
void
siwcs_manage()
{
	si_create();
	XtManageChild(wcs_w);
}

/* called to bring down the wcs solver dialog */
void
siwcs_unmanage()
{
	if (wcs_w)
	    XtUnmanageChild (wcs_w);
}

/* set up the WCS solver fields from fip keywords, if possible.
 * return whether all fields are found.
 */
static int
initSolverFields ()
{
	char msg[1024];
	int ok = 1;
	int i;

	si_create();
	for (i = 0; i < XtNumber(wcsseed); i++)
	    if (setWCSField (&wcsseed[i], msg) < 0)
		ok = 0;

	return (ok);
}

/* create the WCS dialog */
static void
si_createwcsdialog()
{
	Widget sep_w, rc_w;
	Widget w;
	Arg args[20];
	int i;
	int n;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNallowResize, True); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	wcs_w = XmCreateFormDialog (svshell_w, "WCS", args, n);
	set_something (wcs_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (wcs_w, XmNhelpCallback, si_wcshelp_cb, NULL);
	sr_reg (XtParent(wcs_w), "XEphem*WCS.x", skyipcategory, 0);
	sr_reg (XtParent(wcs_w), "XEphem*WCS.y", skyipcategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem WCS solver"); n++;
	XtSetValues (XtParent(wcs_w), args, n);

	/* most go in an rc */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNnumColumns, XtNumber(wcsseed)+2); n++;
	XtSetArg (args[n], XmNspacing, 4); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNadjustMargin, True); n++;
	XtSetArg (args[n], XmNadjustLast, True); n++;
	XtSetArg (args[n], XmNmarginWidth, SINDENT); n++;
	rc_w = XmCreateRowColumn (wcs_w, "WCS", args, n);
	XtManageChild (rc_w);

	    for (i = 0; i < XtNumber(wcsseed); i++) {
		WCSSeed *wsp = &wcsseed[i];
		char kwres[64];

		addLabel (rc_w, wsp->label, -1);

		wsp->v_w = addTF (rc_w, wsp->res, 1);
		sr_reg (wsp->v_w, NULL, skyipcategory, 0);
		wtip (wsp->v_w, wsp->vtip);

		w = addPB (rc_w, "Use field:", 0);
		wtip (w,"Click to search FITS header for field named at right");
		XtAddCallback (w, XmNactivateCallback, si_wcsuse_cb,
								(XtPointer)wsp);

		sprintf (kwres, "%sField", wsp->res);
		wsp->kw_w = addTF (rc_w, kwres, 1);
		XtAddCallback (wsp->kw_w, XmNactivateCallback, si_wcsuse_cb,
								(XtPointer)wsp);
		sr_reg (wsp->kw_w, NULL, skyipcategory, 1);
		wtip (wsp->kw_w, wsp->kwtip);
	    }


	    addLabel (rc_w, "Burned out:", -1);
	    burnt_w = addTF (rc_w, "BurnedOut", 1);
	    XtAddCallback (burnt_w, XmNactivateCallback, si_markstars_cb, 0);
	    sr_reg (burnt_w, NULL, skyipcategory, 1);
	    wtip (burnt_w, "Maximum pixel value to trust");
	    XtManageChild (burnt_w);

	    addLabel (rc_w, "S/N ratio:", -1);
	    snr_w = addTF (rc_w, "SNR", 1);
	    XtAddCallback (snr_w, XmNactivateCallback, si_markstars_cb, 0);
	    sr_reg (snr_w, NULL, skyipcategory, 1);
	    wtip (snr_w, "Minimum standard deviation above mean for stars");
	    XtManageChild (snr_w);

	    addLabel (rc_w, "Max pix acc:", -1);
	    bsep_w = addTF (rc_w, "BestAcc", 1);
	    sr_reg (bsep_w, NULL, skyipcategory, 1);
	    wtip (bsep_w,
		    "Best accuracy this image can possibly produce, pixels");
	    XtManageChild (bsep_w);

	    addLabel (rc_w, "Min pix acc:", -1);
	    wsep_w = addTF (rc_w, "WorstAcc", 1);
	    sr_reg (wsep_w, NULL, skyipcategory, 1);
	    wtip (wsep_w, "Worst accuracy we will accept, pixels");
	    XtManageChild (wsep_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (wcs_w, "Sep", args, n);
	XtManageChild (sep_w);

	/* bottom PBs */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 4); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 24); n++;
	w = XmCreatePushButton (wcs_w, "Close", args, n);
	XtAddCallback (w, XmNactivateCallback, si_wcsclose_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 28); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 48); n++;
	w = XmCreatePushButton (wcs_w, "Go", args, n);
	XtAddCallback (w, XmNactivateCallback, si_wcsgo_cb, 0);
	wtip (w, "Begin solving for WCS headers");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 52); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 72); n++;
	w = XmCreatePushButton (wcs_w, "MS", args, n);
	set_xmstring (w, XmNlabelString, "Mark stars");
	wtip (w, "Mark the image stars that will be used in the solution");
	XtAddCallback (w, XmNactivateCallback, si_markstars_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 76); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 96); n++;
	w = XmCreatePushButton (wcs_w, "Help", args, n);
	XtAddCallback (w, XmNactivateCallback, si_wcshelp_cb, NULL);
	XtManageChild (w);
}

/* add a Label to a RC */
static void
addLabel (rc_w, label, alignment)
Widget rc_w;
char *label;
int alignment;
{
	Arg args[20];
	Widget w;
	int n;

	n = 0;
	if (alignment < 0) {
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	} else if (alignment > 0) {
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	} else {
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	}
	w = XmCreateLabel (rc_w, "L", args, n);
	XtManageChild (w);
	set_xmstring (w, XmNlabelString, label);
}

/* add a PB to a RC */
static Widget
addPB (rc_w, str, alignment)
Widget rc_w;
char *str;
int alignment;
{
	Arg args[20];
	Widget w;
	int n;

	n = 0;
	if (alignment < 0) {
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	} else if (alignment > 0) {
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	} else {
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	}
	w = XmCreatePushButton (rc_w, "L", args, n);
	XtManageChild (w);
	set_xmstring (w, XmNlabelString, str);
	return (w);
}

/* add a TextField to a RC */
static Widget
addTF (rc_w, name, rw)
Widget rc_w;
char *name;
int rw;
{
	Arg args[20];
	Widget w;
	int n;

	n = 0;
	XtSetArg (args[n], XmNcolumns, 14); n++;
	XtSetArg (args[n], XmNeditable, rw); n++;
	XtSetArg (args[n], XmNcursorPositionVisible, rw); n++;
	w = XmCreateTextField (rc_w, name, args, n);
	XtManageChild (w);
	return (w);
}

/* ARGSUSED */
static void
si_wcsclose_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (wcs_w);
}

/* called to start a WCS solution */
/* ARGSUSED */
static void
si_wcsgo_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	watch_cursor (1);
	wcsMatch ();
	watch_cursor (0);
}

/* called to mark seed stars */
/* ARGSUSED */
static void
si_markstars_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	double *sx, *sy;
	int burnt;
	double std;
	char *txt;
	int nis;
	int i;

	if (!fimok) {
	    xe_msg (1, "First open an image");
	    return;
	}

	watch_cursor (1);
	
	/* get max pix to use */
	txt = XmTextFieldGetString (burnt_w);
	burnt = atoi (txt);
	XtFree (txt);
	if (burnt < 0 || burnt > 65535) {
	    watch_cursor (0);
	    xe_msg (1, "Value for burned out pixel is missing or bogus.\nMust be in range 0 .. 65535");
	    return;
	}
	
	/* get SNR */
	txt = XmTextFieldGetString (snr_w);
	std = atof (txt);
	XtFree (txt);

	/* extract list of brighter isolated pixels as seeds for stars */
	nis = quickStars (&fim, &wims, BORDER, burnt, std, &sx, &sy);
	if (nis < 0) {
	    xe_msg (1, "No memory to find stars");
	    return;
	}
	xe_msg (0,"Found %d image stars @ burnt = %d, SNR = %g", nis,burnt,std);

	/* draw */
	sv_all(NULL);	/* clean */
	for (i = 0; i < nis; i++)
	    sv_drawimdot (sx[i], sy[i], 5, 0);

	/* clean up */
	free ((char *)sx);
	free ((char *)sy);

	watch_cursor (0);
}

/* called to plug in a nominal WCS value from an existing field.
 * N.B. don't use call: this is used for a PB and a TF
 * client is pointer into wcsseed[].
 */
/* ARGSUSED */
static void
si_wcsuse_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	WCSSeed *wsp = (WCSSeed *)client;
	char msg[1024];

	/* do nothing if no current image */
	if (!fimok) {
	    xe_msg (1, "First open an image");
	    return;
	}

	if (setWCSField (wsp, msg) < 0)
	    xe_msg (1, msg);
}

/* ARGSUSED */
static void
si_wcshelp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
	    "Pattern match with field stars to find a WCS solution."
	};

	hlp_dialog ("SkyIP_WCS", msg, sizeof(msg)/sizeof(msg[0]));
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: skyip.c,v $ $Date: 2012/11/23 04:14:01 $ $Revision: 1.43 $ $Name:  $"};

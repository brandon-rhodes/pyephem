/* misc handy X Windows functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <dirent.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>

#include "xephem.h"


static void stopd_cb (Widget w, XtPointer data, XtPointer call);
static Widget stopd_w;			/* dialog for user-stop */
static int stopd_stopped;		/* flag set by the stopd PB */

#define	MAXGRAY	30			/* max colors in a grayscale ramp */

/* createFSM aux info */
typedef struct {
    char **suffixes;			/* malloced list of suffixes */
    int nsuffixes;			/* number of suffixes */
    char *sharedir;			/* dir within ShareDir to scan */
    XtCallbackProc cb;			/* callback for each PB */
} FSMInfo;

/* names of resource colors for the planets.
 * N.B. must be in the same order as the defines in astro.h
 */
static char *plcolnames[] = {
    "mercuryColor", "venusColor", "marsColor", "jupiterColor",
    "saturnColor", "uranusColor", "neptuneColor", "plutoColor",
    "sunColor", "moonColor"
};

/* name of resource for all other solar system objects */
static char solsyscolname[] = "solSysColor";

/* spectral colors are defined with X res names starSpectX or starSpectXX.
 * resource is looked up first pick is requested
 */
static char sresbase[] = "starSpect";	/* base name for all spect res names */
typedef struct {
    char class[2];			/* 1 or 2 char spec class */
    GC gc;				/* gc to use */
} SpColor;
static SpColor *spcolors;		/* malloced array as discovered */
static int nspcolors;			/* entries in spcolors[] */

/* font and GCs we manage */
static XFontStruct *viewsfsp;
static XFontStruct *trackingfsp;
static GC pl_gc[XtNumber(plcolnames)];
static GC solsys_gc;
static GC esat_gc;
static GC other_gc;
static GC otherstar_gc;

/* info used to make XmButton look like XmButton or XmLabel */
static Arg look_like_button[] = {
    {XmNtopShadowColor, (XtArgVal) 0},
    {XmNbottomShadowColor, (XtArgVal) 0},
    {XmNtopShadowPixmap, (XtArgVal) 0},
    {XmNbottomShadowPixmap, (XtArgVal) 0},
    {XmNfillOnArm, (XtArgVal) True},
    {XmNtraversalOn, (XtArgVal) True},
};
static Arg look_like_label[] = {
    {XmNtopShadowColor, (XtArgVal) 0},
    {XmNbottomShadowColor, (XtArgVal) 0},
    {XmNtopShadowPixmap, (XtArgVal) 0},
    {XmNbottomShadowPixmap, (XtArgVal) 0},
    {XmNfillOnArm, (XtArgVal) False},
    {XmNtraversalOn, (XtArgVal) False},
};
static int look_like_inited;

/* handy way to set one resource for a widget.
 * shouldn't use this if you have several things to set for the same widget.
 */
void
set_something (w, resource, value)
Widget w;
char *resource;
XtArgVal value;
{
	Arg a[1];

	if (!w) {
	    printf ("set_something(w=%p, res=%s)\n", w, resource);
	    abort();
	}

	XtSetArg (a[0], resource, value);
	XtSetValues (w, a, 1);
}

/* handy way to get one resource for a widget.
 * shouldn't use this if you have several things to get for the same widget.
 */
void
get_something (w, resource, value)
Widget w;
char *resource;
XtArgVal value;
{
	Arg a[1];

	if (!w) {
	    printf ("get_something (%s) called with w==0\n", resource);
	    abort();
	}

	XtSetArg (a[0], resource, value);
	XtGetValues (w, a, 1);
}

/* return the given XmString resource from the given widget as a char *.
 * N.B. based on a sample in Heller, pg 178, the string back from
 *   XmStringGetLtoR should be XtFree'd. Therefore, OUR caller should always
 *   XtFree (*txtp).
 */
void
get_xmstring (w, resource, txtp)
Widget w;
char *resource;
char **txtp;
{
	static char me[] = "get_xmstring()";
	static char hah[] = "??";

	if (!w) {
	    printf ("%s: called for %s with w==0\n", me, resource);
	    abort();
	} else {
	    XmString str;
	    get_something(w, resource, (XtArgVal)&str); 
	    if (!XmStringGetLtoR (str, XmSTRING_DEFAULT_CHARSET, txtp)) {
		/*
		fprintf (stderr, "%s: can't get string resource %s\n", me,
								resource);
		abort();
		*/
		(void) strcpy (*txtp = XtMalloc(sizeof(hah)), hah);
	    }
	    XmStringFree (str);
	}
}

void
set_xmstring (w, resource, txt)
Widget w;
char *resource;
char *txt;
{
	XmString str;

	if (!w) {
	    printf ("set_xmstring(w=%p, res=%s, txt=%s)\n", w, resource, txt);
	    abort();
	}

	str = XmStringCreateLtoR (txt, XmSTRING_DEFAULT_CHARSET);
	set_something (w, resource, (XtArgVal)str);
	XmStringFree (str);
}

/* return 1 if w is on screen else 0 */
int
isUp (w)
Widget w;
{
	XWindowAttributes wa;
	Display *dsp;
	Window win;

	if (!w)
	    return (0);
	dsp = XtDisplay(w);
	win = XtWindow(w);
	return (win && XGetWindowAttributes(dsp, win, &wa) &&
						wa.map_state == IsViewable);
}

/* may be connected as the mapCallback or XmNpopupCallback to
 * center a window on the cursor (allowing for the screen edges).
 */
/* ARGSUSED */
void
prompt_map_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Window root, child;
	int rx, ry, wx, wy;	/* rx/y: cursor loc on root window */
	unsigned sw, sh;	/* screen width/height */
	Dimension ww, wh;	/* this widget's width/height */
	Position x, y;		/* final location */
	unsigned mask;
	Arg args[20];
	int n;

	XQueryPointer (XtDisplay(w), XtWindow(w),
				&root, &child, &rx, &ry, &wx, &wy, &mask);
	sw = WidthOfScreen (XtScreen(w));
	sh = HeightOfScreen(XtScreen(w));
	n = 0;
	XtSetArg (args[n], XmNwidth, &ww); n++;
	XtSetArg (args[n], XmNheight, &wh); n++;
	XtGetValues (w, args, n);

	x = rx - ww/2;
	if (x < 0)
	    x = 0;
	else if (x + ww >= (int)sw)
	    x = sw - ww;
	y = ry - wh/2;
	if (y < 0)
	    y = 0;
	else if (y + wh >= (int)sh)
	    y = sh - wh;

	n = 0;
	XtSetArg (args[n], XmNx, x); n++;
	XtSetArg (args[n], XmNy, y); n++;
	XtSetValues (w, args, n);
}

/* get the named color for w's colormap in *p, else set to White.
 * return 0 if the color was found, -1 if White had to be used.
 */
int
get_color_resource (w, cname, p)
Widget w;
char *cname;
Pixel *p;
{
	Display *dsp = XtDisplay(w);
	Colormap cm;
	XColor defxc, dbxc;
	Arg arg;
	char *cval;

	XtSetArg (arg, XmNcolormap, &cm);
	XtGetValues (w, &arg, 1);
	cval = getXRes (cname, NULL);

	if (!cval || !XAllocNamedColor (dsp, cm, cval, &defxc, &dbxc)) {
	    char msg[128];
	    if (!cval)
		sprintf (msg, "Can not find resource `%.80s'", cname);
	    else
		sprintf (msg, "Can not XAlloc color `%.80s'", cval);
	    strcat (msg, " ... using White");
	    xe_msg (0, msg);
	    *p = WhitePixel (dsp, DefaultScreen(dsp));
	    return (-1);
	} else {
	    *p = defxc.pixel;
	    return (0);
	}
}

/* get the XFontStruct we want to use when drawing text for the display views.
 */
void
get_views_font (dsp, fspp)
Display *dsp;
XFontStruct **fspp;
{
	if (!viewsfsp)
	    viewsfsp = getXResFont ("viewsFont");

	*fspp = viewsfsp;
}

/* set the XFontStruct we want to use when drawing text for the display views.
 * this also means setting all the GCs for objects.
 */
void
set_views_font (dsp, fsp)
Display *dsp;
XFontStruct *fsp;
{
	Font fid;
	int i;

	/* TODO: free old? */
	viewsfsp = fsp;

	/* may not have created gcs yet */
	if (!other_gc)
	    make_objgcs();

	/* update all Fonts in GCs */
	fid = fsp->fid;
	for (i = 0; i < XtNumber(plcolnames); i++)
	    XSetFont (dsp, pl_gc[i], fid);
	for (i = 0; i < nspcolors; i++)
	    XSetFont (dsp, spcolors[i].gc, fid);
	XSetFont (dsp, solsys_gc, fid);
	XSetFont (dsp, other_gc, fid);
	XSetFont (dsp, otherstar_gc, fid);
	XSetFont (dsp, esat_gc, fid);
}

/* get the XFontStruct we want to use when drawing text while tracking cursor.
 */
void
get_tracking_font (dsp, fspp)
Display *dsp;
XFontStruct **fspp;
{
	if (!trackingfsp)
	    trackingfsp = getXResFont ("cursorTrackingFont");

	*fspp = trackingfsp;
}

/* set the XFontStruct we want to use when drawing text while tracking cursor.
 */
void
set_tracking_font (dsp, fsp)
Display *dsp;
XFontStruct *fsp;
{
	/* TODO: free old? */
	trackingfsp = fsp;
}

/* make all the various GCs used for objects from obj_pickgc().
 * TODO: reclaim old stuff if called again, but beware of hoarding users.
 */
void
make_objgcs()
{
	Display *dsp = XtDisplay(toplevel_w);
	Window win = RootWindow (dsp, DefaultScreen (dsp));
	unsigned long gcm;
	XFontStruct *fsp;
	XGCValues gcv;
	Pixel p;
	int i, j;

	/* always set font and foreground */
	gcm = GCFont | GCForeground;
	get_views_font (dsp, &fsp);
	gcv.font = fsp->fid;

	/* make the planet gcs */
	for (i = 0; i < XtNumber(pl_gc); i++) {
	    (void) get_color_resource (toplevel_w, plcolnames[i], &p);
	    gcv.foreground = p;
	    pl_gc[i] = XCreateGC (dsp, win, gcm, &gcv);
	}

	/* make the gc for other solar system objects */
	(void) get_color_resource (toplevel_w, solsyscolname, &p);
	gcv.foreground = p;
	solsys_gc = XCreateGC (dsp, win, gcm, &gcv);

	/* make the gc for fixed types other than stars */
	(void) get_color_resource (toplevel_w, "otherObjColor", &p);
	gcv.foreground = p;
	other_gc = XCreateGC (dsp, win, gcm, &gcv);

	/* make the gc for stars without a matching spectral color */
	gcv.foreground = WhitePixel (dsp, DefaultScreen(dsp));
	otherstar_gc = XCreateGC (dsp, win, gcm, &gcv);

	/* build all spectral colors in resource database */
	for (i = 0; i < nspcolors; i++)
	    XFreeGC (dsp, spcolors[i].gc);
	nspcolors = 0;
	for (i = 'A'; i <= 'Z'; i++) {
	    for (j = 0; j <= '9'; j = j ? j+1 : '0') {
		char *v, res[sizeof(sresbase)+3];
		SpColor *sp;

		sprintf (res, "%s%c%c", sresbase, i, j);
		v = getXRes (res, NULL);
		if (v) {
		    XColor defxc, dbxc;
		    if (!XAllocNamedColor (dsp, xe_cm, v, &defxc, &dbxc))
			defxc.pixel = WhitePixel (dsp, DefaultScreen(dsp));
		    spcolors = (SpColor *) XtRealloc ((char *)spcolors,
						(nspcolors+1)*sizeof(SpColor));
		    sp = &spcolors[nspcolors++];
		    gcv.foreground = defxc.pixel;
		    sp->gc = XCreateGC (dsp, win, gcm, &gcv);
		    sp->class[0] = i;
		    sp->class[1] = j;
		}
	    }
	}

	/* make the gc for earth sats */
	(void) get_color_resource (toplevel_w, "satellitesColor", &p);
	gcv.foreground = p;
	esat_gc = XCreateGC (dsp, win, gcm, &gcv);
}

/* given an object, return a GC for it.
 * Use the colors defined for objects in the X resources, else White.
 */
void
obj_pickgc(op, w, gcp)
Obj *op;
Widget w;
GC *gcp;
{
	/* insure GCs are ready */
	if (!other_gc)
	    make_objgcs();

	if (is_type (op, PLANETM))
	    *gcp = pl_gc[op->pl_code];		/* moons share parent color */
	else if (is_ssobj(op))
	    *gcp = solsys_gc;
	else if (is_type (op, EARTHSATM))
	    *gcp = esat_gc;
	else if (is_type (op, FIXEDM) || is_type (op, BINARYSTARM)) {
	    int l, u, m = 0, d = -1;

	    /* all but bonafide stars use "other" */
	    if (is_type (op, FIXEDM)) {
		switch (op->f_class) {
		case 'T': case 'B': case 'D': case 'M': case 'S': case 'V':
		    break;
		default:
		    *gcp = other_gc;
		    return;
		}
	    }

	    /* binary search for perfect match */
	    l = 0;
	    u = nspcolors - 1;
	    while (l <= u) {
		m = (l+u)/2;
		d = strncmp (spcolors[m].class, op->f_spect, 2);
		if (d == 0)
		    break;
		if (d < 0)
		    l = m+1;
		else
		    u = m-1;
	    }

	    /* select perfect or +/- 1 if same first char */
	    if (d == 0 || op->f_spect[0] == spcolors[m].class[0])
		*gcp = spcolors[m].gc;
	    else if (m > 0 && op->f_spect[0] == spcolors[m-1].class[0])
		*gcp = spcolors[m-1].gc;
	    else if (m< nspcolors-1 && op->f_spect[0] == spcolors[m+1].class[0])
		*gcp = spcolors[m+1].gc;
	    else {
		if (op->f_spect[0]) {
		    xe_msg (0,
		    "No color specified for %s class %c spectral class %.2s",
					op->o_name, op->f_class, op->f_spect);
		}
		*gcp = otherstar_gc;
	    }
	} else {
	    printf ("Unknown object type %d for coloring\n", op->o_type);
	    abort();
	}
}

/* given any widget built from an XmLabel return pointer to the first
 * XFontStruct in its XmFontList.
 */
void
get_xmlabel_font (w, f)
Widget w;
XFontStruct **f;
{
	static char me[] = "get_xmlable_font";
	XmFontList fl;
	XmFontContext fc;
	XmStringCharSet charset;

	get_something (w, XmNfontList, (XtArgVal)&fl);
	if (XmFontListInitFontContext (&fc, fl) != True) {
	    printf ("%s: No Font context!\n", me);
	    abort();
	}
	if (XmFontListGetNextFont (fc, &charset, f) != True) {
	    printf ("%s: no font!\n", me);
	    abort();
	}
	XmFontListFreeFontContext (fc);
}

/* get the font named by the given X resource, else fixed, else bust */
XFontStruct *
getXResFont (rn)
char *rn;
{
	static char fixed[] = "fixed";
	char *fn = getXRes (rn, NULL);
	Display *dsp = XtDisplay(toplevel_w);
	XFontStruct *fsp;

	if (!fn) {
	    xe_msg (0, "No resource `%s' .. using fixed", rn);
	    fn = fixed;
	}
	
	/* use XLoadQueryFont because it returns gracefully if font is not
	 * found; XLoadFont calls the default X error handler.
	 */
	fsp = XLoadQueryFont (dsp, fn);
	if (!fsp) {
	    xe_msg (0, "No font `%s' for `%s' .. using fixed", fn,rn);
	    fsp = XLoadQueryFont (dsp, fixed);
	    if (!fsp) {
		printf ("Can't even get %s!\n", fixed);
		abort();
	    }
	}

	return (fsp);
}


/* load the greek font into *greekfspp then create a new gc at *greekgcp and
 *   set its font to the font id..
 * leave *greekgcp and greekfssp unchanged if there's any problems.
 */
void
loadGreek (Display *dsp, Drawable win, GC *greekgcp, XFontStruct **greekfspp)
{
	static char grres[] = "viewsGreekFont";
	XFontStruct *fsp;	/* local fast access */
	GC ggc;			/* local fast access */
	unsigned long gcm;
	XGCValues gcv;
	char *greekfn;
	
	greekfn = getXRes (grres, NULL);
	if (!greekfn) {
	    xe_msg (0, "No resource: %s", grres);
	    return;
	}

	fsp = XLoadQueryFont (dsp, greekfn);
	if (!fsp) {
	    xe_msg (0, "No font %.100s: %.800s", grres, greekfn);
	    return;
	}

	gcm = GCFont;
	gcv.font = fsp->fid;
	ggc = XCreateGC (dsp, win, gcm, &gcv);
	if (!ggc) {
	    XFreeFont (dsp, fsp);
	    xe_msg (0, "Can not make Greek GC");
	    return;
	}

	*greekgcp = ggc;
	*greekfspp = fsp;

	return;
}

/* return a gray-scale ramp of pixels at *pixp, and the number in the ramp
 * N.B. don't change the pixels -- they are shared with other users.
 */
int
gray_ramp (dsp, cm, pixp)
Display *dsp;
Colormap cm;
Pixel **pixp;
{
	static Pixel gramp[MAXGRAY];
	static int ngray;

	if (ngray == 0) {
	    /* get gray ramp pixels once */
	    XColor white;

	    white.red = white.green = white.blue = ~0;
	    ngray = alloc_ramp (dsp, &white, cm, gramp, MAXGRAY);
	    if (ngray < MAXGRAY)
		xe_msg (0, "Wanted %d but only found %d grays.",MAXGRAY,ngray);
	}

	*pixp = gramp;
	return (ngray);
}

/* try to fill pix[maxn] with linear ramp from black to whatever is in base.
 * each entry will be unique; return said number, which can be <= maxn.
 * N.B. if we end up with just 2 colors, we set pix[0]=0 and pix[1]=1 in
 *   anticipation of caller using a XYBitmap and thus XPutImage for which
 *   color 0/1 uses the background/foreground of a GC.
 */
int
alloc_ramp (Display *dsp, XColor *basep, Colormap cm, Pixel pix[], int maxn)
{
	int nalloc, nunique;
	double r, g, b, h, s, v;
	XColor xc;
	int i, j;

	/* work in HSV space from 0..V */
	r = basep->red/65535.;
	g = basep->green/65535.;
	b = basep->blue/65535.;
	toHSV (r, g, b, &h, &s, &v);

	/* first just try to get them all */
	for (nalloc = 0; nalloc < maxn; nalloc++) {
	    toRGB (h, s, v*nalloc/(maxn-1), &r, &g, &b);
	    xc.red = (int)(r*65535);
	    xc.green = (int)(g*65535);
	    xc.blue = (int)(b*65535);
	    if (XAllocColor (dsp, cm, &xc))
		pix[nalloc] = xc.pixel;
	    else
		break;
	}

	/* see how many are actually unique */
	nunique = 0;
	for (i = 0; i < nalloc; i++) {
	    for (j = i+1; j < nalloc; j++)
		if (pix[i] == pix[j])
		    break;
	    if (j == nalloc)
		nunique++;
	}

	if (nunique < maxn) {
	    /* rebuild the ramp using just nunique entries.
	     * N.B. we assume we can get nunique colors again right away.
	     */
	    XFreeColors (dsp, cm, pix, nalloc, 0);

	    if (nunique <= 2) {
		/* we expect caller to use a XYBitmap via GC */
		pix[0] = 0;
		pix[1] = 1;
		nunique = 2;
	    } else {
		for (i = 0; i < nunique; i++) {
		    toRGB (h, s, v*i/(nunique-1), &r, &g, &b);
		    xc.red = (int)(r*65535);
		    xc.green = (int)(g*65535);
		    xc.blue = (int)(b*65535);
		    if (!XAllocColor (dsp, cm, &xc)) {
			nunique = i;
			break;
		    }
		    pix[i] = xc.pixel;
		}
	    }
	}

	return (nunique);
}

/* create an XImage of size wXh.
 * return XImage * if ok else NULL and xe_msg().
 */
XImage *
create_xim (int w, int h)
{
	Display *dsp = XtDisplay(toplevel_w);
	XImage *xip;
	int mdepth;
	int mbpp;
	int nbytes;
	char *data;

	/* establish depth and bits per pixel */
	get_something (toplevel_w, XmNdepth, (XtArgVal)&mdepth);
	if (mdepth < 8) {
	    fprintf (stderr, "Require at least 8 bit pixel depth\n");
	    return (NULL);
	}
	mbpp = mdepth>=17 ? 32 : (mdepth >= 9 ? 16 : 8);
	nbytes = w*h*mbpp/8;

	/* get memory for image pixels.  */
	data = malloc (nbytes);
	if (!data) {
	    fprintf(stderr,"Can not get %d bytes for image pixels", nbytes);
	    return (NULL);
	}

	/* create the XImage */
	xip = XCreateImage (dsp, DefaultVisual (dsp, DefaultScreen(dsp)),
	    /* depth */         mdepth,
	    /* format */        ZPixmap,
	    /* offset */        0,
	    /* data */          data,
	    /* width */         w,
	    /* height */        h,
	    /* pad */           mbpp,
	    /* bpl */           0);
	if (!xip) {
	    fprintf (stderr, "Can not create %dx%d XImage\n", w, h);
	    free ((void *)data);
	    return (NULL);
	}

        xip->bitmap_bit_order = LSBFirst;
	xip->byte_order = LSBFirst;

	/* ok */
	return (xip);
}

/* like XFreeColors but frees the pixels in xcols[nxcols]
 */
void
freeXColors (Display *dsp, Colormap cm, XColor xcols[], int nxcols)
{
	unsigned long *xpix = (Pixel*)XtMalloc(nxcols * sizeof(unsigned long));
	int i;

	for (i = 0; i < nxcols; i++)
	    xpix[i] = xcols[i].pixel;

	XFreeColors (dsp, cm, xpix, nxcols, 0);

	XtFree ((void *)xpix);
}

/* given a raw (still compressed) gif file in gif[ngif], malloc its 1-byte
 * pixels in *gifpix[*wp][*hp] and fill xcols[256] with X pixels and colors
 * that work when indexed by an gifpix entry.
 * return 0 if ok, else -1 with err[] containing a reason why not.
 */
int
gif2X (
Display *dsp,		/* X server */
Colormap cm,		/* colormap for xcols[] */
unsigned char gif[],	/* raw (still compressed) gif file contents */
int ngif,		/* bytes in gif[] */
int *wp, int *hp,	/* size of exploded gif */
unsigned char **gifpix,	/* ptr to array we malloc for gif pixels */
XColor xcols[256],	/* X pixels and colors when indexed by gifpix */
char err[])		/* error message if we return -1 */
{
	unsigned char gifr[256], gifg[256], gifb[256];
	int i;

	/* uncompress */
	if (explodeGIF(gif, ngif, wp, hp, gifpix, gifr, gifg, gifb, err) < 0)
	    return (-1);

	/* allocate colors -- don't be too fussy */
	for (i = 0; i < 256; i++) {
	    XColor *xcp = xcols+i;
	    xcp->red   = ((short)(gifr[i] & 0xf8) << 8) | 0x7ff;
	    xcp->green = ((short)(gifg[i] & 0xf8) << 8) | 0x7ff;
	    xcp->blue  = ((short)(gifb[i] & 0xf8) << 8) | 0x7ff;
	    if (!XAllocColor (dsp, cm, xcp)) {
		strcpy (err, "Can not get all image map colors");
		free ((void *)(*gifpix));
		if (i > 0)
		    freeXColors (dsp, cm, xcols, i);
		return (-1);
	    }
	}

	/* ok */
	return (0);
}

/* given a raw gif file in gif[ngif] return a new pixmap and its size.
 * return 0 if ok, else fill why[] and return -1.
 */
int
gif2pm (Display *dsp,
Colormap cm,
unsigned char gif[],
int ngif,
int *wp, int *hp,
Pixmap *pmp,
char why[])
{
	Window win = RootWindow(dsp, DefaultScreen(dsp));
	unsigned char *gifpix;
	XColor xcols[256];
	XImage *xip;
	Pixmap pm;
	GC gc;
	int w, h;
	int x, y;

	/* get X version of image */
	if (gif2X (dsp, cm, gif, ngif, &w, &h, &gifpix, xcols, why) < 0)
	    return (-1);

	/* create XImage */
	xip = create_xim (w, h);
	if (!xip) {
	    freeXColors (dsp, cm, xcols, 256);
	    free ((void *)gifpix);
	    strcpy (why, "No memory for image");
	    return (-1);
	}

	/* N.B. now obligued to free xip */

	/* fill XImage with image */
	for (y = 0; y < h; y++) {
	    int yrow = y*w;
	    for (x = 0; x < w; x++) {
		int gp = (int)gifpix[x + yrow];
		unsigned long p = xcols[gp].pixel;
		XPutPixel (xip, x, y, p);
	    }
	}

	/* create pixmap and fill with image */
	pm = XCreatePixmap (dsp, win, w, h, xip->depth);
	gc = DefaultGC (dsp, DefaultScreen(dsp));
	XPutImage (dsp, pm, gc, xip, 0, 0, 0, 0, w, h);

	/* free gifpix and xip */
	free ((void *)gifpix);
	free ((void *)xip->data);
	xip->data = NULL;
	XDestroyImage (xip);

	/* that's it! */
	*wp = w;
	*hp = h;
	*pmp = pm;
	return (0);
}

/* search for  the named X resource from all the usual places.
 * this looks in more places than XGetDefault().
 * we just return it as a string -- caller can do whatever.
 * return def if can't find it anywhere.
 * N.B. memory returned is _not_ malloced so leave it be.
 * N.B. see setXRes for how newlines are handled in the Xrm.
 */
char *
getXRes (name, def)
char *name;
char *def;
{
	static char notfound[] = "_Not_Found_";
	char *res = NULL;
	XtResource xr;

	xr.resource_name = name;
	xr.resource_class = "AnyClass";
	xr.resource_type = XmRString;
	xr.resource_size = sizeof(String);
	xr.resource_offset = 0;
	xr.default_type = XmRImmediate;
	xr.default_addr = (XtPointer)notfound;

	XtGetApplicationResources (toplevel_w, (void *)&res, &xr, 1, NULL, 0);
	if (!res || strcmp (res, notfound) == 0)
	    res = def;

	return (res);
}

/* set the given application (ie, myclass.name) resource.
 * N.B. the combination of setXRes/getXRes has the effect that each backslash-n
 *   in val comes back as a real nl and everything including and after the
 *   the first real nl is discarded. if you think about it, this is the same
 *   behavior as writing to and reading back one res to an app-defaults file.
 */
void
setXRes (name, val)
char *name, *val;
{
	XrmDatabase db = XrmGetDatabase (XtDisplay(toplevel_w));
	char buf[1024];

	sprintf (buf, "%s.%s:%s", myclass, name, val ? val : "");
	XrmPutLineResource (&db, buf);
}

/* build and return a private colormap for toplevel_w.
 * nnew is how many colors we expect to add.
 */
Colormap
createCM(nnew)
int nnew;
{
#define	NPRECM	50  /* try to preload new cm with NPRECM colors from def cm */
	Display *dsp = XtDisplay (toplevel_w);
	Window win = RootWindow (dsp, DefaultScreen(dsp));
	Colormap defcm = DefaultColormap (dsp, DefaultScreen(dsp));
	int defcells = DisplayCells (dsp, DefaultScreen(dsp));
	Visual *v = DefaultVisual (dsp, DefaultScreen(dsp));
	Colormap newcm;

	/* make a new colormap */
	newcm = XCreateColormap (dsp, win, v, AllocNone);

	/* preload with some existing colors to hedge flashing, if room */
	if (nnew + NPRECM < defcells) {
	    XColor preload[NPRECM];
	    int i;

	    for (i = 0; i < NPRECM; i++)
		preload[i].pixel = (unsigned long) i;
	    XQueryColors (dsp, defcm, preload, NPRECM);
	    for (i = 0; i < NPRECM; i++)
		(void) XAllocColor (dsp, newcm, &preload[i]);
	}

	return (newcm);
}

/* depending on the "install" resource and whether cm can hold nwant more
 * colors, return a new colormap or cm again.
 */
Colormap
checkCM(cm, nwant)
Colormap cm;
int nwant;
{
	Display *dsp = XtDisplay(toplevel_w);
	char *inst;

	/* get the install resource value */
	inst = getXRes ("install", "guess");

	/* check each possible value */
	if (strcmp (inst, "no") == 0)
	    return (cm);
	else if (strcmp (inst, "yes") == 0)
	    return (createCM (nwant));
	else if (strcmp (inst, "guess") == 0) {
	    /* get a smattering of colors and opt for private cm if can't.
	     * we use alloc_ramp() because it verifies unique pixels.
	     * we use three to not overstress the resolution of colors.
	     */
	    Pixel *rr, *gr, *br;
	    int neach, nr, ng, nb;
	    XColor xc;

	    /* grab some to test */
	    neach = nwant/3;
	    xc.red = 255 << 8;
	    xc.green = 0;
	    xc.blue = 0;
	    rr = (Pixel *) malloc (neach * sizeof(Pixel));
	    nr = alloc_ramp (dsp, &xc, cm, rr, neach);
	    xc.red = 0;
	    xc.green = 255 << 8;
	    xc.blue = 0;
	    gr = (Pixel *) malloc (neach * sizeof(Pixel));
	    ng = alloc_ramp (dsp, &xc, cm, gr, neach);
	    xc.red = 0;
	    xc.green = 0;
	    xc.blue = 255 << 8;
	    br = (Pixel *) malloc (neach * sizeof(Pixel));
	    nb = alloc_ramp (dsp, &xc, cm, br, neach);

	    /* but don't keep them.
	     * N.B. alloc_ramp just cheats us with B&W if it returns 2.
	     */
	    if (nr > 2)
		XFreeColors (dsp, cm, rr, nr, 0);
	    if (ng > 2)
		XFreeColors (dsp, cm, gr, ng, 0);
	    if (nb > 2)
		XFreeColors (dsp, cm, br, nb, 0);
	    free ((void *)rr);
	    free ((void *)gr);
	    free ((void *)br);

	    if (nr + ng + nb < 3*neach)
		return (createCM(nwant));
	} else
	    printf ("Unknown install `%s' -- defaulting to No\n", inst);

	return (cm);
}

/* explicitly handle pending X events when otherwise too busy */
void
XCheck (app)
XtAppContext app;
{
        while ((XtAppPending (app) & XtIMXEvent) == XtIMXEvent)
	    XtAppProcessEvent (app, XtIMXEvent);
}

/* center the scrollbars in the given scrolled window */
void
centerScrollBars(sw_w)
Widget sw_w;
{
	int min, max, slidersize, value;
	XmScrollBarCallbackStruct sbcs;
	Widget sb_w;

	/* you would think setting XmNvalue would be enough but seems we
	 * must trigger the callback too, at least on MKS
	 */
	memset (&sbcs, 0, sizeof(sbcs));
	sbcs.reason = XmCR_VALUE_CHANGED;
	sbcs.event = NULL;	/* ? */

	get_something (sw_w, XmNhorizontalScrollBar, (XtArgVal)&sb_w);
	get_something (sb_w, XmNminimum, (XtArgVal)&min);
	get_something (sb_w, XmNmaximum, (XtArgVal)&max);
	get_something (sb_w, XmNsliderSize, (XtArgVal)&slidersize);
	sbcs.value = value = (min+max-slidersize)/2;
	set_something (sb_w, XmNvalue, (XtArgVal)value);
	XtCallCallbacks (sb_w, XmNvalueChangedCallback, &sbcs);

	get_something (sw_w, XmNverticalScrollBar, (XtArgVal)&sb_w);
	get_something (sb_w, XmNminimum, (XtArgVal)&min);
	get_something (sb_w, XmNmaximum, (XtArgVal)&max);
	get_something (sb_w, XmNsliderSize, (XtArgVal)&slidersize);
	sbcs.value = value = (min+max-slidersize)/2;
	set_something (sb_w, XmNvalue, (XtArgVal)value);
	XtCallCallbacks (sb_w, XmNvalueChangedCallback, &sbcs);	
}

/* set the XmNcolumns resource of the given Text or TextField widget to the
 * full length of the current string.
 */
void
textColumns (w)
Widget w;
{
	Arg args[10];
	char *bp;
	int n;

	if (XmIsText(w))
	    bp = XmTextGetString (w);
	else if (XmIsTextField(w))
	    bp = XmTextFieldGetString (w);
	else
	    return;

	n = 0;
	XtSetArg (args[n], XmNcolumns, strlen(bp)); n++;
	XtSetValues (w, args, n);

	XtFree (bp);

}

/* check the given XmText or XmTextField value:
 * if empty, fill with "x/y", or just "x" if !y.
 * then set size to accommodate if setcols.
 * while we're at it, fix the text cursor.
 */
void
defaultTextFN (w, setcols,  x, y)
Widget w;
int setcols;
char *x, *y;
{
	char *tp = XmTextGetString (w);

	if (tp[0] == '\0') {
	    char buf[1024];
	    if (y)
		sprintf (buf, "%s/%s", x, y);
	    else
		strcpy (buf, x);
	    XmTextSetString (w, buf);
	}
	XtFree (tp);

	if (setcols)
	    textColumns (w);

	fixTextCursor (w);
}

/* turn cursor on/off to follow focus */
static void
textFixCursorCB(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmAnyCallbackStruct *ap = (XmAnyCallbackStruct *)call;
	Arg a;

	XtSetArg (a, XmNcursorPositionVisible, ap->reason == XmCR_FOCUS);
	XtSetValues (w, &a, 1);
}

/* call just after creation to work around ugly grey cursor in idle Text or
 * TextField
 */
void
fixTextCursor (w)
Widget w;
{
	Arg a;

	XtSetArg (a, XmNcursorPositionVisible, False);
	XtSetValues (w, &a, 1);

	XtAddCallback (w, XmNfocusCallback, textFixCursorCB, 0);
	XtAddCallback (w, XmNlosingFocusCallback, textFixCursorCB, 0);
}

/* convert str to all lowercase IN PLACE */
char *
strtolower (char *str)
{
	char *s = str;

	/* actually faster to /not/ call isupper() first */
	do
	    *s = tolower (*s);
	while (*s++);

	return (str);
}

/* check whether op is within its valid date range at np.
 * if not make a note and return -1 else return 0.
 */
int
dateOK (Now *np, Obj *op)
{
	char dbuf[32];

	if (dateRangeOK (np, op) == 0)
	    return (0);

	fs_date (dbuf, pref_get(PREF_DATE_FORMAT), mjd);
	xe_msg (0, "Elements for %s are not valid on %s", op->o_name, dbuf); 
	return (-1);
}

/* set up look_like_button[] and look_like_label[] */
void
setButtonInfo()
{
	Pixel topshadcol, botshadcol, bgcol;
	Pixmap topshadpm, botshadpm;
	Widget sample;
	Arg args[20];
	int n;

	n = 0;
	sample = XmCreatePushButton (toplevel_w, "TEST", args, n);

	n = 0;
	XtSetArg (args[n], XmNtopShadowColor, &topshadcol); n++;
	XtSetArg (args[n], XmNbottomShadowColor, &botshadcol); n++;
	XtSetArg (args[n], XmNtopShadowPixmap, &topshadpm); n++;
	XtSetArg (args[n], XmNbottomShadowPixmap, &botshadpm); n++;
	XtSetArg (args[n], XmNbackground, &bgcol); n++;
	XtGetValues (sample, args, n);

	look_like_button[0].value = topshadcol;
	look_like_button[1].value = botshadcol;
	look_like_button[2].value = topshadpm;
	look_like_button[3].value = botshadpm;
	look_like_label[0].value = bgcol;
	look_like_label[1].value = bgcol;
	look_like_label[2].value = XmUNSPECIFIED_PIXMAP;
	look_like_label[3].value = XmUNSPECIFIED_PIXMAP;

	XtDestroyWidget (sample);
}

/* manipulate the given XmButton resources so it indeed looks like a button
 * or like a label.
 */
void
buttonAsButton (w, whether)
Widget w;
int whether;
{
	if (!look_like_inited) {
	    setButtonInfo();
	    look_like_inited = 1;
	}

	if (whether)
	    XtSetValues (w, look_like_button, XtNumber(look_like_button));
	else
	    XtSetValues (w, look_like_label, XtNumber(look_like_label));
}

/* pop up a dialog to allow aborting a lengthy operation.
 * then check occasionally using stopd_check()
 */
void
stopd_up()
{
	/* create if first time */
	if (!stopd_w) {
	    Widget rc_w, w;
	    Arg args[20];
	    int n;

	    /* create shell */

	    n = 0;
	    XtSetArg(args[n], XmNcolormap, xe_cm); n++;
	    XtSetArg(args[n], XmNtitle, "xephem Stop");  n++;
	    XtSetArg(args[n], XmNiconName, "Stop");  n++;
	    XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING);  n++;
	    stopd_w = XtCreatePopupShell("NetStop", topLevelShellWidgetClass,
							toplevel_w, args, n);
	    setup_icon (stopd_w);
	    set_something (stopd_w, XmNcolormap, (XtArgVal)xe_cm);
	    XtAddCallback (stopd_w, XmNpopupCallback, prompt_map_cb, NULL);

	    /* rc for controls */

	    n = 0;
	    XtSetArg(args[n], XmNisAligned, True);  n++;
	    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER);  n++;
	    XtSetArg(args[n], XmNspacing, 10);  n++;
	    XtSetArg(args[n], XmNmarginHeight, 10);  n++;
	    XtSetArg(args[n], XmNmarginWidth, 10);  n++;
	    rc_w = XmCreateRowColumn (stopd_w, "SRC", args, n);
	    XtManageChild (rc_w);

	    /* label and stop button */

	    n = 0;
	    w = XmCreateLabel (rc_w, "SL", args, n);
	    set_xmstring (w, XmNlabelString, " Press Stop to cancel... ");
	    XtManageChild (w);

	    n = 0;
	    w = XmCreatePushButton (rc_w, "Stop", args, n);
	    XtAddCallback (w, XmNactivateCallback, stopd_cb, NULL);
	    XtManageChild (w);
	}

	/* bring it up for sure */
	XtPopdown (stopd_w);    /* so Popup causes position and raise */
	XtPopup (stopd_w, XtGrabNone);
	set_something (stopd_w, XmNiconic, (XtArgVal)False);
	XFlush (XtDisplay(stopd_w));
	XmUpdateDisplay (stopd_w);
	XSync (XtDisplay(stopd_w), 0);

	/* init button not pressed */
	stopd_stopped = 0;
}

/* bring down the user-stop dialog */
void
stopd_down()
{
	if (stopd_w)
	    XtPopdown (stopd_w);
	stopd_stopped = 0;
}

/* after calling stopd_up(), poll this to check whether the user wants to stop.
 * return -1 to stop, else 0.
 */
int
stopd_check()
{
	/* check for user button presses */
	XCheck (xe_app);

	if (stopd_stopped) {
	    stopd_stopped = 0;
	    return (-1);
	}
	return (0);
}

/* called when the user presses the Stop button */
/* ARGSUSED */
static void
stopd_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	xe_msg (0, "User stop");
	stopd_stopped = 1;
}

/* compare two pointers to strings, qsort style */
static int
strc_qs (const void *v1, const void *v2)
{
	return (strcmp (*(char **)v1, *(char **)v2));
}

/* called when a pulldown built by createFSM, w, is coming up.
 * client is an FSBInfo.
 */
static void
FSMfillcb (Widget w, XtPointer client, XtPointer call)
{
	FSMInfo *fsm = (FSMInfo *) client;
	WidgetList chil;
	Cardinal nchil;
	Widget pb_w;
	char **files;
	int nfiles;
	char dirs[2][1024];
	int i, j, k;

	/* init dirs and file list, private first */
	sprintf (dirs[0], "%s", getPrivateDir());
	sprintf (dirs[1], "%s/%s", getShareDir(), fsm->sharedir);
	files = NULL;
	nfiles = 0;

	/* scan each dir for each suffix, use only first one seen */
	for (i = 0; i < 2; i++) {
	    DIR *dirp = opendir (dirs[i]);
	    struct dirent *dp;
	    if (!dirp)
		continue;
	    while ((dp = readdir(dirp)) != NULL) {
		for (j = 0; j < fsm->nsuffixes; j++) {
		    int sl = strlen(fsm->suffixes[j]);
		    int fl = strlen(dp->d_name);
		    int dl = fl - sl;
		    if (dl > 0 && !strcmp (dp->d_name+dl, fsm->suffixes[j])) {
			for (k = 0; k < nfiles; k++)
			    if (strcmp (dp->d_name, files[k]) == 0)
				break;	/* already seen */
			if (k == nfiles) {
			    files = (char **) XtRealloc ((char *)files,
						    (nfiles+1)*sizeof(char*));
			    files[nfiles++] = XtNewString (dp->d_name);
			}
		    }
		}
	    }
	    closedir (dirp);
	}

	/* sort */
	qsort (files, nfiles, sizeof(char *), strc_qs);

	/* put each file into pulldown, add if more now */
	get_something (w, XmNnumChildren, (XtArgVal)&nchil);
	for (i = 0; i < nfiles; i++) {
	    if (i < nchil) {
		/* get fresh pointer in case it moves when grown */
		get_something (w, XmNchildren, (XtArgVal)&chil);
		pb_w = chil[i];
	    } else {
		/* add new */
		pb_w = XmCreatePushButton (w, "FSMPB", NULL, 0);
		XtAddCallback (pb_w, XmNactivateCallback, fsm->cb, NULL);
		nchil++;
	    }
	    set_xmstring (pb_w, XmNlabelString, files[i]);
	    XtFree (files[i]);
	    XtManageChild (pb_w);
	}
	XtFree ((char *)files);

	/* multi-column if really long list */
	if (nfiles >= 3)
	    set_something (w,XmNnumColumns,(XtArgVal)((int)(sqrt(nfiles/3.0))));
	else
	    set_something (w, XmNnumColumns, (XtArgVal)1);

	/* turn off any extra PBs */
	get_something (w, XmNchildren, (XtArgVal)&chil);
	for (; i < nchil; i++)
	    XtUnmanageChild (chil[i]);
}

/* create a pulldown that fills itself with names of all files in Shared/sdir
 * and Private with any of the given suffixes. If parent p is a MenuBar we
 * build a pulldown triggered by a cascade button and return the cascade button,
 * else we build and return an option menu. cb will be called when the user
 * selects an item in the menu, it should get file name from the labelString
 * resource of the w.
 * N.B. we assume memory for sdir and each entry in suffix[] are persistent.
 * N.B. we malloc one new FSMInfo for each pulldown.
 * N.B. each suffix must include leading '.'
 */
Widget
createFSM (Widget p, char **suffix, int nsuffix, char *sdir, XtCallbackProc cb)
{
	FSMInfo *fsm;
	Widget pd_w, mgr_w;
	unsigned char rctype;
	Arg args[20];
	int n;

	/* gather the info */
	fsm = (FSMInfo *) XtMalloc (sizeof(FSMInfo));
	fsm->suffixes = (char **) XtMalloc (nsuffix * sizeof(char *));
	fsm->nsuffixes = nsuffix;
	memcpy (fsm->suffixes, suffix, nsuffix * sizeof(char *));
	fsm->sharedir = sdir;
	fsm->cb = cb;

	/* create the pulldown, allow for multicolumn, connect fsm cb */
	n = 0;
	pd_w = XmCreatePulldownMenu (p, "FSM", args, n);
	set_something (pd_w, XmNpacking, (XtArgVal)XmPACK_COLUMN);
	XtAddCallback (pd_w, XmNmapCallback, FSMfillcb, (XtPointer)fsm);

	/* create the pulldown's manager */
	n = 0;
	XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	if (XmIsRowColumn(p) && (get_something (p, XmNrowColumnType,
				    (XtArgVal)&rctype), rctype == XmMENU_BAR)) {
	    mgr_w = XmCreateCascadeButton (p, "FSBCB", args, n);
	    set_xmstring (mgr_w, XmNlabelString, "Files");
	} else {
	    /* put one entry for something to show in putton */
	    Widget pb_w = XmCreatePushButton (pd_w, "XX", NULL, 0);
	    XtAddCallback (pb_w, XmNactivateCallback, cb, NULL);
	    XtManageChild (pb_w);
	    set_xmstring (pb_w, XmNlabelString, "Make selection");
	    mgr_w = XmCreateOptionMenu (p, "FSBOM", args, n);
	}
	XtManageChild (mgr_w);

	return (mgr_w);
}

/* look up xcp->pixel in xe_cm and fill in xcp->red/green/blue.
 * keep a cache of known values to reduce server round trips.
 * if xcp == NULL reset the cache collection.
 */
void
pixCache (XColor *xcp)
{
	static XColor *xc;
	static int nxc;
	int t, b;
	
	/* just reset if no xcp */
	if (!xcp) {
	    if (xc) {
		free (xc);
		xc = NULL;
	    }
	    nxc = 0;
	    return;
	}

	/* binary search */
	t = nxc - 1;
	b = 0;
	while (b <= t) {
	    int m = (t+b)/2;
	    XColor *mxcp = &xc[m];
	    if (xcp->pixel == mxcp->pixel) {
		*xcp = *mxcp;
		return;
	    }
	    if (xcp->pixel < mxcp->pixel)
		t = m-1;
	    else
		b = m+1;
	}

	/* not found, get the r/g/b */
	XQueryColor (XtDisplay(toplevel_w), xe_cm, xcp);

	/* add to cache in ascending order */
	xc = xc ? realloc(xc, (nxc+1)*sizeof(XColor)) : malloc(sizeof(XColor));
	for (t = nxc-1; t >= 0 && xc[t].pixel > xcp->pixel; --t)
	    xc[t+1] = xc[t];
	xc[t+1] = *xcp;
	nxc++;
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: xmisc.c,v $ $Date: 2006/04/10 09:00:06 $ $Revision: 1.57 $ $Name:  $"};

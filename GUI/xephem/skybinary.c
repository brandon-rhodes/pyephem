/* code to display a binary star orbit
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>

#include "xephem.h"

#define	PSTARD	6		/* primary star diameter, pixels */
#define	FSTARD	4		/* future star diameter, pixels */
#define	SSTARD	2		/* secondary star diameter, pixels */
#define	NPLOTS	50		/* points to plot orbit */

static void bs_create_shell_w (void);
static void bs_exp_cb (Widget w, XtPointer client, XtPointer call);
static void bs_close_cb (Widget w, XtPointer client, XtPointer call);
static void bs_help_cb (Widget w, XtPointer client, XtPointer call);
static void bs_print_cb (Widget w, XtPointer client, XtPointer call);
static void bs_print (void);
static void bs_annotate (Now *np);
static void bs_update(void);
static void bs_refresh(void);
static void bs_discrete(Obj *op);
static void bs_orbit(Obj *op);
static void bs_mkgcs(void);

static Widget bsshell_w;	/* main shell */
static Widget bstable_w;	/* ephemerides text */
static Widget bsda_w;		/* orbit drawing area */
static Pixmap bs_pm;		/* off screen drawing staging area */
static Obj bsobj;		/* record of binary to redraw */
static Pixel annot_p;		/* annotation color */
static Pixel skybg_p;		/* sky background color */
static GC bs_gc;		/* drawing context */
static int bsmapw, bsmaph;	/* map size */

static char skybscategory[] = "Sky View -- Binary system map";/*Save category */

/* called to display op, a BINARYSTAR.
 */
void
svbs_manage (Obj *op)
{
	if (!bsshell_w) {
	    bs_create_shell_w();
	    bs_mkgcs();
	}
	
	if (!is_type(op, BINARYSTARM)) {
	    printf ("svbs called without BIN type %d\n", op->o_type);
	    abort();
	}

	/* save locally for bs_update */
	bsobj = *op;

	if (isUp (bsshell_w))
	    bs_update ();               /* already up, just draw */
	else
	    XtManageChild (bsshell_w);  /* expose will update */
}

/* called when basic resources change.
 * rebuild and redraw.
 */
void
svbs_newres()
{
	if (!bsshell_w)
	    return;
	bs_mkgcs();
	bs_update();
}

/* create bsshell_w, the top view dialog */
static void
bs_create_shell_w()
{
	typedef struct {
	    char *label;
	    char *tip;
	    XtCallbackProc cb;
	} Ctrl;
	static Ctrl ctrls[] = {
	    {"Close", "Close this window", bs_close_cb},
	    {"Print", "Print this window", bs_print_cb},
	    {"Help",  "More info about this window", bs_help_cb}
	};
	Widget w, f_w;
	Arg args[20];
	int i;
	int n;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	bsshell_w = XmCreateFormDialog (svshell_w, "BinaryStar", args, n);
	set_something (bsshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (XtParent(bsshell_w),"XEphem*BinaryStar.width",skybscategory,0);
	sr_reg (XtParent(bsshell_w),"XEphem*BinaryStar.height",skybscategory,0);
	sr_reg (XtParent(bsshell_w),"XEphem*BinaryStar.x",skybscategory,0);
	sr_reg (XtParent(bsshell_w),"XEphem*BinaryStar.y",skybscategory,0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Binary system map"); n++;
	XtSetValues (XtParent(bsshell_w), args, n);

	/* controls across the bottom */

	for (i = 0; i < XtNumber(ctrls); i++) {
	    Ctrl *cp = &ctrls[i];

	    n = 0;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 100*(1+i*4)/(4*XtNumber(ctrls)+1)); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 100*(4+i*4)/(4*XtNumber(ctrls)+1)); n++;
	    w = XmCreatePushButton (bsshell_w, "BSPB", args, n);
	    XtAddCallback (w, XmNactivateCallback, cp->cb, NULL);
	    set_xmstring (w, XmNlabelString, cp->label);
	    wtip (w, cp->tip);
	    XtManageChild (w);
	}

	/* drawing area in a frame on left half */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 50); n++;
	f_w = XmCreateFrame (bsshell_w, "BSDAF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    bsda_w = XmCreateDrawingArea (f_w, "Map", args, n);
	    XtAddCallback (bsda_w, XmNexposeCallback, bs_exp_cb, NULL);
	    XtManageChild (bsda_w);

	/* scrolled text on right */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, f_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNeditable, False); n++;
	XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	XtSetArg (args[n], XmNblinkRate, 0); n++;
	bstable_w = XmCreateScrolledText (bsshell_w, "BSST", args, n);
	XtManageChild (bstable_w);

}

/* callback from either expose or resize of the topview.
 */
/* ARGSUSED */
static void
bs_exp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
        Window win = XtWindow(w);
	Display *dsp = XtDisplay(w);
	unsigned int nx, ny, bw, d;
	Window root;
	int x, y;

	/* filter out a few oddball cases */
	switch (c->reason) {
	case XmCR_EXPOSE: {
	    /* turn off gravity so we get expose events for either shrink or
	     * expand.
	     */
	    static int before;
	    XExposeEvent *e = &c->event->xexpose;

	    if (!before) {
		XSetWindowAttributes swa;
                unsigned long mask = CWBitGravity | CWBackingStore;

		swa.bit_gravity = ForgetGravity;
		swa.backing_store = NotUseful; /* we use a pixmap */
		XChangeWindowAttributes (dsp, win, mask, &swa);
		before = 1;
	    }

	    /* wait for the last in the series */
	    if (e->count != 0)
		return;
	    break;
	    }
	default:
	    printf ("Unexpected bsda_w event. type=%d\n", c->reason);
	    abort();
	}

        XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);
	if (!bs_pm || (int)nx != bsmapw || (int)ny != bsmaph) {
	    if (bs_pm)
		XFreePixmap (dsp, bs_pm);
	    bs_pm = XCreatePixmap (dsp, win, nx, ny, d);
	    bsmapw = nx;
	    bsmaph = ny;
	}

	/* recompute, might be new object */
	bs_update ();
}

/* update bs_pm and ephemerides from bsobj then refresh */
static void
bs_update()
{
	Obj *op = &bsobj;

	if (op->b_nbp > 0)
	    bs_discrete(op);
	else
	    bs_orbit(op);

	bs_refresh();
}

/* plot discrete binary positions */
static void
bs_discrete (Obj *op)
{
	BinPos *bpp = op->b_bp;
	Obj secondary;
	XArc xa[MAXBINPOS];
	GC stargc;
	int x0, y0, x, y;
	double maxsep;
	double nicescale, ppas;
	char buf[1024];
	int l, txtl;
	int i;

	/* print header, center if short */
	i = strlen(op->o_name);
	i = i < 13 ? (13-strlen(op->o_name))/2 : 0;
	txtl = 0;
	l = sprintf (buf, "%*sBinary system %s\n", i, "", op->o_name);
	XmTextSetString (bstable_w, buf);
	txtl += l;
	l = sprintf (buf, "      Year      Sep    PA\n");
	XmTextInsert (bstable_w, txtl, buf);
	txtl += l;

	/* find largest separation */
	maxsep = 0;
	for (i = 0; i < op->b_nbp; i++)
	    if (bpp[i].bp_sep > maxsep)
		maxsep = bpp[i].bp_sep;

	/* set center and pixels/arcsec scale */
	x0 = bsmapw/2;
	y0 = bsmaph/2;
	ppas = 0.35 * (bsmapw>bsmaph ? bsmaph/maxsep : bsmapw/maxsep);

	/* erase */
	XSetForeground (XtD, bs_gc, skybg_p);
	XFillRectangle (XtD, bs_pm, bs_gc, 0, 0, bsmapw, bsmaph);

	/* primary star in center */
	obj_pickgc(op, toplevel_w, &stargc);
	xa[0].x = x0-PSTARD/2;
	xa[0].y = y0-PSTARD/2;
	xa[0].width = xa[0].height = PSTARD;
	xa[0].angle1 = 0;
	xa[0].angle2 = 360*64;
	XPSFillArcs (XtD, bs_pm, stargc, &xa[0], 1);

	/* convenient scale indicator */
	XSetForeground (XtD, bs_gc, annot_p);
	nicescale = pow (10.0, floor(log10(2*maxsep)));
	i = (int)floor(ppas*nicescale + 0.5);
	y = y0 + (int)floor(1.2*ppas*maxsep);
	XPSDrawLine (XtD, bs_pm, stargc, x0-i/2, y, x0+i/2, y);
	l = sprintf (buf, "%g\"", nicescale);
	XPSDrawString (XtD, bs_pm, bs_gc, x0-i/2, y+15, buf, l);

	/* N/E annotation */
	y = y0 - (int)floor(1.1*ppas*maxsep);
	XPSDrawString (XtD, bs_pm, bs_gc, x0-3, y, "N", 1);
	x = x0 - (int)floor(1.1*ppas*maxsep);
	XPSDrawString (XtD, bs_pm, bs_gc, x, y0+5, "E", 1);

	memset (&secondary, 0, sizeof(secondary));
	secondary.o_type = FIXED;
	secondary.f_class = 'S';
	strcpy (secondary.o_name, "BinaryTemp");
	memmove (secondary.f_spect, op->b_2spect, sizeof(secondary.f_spect));
	obj_pickgc(&secondary, toplevel_w, &stargc);

	for (i = 0; i < op->b_nbp; i++) {
	    XArc *xap = &xa[i];

	    xap->width = xap->height = FSTARD;

	    l = sprintf (buf, "%2d  %8.3f %7.2f  %5.1f\n", i+1, bpp[i].bp_ep,
					bpp[i].bp_sep, raddeg(bpp[i].bp_pa));
	    XmTextInsert (bstable_w, txtl, buf);
	    txtl += l;

	    xap->angle1 = 0;
	    xap->angle2 = 360*64;
	    xap->x = x0 - (int)floor(ppas * bpp[i].bp_sep * sin(bpp[i].bp_pa)
							+ 0.5) - xap->width/2;
	    xap->y = y0 - (int)floor(ppas * bpp[i].bp_sep * cos(bpp[i].bp_pa)
							+ 0.5) - xap->height/2;

	}

	XPSFillArcs (XtD, bs_pm, stargc, xa, op->b_nbp);

	for (i = 0; i < op->b_nbp; i++) {
	    l = sprintf (buf, "%d", i+1);
	    XPSDrawString (XtD, bs_pm, bs_gc, xa[i].x+5, xa[i].y-5, buf, l);
	}
}


/* plot one orbit using orbital elements */
static void
bs_orbit (Obj *op)
{
	typedef struct { double yr, pa, sep; } BSO;
	Now *np = mm_get_now(), now = *np;
	BinOrbit *bop = &op->b_bo;
	double P = op->b_bo.bo_P;	/* period in years */
	double yr0;			/* starting year */
	Obj secondary;
	XArc xa[NPLOTS+2];
	BSO bso[NPLOTS+2];
	double ticks[NPLOTS+2];
	GC stargc;
	int x0, y0, sz, x, y;
	double maxsep, nicescale, ppas;
	char buf[1024];
	int l, txtl, nt;
	int i;

	/* find nice orbit marks no more than once around */
	mjd_year (mjd, &yr0);
	nt = tickmarks (yr0, yr0+P, NPLOTS, ticks);
	while (ticks[nt-1] >= ticks[0]+P)
	    --nt;

	/* compute orbit points, and find max sep */
	maxsep = 0;
	for (i = 0; i < nt; i++) {
	    BSO *bsp = &bso[i];
	    double yr = ticks[i];

	    year_mjd (yr, &now.n_mjd);
	    op->b_2compute = 1;
	    obj_cir (&now, op);

	    bsp->yr = yr;
	    bsp->sep = bop->bo_sep;
	    bsp->pa = bop->bo_pa;

	    if (bsp->sep > maxsep)
		maxsep = bsp->sep;
	}

	/* set center and pixels/arcsec scale */
	x0 = bsmapw/2;
	y0 = bsmaph/2;
	sz = bsmapw>bsmaph ? bsmaph : bsmapw;
	ppas = (sz-50)/(2*maxsep);

	/* erase */
	XSetForeground (XtD, bs_gc, skybg_p);
	XFillRectangle (XtD, bs_pm, bs_gc, 0, 0, bsmapw, bsmaph);

	/* plot primary star in center */
	obj_pickgc(op, toplevel_w, &stargc);
	xa[0].x = x0-PSTARD/2;
	xa[0].y = y0-PSTARD/2;
	xa[0].width = xa[0].height = PSTARD;
	xa[0].angle1 = 0;
	xa[0].angle2 = 360*64;
	XPSFillArcs (XtD, bs_pm, stargc, &xa[0], 1);

	/* draw scale indicator */
	XSetForeground (XtD, bs_gc, annot_p);
	nicescale = pow (10.0, floor(log10(2*bop->bo_a)));
	i = (int)floor(ppas*nicescale + 0.5);
	y = y0 + sz/2 - 20;
	XPSDrawLine (XtD, bs_pm, stargc, x0-i/2, y, x0+i/2, y);
	l = sprintf (buf, "%g\"", nicescale);
	XPSDrawString (XtD, bs_pm, bs_gc, x0-i/2, y+15, buf, l);

	/* N/E annotation */
	y = y0 - sz/2 + 20;
	XPSDrawString (XtD, bs_pm, bs_gc, x0-3, y, "N", 1);
	x = x0 - sz/2 + 10;
	XPSDrawString (XtD, bs_pm, bs_gc, x, y0+5, "E", 1);

	/* print table header, center if short */
	i = strlen(op->o_name);
	i = i < 13 ? (13-strlen(op->o_name))/2 : 0;
	txtl = 0;
	l = sprintf (buf, "%*sOne orbit for %s\n", i, "", op->o_name);
	XmTextSetString (bstable_w, buf);
	txtl += l;
	l = sprintf (buf, "      Year      Sep    PA\n");
	XmTextInsert (bstable_w, txtl, buf);
	txtl += l;

	/* pick a GC representing the spectral class of the secondary */
	memset (&secondary, 0, sizeof(secondary));
	secondary.o_type = FIXED;
	secondary.f_class = 'S';
	strcpy (secondary.o_name, "BinaryTemp");
	memmove (secondary.f_spect, op->b_2spect, sizeof(secondary.f_spect));
	obj_pickgc(&secondary, toplevel_w, &stargc);

	/* plot the secondary and fill in the table */
	for (i = 0; i < nt; i++) {
	    BSO *bsp = &bso[i];
	    XArc *xap = &xa[i];

	    if (i == 0)
		xap->width = xap->height = PSTARD;
	    else if (i<nt-1 && ((i+1)%10)==0)
		xap->width = xap->height = FSTARD;
	    else
		xap->width = xap->height = SSTARD;

	    l = sprintf (buf, "%2d  %8.3f %7.2f  %5.1f\n", i+1, bsp->yr,
						    bsp->sep, raddeg(bsp->pa));
	    XmTextInsert (bstable_w, txtl, buf);
	    txtl += l;

	    xap->angle1 = 0;
	    xap->angle2 = 360*64;
	    xap->x = x0 - (int)floor(ppas * bsp->sep * sin(bsp->pa) + 0.5)
								- xap->width/2;
	    xap->y = y0 - (int)floor(ppas * bsp->sep * cos(bsp->pa) + 0.5)
								- xap->height/2;

	}
	XPSFillArcs (XtD, bs_pm, stargc, xa, nt);

	/* add some labels */
	for (i = 0; i < nt; i++) {
	    if (i == 0 || (i<nt-1 && ((i+1)%10)==0)) {
		int xoff = xa[i].x > bsmapw/2 ? 5 : -10;
		int yoff = xa[i].y > bsmaph/2 ? 15 : -5;
		l = sprintf (buf, "%d", i+1);
		XPSDrawString (XtD, bs_pm, bs_gc, xa[i].x+xoff, xa[i].y+yoff,
									buf, l);
	    }
	}
}

/* display bs_pm */
static void
bs_refresh ()
{
	XCopyArea (XtD, bs_pm, XtWindow(bsda_w), bs_gc, 0, 0,bsmapw,bsmaph,0,0);
}

/* ARGSUSED */
static void
bs_close_cb (Widget w, XtPointer client, XtPointer call)
{
	XtUnmanageChild (bsshell_w);
}

/* ARGSUSED */
static void
bs_print_cb (Widget w, XtPointer client, XtPointer call)
{
	XPSAsk ("Binary system map", bs_print);
}

/* ARGSUSED */
static void
bs_help_cb (Widget w, XtPointer client, XtPointer call)
{
	static char *msg[] = {
	    "Shows a binary system map and ephemeris from now"
	};
	hlp_dialog ("SkyView_binary", msg, sizeof(msg)/sizeof(msg[0]));
}

static void
bs_mkgcs()
{
	XFontStruct *vfp;

	(void) get_color_resource (bsda_w, "SkyAnnotColor", &annot_p);
	(void) get_color_resource (bsda_w, "SkyColor", &skybg_p);
		        
	bs_gc = XCreateGC (XtD, RootWindow (XtD, DefaultScreen(XtD)), 0L, NULL);
	get_views_font (XtD, &vfp);
	XSetFont (XtD, bs_gc, vfp->fid);
}

static void
bs_print (void)
{
	Now *np = mm_get_now();
	int x0, y0, w, h;

	if (!isUp(bsshell_w)) {
	    xe_msg (1, "Binary system map must be open to print.");
	    XPSClose();
	    return;
	}

	watch_cursor(1);

	/* chop out square center */
	if (bsmapw > bsmaph) {
	    w = h = bsmaph;
	    x0 = (bsmapw - bsmaph)/2;
	    y0 = 0;
	} else {
	    w = h = bsmapw;
	    x0 = 0;
	    y0 = (bsmaph - bsmapw)/2;
	}
	XPSXBegin (bs_pm, x0, y0, w, h, 1*72, 10*72, (int)(3.5*72));

	/* redraw */
	bs_update();

	/* no more X captures */
	XPSXEnd();

	/* add some extra info */
	bs_annotate (np);

	/* finished */
	XPSClose();

	watch_cursor(0);
}

static void
bs_annotate (Now *np)
{
	char *txt = XmTextGetString (bstable_w);
	char *tcpy = XtNewString (txt);
	char buf[1024];
	char *nlp, *t0;
	int r = 70;

	XPSDirect ("/Courier findfont 8 scalefont setfont\n");

	for (t0 = tcpy; (nlp = strchr (t0, '\n')) != NULL; t0 = nlp+1) {
	    *nlp = '\0';
	    sprintf (buf, "(%s) 350 %d lstr\n", t0, AROWY(r--));
	    XPSDirect (buf);
	}

	XPSDirect ("/Helvetica findfont 8 scalefont setfont\n");

	XtFree (txt);
	XtFree (tcpy);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: skybinary.c,v $ $Date: 2004/06/17 15:19:53 $ $Revision: 1.12 $ $Name:  $"};

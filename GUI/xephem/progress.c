/* this is a simple progress meter facility.
 * the graphics are easy; the harder part is using it well.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>

#include "xephem.h"

extern char maincategory[];

static void pm_createshell (void);
static void pm_close_cb (Widget w, XtPointer client, XtPointer call);
static void pm_exp_cb (Widget w, XtPointer client, XtPointer call);
static void show_progress (int p, int force);
static void make_gc (void);

static Widget pmshell_w;	/* the main shell */
static Widget pm_da;		/* graphics drawing area */
static Widget p_w;		/* percentage label */

static unsigned long pm_fgpix;	/* drawing meter foreground color */
static unsigned long pm_bgpix;	/* drawing meter background color */
static int last_per = 100;	/* last percentage drawn */
static int cur_per;		/* current percentage (for exp) */
static long t0;			/* initial time at this run */
static int p0;			/* initial percentage at this run */
static GC pm_gc;		/* GC */
static int nup;			/* up/down stack */

/* called to manage the progress meter */
void
pm_manage()
{
	if (!pmshell_w)
	    pm_createshell();

	if (!isUp(pmshell_w)) {
	    XtPopup (pmshell_w, XtGrabNone);
	    set_something (pmshell_w, XmNiconic, (XtArgVal)False);
	    nup = 1;
	}
}

/* insure the progress meter is visible */
void
pm_up()
{
	if (!pmshell_w)
	    pm_createshell();

	XtPopdown (pmshell_w);    /* so Popup causes raise */
	XtPopup (pmshell_w, XtGrabNone);
	set_something (pmshell_w, XmNiconic, (XtArgVal)False);

	nup++;
}

/* pop an instance of the progress meter, and unmanage if goes to 0 */
void
pm_down()
{
	if (!pmshell_w)
	    pm_createshell();

	if (--nup <= 0) {
	    nup = 0;
	    XtPopdown (pmshell_w);
	}
}

/* set the given percentage on the progress meter.
 */
void
pm_set (p)
int p;
{
	if (!isUp(pmshell_w))
	    return;
	show_progress (p, 0);
}

void
pm_cursor(c)
Cursor c;
{
	Window win;

	if (pmshell_w && (win = XtWindow(pmshell_w)) != 0) {
	    Display *dsp = XtDisplay(pmshell_w);
	    if (c)
		XDefineCursor(dsp, win, c);
	    else
		XUndefineCursor(dsp, win);
	}
}

/* create the progress meter dialog */
static void
pm_createshell()
{
	Arg args[20];
	Widget pm_formw, fr_w, c_w;
	int n;

	/* create shell and form */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Progress"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	XtSetArg (args[n], XmNheight, 105); n++;
	XtSetArg (args[n], XmNwidth, 210); n++;
	pmshell_w = XtCreatePopupShell ("Progress", topLevelShellWidgetClass,
							toplevel_w, args,n);
	setup_icon (pmshell_w);
	set_something (pmshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (pmshell_w, "XEphem*Progress.x", maincategory, 0);
	sr_reg (pmshell_w, "XEphem*Progress.y", maincategory, 0);

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	pm_formw = XmCreateForm (pmshell_w, "Progress", args,n);
	XtManageChild (pm_formw);

	/* Close PB on the bottom */
	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 30); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 70); n++;
	c_w = XmCreatePushButton (pm_formw, "Close", args, n);
	XtAddCallback (c_w, XmNactivateCallback, pm_close_cb, NULL);
	wtip (c_w, "Close this dialog");
	XtManageChild (c_w);

	/* the progress label */
	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, c_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	p_w = XmCreateLabel (pm_formw, "Progress", args, n);
	wtip (p_w, "Percentage complete and time remaining");
	XtManageChild (p_w);

	/* the drawing area in a frame */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, p_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNmarginWidth, 5); n++;
	XtSetArg (args[n], XmNmarginHeight, 5); n++;
	fr_w = XmCreateFrame (pm_formw, "F", args, n);
	XtManageChild (fr_w);

	    n = 0;
	    XtSetArg (args[n], XmNheight, 20); n++;
	    XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
	    pm_da = XmCreateDrawingArea (fr_w, "DA", args, n);
	    XtAddCallback (pm_da, XmNexposeCallback, pm_exp_cb, NULL);
	    wtip (pm_da,
		"Progress bar moves left to right to show 0 .. 100% complete");
	    XtManageChild (pm_da);
}

/* called from the Close PB */
/* ARGSUSED */
static void
pm_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (pmshell_w);
}

/* called when the drawing area gets an expose.
 */
/* ARGSUSED */
static void
pm_exp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;

	switch (c->reason) {
	case XmCR_EXPOSE: {
	    /* turn off gravity so we get expose events for either shrink or
	     * expand.
	     */
	    static int before;
	    XExposeEvent *e = &c->event->xexpose;

	    if (!before) {
		XSetWindowAttributes swa;
                unsigned long mask = CWBitGravity;

		swa.bit_gravity = ForgetGravity;
		XChangeWindowAttributes (e->display, e->window, mask, &swa);
		before = 1;
	    }

	    /* wait for the last in the series */
	    if (e->count != 0)
		return;
	    break;
	    }
	default:
	    printf ("Unexpected progress event. type=%d\n", c->reason);
	    abort();
	}

	show_progress (cur_per, 1);
}

/* display the given percentage numerically and graphically.
 * do as little as possible if we aren't going to do any drawing.
 * update cur_per and last_per when finished.
 */
static void
show_progress (p, force)
int p;
int force;
{
	int new = 0;

	if (!pm_gc)
	    make_gc();

	/* be kind */
	if (p > 100)
	    p = 100;
	if (p < 0)
	    p = 0;

	/* start when see a lower percentage */
	if (p < last_per) {
	    /* new run when new precentage is less than prev (or first time) */
	    t0 = time(NULL);
	    p0 = p;
	    new = 1;
	}

	/* draw if new, forced or changed */
	if (new || force || p != last_per) {
	    Window win = XtWindow(pm_da);
	    Dimension daw, dah;
	    int perw;
	    long t1 = time(NULL);
	    char buf[64];

	    /* in case before realized (managed??) */
	    if (!win)
		return;

	    /* numerically */

	    if (p-p0 >= 5 && t1-t0 >= 5) { /* minimum before estimating */
		int dt = (100-p)*(t1 - t0)/(p - p0);
		(void) sprintf (buf, "%3d%%  %3d:%02d", p, dt/60, dt%60);
	    } else {
		(void) sprintf (buf, "%d%%", p);
	    }
	    set_xmstring (p_w, XmNlabelString, buf);

	    /* graphically */

	    get_something (pm_da, XmNwidth, (XtArgVal)&daw);
	    get_something (pm_da, XmNheight, (XtArgVal)&dah);

	    perw = p*(int)daw/100;

	    XSetForeground (XtD, pm_gc, pm_fgpix);
	    XFillRectangle (XtD, win, pm_gc, 0, 0, perw, (int)dah);
	    XSetForeground (XtD, pm_gc, pm_bgpix);
	    XFillRectangle (XtD, win, pm_gc, perw, 0, (int)daw-perw, (int)dah);

	    XmUpdateDisplay (toplevel_w);
	}

	/* update history */
	last_per = cur_per;
	cur_per = p;
}

/* make the pm_gc GC and extablish the pm_{f,b}gpix pixels. */
static void
make_gc()
{
	pm_gc = XCreateGC (XtD, XtWindow(pm_da), 0L, NULL);

	get_something (pm_da, XmNforeground, (XtArgVal)&pm_fgpix);
	get_something (pm_da, XmNbackground, (XtArgVal)&pm_bgpix);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: progress.c,v $ $Date: 2009/01/05 20:55:54 $ $Revision: 1.17 $ $Name:  $"};

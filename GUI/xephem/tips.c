/* this file contains code to manage the widget tips facility.
 *
 * N.B. this code assumes the strings registered with wtip() live for the
 * lifetime of the widget. If this is not the case, malloced copies must be
 * kept here. This is performed if #define MALLOC_COPIES is set. This is not
 * the default to avoid needlessly peppering the heap with long-lived copies.
 */

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>	/* easy way to bring in Xt stuff is all */

#include "xephem.h"


/* keep a list of the tips for each registered widget.
 * w == 0 is used to mark a recycled entry.
 */
typedef enum {
    T_DOWN, T_ARMED, T_UP
} TState;
typedef struct {
    Widget w;		/* the widget for this tip */
    String s;		/* its text */
    TState state;	/* down, armed or up */
} Tip;

void wtip_init (void);

static void tip_new (Widget w, String s);
static void tip_reclaim (Widget w);
static void tip_eh (Widget w, XtPointer c, XEvent *e, Boolean *cont);
static void tip_startup (Tip *tp);
static void timer_cb (XtPointer client, XtIntervalId *id);
static void tip_up (Tip *tp);
static void tip_down (Tip *tp);
static void tip_destroy_cb (Widget w, XtPointer client, XtPointer call);
static Tip *tip_find (Widget w);
static int tip_sf (const void * a1, const void * a2);
static void tip_sort (void);

#define	GAP		5	/* gap around text, pixels */
#define	BW		1	/* window border width */
#define	VOFFSET		15	/* dist from pointer to window top, pixels */
#define	EDGE		10	/* min space to edge of screen */
#define	TIPTO		1100	/* time to wait before raising tip, ms */
#define	MAXDW		1280	/* max dsp width, beware large virtual roots */

static Tip *tips;		/* all current tips -- w==0 when not in use */
static int ntips;		/* total tips in tips[] */
static Window the_w;		/* tips window */
static GC the_gc;		/* tips gc */
static XFontStruct *the_fs;	/* tips font */
static Pixel the_fg, the_bg;	/* foreground and background colors */
static int tips_sorted;		/* flag set when tips array is sorted */

/* connect the given string as the tip for the given widget */
void
wtip (w, tip)
Widget w;
char *tip;
{
	/* gadgets don't have windows and hence no events or even a XtDisplay */
	if (XmIsGadget (w)) {
	    fprintf (stderr, "Widget is really a Gadget. tip=%s\n", tip);
	    return;
	}

	/* one-time initialization */
	if (!the_w)
	    wtip_init();

	/* connect handlers to this widget.
	 * we tried using translations but it interfered too much
	 */
	XtAddEventHandler (w, EnterWindowMask, False, tip_eh, (XtPointer)1);
	XtAddEventHandler (w, LeaveWindowMask, False, tip_eh, (XtPointer)0);
	XtAddEventHandler (w, ButtonPressMask, False, tip_eh, (XtPointer)0);
	XtAddEventHandler (w, ButtonReleaseMask, False, tip_eh, (XtPointer)0);
	XtAddCallback (w, XmNdestroyCallback, tip_destroy_cb, NULL);

	/* add info to the master list */
	tip_new (w, tip);
}

/* go through all the tips and make sure they are all down.
 */
void
wtip_alldown()
{
	Tip *tp, *ltp;

	for (tp = tips, ltp = &tips[ntips]; tp < ltp; tp++) {
	    if (tp->w && tp->state != T_DOWN) {
		tip_down(tp);
		tp->state = T_DOWN;
	    }
	}
}

/* one-time initialization stuff: create a window, font and gc.
 * TODO: reclaim old stuff when called again
 */
void
wtip_init ()
{
	Display *dsp = XtDisplay(toplevel_w);
	Window root = RootWindow (dsp, DefaultScreen(dsp));
	XSetWindowAttributes wa;
	unsigned long mask;
	XColor defxc, dbxc;
	XGCValues gcv;
	char *cnam;

	/* get tip colors */
	cnam = getXRes ("tipForeground", "black");
	if (!XAllocNamedColor (dsp, xe_cm, cnam, &defxc, &dbxc))
	    the_fg = BlackPixel (dsp, DefaultScreen(dsp));
	else
	    the_fg = defxc.pixel;
	cnam = getXRes ("tipBackground", "white");
	if (!XAllocNamedColor (dsp, xe_cm, cnam, &defxc, &dbxc))
	    the_bg = WhitePixel (dsp, DefaultScreen(dsp));
	else
	    the_bg = defxc.pixel;

	/* create the tips window -- use same one for all.
	 * any size/color will do for now since we change each time anyway.
	 */
	the_w = XCreateSimpleWindow (dsp, root, 0, 0, 10, 10, BW,
							the_fg, the_bg);

	/* prevent window manager from decorating */
	mask = CWOverrideRedirect | CWSaveUnder;
	wa.override_redirect = True;
	wa.save_under = True;
	XChangeWindowAttributes (dsp, the_w, mask, &wa);

	/* pick a font and make a GC */
	the_fs = XLoadQueryFont (dsp, getXRes ("tipFont", "xxx"));
	if (!the_fs)
	    the_fs = XLoadQueryFont (dsp, "fixed");
	if (!the_fs) {
	    printf ("Tips: no fixed font ?!?\n");
	    abort();
	}
	gcv.font = the_fs->fid;
	mask = GCFont;
	the_gc = XCreateGC (dsp, root, mask, &gcv);
}

/* add a new tip to tips[].
 * these tend to occur in bunches so just add to the end and mark the
 * array as unsorted. it will be sorted before being used.
 */
static void
tip_new (w, s)
Widget w;
String s;
{
	Tip *tp, *ltp;

	/* find an empty spot, or grow if none */
	for (tp = tips, ltp = &tips[ntips]; tp < ltp; tp++)
	    if (tp->w == 0)
		break;
	if (tp == ltp) {
	    /* nope -- make a new spot */
	    tips = (Tip *) XtRealloc ((void *)tips, (ntips+1)*sizeof(Tip));
	    tp = &tips[ntips++];
	}

	/* save widget */
	tp->w = w;

	/* save text */
#ifdef MALLOC_COPIES
	tp->s = XtNewString (s);
#else
	tp->s = s;
#endif

	/* init state */
	tp->state = T_DOWN;

	/* mark array as being unsorted now */
	tips_sorted = 0;
}

/* reclaim storage for tip info associated with w */
static void
tip_reclaim (w)
Widget w;
{
	Tip *tp;

	tp = tip_find (w);
	if (tp) {
	    tp->w = (Widget) 0;
#ifdef MALLOC_COPIES
	    XtFree (tp->s);
#endif
	    tips_sorted = 0;
	    return;
	}
	
	/* ha? */
	fprintf (stderr, "Tips: stranger reclaimed: %s\n", XtName(w));
}

/* event handler -- c is 1 if want up, 0 for down */
static void
tip_eh (w, c, e, cont)
Widget w;
XtPointer c;
XEvent *e;
Boolean *cont;
{
	Tip *tp;
	int wantup;

	/* don't do anything if the tips preference is currently turned off */
	if (pref_get (PREF_TIPS) == PREF_NOTIPS)
	    return;

	tp = tip_find (w);
	if (!tp)
	    return;

	wantup = ((long int)c == 1);
	if (wantup) {
	    if (tp->state == T_DOWN) {
		tip_startup (tp);
		tp->state = T_ARMED;
	    }
	} else {
	    if (tp->state == T_UP)
		tip_down (tp);
	    tp->state = T_DOWN;
	}
}

/* start a timer which, if it springs, will pop up the given tip */
static void
tip_startup (tp)
Tip *tp;
{
	XtAppContext ac = XtWidgetToApplicationContext (tp->w);

	/* N.B. tp can move if realloced, index can move if sorted. */
	(void) XtAppAddTimeOut (ac, TIPTO, timer_cb, (XtPointer)(tp->w));
}

/* timer expired so pop up tip for widget in client unless went DOWN again */
/* ARGSUSED */
static void
timer_cb (client, id)
XtPointer client;
XtIntervalId *id;
{
	Widget w = (Widget)client;
	Tip *tp = tip_find (w);

	/* conceivable it was destroyed */
	if (!tp)
	    return;

	if (tp->state == T_ARMED) {
	    tip_up (tp);
	    tp->state = T_UP;
	}
}

/* pop up the given tip */
static void
tip_up (tp)
Tip *tp;
{
	Display *dsp = XtDisplay (tp->w);
	Window win = XtWindow (tp->w);
	int scr = DefaultScreen(dsp);
	int dw = DisplayWidth(dsp,scr);
	int dh = DisplayHeight(dsp,scr);
	int l = strlen (tp->s);
	int dir, asc, des;
	int wid, hei;
	Window root, child;
	XCharStruct oa;
	int x, y, wx, wy;
	unsigned int pqmask;
	XSetWindowAttributes wa;
	unsigned long mask;

	/* compute size of window to wrap nicely around text */
	XTextExtents (the_fs, tp->s, l, &dir, &asc, &des, &oa);
	wid = oa.width + 2*GAP;
	hei = oa.ascent + oa.descent + 2*GAP;

	/* set position near pointer but never over it and beware screen edge */
	if (dw > MAXDW)
	    dw = MAXDW;
	XQueryPointer (dsp, win, &root, &child, &x, &y, &wx, &wy, &pqmask);
	if (x + wid + 2*BW > dw-EDGE)
	    x = dw-EDGE - wid - 2*BW;
	y += VOFFSET;
	if (y + hei + 2*BW > dh-EDGE)
	    y -= 2*VOFFSET + hei + 2*BW;	/* never on pointer */
	XMoveResizeWindow (dsp, the_w, x, y, wid, hei);

	/* set colors to match source */
	mask = CWBackPixel|CWBorderPixel|CWColormap;
	wa.background_pixel = the_bg;
	wa.border_pixel = the_fg;
	wa.colormap = xe_cm;
	XChangeWindowAttributes (dsp, the_w, mask, &wa);
	XClearWindow (dsp, the_w);
	XMapRaised (dsp, the_w);
	XSync (dsp, 0); /* so we don't need to wait for Expose elsewhere */

	/* draw the text */
	XSetForeground (dsp, the_gc, the_fg);
	XDrawString (dsp, the_w, the_gc, GAP, oa.ascent+GAP, tp->s, l);
}

/* pop down the tip window */
static void
tip_down(tp)
Tip *tp;
{
	XUnmapWindow (XtDisplay(tp->w), the_w);
}

/* w is being destroyed so reclaim its tip info */
/* ARGSUSED */
static void
tip_destroy_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	tip_reclaim (w);
}

/* find the Tip for the given widget, else NULL.
 * use a binary search. sort the array first if necessary.
 */
static Tip *
tip_find (w)
Widget w;
{
	long wl = (long)w;
	long s;
	int t, b, m;

	if (!tips_sorted)
	    tip_sort();

	/* binary search */
	t = ntips - 1;
	b = 0;
	while (b <= t) {
	    m = (t+b)/2;
	    s = wl - (long)(tips[m].w);
	    if (s == 0)
		return (&tips[m]);
	    if (s < 0)
		t = m-1;
	    else
		b = m+1;
	}

	return (NULL);
}

/* qsort-style function to sort the tips array by widget */
static int
tip_sf (const void *a1, const void *a2)
{
	Tip *t1 = (Tip *)a1;
	Tip *t2 = (Tip *)a2;
	long d = (long)(t1->w) - (long)(t2->w);

	if (d < 0)
	    return (-1);
	if (d > 0)
	    return (1);
	return (0);
}

/* sort the tips array by widget id, then mark it sorted */
static void
tip_sort ()
{
	if (ntips > 0)
	    qsort ((void *)tips, ntips, sizeof(Tip), tip_sf);
	tips_sorted = 1;
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: tips.c,v $ $Date: 2009/01/05 20:55:54 $ $Revision: 1.14 $ $Name:  $"};

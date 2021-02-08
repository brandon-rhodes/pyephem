/* code to manage the actual drawing of plots.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/Text.h>


#include "xephem.h"

#define	TL	5	/* tick mark length, in pixels */
#define	RB	30	/* right border */
#define	NT	10	/* rough number of tick marks on each axis */

static void mk_gcs (Widget w);
static double binsplit (int i);
static int x_ticks (int asdate, double minx, double maxx, int maxticks,
    double ticks[]);
static int draw_x_label (Display *dsp, Window win, int asdate, double v,
    int x, int y, int w);

static GC plt_gc;		/* the GC to use for everything */
static Pixel plt_p[MAXPLTLINES];/* colors, one per category */
static XFontStruct *plt_fs;	/* handy font metrix for placing text */
static Pixel da_p;		/* the foreground color of the drawing area */
static Pixel dabg_p;		/* the background color of the drawing area */
static int top_border;		/* space for coords values */

/* plot based on the given info in the drawing area in cartesian coords.
 * update di with graph boundaries.
 * TODO: add z tags somehow
 * return 0 if ok, else give a message about trouble and return -1.
 */
int
plot_cartesian (DrawInfo *di, Widget widget, Dimension nx, Dimension ny)
{
	Display *dsp = XtDisplay(widget);
	Window win = XtWindow(widget);
	double xticks[NT+2];
	double yticks[NT+2];
	int nxt, nyt;
	static char fmt[] = "%[A-Za-z0-9 ],%lf,%lf";
	double x, y;	/* N.B. be sure these match what scanf's %lf wants*/
	double minx=0, maxx=0, miny=0, maxy=0, xscale, yscale;
	char buf[128];
	int lx[MAXPLTLINES], ly[MAXPLTLINES], one[MAXPLTLINES];
	char tag[MAXTAG+1], tags[MAXPLTLINES][MAXTAG+1];
	XCharStruct overall;
	int sawtitle;
	int ylblw;	/* label width */
	int nlines;
	int ix, iy;	/* misc X drawing coords */
	int x0;		/* X x coord of lower left of plotting box, in pixels */
	int y0;		/* X y coord of lower left of plotting box, in pixels */
	int w, h;	/* width and height of plotting box, in pixels */
	int asc, desc;	/* font ascent and descent, in pixels */
	int maxlblw;	/* width of longest y axis label, in pixels */
	int i;
#define	XCORD(x)	(x0 + (int)((di->flipx?maxx-(x):(x)-minx)*xscale + 0.5))
#define	YCORD(y)	(y0 - (int)((di->flipy?maxy-(y):(y)-miny)*yscale + 0.5))

	/* find ranges and number of and tags for each unique line */
	nlines = 0;
	while (fgets (buf, sizeof(buf), di->fp)) {
	    if (sscanf (buf, fmt, tag, &x, &y) != 3)
		continue;
	    if (di->xjd_asdate)
		mjd_year (x-MJD0, &x);	/* work in years */
	    if (nlines == 0) {
		maxx = minx = x;
		maxy = miny = y;
	    }
	    for (i = 0; i < nlines; i++)
		if (strcmp(tag, tags[i]) == 0)
		    break;
	    if (i == nlines) {
		if (nlines == MAXPLTLINES) {
		   xe_msg (1, "Plot file contains more than %d functions.",
								   MAXPLTLINES);
		   return(-1);
		}
		strcpy (tags[nlines++], tag);
	    }
	    if (x > maxx) maxx = x;
	    else if (x < minx) minx = x;
	    if (y > maxy) maxy = y;
	    else if (y < miny) miny = y;
	}

	if (nlines == 0) {
	    xe_msg (1, "Plot file appears to be empty.");
	    return(-1);
	}
#define	SMALL	(1e-6)
	if (fabs(minx-maxx) < SMALL || fabs(miny-maxy) < SMALL) {
	    xe_msg (1, "Plot file values contain insufficient spread.");
	    return(-1);
	}

	/* build the gcs, fonts etc if this is the first time. */
	if (!plt_gc)
	    mk_gcs (widget);
	XSetForeground (dsp, plt_gc, da_p);
	XSetFont (dsp, plt_gc, plt_fs->fid);

	/* decide tickmarks */
	nxt = x_ticks (di->xjd_asdate || di->xyr_asdate, minx, maxx, NT, xticks);
	di->data_minx = minx = xticks[0];
	di->data_maxx = maxx = xticks[nxt-1];
	nyt = tickmarks (miny, maxy, NT, yticks);
	di->data_miny = miny = yticks[0];
	di->data_maxy = maxy = yticks[nyt-1];

	/* compute length of longest y-axis label and other char stuff.
	 */
	maxlblw = 0;
	for (i = 0; i < nyt; i++) {
	    int dir, l;
	    (void) sprintf (buf, "%g", yticks[i]);
	    l = strlen(buf);
	    XTextExtents (plt_fs, buf, l, &dir, &asc, &desc, &overall);
	    if (overall.width > maxlblw)
		maxlblw = overall.width;
	    top_border = 2*(asc+desc) + 5;
	}

	/* compute border sizes and the scaling factors */
	di->win_minx = x0 = maxlblw+TL+10;
	w = nx - x0 - RB;
	di->win_maxx = di->win_minx + w;
	y0 = ny - (asc+desc+2+2*TL);
	h = y0 - top_border;
	di->win_miny = y0 - h;
	di->win_maxy = di->win_miny + h;
	xscale = w/(maxx-minx);
	yscale = h/(maxy-miny);

	/* draw y axis, its labels, and optionally the horizontal grid */
	for (i = 0; i < nyt; i++) {
	    int l;
	    (void) sprintf (buf, "%g", yticks[i]);
	    l = strlen(buf);
	    iy = YCORD(yticks[i]);
	    XPSDrawLine (dsp, win, plt_gc, x0-TL, iy, x0, iy);
	    XPSDrawString (dsp, win, plt_gc, 1, iy+(asc-desc)/2, buf, l);
	    if (di->grid)
		XPSDrawLine (dsp, win, plt_gc, x0, iy, x0+w-1, iy);
	}

	/* draw x axis and label it's first and last tick mark.
	 * if there's room, label the center tickmark too.
	 * also grid, if requested.
	 */
	ylblw = 0;
	for (i = 0; i < nxt; i++) {
	    ix = XCORD(xticks[i]);
	    if (di->grid)
		XPSDrawLine (dsp, win, plt_gc, ix, y0, ix, y0-h);
	}
	ylblw += draw_x_label (dsp, win, di->xjd_asdate || di->xyr_asdate, minx, XCORD(minx),y0,w+x0+RB);
	ylblw += draw_x_label (dsp, win, di->xjd_asdate || di->xyr_asdate, maxx, XCORD(maxx),y0,w+x0+RB);
	if (ylblw < w/2)
	    (void) draw_x_label (dsp, win, di->xjd_asdate || di->xyr_asdate, xticks[nxt/2],
					    XCORD(xticks[nxt/2]), y0, w+x0+RB);

	/* draw border of actual plotting area */
	XPSDrawLine (dsp, win, plt_gc, x0, y0-h, x0, y0);
	XPSDrawLine (dsp, win, plt_gc, x0, y0, x0+w, y0);
	XPSDrawLine (dsp, win, plt_gc, x0+w, y0, x0+w, y0-h);
	XPSDrawLine (dsp, win, plt_gc, x0+w, y0-h, x0, y0-h);

	/* read file again, this time plotting the data (finally!).
	 * also, the first line we see that doesn't look like a point
	 * is put up as a title line (minus its first two char and trailing \n).
	 */
	sawtitle = 0;
	rewind (di->fp);
	for (i = 0; i < nlines; i++)
	    one[i] = 0;
	while (fgets (buf, sizeof(buf), di->fp)) {
	    if (sscanf (buf, fmt, tag, &x, &y) != 3) {
		/* a title line ? */
		int l;

		if (!sawtitle && (l = strlen(buf)) > 2) {
		    int di, as, de;
		    XCharStruct ovl;
		    XTextExtents (plt_fs, buf+2, l-2, &di, &as, &de, &ovl);
		    XSetForeground (dsp, plt_gc, da_p);
		    XPSDrawString (dsp, win, plt_gc, x0+(w-ovl.width)/2, asc+1,
								buf+2, l-3);
		    sawtitle = 1;
		}
		continue;
	    }
	    if (di->xjd_asdate)
		mjd_year (x-MJD0, &x);	/* work in years */
	    for (i = 0; i < nlines; i++)
		if (strcmp(tag, tags[i]) == 0)
		    break;
	    ix = XCORD(x);
	    iy = YCORD(y);
	    XSetForeground (dsp, plt_gc, plt_p[i]);
	    if (one[i]++ > 0)
		XPSDrawLine (dsp, win, plt_gc, ix, iy, lx[i], ly[i]);
	    else {
		int ytop = y0 - h + asc;
		XPSDrawString (dsp, win, plt_gc, ix+2, (iy<ytop ? ytop : iy)-2,
							    tag, strlen(tag));
	    }
	    lx[i] = ix;
	    ly[i] = iy;
	}
	return (0);
}

/* given a cursor location in the plot described by di, draw the 
 * corresponding data values
 */
void
plot_coords (Widget da_w, DrawInfo *di, int window_x, int window_y)
{
        Display *dsp = XtDisplay(da_w);
	Window win = XtWindow(da_w);
	int graphx, graphy;
	int wid, hei;
	double datax, datay;
	char buf[1024];
	int bufl;

	/* interpolate data */
	if (window_x < di->win_minx || window_x > di->win_maxx || window_y < di->win_miny
		|| window_y > di->win_maxy)
	    return;
	wid = di->win_maxx - di->win_minx;
	hei = di->win_maxy - di->win_miny;
	graphx = window_x - di->win_minx;
	if (di->flipx)
	    graphx = wid - graphx;
	graphy = window_y - di->win_miny;
	if (di->flipy)
	    graphy = hei - graphy;
	datax = di->data_minx + graphx*(di->data_maxx - di->data_minx)/wid;
	datay = di->data_maxy - graphy*(di->data_maxy - di->data_miny)/hei; /* window 0 is on top */

	/* erase */
	XSetForeground (dsp, plt_gc, dabg_p);
	XFillRectangle (dsp, win, plt_gc, 30, top_border/2, 200, top_border/2);

	/* draw */
	XSetForeground (dsp, plt_gc, da_p);
	bufl = sprintf (buf, "%.6g", datax);
	XDrawString (dsp, win, plt_gc, 30, top_border-2, buf, bufl);
	bufl = sprintf (buf, "%.6g", datay);
	XDrawString (dsp, win, plt_gc, 120, top_border-2, buf, bufl);
}

static void
mk_gcs (w)
Widget w;
{
	Display *dsp = XtDisplay(w);
	Window win = XtWindow(w);
	int i;

	/* create the annotation and default plot color gc,
	 * using the foreground color of the PlotDA.
	 */
	get_something (w, XmNforeground, (XtArgVal)&da_p);
	get_something (w, XmNbackground, (XtArgVal)&dabg_p);
	plt_gc = XCreateGC (dsp, win, 0L, NULL);
	XSetForeground (dsp, plt_gc, da_p);
	get_views_font (dsp, &plt_fs);

	/* fill in plt_p array with pixels to use for function plotting.
	 */
	for (i = 0; i < MAXPLTLINES; i++) {
	    double h, s, v, r, g, b;
	    XColor xc;

	    /* bounce around color wheel */
	    h = binsplit (i);
	    s = i < MAXPLTLINES/2 ? 1.0 : 0.5;
	    v = 1.0;

	    toRGB (h, s, v, &r, &g, &b);
	    xc.red = (int)(65535 * r);
	    xc.green = (int)(65535 * g);
	    xc.blue = (int)(65535 * b);

	    if (!XAllocColor (dsp, xe_cm, &xc))
		xc.pixel = WhitePixel(dsp,DefaultScreen(dsp));
	    plt_p[i] = xc.pixel;
	}
}

/* return a unique number between 0 and 1 for every non-neg value of i.
 * arrange for them to move along ever finer binary steps.
 */
static double
binsplit (int i)
{
	/* 1-based */
	double oneb = i + 1;

	/* divide into base power of two parts */
	double ndiv = pow (2.0, floor(log(oneb)/log(2.0)));

	/* add remainder, divide last evenly on both ends */
	return ((0.5 + oneb - ndiv)/ndiv);
}

static int
x_ticks (asdate, minx, maxx, maxticks, ticks)
int asdate;
double minx, maxx;
int maxticks;
double ticks[];
{
	double jd, minjd, maxjd;
	double d0, d1;
	int m0, m1, y0, y1;
	int nt;
	int i;

	if (!asdate)
	    return (tickmarks (minx, maxx, maxticks, ticks));

	/* find spanning period in mjds */
	year_mjd (minx, &minjd);
	mjd_cal (minjd, &m0, &d0, &y0);
	year_mjd (maxx, &maxjd);
	mjd_cal (maxjd, &m1, &d1, &y1);

	/* use month ticks if spans well more than a month */
	if (maxjd - minjd > 40) {
	    /* put ticks on month boundaries */
	    int dt, nm;
	    double jd0;

	    if (d1 > 1 && ++m1 > 12) {		/* next whole month */
		m1 = 1;
		y1++;
	    }
	    nm = (y1*12+m1) - (y0*12+m0);	/* period */
	    dt = nm/maxticks + 1;		/* step size */
	    nt = (nm+(dt-1))/dt + 1;		/* n ticks */
	    for (i = 0; i < nt; i++) {
		cal_mjd (m0, 1.0, y0, &jd0);
		mjd_year (jd0, &ticks[i]);
		m0 += dt;
		while (m0 > 12) {
		    m0 -= 12;
		    y0++;
		}
	    }
	} else {
	    /* put ticks on day boundaries */
	    int nd, dt;

	    nd = (int)(maxjd - minjd + 1);	/* period */
	    dt = nd/maxticks + 1;		/* round up */
	    nt = (nd+(dt-1))/dt + 1;		/* n ticks */
	    jd = mjd_day (minjd);		/* whole day */
	    for (i = 0; i < nt; i++) {
		mjd_year (jd, &ticks[i]);
		jd += dt;
	    }
	}

	return (nt);
}

/* draw v centered at [x,y].
 * return width of string in pixels.
 */
static int
draw_x_label (dsp, win, asdate, v, x, y, w)
Display *dsp;
Window win;
int asdate;
double v;
int x, y;
int w;
{
	int dir, asc, des;
	XCharStruct ovl;
	char buf[128];
	int l, lx;

	if (asdate) {
	    double jd;

	    year_mjd (v, &jd);
	    fs_date (buf, pref_get(PREF_DATE_FORMAT), jd);
	} else 
	    (void) sprintf (buf, "%g", v);
	l = strlen(buf);
	XTextExtents (plt_fs, buf, l, &dir, &asc, &des, &ovl);
	XSetForeground (dsp, plt_gc, da_p);
	XSetFont (dsp, plt_gc, plt_fs->fid);
	XPSDrawLine (dsp, win, plt_gc, x, y+TL, x, y+2*TL);
	lx = x-ovl.width/2;
	if (lx < 0)
	    lx = 1;
	else if (lx + ovl.width > w)
	    lx = w - ovl.width;
	XPSDrawString (dsp, win, plt_gc, lx, y+2*TL+asc+1, buf, l);

	return (ovl.width);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: plot_aux.c,v $ $Date: 2012/12/30 17:01:02 $ $Revision: 1.12 $ $Name:  $"};

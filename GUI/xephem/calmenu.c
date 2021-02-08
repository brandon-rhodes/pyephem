/* stuff to control the calendar in the main menu.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>

#include "xephem.h"

static void today_cb (Widget w, XtPointer client, XtPointer call);
static void date_changed_cb (Widget w, XtPointer client, XtPointer call);
static void mm_nfmoon (double jd, double tzone, int m, int f, int nd);

#define	CAL_ROWS	6		/* rows in the date matrix */
#define	CAL_COLS	7		/* columns in the date matrix */
#define	NYEARS		12		/* num entries in the year pulldown */
#define	NMONTHS		12		/* num entries in the month pulldown */
static Widget m_w, mmenu_w[NMONTHS];	/* month cascade btn and pulldwn menu */
static Widget y_w, ymenu_w[NYEARS];	/* year cascade btn and pulldwn menu */
static Widget d_w[CAL_ROWS*CAL_COLS];	/* pushbtns in the date matrix */
static Widget tz_w;			/* timezone + title label */

/* must all be fixed-width since XmMENU_BAR RowColumns don't allow for resizing.
 */
static char mnames[][10] = {
   "January  ", "February ", "March    ", "April    ", "May      ", "June     ",
   "July     ", "August   ", "September", "October  ", "November ", "December "
};

typedef struct {
    char name[3];
    Widget w;
} DOW;
static DOW dnames[CAL_COLS] = {
    {"Su"}, {"Mo"}, {"Tu"}, {"We"}, {"Th"}, {"Fr"}, {"Sa"}
};

enum {DAY, MONTH, YEAR};

typedef enum {
    BACKBACK, BACK, TONOW, FORW, FORWFORW
} TodayCuts;

static Pixel fg_pix, bg_pix;		/* used to reverse colors "today" */

#define	NO_DATE	(999)			/* anything outside -31..31 */

/* create a calendar from parent and return the outtermost widget.
 */
Widget
calm_create (parent)
Widget parent;
{
	Arg args[20];
	Widget form_w;
	Widget rc_w;
	Widget mb_w;
	Widget menu_w;
	Widget bw, fw;
	Widget w;
	int n;
	int i;

	/* create the outter form */

	n = 0;
	form_w = XmCreateForm (parent, "CalForm", args, n);

	/* put a timezone and title at the top */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	tz_w = XmCreateLabel (form_w, "Calendar", args, n);
	wtip (tz_w, "Controls to set the date");
	XtManageChild (tz_w);

	/* create a menu bar for the Month and Year menus */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, tz_w); n++;
	XtSetArg (args[n], XmNtopOffset, 3); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 3); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 3); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNmarginWidth, 1); n++;
	mb_w = XmCreateMenuBar (form_w, "MB", args, n);
	XtManageChild (mb_w);

	    /* create the Month and Year cascade button/pulldown menu */

	    n = 0;
	    menu_w = XmCreatePulldownMenu (mb_w, "MonthPD", args, n);
	    /* managed by the CascadeButton */

		for (i = 0; i < NMONTHS; i++) {
		    n = 0;
		    XtSetArg (args[n], XmNuserData, i+1); n++;
		    w = XmCreatePushButton (menu_w, mnames[i], args, n);
		    XtAddCallback (w, XmNactivateCallback, date_changed_cb,
							    (XtPointer)MONTH);
		    XtManageChild (w);
		    mmenu_w[i] = w;
		}

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, menu_w); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    m_w = XmCreateCascadeButton (mb_w, "MonthCB", args, n);
	    XtManageChild (m_w);
	    wtip (m_w, "Click to set the month by picking from a list");

	    n = 0;
	    menu_w = XmCreatePulldownMenu (mb_w, "YearPD", args, n);
	    /* managed by the CascadeButton */

		for (i = 0; i < NYEARS; i++) {
		    n = 0;
		    w = XmCreatePushButton (menu_w, "YearPB", args, n);
		    XtAddCallback (w, XmNactivateCallback, date_changed_cb,
							    (XtPointer)YEAR);
		    XtManageChild (w);
		    ymenu_w[i] = w;
		}

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, menu_w); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    y_w = XmCreateCascadeButton (mb_w, "YearCB", args, n);
	    XtManageChild (y_w);
	    wtip (y_w, "Click to set a recent year by picking from a list");

	    /* slide to the right */

	    set_something (mb_w, XmNmenuHelpWidget, (XtArgVal)y_w);

	/* create the rowcol for the main date matrix */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNnumColumns, CAL_ROWS+1); n++; /* +1 for dnames*/
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
	XtSetArg (args[n], XmNadjustMargin, False); n++;
	XtSetArg (args[n], XmNspacing, 0); n++;
	rc_w = XmCreateRowColumn (form_w, "CalRC", args, n);
	XtManageChild (rc_w);

	/* add the fixed day abbreviations */

	for (i = 0; i < XtNumber(dnames); i++) {
	    n = 0;
	    dnames[i].w = XmCreateLabel (rc_w, "XX", args, n);
	    XtManageChild (dnames[i].w);
	}

	/* add the calendar entries proper */

	for (i = 0; i < CAL_ROWS*CAL_COLS; i++) {
	    n = 0;
	    XtSetArg (args[n], XmNrecomputeSize, False); n++;	/* SLOW!! */
	    d_w[i] = XmCreatePushButton (rc_w, "CD", args, n);
	    XtAddCallback (d_w[i], XmNactivateCallback, date_changed_cb,
								(XtPointer)DAY);
	    XtManageChild (d_w[i]);
	}

	/* add the Now and surrounding buttons */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopOffset, 3); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	w = XmCreatePushButton (form_w, "<<", args, n);
	XtManageChild (w);
	XtAddCallback (w, XmNactivateCallback, today_cb, (XtPointer)BACKBACK);
	wtip (w, "Move back one week");

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopOffset, 3); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, w); n++;
	bw = XmCreatePushButton (form_w, "<", args, n);
	XtManageChild (bw);
	XtAddCallback (bw, XmNactivateCallback, today_cb, (XtPointer)BACK);
	wtip (bw, "Move back one day");

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopOffset, 3); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	w = XmCreatePushButton (form_w, ">>", args, n);
	XtManageChild (w);
	XtAddCallback (w, XmNactivateCallback, today_cb, (XtPointer)FORWFORW);
	wtip (w, "Move forward one week");

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopOffset, 3); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, w); n++;
	fw = XmCreatePushButton (form_w, ">", args, n);
	XtManageChild (fw);
	XtAddCallback (fw, XmNactivateCallback, today_cb, (XtPointer)FORW);
	wtip (fw, "Move forward one day");

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopOffset, 3); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, bw); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, fw); n++;
	w = XmCreatePushButton (form_w, "Now", args, n);
	XtManageChild (w);
	XtAddCallback (w, XmNactivateCallback, today_cb, (XtPointer)TONOW);
	wtip (w, "Set time and date from computer clock");

	get_something (d_w[0], XmNforeground, (XtArgVal)&fg_pix);
	get_something (d_w[0], XmNbackground, (XtArgVal)&bg_pix);

	return (form_w);
}

/* called when new resources have been set so we can update fg/bg_pix */
void
calm_newres()
{
	Widget middle = d_w[CAL_COLS*CAL_ROWS/2];

	get_something (middle, XmNforeground, (XtArgVal)&fg_pix);
	get_something (middle, XmNbackground, (XtArgVal)&bg_pix);
}

/* set the calendar to the time of the given Now *np.
 * use local time if PREF_ZONE is PREF_LOCALTZ
 * only really do it if f_ison() changed or f_ison() now and day changed.
 */
void
calm_set (np)
Now *np;
{
	static double last_jd = -1e5;
	static double last_tz = 1e10;
	static PrefWeekStart last_wkstart = PREF_SUN;
	static int last_zp = -1;
	static int wason = -1;
	int f;		/* day of week of first day of month Sun=0 */
	int nd;		/* number of days in this month */
	double jd;	/* current mjd; corrected for TZ if PREF_LOCALTZ */
	double jd0;	/* mjd of first day of the month */
	Pixel om_pix;
	int localtz;
	char buf[64];
	int zonepref;
	int m, y;
	double d;
	int ison, new;
	int i;

	/* preliminaries for time zone */
	zonepref = pref_get(PREF_ZONE);
	localtz = (zonepref == PREF_LOCALTZ);
	jd = localtz ? mjd - tz/24.0 : mjd;

	/* set name of current timezone preference in title */
	fs_tz (buf, zonepref, np);
	(void) strcat (buf, " Calendar");
	f_showit (tz_w, buf);

	/* proceed if f_ison() changed or on and things have changed */
	ison = f_ison();
	new = ison != wason || tz != last_tz || last_zp != zonepref
			    || (ison && mjd_day(last_jd) != mjd_day(jd))
			    || pref_get (PREF_WEEKSTART) != last_wkstart;
	wason = ison;
	last_tz = tz;
	last_zp = zonepref;
	last_wkstart = pref_get (PREF_WEEKSTART);
	if (!new)
	    return;

	/* remember the new jd we are working on */
	last_jd = jd;

	/* get today's month, day, year */
	mjd_cal (jd, &m, &d, &y);

	/* label the month */
	f_showit (m_w, mnames[m-1]);

	/* label the year and set the menu entries around it */
	(void) sprintf (buf, "%d", y);
	f_showit (y_w, buf);
	for (i = 0; i < NYEARS; i++) {
	    int ty = (y - NYEARS/3) + i; /* current year about 1/3 down list */
	    (void) sprintf (buf, "%d", ty);
	    f_showit (ymenu_w[i], buf);
	    set_something (ymenu_w[i], XmNuserData, (XtArgVal)ty);
	}

	/* find day of week of first day of month */
	cal_mjd (m, 1.0, y, &jd0);
	if (mjd_dow (jd0, &f) < 0) {
	    /* can't figure it out - too hard before Gregorian */
	    for (i = 0; i < CAL_ROWS*CAL_COLS; i++) {
		set_something (d_w[i], XmNuserData, (XtArgVal)NO_DATE);
		f_showit (d_w[i], "  ");
	    }
	    return;
	}

	/* slip f one if start weeks on sat or mon, and
	 * print names of days slipped too
	 */
	switch (pref_get(PREF_WEEKSTART)) {
	case PREF_SAT:
	    f = (f+1)%7;
	    for (i = 0; i < XtNumber(dnames); i++)
		set_xmstring(dnames[i].w, XmNlabelString, dnames[(i+6)%7].name);
	    break;
	case PREF_SUN:
	    /* f is ok */
	    for (i = 0; i < XtNumber(dnames); i++)
		set_xmstring(dnames[i].w, XmNlabelString, dnames[i].name);
	    break;
	case PREF_MON:
	    f = (f+6)%7;	/* really (f-1)%7 */
	    for (i = 0; i < XtNumber(dnames); i++)
		set_xmstring(dnames[i].w, XmNlabelString, dnames[(i+1)%7].name);
	    break;
	}

	/* find number of days in this month */
	mjd_dpm (jd0, &nd);

	/* print the calendar.
	 * set userData to the day of the month.
	 */
	(void) get_color_resource (d_w[0], "CalOtherMonthColor", &om_pix);
	for (i = 0; i < CAL_ROWS*CAL_COLS; i++) {
	    int date = i-f+1;
	    if (i < f || i > f + nd - 1) {
		/* prev or next month */
		double tmpjd, tmpd;
		int tmpm, tmpy;
		cal_mjd (m, (double)(date), y, &tmpjd);
		mjd_cal (tmpjd, &tmpm, &tmpd, &tmpy);
		(void) sprintf (buf, "%2d", (int)floor(tmpd+0.5));
		set_something (d_w[i], XmNforeground, (XtArgVal)om_pix);
		set_something (d_w[i], XmNbackground, (XtArgVal)bg_pix);
	    } else {
		(void) sprintf (buf, "%2d", date);
		if (date == (int)d) {
		    /* reverse colors on today */
		    set_something (d_w[i], XmNforeground, (XtArgVal)bg_pix);
		    set_something (d_w[i], XmNbackground, (XtArgVal)fg_pix);
		} else {
		    set_something (d_w[i], XmNforeground, (XtArgVal)fg_pix);
		    set_something (d_w[i], XmNbackground, (XtArgVal)bg_pix);
		}
	    }

	    f_showit (d_w[i], buf);
	    set_something (d_w[i], XmNuserData, (XtArgVal)(date));
	}

	/* over print the new and full moons for days near this month.
	 * TODO: don't really know which dates to use here (see moonnf())
	 *   so try several to be fairly safe. have to go back to 4/29/1988
	 *   to find the full moon on 5/1 for example.
	 */
	mm_nfmoon (jd0-15, tz, m, f, nd);
	mm_nfmoon (jd0-3, tz, m, f, nd);
	mm_nfmoon (jd0+15, tz, m, f, nd);
	mm_nfmoon (jd0+30, tz, m, f, nd);
}

/* called when Now or its helpers are pressed.
 * client is one of TodayCuts.
 * set m/d/y but retain any partion day.
 */
/* ARGSUSED */
static void
today_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	TodayCuts c = (TodayCuts)client;
	Now *np = mm_get_now();
	Now now;
	double newmjd;

	switch (c) {
	case BACKBACK:
	    newmjd = mjd - 7;
	    break;

	case BACK:
	    newmjd = mjd - 1;
	    break;

	case TONOW:
	    time_fromsys (&now);
	    newmjd = now.n_mjd;
	    break;

	case FORW:
	    newmjd = mjd + 1;
	    break;

	case FORWFORW:
	    newmjd = mjd + 7;
	    break;

	default:
	    newmjd = mjd;
	    break;
	}

	/* this always wants UTC */
	mm_newcaldate (newmjd);
}

/* called when any of the calendar pushbuttons is activated.
 * client is one of DAY, MONTH or YEAR.
 * userData is new value, ie, a month, day or year.
 * N.B. we honor PREF_ZONE
 */
/* ARGSUSED */
static void
date_changed_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Now *np = mm_get_now();
	int code = (long int)client;
	double jd;
	double newmjd;
	int localtz;
	int x;
	int m, y;
	double d;

	get_something (w, XmNuserData, (XtArgVal)&x);
	if (x == NO_DATE)
	    return;

	localtz = pref_get (PREF_ZONE) == PREF_LOCALTZ;
	jd = localtz ? mjd - tz/24.0 : mjd;

	mjd_cal (jd, &m, &d, &y);

	switch (code) {
	case MONTH:
	    cal_mjd (x, d, y, &newmjd);
	    break;
	case DAY:
	    d = x + mjd_hr(jd)/24.0;	/* preserve any partial day */
	    cal_mjd (m, d, y, &newmjd);
	    break;
	case YEAR:
	    cal_mjd (m, d, x, &newmjd);
	    break;
	}

	if (localtz)
	    newmjd += tz/24.0;

	/* this function always wants UTC */
	mm_newcaldate (newmjd);
}

/* print the new and full moons for the months surrounding jd, where
 * m is the month-of-year for the current month and f is index into d_w[]
 * of the first day of this month and nd is number of days of month `m'.
 */
static void
mm_nfmoon (jd, tzone, m, f, nd)
double jd;
double tzone;
int m, f, nd;
{
	static char nms[] = "NM", fms[] = "FM";
	double jdn, jdf;	/* mjd of new and full moons near jd */
	int mm, ym;
	double dm;
	int ndays;
	int di;

	moonnf (jd, &jdn, &jdf);
	if (pref_get(PREF_ZONE) == PREF_LOCALTZ) {
	    jdn -= tzone/24.0;
	    jdf -= tzone/24.0;
	}

	mjd_cal (jdn, &mm, &dm, &ym);
	if ((mm == m - 1) || (mm == 12 && m == 1)) {
	    mjd_dpm (jdn, &ndays);
	    dm -= ndays;
	} else if ((mm ==  m + 1) || (mm == 1 && m == 12)) {
	    dm += nd;
	} else if (mm != m)
	    dm = -f;
	di = (int)floor(dm + f - 1);
	if (di >= 0 && di < CAL_ROWS*CAL_COLS)
	    f_showit (d_w[di], nms);

	mjd_cal (jdf, &mm, &dm, &ym);
	if ((mm == m - 1) || (mm == 12 && m == 1)) {
	    mjd_dpm (jdf, &ndays);
	    dm -= ndays;
	} else if ((mm ==  m + 1) || (mm == 1 && m == 12)) {
	    dm += nd;
	} else if (mm != m)
	    dm = -f;
	di = (int)floor(dm + f - 1);
	if (di >= 0 && di < CAL_ROWS*CAL_COLS)
	    f_showit (d_w[di], fms);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: calmenu.c,v $ $Date: 2009/01/05 20:55:54 $ $Revision: 1.21 $ $Name:  $"};

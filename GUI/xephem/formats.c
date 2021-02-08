/* all the screen oriented printing should go through here.
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include "xephem.h"


/* suppress screen io if this is true, but always log stuff.
 */
static int f_scrnoff;
void
f_on ()
{
	f_scrnoff = 0;
}

void
f_off ()
{
	f_scrnoff = 1;
}

int
f_ison()
{
	return (!f_scrnoff);
}

/* set the given widget's XmNlabelString to s if it's not already the same
 * and we are indeed displaying stuff at the moment.
 * if we don't update the value then mark it as insensitive for feedback.
 */
void
f_showit (w, s)
Widget w;
char *s;
{
	/* testing is faster than setting */
	if (XtIsSensitive(w) != !f_scrnoff)
	    XtSetSensitive (w, !f_scrnoff);

	if (!f_scrnoff) {
	    char *txtp;

	    get_xmstring (w, XmNlabelString, &txtp);
	    if (strcmp (txtp, s))
		set_xmstring (w, XmNlabelString, s);
	    XtFree (txtp);
	}
}

/* print the variable a in sexagesimal format to widget wid.
 * see fs_sexa for full formatting details.
 */
void
f_sexa (wid, a, w, fracbase)
Widget wid;
double a;
int w;
int fracbase;
{
	char out[64];

	fs_sexa (out, a, w, fracbase);
	field_log (wid, a, 1, out);
	f_showit (wid, out);
}

/* print ra, in radians, to widget w in hours according to the precision pref.
 */
void
f_ra (w, ra)
Widget w;
double ra;
{
	if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
	    f_sexa (w, radhr(ra), 2, 600);
	else
	    f_sexa (w, radhr(ra), 2, 360000);
}

/* print ra, in radians, into string out[] in hours according to precision pref.
 */
void
fs_ra (out, ra)
char out[];
double ra;
{
	if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
	    fs_sexa (out, radhr(ra), 2, 600);
	else
	    fs_sexa (out, radhr(ra), 2, 360000);
}

/* print dec, a, in rads, as degrees to widget w according to desired
 * precision preference.
 */
void
f_prdec(w, a)
Widget w;
double a;
{
	if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
	    f_sexa (w, raddeg(a), 3, 60);
	else
	    f_sexa (w, raddeg(a), 3, 36000);
}

/* print dec, a, in rads, as degrees into string out[] according to desired
 * precision preference.
 */
void
fs_prdec(out, a)
char out[];
double a;
{
	if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
	    fs_sexa (out, raddeg(a), 3, 60);
	else
	    fs_sexa (out, raddeg(a), 3, 36000);
}

/* print time, t, as hh:mm:ss */
void
f_time (w, t)
Widget w;
double t;
{
	range (&t, 24.0);
#ifdef AVOID_24H
	if (t >= 24.0 - 1./3600./2.)
	    t = 0;
#endif /* AVOID_24H */
	f_sexa (w, t, 2, 3600);
}

/* print time, t, as hh:mm:ss */
void
fs_time (out, t)
char out[];
double t;
{
	range (&t, 24.0);
#ifdef AVOID_24H
	if (t >= 24.0 - 1./3600./2.)
	    t = 0;
#endif /* AVOID_24H */
	fs_sexa (out, t, 2, 3600);
}

/* print time, t, as hh:mm to widget w */
void
f_mtime (w, t)
Widget w;
double t;
{
	range (&t, 24.0);
#ifdef AVOID_24H
	if (t >= 24.0 - 1./60./2.)
	    t = 0;
#endif /* AVOID_24H */
	f_sexa (w, t, 2, 60);
}

/* print time, t, as hh:mm into out[] */
void
fs_mtime (out, t)
char out[];
double t;
{
	range (&t, 24.0);
#ifdef AVOID_24H
	if (t >= 24.0 - 1./60./2.)
	    t = 0;
#endif /* AVOID_24H */
	fs_sexa (out, t, 2, 60);
}

/* print angle, a, in rads, as degrees to widget w in form ddd:mm */
void
f_dm_angle(w, a)
Widget w;
double a;
{
	f_sexa (w, raddeg(a), 3, 60);
}

/* print angle, a, in rads, as degrees into string out[] in form ddd:mm */
void
fs_dm_angle(out, a)
char out[];
double a;
{
	fs_sexa (out, raddeg(a), 3, 60);
}


/* print angle, a, in rads, as degrees to widget w according to desired
 * precision preference.
 */
void
f_pangle(w, a)
Widget w;
double a;
{
	if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
	    f_sexa (w, raddeg(a), 3, 60);
	else
	    f_sexa (w, raddeg(a), 3, 3600);
}

/* print angle, a, in rads, as degrees into string out[] according to desired
 * precision preference.
 */
void
fs_pangle(out, a)
char out[];
double a;
{
	if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
	    fs_sexa (out, raddeg(a), 3, 60);
	else
	    fs_sexa (out, raddeg(a), 3, 3600);
}

/* print angle, a, in rads, as degrees to widget w in form dddd:mm:ss */
void
f_dms_angle(w, a)
Widget w;
double a;
{
	f_sexa (w, raddeg(a), 4, 3600);
}

/* print angle, a, in rads, as degrees into string out[] in form dddd:mm:ss */
void
fs_dms_angle(out, a)
char out[];
double a;
{
	fs_sexa (out, raddeg(a), 4, 3600);
}

/* print the given modified Julian date, jd, in the preferred format.
 */
void
f_date (w, jd)
Widget w;
double jd;
{
	char out[32];
	double tmp;

	fs_date (out, pref_get(PREF_DATE_FORMAT), jd);

	/* shadow to the plot subsystem as years. */
	mjd_year (jd, &tmp);
	field_log (w, tmp, 1, out);
	f_showit (w, out);
}

/* set w's XmNlabelString to s if it's not already the same and we are
 * showing fields now.
 * also, log the string if w is being used for logging now.
 * N.B. do not use this for any widget that has its XmNuserData anything but
 *   the default (NULL) or a valid field id string; ie. use only for widgets
 *   that are buttons that can be "logged" for plotting etc. In all other cases
 *   use f_showit() directly.
 */
void
f_string (w, s)
Widget w;
char *s;
{
	field_log (w, 0.0, 0, s);
	f_showit (w, s);
}

void
f_double (w, fmt, f)
Widget w;
char *fmt;
double f;
{
	char str[80];
	(void) sprintf (str, fmt, f);
	field_log (w, f, 1, str);
	f_showit (w, str);
}

/* fill buf() with given timezone name */
void
fs_tz (buf, tzpref, np)
char buf[];
int tzpref;	/* PREF_UTCTZ or PREF_LOCALTZ */
Now *np;
{
	if (tzpref == PREF_UTCTZ)
	    (void) strcpy(buf, "UTC");
	else if (tznm[0] == '\0') {
	    if (tz == 0)
		(void) strcpy(buf, "UTC");
	    else
		(void) sprintf(buf, "UTC%c%g", tz<0?'+':'-', fabs(tz));
	} else
	    (void) strcpy (buf, tznm);
}

/* fill buf[] with time stamp from np */
void
fs_timestamp (np, stamp)
Now *np;
char stamp[];
{
	double lmjd;
	char d[32], t[32];
	char timezonename[32];
	int tzpref = pref_get (PREF_ZONE);

	lmjd = mjd;
	if (tzpref == PREF_LOCALTZ)
	    lmjd -= tz/24.0;

	fs_date (d, pref_get(PREF_DATE_FORMAT), mjd_day(lmjd));
	fs_time (t, mjd_hr(lmjd));

	fs_tz (timezonename, tzpref, np);
	(void) sprintf (stamp, "%s %s %s", d, t, timezonename);
}

/* set the XmNlabelString resource of the given widget to the date and time 
 * as given in the Now struct at *np.
 * avoid redrawing the string if it has not changed but don't use f_showit()
 *   because we want the time to always be updated even during movie loops.
 */
void
timestamp (np, w)
Now *np;
Widget w;
{
	char stamp[64];
	char *txtp;

	fs_timestamp (np, stamp);
	get_xmstring (w, XmNlabelString, &txtp);
	if (strcmp (txtp, stamp)) {
	    set_xmstring (w, XmNlabelString, stamp);
	    /* just XSync here doesn't get the time updated regularly enough
	     * though this Update causes Sky View to get extra exposes before
	     * it makes it's first pixmap.
	     */
	    XmUpdateDisplay (w);
	}
	XtFree (txtp);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: formats.c,v $ $Date: 2006/04/10 09:00:06 $ $Revision: 1.4 $ $Name:  $"};

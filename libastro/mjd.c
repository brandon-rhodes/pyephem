/* functions to manipulate the modified-julian-date used throughout xephem. */

#include <stdio.h>
#include <math.h>

#include "P_.h"
#include "astro.h"

/* given a date in months, mn, days, dy, years, yr,
 * return the modified Julian date (number of days elapsed since 1900 jan 0.5),
 * *mjd.
 */
void
cal_mjd (mn, dy, yr, mjd)
int mn, yr;
double dy;
double *mjd;
{
	static double last_mjd, last_dy;
	static int last_mn, last_yr;
	int b, d, m, y;
	long c;

	if (mn == last_mn && yr == last_yr && dy == last_dy) {
	    *mjd = last_mjd;
	    return;
	}

	m = mn;
	y = (yr < 0) ? yr + 1 : yr;
	if (mn < 3) {
	    m += 12;
	    y -= 1;
	}

	if (yr < 1582 || (yr == 1582 && (mn < 10 || (mn == 10 && dy < 15))))
	    b = 0;
	else {
	    int a;
	    a = y/100;
	    b = 2 - a + a/4;
	}

	if (y < 0)
	    c = (long)((365.25*y) - 0.75) - 694025L;
	else
	    c = (long)(365.25*y) - 694025L;

	d = (int)(30.6001*(m+1));

	*mjd = b + c + d + dy - 0.5;

	last_mn = mn;
	last_dy = dy;
	last_yr = yr;
	last_mjd = *mjd;
}

/* given the modified Julian date (number of days elapsed since 1900 jan 0.5,),
 * mjd, return the calendar date in months, *mn, days, *dy, and years, *yr.
 */
void
mjd_cal (mjd, mn, dy, yr)
double mjd;
int *mn, *yr;
double *dy;
{
	static double last_mjd, last_dy;
	static int last_mn, last_yr;
	double d, f;
	double i, a, b, ce, g;

	/* we get called with 0 quite a bit from unused epoch fields.
	 * 0 is noon the last day of 1899.
	 */
	if (mjd == 0.0) {
	    *mn = 12;
	    *dy = 31.5;
	    *yr = 1899;
	    return;
	}

	if (mjd == last_mjd) {
	    *mn = last_mn;
	    *yr = last_yr;
	    *dy = last_dy;
	    return;
	}

	d = mjd + 0.5;
	i = floor(d);
	f = d-i;
	if (f == 1) {
	    f = 0;
	    i += 1;
	}

	if (i > -115860.0) {
	    a = floor((i/36524.25)+.99835726)+14;
	    i += 1 + a - floor(a/4.0);
	}

	b = floor((i/365.25)+.802601);
	ce = i - floor((365.25*b)+.750001)+416;
	g = floor(ce/30.6001);
	*mn = (int)(g - 1);
	*dy = ce - floor(30.6001*g)+f;
	*yr = (int)(b + 1899);

	if (g > 13.5)
	    *mn = (int)(g - 13);
	if (*mn < 2.5)
	    *yr = (int)(b + 1900);
	if (*yr < 1)
	    *yr -= 1;

	last_mn = *mn;
	last_dy = *dy;
	last_yr = *yr;
	last_mjd = mjd;
}

/* given an mjd, set *dow to 0..6 according to which day of the week it falls
 * on (0=sunday).
 * return 0 if ok else -1 if can't figure it out.
 */
int
mjd_dow (mjd, dow)
double mjd;
int *dow;
{
	/* cal_mjd() uses Gregorian dates on or after Oct 15, 1582.
	 * (Pope Gregory XIII dropped 10 days, Oct 5..14, and improved the leap-
	 * year algorithm). however, Great Britian and the colonies did not
	 * adopt it until Sept 14, 1752 (they dropped 11 days, Sept 3-13,
	 * due to additional accumulated error). leap years before 1752 thus
	 * can not easily be accounted for from the cal_mjd() number...
	 */
	if (mjd < -53798.5) {
	    /* pre sept 14, 1752 too hard to correct |:-S */
	    return (-1);
	}
	*dow = ((long)floor(mjd-.5) + 1) % 7;/* 1/1/1900 (mjd 0.5) is a Monday*/
	if (*dow < 0)
	    *dow += 7;
	return (0);
}

/* given a year, return whether it is a leap year */
int
isleapyear (y)
int y;
{
	return ((y%4==0 && y%100!=0) || y%400==0);
}

/* given a mjd, return the the number of days in the month.  */
void
mjd_dpm (mjd, ndays)
double mjd;
int *ndays;
{
	static short dpm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int m, y;
	double d;

	mjd_cal (mjd, &m, &d, &y);
	*ndays = (m==2 && isleapyear(y)) ? 29 : dpm[m-1];
}

/* given a mjd, return the year and number of days since 00:00 Jan 1 */
void
mjd_dayno (mjd, yr, dy)
double mjd;
int *yr;
double *dy;
{
	double yrd;
	int yri;
	int dpy;

	mjd_year (mjd, &yrd);
	*yr = yri = (int)yrd;
	dpy = isleapyear(yri) ? 366 : 365;
	*dy = dpy*(yrd-yri);
}

/* given a mjd, return the year as a double. */
void
mjd_year (mjd, yr)
double mjd;
double *yr;
{
	static double last_mjd, last_yr;
	int m, y;
	double d;
	double e0, e1;	/* mjd of start of this year, start of next year */

	if (mjd == last_mjd) {
	    *yr = last_yr;
	    return;
	}

	mjd_cal (mjd, &m, &d, &y);
	if (y == -1) y = -2;
	cal_mjd (1, 1.0, y, &e0);
	cal_mjd (1, 1.0, y+1, &e1);
	*yr = y + (mjd - e0)/(e1 - e0);

	last_mjd = mjd;
	last_yr = *yr;
}

/* given a decimal year, return mjd */
void
year_mjd (y, mjd)
double y;
double *mjd;
{
	double e0, e1;	/* mjd of start of this year, start of next year */
	int yf = (int)floor (y);
	if (yf == -1) yf = -2;

	cal_mjd (1, 1.0, yf, &e0);
	cal_mjd (1, 1.0, yf+1, &e1);
	*mjd = e0 + (y - yf)*(e1-e0);
}

/* round a time in days, *t, to the nearest second, IN PLACE. */
void
rnd_second (t)
double *t;
{
	*t = floor(*t*SPD+0.5)/SPD;
}
	
/* given an mjd, truncate it to the beginning of the whole day */
double
mjd_day(jd)
double jd;
{
	return (floor(jd-0.5)+0.5);
}

/* given an mjd, return the number of hours past midnight of the whole day */
double
mjd_hr(jd)
double jd;
{
	return ((jd-mjd_day(jd))*24.0);
}

/* insure 0 <= *v < r.
 */
void
range (v, r)
double *v, r;
{
	*v -= r*floor(*v/r);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: mjd.c,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $"};

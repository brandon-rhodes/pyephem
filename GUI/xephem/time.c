/* get the time from the os.
 */

#include <stdio.h>
#include <time.h>
#include <math.h>

#include <Xm/Xm.h>

#include "xephem.h"


static long c0;
static double mjd0;

/* save current mjd and corresponding system clock for use by inc_mjd().
 * this establishes the base correspondence between the mjd and system clock.
 */
void
set_t0 (np)
Now *np;
{
	mjd0 = mjd;
	c0 = time (NULL);
}

/* fill in n_mjd from the system clock.
 * try to set timezone name and offset too but only if __STDC__ -- there's too
 *   many conflicting ways to do it otherwise.
 */
void
time_fromsys (np)
Now *np;
{
	time_t t;

	t = time(NULL);

	/* t is seconds since 00:00:00 1/1/1970 UTC on UNIX systems;
	 * mjd was 25567.5 then.
	 */
#if defined(VMS) && (__VMS_VER < 70000000)
	/* VMS returns t in seconds since 00:00:00 1/1/1970 Local Time
	 * so we need to add the timezone offset to get UTC.
	 * Don't need to worry about 'set_t0' and 'inc_mjd' because
	 * they only deal in relative times.
	 * this change courtesy Stephen Hirsch <oahirsch@southpower.co.nz>
         * - OpenVMS V7.0 finally has gmt support! so use standard time
	 * - algorithm Vance Haemmerle <vance@toyvax.Tucson.AZ.US>
	 */
	mjd = (25567.5 + t/3600.0/24.0) + (tz/24.0);
#else
	mjd = 25567.5 + t/3600.0/24.0;
#endif

	(void) tz_fromsys(np);
}

/* given the mjd within np, try to figure the timezone from the os.
 * return 0 if it looks like it worked, else -1.
 */
int
tz_fromsys (np)
Now *np;
{
	struct tm *gtmp;
	double m0;
	time_t t;

	/* if outside UNIX time range (~1901..2038) use 2000 */
	if (mjd < 713 || mjd > 50422) {
	    int m, y;
	    double d;
	    mjd_cal (mjd, &m, &d, &y);
	    y = 2000;
	    cal_mjd (m, d, y, &m0);
	} else
	    m0 = mjd;
	    

	/* UNIX uses a 1970 epoch */
	t = (time_t)((m0 - 25567.5) * (3600.0*24.0) + 0.5);

	/* try to find out timezone by comparing local with UTC time.
	 * GNU doesn't have difftime() so we do time math with doubles.
	 */
	gtmp = gmtime (&t);
	if (gtmp) {
	    double gmkt, lmkt;
	    struct tm *ltmp;

	    gtmp->tm_isdst = 0;	/* _should_ always be 0 already */
	    gmkt = (double) mktime (gtmp);

	    ltmp = localtime (&t);
	    ltmp->tm_isdst = 0;	/* let mktime() figure out zone */
	    lmkt = (double) mktime (ltmp);

	    tz = (gmkt - lmkt) / 3600.0;
	    (void) strftime (tznm, sizeof(tznm)-1, "%Z", ltmp);
	    return (0);
	} else
	    return (-1);
}

void
inc_mjd (np, inc, rev, rtcflag)
Now *np;
double inc;	/* hours to increment mjd */
int rev;	/* set if want to go in reverse */
int rtcflag;	/* relative since set mjd0 */
{
	if (rtcflag) {
	    if (rev)
		mjd = mjd0 - (time(NULL) - c0)/SPD;
	    else
		mjd = mjd0 + (time(NULL) - c0)/SPD;
	} else {
	    if (rev)
		mjd -= inc/24.0;
	    else
		mjd += inc/24.0;
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: time.c,v $ $Date: 2005/07/09 02:11:56 $ $Revision: 1.9 $ $Name:  $"};

#include "P_.h"
#include "astro.h"

static double gmst0 P_((double mjd));

/* given a modified julian date, mjd, and a universally coordinated time, utc,
 * return greenwich mean siderial time, *gst.
 * N.B. mjd must be at the beginning of the day.
 */
void
utc_gst (mjd, utc, gst)
double mjd;
double utc;
double *gst;
{
	static double lastmjd = -18981;
	static double t0;

	if (mjd != lastmjd) {
	    t0 = gmst0(mjd);
	    lastmjd = mjd;
	}
	*gst = (1.0/SIDRATE)*utc + t0;
	range (gst, 24.0);
}

/* given a modified julian date, mjd, and a greenwich mean siderial time, gst,
 * return universally coordinated time, *utc.
 * N.B. mjd must be at the beginning of the day.
 */
void
gst_utc (mjd, gst, utc)
double mjd;
double gst;
double *utc;
{
	static double lastmjd = -10000;
	static double t0;

	if (mjd != lastmjd) {
	    t0 = gmst0 (mjd);
	    lastmjd = mjd;
	}
	*utc = gst - t0;
	range (utc, 24.0);
	*utc *= SIDRATE;
}

/* gmst0() - return Greenwich Mean Sidereal Time at 0h UT; stern
 */
static double
gmst0 (mjd)
double mjd;	/* date at 0h UT in julian days since MJD0 */
{
	double T, x;

	T = ((int)(mjd - 0.5) + 0.5 - J2000)/36525.0;
	x = 24110.54841 +
		(8640184.812866 + (0.093104 - 6.2e-6 * T) * T) * T;
	x /= 3600.0;
	range(&x, 24.0);
	return (x);
}

#ifdef TEST_GMST

/* original routine by elwood; has a secular drift of 0.08s/cty */
static double
tnaught (mjd)
double mjd;	/* julian days since 1900 jan 0.5 */
{
	double dmjd;
	int m, y;
	double d;
	double t, t0;

	mjd_cal (mjd, &m, &d, &y);
	cal_mjd (1, 0., y, &dmjd);
	t = dmjd/36525;
	t0 = 6.57098e-2 * (mjd - dmjd) - 
	     (24 - (6.6460656 + (5.1262e-2 + (t * 2.581e-5))*t) -
		   (2400 * (t - (((double)y - 1900)/100))));
	range(&t0, 24.0);
	return (t0);
}

#include <stdlib.h>
main(argc, argv)
  int argc;
  char *argv[];
{
	double mjd, gst;
	while (scanf("%lf", &mjd) == 1) {
		mjd -= MJD0;
		gst = tnaught(mjd);
		printf("%17.9f %10.7f %10.7f\n", mjd + MJD0, gst, gmst0(mjd));
	}
}
#endif

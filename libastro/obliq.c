#include <stdio.h>

#include "P_.h"
#include "astro.h"

/* given the modified Julian date, mjd, find the mean obliquity of the
 * ecliptic, *eps, in radians.
 *
 * IAU expression (see e.g. Astron. Almanac 1984); stern
 */
void
obliquity (mjd, eps)
double mjd;
double *eps;
{
	static double lastmjd = -16347, lasteps;

	if (mjd != lastmjd) {
	    double t = (mjd - J2000)/36525.;	/* centuries from J2000 */
	    lasteps = degrad(23.4392911 +	/* 23^ 26' 21".448 */
			    t * (-46.8150 +
			    t * ( -0.00059 +
			    t * (  0.001813 )))/3600.0);
	    lastmjd = mjd;
	}
	*eps = lasteps;
}

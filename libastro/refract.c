#include <stdio.h>
#include <math.h>

#include "P_.h"
#include "astro.h"

/* correct the apparent altitude, aa, for refraction to the true altitude, ta,
 * each in radians, given the local atmospheric pressure, pr, in mbars, and
 * the temperature, tr, in degrees C.
 */
void
unrefract (pr, tr, aa, ta)
double pr, tr;
double aa;
double *ta;
{
	double r;	/* refraction correction*/

        if (aa >= degrad(15.)) {
	    /* model for altitudes at least 15 degrees above horizon */
            r = 7.888888e-5*pr/((273+tr)*tan(aa));
	} else {
	    /* better model for altitudes below 15 degrees */
	    double a, b, aadeg = raddeg(aa);
	    a = ((2e-5*aadeg+1.96e-2)*aadeg+1.594e-1)*pr;
	    b = (273+tr)*((8.45e-2*aadeg+5.05e-1)*aadeg+1);
	    r = degrad(a/b);
	}

	*ta  =  aa - r;
}

/* correct the true altitude, ta, for refraction to the apparent altitude, aa,
 * each in radians, given the local atmospheric pressure, pr, in mbars, and
 * the temperature, tr, in degrees C.
 */
void
refract (pr, tr, ta, aa)
double pr, tr;
double ta;
double *aa;
{
#define	MAXRERR	degrad(1./3600.)	/* desired accuracy, rads */

	double d, t, t0, a;

	/* first guess of error is to go backwards.
	 * make use that we know delta-apparent is always < delta-true.
	 */
	unrefract (pr, tr, ta, &t);
	d = 0.8*(ta - t);
	t0 = t;
	a = ta;

	/* use secant method to discover a value that unrefracts to ta.
	 * max=7 ave=2.4 loops in hundreds of test cases.
	 */
	do {
	    a += d;
	    unrefract (pr, tr, a, &t);
	    d *= -(ta - t)/(t0 - t);
	    t0 = t;
	} while (fabs(ta-t) > MAXRERR);

	*aa = a;

#undef	MAXRERR
}

#include <stdio.h>
#include <math.h>

#include "P_.h"
#include "astro.h"

static void ecleq_aux P_((int sw, double mjd, double x, double y,
    double *p, double *q));

#define	EQtoECL	1
#define	ECLtoEQ	(-1)


/* given the modified Julian date, mjd, and an equitorial ra and dec, each in
 * radians, find the corresponding geocentric ecliptic latitude, *lat, and
 * longititude, *lng, also each in radians.
 * correction for the effect on the angle of the obliquity due to nutation is
 * not included.
 */
void
eq_ecl (mjd, ra, dec, lat, lng)
double mjd;
double ra, dec;
double *lat, *lng;
{
	ecleq_aux (EQtoECL, mjd, ra, dec, lng, lat);
}

/* given the modified Julian date, mjd, and a geocentric ecliptic latitude,
 * *lat, and longititude, *lng, each in radians, find the corresponding
 * equitorial ra and dec, also each in radians.
 * correction for the effect on the angle of the obliquity due to nutation is
 * not included.
 */
void
ecl_eq (mjd, lat, lng, ra, dec)
double mjd;
double lat, lng;
double *ra, *dec;
{
	ecleq_aux (ECLtoEQ, mjd, lng, lat, ra, dec);
}

static void
ecleq_aux (sw, mjd, x, y, p, q)
int sw;			/* +1 for eq to ecliptic, -1 for vv. */
double mjd;
double x, y;		/* sw==1: x==ra, y==dec.  sw==-1: x==lng, y==lat. */
double *p, *q;		/* sw==1: p==lng, q==lat. sw==-1: p==ra, q==dec. */
{
	static double lastmjd = -10000;	/* last mjd calculated */
	static double seps, ceps;	/* sin and cos of mean obliquity */
	double sx, cx, sy, cy, ty, sq;

	if (mjd != lastmjd) {
	    double eps;
	    obliquity (mjd, &eps);		/* mean obliquity for date */
    	    seps = sin(eps);
	    ceps = cos(eps);
	    lastmjd = mjd;
	}

	sy = sin(y);
	cy = cos(y);				/* always non-negative */
        if (fabs(cy)<1e-20) cy = 1e-20;		/* insure > 0 */
        ty = sy/cy;
	cx = cos(x);
	sx = sin(x);
        sq = (sy*ceps)-(cy*seps*sx*sw);
	if (sq < -1) sq = -1;
	if (sq >  1) sq =  1;
        *q = asin(sq);
        *p = atan(((sx*ceps)+(ty*seps*sw))/cx);
        if (cx<0) *p += PI;		/* account for atan quad ambiguity */
	range (p, 2*PI);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: eq_ecl.c,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $"};

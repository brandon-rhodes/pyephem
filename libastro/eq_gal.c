/* code to convert between equitorial and galactic coordinates */

#include <stdio.h>
#include <math.h>

#include "P_.h"
#include "astro.h"

static void galeq_aux P_((int sw, double x, double y, double *p, double *q));
static void galeq_init P_((void));

#define	EQtoGAL	1
#define	GALtoEQ	(-1)
#define	SMALL	(1e-20)

static double an = degrad(33.0);	/* G lng of asc node on equator */
static double gpr = degrad(192.255);	/* RA of North Gal Pole, 2000 */
static double gpd = degrad(27.40);	/* Dec of  " */
static double cgpd, sgpd;		/* cos() and sin() of gpd */
static double mjd2000;			/* mjd of 2000 */
static int before;			/* whether these have been set yet */

/* given ra and dec, each in radians, for the given epoch, find the
 * corresponding galactic latitude, *lat, and longititude, *lng, also each in
 * radians.
 */
void
eq_gal (mjd, ra, dec, lat, lng)
double mjd, ra, dec;
double *lat, *lng;
{
	galeq_init();
	precess (mjd, mjd2000, &ra, &dec);
	galeq_aux (EQtoGAL, ra, dec, lng, lat);
}

/* given galactic latitude, lat, and longititude, lng, each in radians, find
 * the corresponding equitorial ra and dec, also each in radians, at the 
 * given epoch.
 */
void
gal_eq (mjd, lat, lng, ra, dec)
double mjd, lat, lng;
double *ra, *dec;
{
	galeq_init();
	galeq_aux (GALtoEQ, lng, lat, ra, dec);
	precess (mjd2000, mjd, ra, dec);
}

static void
galeq_aux (sw, x, y, p, q)
int sw;			/* +1 for eq to gal, -1 for vv. */
double x, y;		/* sw==1: x==ra, y==dec.  sw==-1: x==lng, y==lat. */
double *p, *q;		/* sw==1: p==lng, q==lat. sw==-1: p==ra, q==dec. */
{
	double sy, cy, a, ca, sa, b, sq, c, d; 

	cy = cos(y);
	sy = sin(y);
	a = x - an;
	if (sw == EQtoGAL)
	    a = x - gpr;
	ca = cos(a);
	sa = sin(a);
	b = sa;
	if (sw == EQtoGAL)
	    b = ca;
	sq = (cy*cgpd*b) + (sy*sgpd);
	*q = asin (sq);

	if (sw == GALtoEQ) {
	    c = cy*ca;
	    d = (sy*cgpd) - (cy*sgpd*sa);
	    if (fabs(d) < SMALL)
		d = SMALL;
	    *p = atan (c/d) + gpr;
	} else {
	    c = sy - (sq*sgpd);
	    d = cy*sa*cgpd;
	    if (fabs(d) < SMALL)
		d = SMALL;
	    *p = atan (c/d) + an;
	}

	if (d < 0) *p += PI;
	if (*p < 0) *p += 2*PI;
	if (*p > 2*PI) *p -= 2*PI;
}

/* set up the definitions */
static void
galeq_init()
{
	if (!before) {
	    cgpd = cos (gpd);
	    sgpd = sin (gpd);
	    mjd2000 = J2000;
	    before = 1;
	}
}

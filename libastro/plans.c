/* rewritten for Bureau des Longitude theories by Bretagnon and Chapront
 * Michael Sternberg <sternberg@physik.tu-chemnitz.de>
 */
#include <stdio.h>
#include <math.h>

#include "P_.h"
#include "astro.h"
#include "vsop87.h"
#include "chap95.h"

static void pluto_ell P_((double mjd, double *ret));
static void chap_trans P_((double mjd, double *ret));
static void planpos P_((double mjd, int obj, double prec, double *ret));

/* coordinate transformation
 * from:
 *	J2000.0 rectangular equatoreal			ret[{0,1,2}] = {x,y,z}
 * to:
 *	mean equinox of date spherical ecliptical	ret[{0,1,2}] = {l,b,r}
 */
static void
chap_trans (mjd, ret)
double mjd;	/* destination epoch */
double *ret;	/* vector to be transformed _IN PLACE_ */
{
	double ra, dec, r, eps;
	double sr, cr, sd, cd, se, ce;

	cartsph(ret[0], ret[1], ret[2], &ra, &dec, &r);
	precess(J2000, mjd, &ra, &dec);
	obliquity(mjd, &eps);
	sr = sin(ra); cr = cos(ra);
	sd = sin(dec); cd = cos(dec);
	se = sin(eps); ce = cos(eps);
	ret[0] = atan2( sr * ce + sd/cd * se, cr);	/* long */
	ret[1] = asin( sd * ce - cd * se * sr);		/* lat */
	ret[2] = r;					/* radius */
}

/* low precision ecliptic coordinates of Pluto from mean orbit.
 * Only for sake of completeness outside available perturbation theories.
 */
static void
pluto_ell (mjd, ret)
double mjd;	/* epoch */
double *ret;	/* ecliptic coordinates {l,b,r} at equinox of date */
{
	/* mean orbital elements of Pluto.
	 * The origin of these is somewhat obscure.
	 */
	double	a = 39.543,			/* semimajor axis, au */
		e = 0.2490,			/* excentricity */
		inc0 = 17.140,			/* inclination, deg */
		Om0 = 110.307,			/* long asc node, deg */
		omeg0 = 113.768,		/* arg of perihel, deg */
		mjdp = 2448045.539 - MJD0,	/* epoch of perihel */
		mjdeq = J2000,			/* equinox of elements */
		n = 144.9600/36525.;            /* daily motion, deg */

	double inc, Om, omeg;	/* orbital elements at epoch of date */
	double ma, ea, nu;	/* mean, excentric and true anomaly */
	double lo, slo, clo;	/* longitude in orbit from asc node */

	reduce_elements(mjdeq, mjd, degrad(inc0), degrad(omeg0), degrad(Om0),
				&inc, &omeg, &Om);
	ma = degrad((mjd - mjdp) * n);
	anomaly(ma, e, &nu, &ea);
	ret[2] = a * (1.0 - e*cos(ea));			/* r */
	lo = omeg + nu;
	slo = sin(lo);
	clo = cos(lo);
	ret[1] = asin(slo * sin(inc));			/* b */
	ret[0] = atan2(slo * cos(inc), clo) + Om;	/* l */
}

/*************************************************************/

/* geometric heliocentric position of planet, mean ecliptic of date
 * (not corrected for light-time)
 */
static void
planpos (mjd, obj, prec, ret)
double mjd;
int obj;
double prec;
double *ret;
{
	if (mjd >= CHAP_BEGIN && mjd <= CHAP_END) {
	    if (obj >= JUPITER) {		/* prefer Chapront */
		chap95(mjd, obj, prec, ret);
		chap_trans (mjd, ret);
	    } else {				/* VSOP for inner planets */
		vsop87(mjd, obj, prec, ret);
	    }
	} else {				/* outside Chapront time: */
	    if (obj != PLUTO) {			/* VSOP for all but Pluto */
		vsop87(mjd, obj, prec, ret);
	    } else {				/* Pluto mean elliptic orbit */
		pluto_ell(mjd, ret);
	    }
	}
}

/*************************************************************/

/* visual elements of planets
 * [planet][0] = angular size at 1 AU
 * [planet][1] = magnitude at 1 AU from sun and earth and 0 deg phase angle
 * [planet][2] = A
 * [planet][3] = B
 * [planet][4] = C
 *   where mag correction = A*(i/100) + B*(i/100)^2 + C*(i/100)^3
 *      i = angle between sun and earth from planet, degrees
 * from Explanatory Supplement, 1992
 */
static double vis_elements[8][5] = {
	/* Mercury */	{ 6.74, -0.36, 3.8, -2.73, 2.00},
	/* Venus */	{ 16.92, -4.29, 0.09, 2.39, -.65},
	/* Mars */	{ 9.36, -1.52, 1.60, 0., 0.},
	/* Jupiter */	{ 196.74, -9.25, 0.50, 0., 0.},
	/* Saturn */	{ 165.6, -8.88, 4.40, 0., 0.},
	/* Uranus */	{ 65.8, -7.19, 0.28, 0., 0.},
	/* Neptune */	{ 62.2, -6.87, 0., 0., 0.},
	/* Pluto */	{ 8.2, -1.01, 4.1, 0., 0.}
};

/* given a modified Julian date, mjd, and a planet, p, find:
 *   lpd0: heliocentric longitude, 
 *   psi0: heliocentric latitude,
 *   rp0:  distance from the sun to the planet, 
 *   rho0: distance from the Earth to the planet,
 *         none corrected for light time, ie, they are the true values for the
 *         given instant.
 *   lam:  geocentric ecliptic longitude, 
 *   bet:  geocentric ecliptic latitude,
 *         each corrected for light time, ie, they are the apparent values as
 *	   seen from the center of the Earth for the given instant.
 *   dia:  angular diameter in arcsec at 1 AU, 
 *   mag:  visual magnitude
 *
 * all angles are in radians, all distances in AU.
 *
 * corrections for nutation and abberation must be made by the caller. The RA 
 *   and DEC calculated from the fully-corrected ecliptic coordinates are then
 *   the apparent geocentric coordinates. Further corrections can be made, if
 *   required, for atmospheric refraction and geocentric parallax.
 */
void
plans (mjd, p, lpd0, psi0, rp0, rho0, lam, bet, dia, mag)
double mjd;
int p;
double *lpd0, *psi0, *rp0, *rho0, *lam, *bet, *dia, *mag;
{
	static double lastmjd = -10000;
	static double lsn, bsn, rsn;	/* geocentric coords of sun */
	static double xsn, ysn, zsn;	/* cartesian " */
	double lp, bp, rp;		/* heliocentric coords of planet */
	double xp, yp, zp, rho;		/* rect. coords and geocentric dist. */
	double dt;			/* light time */
	double *vp;			/* vis_elements[p] */
	double ci, i;			/* sun/earth angle: cos, degrees */
	int pass;

	/* get sun cartesian; needed only once at mjd */
	if (mjd != lastmjd) {
	    sunpos (mjd, &lsn, &rsn, &bsn);
	    sphcart (lsn, bsn, rsn, &xsn, &ysn, &zsn);
            lastmjd = mjd;
        }

	/* first find the true position of the planet at mjd.
	 * then repeat a second time for a slightly different time based
	 * on the position found in the first pass to account for light-travel
	 * time.
	 */
	dt = 0.0;
	for (pass = 0; pass < 2; pass++) {
	    double ret[6];

	    /* get spherical coordinates of planet from precision routines,
	     * retarded for light time in second pass;
	     * alternative option:  vsop allows calculating rates.
	     */
	    planpos(mjd - dt, p, 0.0, ret);

	    lp = ret[0];
	    bp = ret[1];
	    rp = ret[2];

	    sphcart (lp, bp, rp, &xp, &yp, &zp);
	    cartsph (xp + xsn, yp + ysn, zp + zsn, lam, bet, &rho);

	    if (pass == 0) {
		/* save heliocentric coordinates at first pass since, being
		 * true, they are NOT to be corrected for light-travel time.
		 */
		*lpd0 = lp;
		range (lpd0, 2.*PI);
		*psi0 = bp;
		*rp0 = rp;
		*rho0 = rho;
	    }

	    /* when we view a planet we see it in the position it occupied
	     * dt days ago, where rho is the distance between it and earth,
	     * in AU. use this as the new time for the next pass.
	     */
	    dt = rho * 5.7755183e-3;
	}

	vp = vis_elements[p];
	*dia = vp[0];

	/* solve plane triangle, assume sun/earth dist == 1 */
	ci = (rp*rp + rho*rho - 1)/(2*rp*rho);

	/* expl supp equation for mag */
	if (ci < -1) ci = -1;
	if (ci >  1) ci =  1;
	i = raddeg(acos(ci))/100.;
	*mag = vp[1] + 5*log10(rho*rp) + i*(vp[2] + i*(vp[3] + i*vp[4]));

	/* rings contribution if SATURN */
	if (p == SATURN) {
	    double et, st, set;
	    satrings (bp, lp, rp, lsn+PI, rsn, mjd+MJD0, &et, &st);
	    set = sin(fabs(et));
	    *mag += (-2.60 + 1.25*set)*set;
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: plans.c,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $"};

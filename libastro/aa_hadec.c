/* function to convert between alt/az and ha/dec.
 */

#include <stdio.h>
#include <math.h>

#include "P_.h"
#include "astro.h"

static void aaha_aux P_((double lat, double x, double y, double *p, double *q));

/* given geographical latitude (n+, radians), lat, altitude (up+, radians),
 * alt, and azimuth (angle round to the east from north+, radians),
 * return hour angle (radians), ha, and declination (radians), dec.
 */
void
aa_hadec (lat, alt, az, ha, dec)
double lat;
double alt, az;
double *ha, *dec;
{
	aaha_aux (lat, az, alt, ha, dec);
	if (*ha > PI)
	    *ha -= 2*PI;
}

/* given geographical (n+, radians), lat, hour angle (radians), ha, and
 * declination (radians), dec, return altitude (up+, radians), alt, and
 * azimuth (angle round to the east from north+, radians),
 */
void
hadec_aa (lat, ha, dec, alt, az)
double lat;
double ha, dec;
double *alt, *az;
{
	aaha_aux (lat, ha, dec, az, alt);
}

#ifdef NEED_GEOC
/* given a geographic (surface-normal) latitude, phi, return the geocentric
 * latitude, psi.
 */
double
geoc_lat (phi)
double phi;
{
#define	MAXLAT	degrad(89.9999)	/* avoid tan() greater than this */
	return (fabs(phi)>MAXLAT ? phi : atan(tan(phi)/1.00674));
}
#endif

/* the actual formula is the same for both transformation directions so
 * do it here once for each way.
 * N.B. all arguments are in radians.
 */
static void
aaha_aux (lat, x, y, p, q)
double lat;
double x, y;
double *p, *q;
{
	static double last_lat = -3434, slat, clat;
	double cap, B;

	if (lat != last_lat) {
	    slat = sin(lat);
	    clat = cos(lat);
	    last_lat = lat;
	}

	solve_sphere (-x, PI/2-y, slat, clat, &cap, &B);
	*p = B;
	*q = PI/2 - acos(cap);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: aa_hadec.c,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $"};

#include <stdio.h>
#include <math.h>

#include "P_.h"
#include "astro.h"

/* given the true geocentric ra and dec of an object, the observer's latitude,
 *   lat, and a horizon displacement correction, dis, all in radians, find the
 *   local sidereal times and azimuths of rising and setting, lstr/s
 *   and azr/s, also all in radians, respectively.
 * dis is the vertical displacement from the true position of the horizon. it
 *   is positive if the apparent position is higher than the true position.
 *   said another way, it is positive if the shift causes the object to spend
 *   longer above the horizon. for example, atmospheric refraction is typically
 *   assumed to produce a vertical shift of 34 arc minutes at the horizon; dis
 *   would then take on the value +9.89e-3 (radians). On the other hand, if
 *   your horizon has hills such that your apparent horizon is, say, 1 degree
 *   above sea level, you would allow for this by setting dis to -1.75e-2
 *   (radians).
 *
 * This version contributed by Konrad Bernloehr, Nov. 1996
 *
 * status: 0=normal; 1=never rises; -1=circumpolar.
 * In case of non-zero status, all other returned variables are undefined.
 */
void
riset (ra, dec, lat, dis, lstr, lsts, azr, azs, status)
double ra, dec;
double lat, dis;
double *lstr, *lsts;
double *azr, *azs;
int *status;
{
#define	EPS	(1e-9)	/* math rounding fudge - always the way, eh? */
	double h;		/* hour angle */
	double cos_h;		/* cos h */
	double z;		/* zenith angle */
	double zmin, zmax;	/* Minimum and maximum zenith angles */
	double xaz, yaz;	/* components of az */
	int shemi;		/* flag for southern hemisphere reflection */

	/* reflect lat and dec if in southern hemisphere, then az back later */
	if ((shemi= (lat < 0.)) != 0) {
	    lat = -lat;
	    dec = -dec;
	}

	/* establish zenith angle, and its extrema */
	z = (PI/2.) + dis;
	zmin = fabs (dec - lat);
	zmax = PI - fabs(dec + lat);

	/* first consider special cases.
	 * these also avoid any boundary problems in subsequent computations.
	 */
	if (zmax <= z + EPS) {
	    *status = -1;	/* never sets */
	    return;
	}
	if (zmin >= z - EPS) {
	    *status = 1;	/* never rises */
	    return;
	}

	/* compute rising hour angle -- beware found off */
	cos_h = (cos(z)-sin(lat)*sin(dec))/(cos(lat)*cos(dec));
	if (cos_h >= 1.)
	    h =  0.;
	else if (cos_h <= -1.)
	    h = PI;
	else
	    h = acos (cos_h);

	/* compute setting azimuth -- beware found off */
	xaz = sin(dec)*cos(lat)-cos(dec)*cos(h)*sin(lat);
	yaz = -1.*cos(dec)*sin(h);
	if (xaz == 0.) {
	    if (yaz > 0)
		*azs = PI/2;
	    else
		*azs = -PI/2;
	} else
	    *azs = atan2 (yaz, xaz);

	/* reflect az back if southern */
	if (shemi)
	    *azs = PI - *azs;
	range(azs, 2.*PI);

	/* rising is just the opposite side */
	*azr = 2.*PI - *azs;
	range(azr, 2.*PI);

	/* rise and set are just ha either side of ra */
	*lstr = radhr(ra-h);
	range(lstr,24.0);
	*lsts = radhr(ra+h);
	range(lsts,24.0);

	/* OK */
	*status = 0;
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: riset.c,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $"};

#include <math.h>
#include "P_.h"
#include "astro.h"

/* transformation from spherical to cartesian coordinates */
void
sphcart (l, b, r, x, y, z)
double l, b, r;		/* source: spherical coordinates */
double *x, *y, *z;	/* result: rectangular coordinates */
{
	double rcb = r * cos(b);

	*x = rcb * cos(l);
	*y = rcb * sin(l);
	*z = r * sin(b);
}

/* transformation from cartesian to spherical coordinates */
void
cartsph (x, y, z, l, b, r)
double x, y, z;		/* source: rectangular coordinates */
double *l, *b, *r;	/* result: spherical coordinates */
{
	double rho = x*x + y*y;

	if (rho > 1e-35) {	/* standard case: off axis */
	    *l = atan2(y, x);
	    range (l, 2*PI);
	    *b = atan2(z, sqrt(rho));
	    *r = sqrt(rho + z*z);
	} else {		/* degenerate case; avoid math error */
	    *l = 0.0;
	    if (z == 0.0)
		*b = 0.0;
	    else
		*b = (z > 0.0) ? PI/2. : -PI/2.;
	    *r = fabs(z);
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: sphcart.c,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $"};

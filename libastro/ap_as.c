#include <string.h>

#include "P_.h"
#include "astro.h"
#include "circum.h"

/* convert the given apparent RA/Dec to astrometric precessed to Mjd IN PLACE.
 * we have no un-abberation etc so to find the correction: assume
 * *rap and *decp are astrometric@EOD, convert to apparent and back out
 * the difference; then precess to Mjd.
 */
void
ap_as (np, Mjd, rap, decp)
Now *np;
double Mjd;
double *rap, *decp;
{
	Obj o;
	Now n;

	zero_mem ((void *)&o, sizeof(o));
	o.o_type = FIXED;
	o.f_RA = (float)*rap;
	o.f_dec = (float)*decp;
	o.f_epoch = (float)mjd;
	memcpy ((void *)&n, (void *)np, sizeof(Now));
	n.n_epoch = EOD;
	obj_cir (&n, &o);
	*rap -= o.s_ra - *rap;
	*decp -= o.s_dec - *decp;
	if (*decp >  PI/2) {*decp =  PI - *decp; *rap += PI; }
	if (*decp < -PI/2) {*decp = -PI - *decp; *rap += PI; }
	range (rap, 2*PI);
	precess (mjd, Mjd, rap, decp);
}

/* convert the given astrometric RA/Dec which are precessed to Mjd into
 * apparent @ EOD IN PLACE.
 */
void
as_ap (np, Mjd, rap, decp)
Now *np;
double Mjd;
double *rap, *decp;
{
	Obj o;
	Now n;

	zero_mem ((void *)&o, sizeof(o));
	o.o_type = FIXED;
	o.f_RA = (float)*rap;
	o.f_dec = (float)*decp;
	o.f_epoch = (float)Mjd;
	memcpy ((void *)&n, (void *)np, sizeof(Now));
	n.n_epoch = EOD;
	obj_cir (&n, &o);
	*rap = o.s_ra;
	*decp = o.s_dec;
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: ap_as.c,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $"};

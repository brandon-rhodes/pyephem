/* find rise and set circumstances, ie, riset_cir() and related functions. */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "astro.h"

#define	TMACC	(10./3600./24.0)	/* convergence accuracy, days */

static void e_riset_cir (Now *np, Obj *op, double dis, RiseSet *rp);
static int find_0alt (double dt, double dis, Now *np, Obj *op);
static int find_transit (double dt, Now *np, Obj *op);
static int find_max (Now *np, Obj *op, double tr, double ts, double *tp,
    double *alp);

/* find where and when an object, op, will rise and set and
 *   it's transit circumstances. all times are utc mjd, angles rads e of n.
 * dis is the angle down from an ideal horizon, in rads (see riset()).
 * N.B. dis should NOT include refraction, we do that here.
 */
void
riset_cir (Now *np, Obj *op, double dis, RiseSet *rp)
{
	double mjdn;	/* mjd of local noon */
	double lstn;	/* lst at local noon */
	double lr, ls;	/* lst rise/set times */
	double ar, as;	/* az of rise/set */
	double ran;	/* RA at noon */
	Now n;		/* copy to move time around */
	Obj o;		/* copy to get circumstances at n */
	int rss;	/* temp status */

	/* work with local copies so we can move the time around */
	(void) memcpy ((void *)&n, (void *)np, sizeof(n));
	(void) memcpy ((void *)&o, (void *)op, sizeof(o));

	/* fast Earth satellites need a different approach.
	 * "fast" here is pretty arbitrary -- just too fast to work with the
	 * iterative approach based on refining the times for a "fixed" object.
	 */
	if (op->o_type == EARTHSAT && op->es_n > FAST_SAT_RPD) {
	    e_riset_cir (&n, &o, dis, rp);
	    return;
	}

	/* assume no problems initially */
	rp->rs_flags = 0;

	/* start the iteration at local noon */
	mjdn = mjd_day(mjd - tz/24.0) + tz/24.0 + 0.5;
	n.n_mjd = mjdn;
	now_lst (&n, &lstn);

	/* first approximation is to find rise/set times of a fixed object
	 * at the current epoch in its position at local noon.
	 * N.B. add typical refraction if dis is above horizon for initial
	 *   go/no-go test. if it passes, real code does refraction rigorously.
	 */
	n.n_mjd = mjdn;
	if (obj_cir (&n, &o) < 0) {
	    rp->rs_flags = RS_ERROR;
	    return;
	}
	ran = o.s_gaera;
	riset (o.s_gaera, o.s_gaedec, lat, dis+(dis>.01 ? 0 : .01), &lr, &ls,
								&ar, &as, &rss);
	switch (rss) {
	case  0:  break;
	case  1: rp->rs_flags = RS_NEVERUP; return;
	case -1: rp->rs_flags = RS_CIRCUMPOLAR; goto dotransit;
	default: rp->rs_flags = RS_ERROR; return;
	}

	/* iterate to find better rise time */
	n.n_mjd = mjdn;
	switch (find_0alt ((lr - lstn)/SIDRATE, dis, &n, &o)) {
	case 0: /* ok */
	    rp->rs_risetm = n.n_mjd;
	    rp->rs_riseaz = o.s_az;
	    break;
	case -1: /* obj_cir error */
	    rp->rs_flags |= RS_RISERR;
	    break;
	case -2: /* converged but not today, err but give times anyway */
	    rp->rs_risetm = n.n_mjd;
	    rp->rs_riseaz = o.s_az;
	    rp->rs_flags |= RS_NORISE;
	    break;
	case -3: /* probably never up */
	    rp->rs_flags |= RS_NEVERUP;
	    break;
	}

	/* iterate to find better set time */
	n.n_mjd = mjdn;
	switch (find_0alt ((ls - lstn)/SIDRATE, dis, &n, &o)) {
	case 0: /* ok */
	    rp->rs_settm = n.n_mjd;
	    rp->rs_setaz = o.s_az;
	    break;
	case -1: /* obj_cir error */
	    rp->rs_flags |= RS_SETERR;
	    break;
	case -2: /* converged but not today, err but give times anyway */
	    rp->rs_settm = n.n_mjd;
	    rp->rs_setaz = o.s_az;
	    rp->rs_flags |= RS_NOSET;
	    break;
	case -3: /* probably circumpolar */
	    rp->rs_flags |= RS_CIRCUMPOLAR;
	    break;
	}

	/* can try transit even if rise or set failed */
    dotransit:
	n.n_mjd = mjdn;
	switch (find_transit ((radhr(ran) - lstn)/SIDRATE, &n, &o)) {
	case 0: /* ok */
	    rp->rs_trantm = n.n_mjd;
	    rp->rs_tranalt = o.s_alt;
	    break;
	case -1: /* did not converge */
	    rp->rs_flags |= RS_TRANSERR;
	    break;
	case -2: /* converged but not today */
	    rp->rs_flags |= RS_NOTRANS;
	    break;
	}
}

/* find local times when sun is dis rads below horizon.
 */
void
twilight_cir (Now *np, double dis, double *dawn, double *dusk, int *status)
{
	RiseSet rs;
	Obj o;

	memset (&o, 0, sizeof(o));
	o.o_type = PLANET;
	o.pl_code = SUN;
	(void) strcpy (o.o_name, "Sun");
	riset_cir (np, &o, dis, &rs);
	*dawn = rs.rs_risetm;
	*dusk = rs.rs_settm;
	*status = rs.rs_flags;
}

/* find where and when a fast-moving Earth satellite, op, will rise and set and
 *   it's transit circumstances. all times are mjd, angles rads e of n.
 * dis is the angle down from the local topo horizon, in rads (see riset()).
 * idea is to walk forward in time looking for alt+dis==0 crossings.
 * initial time step is a few degrees (based on average daily motion).
 * we stop as soon as we see both a rise and set.
 * N.B. we assume *np and *op are working copies we can mess up.
 */
static void
e_riset_cir (Now *np, Obj *op, double dis, RiseSet *rp)
{
#define	DEGSTEP	5		/* time step is about this many degrees */
	int steps;		/* max number of time steps */
	double dt;		/* time change per step, days */
	double t0, t1;		/* current and next mjd values */
	double a0, a1;		/* altitude at t0 and t1 */
	int rise, set;		/* flags to check when we find these events */
	int i;

	dt = DEGSTEP * (1.0/360.0/op->es_n);
	steps = (int)(1.0/dt);
	rise = set = 0;
	rp->rs_flags = 0;

	if (obj_cir (np, op) < 0) {
	    rp->rs_flags |= RS_ERROR;
	    return;
	}

	t0 = mjd;
	a0 = op->s_alt + dis;

	for (i = 0; i < steps && (!rise || !set); i++) {
	    mjd = t1 = t0 + dt;
	    if (obj_cir (np, op) < 0) {
		rp->rs_flags |= RS_ERROR;
		return;
	    }
	    a1 = op->s_alt + dis;

	    if (a0 < 0 && a1 > 0 && !rise) {
		/* found a rise event -- interate to refine */
		switch (find_0alt (0.0, dis, np, op)) {
		case 0: /* ok */
		    rp->rs_risetm = np->n_mjd;
		    rp->rs_riseaz = op->s_az;
		    rise = 1;
		    break;
		case -1: /* obj_cir error */
		    rp->rs_flags |= RS_RISERR;
		    return;
		case -2: /* converged but not today */ /* FALLTHRU */
		case -3: /* probably never up */
		    rp->rs_flags |= RS_NORISE;
		    return;
		}
	    } else if (a0 > 0 && a1 < 0 && !set) {
		/* found a setting event -- interate to refine */
		switch (find_0alt (0.0, dis, np, op)) {
		case 0: /* ok */
		    rp->rs_settm = np->n_mjd;
		    rp->rs_setaz = op->s_az;
		    set = 1;
		    break;
		case -1: /* obj_cir error */
		    rp->rs_flags |= RS_SETERR;
		    return;
		case -2: /* converged but not today */ /* FALLTHRU */
		case -3: /* probably circumpolar */
		    rp->rs_flags |= RS_NOSET;
		    return;
		}
	    }

	    t0 = t1;
	    a0 = a1;
	}

	/* instead of transit, for satellites we find time of maximum
	 * altitude, if we know both the rise and set times.
	 */
	if (rise && set) {
	    double tt, al;
	    if (find_max (np, op, rp->rs_risetm, rp->rs_settm, &tt, &al) < 0) {
		rp->rs_flags |= RS_TRANSERR;
		return;
	    }
	    rp->rs_trantm = tt;
	    rp->rs_tranalt = al;
	} else
	    rp->rs_flags |= RS_NOTRANS;

	/* check for some bad conditions */
	if (!rise) {
	    if (a0 > 0)
		rp->rs_flags |= RS_CIRCUMPOLAR;
	    else
		rp->rs_flags |= RS_NORISE;
	}
	if (!set) {
	    if (a0 < 0)
		rp->rs_flags |= RS_NEVERUP;
	    else
		rp->rs_flags |= RS_NOSET;
	}
}

/* given a Now at noon and a dt from noon, in hours, for a first approximation
 * to a rise or set event, refine the event by searching for when alt+dis = 0.
 * return 0: if find one within 12 hours of noon with np and op set to the
 *    better time and circumstances;
 * return -1: if error from obj_cir;
 * return -2: if converges but not today;
 * return -3: if does not converge at all (probably circumpolar or never up);
 */
static int
find_0alt (
double dt,	/* hours from noon to first guess at event */
double dis,	/* horizon displacement, rads */
Now *np,	/* working Now -- starts with mjd is noon, returns as answer */
Obj *op)	/* working object -- returns as answer */
{
#define	MAXPASSES	20		/* max iterations to try */
#define	FIRSTSTEP	(1.0/60.0/24.0)	/* first time step, days */
#define	MAXSTEP		(12.0/24.0)/* max time step,days (to detect flat)*/

	double a0 = 0;
	double mjdn = mjd;
	int npasses;

	/* insure initial guess is today -- if not, move by 24 hours */
	if (dt < -12.0 && !find_0alt (dt+24, dis, np, op))
	    return (0);
	mjd = mjdn;
	if (dt > 12.0 && !find_0alt (dt-24, dis, np, op))
	    return (0);
	mjd = mjdn;
	
	/* convert dt to days for remainder of algorithm */
	dt /= 24.0;

	/* use secant method to look for s_alt + dis == 0 */
	npasses = 0;
	do {
	    double a1;

	    mjd += dt;
	    if (obj_cir (np, op) < 0)
		return (-1);
	    a1 = op->s_alt;

	    dt = (npasses == 0) ? FIRSTSTEP : (dis+a1)*dt/(a0-a1);
	    a0 = a1;

	    if (++npasses > MAXPASSES || fabs(dt) >= MAXSTEP)
		return (-3);

	} while (fabs(dt)>TMACC);

	/* return codes */
	return (fabs(mjdn-mjd) < .5 ? 0 : -2);

#undef	MAXPASSES
#undef	FIRSTSTEP
#undef	MAXSTEP
}

/* find when the given object transits. start the search when LST matches the
 *   object's RA at noon.
 * if ok, return 0 with np and op set to the transit conditions; if can't
 *   converge return -1; if converges ok but not today return -2.
 * N.B. we assume np is passed set to local noon.
 */
static int
find_transit (double dt, Now *np, Obj *op)
{
#define	MAXLOOPS	10
#define	MAXERR		(0.25/60.)		/* hours */
	double mjdn = mjd;
	double lst;
	int i;

	/* insure initial guess is today -- if not, move by 24 hours */
	if (dt < -12.0)
	    dt += 24.0;
	if (dt > 12.0)
	    dt -= 24.0;

	i = 0;
	do {
	    mjd += dt/24.0;
	    if (obj_cir (np, op) < 0)
		return (-1);
	    now_lst (np, &lst);
	    dt = (radhr(op->s_gaera) - lst);
	    if (dt < -12.0)
		dt += 24.0;
	    if (dt > 12.0)
		dt -= 24.0;
	} while (++i < MAXLOOPS && fabs(dt) > MAXERR);

	/* return codes */
	if (i == MAXLOOPS)
	    return (-1);
	return (fabs(mjd - mjdn) < 0.5 ? 0 : -2);

#undef	MAXLOOPS
#undef	MAXERR
}

/* find the mjd time of max altitude between the given rise and set times.
 * N.B. we assume *np and *op are working copies we can mess up.
 * N.B. we just assume max occurs at the center time.
 * return 0 if ok, else -1.
 */
static int
find_max (
Now *np,
Obj *op,
double tr, double ts,		/* times of rise and set */
double *tp, double *alp)	/* time of max altitude, and that altitude */
{
	/* want rise before set */
	while (ts < tr)
	    tr -= 1.0/op->es_n;
	mjd = (ts + tr)/2;
	if (obj_cir (np, op) < 0)
	    return (-1);
	*tp = mjd;
	*alp = op->s_alt;
	return (0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: riset_cir.c,v $ $Date: 2009/04/06 00:17:41 $ $Revision: 1.13 $ $Name:  $"};

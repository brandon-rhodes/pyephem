/* misc handy functions.
 * every system has such, no?
 *  4/20/98 now_lst() always just returns apparent time
 */

#include <stdio.h>
#include <math.h>

#if defined(__STDC__)
#include <stdlib.h>
#include <string.h>
#else
extern double atof();
#endif

#include "P_.h"
#include "astro.h"
#include "circum.h"

/* zero from loc for len bytes */
void
zero_mem (loc, len)
void *loc;
unsigned len;
{
	(void) memset (loc, 0, len);
}

/* given min and max and an approximate number of divisions desired,
 * fill in ticks[] with nicely spaced values and return how many.
 * N.B. return value, and hence number of entries to ticks[], might be as
 *   much as 2 more than numdiv.
 */
int
tickmarks (min, max, numdiv, ticks)
double min, max;
int numdiv;
double ticks[];
{
        static int factor[] = { 1, 2, 5 };
        double minscale;
        double delta;
	double lo;
        double v;
        int n;

        minscale = fabs(max - min);
        delta = minscale/numdiv;
        for (n=0; n < sizeof(factor)/sizeof(factor[0]); n++) {
	    double scale;
	    double x = delta/factor[n];
            if ((scale = (pow(10.0, ceil(log10(x)))*factor[n])) < minscale)
		minscale = scale;
	}
        delta = minscale;

        lo = floor(min/delta);
        for (n = 0; (v = delta*(lo+n)) < max+delta; )
	    ticks[n++] = v;

	return (n);
}

/* given an Obj *, return its type as a descriptive string.
 * if it's of type fixed then return its class description.
 * N.B. we return the address of static storage -- do not free or change.
 */
char *
obj_description (op)
Obj *op;
{
	static struct {
	    char class;
	    char *desc;
	} fixed_class_map[] = {
	    {'A', "Cluster of Galaxies"},
	    {'B', "Binary Star"},
	    {'C', "Globular Cluster"},
	    {'D', "Double Star"},
	    {'F', "Diffuse Nebula"},
	    {'G', "Spiral Galaxy"},
	    {'H', "Spherical Galaxy"},
	    {'J', "Radio"},
	    {'K', "Dark Nebula"},
	    {'L', "Pulsar"},
	    {'M', "Multiple Star"},
	    {'N', "Bright Nebula"},
	    {'O', "Open Cluster"},
	    {'P', "Planetary Nebula"},
	    {'Q', "Quasar"},
	    {'R', "Supernova Remnant"},
	    {'S', "Star"},
	    {'T', "Star-like Object"},
	    {'U', "Cluster, with nebulosity"},
	    {'V', "Variable Star"},
	};
#define	NFCM	(sizeof(fixed_class_map)/sizeof(fixed_class_map[0]))

	switch (op->o_type) {
	case FIXED:
	    if (op->f_class) {
		int i;
		for (i = 0; i < NFCM; i++)
		    if (fixed_class_map[i].class == op->f_class)
			return (fixed_class_map[i].desc);
	    }
	    return (op->o_type == FIXED ? "Fixed" : "Field Star");
	case PARABOLIC:
	    return ("Solar - Parabolic");
	case HYPERBOLIC:
	    return ("Solar - Hyperbolic");
	case ELLIPTICAL:
	    return ("Solar - Elliptical");
	case PLANET:
	    return ("Planet");
	case EARTHSAT:
	    return ("Earth Sat");
	default:
	    printf ("obj_description: unknown type: 0x%x\n", op->o_type);
	    exit (1);
	    return (NULL);	/* for lint */
	}
}

/* given a Now *, find the local apparent sidereal time, in hours.
 */
void
now_lst (np, lstp)
Now *np;
double *lstp;
{
	static double last_mjd = -23243, last_lng = 121212, last_lst;
	double eps, lst, deps, dpsi;

	if (last_mjd == mjd && last_lng == lng) {
	    *lstp = last_lst;
	    return;
	}

	utc_gst (mjd_day(mjd), mjd_hr(mjd), &lst);
	lst += radhr(lng);

	obliquity(mjd, &eps);
	nutation(mjd, &deps, &dpsi);
	lst += radhr(dpsi*cos(eps+deps));

	range (&lst, 24.0);

	last_mjd = mjd;
	last_lng = lng;
	*lstp = last_lst = lst;
}

/* convert ra to ha, in range -PI .. PI.
 * need dec too if not already apparent.
 */
void
radec2ha (np, ra, dec, hap)
Now *np;
double ra, dec;
double *hap;
{
	double ha, lst;

	if (epoch != EOD)
	    as_ap (np, epoch, &ra, &dec);
	now_lst (np, &lst);
	ha = hrrad(lst) - ra;
	if (ha < -PI) ha += 2*PI;
	if (ha >  PI) ha -= 2*PI;
	*hap = ha;
}

/* given a circle and a line segment, find a segment of the line inside the 
 *   circle.
 * return 0 and the segment end points if one exists, else -1.
 * We use a parametric representation of the line:
 *   x = x1 + (x2-x1)*t and y = y1 + (y2-y1)*t, 0 < t < 1
 * and a centered representation of the circle:
 *   (x - xc)**2 + (y - yc)**2 = r**2
 * and solve for the t's that work, checking for usual conditions.
 */
int
lc (cx, cy, cw, x1, y1, x2, y2, sx1, sy1, sx2, sy2)
int cx, cy, cw;			/* circle bounding box corner and width */
int x1, y1, x2, y2;		/* line segment endpoints */
int *sx1, *sy1, *sx2, *sy2;	/* segment inside the circle */
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	int r = cw/2;
	int xc = cx + r;
	int yc = cy + r;
	int A = x1 - xc;
	int B = y1 - yc;
	double a = dx*dx + dy*dy;	/* O(2 * 2**16 * 2**16) */
	double b = 2*(dx*A + dy*B);	/* O(4 * 2**16 * 2**16) */
	double c = A*A + B*B - r*r;	/* O(2 * 2**16 * 2**16) */
	double d = b*b - 4*a*c;		/* O(2**32 * 2**32) */
	double sqrtd;
	double t1, t2;

	if (d <= 0)
	    return (-1);	/* containing line is purely outside circle */

	sqrtd = sqrt(d);
	t1 = (-b - sqrtd)/(2.0*a);
	t2 = (-b + sqrtd)/(2.0*a);

	if (t1 >= 1.0 || t2 <= 0.0)
	    return (-1);	/* segment is purely outside circle */

	/* we know now that some part of the segment is inside,
	 * ie, t1 < 1 && t2 > 0
	 */

	if (t1 <= 0.0) {
	    /* (x1,y1) is inside circle */
	    *sx1 = x1;
	    *sy1 = y1;
	} else {
	    *sx1 = (int)(x1 + dx*t1);
	    *sy1 = (int)(y1 + dy*t1);
	}

	if (t2 >= 1.0) {
	    /* (x2,y2) is inside circle */
	    *sx2 = x2;
	    *sy2 = y2;
	} else {
	    *sx2 = (int)(x1 + dx*t2);
	    *sy2 = (int)(y1 + dy*t2);
	}

	return (0);
}

/* compute visual magnitude using the H/G parameters used in the Astro Almanac.
 * these are commonly used for asteroids.
 */
void
hg_mag (h, g, rp, rho, rsn, mp)
double h, g;
double rp;	/* sun-obj dist, AU */
double rho;	/* earth-obj dist, AU */
double rsn;	/* sun-earth dist, AU */
double *mp;
{
	double psi_t, Psi_1, Psi_2, beta;
	double c;
	double tb2;

	c = (rp*rp + rho*rho - rsn*rsn)/(2*rp*rho);
	if (c <= -1)
	    beta = PI;
	else if (c >= 1)
	    beta = 0;
	else
	    beta = acos(c);;
	tb2 = tan(beta/2.0);
	/* psi_t = exp(log(tan(beta/2.0))*0.63); */
	psi_t = pow (tb2, 0.63);
	Psi_1 = exp(-3.33*psi_t);
	/* psi_t = exp(log(tan(beta/2.0))*1.22); */
	psi_t = pow (tb2, 1.22);
	Psi_2 = exp(-1.87*psi_t);
	*mp = h + 5.0*log10(rp*rho) - 2.5*log10((1-g)*Psi_1 + g*Psi_2);
}

/* given faintest desired mag, mag step magstp, image scale and object
 * magnitude and size, return diameter to draw object, in pixels, or 0 if
 * dimmer than fmag.
 */
int
magdiam (fmag, magstp, scale, mag, size)
int fmag;	/* faintest mag */
int magstp;	/* mag range per dot size */
double scale;	/* rads per pixel */
double mag;	/* magnitude */
double size;	/* rads, or 0 */
{
	int diam, sized;
	
	if (mag > fmag)
	    return (0);
	diam = (int)((fmag - mag)/magstp + 1);
	sized = (int)(size/scale + 0.5);
	if (sized > diam)
	    diam = sized;

	return (diam);
}

/* computer visual magnitude using the g/k parameters commonly used for comets.
 */
void
gk_mag (g, k, rp, rho, mp)
double g, k;
double rp;	/* sun-obj dist, AU */
double rho;	/* earth-obj dist, AU */
double *mp;
{
	*mp = g + 5.0*log10(rho) + 2.5*k*log10(rp);
}

/* given a string convert to floating point and return it as a double.
 * this is to isolate possible unportabilities associated with declaring atof().
 * it's worth it because atof() is often some 50% faster than sscanf ("%lf");
 */
double
atod (buf)
char *buf;
{
	return (atof (buf));
}

/* solve a spherical triangle:
 *           A
 *          /  \
 *         /    \
 *      c /      \ b
 *       /        \
 *      /          \
 *    B ____________ C
 *           a
 *
 * given A, b, c find B and a in range 0..B..2PI and 0..a..PI, respectively..
 * cap and Bp may be NULL if not interested in either one.
 * N.B. we pass in cos(c) and sin(c) because in many problems one of the sides
 *   remains constant for many values of A and b.
 */
void
solve_sphere (A, b, cc, sc, cap, Bp)
double A, b;
double cc, sc;
double *cap, *Bp;
{
	double cb = cos(b), sb = sin(b);
	double cA = cos(A);
	double ca;
	double B;

	ca = cb*cc + sb*sc*cA;
	if (ca >  1.0) ca =  1.0;
	if (ca < -1.0) ca = -1.0;
	if (cap)
	    *cap = ca;

	if (!Bp)
	    return;

	if (cc > .99999) {
	    /* as c approaches 0, B approaches pi - A */
	    B = PI - A;
	} else if (cc < -.99999) {
	    /* as c approaches PI, B approaches A */
	    B = A;
	} else {
	    /* compute cB and sB and remove common factor of sa from quotient.
	     * be careful where B causes atan to blow.
	     */
	    double sA = sin(A);
	    double x, y;

	    y = sA*sb*sc;
	    x = cb - ca*cc;
	
	    if (fabs(x) < 1e-5)
		B = y < 0 ? 3*PI/2 : PI/2;
	    else
		B = atan2 (y, x);
	}

	*Bp = B;
	range (Bp, 2*PI);
}

/* #define WANT_MATHERR if your system supports it. it gives SGI fits.
 */
#undef WANT_MATHERR
#if defined(WANT_MATHERR)
/* attempt to do *something* reasonable when a math function blows.
 */
matherr (xp)
struct exception *xp;
{
	static char *names[8] = {
	    "acos", "asin", "atan2", "pow",
	    "exp", "log", "log10", "sqrt"
	};
	int i;

	/* catch-all */
	xp->retval = 0.0;

	for (i = 0; i < sizeof(names)/sizeof(names[0]); i++)
	    if (strcmp (xp->name, names[i]) == 0)
		switch (i) {
		case 0:	/* acos */
		    xp->retval = xp->arg1 >= 1.0 ? 0.0 : -PI;
		    break;
		case 1: /* asin */
		    xp->retval = xp->arg1 >= 1.0 ? PI/2 : -PI/2;
		    break;
		case 2: /* atan2 */
		    if (xp->arg1 == 0.0)
			xp->retval = xp->arg2 < 0.0 ? PI : 0.0;
		    else if (xp->arg2 == 0.0)
			xp->retval = xp->arg1 < 0.0 ? -PI/2 : PI/2;
		    else
			xp->retval = 0.0;
		    break;
		case 3: /* pow */
		/* FALLTHRU */
		case 4: /* exp */
		    xp->retval = xp->o_type == OVERFLOW ? 1e308 : 0.0;
		    break;
		case 5: /* log */
		/* FALLTHRU */
		case 6: /* log10 */
		    xp->retval = xp->arg1 <= 0.0 ? -1e308 : 0;
		    break;
		case 7: /* sqrt */
		    xp->retval = 0.0;
		    break;
		}

        return (1);     /* suppress default error handling */
}
#endif

/* given the difference in two RA's, in rads, return their difference,
 *   accounting for wrap at 2*PI. caller need *not* first force it into the
 *   range 0..2*PI.
 */
double
delra (dra)
double dra;
{
	double fdra = fmod(fabs(dra), 2*PI);

	if (fdra > PI)
	    fdra = 2*PI - fdra;
	return (fdra);
}

/* return 1 if object is considered to be "deep sky", else 0.
 * The only things deep-sky are fixed objects other than stars.
 */
int
is_deepsky (op)
Obj *op;
{
	int deepsky = 0;

	if (is_type(op, FIXEDM)) {
	    switch (op->f_class) {
	    case 'T':
	    case 'B':
	    case 'D':
	    case 'M':
	    case 'S':
	    case 'V':
		break;
	    default:
		deepsky = 1;
		break;
	    }
	}

	return (deepsky);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: misc.c,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $"};

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "P_.h"
#include "astro.h"
#include "preferences.h"

/* sprint the variable a in sexagesimal format into out[].
 * w is the number of spaces for the whole part.
 * fracbase is the number of pieces a whole is to broken into; valid options:
 *	360000:	<w>:mm:ss.ss
 *	36000:	<w>:mm:ss.s
 *	3600:	<w>:mm:ss
 *	600:	<w>:mm.m
 *	60:	<w>:mm
 */
void
fs_sexa (out, a, w, fracbase)
char *out;
double a;
int w;
int fracbase;
{
	unsigned long n;
	int d;
	int f;
	int m;
	int s;
	int isneg;

	/* save whether it's negative but do all the rest with a positive */
	isneg = (a < 0);
	if (isneg)
	    a = -a;

	/* convert to an integral number of whole portions */
	n = (unsigned long)(a * fracbase + 0.5);
	d = n/fracbase;
	f = n%fracbase;

	/* form the whole part; "negative 0" is a special case */
	if (isneg && d == 0)
	    (void) sprintf (out, "%*s-0", w-2, "");
	else
	    (void) sprintf (out, "%*d", w, isneg ? -d : d);
	out += w;

	/* do the rest */
	switch (fracbase) {
	case 60:	/* dd:mm */
	    m = f/(fracbase/60);
	    (void) sprintf (out, ":%02d", m);
	    break;
	case 600:	/* dd:mm.m */
	    (void) sprintf (out, ":%02d.%1d", f/10, f%10);
	    break;
	case 3600:	/* dd:mm:ss */
	    m = f/(fracbase/60);
	    s = f%(fracbase/60);
	    (void) sprintf (out, ":%02d:%02d", m, s);
	    break;
	case 36000:	/* dd:mm:ss.s*/
	    m = f/(fracbase/60);
	    s = f%(fracbase/60);
	    (void) sprintf (out, ":%02d:%02d.%1d", m, s/10, s%10);
	    break;
	case 360000:	/* dd:mm:ss.ss */
	    m = f/(fracbase/60);
	    s = f%(fracbase/60);
	    (void) sprintf (out, ":%02d:%02d.%02d", m, s/100, s%100);
	    break;
	default:
	    printf ("fs_sexa: unknown fracbase: %d\n", fracbase);
	    exit(1);
	}
}

/* put the given modified Julian date, jd, in out[] according to preference
 * format.
 */
void
fs_date (out, jd)
char out[];
double jd;
{
	int p = pref_get (PREF_DATE_FORMAT);
	int m, y;
	double d;

	mjd_cal (jd, &m, &d, &y);
	/* beware of %g rounding day up */
	if ((d < 1.0 && d - floor(d) >= .9999995)
				    || (d < 10.0 && d - floor(d) >= .999995)
				    || (d >= 10.0 && d - floor(d) >= .99995))
	    mjd_cal (mjd_day(jd+0.5), &m, &d, &y);

	switch (p) {
	case PREF_YMD:
	    (void) sprintf (out, "%4d/%02d/%02.6g", y, m, d);
	    break;
	case PREF_DMY:
	    (void) sprintf (out, "%2.6g/%02d/%-4d", d, m, y);
	    break;
	case PREF_MDY:
	    (void) sprintf (out, "%2d/%02.6g/%-4d", m, d, y);
	    break;
	default:
	    printf ("fs_date: bad date pref: %d\n", p);
	    exit (1);
	}
}

/* scan a sexagesimal string and update a double. the string is of the form
 *   H:M:S. a negative value may be indicated by a '-' char before any
 *   component. All components may be integral or real. In addition to ':' the
 *    separator may also be '/' or ';' or ',' or '-'.
 * any components not specified in bp[] are copied from old's.
 *   eg:  ::10	only changes S
 *        10    only changes H
 *        10:0  changes H and M
 */
void
f_scansex (o, bp, np)
double o;	/* old value */
char bp[];	/* input string */
double *np;	/* new value */
{
	double oh, om, os;
	double nh, nm, ns;
	int nneg;
	double tmp;
	char c;

	/* fast macro to scan each segment of bp */
#define	SCANSEX(new,old)						\
	if (*bp == '-') {nneg = 1; bp++;}				\
	if (sscanf (bp, "%lf", &new) != 1) new = old;			\
	while ((c=(*bp)) != '\0') {					\
	    bp++;							\
	    if (c==':' || c=='/' || c==';' || c==',' || c=='-')		\
		break;							\
	}

	/* get h:m:s components of o in case bp[] defers.
	 * and constrain to positive for now.
	 */
	if (o < 0.0)
	    o = -o;
        oh = floor(o);
	tmp = (o - oh)*60.0;
	om = floor(tmp);
	os = (tmp - om)*60.0;
	
	/* round to small portion of a second to reduce differencing errors. */
	if (os > 59.99999) {
	    os = 0.0;
	    if ((om += 1.0) >= 60.0) {
		om = 0.0;
		oh += 1.0;
	    }
	}

	/* scan each component of the buffer */
	while (*bp == ' ') bp++;
	nneg = 0;
	SCANSEX (nh, oh)
	SCANSEX (nm, om)
	SCANSEX (ns, os)

	/* back to hours */
	tmp = ns/3600.0 + nm/60.0 + nh;
	if (nneg)
	    tmp = -tmp;

	*np = tmp;
}

/* crack a floating date string, bp, of the form X/Y/Z determined by the
 *   PREF_DATE_FORMAT preference into its components. allow the day to be a
 *   floating point number,
 * a lone component is always a year if it contains a decimal point or pref
 *   is MDY or DMY and it is not a reasonable month or day value, respectively.
 * leave any unspecified component unchanged.
 * actually, the slashes may be anything but digits or a decimal point.
 */
void
f_sscandate (bp, pref, m, d, y)
char *bp;
int pref;       /* one of PREF_X for PREF_DATE_FORMAT */
int *m, *y;
double *d;
{
        double comp[3]; /* the three components */
        int set[3];     /* set[i] is 1 if component i is present */
	int in;		/* scan state: 1 while we are in a component */
	int ncomp;	/* incremented as each component is discovered */
	int ndp;	/* number of decimal points in last component */
        char *bp0 = NULL, c;

	set[0] = set[1] = set[2] = 0;
	ncomp = 0;
	in = 0;
	ndp = 0;

	/* scan the input string.
	 * '\0' counts as a component terminator too.
	 */
	do {
	    /* here, *bp is the next character to be investigated */
	    c = *bp;
	    if (c == '.' || isdigit(c) || (c == '-' && !in)) {
		/* found what passes for a floating point number */
		if (in == 0) {
		    /* save the beginning of it in bp0 and init ndp */
		    bp0 = bp;
		    in = 1;
		    ndp = 0;
		}
		if (c == '.')
		    ndp++;
	    } else if (c != ' ') {
		/* not in a number now ... */
		if (in) {
		    /* ... but we *were* in a number, so it counts */
		    if (ncomp >= 3)
			return;	/* too many components.. just bail */
		    comp[ncomp] = atod (bp0);
		    set[ncomp] = 1;
		    in = 0;
		}

		/* regardless, a non-blank non-float means another component */
		ncomp++;
	    }
	    bp++;
	} while (c);

	/* it's a decimal year if there is exactly one component and
	 *   it contains a decimal point
	 *   or we are in MDY format and it's not in the range 1..12
	 *   or we are in DMY format and it's not int the rangle 1..31
	 */
	if (ncomp == 1 &&
		(ndp > 0
		 || (pref == PREF_MDY && !(comp[0] >= 1 && comp[0] <= 12))
		 || (pref == PREF_DMY && !(comp[0] >= 1 && comp[0] <= 31)))) {
	    double Mjd;
	    year_mjd (comp[0], &Mjd);
	    mjd_cal (Mjd, m, d, y);
	    return;
	}

        switch (pref) {
        case PREF_MDY:
            if (set[0]) *m = (int)comp[0];
            if (set[1]) *d = comp[1];
            if (set[2]) *y = (int)comp[2];
            break;
        case PREF_YMD:
            if (set[0]) *y = (int)comp[0];
            if (set[1]) *m = (int)comp[1];
            if (set[2]) *d = comp[2];
            break;
        case PREF_DMY:
            if (set[0]) *d = comp[0];
            if (set[1]) *m = (int)comp[1];
            if (set[2]) *y = (int)comp[2];
            break;
	}
}

/* given a string of the form xx:mm[:ss] or xx:mm.dd, convert it to a double at
 * *dp. actually, ':' may also be ';', '/' or ',' too. all components may be 
 * floats.
 * return -1 if trouble, else 0
 */
int
scansex (str, dp)
char *str;
double *dp;
{
	double x, m = 0.0, s = 0.0;
	int isneg;
	int nf;

	while (isspace(*str))
	    str++;
	if (*str == '-') {
	    isneg = 1;
	    str++;
	} else
	    isneg = 0;

	nf = sscanf (str, "%lf%*[:;/,]%lf%*[:;/,]%lf", &x, &m, &s);
	if (nf < 1)
	    return (-1);
	*dp = x + m/60.0 + s/3600.0;
	if (isneg)
	    *dp = - *dp;
	return (0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: formats.c,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $"};

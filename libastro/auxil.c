/* aux functions so programs besides XEphem can use this library.
 */

#include <stdio.h>

#if defined (__STDC__)
#include <stdlib.h>
#endif

#include "P_.h"
#include "astro.h"
#include "circum.h"
#include "preferences.h"

static int prefs[NPREFS] = {
    PREF_TOPO, PREF_METRIC, PREF_MDY, PREF_UTCTZ, PREF_HIPREC, PREF_NOMSGBELL,
    PREF_PREFILL, PREF_TIPSON, PREF_CONFIRMON, PREF_WEEKSTART
};

/* called anytime we want to know a preference.
 */
int
pref_get(pref)
Preferences pref;
{
	return (prefs[pref]);
}

/* call to force a certain preference, return the old setting.
 */
int
pref_set (pref, new)
Preferences pref;
int new;
{
	int prior = pref_get(pref);
	prefs[pref] = new;
	return (prior);
}

/* given an mjd, return it modified for terrestial dynamical time */
double
mm_mjed (np)
Now *np;
{
	return (mjd + deltat(mjd)/86400.0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: auxil.c,v $ $Date: 2003/03/04 05:44:49 $ $Revision: 1.1 $ $Name:  $"};

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "astro.h"
#include "preferences.h"

extern void txt_report (Now *np, Obj *op, int topo);

static void crackArgs (int ac, char *av[]);
static void usage (void);
static void initNow (Now *np);
static int findResFile (char path[]);
static int findRes (FILE *fp, char *name, char valu[]);
static void processResFile (Now *np, char path[]);
static double crackSexa (char str[]);
static void process1edb (Now *np, char edbline[]);

static char *myname;
static char *myresfile;
static char *myjd;
static int wanttopo;
static int verbose;

int
main (int ac, char *av[])
{
	char buf[1024];
	Now now;

	/* save our name */
	myname = av[0];

	/* crack args */
	crackArgs (ac, av);

	/* init circumstances */
	initNow(&now);

	/* process each input line */
	while (fgets (buf, sizeof(buf), stdin))
	    process1edb (&now, buf);

	/* finished */
	return (0);
}

/* go through argv list processing arguments.
 * exit(1) with usage message if find something unexpected.
 */
static void
crackArgs (int ac, char *av[])
{
	while ((--ac > 0) && ((*++av)[0] == '-')) {
	    char *s;
	    for (s = av[0]+1; *s != '\0'; s++) {
		switch (*s) {
		case 'j':
		    if (ac < 2)
			usage();
		    myjd = *++av;
		    ac--;
		    break;
		case 'r':
		    if (ac < 2)
			usage();
		    myresfile = *++av;
		    ac--;
		    break;
		case 't':
		    wanttopo++;
		    break;
		case 'v':
		    verbose++;
		    break;
		default:
		    usage();
		}
	    }
	}

	/* no strays */
	if (ac > 0)
	    usage();
}

/* print program args and exit(1) */
static void
usage()
{
	fprintf (stderr, "Usage: %s [options]\n", myname);
	fprintf (stderr, "Purpose: compute ephemerides from .edb on stdin\n");
	fprintf (stderr, "$Revision: 1.6 $\n");
	fprintf (stderr, "Options:\n");
	fprintf (stderr, "  -j j : set time to JD j, else use current host\n");
	fprintf (stderr, "  -r f : use XEphem resource file f, else round up the usual suspects\n");
	fprintf (stderr, "  -t   : generate topocentric, else geocentric\n");
	fprintf (stderr, "  -v   : verbose\n");

	exit (1);
}

/* initialize np.
 * exit(1) if trouble
 */
static void
initNow (Now *np)
{
	char buf[1024];

	/* fresh start */
	memset (np, 0, sizeof(*np));

	/* get position info from an XEphem resource file */
	if (myresfile)
	    processResFile (np, myresfile);
	else if (!findResFile(buf))
	    processResFile (np, buf);
	else {
	    fprintf (stderr, "Can not find an XEphem resource file\n");
	    exit(1);
	}

	/* get time from arg else OS */
	if (myjd)
	    mjd = atof (myjd) - MJD0;
	else {
	    /* t is seconds since 00:00:00 1/1/1970 UTC on UNIX systems;
	     * mjd was 25567.5 then.
	     */
	    time_t t;
	    time (&t);
	    mjd = 25567.5 + t/3600.0/24.0;
	}

	/* set desired vantage */
	pref_set (PREF_EQUATORIAL, wanttopo ? PREF_TOPO : PREF_GEO);

	if (verbose) {
	    double y;
	    mjd_year (mjd, &y);
	    fprintf (stderr, "  mjd %13.5f AD\n", y);
	    fprintf (stderr, "  lat %13.5f °\n", raddeg(lat));
	    fprintf (stderr, "  lng %13.5f °\n", raddeg(lng));
	    fprintf (stderr, " temp %13.5f °C\n", temp);
	    fprintf (stderr, "press %13.5f hPa\n", pressure);
	    fprintf (stderr, " elev %13.5f m\n", elev*ERAD);
	    fprintf (stderr, "   tz %13.5f hrs behind UTC\n", tz);
	    if (epoch == EOD)
		fprintf (stderr, "epoch %13s\n", "EOD");
	    else {
		mjd_year (epoch, &y);
		fprintf (stderr, "epoch %13.5f AD\n", y);
	    }
	    fprintf (stderr, "  equ %13s\n", wanttopo ? "Topo" : "Geo");
	}
}

/* look for an XEphem resource file.
 * do this manually so we aren't a full X program just for this.
 * if find fill path and return 0, else return -1.
 */
static int
findResFile (char path[])
{
	char *home = getenv ("HOME");
	char buf[1024];
	FILE *fp;

	/* try default */
	sprintf (path, "%s/XEphem/XEphem", home);
	if ((fp = fopen (path, "r")) != NULL) {
	    fclose (fp);
	    return (0);
	}

	/* try specified local */
	sprintf (buf, "%s/.xephemrc", home);
	fp = fopen (buf, "r");
	if (fp) {
	    int found = findRes (fp, "XEphem.PrivateDir", path);
	    fclose (fp);
	    if (found == 0)
		strcat (path, "/XEphem");
	    return (found);
	}

	/* try some common system places */
	sprintf (path, "/usr/X11R6/lib/X11/app-defaults/XEphem");
	if ((fp = fopen (path, "r")) != NULL) {
	    fclose (fp);
	    return (0);
	}
	sprintf (path, "/usr/lib/X11/app-defaults/XEphem");
	if ((fp = fopen (path, "r")) != NULL) {
	    fclose (fp);
	    return (0);
	}

	/* sorry */
	return (-1);
}

/* scan entire file fp for line of the form "<name>: <valu>".
 * if find, fill valu[] sans leading white space and return 0 else -1.
 */
static int
findRes (FILE *fp, char *name, char valu[])
{
	char buf[1024];

	rewind (fp);
	while (fgets (buf, sizeof(buf), fp)) {
	    char *colon = strchr(buf, ':');
	    if (colon) {
		*colon = '\0';
		if (!strcmp (name, buf)) {
		    while (*++colon == ' ')
			continue;
		    colon[strlen(colon)-1] = '\0';	/* no \n */
		    strcpy (valu, colon);
		    return (0);
		}
	    }
	}

	return (-1);
}

/* read the given XEphem resource file and fill in np.
 * exit(1) if insufficient enties.
 */
static void
processResFile (Now *np, char path[])
{
	char *resn, resv[1024];
	FILE *fp;

	/* open resource file */

	if (verbose)
	    fprintf (stderr, "Using resource file %s\n", path);
	fp = fopen (path, "r");
	if (!fp) {
	    fprintf (stderr, "%s: %s\n", path, strerror(errno));
	    exit(1);
	}

	/* extract required fields */

	resn = "XEphem.Lat";
	if (findRes(fp, resn, resv) < 0) {
	    fprintf (stderr, "%s: need %s\n", path, resn);
	    exit(1);
	}
	lat = degrad(crackSexa(resv));

	resn = "XEphem.Long";
	if (findRes(fp, resn, resv) < 0) {
	    fprintf (stderr, "%s: need %s\n", path, resn);
	    exit(1);
	}
	lng = -degrad(crackSexa(resv));	/* want +E */

	resn = "XEphem.Elevation";
	if (findRes(fp, resn, resv) < 0) {
	    fprintf (stderr, "%s: need %s\n", path, resn);
	    exit(1);
	}
	elev = atof(resv)/ERAD;		/* want earth radii */

	/* extract optional fields */

	resn = "XEphem.Pressure";
	if (findRes(fp, resn, resv) < 0)
	    strcpy (resv, "1010");
	pressure = atof(resv);

	resn = "XEphem.Temp";
	if (findRes(fp, resn, resv) < 0)
	    strcpy (resv, "10");
	temp = atof(resv);

	resn = "XEphem.TZone";
	if (findRes(fp, resn, resv) < 0)
	    strcpy (resv, "0");
	tz = crackSexa(resv);

	resn = "XEphem.Equinox";
	if (findRes(fp, resn, resv) < 0)
	    strcpy (resv, "2000");
	if (!strcmp (resv, "Of Date"))
	    epoch = EOD;
	else
	    year_mjd (atof(resv), &epoch);

	fclose (fp);
}

/* crack str of the form h:m:s to a double.
 * no error checking.
 */
static double
crackSexa (char str[])
{
	double h = 0, m = 0, s = 0;
	char *neg = strchr (str, '-');

	if (neg)
	    *neg = ' ';
	sscanf (str, "%lf:%lf:%lf", &h, &m, &s);
	h += m/60 + s/3600;
	if (neg)
	    h = -h;
	return (h);
}

/* process 1 edb line according to np.
 * skip if trouble.
 */
static void
process1edb (Now *np, char edbline[])
{
	Obj o, *op = &o;
	char buf[1024];

	if (verbose)
	    fprintf (stderr, "Processing: %s\n", edbline);

	/* convert line to Obj */
	if (db_crack_line (edbline, op, NULL, 0, buf) < 0) {
	    fprintf (stderr, "%s: %s\n", edbline, buf);
	    return;
	}

	/* compute circumstances @ np */
	if (obj_cir (np, op) < 0) {
	    fprintf (stderr, "%s: can not compute ephemeris\n", op->o_name);
	    return;
	}

	/* print */
	txt_report (np, op, wanttopo);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: xedb.c,v $ $Date: 2003/11/05 01:47:59 $ $Revision: 1.6 $ $Name:  $"};

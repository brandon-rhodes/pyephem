/* crack natural satellite files from BDL */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "astro.h"
#include "bdl.h"

typedef enum {I, F, NL} ScanType;
#define	SCANFLD(f,w,vp)	if(readField(fp,f,w,(void *)vp,ynot)<0) return (-1)

static int readField (FILE *fp, ScanType f, int w, void *ptr, char ynot[]);
static int readRec (FILE *fp, double *t0, double cmx[], double cfx[],
    double cmy[], double cfy[], double cmz[], double cfz[], char ynot[]);

/* given a sequencial text file in BDL natural satellite ephemeris format and a
 * JD, find the x/y/z positions of each satellite. store in the given arrays,
 * assumed to have one entry per moon. values are planetocentric, +x east, +y
 * north, +z away from earth, all in au. corrected for light time.
 * return the number of satellites or -1 and reason in ymot[].
 * files obtained from ftp://ftp.bdl.fr/pub/misc/satxyz.
 */
int
read_bdl (FILE *fp, double jd, double *xp, double *yp, double *zp, char ynot[])
{
	int npla;
	int nsat;
	int idn[8];
	double freq[8];
	double delt[8];
	double djj;
	double cmx[6], cfx[4], cmy[6], cfy[4], cmz[6], cfz[4];
	int ienrf;
	int jan;
	int reclen;
	long os0;
	double t0;
	int i;

	/* read header line */
	SCANFLD (I, 2, &npla);
	SCANFLD (I, 2, &nsat);
	for (i = 0; i < nsat; i++)
	    SCANFLD (I, 5, &idn[i]);
	for (i = 0; i < nsat; i++)
	    SCANFLD (F, 8, &freq[i]);
	for (i = 0; i < nsat; i++)
	    SCANFLD (F, 5, &delt[i]);
	SCANFLD (I, 5, &ienrf);
	SCANFLD (F, 15, &djj);
	SCANFLD (I, 5, &jan);
	SCANFLD (NL, 0, NULL);

	/* record position of first record */
	os0 = ftell (fp);

	/* read first record to get length */
	reclen = readRec (fp, &t0, cmx, cfx, cmy, cfy, cmz, cfz, ynot);
	if (reclen < 0)
	    return (-1);

	/* compute location of each satellite */
	for (i = 0; i < nsat; i++) {
	    int id = (int)floor((jd-djj)/delt[i]) + idn[i] - 2;
	    long os = os0 + id*reclen;
	    double t1, anu, tau, tau2, at;
	    double tbx, tby, tbz;

	    if (fseek (fp, os, SEEK_SET) < 0) {
		sprintf (ynot, "Seek error to %ld for rec %d", os, id);
		return (-1);
	    }

	    if (readRec (fp, &t0, cmx, cfx, cmy, cfy, cmz, cfz, ynot) < 0)
		return (-1);

	    t1 = floor(t0) + 0.5;
	    anu = freq[i];
	    tau = jd - t1;
	    tau2 = tau * tau;
	    at = tau*anu;

	    tbx = cmx[0]+cmx[1]*tau+cmx[2]*sin(at+cfx[0])
			    +cmx[3]*tau*sin(at+cfx[1])
			    +cmx[4]*tau2*sin(at+cfx[2])
			    +cmx[5]*sin(2*at+cfx[3]);
	    tby = cmy[0]+cmy[1]*tau+cmy[2]*sin(at+cfy[0])
			    +cmy[3]*tau*sin(at+cfy[1])
			    +cmy[4]*tau2*sin(at+cfy[2])
			    +cmy[5]*sin(2*at+cfy[3]);
	    tbz = cmz[0]+cmz[1]*tau+cmz[2]*sin(at+cfz[0])
			    +cmz[3]*tau*sin(at+cfz[1])
			    +cmz[4]*tau2*sin(at+cfz[2])
			    +cmz[5]*sin(2*at+cfz[3]);

	    xp[i] = tbx*1000./149597870.;
	    yp[i] = tby*1000./149597870.;
	    zp[i] = tbz*1000./149597870.;
	}

	return (nsat);
}

/* read one field.
 * return 0 if ok else -1
 * N.B. this is enforce width, without skipping leading blanks.
 */
static int
readField (FILE *fp, ScanType f, int width, void *ptr, char ynot[])
{
	char buf[128];
	char *bp;

	if (width > sizeof(buf)-1) {
	    sprintf (ynot, "BDL Field width %d > %d", width, (int)sizeof(buf));
	    return (-1);
	}
	if (width != (int)fread (buf, 1, width, fp)) {
	    if (ferror(fp)) strcpy (ynot, "BDL IO error");
	    else if (feof(fp)) strcpy (ynot, "BDL unexpected EOF");
	    else strcpy (ynot, "BDL short file");
	    return (-1);
	}

	buf[width] = '\0';
	switch (f) {
	case I:
	    *(int *)ptr = atoi (buf);
	    break;
	case F:
	    bp = strchr (buf, 'D');
	    if (bp)
		*bp = 'e';
	    *(double *)ptr = atof (buf);
	    break;
	case NL:
	    fgets (buf, sizeof(buf), fp);
	    break;
	default:
	    sprintf (ynot, "Bug! format = %d", f);
	    return (-1);
	}
	return (0);
}

/* read one satellite record.
 * return number of chars read else -1.
 */
static int
readRec (FILE *fp, double *t0, double cmx[], double cfx[], double cmy[],
double cfy[], double cmz[], double cfz[], char ynot[])
{

	long pos0, pos1;
	int isat, idx;
	int ldat1, ldat2;
	int i;

	pos0 = ftell (fp);

	SCANFLD (I, 1, &isat);
	SCANFLD (I, 5, &idx);
	SCANFLD (I, 8, &ldat1);
	SCANFLD (I, 8, &ldat2);
	SCANFLD (F, 9, t0);
	for (i = 0; i < 6; i++)
	    SCANFLD (F, 17, &cmx[i]);
	for (i = 0; i < 4; i++)
	    SCANFLD (F, 17, &cfx[i]);
	for (i = 0; i < 6; i++)
	    SCANFLD (F, 17, &cmy[i]);
	for (i = 0; i < 4; i++)
	    SCANFLD (F, 17, &cfy[i]);
	for (i = 0; i < 6; i++)
	    SCANFLD (F, 17, &cmz[i]);
	for (i = 0; i < 4; i++)
	    SCANFLD (F, 17, &cfz[i]);
	SCANFLD (NL, 0, NULL);

	pos1 = ftell (fp);

	return (pos1 - pos0);
}


#ifdef TEST_IT
/* stand-alone test program.
 * for example, compare
 *   a.out jupiter.9910 2451910.50000
 * with
 *   satxyz2
 *     jup.dir.9910
 *     2001 1 1 0 0 0
 *     1 0 0 0
 *     1
 */
int
main (int ac, char *av[])
{
	double x[10], y[10], z[10];
	char ynot[1024];
	double jd;
	char *fn;
	FILE *fp;
	int nm;
	int i;

	if (ac != 3) {
	    fprintf (stderr, "Usage: %s <filename> <jd>\n", av[0]);
	    abort();
	}
	fn = av[1];
	jd = atof (av[2]);

	fp = fopen (fn, "r");
	if (!fp) {
	    perror (fn);
	    abort();
	}

	nm = read_bdl (fp, jd, x, y, z, ynot);
	if (nm < 0) {
	    fprintf (stderr, "%s\n", ynot);
	    abort();
	}

	for (i = 0; i < nm; i++)
	    printf (" X= %19.11E Y= %19.11E Z= %19.11E\n", x[i], y[i], z[i]);

	return (0);
}
#endif

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: bdl.c,v $ $Date: 2008/04/20 08:11:35 $ $Revision: 1.6 $ $Name:  $"};

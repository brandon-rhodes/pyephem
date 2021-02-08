/* USNOSetup(): call to change options and base directories.
 * USNOFetch(): return an array of ObjF matching the given criteria.
 * based on sample code in demo.tar on SA1.0 CDROM and info in read.use.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xephem.h"

#define	CATBPR	12	/* bytes per star record in .cat file */
#define	ACCBPR	30	/* bytes per record in .acc file */

typedef unsigned int UI;
typedef unsigned char UC;

/* One Field star */
typedef struct {
    float ra, dec;	/* J2000, rads */
    float mag;		/* magnitude */
} FieldStar;

/* an array of ObjF which can be grown efficiently in mults of NINC */
typedef struct {
    ObjF *mem;		/* malloced array */
    int used;		/* number actually in use */
    int max;		/* cells in mem[] */
} StarArray;

#define	NINC	16	/* grow StarArray array this many at a time */

static int corner (double r0, double d0, double rov, int *nr, double fr[2],
    double lr[2], int *nd, double fd[2], double ld[2], int zone[2], char msg[]);
static int fetchSwath (int zone, double maxmag, double fr, double lr,
    double fd, double ld, StarArray *sap, char msg[]);
static int crackCatBuf (UC buf[CATBPR], FieldStar *fsp);
static int addGS (StarArray *sap, FieldStar *fsp);

static char *cdpath;		/* where CD rom is mounted */
static int nogsc;		/* set to 1 to exclude GSC stars */

/* save the path to the base of the cdrom.
 * test for some reasonable entries.
 * return 0 if looks ok, else -1 and reason in msg[].
 */
int
USNOSetup (cdp, wantgsc, msg)
char *cdp;
int wantgsc;
char msg[];
{
	char tstname[1024];
	FILE *fp = NULL;
	int n;

	/* look for any legal zone*.cat file (this allows for A CDs) */
	for (n = 0; n <= 1725; n += 75) {
	    (void) sprintf (tstname, "%s/zone%04d.cat", cdp, n);
	    fp = fopenh (tstname, "r");
	    if (fp)
		break;
	}
	if (!fp) {
	    sprintf (msg, "No zone files in %s: %s", cdp, syserrstr());
	    return (-1);
	}
	fclose (fp);

	/* store our own copy of path */
	if (cdpath)
	    free (cdpath);
	cdpath = malloc (strlen(cdp) + 1);
	strcpy (cdpath, cdp);

	/* store GSC flag */
	nogsc = !wantgsc;

	/* probably ok */
	return (0);

}

/* create or add to a malloced array of ObjF at *opp in the given region and
 *   return the new total count.
 * if opp == NULL we don't malloc anything but just do the side effects;
 * else *opp already has nopp ObjF in it (it's ok if *opp == NULL).
 * we return new total number of stars or -1 if real trouble.
 * *opp is only changed if we added any.
 * msg might contain a message regardless of the return value.
 */
int
USNOFetch (r0, d0, fov, fmag, opp, nopp, msg)
double r0;	/* center RA, rads */
double d0;	/* center Dec, rads */
double fov;	/* field of view, rads */
double fmag;	/* faintest mag */
ObjF **opp;	/* *opp will be a malloced array of the ObjF in region */
int nopp;       /* if opp: initial number of ObjF already in *opp */
char msg[];	/* filled with error message if return -1 */
{
	double fr[2], lr[2];	/* first and last ra in each region, up to 2 */
	double fd[2], ld[2];	/* first and last dec in each region, up to 2 */
	int nr, nd;		/* number of ra and dec regions, max 2 each */
	int zone[2];		/* zone for filename, up to 2 */
	double rov;		/* radius of view, degrees */
	StarArray sa;		/* malloc accumulator */
	int i, j, s;

	/* insure there is a cdpath set up */
	if (!cdpath) {
	    strcpy (msg, "USNOFetch() called before USNOSetup()");
	    return (-1);
	}

	/* convert to cdrom units */
	r0 = raddeg(r0);
	d0 = raddeg(d0);
	rov = raddeg (fov)/2;
	if (rov >= 7.5) {
	    xe_msg (0, "USNO FOV being cut to 15 degrees");
	    rov = 7.5;
	}

	/* find the files to use and ranges in each */
	i = corner (r0, d0, rov, &nr, fr, lr, &nd, fd, ld, zone, msg);
	if (i < 0)
	    return (-1);

	/* init the array.
	 * max == -1 will mean we just keep a count but don't build the array.
	 */
	if (opp) {
	    sa.mem = *opp;
	    sa.used = nopp;
	    sa.max = nopp;
	} else {
	    sa.mem = NULL;
	    sa.used = 0;
	    sa.max = -1;
	}

	/* fetch each chunk, adding to sa */
	for (i = 0; i < nd; i++) {
	    for (j = 0; j < nr; j++) {
		s = fetchSwath(zone[i],fmag,fr[j],lr[j],fd[i],ld[i],&sa,msg);
		if (s < 0) {
		    /* array has likely moved */
		    if (opp && sa.mem)
			*opp = sa.mem;
		    return (-1);
		}
	    }
	}

	/* pass back to caller if interested else just toss */
	if (opp && sa.mem)
	    *opp = sa.mem;
	else if (sa.mem)
	    free ((void *)sa.mem);

	return (sa.used);
}

static int
corner (double r0, double d0, double rov, int *nr, double fr[2], double lr[2],
int *nd, double fd[2], double ld[2], int zone[2], char msg[])
{
	double x1, x2;
	int z1, z2;
	int z, j;

	/* find limits on ra, taking care at poles and if span 24h */
	if (d0 - rov <= -90.0 || d0 + rov >= 90.0) {
	    x1 = 0.0;
	    x2 = 360.0;
	} else {
	    double cd = cos(degrad(d0));
	    x1 = r0 - rov/cd;
	    x2 = r0 + rov/cd;
	}
	if (x1 < 0.0) {
	    *nr = 2;
	    fr[0] = 0.0;
	    lr[0] = x2;
	    fr[1] = 360.0 + x1;
	    lr[1] = 360.0;
	} else if (x2 > 360.0) {
	    *nr = 2;
	    fr[0] = 0.0;
	    lr[0] = x2 - 360.0;
	    fr[1] = x1;
	    lr[1] = 360.0;
	} else {
	    *nr = 1;
	    fr[0] = x1;
	    lr[0] = x2;
	}

	/* find dec limits and zones */
	x1 = d0 - rov;
	if (x1 < -90.0)
	    x1 = -90.0;
	x2 = d0 + rov;
	if (x2 >= 90.0)
	    x2 = 90.0 - 1./1e5;
	z1 = (int)floor((x1 + 90.0)/7.5);
	z2 = (int)floor((x2 + 90.0)/7.5);
	*nd = z2 - z1 + 1;
	if (*nd > 2) {
	    *nd = 2;
	    z2 = z1 + 1;
	    /*
	    sprintf (msg, "No! ndec = %d", *nd);
	    return (-1);
	    */
	}
	j = 0;
	for (z = z1; z <= z2; z++) {
	    double dmin = z*7.5 - 90.0;
	    double dmax = dmin+7.5;
	    zone[j] = z*75;
	    fd[j] = x1 > dmin ? x1 : dmin;
	    ld[j] = x2 < dmax ? x2 : dmax;
	    j++;
	}

	return (0);
}

static int
fetchSwath (int zone, double maxmag, double fr, double lr, double fd,
double ld, StarArray *sap, char msg[])
{
	char fn[1024];
	char buf[ACCBPR];
	UC catbuf[CATBPR];
	FieldStar fs;
	long frec;
	long os;
	FILE *fp;

	/* read access file for position in catalog file */
	sprintf (fn, "%s/zone%04d.acc", cdpath, zone);
	fp = fopenh (fn, "r");
	if (fp == NULL) {
	    sprintf (msg, "%s: %s", fn, syserrstr());
	    return (-1);
	}
	os = ACCBPR*(long)floor(fr/3.75);
	if (fseek (fp, os, SEEK_SET) < 0) {
	    sprintf (msg, "%s: fseek(%ld): %s", fn, (long)os, syserrstr());
	    fclose (fp);
	    return (-1);
	}
	if (fread (buf, ACCBPR, 1, fp) != 1) {
	    sprintf (msg, "%s: fread(@%ld): %s", fn, (long)os,syserrstr());
	    fclose (fp);
	    return (-1);
	}
	fclose (fp);
	if (sscanf (buf, "%*f %ld", &frec) != 1) {
	    sprintf (msg, "%s: sscanf(%s)", fn, buf);
	    return (-1);
	}

	/* open and position the catalog file */
	sprintf (fn, "%s/zone%04d.cat", cdpath, zone);
	fp = fopenh (fn, "r");
	if (fp == NULL) {
	    sprintf (msg, "%s: %s", fn, syserrstr());
	    return (-1);
	}
	os = (long)(frec-1)*CATBPR;
	if (fseek (fp, os, SEEK_SET) < 0) {
	    sprintf (msg, "%s: fseek(%ld): %s", fn, (long)os, syserrstr());
	    fclose (fp);
	    return (-1);
	}

	/* now want in rads */
	fr = degrad(fr);
	lr = degrad(lr);
	fd = degrad(fd);
	ld = degrad(ld);

	/* read until end or find star with RA larger than lr */
	while (fread (catbuf, CATBPR, 1, fp) == 1) {
	    os += CATBPR;
	    if (crackCatBuf (catbuf, &fs)==0 && fs.mag<=maxmag && fs.ra>=fr
				    && fs.ra<=lr && fs.dec>=fd && fs.dec<=ld) {
		if (addGS (sap, &fs) < 0) {
		    sprintf (msg, "No more memory");
		    fclose (fp);
		    return (-1);
		}
	    }

	    /* sorted by ra so finished when hit upper limit */
	    if (fs.ra > lr)
		break;
	}

	/* finished */
	fclose (fp);

	/* ok*/
	return (0);
}

/* crack the star field in buf.
 * return 0 if ok, else -1.
 */
static int
crackCatBuf (UC buf[CATBPR], FieldStar *fsp)
{
#define	BEUPACK(b) (((UI)((b)[0])<<24) | ((UI)((b)[1])<<16) | ((UI)((b)[2])<<8)\
							    | ((UI)((b)[3])))
	double ra, dec;
	int red, blu;
	UI mag;

	/* first 4 bytes are packed RA, big-endian */
	ra = BEUPACK(buf)/(100.0*3600.0*15.0);
	fsp->ra = (float)hrrad(ra);

	/* next 4 bytes are packed Dec, big-endian */
	dec = BEUPACK(buf+4)/(100.0*3600.0) - 90.0;
	fsp->dec = (float)degrad(dec);

	/* last 4 bytes are packed mag info -- can lead to rejection */
	mag = BEUPACK(buf+8);
	if (mag & 0x8000) {
	    /* negative means corresponding GSC */
	    if (nogsc)
		return (-1);
	    mag &= 0x7fffffff;	/* 1's or 2's comp?? */
	}
	if (mag/1000000000 != 0)
	    return (-1);	/* poor magnitudes */
	red = mag % 1000u;	/* lower 3 digits */
	blu = (mag/1000u)%1000u;/* next 3 digits up */
	if (red > 250) {
	    if (blu > 250)
		return (-1);
	    else
		fsp->mag = (float)(blu/10.);
	} else
	    fsp->mag = (float)(red/10.);

	return (0);
}

/* add fsp to sa as another ObjF.
 * return -1 if no more memory, else 0.
 */
static int
addGS (StarArray *sap, FieldStar *fsp)
{
	char rstr[32], dstr[32];
	Obj *op;

	/* if max < 0, we are just counting */
	if (sap->max < 0)
	    return (0);

	/* get next entry, mallocing if fresh out */
	if (sap->used == sap->max) {
	    char *newmem = (char *)sap->mem;
	    int nbytesnow = sap->max * sizeof(ObjF);
	    int morebytes = NINC * sizeof(ObjF);

	    newmem = newmem ? realloc (newmem, nbytesnow + morebytes)
			    : malloc (morebytes);
	    if (!newmem)
		return (-1);
	    zero_mem (newmem + nbytesnow, morebytes);
	    sap->mem = (ObjF *)newmem;
	    sap->max += NINC;
	}

	/* next ObjF */
	op = (Obj *) &sap->mem[sap->used++];

	/* make up a fixed name */
	fs_sexa (rstr, radhr(fsp->ra), 2, 3600);
	if (rstr[0] == ' ') rstr[0] = '0';
	memmove (rstr+2, rstr+3, 2);
	memmove (rstr+4, rstr+6, 3);
	fs_sexa (dstr, raddeg(fsp->dec), 3, 3600);
	if (dstr[1] == ' ') { dstr[0] = '+'; dstr[1] = '0'; }
	if (dstr[1] == '-') { dstr[0] = '-'; dstr[1] = '0'; }
	if (dstr[0] == ' ')   dstr[0] = '+';
	memmove (dstr+3, dstr+4, 2);
	memmove (dstr+5, dstr+7, 3);
	if (sprintf (op->o_name, "USNO %s%s", rstr, dstr) >= MAXNM) {
	    printf ("Bug! USNO name format too long\n");
	    abort();
	}

	/* other info */
	op->o_type = FIXED;
	op->f_class = 'S';
	op->f_RA = fsp->ra;
	op->f_dec = fsp->dec;
	op->f_epoch = J2000;
	set_fmag (op, fsp->mag);

	/* ok */
	return (0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: usno.c,v $ $Date: 2004/05/05 17:43:32 $ $Revision: 1.15 $ $Name:  $"};

/* read the UCAC2, 3 or 4 catalog, depending on which index file is found.
 * UCACSetup(): call to change options and base directories.
 * UCACFetch(): return an array of ObjF matching the given criteria.
 * UCAC2 and 3 stars are in 360 files each .5 deg hi, within 240 bins each
    .1 hour RA wide.
 * UCAC4 stars are in 900 files each .2 deg hi, within 1440 bins each
    1 minute of RA wide.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "xephem.h"

#define	MAXFOV	15.0			/* max fov, degs */

typedef unsigned char UC;		/* byte */
typedef unsigned int UI;		/* unsigned integer */

/* access an I*2 or I*4 at offset i in UC array a in little-endian byte order.
 * a bit slow but ultra portable.
 */
#define	I2(a,i)		((int)(short)((((UI)(a)[i]) | (((UI)(a)[i+1])<<8))))
#define	I4(a,i)		((int)((((UI)(a)[i]) | (((UI)(a)[i+1])<<8) | \
				(((UI)(a)[i+2])<<16) | (((UI)(a)[i+3])<<24))))

/* keep track of an array of ObjF */
typedef struct {
    ObjF *mem;				/* malloced array, or NULL if none */
    int n;				/* number actually in use */
} ObjFArray;

/* zone sizes for UCAC2 and 3 */
#define	NZW	240			/* number ra zones wide */
#define	ZW	(24./NZW)		/* zone width, hours */
#define	NZH	360			/* number dec zones hi */
#define	ZH	(180./NZH)		/* zone height, degrees */

/* zones sizes for UCAC4 */
#define	NZW4	1440			/* number ra zones wide */
#define	ZW4	(24./NZW4)		/* zone width, hours */
#define	NZH4	900			/* number dec zones hi */
#define	ZH4	(180./NZH4)		/* zone height, degrees */

#define	DPMAS	(1.0/3600000.0)		/* degrees per milliarcsecond */

typedef UC U2Star[44];			/* UCAC2 record */
typedef UC U3Star[84];			/* UCAC3 record */
typedef UC U4Star[78];			/* UCAC4 record */
static char *basedir;			/* full dir with zone files and index */
static FILE *indexfp;			/* index file handle */

static FILE *openIndex (char dir[], char msg[], int *ucacvp);
static int add4Bin (ObjFArray *oap, int rz, int dz);
static int read4Index (int rz, int dz, int *nskip, int *nnew);
static int read4Raw (U4Star u[], int dz, int nskip, int nraw);
static void crack4 (U4Star u, int dz, int znm, Obj *op);
static int readu4hpm (int rnm, int *pmra, int *pmdec);
static int add3Bin (ObjFArray *oap, int rz, int dz);
static int read3Index (int rz, int dz, int *nskip, int *nnew);
static int read3Raw (U3Star u[], int dz, int nskip, int nraw);
static void crack3 (U3Star u, Obj *op);
static int add2Bin (ObjFArray *oap, int rz, int dz);
static int get2N (int rz, int dz, int *idp);
static int read2Raw (U2Star u[], int dz, int nskip, int nnew);
static void crack2 (U2Star u, int id, Obj *op);
static void jkspect (double j, double k, Obj *op);

/* save the path to the base dir.
 * test for some reasonable entries.
 * return 0 if looks ok, else -1 and reason in msg[].
 */
int
UCACSetup (char *path, char msg[])
{
	FILE *fp;
	int ucacv;

	/* sanity check the path by looking for the index file */
	fp = openIndex (path, msg, &ucacv);
	if (!fp)
	    return (-1);
	fclose (fp);

	/* store persistent copy of path */
	if (basedir)
	    free (basedir);
	strcpy (basedir = malloc(strlen(path)+1), path);

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
UCACFetch (
double r0,	/* center RA, rads */
double d0,	/* center Dec, rads */
double fov,	/* field of view, rads */
double fmag,	/* faintest mag */
ObjF **opp,	/* *opp will be a malloced array of the ObjF in region */
int nopp,       /* if opp: initial number of ObjF already in *opp */
char msg[] 	/* status or error message if msg[0] != '\0' on return */
)
{
	int (*doucac)() = 0;		/* function to process */
	int d0z, drovz;			/* dec center and radius, in 0-zones */
	ObjFArray oa;			/* malloc accumulator */
	int dz;				/* scanning dec zone number */
	double zw = 0, zh = 0;		/* zone width and height */
	int nzw = 0, nzh = 0;		/* number of zones wide and high */
	int ucacv;			/* version 2, 3, or 4 */

	/* init message */
	msg[0] = '\0';

	/* insure there is a basedir set up */
	if (!basedir) {
	    strcpy (msg, "UCACFetch() called before UCACSetup()");
	    return (-1);
	}

	/* don't go crazy */
	if (fov > degrad(MAXFOV)) {
	    sprintf (msg, "UCAC FOV being clamped to %g degs", MAXFOV);
	    fov = degrad(MAXFOV);
	}

	/* open the index file */
	indexfp = openIndex (basedir, msg, &ucacv);
	if (!indexfp)
	    return (-1);

	/* setup based on which version */
	switch (ucacv) {
	case 2:
	    doucac = add2Bin;
	    zw = ZW;
	    zh = ZH;
	    nzw = NZW;
	    nzh = NZH;
	    break;

	case 3:
	    doucac = add3Bin;
	    zw = ZW;
	    zh = ZH;
	    nzw = NZW;
	    nzh = NZH;
	    break;

	case 4:
	    doucac = add4Bin;
	    zw = ZW4;
	    zh = ZH4;
	    nzw = NZW4;
	    nzh = NZH4;
	    break;
	}

	/* init the array.
	 * mem==NULL means we just keep a count but don't build the array.
	 */
	if (opp) {
	    oa.mem = *opp;
	    if (!oa.mem)
		oa.mem = (ObjF*)malloc(1);	/* seed for realloc */
	    oa.n = nopp;
	} else {
	    oa.mem = NULL;
	    oa.n = 0;
	}

	/* convert to zones */
	d0z = (int)floor(raddeg(d0+PI/2)/zh);
	drovz = (int)ceil(raddeg(fov/2)/zh);

	/* scan each ra bin in each dec band */
	for (dz = d0z - drovz; dz <= d0z + drovz; dz++) {
	    int rz, rz0, rz1;			/* scanning ra zones */
	    if (dz < 0 || dz >= nzh)
		continue;			/* over a pole */
	    if (dz > 2*nzh-d0z-drovz || dz < drovz-d0z) {
		rz0 = 0;			/* pole in view */
		rz1 = nzw-1;
	    } else {
		int r0z, rrovz;			/* RA center and rad, 0-zones */
		r0z = (int)floor(radhr(r0)/zw);
		if (fabs(d0) < PI/2) {
		    rrovz = (int)ceil(radhr(fov/2)/zw/cos(d0));
		    if (rrovz > nzw/2)
			rrovz = nzw/2;
		} else
		    rrovz = nzw/2;
		rz0 = r0z - rrovz;		/* normal patch */
		rz1 = r0z + rrovz;
	    }
	    for (rz = rz0; rz <= rz1; rz++)
		if ((*doucac) (&oa, rz, dz) < 0) {
		    char rstr[32], dstr[32];
		    fs_sexa (rstr, rz*zw, 2, 60);
		    fs_sexa (dstr, dz*zh-90, 3, 60);
		    sprintf (msg, "Error reading UCAC data near %s %s",rstr,dstr);
		    if (!opp && oa.mem)
			free ((void *)oa.mem);
		    fclose (indexfp);
		    return (-1);
		}
	}

	/* pass back to caller if interested */
	if (opp)
	    *opp = oa.mem;

	/* close index file */
	fclose (indexfp);

	/* return list */
	return (oa.n);
}

/* add the stars in the given 0-based ra/dec zone to oap.
 * return 0 if ok, else -1
 * N.B. rz may wrap 
 */
static int
add4Bin (ObjFArray *oap, int rz, int dz)
{
	int nskip, nnew;
	U4Star *u;
	int i;

	/* beware of ra wrap */
	if (rz < 0)
	    rz += NZW4;
	if (rz >= NZW4)
	    rz -= NZW4;

	/* n stars up through and including this patch */
	if (read4Index (rz, dz, &nskip, &nnew) < 0)
	    return (-1);

	/* read the raw star records in this record */
	u = malloc (nnew * sizeof(U4Star));
	if (!u) {
	    xe_msg (0, "UCAC: malloc(%d) failed", nnew*sizeof(U4Star));
	    return (-1);
	}
	if (read4Raw (u, dz, nskip, nnew) < 0) {
	    free ((char *)u);
	    return (-1);
	}

	/* expand obj memory and crack if interested */
	if (oap->mem) {
	    char *moreobj = realloc (oap->mem, (oap->n + nnew)*sizeof(ObjF));
	    if (!moreobj) {
		xe_msg (0, "UCAC: malloc failed");
		free (u);
		return (-1);
	    }
	    oap->mem = (ObjF *)moreobj;

	    for (i = 0; i < nnew; i++)
		crack4 (u[i], dz+1, nskip+i+1, (Obj *)(oap->mem + oap->n + i));
	}

	/* always update count */
	oap->n += nnew;

	/* ok */
	free ((char *)u);
	return (0);
}

/* given 0-based ra and dec zone, return number of stars to skip in dec band
 * and how many are in the zone.
 * return 0 if ok, else -1
 */
static int
read4Index (int rz, int dz, int *nskip, int *nnew)
{
	off_t offset;
	UC i4[4];

	offset = (rz*NZH4 + dz)*sizeof(i4);
	if (fseek (indexfp, offset, SEEK_SET) < 0) {
	    xe_msg (0, "UCAC: index n0 seek %ld: %s", offset, syserrstr());
	    return (-1);
	}
	if (fread (i4, sizeof(i4), 1, indexfp) != 1) {
	    xe_msg (0, "UCAC: index n0 read %d: %s", sizeof(i4), syserrstr());
	    return (-1);
	}
	*nskip = I4(i4, 0);


	offset = (NZW4*NZH4 + rz*NZH4 + dz)*sizeof(i4);
	if (fseek (indexfp, offset, SEEK_SET) < 0) {
	    xe_msg (0, "UCAC: index nn seek %ld: %s", offset, syserrstr());
	    return (-1);
	}
	if (fread (i4, sizeof(i4), 1, indexfp) != 1) {
	    xe_msg (0, "UCAC: index nn read %d: %s", sizeof(i4), syserrstr());
	    return (-1);
	}
	*nnew  = I4(i4, 0);

	return (0);
}

/* read the nnew raw star records starting after nskip records for the given
 * 0-based dec zone.
 * return 0 if ok, else -1
 */
static int
read4Raw (U4Star u[], int dz, int nskip, int nnew)
{
	char fn[1024];
	FILE *fp;

	sprintf (fn, "%s/u4b/z%03d", basedir, dz+1);
	fp = fopen (fn, "r");
	if (!fp) {
	    xe_msg (0, "UCAC: open %s: %s", fn, syserrstr());
	    return (-1);
	}
	if (fseek (fp, nskip*sizeof(U4Star), SEEK_SET) < 0) {
	    xe_msg (0, "UCAC: seek %ld %s: %s", nskip*sizeof(U4Star), fn, syserrstr());
	    fclose (fp);
	    return (-1);
	}
	if (fread (u, sizeof(U4Star), nnew, fp) != nnew) {
	    xe_msg (0, "UCAC: read %d, %s: %s", sizeof(U4Star), fn, syserrstr());
	    fclose (fp);
	    return (-1);
	}
	fclose (fp);
	return (0);
}

/* convert the raw UCAC record into the ObjF portion of op.
 * proper motion applied to 2000, libastro::circum.c takes it from there.
 */
static void
crack4 (U4Star u, int dz, int znm, Obj *op)
{
	int rawpmra, rawpmdec;
	double epra, epdc;
	double spd;

	memset (op, 0, sizeof(ObjF));	/* N.B. ObjF, not Obj */

	op->o_type = FIXED;
	op->f_class = (u[14] == 0) ? ((u[13] == 0) ? 'S' : 'T') : 'D';
	op->f_RA = degrad (I4(u,0)*DPMAS);
	spd = I4(u,4)*DPMAS;
	op->f_dec = degrad (spd - 90);
	op->f_epoch = J2000;
	set_fmag (op, I2(u,10)/1000.0);

	epra = I2(u,20)*.01+1900;	/* pmra epoch, years */
	epdc = I2(u,22)*.01+1900;	/* pmdec epoch, years */
	rawpmra = I2(u,24);		/* .1 mas/yr, on sky */
	rawpmdec = I2(u,26);		/* .1 mas/yr */
	if (rawpmra == 32767 || rawpmdec == 32767) {
	    int rnm = I4(u,68);
	    if (readu4hpm (rnm, &rawpmra, &rawpmdec) < 0)
		xe_msg (1, "Can not find proper motion for star UCAC4 %d", rnm);
	}
	op->f_pmRA =  degrad(rawpmra*0.1*DPMAS)/365.25/cos(op->f_dec);	/* want RA rad/day */
	op->f_pmdec = degrad(rawpmdec*0.1*DPMAS)/365.25;		/* want Dec rad/day */
	op->f_RA += op->f_pmRA * (2000.0 - epra)*365.25;		/* move to 2000 */
	op->f_dec += op->f_pmdec * (2000.0 - epdc)*365.25;		/* move to 2000 */

	jkspect (I2(u,34)*0.001, I2(u,38)*0.001, op);

	sprintf (op->o_name, "UCAC4-%03d-%06d", dz, znm);
}

/* convert running star number to proper motion.
 * return 0 if ok, else -1
 */
static int
readu4hpm (int rnm, int *pmra, int *pmdec)
{
	typedef struct {
	    int rnm;
	    int zn;
	    int rnz;
	    int pmrc;
	    int pmd;
	    int ra;
	    int spd;
	    int maga;
	} U4HPM;
	static U4HPM *u4hpm;
	static int nu4hpm;
	int i;

	/* cache file first time only */
	if (!u4hpm) {
	    char fn[1024];
	    char line[128];
	    FILE*fp;

	    sprintf (fn, "%s/u4i/u4hpm.dat", basedir);
	    fp = fopen (fn, "r");
	    if (!fp) {
		xe_msg (1, "Can not open %s", fn);
		return (-1);
	    }
	    while (fgets (line, sizeof(line), fp)) {
		u4hpm = (U4HPM *) realloc (u4hpm, (nu4hpm+1)*sizeof(U4HPM));
		U4HPM *up = &u4hpm[nu4hpm++];
		sscanf (line, "%d %d %d %d %d %d %d %d", &up->rnm, &up->zn, &up->rnz,
				&up->pmrc, &up->pmd, &up->ra, &up->spd, &up->maga);
	    }
	    fclose (fp);

	};

	/* search for matching rnm */
	for (i = 0; i < nu4hpm; i++) {
	    if (rnm == u4hpm[i].rnm) {
		*pmra = u4hpm[i].pmrc;
		*pmdec = u4hpm[i].pmd;
		return (0);
	    }
	}

	xe_msg (1, "Can not find UCAC4 u4hpm entry for %d", rnm);
	/* not found */
	return (-1);
}

/* add the stars in the given 0-based ra/dec zone to oap.
 * return 0 if ok, else -1
 * N.B. rz may wrap 
 */
static int
add3Bin (ObjFArray *oap, int rz, int dz)
{
	int nskip, nnew;
	U3Star *u;
	int i;

	/* beware of ra wrap */
	if (rz < 0)
	    rz += NZW;
	if (rz >= NZW)
	    rz -= NZW;

	/* n stars up through and including this patch */
	if (read3Index (rz, dz, &nskip, &nnew) < 0)
	    return (-1);

	/* read the raw star records in this record */
	u = malloc (nnew * sizeof(U3Star));
	if (!u) {
	    xe_msg (0, "UCAC: malloc(%d) failed", nnew*sizeof(U3Star));
	    return (-1);
	}
	if (read3Raw (u, dz, nskip, nnew) < 0) {
	    free ((char *)u);
	    return (-1);
	}

	/* expand obj memory and crack if interested */
	if (oap->mem) {
	    char *moreobj = realloc (oap->mem, (oap->n + nnew)*sizeof(ObjF));
	    if (!moreobj) {
		xe_msg (0, "UCAC: malloc failed");
		free (u);
		return (-1);
	    }
	    oap->mem = (ObjF *)moreobj;

	    for (i = 0; i < nnew; i++)
		crack3 (u[i], (Obj *)(oap->mem + oap->n + i));
	}

	/* always update count */
	oap->n += nnew;

	/* ok */
	free ((char *)u);
	return (0);
}

/* given 0-based ra and dec zone, return number of stars to skip in dec band
 * and how many are in the zone.
 * return 0 if ok, else -1
 */
static int
read3Index (int rz, int dz, int *nskip, int *nnew)
{
	off_t offset;
	UC i4[4];

	offset = (rz*NZH + dz)*sizeof(i4);
	if (fseek (indexfp, offset, SEEK_SET) < 0) {
	    xe_msg (0, "UCAC: index n0 seek %ld: %s", offset, syserrstr());
	    return (-1);
	}
	if (fread (i4, sizeof(i4), 1, indexfp) != 1) {
	    xe_msg (0, "UCAC: index n0 read %d: %s", sizeof(i4), syserrstr());
	    return (-1);
	}
	*nskip = I4(i4, 0);


	offset = (NZW*NZH + rz*NZH + dz)*sizeof(i4);
	if (fseek (indexfp, offset, SEEK_SET) < 0) {
	    xe_msg (0, "UCAC: index nn seek %ld: %s", offset, syserrstr());
	    return (-1);
	}
	if (fread (i4, sizeof(i4), 1, indexfp) != 1) {
	    xe_msg (0, "UCAC: index nn read %d: %s", sizeof(i4), syserrstr());
	    return (-1);
	}
	*nnew  = I4(i4, 0);

	return (0);
}

/* read the nnew raw star records starting after nskip records for the given
 * 0-based dec zone.
 * return 0 if ok, else -1
 */
static int
read3Raw (U3Star u[], int dz, int nskip, int nnew)
{
	char fn[1024];
	FILE *fp;

	sprintf (fn, "%s/z%03d", basedir, dz+1);
	fp = fopen (fn, "r");
	if (!fp) {
	    xe_msg (0, "UCAC: open %s: %s", fn, syserrstr());
	    return (-1);
	}
	if (fseek (fp, nskip*sizeof(U3Star), SEEK_SET) < 0) {
	    xe_msg (0, "UCAC: seek %ld %s: %s", nskip*sizeof(U3Star), fn, syserrstr());
	    fclose (fp);
	    return (-1);
	}
	if (fread (u, sizeof(U3Star), nnew, fp) != nnew) {
	    xe_msg (0, "UCAC: read %d, %s: %s", sizeof(U3Star), fn, syserrstr());
	    fclose (fp);
	    return (-1);
	}
	fclose (fp);
	return (0);
}

/* convert the raw UCAC record into the ObjF portion of op */
static void
crack3 (U3Star u, Obj *op)
{
	memset (op, 0, sizeof(ObjF));	/* N.B. ObjF, not Obj */

	sprintf (op->o_name, "3UCAC%09d", I4(u,80));
	op->o_type = FIXED;
	op->f_class = (u[14] == 0 || u[14] == 1) ? 'S' : 'T';
	op->f_RA = degrad (I4(u,0)*DPMAS);
	op->f_dec = degrad (I4(u,4)*DPMAS - 90.0);
	op->f_epoch = J2000;
	set_fmag (op, I2(u,10)/1000.0);
	op->f_pmRA =  degrad(I4(u,28)/10.0*DPMAS)/365.0/cos(op->f_dec);
	op->f_pmdec = degrad(I4(u,32)/10.0*DPMAS)/365.0;
	jkspect (I2(u,44)*0.001, I2(u,48)*0.001, op);
}

/* add the stars in the given 0-based ra/dec zone to oap.
 * return 0 if ok, else -1
 * N.B. rz may wrap 
 */
static int
add2Bin (ObjFArray *oap, int rz, int dz)
{
	int nthis, nprior, npriorzone, nnew;
	U2Star *u;
	int r, d;
	int i;

	/* beware of ra wrap */
	if (rz < 0)
	    rz += NZW;
	if (rz >= NZW)
	    rz -= NZW;

	/* n stars up through and including this patch */
	if (get2N (rz, dz, &nthis) < 0)
	    return (-1);

	/* n stars up to but not including this patch */
	if (rz > 0) {
	    r = rz-1;
	    d = dz;
	} else {
	    r = NZW-1;
	    d = dz-1;
	}
	if (d < 0)
	    nprior = 0;
	else if (get2N (r, d, &nprior) < 0)
	    return (-1);

	/* n stars up through last dec zone */
	if (dz == 0)
	    npriorzone = 0;
	else if (get2N (NZW-1, dz-1, &npriorzone) < 0)
	    return (-1);

	/* read the raw star records in this record */
	nnew = nthis - nprior;
	u = malloc (nnew * sizeof(U2Star));
	if (!u)
	    return (-1);
	if (read2Raw (u, dz, nprior-npriorzone, nnew) < 0) {
	    free ((char *)u);
	    return (-1);
	}

	/* expand obj memory and crack if interested */
	if (oap->mem) {
	    char *moreobj = realloc (oap->mem, (oap->n + nnew)*sizeof(ObjF));
	    if (!moreobj) {
		free (u);
		return (-1);
	    }
	    oap->mem = (ObjF *)moreobj;

	    for (i = 0; i < nnew; i++)
		crack2 (u[i], nprior+i+1, (Obj *)(oap->mem + oap->n + i));
	}

	/* always count */
	oap->n += nnew;

	/* ok */
	free ((char *)u);
	return (0);
}

/* return the number of stars up through and including the given 0-based patch.
 * return 0 if ok, else -1
 */
static int
get2N (int rz, int dz, int *idp)
{
	off_t offset;
	UC nat[4];

	offset = (dz*NZW + rz)*sizeof(nat);
	if (fseek (indexfp, offset, SEEK_SET) < 0)
	    return (-1);
	if (fread (nat, sizeof(nat), 1, indexfp) != 1)
	    return (-1);
	*idp = I4(nat,0);
	return (0);
}

/* read the n raw star records starting after nskip records from the given
 * 0-based dec zone.
 * return 0 if ok, else -1
 */
static int
read2Raw (U2Star u[], int dz, int nskip, int nnew)
{
	char fn[1024];
	FILE *fp;

	sprintf (fn, "%s/z%03d", basedir, dz+1);
	fp = fopen (fn, "r");
	if (!fp)
	    return (-1);
	if (fseek (fp, nskip*sizeof(U2Star), SEEK_SET) < 0) {
	    fclose (fp);
	    return (-1);
	}
	if (fread (u, sizeof(U2Star), nnew, fp) != nnew) {
	    fclose (fp);
	    return (-1);
	}
	fclose (fp);
	return (0);
}

/* convert the raw UCAC record into the ObjF portion of op */
static void
crack2 (U2Star u, int id, Obj *op)
{
	memset (op, 0, sizeof(ObjF));	/* N.B. ObjF, not Obj */

	sprintf (op->o_name, "UCAC%08d", id);
	op->o_type = FIXED;
	op->f_class = 'S';
	op->f_RA = degrad (I4(u,0)*DPMAS);
	op->f_dec = degrad (I4(u,4)*DPMAS);
	op->f_epoch = J2000;
	set_fmag (op, I2(u,8)*0.01);
	if ((char)u[30] <= -27 && (char)u[31] <= -27) {
	    /* PM only store if "goodness of fit" is less than 5 */
	    op->f_pmRA =  degrad(I4(u,20)*0.1*DPMAS)/365.0;
	    op->f_pmdec = degrad(I4(u,24)*0.1*DPMAS)/365.0;
	}
	jkspect (I2(u,36)*0.001, I2(u,40)*0.001, op);
}

/* open index file in dir, return version and FILE * else NULL with message */
static FILE *
openIndex (char dir[], char msg[], int *ucacvp)
{
	static char u2[] = "u2index.da";	/* zone index file name */
	static char u3[] = "u3index.unf";	/* zone index file name */
	static char u4[] = "u4index.unf";	/* zone index file name */
	char full[1024];
	FILE *fp;

	sprintf (full, "%s/%s", dir, u2);
	fp = fopen (full, "r");
	if (fp) {
	    *ucacvp = 2;
	} else {
	    sprintf (full, "%s/%s", dir, u3);
	    fp = fopen (full, "r");
	    if (fp) {
		*ucacvp = 3;
	    } else {
		sprintf (full, "%s/u4i/%s", dir, u4);
		fp = fopen (full, "r");
		if (fp) {
		    *ucacvp = 4;
		} else {
		    sprintf (msg, "Can not find %s or %s or %s", u2, u3, u4);
		}
	    }
	}
	return (fp);
}

/* given the J and K_s fields in the UCAC catalog, fill in f_spect[] in op.
 * from Brian Skiff off TASS "Stellar Catalogs" page, with artistic license.
 * http://stupendous.rit.edu/tass/catalogs/uvby.calib
 */
static void
jkspect (double j, double k, Obj *op)
{
	static struct {
	    char sp[sizeof(op->f_spect)+1];	/* +1 for \0 in init string */
	    float jminusk;			/* j - k */
	} jkmag[] = {
	    {"B2",   -0.22},
	    {"B5",   -0.20},
	    {"B0",   -0.17},
	    {"A0",    0.00},
	    {"F0",    0.15},
	    {"G2",    0.37},
	    {"G8",    0.58},
	    {"K0",    0.63},
	    {"M0",    1.01},
	    {"M2",    1.22},
	    {"M5",    1.25},
	    {"M6",    1.26},
	    {"M7",    1.27},
	    {"V4",    1.57},
	};

	double jmk = j-k;
	int i;

	for (i = 1; i < sizeof(jkmag)/sizeof(jkmag[0]); i++) {
	    if (jmk <= jkmag[i].jminusk) {
		strncpy (op->f_spect, jkmag[i-1].sp, sizeof(op->f_spect));
		return;
	    }
	}
}

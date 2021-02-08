/* GSCSetup(): call to change base directories.
 * GSCFetch(): return an array of ObjF matching the given criteria.
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "xephem.h"


/* .ech file format history:
 * 1.5	3/18/96	just use the first if multiple entries (and there are many)
 * 1.4	10/5/95	change header to accommodate more stars in one region.
 */

static double cacheversion = 1.5;	/* must be writeable using %.1f */

/* Program history:
 * 2/22/96	put all into one file for external convenience.
 * 10/5		allow more free-form header format
 * 8/16		fix fov limiting; send End message when finished; lint cleanups.
 * 8/2		mimic the cd-rom's subdirectory structure.
 * 5/31		change some defaults and improve help message.
 * 5/2/95	begin work
 */

/* the original request to be fullfilled */
typedef struct {
    double ra, dec;	/* center of interest, J2000, rads */
    double cdec, sdec;	/* handy cos(dec) and sin(dec) */
    double rov;		/* radius-of-view, rads */
    double crov;	/* cos(rov), for convenience */
    double mag;		/* limiting mag */
} Request;

/* an array of ObjF which can be grown efficiently in mults of NINC */
typedef struct {
    ObjF *mem;		/* malloced array */
    int max;		/* max number of cells in mem[] or -1 to just count */
    int used;		/* number actually in use */
} GSCArray;

#define	NINC	64	/* grow GSCArray mem this many at a time */


/* what we need to know about a small GSC region */
typedef struct {
    /* following are filled by gscPickRegion() */
    char dir[6];	/* dir off gsc/, e.g. "n1234" */
    char file[9];	/* file off gsc/<dir>, e.g. "5678.gsc" */
    int south;		/* 0 if north, 1 if north */
    int id;		/* small region id number */
    double ra0;		/* eastern edge, rads */
    double dec0;	/* fabs(dec edge nearest 0), rads */

    /* following are filled by gscOpenRegion() or cacheOpen/CreateRegion() */
    FILE *fp;		/* file pointer, once open */
    int nrows;		/* rows (stars) remaining */

    /* following is only used by the cache subsystem. */
    double cosdec0;
} GSCRegion;

/* info about one GSC entry.
 * filled in by gscNextEntry()
 */
typedef struct {
    int id;		/* id */
    int class;		/* 0 star */
    double ra;		/* rads */
    double dec;		/* rads */
    double mag;		/* magnitude */
} GSCEntry;


/* paths and options */
static char *cdpath;	/* dir where GSC CD rom is mounted */
static char *chpath;	/* dir of cache base */
static int nocache;	/* set if don't want to use the cache */
static int nocdrom;	/* set if don't wnat to use the cdrom */

static char *lmsg;	/* local ptr to user's msg buffer */

static int handleRequest (Request *qp, GSCArray *ap);
static int fetchRegion (GSCRegion *rp, Request *qp, GSCArray *ap);
static int inFOV (Request *qp, GSCEntry *ep);
static int magOK (Request *qp, GSCEntry *ep);
static int addOneStar (GSCArray *ap, GSCRegion *rp, GSCEntry *ep);
static int mymkdir (char *path);

static void gscPickRegion (double ra, double dec, GSCRegion *rp);
static int gscOpenRegion (GSCRegion *rp);
static int gscSimpleFits (FILE *fp);
static void gscOpenFile (GSCRegion *rp, char msg[]);
static int gscGetNextEntry (GSCRegion *rp, GSCEntry *ep);
static void gscCloseRegion (GSCRegion *rp);
static int cacheCreateRegion (GSCRegion *rp, GSCRegion *cp);

static int cacheOpenRegion (GSCRegion *rp, GSCRegion *cp);
static int cacheGetNextEntry (GSCRegion *rp, GSCEntry *ep);
static void cacheCloseRegion (GSCRegion *rp);
static void cacheAddEntry (GSCRegion *rp, GSCEntry *ep);
static void cacheBuildFilename (char fullpath[], GSCRegion *rp);
static void cacheBuildDir (char fullpath[], GSCRegion *rp);
static int cacheReadHeader (GSCRegion *rp);
static void cacheWriteHeader (GSCRegion *rp);
static void cacheReadEntry (GSCRegion *rp, GSCEntry *ep);
static void cacheWriteEntry (GSCRegion *rp, GSCEntry *ep);

#define	FLL	80		/* FITS line length */

#define	FOVSTEP	degrad(1)	/* FOV scan step size, rads */

/* call this any time before using GSCFetch() to set the path to the base of
 * the GSC cdrom and the path to the base of the local disk cache.
 * if either is NULL it means don't use that facility at all.
 * return 0 if new paths look reasonable, else fill msg[] and return -1.
 * N.B. cdp and chp must remain valid for GSCFetch().
 */
int
GSCSetup (cdp, chp, msg)
char *cdp;
char *chp;
char msg[];
{
	/* set up the new paths and flags */
	cdpath = cdp;
	nocdrom = !cdpath;
	chpath = chp;
	nocache = !chpath;

	/* check whether cdpath is reasonable */
	if (cdpath) {
	    GSCRegion r;

	    /* try reading a typical north file */
	    (void) strcpy (r.dir, "n8230");
	    (void) strcpy (r.file, "4615.gsc");
	    gscOpenFile (&r, msg);
	    if (!r.fp) {
		/* nope -- try a south file */
		(void) strcpy (r.dir, "s8230");
		(void) strcpy (r.file, "9490.gsc");
		gscOpenFile (&r, msg);
		if (!r.fp)
		    return (-1);
	    }
	    gscCloseRegion (&r);
	}

#ifndef VMS
	/* can't seem to access directories under VMS */
	/* check whether chpath is reasonable -- just check the dir 
	 * since files themselves can accumulate with usage.
	 */
	if (chpath) {
	    char fullpath[1024];
	    GSCRegion r;
	    
	    (void) strcpy (r.dir, "");
	    cacheBuildDir (fullpath, &r);
	    if (existsh (fullpath) < 0) {
		(void) sprintf (msg, "GSC Cache %s:\n%s", chp, syserrstr());
		return (-1);
	    }
	}
#endif

	/* ok */
	return (0);
}

/* create or add to a malloced array of ObjF at *spp in the given region and
 *   return the new total count.
 * if spp == NULL we don't malloc anything but just do the side effects;
 * else *spp already has nspp ObjF in it (it's ok if *spp == NULL).
 * we return new total number of stars or -1 if real trouble.
 * *spp is only changed if we added any.
 * msg might contain a message regardless of the return value.
 */
int
GSCFetch (ra0, dec0, fov, fmag, spp, nspp, msg)
double ra0;	/* center RA, rads */
double dec0;	/* center Dec, rads */
double fov;	/* field of view, rads */
double fmag;	/* faintest mag */
ObjF **spp;	/* *spp will be a malloced array of ObjF stars in region */
int nspp;	/* if spp: initial number of ObjF already in *spp */
char msg[];	/* possible return error or status message */
{
	Request q;
	GSCArray sa;

	/* sanity checks */
	if (nocdrom && nocache) {
	    (void) sprintf (msg, "CDROM and Cache are both disabled");
	    return (-1);
	}
	if (!nocdrom && !cdpath) {
	    (void) sprintf (msg, "No path to CDROM");
	    return (-1);
	}
	if (!nocache && !chpath) {
	    (void) sprintf (msg, "No path to cache");
	    return (-1);
	}

	/* collect (and adjust) the criteria */
	q.ra = ra0;
	q.dec = dec0;
	q.rov = fov/2;
	q.crov = cos(q.rov);
	q.mag = fmag;

	/* compute handy trig values once */
	q.cdec = cos(q.dec);
	q.sdec = sin(q.dec);

	/* setup local access to msg and init it */
	lmsg = msg;
	lmsg[0] = '\0';

	/* init the array.
	 * max == -1 will mean we just keep a count but don't build the array.
	 */
	if (spp) {
	    sa.mem = *spp;
	    sa.used = sa.max = nspp;
	} else {
	    sa.mem = NULL;
	    sa.used = 0;
	    sa.max = -1;
	}

	/* fetch the stars */
	if (handleRequest (&q, &sa) < 0) {
	    /* beware of a partial collection -- array has likely moved */
	    if (spp && sa.mem)
		*spp = sa.mem;
	    return (-1);
	}

	/* pass back to caller if interested else just toss */
	if (spp && sa.mem)
	    *spp = sa.mem;
	else if (sa.mem)
	    free ((void *)sa.mem);

	return (sa.used);
}

/* fill GSCArray with stars which meet the given request.
 * return 0 if ok else fill lmsg[] and return -1 if trouble.
 */
static int
handleRequest (qp, ap)
Request *qp;
GSCArray *ap;
{
	char ids[9537];		/* set when region has already been visited */
	GSCRegion region;
	double dec, dec0, dec1, cosmaxrov;

	/* init the record of regions we have seen */
	zero_mem (ids, sizeof(ids));

	/* get and send the region we are directly in for sure */
	gscPickRegion (qp->ra, qp->dec, &region);
	if (fetchRegion (&region, qp, ap) < 0)
	    return (-1);
	if (region.id < 1 || region.id > sizeof(ids)) {
	    printf ("Bogus region id: %d\n", region.id);
	    abort();	/* *real* trouble! */
	}
	ids[region.id-1] = 1;

	/* scan the sphere in FOVSTEP patches looking for more close ones.
	 * duplicate regions are avoided by recording and checking ids.
	 */
	dec0 = floor((qp->dec - qp->rov)/FOVSTEP)*FOVSTEP;
	if (dec0 < -PI/2 + FOVSTEP) dec0 = -PI/2 + FOVSTEP;
	dec1 = qp->dec + qp->rov + FOVSTEP;
	if (dec1 > PI/2) dec1 = PI/2;
	cosmaxrov = cos(qp->rov + 1.415*FOVSTEP);

	for (dec = dec0; dec < dec1; dec += FOVSTEP) {
	    double cdec = cos(dec);
	    double sdec = sin(dec);
	    double ra;

	    for (ra = 0.0; ra < 2*PI; ra += FOVSTEP) {
		double cosa;

		pm_set ((int)((dec-dec0+ra/(2*PI)*FOVSTEP)/(dec1-dec0)*100));

		cosa = sdec*qp->sdec + cdec*qp->cdec*cos(ra - qp->ra);
		if (cosa < cosmaxrov)
		    continue;

		gscPickRegion (ra, dec, &region);

		/* if not already visited, fetch region and mark.
		 * N.B. regions are numbered 1-based.
		 */
		if (region.id < 1 || region.id > sizeof(ids)) {
		    printf ("Bogus scan region id: %d\n", region.id);
		    abort();	/* *real* trouble! */
		}
		if (!ids[region.id-1]) {
		    if (fetchRegion (&region, qp, ap) < 0)
			return (-1);
		    ids[region.id-1] = 1;
		}
	    }
	}

	return (0);
}

/* add stars to the GSCArray for this region and center location.
 * use cache if possible (and desired) else cdrom (if desired).
 * if no cache copy exists, create it along the way (if desired).
 * if ok return count, else put reason in lmsg and return -1.
 */
static int
fetchRegion (rp, qp, ap)
GSCRegion *rp;
Request *qp;
GSCArray *ap;
{
	GSCRegion cr;	/* used for the cache entry */

	/* first try to use the cache, if want and can. */
	if (!nocache && cacheOpenRegion (rp, &cr) == 0) {
	    int s = 0;
	    GSCEntry e;

	    while (cacheGetNextEntry (&cr, &e) == 0) {
		if (inFOV(qp,&e)==0 && magOK(qp,&e)==0) {
		    if (addOneStar (ap, &cr, &e) < 0) {
			s = -1;
			break;
		    }
		}
	    }

	    cacheCloseRegion (&cr);
	    return (s);
	}

	/* if not or can't, then the cdrom.
	 * also make the cache entry along the way, if desired.
	 */
	if (!nocdrom && gscOpenRegion (rp) == 0) {
	    int cacheok = nocache ? -1 : cacheCreateRegion (rp, &cr); /* 0=ok */
	    int lastid = -1;	/* ignore all but first of multi entries */
	    int nunique = 0;	/* count of unique entries */
	    int s = 0;
	    GSCEntry e;

	    /* read the whole CDROM but only add unique id entries */
	    while (gscGetNextEntry (rp, &e) == 0) {
		if (e.id != lastid) {
		    if (inFOV(qp,&e)==0 && magOK(qp,&e)==0) {
			if (addOneStar (ap, rp, &e) < 0) {
			    s = -1;
			    /* quit unless building cache */
			    if (cacheok < 0)
				break;
			}
		    }

		    if (cacheok == 0)
			cacheAddEntry (&cr, &e);

		    nunique++;
		    lastid = e.id;
		}
	    }

	    if (cacheok == 0) {
		fseek (cr.fp, 0L, SEEK_SET);
		cr.nrows = nunique;
		cacheWriteHeader (&cr);
		cacheCloseRegion (&cr);
	    }

	    gscCloseRegion (rp);
	    return (s);
	}

	/* if get here nothing worked */
	if (lmsg[0] == '\0')
	    (void) sprintf (lmsg, "Can not find GSC stars anywhere.");
	return (-1);
}

/* return 0 if the given entry is within the given request, else return -1.
 */
static int
inFOV (qp, ep)
Request *qp;
GSCEntry *ep;
{
	double cr = qp->sdec*sin(ep->dec) +
				    qp->cdec*cos(ep->dec)*cos(qp->ra - ep->ra);
	return (cr < qp->crov);
}

/* return 0 if the given entry is at least as bright as the faint limit, else -1 */
static int
magOK (qp, ep)
Request *qp;
GSCEntry *ep;
{
	return (ep->mag <= qp->mag ? 0 : -1);
}

/* add one GSC entry to ap[], growing if necessary.
 * if max is -1 we just count but don't actually build the array.
 * return 0 if ok, else put reason in lmsg and return -1
 */
static int
addOneStar (ap, rp, ep)
GSCArray *ap;
GSCRegion *rp;
GSCEntry *ep;
{
	int sz = sizeof(ObjF);
	Obj *op;

	if (ap->max < 0) {
	    char rstr[32], dstr[32];
	    fs_sexa (rstr, radhr(ep->ra), 2, 360000);
	    fs_sexa (dstr, raddeg(ep->dec), 4, 36000);
	    printf ("GSC %04d-%04d,f|%c,%s,%s,%g\n", rp->id, ep->id,
				ep->class ? 'T' : 'S', rstr, dstr, ep->mag);

	    ap->used++;
	    return (0);
	}

	if (ap->used >= ap->max) {
	    /* add room for NINC more */
	    char *newmem = ap->mem ? realloc((void *)ap->mem, (ap->max+NINC)*sz)
				   : malloc (NINC*sz);
	    if (!newmem) {
		(void) sprintf (lmsg, "No more memory");
		return (-1);
	    }
	    zero_mem (newmem + ap->max*sz, NINC*sz);
	    ap->mem = (ObjF *)newmem;
	    ap->max += NINC;
	}

	op = (Obj *)&ap->mem[ap->used++];

	(void) sprintf (op->o_name, "GSC %04d-%04d", rp->id, ep->id);
	op->o_type = FIXED;
	op->f_class = ep->class == 0 ? 'S' : 'T';
	op->f_RA = (float)ep->ra;
	op->f_dec = (float)ep->dec;
	op->f_epoch = (float)J2000;
	set_fmag (op, ep->mag);

	return (0);
}

static int
mymkdir (path)
char *path;
{
	return (mkdir (expand_home (path), 0755));
}


/* code to handle the details of reading the CDROM */

/* index by dec band and get the number of large regions in it and its first
 * large region number. these are for nothern bands; southern have the same
 * number of large regions but the first is the value in the table + 366.
 */
typedef struct {
    short nlrg;
    short firstlg;
} LargeRegionTable;
static LargeRegionTable lg_reg[12] = {
    {48,  1},
    {47,  49},
    {45,  96},
    {43,  141},
    {40,  184},
    {36,  224},
    {32,  260},
    {27,  292},
    {21,  319},
    {15,  340},
    {9,	  355},
    {3,	  364},
};

/* index by large region number (which start at 1, not 0) and get its first
 * small region number and the number of ra and dec subdivisions.
 */
typedef struct {
    short firstsm;
    short nsub;
} SmallRegionTable;
static SmallRegionTable sm_reg[733] = {
    {0,    0}, {1,    3}, {10,   3}, {19,   3}, {28,   3}, {37,   3}, {46,   3},
    {55,   3}, {64,   3}, {73,   3}, {82,   4}, {98,   4}, {114,  4}, {130,  4},
    {146,  4}, {162,  4}, {178,  4}, {194,  4}, {210,  4}, {226,  3}, {235,  3},
    {244,  3}, {253,  3}, {262,  3}, {271,  3}, {280,  3}, {289,  3}, {298,  3},
    {307,  3}, {316,  3}, {325,  3}, {334,  4}, {350,  4}, {366,  4}, {382,  4},
    {398,  4}, {414,  4}, {430,  4}, {446,  4}, {462,  4}, {478,  4}, {494,  4},
    {510,  4}, {526,  4}, {542,  4}, {558,  3}, {567,  3}, {576,  3}, {585,  3},
    {594,  3}, {603,  3}, {612,  3}, {621,  3}, {630,  3}, {639,  3}, {648,  3},
    {657,  3}, {666,  4}, {682,  4}, {698,  4}, {714,  4}, {730,  4}, {746,  4},
    {762,  4}, {778,  4}, {794,  4}, {810,  3}, {819,  3}, {828,  3}, {837,  3},
    {846,  3}, {855,  3}, {864,  3}, {873,  3}, {882,  3}, {891,  3}, {900,  3},
    {909,  3}, {918,  3}, {927,  4}, {943,  4}, {959,  4}, {975,  4}, {991,  4},
    {1007, 4}, {1023, 4}, {1039, 4}, {1055, 4}, {1071, 4}, {1087, 4}, {1103, 4},
    {1119, 4}, {1135, 4}, {1151, 3}, {1160, 3}, {1169, 3}, {1178, 3}, {1187, 3},
    {1196, 3}, {1205, 3}, {1214, 3}, {1223, 3}, {1232, 4}, {1248, 4}, {1264, 4},
    {1280, 4}, {1296, 4}, {1312, 4}, {1328, 4}, {1344, 4}, {1360, 4}, {1376, 4},
    {1392, 3}, {1401, 3}, {1410, 3}, {1419, 3}, {1428, 3}, {1437, 2}, {1441, 2},
    {1445, 2}, {1449, 3}, {1458, 3}, {1467, 3}, {1476, 3}, {1485, 3}, {1494, 3},
    {1503, 4}, {1519, 4}, {1535, 4}, {1551, 4}, {1567, 4}, {1583, 4}, {1599, 4},
    {1615, 4}, {1631, 4}, {1647, 4}, {1663, 4}, {1679, 4}, {1695, 4}, {1711, 3},
    {1720, 3}, {1729, 3}, {1738, 3}, {1747, 3}, {1756, 3}, {1765, 4}, {1781, 4},
    {1797, 4}, {1813, 4}, {1829, 4}, {1845, 4}, {1861, 4}, {1877, 4}, {1893, 4},
    {1909, 4}, {1925, 4}, {1941, 3}, {1950, 3}, {1959, 3}, {1968, 3}, {1977, 2},
    {1981, 2}, {1985, 2}, {1989, 2}, {1993, 2}, {1997, 3}, {2006, 3}, {2015, 3},
    {2024, 3}, {2033, 3}, {2042, 4}, {2058, 4}, {2074, 4}, {2090, 4}, {2106, 4},
    {2122, 4}, {2138, 4}, {2154, 4}, {2170, 4}, {2186, 4}, {2202, 4}, {2218, 4},
    {2234, 4}, {2250, 3}, {2259, 4}, {2275, 4}, {2291, 4}, {2307, 4}, {2323, 4},
    {2339, 4}, {2355, 4}, {2371, 4}, {2387, 4}, {2403, 4}, {2419, 4}, {2435, 4},
    {2451, 4}, {2467, 4}, {2483, 3}, {2492, 3}, {2501, 3}, {2510, 3}, {2519, 2},
    {2523, 2}, {2527, 2}, {2531, 2}, {2535, 3}, {2544, 3}, {2553, 3}, {2562, 3},
    {2571, 3}, {2580, 3}, {2589, 4}, {2605, 4}, {2621, 4}, {2637, 4}, {2653, 4},
    {2669, 4}, {2685, 4}, {2701, 4}, {2717, 4}, {2733, 4}, {2749, 4}, {2765, 4},
    {2781, 4}, {2797, 4}, {2813, 4}, {2829, 4}, {2845, 4}, {2861, 4}, {2877, 4},
    {2893, 4}, {2909, 4}, {2925, 4}, {2941, 4}, {2957, 4}, {2973, 3}, {2982, 3},
    {2991, 3}, {3000, 3}, {3009, 2}, {3013, 2}, {3017, 2}, {3021, 2}, {3025, 3},
    {3034, 3}, {3043, 3}, {3052, 3}, {3061, 3}, {3070, 4}, {3086, 4}, {3102, 4},
    {3118, 4}, {3134, 4}, {3150, 4}, {3166, 4}, {3182, 4}, {3198, 4}, {3214, 4},
    {3230, 4}, {3246, 4}, {3262, 4}, {3278, 4}, {3294, 4}, {3310, 4}, {3326, 4},
    {3342, 4}, {3358, 4}, {3374, 4}, {3390, 4}, {3406, 3}, {3415, 3}, {3424, 3},
    {3433, 3}, {3442, 3}, {3451, 2}, {3455, 2}, {3459, 2}, {3463, 3}, {3472, 3},
    {3481, 3}, {3490, 3}, {3499, 3}, {3508, 4}, {3524, 4}, {3540, 4}, {3556, 4},
    {3572, 4}, {3588, 4}, {3604, 4}, {3620, 4}, {3636, 4}, {3652, 4}, {3668, 4},
    {3684, 4}, {3700, 4}, {3716, 4}, {3732, 4}, {3748, 4}, {3764, 4}, {3780, 4},
    {3796, 3}, {3805, 3}, {3814, 3}, {3823, 3}, {3832, 3}, {3841, 3}, {3850, 3},
    {3859, 3}, {3868, 3}, {3877, 3}, {3886, 4}, {3902, 4}, {3918, 4}, {3934, 4},
    {3950, 4}, {3966, 4}, {3982, 4}, {3998, 4}, {4014, 4}, {4030, 4}, {4046, 4},
    {4062, 4}, {4078, 4}, {4094, 4}, {4110, 4}, {4126, 3}, {4135, 3}, {4144, 3},
    {4153, 3}, {4162, 3}, {4171, 3}, {4180, 3}, {4189, 3}, {4198, 4}, {4214, 4},
    {4230, 4}, {4246, 4}, {4262, 4}, {4278, 4}, {4294, 4}, {4310, 4}, {4326, 4},
    {4342, 4}, {4358, 4}, {4374, 3}, {4383, 3}, {4392, 3}, {4401, 3}, {4410, 3},
    {4419, 3}, {4428, 4}, {4444, 4}, {4460, 4}, {4476, 4}, {4492, 4}, {4508, 4},
    {4524, 4}, {4540, 3}, {4549, 3}, {4558, 3}, {4567, 4}, {4583, 4}, {4599, 4},
    {4615, 4}, {4631, 4}, {4647, 4}, {4663, 3}, {4672, 3}, {4681, 2}, {4685, 2},
    {4689, 3}, {4698, 3}, {4707, 3}, {4716, 3}, {4725, 3}, {4734, 4}, {4750, 4},
    {4766, 4}, {4782, 4}, {4798, 4}, {4814, 4}, {4830, 4}, {4846, 4}, {4862, 4},
    {4878, 4}, {4894, 3}, {4903, 3}, {4912, 3}, {4921, 3}, {4930, 3}, {4939, 3},
    {4948, 3}, {4957, 3}, {4966, 3}, {4975, 3}, {4984, 4}, {5000, 4}, {5016, 4},
    {5032, 4}, {5048, 4}, {5064, 4}, {5080, 4}, {5096, 4}, {5112, 4}, {5128, 4},
    {5144, 4}, {5160, 4}, {5176, 4}, {5192, 4}, {5208, 4}, {5224, 3}, {5233, 3},
    {5242, 3}, {5251, 3}, {5260, 3}, {5269, 2}, {5273, 2}, {5277, 2}, {5281, 2},
    {5285, 3}, {5294, 3}, {5303, 3}, {5312, 3}, {5321, 3}, {5330, 4}, {5346, 4},
    {5362, 4}, {5378, 4}, {5394, 4}, {5410, 4}, {5426, 4}, {5442, 4}, {5458, 4},
    {5474, 4}, {5490, 3}, {5499, 3}, {5508, 3}, {5517, 3}, {5526, 3}, {5535, 3},
    {5544, 3}, {5553, 3}, {5562, 4}, {5578, 4}, {5594, 4}, {5610, 4}, {5626, 4},
    {5642, 4}, {5658, 4}, {5674, 4}, {5690, 4}, {5706, 4}, {5722, 4}, {5738, 4},
    {5754, 4}, {5770, 4}, {5786, 4}, {5802, 3}, {5811, 3}, {5820, 3}, {5829, 3},
    {5838, 3}, {5847, 2}, {5851, 2}, {5855, 2}, {5859, 2}, {5863, 3}, {5872, 3},
    {5881, 3}, {5890, 3}, {5899, 4}, {5915, 4}, {5931, 4}, {5947, 4}, {5963, 4},
    {5979, 4}, {5995, 4}, {6011, 4}, {6027, 4}, {6043, 4}, {6059, 4}, {6075, 3},
    {6084, 3}, {6093, 3}, {6102, 3}, {6111, 3}, {6120, 4}, {6136, 4}, {6152, 4},
    {6168, 4}, {6184, 4}, {6200, 4}, {6216, 4}, {6232, 4}, {6248, 4}, {6264, 4},
    {6280, 4}, {6296, 4}, {6312, 4}, {6328, 4}, {6344, 4}, {6360, 4}, {6376, 3},
    {6385, 3}, {6394, 3}, {6403, 3}, {6412, 3}, {6421, 2}, {6425, 2}, {6429, 2},
    {6433, 2}, {6437, 3}, {6446, 3}, {6455, 3}, {6464, 3}, {6473, 4}, {6489, 4},
    {6505, 4}, {6521, 4}, {6537, 4}, {6553, 4}, {6569, 4}, {6585, 4}, {6601, 4},
    {6617, 4}, {6633, 4}, {6649, 4}, {6665, 4}, {6681, 4}, {6697, 4}, {6713, 4},
    {6729, 4}, {6745, 4}, {6761, 4}, {6777, 4}, {6793, 4}, {6809, 4}, {6825, 4},
    {6841, 4}, {6857, 4}, {6873, 4}, {6889, 4}, {6905, 4}, {6921, 4}, {6937, 4},
    {6953, 3}, {6962, 3}, {6971, 3}, {6980, 3}, {6989, 3}, {6998, 2}, {7002, 2},
    {7006, 2}, {7010, 3}, {7019, 3}, {7028, 3}, {7037, 3}, {7046, 3}, {7055, 4},
    {7071, 4}, {7087, 4}, {7103, 4}, {7119, 4}, {7135, 4}, {7151, 4}, {7167, 4},
    {7183, 4}, {7199, 4}, {7215, 4}, {7231, 4}, {7247, 4}, {7263, 4}, {7279, 4},
    {7295, 4}, {7311, 4}, {7327, 4}, {7343, 4}, {7359, 4}, {7375, 4}, {7391, 4},
    {7407, 4}, {7423, 4}, {7439, 4}, {7455, 4}, {7471, 4}, {7487, 3}, {7496, 3},
    {7505, 3}, {7514, 3}, {7523, 3}, {7532, 3}, {7541, 3}, {7550, 3}, {7559, 3},
    {7568, 3}, {7577, 3}, {7586, 3}, {7595, 4}, {7611, 4}, {7627, 4}, {7643, 4},
    {7659, 4}, {7675, 4}, {7691, 4}, {7707, 4}, {7723, 4}, {7739, 4}, {7755, 4},
    {7771, 4}, {7787, 4}, {7803, 4}, {7819, 4}, {7835, 4}, {7851, 4}, {7867, 4},
    {7883, 4}, {7899, 4}, {7915, 4}, {7931, 4}, {7947, 4}, {7963, 4}, {7979, 4},
    {7995, 3}, {8004, 3}, {8013, 3}, {8022, 3}, {8031, 3}, {8040, 3}, {8049, 3},
    {8058, 3}, {8067, 3}, {8076, 3}, {8085, 4}, {8101, 4}, {8117, 4}, {8133, 4},
    {8149, 4}, {8165, 4}, {8181, 4}, {8197, 4}, {8213, 4}, {8229, 4}, {8245, 4},
    {8261, 4}, {8277, 4}, {8293, 4}, {8309, 4}, {8325, 4}, {8341, 4}, {8357, 4},
    {8373, 4}, {8389, 4}, {8405, 4}, {8421, 4}, {8437, 3}, {8446, 3}, {8455, 3},
    {8464, 3}, {8473, 3}, {8482, 3}, {8491, 3}, {8500, 3}, {8509, 3}, {8518, 4},
    {8534, 4}, {8550, 4}, {8566, 4}, {8582, 4}, {8598, 4}, {8614, 4}, {8630, 4},
    {8646, 4}, {8662, 4}, {8678, 4}, {8694, 4}, {8710, 4}, {8726, 4}, {8742, 4},
    {8758, 4}, {8774, 4}, {8790, 4}, {8806, 4}, {8822, 3}, {8831, 3}, {8840, 3},
    {8849, 3}, {8858, 3}, {8867, 3}, {8876, 4}, {8892, 4}, {8908, 4}, {8924, 4},
    {8940, 4}, {8956, 4}, {8972, 4}, {8988, 4}, {9004, 4}, {9020, 4}, {9036, 4},
    {9052, 4}, {9068, 4}, {9084, 4}, {9100, 4}, {9116, 3}, {9125, 3}, {9134, 3},
    {9143, 3}, {9152, 3}, {9161, 4}, {9177, 4}, {9193, 4}, {9209, 4}, {9225, 4},
    {9241, 4}, {9257, 4}, {9273, 4}, {9289, 4}, {9305, 4}, {9321, 4}, {9337, 3},
    {9346, 4}, {9362, 4}, {9378, 4}, {9394, 4}, {9410, 4}, {9426, 4}, {9442, 4},
    {9458, 4}, {9474, 4}, {9490, 4}, {9506, 4}, {9522, 4},
};

/* given an ra and dec, each in rads fill in the *rp GSCRegion with the name of
 *   the file (relative to cdpath), n/s hemisphere, region id and the
 *   origin ra/dec of the region (also in rads).
 * This is all based upon the comments found on the CD-ROM files in tables/
 *   sm_reg_x.tbl, lg_reg_x.tbl and regions.tbl.
 */
static void
gscPickRegion (ra, dec, rp)
double ra;
double dec;
GSCRegion *rp;
{
	int decband;	/* which 7.5-degree dec band: 0..11 */
	int raband;	/* which group of small ra bands */
	int lgregno;	/* large region number */
	int nlrg;	/* n large regions in this dec band */
	int firstsm;	/* first small region number in this large region */
	int nsub;	/* n subdivisions in each dim in this lrge regn */
	int nsmdec;	/* n small regions forward in dec */
	int nsmra;	/* n small regions forward in ra */
	volatile double raLrg;	/* fractions of large RA regions to target */
	volatile double decLrg;	/* fractios of large Dec regions to target */
			/* volatile fixes gcc -O2 */

	/* convert to degs for remainder of algorithm */
	ra = raddeg(ra);
	dec = raddeg(dec);

	/* insure -90 < dec < 90 -- need not be perfect but avoids close
	 * calls with all the modulo arithmetic.
	 */
#define	CLOSEPOLE	(90.0 - 10.0/3600.0)
	if (dec <= -CLOSEPOLE)
	    dec = -CLOSEPOLE;
	else if (dec >= CLOSEPOLE)
	    dec = CLOSEPOLE;
#undef CLOSEPOLE

	/* insure 0 <= ra < 360 */
	range (&ra, 360.0);

	/* work effectively in "modulo nothern hemisphere" and start dir[].
	 */
	if (dec < 0.0) {
	    rp->south = 1;
	    dec = -dec;
	    rp->dir[0] = 's';
	} else {
	    rp->south = 0;
	    rp->dir[0] = 'n';
	}

	/* find which 7.5-degree dec band and add more to dir */
	decLrg = dec/7.5;
	decband = (int)floor(decLrg);
	(void) sprintf (rp->dir+1, "%04d", decband*750);
	if (rp->dir[3] == '5')
	    rp->dir[3] = '3';

	/* find the large region number */
	nlrg = lg_reg[decband].nlrg;
	raLrg = ra*nlrg/360.0;
	raband = (int)floor(raLrg);
	lgregno = lg_reg[decband].firstlg + raband;
	if (rp->south)
	    lgregno += 366;

	/* lookup the number of subdivisions */
	firstsm = sm_reg[lgregno].firstsm;
	nsub = sm_reg[lgregno].nsub;

	/* find the subdivision indices and origin */
        nsmdec = (int)(nsub*(decLrg-decband));
	rp->dec0 = 7.5 * (decband + (double)nsmdec/nsub);
	rp->dec0 = degrad(rp->dec0);
	nsmra = (int)(nsub*(raLrg-raband));
	rp->ra0 = (360.0/nlrg) * (raband + (double)nsmra/nsub);
	rp->ra0 = degrad(rp->ra0);

	/* small region number is then the first in this large region plus
	 * the number of subdivisions away from the origin.
	 * the large region is broken up into a square array of nsub*nsub.
	 */
	rp->id = firstsm + nsub*nsmdec + nsmra;

	/* fill in the filename */
	(void) sprintf (rp->file, "%04d.gsc", rp->id);
}

/* open and setup rp->fp and nrows from an existing cdrom file off cdpath for
 *   the given region so it is ready to return successive entries.
 * return 0 if ok, else write error message to lmsg and return -1.
 * N.B. this code heavily assumes these the GSC FITS files.
 */
static int
gscOpenRegion (rp)
GSCRegion *rp;
{
	typedef enum {FINDNAXIS2, FINDEND, SKIPTOBLOCK, FOUNDTABLE} State;
	char buf[FLL+1];	/* we add EOS */
	char fn[1024];
	State state;
	int n;

	/* open the cdrom file */
	gscOpenFile (rp, fn);
	if (!rp->fp) {
	    (void) strcpy (lmsg, fn);
	    return (-1);
	}

	/* make sure it's a SIMPLE FITS file at least */
	if (gscSimpleFits (rp->fp) < 0) {
	    (void) sprintf (lmsg, "%s:\nnot a SIMPLE FITS file", fn);
	    fclose (rp->fp);
	    rp->fp = NULL;
	    return (-1);
	}

	/* find table length in NAXIS2.
	 * position rp->fp at first row of table.
	 * N.B. start n at 2 so after the fread it is count of FLL records read.
	 */
	for (n = 2, state = FINDNAXIS2; state != FOUNDTABLE; n++) {
	    if (fread (buf, 1, FLL, rp->fp) != FLL) {
		(void) sprintf (lmsg, "%s:\nunexpected EOF", fn);
		fclose (rp->fp);
		rp->fp = NULL;
		return (-1);
	    }
	    buf[FLL] = '\0';

	    switch (state) {
	    case FINDNAXIS2:
		if (sscanf (buf, "NAXIS2  =%d", &rp->nrows) == 1)
		    state = FINDEND;
		break;
	    case FINDEND:
		if (strncmp (buf, "END", 3) == 0) {
		    if (((n*FLL)%2880) == 0)
			state = FOUNDTABLE;
		    else
			state = SKIPTOBLOCK;
		}
		break;
	    case SKIPTOBLOCK:
		if (((n*FLL)%2880) == 0)
		    state = FOUNDTABLE;
		break;
	    case FOUNDTABLE:
		break;
	    }

	}

	return (0);
}

/* at least see that fp starts as a SIMPLE FITS file.
 * if ok return 0 with fp positioned at the next FLL record,
 * else return -1.
 */
static int
gscSimpleFits (fp)
FILE *fp;
{
	static char smpl[31] = "SIMPLE  =                    T";
	char buf[FLL];

	if (fread (buf, 1, FLL, fp) != FLL)
	    return (-1);
	if (strncmp (buf, smpl, sizeof(smpl)-1))
	    return (-1);
	return (0);
}

/* open the file for the given region.
 * if succcessful, set rp->fp and fill msg with what opened,
 * else set rp->fp to NULL and full msg with an error message.
 */
static void
gscOpenFile (rp, msg)
GSCRegion *rp;
char *msg;
{
	char path[1024];

	(void) sprintf (path, "%s/gsc/%s/%s", cdpath, rp->dir, rp->file);
	rp->fp = fopenh (path, "rb");

	if (!rp->fp) {
	    /* try it with a trailing ";1" */
	    (void) strcat (path, ";1");
	    rp->fp = fopenh (path, "rb");
	}

	if (!rp->fp)
	    (void) sprintf (msg, "No GSC files in %s", cdpath);
}

/* return the next entry from the GSC region.
 * decrement rp->nrows as we go and return -1 when it reaches 0, else 0.
 */
static int
gscGetNextEntry (rp, ep)
GSCRegion *rp;
GSCEntry *ep;
{
	char buf[16];

	/* can't use scanf because we are breaking on columns not whitespace
	 * so just read in in pieces as we go.
	 * I5	ID within region
	 * F9	RA, degrees
	 * F9	Dec, degrees
	 * F5	position error, arc seconds
	 * F5	magnitude
	 * F4	magnitude error
	 * I2	magnitude band
	 * I1	classification
	 * A4	internal plate number
	 * A1	T/F whether there are more
	 */

	if (rp->nrows <= 0)
	    return (-1);

	(void) fread (buf, 1, 5, rp->fp);	/* ID */
	buf[5] = '\0';
	ep->id = atoi (buf);

	(void) fread (buf, 1, 9, rp->fp);	/* RA */
	buf[9] = '\0';
	(void) sscanf (buf, "%lf", &ep->ra);
	ep->ra = degrad(ep->ra);

	(void) fread (buf, 1, 9, rp->fp);	/* Dec */
	buf[9] = '\0';
	(void) sscanf (buf, "%lf", &ep->dec);
	ep->dec = degrad(ep->dec);

	(void) fread (buf, 1, 5, rp->fp);	/* poserr (discarded) */

	(void) fread (buf, 1, 5, rp->fp);	/* mag */
	buf[5] = '\0';
	(void) sscanf (buf, "%lf", &ep->mag);

	(void) fread (buf, 1, 4, rp->fp);	/* mag error (discarded) */

	(void) fread (buf, 1, 2, rp->fp);	/* mag band (discarded) */

	(void) fread (buf, 1, 1, rp->fp);	/* class */
	buf[1] = '\0';
	ep->class = atoi (buf);

	(void) fread (buf, 1, 4, rp->fp);	/* plate (discarded)*/
	(void) fread (buf, 1, 1, rp->fp);	/* multiflag (discarded)*/

	if (feof(rp->fp) || ferror(rp->fp)) {
	    fprintf (stderr, "%s:\n%d stars short\n", rp->file, rp->nrows);
	    return(-1);
	}

	rp->nrows -= 1;

	return (0);
}

/* do whatever cleanup might be required when finished with rp
 */
static void
gscCloseRegion (rp)
GSCRegion *rp;
{
	fclose (rp->fp);
	rp->fp = NULL;
}


/* code to handle the details of the GSC cache files.
 * the files have a short header at the front in ASCII.
 * the stars then follow in a packed binary format.
 * the packed format stores locations to within about .2 arcseconds.
 * see the cacheReadEntry() function comments for format details.
 */

#define	RAERR		(1e-6)		/* allowed ra slop, rads */
#define	DECERR		(1e-6)		/* allowed dec slop, rads */
#define	MAGX		900.0		/* magnitude scale */

#undef LS
#undef RS		/* already exists on HP's */
#define	LS(v,n)		(((unsigned)(v)) << (n))
#define	RS(v,n)	((unsigned char)(0xff & ((n)==0?(v):(((unsigned)(v)) >> (n)))))


/* open and setup cp->fp, nrows and cosdec0 from an existing cache file off
 *   *chpath for the given region so it is ready to return successive entries.
 * the other header fields should match those in rp.
 * write any errs to lmsg but only if nocdrom since then this is likely just a
 *   first try.
 * return 0 if ok else return -1.
 */
static int
cacheOpenRegion (rp, cp)
GSCRegion *rp;
GSCRegion *cp;
{
	char fullpath[1024];

	/* open the cache file */
	cacheBuildFilename (fullpath, rp);
	cp->fp = fopenh (fullpath, "rb");
	if (!cp->fp) {
	    if (nocdrom)
		(void) sprintf (lmsg, "%s:\n%s", fullpath, syserrstr());
	    return (-1);
	}

	/* read the header */
	if (cacheReadHeader (cp) < 0) {
	    fclose (cp->fp);
	    cp->fp = NULL;
	    return (-1);
	}

	/* header must agree with rp.
	 * N.B. we don't know rp->nrows because that only gets set if we
	 *   can open the cdrom file -- but then we wouldn't be here :-)
	 */
	if (rp->south != cp->south || rp->id != cp->id
					|| fabs(rp->ra0 - cp->ra0) > RAERR
					|| fabs(rp->dec0 - cp->dec0) > DECERR) {
	    (void) sprintf (lmsg, "%s:\nheader mismatch", fullpath);
	    fprintf (stderr, "%s:\nheader mismatch\n", fullpath);
	    fprintf (stderr, "  %d %d %g %g %d\n", 
			    cp->id, cp->nrows, cp->ra0, cp->dec0, cp->south);
	    fprintf (stderr, "  %d ?? %g %g %d\n",
					rp->id, rp->ra0, rp->dec0, rp->south);
	    fclose (cp->fp);
	    cp->fp = NULL;
	    return (-1);
	}

	cp->cosdec0 = cos(cp->dec0);

	return (0);
}

/* return the next entry from the cache region.
 * decrement rp->nrows as we go and return -1 when it reaches 0, else 0.
 */
static int
cacheGetNextEntry (rp, ep)
GSCRegion *rp;
GSCEntry *ep;
{
	if (rp->nrows <= 0)
	    return (-1);
	cacheReadEntry (rp, ep);
	rp->nrows -= 1;
	return (0);
}

 
/* do whatever cleanup might be required when finished with rp.
 */
static void
cacheCloseRegion (rp)
GSCRegion *rp;
{
	fclose (rp->fp);
	rp->fp = NULL;
}

/* try to create a new cache file for region *rp off the chpath dir.
 * make a new descriptor for it at *cp.
 * return 0 if ok, else complain to lmsg and return -1.
 */
static int
cacheCreateRegion (rp, cp)
GSCRegion *rp;
GSCRegion *cp;
{
	char fullpath[1024];

	/* copy most fields from rp */
	*cp = *rp;

	/* create a new file, and directory too if necessary */
	cacheBuildFilename (fullpath, rp);
	cp->fp = fopenh (fullpath, "wb");
	if (!cp->fp) {
	    if (errno == ENOENT) {
		/* failed because of nonexistent dir component */
		char fulldir[1024];

		cacheBuildDir (fulldir, rp);
		if (mymkdir (fulldir) < 0) {
		    (void) sprintf (lmsg, "mkdir(%s): %s", fulldir,syserrstr());
		    return (-1);
		}
		cp->fp = fopenh (fullpath, "wb");
	    } 

	    if (!cp->fp) {
		(void) sprintf (lmsg, "create(%s): %s", fullpath, syserrstr());
		return (-1);
	    }
	}

	/* set up the cosdec0 shortcut */
	cp->cosdec0 = cos(cp->dec0);

	/* write header onto the new cache file */
	cacheWriteHeader (cp);

	return (0);
}

/* write ep to rp->fp.
 */
static void
cacheAddEntry (rp, ep)
GSCRegion *rp;
GSCEntry *ep;
{
	cacheWriteEntry (rp, ep);
}

/* build a full cache filename for rp off chpath dir.
 */
static void
cacheBuildFilename (fullpath, rp)
char fullpath[];
GSCRegion *rp;
{
	(void) sprintf (fullpath, "%s/%s/%04d.ech", chpath, rp->dir, rp->id);
}

/* build a full cache dir for rp off chpath dir.
 */
static void
cacheBuildDir (fullpath, rp)
char fullpath[];
GSCRegion *rp;
{
	(void) sprintf (fullpath, "%s/%s", chpath, rp->dir);
}

/* read a cache file header and fill in rp.
 * leave rp->fp positioned at first star.
 */
static int
cacheReadHeader (rp)
GSCRegion *rp;
{
	char hdr[128];
	double v;
	int c, i;

	/* read up to first nl */
	for (i=0; (c=getc(rp->fp)) != EOF && c != '\n' && i < sizeof(hdr); i++)
	    hdr[i] = (char)c;
	if (i == sizeof(hdr)) {
	    char fullpath[1024];
	    cacheBuildFilename (fullpath, rp);
	    hdr[sizeof(hdr)-1] = '\0';
	    (void) sprintf (lmsg, "%s:\nno header: %s", fullpath, hdr);
	    return (-1);
	}

	/* crack it */
	hdr[i] = '\0';
	i = sscanf (hdr, "%lf %d %d %lf %lf %d\n", &v, &rp->id, &rp->nrows,
					&rp->ra0, &rp->dec0, &rp->south);
	if (i != 6) {
	    char fullpath[128];
	    cacheBuildFilename (fullpath, rp);
	    (void) sprintf (lmsg, "%s:\nbad header: %s", fullpath, hdr);
	    return (-1);
	}

	if (v > cacheversion) {
	    char fullpath[128];
	    cacheBuildFilename (fullpath, rp);
	    (void) sprintf (lmsg, "%s:\nunsupported version: %g", fullpath, v);
	    return (-1);
	}

	return (0);
}

/* write a cache file header from rp.
 * N.B. must be same length every time because we are called to overright.
 */
static void
cacheWriteHeader (rp)
GSCRegion *rp;
{
	char hdr[128];
	int n;

	/* format the header */
	(void) sprintf (hdr, "%4.1f %4d %5d %9.7f %9.7f %1d\n",
		cacheversion, rp->id, rp->nrows, rp->ra0, rp->dec0, rp->south);
	n = strlen (hdr);

	/* write it */
	fwrite (hdr, 1, n, rp->fp);
}

/* read a cache file entry 
 *   0  2  delta ra, rads, 0 .. degrad(4) * cos(dec0), scaled 0 .. 0xffff
 *   2  2  delta dec, rads, 0 .. degrad(4), scaled 0 .. 0xffff
 *   4  2  small region id
 *   6  2  upper 2 bits are class, lower 14 are magnitude*MAGX
 *   8     total bytes per entry
 *  all values stored in big-endian byte order.
 */
static void
cacheReadEntry (rp, ep)
GSCRegion *rp;
GSCEntry *ep;
{
	FILE *fp = rp->fp;
	unsigned char buf[2];
	unsigned i;

	(void) fread (buf, 1, 2, fp);
	i = LS(buf[0],8) | LS(buf[1],0);
	ep->ra = rp->ra0 + (double)i/(double)0xffff*degrad(4)/rp->cosdec0;

	(void) fread (buf, 1, 2, fp);
	i = LS(buf[0],8) | LS(buf[1],0);
	ep->dec = rp->dec0 + (double)i/(double)0xffff*degrad(4);
	if (rp->south)
	    ep->dec = -ep->dec;

	(void) fread (buf, 1, 2, fp);
	ep->id = LS(buf[0],8) | LS(buf[1],0);

	(void) fread (buf, 1, 2, fp);
	i = LS(buf[0],8) | LS(buf[1],0);
	ep->class = (i>>14)&0x3;
	ep->mag = (i&0x3fff)/MAGX;
}

/* write a cache file entry 
 *   0  2  delta ra, rads, 0 .. degrad(4) * cos(dec0), scaled 0 .. 0xffff
 *   2  2  delta dec, rads, 0 .. degrad(4), scaled 0 .. 0xffff
 *   4  2  small region id
 *   6  2  upper 2 bits are class, lower 14 are magnitude*MAGX
 *   8     total bytes per entry
 *  all values stored in big-endian byte order.
 */
static void
cacheWriteEntry (rp, ep)
GSCRegion *rp;
GSCEntry *ep;
{
	FILE *fp = rp->fp;
	unsigned char buf[2];
	unsigned i;
	double d, a;

	d = (ep->ra - rp->ra0)*(double)0xffff/degrad(4)*rp->cosdec0;
	i = (unsigned)floor(d + 0.5);
	buf[0] = RS(i,8);
	buf[1] = RS(i,0);
	fwrite (buf, 1, 2, fp);

	d = rp->south ? -ep->dec : ep->dec;
	a = (d - rp->dec0)*(double)0xffff/degrad(4);
	i = (unsigned)floor(a + 0.5);
	buf[0] = RS(i,8);
	buf[1] = RS(i,0);
	fwrite (buf, 1, 2, fp);

	i = ep->id;
	buf[0] = RS(i,8);
	buf[1] = RS(i,0);
	fwrite (buf, 1, 2, fp);

	i = (unsigned)floor(ep->mag*MAGX + 0.5) | (ep->class << 14);
	buf[0] = RS(i,8);
	buf[1] = RS(i,0);
	fwrite (buf, 1, 2, fp);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: gsc.c,v $ $Date: 2004/05/14 02:14:28 $ $Revision: 1.11 $ $Name:  $"};

/* code to gather ppm and gsc stars in a region
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef const void *qsort_arg; 

#include "xephem.h"

#define	DUPSEP		degrad(4/3600.)	/* dup if < this close, rads */
#define	DUPMAG		(3.0)		/* dup if mags differ < this much */

static int objfd_cmp (qsort_arg v1, qsort_arg v2);
static int fs_nodbdups (ObjF *fop, int nfop, double ra, double dec,
    double fov);
static int scanchk (Obj *mop, Obj *op, double cdec);

static char ppm_fn[1024];
static char gsc_fn[1024];


/* establish the file paths off basedir.
 * return 0 if ok, else -1 and excuse in msg[]
 */
int
fs_setup (char basedir[], char msg[])
{
	(void) sprintf (ppm_fn, "%s/ppm.xe2", basedir);
	if (xe2chkfile (ppm_fn, msg) < 0)
	    return (-1);
	(void) sprintf (gsc_fn, "%s/gsc", basedir);
	if (GSCSetup (NULL, gsc_fn, msg) < 0)
	    return (-1);
	return (0);
}

/* call to fetch a set of ObjF from the field star sources.
 * we eliminate any that seem to be dups of existing FIXED db objects.
 * each o_flag in each resulting star is marked with FLDSTAR.
 * return -1 if something looks busted else count.
 * N.B. we only malloc *opp if we return > 0.
 */
int
fsfetch (np, want_ppm, want_gsc, ra, dec, fov, mag, opp)
Now *np;	/* now */
int want_ppm;   /* whether to fetch ppm stars */
int want_gsc;   /* whether to fetch gsc stars */
double ra, dec;	/* at np->n_epoch */
double fov;	/* field of view, rads */
double mag;	/* limiting mag */
ObjF **opp;	/* we set *opp to a malloced list of ObjF */
{
	ObjF *op = NULL;
	int nop = 0;
	int s = 0;
	char msg[256];
	int i;

	/* the fetch tools all want and return J2000 values */
	if (epoch != J2000)
	    precess (epoch == EOD ? mjd : epoch, J2000, &ra, &dec);

	/* get up to one PM catalog */
	if (want_ppm)
	    nop = xe2fetch (ppm_fn, np, ra, dec, fov, mag, &op, msg);

	/* add the GSC, if desired */
	if (want_gsc)
	    nop = GSCFetch (ra, dec, fov, mag, &op, nop, msg);	/* adds */

	/* set the s_fields to np */
	for (i = 0; i < nop; i++)
	    obj_cir (np, (Obj *)&op[i]);

	/* squeeze out stars which duplicate each other or the main database.
	 */
	if (nop > 0) {
	    int newnop = fs_nodbdups (op, nop, ra, dec, fov);
	    if (newnop < nop) {
		/* shrink down to just newnop entries now */
		nop = newnop;
		if (nop > 0)
		    op = (ObjF *) realloc ((void *)op, nop*sizeof(ObjF));
		else {
		    free ((void *)op);	/* *all* are dups! */
		    op = NULL;
		    nop = 0;
		}
	    }
	} else {
	    /* still set the FLDSTAR flag on all entries */
	    for (i = 0; i < nop; i++)
		((Obj *)(&op[i]))->o_flags |= FLDSTAR;
	}

	/* pass back the result, if there's something left */
	if (nop > 0) {
	    *opp = op;
	    s = nop;
	} else if (nop < 0) {
	    s = -1;
	}

	return (s);
}

/* qsort-style comparison of two ObjF's by dec */
static int
objfd_cmp (v1, v2)
qsort_arg v1, v2;
{
	Obj *op1 = (Obj *)v1;
	Obj *op2 = (Obj *)v2;
	double d = op1->s_dec - op2->s_dec;

	if (d < 0.0)
	    return (-1);
	if (d > 0.0)
	    return (1);
	return (0);
}

/* squeeze out all entries in fop[] which are located within DUPSEP and have a
 *   magnitude within DUPMAG of any FIXED object currently in the database. We
 *   also squeeze out any GSC or which meet those same criteria
 *   for any PM star.
 * when finished, all remaining entries will be contiguous at the front of the
 *   fop array and each will have FLDSTAR set in o_flag.
 * return the number of objects which remain.
 * N.B. we assume FLDSTAR bit is initially off in each o_flag.
 * N.B. we assume all fop[] have already been obj_cir()'d to mm_get_now().
 * N.B. we assume all entries in fop[] are within fov of ra/dec.
 */
static int
fs_nodbdups (fop, nfop, ra, dec, fov)
ObjF *fop;
int nfop;
double ra, dec, fov;
{
	double rov = fov/2;
	Obj *op;
	DBScan dbs;
	int l, u, m;
	Obj *mop;
	double diff;
	double cdec;

	/* sort fop by increasing dec */
	qsort ((void *)fop, nfop, sizeof(ObjF), objfd_cmp);

	/* mark all GSC stars with FLDSTAR flag which are close to any PM.
	 * don't compare GSC and PM stars with themselves.
	 */
	for (m = 0; m < nfop; m++) {
	    mop = (Obj *)&fop[m];

	    /* only check for GSC stars close to PM stars, not v.v. */
	    if (mop->o_name[0] == 'G')
		continue;

	    /* scan each way from mop and mark close GSC stars */
	    cdec = cos(mop->s_dec);
	    for (u = m+1; u < nfop; u++) {
		op = (Obj *)&fop[u];
		if (fabs(mop->s_dec - op->s_dec) >= DUPSEP)
		    break;
		if (op->o_name[0] == 'G'
			&& cdec*delra(mop->s_ra - op->s_ra) < DUPSEP
			&& fabs(get_mag(mop) - get_mag(op)) < DUPMAG)
		    op->o_flags |= FLDSTAR;
	    }
	    for (l = m-1; l >= 0; l--) {
		op = (Obj *)&fop[l];
		if (fabs(mop->s_dec - op->s_dec) >= DUPSEP)
		    break;
		if (op->o_name[0] == 'G'
			&& cdec*delra(mop->s_ra - op->s_ra) < DUPSEP
			&& fabs(get_mag(mop) - get_mag(op)) < DUPMAG)
		    op->o_flags |= FLDSTAR;
	    }
	}

	/* scan all FIXED objects and mark all field stars with FLDSTAR
	 * which appears to be the same any one.
	 */
	for (db_scaninit(&dbs, FIXEDM, NULL, 0); (op = db_scan(&dbs)) != 0; ) {
	    /* update the s_ra/dec fields */
	    db_update (op);

	    /* skip ops outside the given field */
	    cdec = cos(op->s_dec);
	    if (fabs(op->s_dec - dec) > rov || cdec*delra(op->s_ra - ra) > rov)
		continue;

	    /* binary search to find the fop closest to op */
	    l = 0;
	    u = nfop - 1;
	    while (l <= u) {
		m = (l+u)/2;
		mop = (Obj *)(&fop[m]);
		diff = mop->s_dec - op->s_dec;
		if (diff < 0.0)
		    l = m+1;
		else
		    u = m-1;
	    }

	    /* scan each way from m and mark all that are dups with FLDSTAR.
	     * N.B. here, FLDSTAR marks *dups* (not the entries to keep)
	     * N.B. remember, u and l have crossed each other by now.
	     */
	    for (; l < nfop; l++)
		if (scanchk ((Obj *)(&fop[l]), op, cdec) < 0)
		    break;
	    for (; u >= 0; --u)
		if (scanchk ((Obj *)(&fop[u]), op, cdec) < 0)
		    break;
	}

	/* squeeze all entries marked with FLDSTAR to the front of fop[]
	 * and mark those that remain with FLDSTAR (yes, the bit now finally
	 * means what it says).
	 */
	for (u = l = 0; u < nfop; u++) {
	    mop = (Obj *)(&fop[u]);
	    if (!(mop->o_flags & FLDSTAR)) {
		mop->o_flags |= FLDSTAR; /* now the mark means a *keeper* */
		if (u > l)
		    memcpy ((void *)(&fop[l]), (void *)mop, sizeof(ObjF));
		l++;
	    }
	}

	return (l);
}

/* if mop is further than DUPSEP from op in dec, return -1.
 * then if mop is within DUPSEP in ra and within DUPMAG in mag too set FLDSTAR
 *   in mop->o_flags.
 * return 0.
 */
static int
scanchk (mop, op, cdec)
Obj *mop, *op;
double cdec;
{
	if (fabs(mop->s_dec - op->s_dec) >= DUPSEP)
	    return (-1);
	if (cdec*delra(mop->s_ra - op->s_ra) < DUPSEP
		&& fabs(get_mag(mop) - get_mag(op)) < DUPMAG)
	    mop->o_flags |= FLDSTAR;
	return (0);
}

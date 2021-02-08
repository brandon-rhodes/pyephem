/* find best WCS.
 * technique inspired by a paper by Frank Valdes PASP, vol 107, page 1119 (1995)
 *
 * N.B. this implementation is not reentrant.
 */

/* TRACE:
 *  0: none
 *  1: array counts, FITS evolution, final set of pairs
 *  2: plus each Tri pair
 *  3: plus each LstSqr step
 *  4: plus all points and triangles
 */
#define	trace(level)	if (trlevel >= (level)) printf

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "astro.h"
#include "ip.h"
#include "fsmatch.h"


#define	MAXIS		40	/* max image stars to use */
#define	MAXFS		40	/* max field stars to use */
#define	MINPAIRS	3	/* minimum number of star pairs to accept */
#define	MINTP		3	/* min number tri's a point must be in */
#define	MINNT		4	/* min number of tri pairs */
#define	MINAREA		.01	/* min fraction total area in a tri */
#define	SIMAREA		.95	/* SIMAREA .. 1/SIMAREA range of similar area */
#define	STDCUT		1.50	/* outlyer definition */
#define	LSTOL		.005	/* least squares convergence factor */
#define	MAXOUTL		5	/* max times to refine lst-sqrs fit */

/* combination of n objects taken 3 at a time */
#define	CMBN3(n)	((n)*((n)-1)*((n)-2)/6)

/* one vertex */
typedef struct _ver {
    double x, y;		/* coords */
    Obj *op;			/* reference Object (field star only) */
} Vtx;

/* one triangle */
typedef struct _tri {
    Vtx *v0, *v1, *v2;		/* constituent Vtx | v0-v1 < v1-v2 < v0-v2 */
    double x, y;		/* location in triangle-space */
    double a;			/* area */
    struct _tri *tp;		/* matching tri, if any */
} Tri;

/* one matching pair */
typedef struct {
    Vtx *iv, *fv;		/* image and field vertex of pair */
    int n;			/* occurances in matching triangles */
} PVtx;

static int newTri (Tri *tp, double mina, Vtx *v0, Vtx *v1, Vtx *v2);
static void initFITS (FImage *initial, FImage *test);
static void prWCS (char *label, FImage *fip);
static int lsFit (FImage *fip);
static double chisqr(double p[]);
static int discardOutlyers(FImage *fip);
static int pv_qscmpf (const void *v1, const void *v2);
static void setTrLevel(void);

/* global for use by lstsqr() solver function.
 * N.B. hence not reentrant!
 */
static FImage _fi;		/* candidate solution */
static PVtx *_pv;		/* global shadow */
static int _nv;			/* global shadow */
static Vtx *_iv;		/* global shadow */
static Vtx *_fv;		/* global shadow */
static double _best;		/* best accuracy we can hope for */
static double _worst;		/* worst accuracy we will accept */

static int trlevel;		/* sets trace level */

/* given a guess for WCS already in fip, a list of field stars in the
 * vicinity sorted by brightness, and a list of coordinates for starlike
 * things in the image sorted by brightest, put a best fit solution into *fip.
 * if successful:
 *   fs and isx/y are reordered so matching pairs actually used are in front.
 *   return number of pairs used in solution.
 * else
 *   write excuse in msg[] and return -1
 */
int
fsmatch (FImage *fip, void (*drawf)(double x, double y, int rad, int ccode),
ObjF *fs, int nfs, double *isx, double *isy, int nis, double best,
double worst, char msg[])
{
	FImage lsfits, fits0;		/* working solutions */
	double maxd2 = 2*sqr((fip->sw+2*worst)/(fip->sw-2*worst) - 1.);
	double minarea = MINAREA * fip->sw * fip->sh;
	PVtx pv[MAXIS];			/* matching pairs of vertices */
	Vtx iv[MAXIS];			/* list of image vertices */
	Vtx fv[MAXFS];			/* list of field vertices */
	ObjF fscopy[MAXFS];		/* copy of fs for local rearranging */
	int niv, nfv;			/* actual number of " */
	Tri *it = NULL;			/* malloced image star triangles */
	Tri *ft = NULL;			/* malloced field star triangles */
	int nit, nft;			/* actual number of " */
	int vcount[MAXIS][MAXFS];	/* n occurances of each pair */
	int i, j, k;
	Vtx *vp;
	Tri *tp;

	setTrLevel();
	trace(1) ("\nfsmatch(nfs=%d, nis=%d, best=%g worst=%g)\n",
						    nfs, nis, best, worst);

	/* sanity checks */
	if (nis < MINPAIRS) {
	    sprintf (msg, "%d image stars, need at least %d", nis, MINPAIRS);
	    return (-1);
	}
	if (nfs < MINPAIRS) {
	    sprintf (msg, "%d field stars, need at least %d", nfs, MINPAIRS);
	    return (-1);
	}

	/* collect up to MAXIS image stars into iv[] */
	niv = nis < MAXIS ? nis : MAXIS;
	memset (iv, 0, niv*sizeof(iv[0]));
	for (vp = iv, i = 0; i < niv; i++, vp++) {
	    vp->x = isx[i];
	    vp->y = isy[i];
	    (*drawf) (vp->x, vp->y, 5, 0);
	    trace(4) ("IV%03d %6.1f %6.1f %d\n", (int)(vp-iv), vp->x, vp->y,
		    ((CamPix *)fip->image)[(int)(vp->y*fip->sw) + (int)vp->x]);
	}

	/* build each image triangle with sufficient area in it[] */
	nit = CMBN3(niv);
	tp = it = (Tri *) calloc (nit, sizeof(Tri));
	if (!it) {
	    sprintf (msg, "No memory for %d image triangles", nit);
	    return (-1);
	}
	for (i = 0; i < niv; i++)
	    for (j = i+1; j < niv; j++)
		for (k = j+1; k < niv; k++)
		    if (!newTri (tp, minarea, &iv[i], &iv[j], &iv[k])) {
			trace(4) ("IT%05d IV%03d IV%03d IV%03d %10.7f %10.7f\n",
						(int)(tp-it), i, j, k, tp->x, tp->y);
			tp++;
		    }
	nit = tp - it;
	trace(1) ("%5d image triangles from %d image stars\n", nit, niv);

	/* collect up to MAXFS field stars into fv[] based on initial fip */
	nfv = nfs < MAXFS ? nfs : MAXFS;
	memset (fv, 0, nfv*sizeof(fv[0]));
	memcpy (fscopy, fs, nfv*sizeof(fscopy[0]));
	for (vp = fv, i = 0; i < nfv; i++, vp++) {
	    vp->op = (Obj*)(&fscopy[i]);
	    RADec2xy (fip, vp->op->f_RA, vp->op->f_dec, &vp->x, &vp->y);
	    (*drawf) (vp->x, vp->y, 4, 2);
	    trace(4) ("FV%03d %6.1f %6.1f %5.2f %s\n", (int)(vp-fv), vp->x, vp->y,
					    get_mag(vp->op), vp->op->o_name);
	}

	/* build each field triangle with sufficient area in ft[] */
	nft = CMBN3(nfv);
	tp = ft = (Tri *) calloc (nft, sizeof(Tri));
	if (!ft) {
	    sprintf (msg, "No memory for %d field star triangles", nft);
	    free (it);
	    return (-1);
	}
	for (i = 0; i < nfv; i++)
	    for (j = i+1; j < nfv; j++)
		for (k = j+1; k < nfv; k++)
		    if (!newTri (tp, minarea, &fv[i], &fv[j], &fv[k])) {
			trace(4) ("FT%05d FV%03d FV%03d FV%03d %10.7f %10.7f\n",
						(int)(tp-ft), i, j, k, tp->x, tp->y);
			tp++;
		    }
	nft = tp - ft;
	trace(1) ("%5d field triangles from %d field stars\n", nft, nfv);

	/* at this point we have a list of image triangles, it[nit], and their
	 * vertices, iv[niv], and a list of field star triangles, ft[nft], and
	 * their vertices, fv[nfv], such that all triangles occupy at least a
	 * minimum fraction of the total image area.
	 * N.B. from here on, always free() it and ft before returning.
	 */

	/* find closest f to each i tri, must be closer than threshold */
	tp = it;
	for (i = 0; i < nit; i++) {
	    Tri *itp = &it[i];
	    double mind2 = maxd2;

	    /* init no, then find ft closest to itp */
	    itp->tp = 0;
	    for (j = 0; j < nft; j++) {
		Tri *ftp = &ft[j];
		double r = itp->a/ftp->a;
		if (SIMAREA < r && r < (1./SIMAREA)) {
		    double d2 = sqr(itp->x-ftp->x) + sqr(itp->y-ftp->y);
		    if (d2 < mind2) {
			mind2 = d2;
			itp->tp = ftp;	/* link i tri to best f tri */
		    }
		}
	    }
	    if (itp->tp) {
		if (itp != tp)
		    *tp = *itp;		/* avoid inplace copy */
		tp++;
		trace(2)("IT%05d FT%05d D= %8.6f\n", i, (int)(itp->tp-ft),sqrt(mind2));
	    }
	}
	nit = tp - it;
	trace(1) ("Found %d similar triangle pairs D< %g\n", nit, sqrt(maxd2));

	if (nit < MINNT) {
	    sprintf (msg, "%d similar triangle pairs, need at least %d",
								    nit, MINNT);
	    free (it);
	    free (ft);
	    return (-1);
	}

	/* count each occurance of a vertex pairing */
	memset (vcount, 0, sizeof(vcount));
	for (i = 0; i < nit; i++) {
	    Tri *itp = &it[i], *ftp = itp->tp;
	    vcount[itp->v0-iv][ftp->v0-fv]++;
	    vcount[itp->v1-iv][ftp->v1-fv]++;
	    vcount[itp->v2-iv][ftp->v2-fv]++;
	}

	/* build list of best vertex pairs */
	for (i = 0; i < niv; i++) {
	    k = 0;
	    for (j = 1; j < nfv; j++)
		if (vcount[i][j] > vcount[i][k])
		    k = j;
	    pv[i].iv = &iv[i];
	    pv[i].fv = &fv[k];
	    pv[i].n = vcount[i][k];
	}

	/* sort by decreasing count. require basic minimum */
	qsort (pv, niv, sizeof(pv[0]), pv_qscmpf);
	j = pv[0].n/2;
	if (j < MINTP)
	    j = MINTP;
	for (i = 0; i < niv && j < pv[i].n; i++) {
	    trace(1) ("IV%03d @ %4.0f %4.0f FV%03d %4.0f %4.0f N= %3d %-*s\n",
					(int)(pv[i].iv-iv), pv[i].iv->x, pv[i].iv->y,
					(int)(pv[i].fv-fv), pv[i].fv->x, pv[i].fv->y,
					pv[i].n, MAXNM, pv[i].fv->op->o_name);
	}
	if (i < MINPAIRS) {
	    sprintf (msg, "Found %d star pairs, need %d", i, MINPAIRS);
	    free (it);
	    free (ft);
	    return (-1);
	}

	/* give solver access to our stack info */
	_nv = i;
	_pv = pv;
	_iv = iv;
	_fv = fv;
	_best = best;
	_worst = worst;

	/* init fits0 using best 3 vertex pairs */
	prWCS ("Initial seed", fip);
	initFITS (&fits0, fip);
	prWCS ("Reference fit", &fits0);

	/* lstsqr fits, discarding outlyers each time around until none bad */
	for (i = 0; i < MAXOUTL; i++) {
	    trace(1) ("LSFIT loop %2d with %2d star pairs\n", i, _nv);
	    lsfits = fits0;
	    if (lsFit(&lsfits) < 0) {
		sprintf (msg, "Lst-sqr did not converge -- bogus pairing");
		free (it);
		free (ft);
		return (-1);
	    }

	    prWCS ("LSFIT result", &lsfits);
	    j = discardOutlyers(&lsfits);
	    if (j == 0)
		break;	/* good enough or at least no improvment */
	    _nv -= j;
	    if (_nv < MINPAIRS) {
		sprintf (msg, "%d statistically significant pairs, need %d",
								_nv, MINPAIRS);
		free (it);
		free (ft);
		return (-1);
	    }
	}
	if (i == MAXOUTL) {
	    sprintf (msg, "No matching star pairs found.");
	    free (it);
	    free (ft);
	    return (-1);
	}

	/* lsfits is the best guess. check if all pairs lie within worst */
	for (i = 0; i < _nv; i++) {
	    double x, y, e;
	    RADec2xy (&lsfits, pv[i].fv->op->f_RA, pv[i].fv->op->f_dec, &x, &y);
	    e = sqrt(sqr(x-pv[i].iv->x) + sqr(y-pv[i].iv->y));
	    if (e > worst) {
		sprintf (msg, "Best solution has errors at least %.2f", e);
		free (it);
		free (ft);
		return (-1);
	    }
	}

	/* Ok! looks like lsfits is the answer! */
	*fip = lsfits;
	prWCS ("Final answer", fip);
	setRealFITS (fip,   "CDELT1", fip->xinc, 10, "RA step right, degs/pix");
	setRealFITS (fip,   "CDELT2", fip->yinc, 10, "Dec step up, degs/pix");
	setRealFITS (fip,   "CRPIX1", fip->xrefpix, 10, "Reference RA pixel");
	setRealFITS (fip,   "CRPIX2", fip->yrefpix, 10, "Reference Dec pixel");
	setRealFITS (fip,   "CRVAL1", fip->xref, 10, "Reference RA, degs");
	setRealFITS (fip,   "CRVAL2", fip->yref, 10, "Reference Dec, degs");
	setRealFITS (fip,   "CROTA2", fip->rot, 10, "Rotation E from N, degs");
	setStringFITS (fip, "CTYPE1", "RA---TAN", "RA Projection");
	setStringFITS (fip, "CTYPE2", "DEC--TAN", "Dec Projection");

	/* put pairs used in solution back into top of fs[] and isx/y[] */
	for (i = 0; i < _nv; i++) {
	    fs[i] = *((ObjF *)(pv[i].fv->op));
	    isx[i] = pv[i].iv->x;
	    isy[i] = pv[i].iv->y;
	}

	free (it);
	free (ft);
	return (_nv);
}

/* set tp->v0..2 such that v0..1 < v1..2 < v0..v2, and tp->x and y.
 * return 0 if area > mina and a given handedness else -1.
 */
static int
newTri (Tri *tp, double mina, Vtx *v0, Vtx *v1, Vtx *v2)
{
	double d01 = sqrt(sqr(v0->x - v1->x) + sqr(v0->y - v1->y));
	double d12 = sqrt(sqr(v1->x - v2->x) + sqr(v1->y - v2->y));
	double d02 = sqrt(sqr(v0->x - v2->x) + sqr(v0->y - v2->y));
	int lt0112 = d01 < d12;
	int lt1202 = d12 < d02;
	int lt0102 = d01 < d02;
	double s;

	s = 0.5*(d01 + d12 + d02);
	tp->a = sqrt (s*(s-d01)*(s-d12)*(s-d02));
	if (tp->a < mina)
	    return (-1);

	/* "inline" sort */
	if (lt0112) {
	    if (lt1202) {
		    tp->v0 = v0; tp->v1 = v1; tp->v2 = v2;
		    tp->x = d01/d02; tp->y = d12/d02;
	    } else {
		if (lt0102) {
		    tp->v0 = v1; tp->v1 = v0; tp->v2 = v2;
		    tp->x = d01/d12; tp->y = d02/d12;
		} else {
		    tp->v0 = v2; tp->v1 = v0; tp->v2 = v1;
		    tp->x = d02/d12; tp->y = d01/d12;
		}
	    }
	} else {
	    if (lt0102) {
		    tp->v0 = v2; tp->v1 = v1; tp->v2 = v0;
		    tp->x = d12/d02; tp->y = d01/d02;
	    } else {
		if (lt1202) {
		    tp->v0 = v1; tp->v1 = v2; tp->v2 = v0;
		    tp->x = d12/d01; tp->y = d02/d01;
		} else {
		    tp->v0 = v0; tp->v1 = v2; tp->v2 = v1;
		    tp->x = d02/d01; tp->y = d12/d01;
		}
	    }
	}

	/* want all same handedness (either one will do) */
	if ((tp->v1->x - tp->v0->x) * (tp->v2->y - tp->v1->y)
			- (tp->v1->y - tp->v0->y) * (tp->v2->x - tp->v1->x) < 0)
	    return (-1);

	return (0);
}

/* init tfip from fip and 3 best vertex pairs */
static void
initFITS (FImage *tfip, FImage *fip)
{
	double ix0 = _pv[0].iv->x, iy0 = _pv[0].iv->y;
	double ix1 = _pv[1].iv->x, iy1 = _pv[1].iv->y;
	double ix2 = _pv[2].iv->x, iy2 = _pv[2].iv->y;
	Obj *op0 = _pv[0].fv->op;
	Obj *op1 = _pv[1].fv->op;
	Obj *op2 = _pv[2].fv->op;
	double fx0, fy0, fx1, fy1, fx2, fy2;
	double tmp1, tmp2, tmp3;

	/* start tfip with fip */
	*tfip = *fip;

	/* rotate to make first 2 points parallel */
	RADec2xy (tfip, op0->f_RA, op0->f_dec, &fx0, &fy0);
	RADec2xy (tfip, op1->f_RA, op1->f_dec, &fx1, &fy1);
	tmp1 = raddeg (atan2 (iy1 - iy0, ix1 - ix0));
	tmp2 = raddeg (atan2 (fy1 - fy0, fx1 - fx0));
	tmp3 = tmp1 - tmp2;
	tfip->rot += tmp3;
	RADec2xy (tfip, op0->f_RA, op0->f_dec, &fx0, &fy0);
	RADec2xy (tfip, op1->f_RA, op1->f_dec, &fx1, &fy1);
	prWCS ("After rotation", tfip);
	
	/* use sign of cross product with 3rd point to check whether flipped */
	RADec2xy (tfip, op2->f_RA, op2->f_dec, &fx2, &fy2);
	if (((fx1-fx0)*(fy2-fy1)-(fy1-fy0)*(fx2-fx1)) *
				((ix1-ix0)*(iy2-iy1)-(iy1-iy0)*(ix2-ix1)) < 0) {
	    tfip->yinc *= -1;
	    tfip->rot += 2*tmp2;
	    RADec2xy (tfip, op0->f_RA, op0->f_dec, &fx0, &fy0);
	    RADec2xy (tfip, op1->f_RA, op1->f_dec, &fx1, &fy1);
	    prWCS ("After flipping", tfip);
	}

	/* prefer angle in range -90..90 */
	while (tfip->rot > 90) {
	    tfip->rot -= 180;
	    tfip->xinc *= -1;
	    tfip->yinc *= -1;
	}
	while (tfip->rot < -90) {
	    tfip->rot += 180;
	    tfip->xinc *= -1;
	    tfip->yinc *= -1;
	}
	prWCS ("After pretty rot", tfip);

	/* scale by matching length */
	tmp1 = sqrt(sqr(fx1-fx0) + sqr(fy1-fy0));
	if (tmp1 != 0) {
	    tmp2 = sqrt(sqr(ix1-ix0) + sqr(iy1-iy0));
	    tmp3 = tmp2/tmp1;
	    tfip->xinc *= tmp3;
	    tfip->yinc *= tmp3;
	}
	RADec2xy (tfip, op0->f_RA, op0->f_dec, &fx0, &fy0);
	RADec2xy (tfip, op1->f_RA, op1->f_dec, &fx1, &fy1);
	prWCS ("After scaling", tfip);

	/* translate either end to align */
	tmp1 = fx0 - ix0;
	tmp2 = fy0 - iy0;
	tfip->xrefpix += tmp1;
	tfip->yrefpix += tmp2;
	prWCS ("After translating", tfip);
}

static void
prWCS (char *label, FImage *fip)
{
	trace(1)
	    ("FITS %-17s R= %7.2f X/YINC= %5.2f %5.2f X/YREFPIX= %6.1f %6.1f\n",
				label, fip->rot, fip->xinc*3600, fip->yinc*3600,
				    fip->xrefpix, fip->yrefpix);
}

/* use least-squares method to find the fip that best maps the _fv[] to iv[].
 * return 0 if converged, else -1
 */
static int
lsFit (FImage *fip)
{
	double p0[4], p1[4];

	/* set up initial palues in order expected by chisqr().
	 * init either way a few percent or so.
	 */
	p0[0] = fip->rot - 2;
	p0[1] = fip->xrefpix - fip->sw*.01;
	p0[2] = fip->yrefpix - fip->sh*.01;
	p0[3] = .99;

	p1[0] = fip->rot + 2;
	p1[1] = fip->xrefpix + fip->sw*.01;
	p1[2] = fip->yrefpix + fip->sh*.01;
	p1[3] = 1.01;

	/* store initial FITS in global for chisqr */
	_fi = *fip;

	/* go */
	if (lstsqr (chisqr, p0, p1, 4, LSTOL) < 0)
	    return (-1);

	/* unpack answer back into fip */
	fip->rot     = p0[0];
	fip->xrefpix = p0[1];
	fip->yrefpix = p0[2];
	fip->xinc    = _fi.xinc * p0[3];
	fip->yinc    = _fi.yinc * p0[3];

	return (0);
}

/* evaluate fit at each _pv[] @ p. */
static double
chisqr (double p[4])
{
	FImage ftmp = _fi;
	double sum2;
	int i;

	/* set up trial */
	ftmp.rot     = p[0];
	ftmp.xrefpix = p[1];
	ftmp.yrefpix = p[2];
	ftmp.xinc    = _fi.xinc * p[3];
	ftmp.yinc    = _fi.yinc * p[3];

	/* sum of of distances^2 between image and projected field stars */
	for (sum2 = i = 0; i < _nv; i++) {
	    double fx, fy, e2;
	    RADec2xy(&ftmp, _pv[i].fv->op->f_RA, _pv[i].fv->op->f_dec, &fx,&fy);
	    e2 = sqr(_pv[i].iv->x - fx) + sqr(_pv[i].iv->y - fy);
	    sum2 += e2;
	}

	/* N.B. noticed solver can bounce forever when really small */
	if (sum2 < 1e-5)
	    sum2 = 0;

	trace (3) ("chisqr= %9.3f @ R= %8.3f dX/Y= %8.6f %8.6f X/Y= %7.2f %7.2f\n",
				sum2, ftmp.rot, ftmp.xinc, ftmp.yinc,
				ftmp.xrefpix, ftmp.yrefpix);
	return (sum2);
}

/* remove outlyers from _pv[] based on *fip proposal.
 * return number of pairs removed.
 */
static int
discardOutlyers(FImage *fip)
{
	double err[MAXIS];
	double max, sum, mean;
	double cut;
	int i, n;

	/* find mean error */
	max = sum = 0;
	for (i = 0; i < _nv; i++) {
	    double fx, fy;
	    RADec2xy (fip, _pv[i].fv->op->f_RA, _pv[i].fv->op->f_dec, &fx, &fy);
	    err[i] = sqrt(sqr(_pv[i].iv->x - fx) + sqr(_pv[i].iv->y - fy));
	    sum += err[i];
	    if (err[i] > max)
		max = err[i];
	}

	/* done if get mean bellow best expected accuracy */
	mean = sum/_nv;
	if (mean < _best) {
	    trace(1) ("LSFIT mean Err = %g, better than best %g\n", mean,_best);
	    return (0);
	}

	cut = STDCUT*mean;
	trace(1)("LSFIT n= %d mean Err = %g cutting @ %g\n",_nv, mean, cut);

	/* discard those beyond cut */
	n = 0;
	for (i = 0; i < _nv; i++) {
	    if (err[i] < cut) {
		if (i != n)		/* just to avoid inplace copies */
		    _pv[n] = _pv[i];
		n++;
		trace(1)("LSFIT Keeping  %3d: Err = %9.2f @ IV%03d FV%03d %s\n",
				    i, err[i], (int)(_pv[i].iv-_iv), (int)(_pv[i].fv-_fv),
				    	_pv[i].fv->op->o_name);

	    } else {
		trace(1)("LSFIT Cutting  %3d: Err = %9.2f @ IV%03d FV%03d %s\n",
				    i, err[i], (int)(_pv[i].iv-_iv), (int)(_pv[i].fv-_fv),
					_pv[i].fv->op->o_name);
	    }
	}

	return (_nv-n);
}

/* compare two PVtx in order of _decreasing_ n, qsort-style */
static int
pv_qscmpf (const void *v1, const void *v2)
{
	return (((PVtx*)v2)->n - ((PVtx*)v1)->n);
}

/* set trlevel from FSMTL env var */
static void
setTrLevel()
{
	static char *ev;

	if (!ev)
	    ev = getenv ("FSMTL");
	if (ev)
	    trlevel = atoi (ev);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: fsmatch.c,v $ $Date: 2009/01/05 20:55:16 $ $Revision: 1.13 $ $Name:  $"};

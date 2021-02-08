#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ip.h"

#define	MAXSR		20		/* max star radius, ie, pix to noise */
#define	MINSR		1		/* min star radius */
#define	SKYR		2		/* measure sky @ starr * SKYR */
#define	MINSS		2		/* min star separation */
#define	NWALK		8		/* n pixels surrounding a pixel */
#define	NCONN		3		/* min connected pixels to qualify */
#define	PKP		5		/* dips must be > this % of peak-med */

static double gauss2d (Gaussian *hgp, Gaussian *vgp, int x, int y);

/* extract some "stars" from fip, no guarantees but fast.
 * return malloced arrays of centroided locations sorted by brightest pixel.
 * N.B. caller must always free *xpp and *ypp if return >= 0.
 */
int
quickStars (FImage *fip, ImStats *isp, int border, int burnt, double nsd,
double **xpp, double **ypp)
{
#define	IFDP	if (dfp && abs(x-dx)<=dr && abs(y-dy)<=dr)
	CamPix *ip, *ip0 = (CamPix *)fip->image;
	int bsr = border+MAXSR;		/* handy border-MAXSR */
	int w = fip->sw;		/* handy */
	int h = fip->sh;		/* handy */
	int walk[NWALK];		/* fast walk about */
	int *br;			/* temp array to sort by brightness */
	int dx, dy, dr;			/* dump region */
	FILE *dfp;			/* dump file, if any */
	double *xp, *yp;
	int x, y;
	int nis;

	/* prime the arrays */
	xp = (double *) malloc (10*sizeof(double));
	yp = (double *) malloc (10*sizeof(double));
	br = (int *)    malloc (10*sizeof(int));
	if (!xp || !yp || !br) {
	    if (xp) free (xp);
	    if (yp) free (yp);
	    if (br) free (br);
	    return (-1);
	}
	nis = 0;

	/* set up walk-about */
	walk[0] =  1;
	walk[1] =  1 - w;
	walk[2] =    - w;
	walk[3] = -1 - w;
	walk[4] = -1;
	walk[5] = -1 + w;
	walk[6] =      w;
	walk[7] =  1 + w;

	/* check for dump file */
	dfp = fopen ("x.dump", "r");
	if (dfp) {
	    char fname[128];
	    int ok = fscanf (dfp, "%s %d %d %d", fname, &dx, &dy, &dr) == 4;
	    fclose (dfp);
	    dfp = ok ? fopen (fname, "w") : NULL;
	}

	/* scan inside border+MAXSR for local maxima */
	ip = ip0 + bsr*w + bsr;
	for (y = bsr; y < h-bsr; y++) {
	    for (x = bsr; x < w-bsr; x++) {
		int p0 = (int)*ip;	/* current central pixel */
		double cx, cy;		/* centroided position */
		int sky, cut;		/* sky, real star cutoff */
		double std;		/* 1 sigma */
		int rmin, rmax;		/* min and max of a given ring */
		int lmax;		/* rmax in last radii */
		int starr;		/* radius of star edge */
		int mindip;		/* minimum dip before peaking */
		int mass;		/* star brightness */
		int ncon;
		int i, r;

		/* immediately discard burnt pixels or ones below median */
		mindip = PKP*(p0 - isp->median);
		if (mindip < 0) {
		    IFDP fprintf (dfp, "%4d %4d: %5d < median %5d\n", x, y,
							    p0, isp->median);
		    goto no;
		}
		if (p0 >= burnt) {
		    IFDP fprintf (dfp, "%4d %4d: burnt: %5d >= %5d\n", x, y,
								    p0, burnt);
		    goto no;
		}

		/* init ranges */
		lmax = p0;
		starr = 0;

		/* check successively greater rings. Found star outter edge
		 * when max has decreased and increased again.
		 */
		for (r = 1; r < MAXSR; r++) {

		    /* get stats for ring with radius r */
		    ringStats (ip, w, r, NULL, &rmax, NULL);

		    /* reject any burnt pixels */
		    if (rmax >= burnt) {
			IFDP fprintf (dfp, "%4d %4d: ring %d burnt %5d\n",
								x, y, r, rmax);
			goto no;
		    }

		    /* found star edge when max first increases.
		     * allow for a little benign bumpiness near the top
		     */
		    if (rmax > lmax && 100*(p0-lmax) > mindip) {
			starr = r - 1;	/* stats are for last radius */
			break;
		    }

		    /* p0 is not a peak if neighbors are brighter */
		    if (rmax > p0) {
			IFDP fprintf (dfp,
			    "%4d %4d: %5d but %5d in ring @ r= %d\n",
							x, y, p0, rmax, r);
			goto no;
		    }

		    /* save last ring extreme */
		    lmax = rmax;
		}

		if (r == MAXSR) {
		    IFDP fprintf (dfp, "%4d %4d: starr %d too large\n",
								x, y, r);
		    goto no;
		}
		if (starr < MINSR) {
		    IFDP fprintf (dfp, "%4d %4d: starr %d too small\n",
								x, y, starr);
		    goto no;
		}

		/* estimate sky and noise and set cutoff */
		r = SKYR*starr;
		if (r > MAXSR)
		    r = MAXSR;
		ringStats (ip, w, r, &rmin, NULL, &sky);
		std = (sky - rmin)/3.0;	/* ?? */
		cut = sky + (int)(nsd*std);
		if (p0 < cut) {
		    IFDP fprintf (dfp,
				"%4d %4d: %5d too dim r= %d sky= %d std= %g\n",
						    x, y, p0, starr, sky, std);
		    goto no;
		}

		/* want several connected neighbors above cutoff */
		ncon = 0;
		for (i = 0; i < NWALK+NCONN; i++)
		    if (ip[walk[i%NWALK]] >= cut) {
			if (++ncon == NCONN)
			    break;
		    } else
			ncon = 0;
		if (ncon < NCONN) {
		    IFDP fprintf (dfp,
			    "%4d %4d: only %d connected neighbors above %d\n",
							    x, y, ncon, cut);
		    goto no;
		}

		/* find centroided position */
		starCentroid (ip, w, starr, sky, &cx, &cy, &mass);
		cx += x;
		cy += y;

		/* must be a few pixels from any other */
		for (i = 0; i < nis; i++)
		    if (fabs(cx-xp[i]) < MINSS && fabs(cy-yp[i]) < MINSS) {
			IFDP fprintf (dfp,
				    "%4d %4d: %5d dup @ %4.0f %4.0f\n",
						x, y, p0, xp[i], yp[i]);
			goto no;
		    }

		/* Ok! insert in order of decreasing brightness */
		xp = (double *) realloc ((char *)xp, (nis+1)*sizeof(double));
		yp = (double *) realloc ((char *)yp, (nis+1)*sizeof(double));
		br = (int *)    realloc ((char *)br, (nis+1)*sizeof(int));
		if (!xp || !yp || !br) {
		    if (xp) free (xp);
		    if (yp) free (yp);
		    if (br) free (br);
		    return (-1);
		}
		for (i = nis; i > 0 && mass > br[i-1]; --i) {
		    br[i] = br[i-1];
		    xp[i] = xp[i-1];
		    yp[i] = yp[i-1];
		}
		br[i] = mass;
		xp[i] = cx;
		yp[i] = cy;

		IFDP fprintf (dfp,
		    "%4d %4d: OK %5d > %5d %6.1f %6.1f r= %d sky= %d std= %g\n",
				    x, y, p0, cut, cx, cy, starr, sky, std);

		/* inc number of stars found */
		nis++;

	      no:

		/* move to next pixel right */
		ip++;
	    }

	    /* move to start of next row */
	    ip += 2*bsr;
	}

	free ((char *)br);
	*xpp = xp;
	*ypp = yp;
	if (dfp)
	    fclose (dfp);
	return (nis);
}

/* given a region that presumably contains a single star, fill in sp
 * with best-fit 2d commensurate gaussian.
 * return 0 if ok, else -1
 * N.B. we assume rp has already been clamped.
 */
int
getStar (ImRegion *rp, Star *sp)
{
	CamPix *im = &rp->im[rp->ry*rp->iw + rp->rx];
	int wrap = rp->iw - rp->rw;
	int nr = rp->rw * rp->rh;
	double sum = 0, sum2 = 0;
	double v;
	int x, y;

	/* on guard */
	if (nr < 2) {
	    printf ("getStar called with region of 1\n");
	    return (-1);
	}

	/* find commensurate gaussians */
	if (gauss2fit (rp, &sp->hg, &sp->vg) < 0)
	    return (-1);

	/* position is with respect to corner of region */
	sp->x = rp->rx + sp->hg.m;
	sp->y = rp->ry + sp->vg.m;

	/* compare ideal to real to estimate noise */
	for (y = 0; y < rp->rh; y++) {
	    for (x = 0; x < rp->rw; x++) {
		double ideal = gauss2d (&sp->hg, &sp->vg, x, y);
		double real = (double)*im++;
		double err = ideal - real;
		sum += err;
		sum2 += sqr(err);
	    }
	    im += wrap;
	}

	/* error is STARSIGMA sigma noise */
	v = (sum2 - sum*sum/nr)/(nr-1);
	sp->err = v <= 0 ? 0 : STARSIGMA*sqrt(v);

	return (0);
}

/* find the magnitude of s1 wrt s0 and the error in the estimate.
 * gaussian volume taken to be proportional to height * sigmax * sigmay.
 * return 0 if ok, else -1 if numerical troubles.
 */
int
cmpStars (Star *s0, Star *s1, double *magp, double *errp)
{
	double v0, v1, r;

	/* magnitude is ratio of guassian volumes */
	v0 = s0->hg.A * s0->hg.s * s0->vg.s;
	v1 = s1->hg.A * s1->hg.s * s1->vg.s;
	if (v0 == 0 || (r = v1/v0) <= 0)
	    return (-1);
	*magp = -2.511886*log10(r);		/* + is dimmer */

	/* error = (log(largest ratio) - log(smallest ratio))/2.
	 *       = (log((largest ratio) / (smallest ratio)))/2.
	 * final /2 because we want to report as +/-
	 */
	v1 = (s1->hg.A + s1->err)/(s0->hg.A - s0->err);	/* largest */
	v0 = (s1->hg.A - s1->err)/(s0->hg.A + s0->err);	/* smallest */
	if (v0 == 0)
	    return (-1);
	*errp = (2.511886/2)*log10(fabs(v1/v0));

	/* made it */
	return (0);
}


/* compute value of 2d gaussian at [x,y] */
static double
gauss2d (Gaussian *hgp, Gaussian *vgp, int x, int y)
{
	return (hgp->B + hgp->A*exp(-.5*(sqr((x-hgp->m)/hgp->s)+
	                                 sqr((y-vgp->m)/vgp->s))));
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: stars.c,v $ $Date: 2002/01/15 19:48:27 $ $Revision: 1.7 $ $Name:  $"};

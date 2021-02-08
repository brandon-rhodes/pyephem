/* basic pixel statistics */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ip.h"

/* given an image and a region within, return some basic stats.
 * N.B. we do /not/ check that the region has finite area and lies within image.
 */
void
regionStats (ImRegion *rp, ImStats *sp)
{
	CamPix *im = &rp->im[rp->ry*rp->iw + rp->rx];
	int rn = rp->rw * rp->rh;
	int wrap = rp->iw - rp->rw;
	double sum, sum2;
	int hist[NCAMPIX];
	int min, max;
	double v;
	int x, y;
	int i, n;

	/* scan region gathering stats */
	memset (hist, 0, sizeof(hist));
	min = max = im[0];
	sp->maxatx = rp->rx;
	sp->maxaty = rp->ry;
	sum = sum2 = 0;
	for (y = 0; y < rp->rh; y++) {
	    for (x = 0; x < rp->rw; x++) {
		int p = (int)(*im++);
		hist[p]++;
		sum += (double)p;
		sum2 += (double)p*p;
		if (p > max) {
		    max = p;
		    sp->maxatx = rp->rx+x;
		    sp->maxaty = rp->ry+y;
		}
		if (p < min)
		    min = p;
	    }
	    im += wrap;
	}


	sp->min = min;
	sp->max = max;

	if (rn > 1) {
	    sp->mean = sum/rn;
	    v = (sum2 - sum*sp->mean)/(rn-1);
	    sp->std = v <= 0.0 ? 0.0 : sqrt (v);
	} else {
	    sp->mean = (sp->min+sp->max)/2;
	    sp->std = (sp->max-sp->min)/3;
	}

	/* scan 25..75 percentile for central stats */
	n = 0;
	sum = sum2 = 0;
	min = max = -1;				/* min = p@25%, max = p@50% */
	x = 0;					/* pixels in center half */
	for (i = 0; n <= 3*rn/4; i++) {
	    n += hist[i];
	    if (max < 0 && n >= rn/2)
		max = i;
	    if (min < 0 && n >= rn/4)
		min = i;
	    if (min >= 0) {
		sum += (double)i*hist[i];
		sum2 += (double)i*i*hist[i];
		x += hist[i];
	    }
	}

	/* med is 50 percentile */
	sp->median = max;

	/* compute central mean/std */
	if (x > 1) {
	    sp->cmean = sum/x;
	    v = (sum2 - sum*sp->cmean)/(x-1);
	    sp->cstd = v <= 0.0 ? 0.0 : sqrt (v);
	} else {
	    sp->cmean = (min+max)/2;
	    sp->cstd = (max-min)/3;
	}
}

/* return min, max and median of ring of radius [r..r+1) centered at ip.
 * imw is width of host image. any return pointers may be NULL if unwanted.
 * N.B. we do no range checking.
 */
void
ringStats (CamPix *ip, int imw, int r, int *minp, int *maxp, int *medp)
{
	CamPix ring[1000];
	int r2min = r*r;
	int r2max = (r+1)*(r+1);
	int rmin = MAXCAMPIX;
	int rmax = 0;
	int nr = 0;
	int rx, ry;

	for (ry = -r; ry <= r; ry++) {
	    CamPix *iprow = &ip[ry*imw - r];
	    int d2, ry2 = ry*ry;
	    for (rx = -r; rx <= r; rx++) {
		CamPix p1 = (*iprow++);
		d2 = rx*rx + ry2;
		if (d2 >= r2min && d2 < r2max) {
		    ring[nr++] = p1;
		    if (p1 < rmin)
			rmin = p1;
		    if (p1 > rmax)
			rmax = p1;
		}
	    }
	}

	if (minp)
	    *minp = rmin;
	if (maxp)
	    *maxp = rmax;
	if (medp)
	    *medp = cmedian (ring, nr);
}

/* return position and (optional) "mass" of circular region [0..r] centered
 * at ip, assumed to be from host image of width imw.
 * N.B. we do no range checking.
 */
void
starCentroid (CamPix *ip, int imw, int r, int sky, double *dxp, double *dyp,
int *massp)
{
	int sx = 0, sy = 0, sm = 0;
	int r2 = r*r;
	int rx, ry;

	for (ry = -r; ry <= r; ry++) {
	    CamPix *iprow = &ip[ry*imw - r];
	    int ry2 = ry*ry;
	    for (rx = -r; rx <= r; rx++) {
		int p1 = (int)(*iprow++) - sky;
		if (p1 > 0 && rx*rx + ry2 <= r2) {
		    sx += rx*p1;
		    sy += ry*p1;
		    sm += p1;
		}
	    }
	}

	*dxp = (double)sx/sm;
	*dyp = (double)sy/sm;
	if (massp)
	    *massp = sm;
}

/* check and fix the given region so as to be all within the image.
 * return 0 if successful, -1 if region is _completely_ outside the image
 */
int
clampRegion (ImRegion *rp)
{
	if (rp->rx < 0) {
	    rp->rw += rp->rx;
	    rp->rx = 0;
	} else if (rp->rx + rp->rw > rp->iw) {
	    rp->rw = rp->iw - rp->rx;
	}
	if (rp->rw <= 0)
	    return (-1);

	if (rp->ry < 0) {
	    rp->rh += rp->ry;
	    rp->ry = 0;
	} else if (rp->ry + rp->rh > rp->ih) {
	    rp->rh = rp->ih - rp->ry;
	}
	if (rp->rh <= 0)
	    return (-1);

	return (0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: stats.c,v $ $Date: 2004/05/20 06:02:24 $ $Revision: 1.6 $ $Name:  $"};

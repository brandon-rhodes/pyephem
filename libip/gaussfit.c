/* find best-fit gaussian to 1-d array and 2-d region.
 * parameters found are:
 *   A: distance from base to peak
 *   B: distance from 0 to base
 *   s: sigma = 0.4246 FWHM
 *   m: float index from first entry to position of max
 * the function then is:
 *
 *                    (x - m)^2
 *                  - --------
 *                      2 s^2
 *  f(x) = B + A e
 *
 * The 2d case seeks best m and s in V and H directions with same A, B.
 *
 * N.B. this code is not reentrant
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ip.h"

#define	FTOL		.01		/* fractional tolerance */

/* globals for use by chisqr1d() evaluator */
static CamPix *_a;
static int _na;

/* evaluate p quality */
static double
chisqr1d (double p[4])
{
	double A = p[0];
	double B = p[1];
	double s = p[2];
	double m = p[3];
	double sum2 = 0;
	int i;

	for (i = 0; i < _na; i++)
	    sum2 += sqr(B + A*exp(-.5*sqr((i - m)/s)) - _a[i]);
	return (sum2);
}

/* find best gaussian to given array of pixels.
 * return 0 if ok, else -1
 */
int
gaussfit (CamPix a[], int na, Gaussian *gp)
{
	double p0[4], p1[4];
	int min, max;
	int maxi, hw0i, hw1i;
	int mid;
	int i;

	/* scan for min, max, position of max */
	min = MAXCAMPIX;
	max = 0;
	maxi = 0;
	for (i = 0; i < na; i++) {
	    int ai = a[i];
	    if (ai < min)
		min = ai;
	    if (ai > max) {
		max = ai;
		maxi = i;
	    }
	}

	/* scan from maxi for positions that are halfway down to min
	 *   (or edge if hit first).
	 * this is more like half-peak than half-width but it's something.
	 */
	mid = (max + min)/2;
	hw0i = hw1i = -1;
	for (i = 0; i < na; i++) {
	    if (hw0i == -1 && (maxi-i == 0 || a[maxi-i] <= mid))
		hw0i = maxi-i;
	    if (hw1i == -1 && (maxi+i == na-1 || a[maxi+i] <= mid))
		hw1i = maxi+i;
	}

	/* initial estimates */
	p0[0] = max-min;
	p0[1] = min;
	p0[2] = (hw1i - hw0i + 1)/FWHMSIG;
	p0[3] = maxi;
	p1[0] = p0[0] * .9;
	p1[1] = p0[1] * .9;
	p1[2] = p0[2] * 1.1;
	p1[3] = p0[3] + 1;

	/* go */
	_a = a;
	_na = na;
	if (lstsqr (chisqr1d, p0, p1, 4, FTOL) < 0)
	    return (-1);

	/* ok! */
	gp->A = p0[0];
	gp->B = p0[1];
	gp->s = p0[2];
	gp->m = p0[3];
	return (0);
}

/* globals for use by chisqr2d() evaluator */
static ImRegion *_rp;
static CamPix *_im;
static int _wrap;

/* evaluate p quality */
static double
chisqr2d (double p[6])
{
	CamPix *im = _im;
	int wrap = _wrap;
	double A  = p[0];
	double B  = p[1];
	double sx = p[2];
	double mx = p[3];
	double sy = p[4];
	double my = p[5];
	double sum2 = 0;
	int x, y;

	for (y = 0; y < _rp->rh; y++) {
	    for (x = 0; x < _rp->rw; x++) {
		double f = B + A*exp(-0.5*(sqr((x-mx)/sx) + sqr((y-my)/sy)));
		CamPix p = *im++;
		sum2 += sqr(f-p);
	    }
	    im += wrap;
	}

	return (sum2);
}

/* find best m and s in V and H directions with same A, B.
 * return 0 if ok, else -1
 */
int
gauss2fit (ImRegion *rp, Gaussian *hgp, Gaussian *vgp)
{
	double p0[6], p1[6];
	CamPix *imc;
	ImStats ims;
	int hw0i, hw1i;
	int vw0i, vw1i;
	int i, mid;

	/* get min/max/xy */
	regionStats (rp, &ims);

	/* scan from maxx/y for positions that are halfway down to min
	 *   (or edge if hit first).
	 * this is more like half-peak than half-width but it's something.
	 */
	imc = &rp->im[ims.maxaty*rp->iw + ims.maxatx];
	mid = ((int)ims.max + ims.min)/2;
	hw0i = hw1i = 0;
	for (i = 1; i < rp->rw && (!hw0i || !hw1i); i++) {
	    /* scan left/right to mid or edge */
	    if (!hw0i && (imc[-i] <= mid || i == ims.maxatx - rp->rx))
		hw0i = -i;
	    if (!hw1i && (imc[i] <= mid || i == rp->rx+rp->rw-ims.maxatx))
		hw1i = i;
	}
	vw0i = vw1i = 0;
	for (i = 1; i < rp->rh && (!vw0i || !vw1i); i++) {
	    /* scan up/down to mid or edge */
	    int dy = i*rp->iw;
	    if (!vw0i && (imc[-dy] <= mid || i == ims.maxaty - rp->ry))
		vw0i = -i;
	    if (!vw1i && (imc[dy] <= mid || i == rp->ry+rp->rh-ims.maxaty))
		vw1i = i;
	}

	/* initial estimates */
	p0[0] = ims.max-ims.min;
	p0[1] = ims.min;
	p0[2] = (hw1i - hw0i + 1)/FWHMSIG;
	p0[3] = ims.maxatx - rp->rx;
	p0[4] = (vw1i - vw0i + 1)/FWHMSIG;
	p0[5] = ims.maxaty - rp->ry;
	p1[0] = p0[0] * .9;
	p1[1] = p0[1] * .9;
	p1[2] = p0[2] * 1.1;
	p1[3] = p0[3] + 1;
	p1[4] = p0[4] * 1.1;
	p1[5] = p0[5] + 1;

	/* go */
	_rp = rp;
	_im = &_rp->im[_rp->ry*_rp->iw + _rp->rx];
	_wrap = _rp->iw - _rp->rw;
	if (lstsqr (chisqr2d, p0, p1, 6, FTOL) < 0)
	    return (-1);

	/* ok! */
	hgp->A = p0[0];
	hgp->B = p0[1];
	hgp->s = p0[2];
	hgp->m = p0[3];
	vgp->A = p0[0];
	vgp->B = p0[1];
	vgp->s = p0[4];
	vgp->m = p0[5];
	return (0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: gaussfit.c,v $ $Date: 2002/01/03 19:34:12 $ $Revision: 1.4 $ $Name:  $"};

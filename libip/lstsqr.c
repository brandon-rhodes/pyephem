/* general purpose least squares solver.
 * Uses the Amoeba solver from Numerical Recipes.
 * N.B. due to the desire to let the caller user 0-based arrays and hence our
 *   having to provide an intermediate chisqr handler, this is _NOT_ reentrant.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ip.h"

/* from Numerical Recipes */
static int amoeba(double **p, double *y, int ndim, double ftol,
    double (*funk)(), int *nfunk);

/* this lets us map 1-based arrays into 0-based arrays */
static double (*chisqr_0based)(double p[]);
static double
chisqr_1based (double p[])
{
	return ((*chisqr_0based) (p+1));
}

/* least squares solver.
 * returns number of iterations if solution converged, else -1.
 */
int
lstsqr (
double (*chisqr)(double p[]),	/* function to evaluate chisqr with at p */
double params0[],		/* in: guess: back: best */
double params1[],		/* second guess to set characteristic scale */
int np,				/* entries in params0[] and params1[] */
double ftol)			/* desired fractional tolerance */
{
	/* set up the necessary temp arrays and call the amoeba() multivariat
	 * solver. amoeba() was evidently transliterated from fortran because
	 * it indexes the arrays starting at [1].
	 */
	double **p;	/* np+1 rows, np columns (+1 due to 1-based) */
	double *y;	/* np columns ( " ) */
	int iter;
	int i, j;
	int ret;

	/* save the caller's 0-based chi sqr function */
	chisqr_0based = chisqr;

	/* fill p[][] with np+1 rows of each set of guesses of the np params.
	 * fill y[] with chisqr() for each of these sets.
	 * chisqr() is actually the function the amoeba is minimizing.
	 */
	p = (double **) malloc ((np+2)*sizeof(double *));
	y = (double *) malloc ((np+2)*sizeof(double));
	for (i = 1; i <= np+1; i++) {
	    double *pi = p[i] = (double *) malloc ((np+1)*sizeof(double));
	    for (j = 1; j <= np; j++)
		pi[j] = (j == i-1) ? params1[j-1] : params0[j-1];
	    y[i] = chisqr_1based(pi);
	}

	/* solve */
	ret = amoeba (p, y, np, ftol, chisqr_1based, &iter);

	/* on return, each row i of p has solutions at p[1..np+1][i].
	 * average them?? pick first one??
	 */
	if (ret == 0) {
	    for (i = 0; i < np; i++) {
#ifdef USE_AVERAGE
		double sum = 0.0;
		for (j = 1; j <= np+1; j++)
		    sum += p[j][i+1];
		params0[i] = sum/(np+1);
#else
		params0[i] = p[1][i+1];
#endif
	    }
	    ret = iter;
	}

	/* free the temp space */
	for (i = 1; i < np+2; i++)
	    free ((void *)p[i]);
	free ((void *)p);
	free ((void *)y);

	return (ret);
}


/* following are from Numerical Recipes in C */

static double amotry(double **p, double *y, double *psum, int ndim,
    double (*funk)(), int ihi, int *nfunk, double fac);
static void nrerror( char error_text[]);
static double *vector(int nl, int nh);
static void free_vector(double *v, int nl, int nh);

/* amoeba.c */

#define NMAX 5000
#define ALPHA 1.0
#define BETA 0.5
#define GAMMA 2.0

#define GET_PSUM for (j=1;j<=ndim;j++) { for (i=1,sum=0.0;i<=mpts;i++)\
						sum += p[i][j]; psum[j]=sum;}

static int
amoeba(p,y,ndim,ftol,funk,nfunk)
double **p,y[],ftol,(*funk)();
int ndim,*nfunk;
{
	int i,j,ilo,ihi,inhi,mpts=ndim+1;
	double ytry,ysave,sum,rtol,amotry(),*psum,*vector();
	void nrerror(),free_vector();
	double yihilo;

	psum=vector(1,ndim);
	*nfunk=0;
	GET_PSUM
	for (;;) {
		ilo=1;
		ihi = y[1]>y[2] ? (inhi=2,1) : (inhi=1,2);
		for (i=1;i<=mpts;i++) {
			if (y[i] < y[ilo]) ilo=i;
			if (y[i] > y[ihi]) {
				inhi=ihi;
				ihi=i;
			} else if (y[i] > y[inhi])
				if (i != ihi) inhi=i;
		}
		yihilo = fabs(y[ihi])+fabs(y[ilo]);
		if (yihilo == 0) break;
		rtol=2.0*fabs(y[ihi]-y[ilo])/yihilo;
		if (rtol < ftol) break;
		if (*nfunk >= NMAX) {
		    /* fprintf(stderr, "Too many iterations in AMOEBA\n"); */
		    free_vector(psum,1,ndim);
		    return(-1);
		}
		ytry=amotry(p,y,psum,ndim,funk,ihi,nfunk,-ALPHA);
		if (ytry <= y[ilo])
			ytry=amotry(p,y,psum,ndim,funk,ihi,nfunk,GAMMA);
		else if (ytry >= y[inhi]) {
			ysave=y[ihi];
			ytry=amotry(p,y,psum,ndim,funk,ihi,nfunk,BETA);
			if (ytry >= ysave) {
				for (i=1;i<=mpts;i++) {
					if (i != ilo) {
						for (j=1;j<=ndim;j++) {
							psum[j]=0.5*(p[i][j]+p[ilo][j]);
							p[i][j]=psum[j];
						}
						y[i]=(*funk)(psum);
					}
				}
				*nfunk += ndim;
				GET_PSUM
			}
		}
	}

	free_vector(psum,1,ndim);

	return (0);
}

static double
amotry(p,y,psum,ndim,funk,ihi,nfunk,fac)
double **p,*y,*psum,(*funk)(),fac;
int ndim,ihi,*nfunk;
{
	int j;
	double fac1,fac2,ytry,*ptry,*vector();
	void nrerror(),free_vector();

	ptry=vector(1,ndim);
	fac1=(1.0-fac)/ndim;
	fac2=fac1-fac;
	for (j=1;j<=ndim;j++) ptry[j]=psum[j]*fac1-p[ihi][j]*fac2;
	ytry=(*funk)(ptry);
	++(*nfunk);
	if (ytry < y[ihi]) {
		y[ihi]=ytry;
		for (j=1;j<=ndim;j++) {
			psum[j] += ptry[j]-p[ihi][j];
			p[ihi][j]=ptry[j];
		}
	}
	free_vector(ptry,1,ndim);
	return ytry;
}

#undef ALPHA
#undef BETA
#undef GAMMA
#undef NMAX


/* nrutil.c */

static void
nrerror(error_text)
char error_text[];
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}



static double *
vector(nl,nh)
int nl,nh;
{
	double *v;

	v=(double *)malloc((unsigned) (nh-nl+1)*sizeof(double));
	if (!v) nrerror("allocation failure in vector()");
	return v-nl;
}


static void
free_vector(v,nl,nh)
double *v;
int nl,nh;
{
	free((char*) (v+nl));
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: lstsqr.c,v $ $Date: 2001/12/19 21:29:48 $ $Revision: 1.3 $ $Name:  $"};

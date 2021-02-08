#ifndef _IP_H
#define _IP_H

/* include file to use the functions in libip
 */

/* data structures */

#define	FITS_HROWS	36
#define	FITS_HCOLS	80
typedef char		FITSRow[FITS_HCOLS];

typedef unsigned short CamPix;			/* C type of 16bit pixel */
#define	NCAMPIX	(1<<(int)(8*sizeof(CamPix)))	/* number of unique CamPix */
#define	MAXCAMPIX	(NCAMPIX-1)		/* largest value in a CamPix*/

typedef struct {
    /* following fields are cracked from the header for easy reference */
    int bitpix;		/* handy BITPIX */
    int sw, sh;		/* handy NAXIS1 and NAXIS2 */

    FITSRow *var;	/* malloced array of all unrecognized header lines */
    int nvar;		/* number of var[] */

    char *image;	/* malloced image array of sw*sh*(bitpix/8) bytes */
    int nbytes;		/* bytes read so far .. used for incremental loading */
    int totbytes;	/* total bytes to be read .. aka, size of image[] */

    /* cache of WCS info from recent FImage */
    int wcsset;		/* set when following are ok */
    double xref;	/* x reference coordinate value (deg) */
    double yref;	/* y reference coordinate value (deg) */
    double xrefpix;	/* x reference pixel */
    double yrefpix;	/* y reference pixel */
    double xinc;	/* x coordinate increment (deg) */
    double yinc;	/* y coordinate increment (deg) */
    double rot;		/* rotation (deg)  (from N through E) */
    char type[5];	/* projection code, see worldpos() */
} FImage;

/* "regsion": rectangular subset of an image */
typedef struct {
    CamPix *im;		/* entire 2d array */
    int iw, ih;		/* dimensions of im */
    int rx, ry;		/* ul corner */
    int rw, rh;		/* size of region */
} ImRegion;
#define ImRCenter(rp)	(((rp)->ry+(rp)->rh/2)*(rp)->iw + (rp)->rx+(rp)->rw/2)

/* stats for a region */
typedef struct {
    CamPix min, max;	/* smallest and largest pixel */
    CamPix median;	/* pixel value in middle of population */
    int maxatx, maxaty;	/* location of largtest pixel, wrt image */ 
    double mean, std;	/* mean and std of all pixels in region */
    double cmean, cstd;	/* " of central half (25..75 percentile) of pixels */
} ImStats;

/* characterization of a guassian curve */
typedef struct {
    double B;		/* base */
    double A;		/* amplitude */
    double s;		/* sigma */
    double m;		/* median position */
} Gaussian;
#define	FWHMSIG	2.355	/* FWHM/sigma */

/* definition of a star.
 * model it as 2 1d commensurate gaussians (same A and B) in x and y.
 * err is 1 sigma value of deviations at each pixel from the ideal shape.
 */
typedef struct {
    double x, y;	/* position when each m == 0 */
    Gaussian vg;	/* vertical best fit */
    Gaussian hg;	/* horizontal best fit */
    double err;		/* deviation @ STARSIGMA sigma */
} Star;
#define	STARSIGMA	3.0	/* sigmas of noise for star err */

/****************************************************************************/
/* explodegif.c */

extern int explodeGIF (unsigned char *raw, int nraw, int *wp, int *hp,
    unsigned char *pixap[], unsigned char ra[], unsigned char ga[],
    unsigned char ba[], char errmsg[]);


/****************************************************************************/
/* fits.c */

extern int writeFITS (int fd, FImage *fip, char *errmsg, int restore);
extern int writeFITSmem (FImage *fip, char **fts, char errmsg[], int restore);
extern int readFITS (int fd, FImage *fip, char *errmsg);
extern int readIncFITS (int fd, FImage *fip, char *errmsg);
extern int readFITSHeader (int fd, FImage *fip, char *errmsg);
extern int readFITSmem (char *fits, int nfits, FImage *fip, char errmsg[]);
extern int writeSimpleFITS (int fd, char *pix, int w, int h, int restore);
extern int getNAXIS (FImage *fip, int *n1p, int *n2p, char errmsg[]);
extern void initFImage (FImage *fip);
extern void resetFImage (FImage *fip);
extern void cloneFImage (FImage *new, FImage *old, int pixtoo);
extern void setSimpleFITSHeader (FImage *fip);
extern void setLogicalFITS (FImage *fip, char *name, int v, char *comment);
extern void setIntFITS (FImage *fip, char *name, int v, char *comment);
extern void setRealFITS (FImage *fip, char *name,double v,int sigdig,char *cmt);
extern void setCommentFITS (FImage *fip, char *name, char *comment);
extern void setStringFITS (FImage *fip, char *name, char *string, char *cmt);
extern int getLogicalFITS (FImage *fip, char *name, int *vp);
extern int getIntFITS (FImage *fip, char *name, int *vp);
extern int getRealFITS (FImage *fip, char *name, double *vp);
extern int getCommentFITS (FImage *fip, char *name, char *buf);
extern int getStringFITS (FImage *fip, char *name, char *string);
extern void addFImageVar (FImage *fip, FITSRow row);
extern int delFImageVar (FImage *fip, char *name);
extern int cpyFImageVar (FImage *dstfip, FImage *srcfip, char *name);

/****************************************************************************/
/* gaussfit.c */

extern int gaussfit (CamPix a[], int na, Gaussian *gp);
extern int gauss2fit (ImRegion *ip, Gaussian *hgp, Gaussian *vgp);

/****************************************************************************/
/* lstsqr.c */

extern int lstsqr (double (*chisqr)(double p[]), double params0[],
    double params1[], int np, double ftol);

/****************************************************************************/
/* median.c */

extern double dmedian (double a[], int n);
extern CamPix cmedian (CamPix a[], int n);

/****************************************************************************/
/* sqr.c */

extern double sqr (double x);

/****************************************************************************/
/* stars.c */

extern int quickStars (FImage *fip, ImStats *isp, int border, int burnt,
    double std, double **xpp, double **ypp);
extern int getStar (ImRegion *rp, Star *sp);
extern int cmpStars (Star *s1, Star *s0, double *magp, double *errp);

/****************************************************************************/
/* stats.c */

extern void regionStats (ImRegion *rp, ImStats *sp);
extern void ringStats (CamPix *ip, int imw, int r, int *minp, int *maxp,
    int *medp);
extern void starCentroid (CamPix *ip, int imw, int r, int sky, double *dxp,
    double *dyp, int *massp);
extern int clampRegion (ImRegion *rp);

/****************************************************************************/
/* walk.c */

extern void brightWalk (ImRegion *rp, int scan[8], int *bx, int *by);

/****************************************************************************/
/* wcs.c */

extern int RADec2xy(FImage *fip, double ra, double dec, double *xp, double *yp);
extern int xy2RADec(FImage *fip, double x, double y, double *rap, double *decp);

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: ip.h,v $ $Date: 2005/11/18 13:42:24 $ $Revision: 1.11 $ $Name:  $
 */

#endif /* _IP_H */

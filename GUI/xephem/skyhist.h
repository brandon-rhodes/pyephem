#ifndef _SKYHIST_H
#define _SKYHIST_H

/* glue to hook skyviewmenu.c and skyhist.c together.
 */


/* history record */
#define	GRIDLEN		9	/* useable chars in grid size TF */
typedef struct {
    double fov;			/* sv_fov value */
    double azra;		/* sv_azra value */
    double altdec;		/* sv_altdec value */
    int winw, winh;		/* window width/height */

    int stmag, ssmag, dsmag;	/* star, solsys and deep sky limiting mag */
    int magstp;			/* magnitude step size */

    int grid;			/* grid option */
    int autogrid;		/* auto grid option */
    int aagrid;			/* alt-az grid option */
    int gridlbl;		/* grid labels option */
    char vgrid[GRIDLEN+1];	/* vertical grid size text */
    char hgrid[GRIDLEN+1];	/* vertical grid size text */

    int aa_mode;		/* aa_mode value */
    int cyl_proj;		/* cyl_proj value */
    int flip_lr, flip_tb;	/* flip values */
    int justd; 			/* justdots option */
    int eclip, galac, eq;	/* ecliptic, galactic and equator options */
    int hznmap;			/* horizon map */
    int hznclipping;		/* horizon clipping option */
    int conn;			/* constellatin name opt */
    int conf, conr;		/* constellatin figures opts */
    int conb, cona;		/* constellatin boundaries, abbrev opts */
    int eyep;			/* eyepiece option */
    int magscale;		/* mag scale option */
    int automag;		/* auto mag option */

    int lbl_lst, lbl_lfs, lbl_lss, lbl_lds;	/* name/mags label flags */
    int lbl_bst, lbl_bfs, lbl_bss, lbl_bds;	/* N brightest */

    char type_table[NOBJTYPES];	/* copy of skyfilt's type table */
    char fclass_table[NCLASSES];/* copy of skyfilt's fclass table */
} SvHistory;

extern void svh_goto (SvHistory *hp);
extern void svh_get (SvHistory *hp);
extern void svh_create (Widget mb_w);
extern void svh_unmanage(void);
extern void svh_add_current(void);
extern int svh_nhist(void);


/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: skyhist.h,v $ $Date: 2012/07/07 18:04:42 $ $Revision: 1.13 $ $Name:  $
 */

#endif /* _SKYHIST_H */

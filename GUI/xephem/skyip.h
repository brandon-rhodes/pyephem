#ifndef _SKYIP_H
#define _SKYIP_H

/* zoom support */
typedef struct {
    int x0, y0, x1, y1;		/* aoi corners */
    double ad;			/* alt/dec */
    double ar;			/* az/ra */
    double fov;			/* fov */
} ZM_Undo;			/* info about each undo level */

extern FImage *si_getFImage (void);
extern Pixmap si_getPixmap (void);

extern void sf_cursor (Cursor c);
extern void sf_getName (char *filename, char *objname);
extern void sf_newFITS (FImage *fip, char name[], int autocon);
extern void sf_go_cb (Widget w, XtPointer client, XtPointer call);
extern void sf_manage (void);
extern void sf_showHeader (FImage *fip);
extern void sf_unmanage (void);

extern int si_ison (void);
extern int si_isup (void);
extern void si_create (void);
extern void si_cursor (Cursor c);
extern void si_doGauss (Display *dsp, int ww, int wh, int wx, int wy,
    int lr, int tb);
extern void si_doGlass (Display *dsp, Window win, int b1p, int m1,
    int ww, int wh, int wx, int wy, int sx, int sy, int lr, int tb);
extern void si_doROI (Display *dsp, int ww, int wh, int lr, int tb,ZM_Undo *zp);
extern void si_doSlice (Display *dsp, Window win, int state,
    int ww, int wh, int wx, int wy, int lr, int tb);
extern void si_findSnap (int ww, int wh, int wx, int wy, int lr, int tb,
    int *sxp, int *syp);
extern void si_im2win (double imx, double imy, int winw, int winh,
    int *winxp, int *winyp);
extern void si_manage (void);
extern void si_newPixmap (int w, int h, int lr, int tb, ZM_Undo *zp,int nz);
extern void si_newfim (FImage *fip, char *name, int autocon);
extern void si_off (void);
extern void si_ps (void);
extern void si_setContrast (FImage *fip);
extern void si_setPhotomRef (double ix, double iy, double newmag);
extern void si_setRefMag (double newmag);
extern void si_unmanage (void);
extern void si_win2im (int winx, int winy, int winw, int winh,
    double *imxp, double *imyp);

extern void siwcs_manage (void);
extern void siwcs_unmanage (void);

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: skyip.h,v $ $Date: 2005/11/18 13:42:59 $ $Revision: 1.4 $ $Name:  $
 */

#endif /* _SKYIP_H */

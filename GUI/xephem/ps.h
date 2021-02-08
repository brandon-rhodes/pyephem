#ifndef _PS_H
#define _PS_H

/* public include file for X postscript interface, ps.c.
 */

extern void XPSAsk (char *title, void (*go)());
extern void XPSXBegin (Window win, int Xx0, int Xy0, int Xw, int Xh, int Px0,
    int Py0, int Pw);
extern void XPSXEnd (void);
extern void XPSDirect (char *s);
extern char *XPSCleanStr (char *s, int l);
extern void XPSClose (void);
extern int XPSInColor (void);
extern int XPSDrawing (void);
extern void XPSPaperColor (unsigned long pix);

extern void XPSDrawEllipse (Display *dsp, Drawable win, GC gc, int x, int y,
    int a0, unsigned w, unsigned h, int a1, int a2);
extern void XPSFillEllipse (Display *dsp, Drawable win, GC gc, int x, int y,
    int a0, unsigned w, unsigned h, int a1, int a2);
extern void XPSDrawArc (Display *dsp, Drawable win, GC gc, int x, int y,
    unsigned w, unsigned  h, int a1, int a2);
extern void XPSDrawArcs (Display *dsp, Drawable win, GC gc, XArc xa[], int nxa);
extern void XPSDrawLine (Display *dsp, Drawable win, GC gc, int x1, int x2,
    int y1, int y2);
extern void XPSDrawLines (Display *dsp, Drawable win, GC gc, XPoint xp[],
    int nxp, int mode);
extern void XPSDrawPoint (Display *dsp, Drawable win, GC gc, int x, int y);
extern void XPSDrawPoints (Display *dsp, Drawable win, GC gc, XPoint xp[],
    int nxp, int mode);
extern void XPSDrawRectangle (Display *dsp, Drawable win, GC gc,
    int x, int y, unsigned w, unsigned h);
extern void XPSDrawRectangles (Display *dsp, Drawable win, GC gc,
    XRectangle xra[], int nxr);
extern void XPSDrawSegments (Display *dsp, Drawable win, GC gc,
    XSegment xs[], int nxs);
extern void XPSDrawString (Display *dsp, Drawable win, GC gc, int x, int y,
    char *s, int l);
extern void XPSRotDrawAlignedString (Display *dpy, XFontStruct *font,
    double angle, double mag, Drawable drawable, GC gc, int x, int y, char *str,
    int alignment);
extern void XPSDrawStar(Display *dsp, Drawable win, GC gc, int x, int y, int d);
extern void XPSFillArc (Display *dsp, Drawable win, GC gc, int x, int y,
    unsigned w, unsigned h, int a1, int a2);
extern void XPSFillArcs (Display *dsp, Drawable win, GC gc, XArc xa[], int nxa);
extern void XPSFillPolygon (Display *dsp, Drawable win, GC gc, XPoint xp[],
    int nxp, int shape, int mode);
extern void XPSFillRectangle (Display *dsp, Drawable win, GC gc,
    int x, int y, unsigned w, unsigned h);
extern void XPSFillRectangles (Display *dsp, Drawable win, GC gc,
    XRectangle xra[], int nxr);
extern void XPSPixmap (Pixmap pm, unsigned wid, unsigned hei, Colormap cm,
    GC bggc, int darken);
extern void toHSV (double r, double g, double b, double *hp, double *sp,
    double *vp);
extern void toRGB (double h, double s, double v, double *rp, double *gp,
    double *bp);


/* annotation y coord from row */
#define	ANNOT_PTSZ	8	/* font size for annotation, PS's points.
				 * if change, must redesign all annotations.
				 */
#define	AROWY(r)	(72 + (ANNOT_PTSZ+1)*(r))

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: ps.h,v $ $Date: 2006/05/07 02:11:52 $ $Revision: 1.11 $ $Name:  $
 */

#endif /* _PS_H */

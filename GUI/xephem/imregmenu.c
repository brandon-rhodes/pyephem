/* code to perform image registration.
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/TextF.h>

#include "xephem.h"

static Widget irshell_w;		/* the main shell */
static Widget l11_w;			/* label for 1st ref image 1st star */
static Widget l12_w;			/* label for 1st ref image 2nd star */
static Widget l21_w;			/* label for 2nd ref image 1st star */
static Widget l22_w;			/* label for 2nd ref image 2nd star */
static Widget s1_w;			/* step 1 TB */
static Widget s2_w;			/* step 2 TB */

typedef struct {
    double x0, y0;			/* one star coord */
    double x1, y1;			/* second star coord */
} StarPair;				/* pair of star coords */

static StarPair s0;			/* star coords on ref image */
static StarPair s1;			/* star coords on image to be register*/

typedef struct {
    double A, B, C, D, E, F;		/* x' = Ax+By+C y' = Dx+Ey+F */
    double scale, theta;		/* scale and rotation (rads) */
} XForm;				/* linear 2D transform */

/* these steps follow the user through the process.
 */
typedef enum {
    S11,				/* select first star on ref image */
    S12,				/* select 2nd star on ref image */
    S21,				/* select 1st star on reg image */
    S22					/* select 2nd star on reg image and go*/
} RefStep;
static RefStep step;			/* step of procedure */

#define	RSZ	5			/* search region for star, pixels */

#define	SQR(x)	((x)*(x))		/* handy */

static char ircategory[] = "Image registration";      /* Save category */

static void ir_create (void);
static void ir_help_cb (Widget w, XtPointer client, XtPointer call);
static void ir_close_cb (Widget w, XtPointer client, XtPointer call);
static void ref_cb (Widget w, XtPointer client, XtPointer call);
static void reg_cb (Widget w, XtPointer client, XtPointer call);
static void regCursor (int whether);
static void find2dXform (StarPair *s0, StarPair *to, XForm *xf);
static void imXform (FImage *s, FImage *d, XForm *xf);
static void wcsXform (FImage *im, StarPair *s0, StarPair *s1, XForm *xf);
static void doreg(void);

void
ir_manage()
{
	if (!irshell_w)
	    ir_create();

        XtManageChild (irshell_w);
}

/* return whether we are currently looking for a star coord to set a ref star */
int
ir_setting()
{
	return ((s1_w && XmToggleButtonGetState(s1_w))
			    || (s1_w && XmToggleButtonGetState(s2_w)));
}

/* called when user clicks on a star.
 * we find the gaussian star center, draw a confirmation circle, then use this
 * star depending on which step we are in the procedure.
 */
void
ir_setstar (double ix, double iy)
{
	FImage *im = si_getFImage();
	char buf[128];
	ImRegion r;
	Star s;

	r.im = (CamPix *) im->image;
	r.iw = im->sw;
	r.ih = im->sh;
	r.rx = (int)(ix - RSZ/2);
	if (r.rx < 0)
	    r.rx = 0;
	r.ry = (int)(iy - RSZ/2);
	if (r.ry < 0)
	    r.ry = 0;
	r.rw = RSZ;
	r.rh = RSZ;

	getStar (&r, &s);
	sv_drawimdot (s.x, s.y, 5, 1);

	switch (step) {
	case S11:
	    sprintf (buf, "Star 1 at %7.2f %7.2f", s.x, s.y);
	    set_xmstring (l11_w, XmNlabelString, buf);
	    step = S12;
	    s0.x0 = s.x;
	    s0.y0 = s.y;
	    break;
	case S12:
	    sprintf (buf, "Star 2 at %7.2f %7.2f", s.x, s.y);
	    set_xmstring (l12_w, XmNlabelString, buf);
	    XmToggleButtonSetState (s1_w, False, True);
	    s0.x1 = s.x;
	    s0.y1 = s.y;
	    break;
	case S21:
	    sprintf (buf, "Star 1 at %7.2f %7.2f", s.x, s.y);
	    set_xmstring (l21_w, XmNlabelString, buf);
	    step = S22;
	    s1.x0 = s.x;
	    s1.y0 = s.y;
	    break;
	case S22:
	    sprintf (buf, "Star 2 at %7.2f %7.2f", s.x, s.y);
	    set_xmstring (l22_w, XmNlabelString, buf);
	    XmToggleButtonSetState (s2_w, False, True);
	    s1.x1 = s.x;
	    s1.y1 = s.y;
	    doreg();
	    break;
	}
}

/* called to put up or remove the watch cursor.  */
void
ir_cursor (c)
Cursor c;
{
	Window win;

	if (irshell_w && (win = XtWindow(irshell_w)) != 0) {
	    Display *dsp = XtDisplay(irshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

static void
ir_create()
{
	Widget w, sep_w;
	Arg args[20];
	int n;

	/* create main form */

	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 4); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	irshell_w = XmCreateFormDialog (svshell_w, "ImReg", args, n);
	set_something (irshell_w, XmNcolormap, (XtArgVal)xe_cm);
	set_xmstring (irshell_w, XmNdialogTitle, "Image registration");
        XtAddCallback (irshell_w, XmNhelpCallback, ir_help_cb, NULL);
	sr_reg (XtParent(irshell_w), "XEphem*ImReg.x", ircategory, 0);
	sr_reg (XtParent(irshell_w), "XEphem*ImReg.y", ircategory, 0);

	/* first step */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	s1_w = XmCreateToggleButton (irshell_w, "S1TB", args, n);
	XtAddCallback (s1_w, XmNvalueChangedCallback, ref_cb, NULL);
	set_xmstring (s1_w, XmNlabelString,
				"Reference image is loaded, now click 2 stars");
	XtManageChild (s1_w);

	/* labels for first set of reference stars */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, s1_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 30); n++;
	l11_w = XmCreateLabel (irshell_w, "R11L", args, n);
	set_xmstring (l11_w, XmNlabelString, "Star 1 at");
	XtManageChild (l11_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, l11_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 30); n++;
	l12_w = XmCreateLabel (irshell_w, "R12L", args, n);
	set_xmstring (l12_w, XmNlabelString, "Star 2 at");
	XtManageChild (l12_w);

	/* second step */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, l12_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	s2_w = XmCreateToggleButton (irshell_w, "S1TB", args, n);
	XtAddCallback (s2_w, XmNvalueChangedCallback, reg_cb, NULL);
	set_xmstring (s2_w, XmNlabelString,
		    "Image to be registed is loaded, now click same 2 stars");
	XtManageChild (s2_w);

	/* labels for second set of reference stars */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, s2_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 30); n++;
	l21_w = XmCreateLabel (irshell_w, "R21L", args, n);
	set_xmstring (l21_w, XmNlabelString, "Star 1 at");
	XtManageChild (l21_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, l21_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 30); n++;
	l22_w = XmCreateLabel (irshell_w, "R22L", args, n);
	set_xmstring (l22_w, XmNlabelString, "Star 2 at");
	XtManageChild (l22_w);

	/* controls at the bottom */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, l22_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (irshell_w, "HZS", args, n);
	XtManageChild (sep_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 22); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 38); n++;
	w = XmCreatePushButton (irshell_w, "Close", args, n);
        XtAddCallback (w, XmNactivateCallback, ir_close_cb, NULL);
	wtip (w, "Close this window");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 62); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 78); n++;
	w = XmCreatePushButton (irshell_w, "Help", args, n);
        XtAddCallback (w, XmNactivateCallback, ir_help_cb, NULL);
	wtip (w, "Get more information about this window");
	XtManageChild (w);
}

/* called from Close */
/* ARGSUSED */
static void
ir_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (irshell_w);
}

/* called from Help */
/* ARGSUSED */
static void
ir_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
        static char *msg[] = {"Define reference stars then each additional."};

	hlp_dialog ("Registration", msg, sizeof(msg)/sizeof(msg[0]));

}

/* called when TB clicked to start working with reference image */
/* ARGSUSED */
static void
ref_cb (Widget w, XtPointer client, XtPointer call)
{
	int on = XmToggleButtonGetState(w);

	if (on) {
	    FImage *im1 = si_getFImage();

	    if (!im1) {
		xe_msg (1, "Please load your reference image");
		XmToggleButtonSetState (w, False, False);
		return;
	    }

	    set_xmstring (l11_w, XmNlabelString, "Star 1 at");
	    set_xmstring (l12_w, XmNlabelString, "Star 2 at");
	    sv_all (NULL);		/* clean */
	    regCursor (1);
	    step = S11;
	} else {
	    regCursor (0);
	}
}

/* called when TB clicked to start working with an image to be registered */
/* ARGSUSED */
static void
reg_cb (Widget w, XtPointer client, XtPointer call)
{
	int on = XmToggleButtonGetState(w);

	if (on) {
	    FImage *im1 = si_getFImage();

	    if (!im1) {
		xe_msg (1, "Please load your reference image");
		XmToggleButtonSetState (w, False, False);
		return;
	    }

	    set_xmstring (l21_w, XmNlabelString, "Star 1 at");
	    set_xmstring (l22_w, XmNlabelString, "Star 2 at");
	    sv_all (NULL);		/* clean */
	    regCursor (1);
	    step = S21;
	} else {
	    regCursor (0);
	}
}

/* whether to show a special registration cursor in sky view */
static void
regCursor (int whether)
{
	static Cursor rc;

	if (!rc)
	    rc = XCreateFontCursor (XtD, XC_crosshair);

	sv_cursor (whether ? rc : 0);
}

/* find 2d linear transform of the form x' = Ax+By+C y' = Dx+Ey+F that
 *   transforms coordinate pairs s0 to s1.
 * these equations were derived from working through the matrix ops to:
 *   translate s0's x0 to the origin,
 *   rotate by the angle between s1 and s0,
 *   scale by the ratio of s1 to s0,
 *   translate by s1's x0 position.
 */
static void
find2dXform (StarPair *s0, StarPair *s1, XForm *xf)
{
	double Tx, Ty;
	double stheta, ctheta;
	double Txp, Typ;

	/* translate to origin */
	Tx = -s0->x0;
	Ty = -s0->y0;

	/* rotate */
	xf->theta = atan2 (s1->y1 - s1->y0, s1->x1 - s1->x0)
				    - atan2 (s0->y1 - s0->y0, s0->x1 - s0->x0);
	stheta = sin(xf->theta);
	ctheta = cos(xf->theta);

	/* scale */
	xf->scale = sqrt((SQR(s1->y1 - s1->y0) + SQR(s1->x1 - s1->x0)) /
				(SQR(s0->y1 - s0->y0) + SQR(s0->x1 - s0->x0)));

	/* translate to s1 */
	Txp = s1->x0;
	Typ = s1->y0;

	/* done */
	xf->A = xf->scale*ctheta;
	xf->B = -xf->scale*stheta;
	xf->C = xf->scale*(Tx*ctheta - Ty*stheta) + Txp;
	xf->D = xf->scale*stheta;
	xf->E = xf->scale*ctheta;
	xf->F = xf->scale*(Tx*stheta + Ty*ctheta) + Typ;
}

/* transform s to d, using xf.
 */
static void
imXform (FImage *s, FImage *d, XForm *xf)
{
	CamPix *sp = (CamPix *) s->image;
	CamPix *dp = (CamPix *) d->image;
	int x, y;

	memset (d->image, 0, d->sw*d->sh*sizeof(CamPix));
	for (y = 0; y < d->sh; y++) {
	    for (x = 0; x < d->sw; x++) {
		int xp = (int)floor(xf->A*x + xf->B*y + xf->C + 0.5);
		int yp = (int)floor(xf->D*x + xf->E*y + xf->F + 0.5);
		if (xp>=0 && xp<s->sw && yp>=0 && yp<s->sh)
		    *dp = sp[xp + yp*s->sw];
		else
		    *dp = 0;
		dp++;
	    }
	}
}

/* update WCS in im, if any, according to pixel shift from s0 to s1 via xf */
static void
wcsXform (FImage *im, StarPair *s0, StarPair *s1, XForm *xf)
{
	double r1, d1;

	/* find ra/dec of first star in s1 */
	if (xy2RADec (im, s1->x0, s1->y0, &r1, &d1) < 0)
	    return;

	/* new WCS, use s0 as new reference */
	im->xref = raddeg(r1);
	im->yref = raddeg(d1);
	im->xrefpix = s0->x0;
	im->yrefpix = s0->y0;
	im->xinc *= xf->scale;
	im->yinc *= xf->scale;
	im->rot -= raddeg(xf->theta);

	setRealFITS (im, "CRVAL1", im->xref, 10, "deg ra@ref");
	setRealFITS (im, "CRVAL2", im->yref, 10, "deg dec@ref");
	setRealFITS (im, "CRPIX1", im->xrefpix, 10, "ref pix x");
	setRealFITS (im, "CRPIX2", im->yrefpix, 10, "ref pix y");
	setRealFITS (im, "CDELT1", im->xinc, 10, "deg right/pix");
	setRealFITS (im, "CDELT2", im->yinc, 10, "deg up/pix");
	setRealFITS (im, "CROTA1", im->rot,  10, "deg rot");
	setRealFITS (im, "CROTA2", im->rot,  10, "deg rot");
}

/* affect the registration process on the currently displayed image */
static void
doreg()
{
	FImage *s = si_getFImage();	/* image to be registered */
	FImage new, *d = &new;
	XForm xf;

	cloneFImage (d, s, 0);
	find2dXform (&s0, &s1, &xf);
	imXform (s, d, &xf);
	wcsXform (d, &s0, &s1, &xf);

	sf_newFITS (d, "registered.fts", 0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: imregmenu.c,v $ $Date: 2005/11/27 16:19:25 $ $Revision: 1.6 $ $Name:  $"};

/* stuff for the eyepiece dialog */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Scale.h>
#include <Xm/SelectioB.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>

#include "xephem.h"
#include "lilxml.h"

/* a list of EyePieces */
static EyePiece *eyep;		/* malloced list of eyepieces */
static int neyep;		/* number of entries off eyep */

#define	MXSCALE degrad(90)	/* max angle to show */
#define	LOSCALE	degrad(1./60)	/* rads/step at low end */
#define	HISCALE	degrad(10./60)	/* rads/step at hi end */
#define	CHSCALE degrad(10.)	/* where they switch */
#define	SCALEMAX	((int)(CHSCALE/LOSCALE + (MXSCALE-CHSCALE)/HISCALE))

/* favorites are managed in an array which is never shuffled. Entries are
 * reassigned when needed from unused entries or the array is expanded.
 * callbacks have the array index in their client param.
 */
#define	MAXEPNM		60	/* maximum name for eyepiece (including \0) */
#define	MAXEPNMD	21	/* maximum name displayed */
typedef struct {
    int inuse;			/* whether this entry is in use */
    char name[MAXEPNM];		/* use's name */
    double w, h;		/* size, rads */
    int isE;			/* whether Elliptical or Rectangular */
    int isS;			/* whether Solid or Outline */
    Widget row_w;		/* widget with GUI controls for this entry */
    Widget name_w;		/* TF with name */
} FavEyeP;
static FavEyeP *favs;		/* malloced array of favorite eyepieces */
static int nfavs;		/* number of entries (regardless of inuse) */
static Widget favrc_w;		/* RC for listing favorites */
static Widget saved_w;		/* TF for name of file to save definitions */
static Widget savep_w;		/* TF for name of file to save positions */

static char defeyepfn[] = "eyepieces.epd";	/* default defs file name */
static char eyep_suffix[] = ".epd";		/* def file name suffix */
static char defeyelfn[] = "eyepieces.epp";	/* default positions file name*/
static char eyel_suffix[] = ".epp";		/* positions file name suffix */

static Widget eyep_w;		/* overall eyepiece dialog */
static Widget eyepws_w;		/* eyepiece width scale */
static Widget eyephs_w;		/* eyepiece height scale */
static Widget eyepwl_w;		/* eyepiece width label */
static Widget eyepa_w;		/* eyepiece rotation angle, scale */
static Widget eyepal_w;		/* eyepiece rotation angle label */
static Widget eyephl_w;		/* eyepiece height label */
static Widget eyer_w;		/* eyepiece Round TB */
static Widget eyes_w;		/* eyepiece Square TB */
static Widget eyef_w;		/* eyepiece filled TB */
static Widget eyeb_w;		/* eyepiece border TB */
static Widget telrad_w;		/* telrad on/off TB */
static Widget delep_w;		/* the delete all PB */
static Widget lock_w;		/* lock scales TB */
static Widget sa1_w;		/* sky angle L for formula 1 results */
static Widget sa2_w;		/* sky angle L for formula 2 results */
static Widget fl_w;		/* focal length TF */
static Widget fp_w;		/* focal plane length TF */
static Widget afov_w;		/* apparent eyepiece fov TF */
static Widget efl_w;		/* eyepiece focal length TF */
static Widget mfl_w;		/* mirror focal length TF */

static void se_create_eyep_w (void);
static void se_eyepsz (double *wp, double *hp, double *pp, int *rp, int *fp);
static void se_scale_fmt (Widget s_w, Widget l_w);
static void se_pascale_fmt (Widget s_w, Widget l_w);
static void se_telrad_cb (Widget w, XtPointer client, XtPointer call);
static void se_skyW_cb (Widget w, XtPointer client, XtPointer call);
static void se_skyH_cb (Widget w, XtPointer client, XtPointer call);
static void se_wscale_cb (Widget w, XtPointer client, XtPointer call);
static void se_hscale_cb (Widget w, XtPointer client, XtPointer call);
static void se_pscale_cb (Widget w, XtPointer client, XtPointer call);
static void se_delall_cb (Widget w, XtPointer client, XtPointer call);
static void se_close_cb (Widget w, XtPointer client, XtPointer call);
static void se_addfav_cb (Widget w, XtPointer client, XtPointer call);
static void se_saved_cb (Widget w, XtPointer client, XtPointer call);
static void se_loadd_cb (Widget w, XtPointer client, XtPointer call);
static void se_savep_cb (Widget w, XtPointer client, XtPointer call);
static void se_loadp_cb (Widget w, XtPointer client, XtPointer call);
static void se_delfav_cb (Widget w, XtPointer client, XtPointer call);
static void se_usefav_cb (Widget w, XtPointer client, XtPointer call);
static void se_help_cb (Widget w, XtPointer client, XtPointer call);
static void se_calc1_cb (Widget w, XtPointer client, XtPointer call);
static void se_calc1 (void);
static void se_calc2_cb (Widget w, XtPointer client, XtPointer call);
static void se_calc2 (void);
static void se_delall (void);
static EyePiece *se_addeyep (void);
static double se_getScale (Widget w);
static void se_setScale (Widget w, double a);
static double se_getPScale (Widget w);
static void se_addFavEyeP (FavEyeP *newfp);
static void se_loadEyep (char *fn);
static void se_saveFav (char *fn);
static void se_loadPos (char *fn);
static void se_savePos (char *fn);

/* telrad circle diameters, degrees */
static double telrad_sz[] = {.5, 2., 4.};

static char skyepcategory[] = "Sky View -- Eyepieces";

void 
se_manage()
{
	if (!eyep_w)
	    se_create_eyep_w();
	XtManageChild (eyep_w);
}

void 
se_unmanage()
{
	if (eyep_w)
	    XtUnmanageChild (eyep_w);
}

/* called to put up or remove the watch cursor.  */
void
se_cursor (c)
Cursor c;
{
	Window win;

	if (eyep_w && (win = XtWindow(eyep_w)) != 0) {
	    Display *dsp = XtDisplay(eyep_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* add one eyepiece with the current definition for the given location */
void
se_add (int aamode, double azra, double altdec)
{
	int telrad;
	int nnew;

	/* check for first time */
	if (!eyep_w)
	    se_create_eyep_w();

	/* add 1 or 3 if telrad */
	telrad = XmToggleButtonGetState (telrad_w);
	nnew = telrad ? 3 : 1;

	/* fill in the details */
	while (--nnew >= 0) {
	    EyePiece *new = se_addeyep();
	    new->azra = azra;
	    new->altdec = altdec;
	    new->aamode = aamode;
	    if (telrad) {
		new->eyepw = degrad(telrad_sz[nnew]);
		new->eyeph = degrad(telrad_sz[nnew]);
		new->eyepa = 0;
		new->round = 1;
		new->solid = 0;
	    } else
		se_eyepsz (&new->eyepw, &new->eyeph, &new->eyepa, &new->round, &new->solid);
	}

	/* at least one to delete now */
	XtSetSensitive (delep_w, True);
}

/* return whether there are any eyepieces that cover the given location,
 * aamode determining the interpretation of the coords.
 */
int
se_isOneHere (int aamode, double azra, double altdec)
{
	double caltdec = cos(altdec);
	double saltdec = sin(altdec);
	int i;

	for (i = 0; i < neyep; i++) {
	    EyePiece *ep = &eyep[i];
	    double L, l, csep, maxr;

	    if (aamode == ep->aamode) {
		L = ep->azra;
		l = ep->altdec;
	    } else {
		sv_other (ep->altdec, ep->azra, ep->aamode, &l, &L);
	    }

	    solve_sphere (azra-L, PI/2-l, saltdec, caltdec, &csep, NULL);
	    maxr = (ep->eyepw < ep->eyeph ? ep->eyepw : ep->eyeph)/2;
	    if (acos(csep) < maxr)
		return (1);	/* yes, there is */
	}

	/* none found */
	return (0);
}

/* delete eyepiece that most closely covers the given location.
 * aamode determines how to interpret the coords.
 */
void
se_del (int aamode, double azra, double altdec)
{
	double caltdec, saltdec;
	double r, maxcsep;
	EyePiece *ep, *endep, *minep;

	/* scan for eyepiece closest to target position, leave in minep */
	caltdec = cos(altdec);
	saltdec = sin(altdec);
	maxcsep = 0;
	endep = &eyep[neyep];
	minep = NULL;
	for (ep = eyep; ep < endep; ep++) {
	    double L, l, csep;

	    if (aamode == ep->aamode) {
		L = ep->azra;
		l = ep->altdec;
	    } else {
		sv_other (ep->altdec, ep->azra, ep->aamode, &l, &L);
	    }

	    solve_sphere (azra-L, PI/2-l, saltdec, caltdec, &csep, NULL);
	    if (csep > maxcsep) {
		maxcsep = csep;
		minep = ep;
	    }
	}
	if (!minep)
	    return;

	/* if actually under eyepiece, remove it */
	r = (minep->eyepw < minep->eyeph ? minep->eyepw : minep->eyeph)/2;
	if (maxcsep > cos(r)) {
	    while (++minep < endep)
		minep[-1] = minep[0];

	    /* drop count, but leave array.. likely grows again anyway */
	    neyep--;

	    /* may have no more left now! */
	    XtSetSensitive (delep_w, neyep);
	}
}


/* return the list of current eyepieces, if interested, and the count.
 */
int
se_getlist (EyePiece **ep)
{
	if (ep)
	    *ep = eyep;
	if (delep_w)
	    XtSetSensitive (delep_w, neyep);
	return (neyep);
}

/* fetch the current eyepiece size and rotation angle, all in rads,
 * whether it is round, and whether it is filled from the dialog.
 */
static void
se_eyepsz(double *wp, double *hp, double *ap, int *rp, int *fp)
{
	if (!eyep_w)
	    se_create_eyep_w();

	*wp = se_getScale (eyepws_w);
	*hp = se_getScale (eyephs_w);
	*ap = se_getPScale (eyepa_w);

	*rp = XmToggleButtonGetState (eyer_w);
	*fp = XmToggleButtonGetState (eyef_w);
}

/* increase size of eyep[] by one and return pointer to new location */
static EyePiece *
se_addeyep ()
{
	eyep = (EyePiece *) XtRealloc ((void*)eyep, (neyep+1)*sizeof(EyePiece));
	return (&eyep[neyep++]);
}

/* create the eyepiece size dialog */
static void
se_create_eyep_w()
{
	Widget w, sep_w;
	Widget l_w, rb_w;
	Widget pb_w, sfm_w;
	Arg args[20];
	char *s[1];
	int n;

	/* create form */

	n = 0;
	XtSetArg(args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	eyep_w = XmCreateFormDialog (svshell_w, "SkyEyep", args, n);
	set_something (eyep_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (eyep_w, XmNhelpCallback, se_help_cb, 0);
	sr_reg (XtParent(eyep_w), "XEphem*SkyEyep.x", skyepcategory, 0);
	sr_reg (XtParent(eyep_w), "XEphem*SkyEyep.y", skyepcategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Eyepiece Setup"); n++;
	XtSetValues (XtParent(eyep_w), args, n);

	/* title label */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	w = XmCreateLabel (eyep_w, "L", args, n);
	set_xmstring (w, XmNlabelString, "Set next eyepiece size, angle, shape and style:");
	XtManageChild (w);

	/* w scale and its labels */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	eyepwl_w = XmCreateLabel (eyep_w, "EyepWL", args, n);
	XtManageChild (eyepwl_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 20); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNshowValue, False); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNminimum, 1); n++;
	XtSetArg (args[n], XmNmaximum, SCALEMAX); n++;
	eyepws_w = XmCreateScale (eyep_w, "EyepW", args, n);
	XtAddCallback (eyepws_w, XmNdragCallback, se_wscale_cb, 0);
	XtAddCallback (eyepws_w, XmNvalueChangedCallback, se_wscale_cb, 0);
	wtip (eyepws_w, "Slide to desired width of eyepiece, D:M");
	sr_reg (eyepws_w, NULL, skyepcategory, 0);
	se_scale_fmt (eyepws_w, eyepwl_w);
	XtManageChild (eyepws_w);

	/* h scale and its label */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, eyepws_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	eyephl_w = XmCreateLabel (eyep_w, "EyepHL", args, n);
	XtManageChild (eyephl_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, eyepws_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 20); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNshowValue, False); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNminimum, 1); n++;
	XtSetArg (args[n], XmNmaximum, SCALEMAX); n++;
	eyephs_w = XmCreateScale (eyep_w, "EyepH", args, n);
	XtAddCallback (eyephs_w, XmNdragCallback, se_hscale_cb, 0);
	XtAddCallback (eyephs_w, XmNvalueChangedCallback, se_hscale_cb, 0);
	wtip (eyephs_w, "Slide to desired height of eyepiece, D:M");
	sr_reg (eyephs_w, NULL, skyepcategory, 0);
	se_scale_fmt (eyephs_w, eyephl_w);
	XtManageChild (eyephs_w);

	/* angle scale and its label */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, eyephs_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	eyepal_w = XmCreateLabel (eyep_w, "EyepPL", args, n);
	XtManageChild (eyepal_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, eyephs_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 20); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNshowValue, False); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNmaximum, 359); n++;
	eyepa_w = XmCreateScale (eyep_w, "EyepA", args, n);
	XtAddCallback (eyepa_w, XmNdragCallback, se_pscale_cb, 0);
	XtAddCallback (eyepa_w, XmNvalueChangedCallback, se_pscale_cb, 0);
	wtip (eyepa_w, "Rotation angle of eyepiece, from Z if Alt/Az else from NCP");
	sr_reg (eyepa_w, NULL, skyepcategory, 0);
	se_pascale_fmt (eyepa_w, eyepal_w);
	XtManageChild (eyepa_w);

	/* lock TB */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, eyepa_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	lock_w = XmCreateToggleButton (eyep_w, "Lock", args, n);
	set_xmstring (lock_w, XmNlabelString, "Lock W and H together");
	wtip (lock_w, "When on, width and height scales move as one");
	XtManageChild (lock_w);
	sr_reg (lock_w, NULL, skyepcategory, 0);

	/* telrad TB */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, lock_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	telrad_w = XmCreateToggleButton (eyep_w, "Telrad", args, n);
	XtAddCallback (telrad_w, XmNvalueChangedCallback, se_telrad_cb, NULL);
	set_xmstring (telrad_w, XmNlabelString, "Telrad circles of 0.5, 2 and 4° diameter");
	wtip (telrad_w, "When on, next eyepiece will be 3 open circles matching the Telrad.");
	XtManageChild (telrad_w);
	sr_reg (telrad_w, NULL, skyepcategory, 0);

	/* shape label */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, telrad_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	l_w = XmCreateLabel (eyep_w, "S", args, n);
	set_xmstring (l_w, XmNlabelString, "Shape:");
	XtManageChild (l_w);

	/* round or square Radio box */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, telrad_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, l_w); n++;
	XtSetArg (args[n], XmNleftOffset, 5); n++;
	rb_w = XmCreateRadioBox (eyep_w, "RSRB", args, n);
	XtManageChild (rb_w);

	    n = 0;
	    eyer_w = XmCreateToggleButton (rb_w, "Elliptical", args, n);
	    wtip (eyer_w, "When on, next eyepiece will be elliptical");
	    XtManageChild (eyer_w);
	    sr_reg (eyer_w, NULL, skyepcategory, 0);

	    n = 0;
	    eyes_w = XmCreateToggleButton (rb_w, "Rectangular", args, n);
	    wtip (eyes_w, "When on, next eyepiece will be rectangular");
	    XtManageChild (eyes_w);

	    /* "Elliptical" establishes truth setting */
	    XmToggleButtonSetState (eyes_w, !XmToggleButtonGetState(eyer_w), 0);

	/* style label */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, telrad_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 50); n++;
	l_w = XmCreateLabel (eyep_w, "St", args, n);
	set_xmstring (l_w, XmNlabelString, "Style:");
	XtManageChild (l_w);

	/* style Radio box */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, telrad_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, l_w); n++;
	XtSetArg (args[n], XmNleftOffset, 5); n++;
	rb_w = XmCreateRadioBox (eyep_w, "FBRB", args, n);
	XtManageChild (rb_w);

	    n = 0;
	    eyef_w = XmCreateToggleButton (rb_w, "Solid", args, n);
	    wtip (eyef_w, "When on, next eyepiece will be solid");
	    XtManageChild (eyef_w);
	    sr_reg (eyef_w, NULL, skyepcategory, 0);

	    n = 0;
	    eyeb_w = XmCreateToggleButton (rb_w, "Outline", args, n);
	    wtip (eyeb_w, "When on, next eyepiece will be just a border");
	    XtManageChild (eyeb_w);

	    /* "Solid" establishes truth setting */
	    XmToggleButtonSetState (eyeb_w, !XmToggleButtonGetState(eyef_w), 0);

	/* calculator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNseparatorType, XmDOUBLE_LINE); n++;
	sep_w = XmCreateSeparator (eyep_w, "Sep", args, n);
	XtManageChild (sep_w);

	    /* title */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    l_w = XmCreateLabel (eyep_w, "EL", args, n);
	    set_xmstring(l_w,XmNlabelString,"Field-of-View Calculators");
	    XtManageChild (l_w);

	    /* formula # 1 */

	    /* labels */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 28); n++;
	    w = XmCreateLabel (eyep_w, "EL", args, n);
	    set_xmstring (w, XmNlabelString, "Focal length\n(mm) m in");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 30); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 58); n++; /*first includes 10*/
	    w = XmCreateLabel (eyep_w, "EL", args, n);
	    set_xmstring (w, XmNlabelString, "F Plane Size\n(µm) mm in");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 60); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 88); n++;
	    w = XmCreateLabel (eyep_w, "EL", args, n);
	    set_xmstring (w, XmNlabelString, "Sky angle\nD:M:S");
	    XtManageChild (w);

	    /* TFs */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 28); n++;
	    XtSetArg (args[n], XmNcolumns, 8); n++;
	    fl_w = XmCreateTextField (eyep_w, "FocalLength", args, n);
	    wtip (fl_w, "Enter effective focal length, specifying units of mm, m or inches");
	    sr_reg (fl_w, NULL, skyepcategory, 0);
	    XtManageChild (fl_w);
	    XtAddCallback (fl_w, XmNvalueChangedCallback, se_calc1_cb, NULL);
	    XtAddCallback (fl_w, XmNactivateCallback, se_calc1_cb, NULL);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 30); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 58); n++;
	    XtSetArg (args[n], XmNcolumns, 8); n++;
	    fp_w = XmCreateTextField (eyep_w, "FPlaneSize", args, n);
	    wtip (fp_w, "Enter size of object on focal plane, in microns, mm or inches");
	    sr_reg (fp_w, NULL, skyepcategory, 0);
	    XtManageChild (fp_w);
	    XtAddCallback (fp_w, XmNvalueChangedCallback, se_calc1_cb, NULL);
	    XtAddCallback (fp_w, XmNactivateCallback, se_calc1_cb, NULL);

	    /* formula #1 sky angle result label */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 60); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 88); n++;
	    sa1_w = XmCreateLabel (eyep_w, "SAL", args, n);
	    wtip (sa1_w, "Sky angle with given focal length and plane size");
	    set_xmstring (sa1_w, XmNlabelString, "xxx:xx:xx.x");
	    XtManageChild (sa1_w);

	    /* formula #1 set w and h */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 90); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (eyep_w, "Set", args, n);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNtopOffset, 1); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 90); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    w = XmCreatePushButton (eyep_w, "W", args, n);
	    wtip (w, "Set eyepiece width scale to this Sky angle");
	    XtAddCallback (w, XmNactivateCallback, se_skyW_cb,(XtPointer)sa1_w);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNtopOffset, 1); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 90); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    w = XmCreatePushButton (eyep_w, "H", args, n);
	    wtip (w, "Set eyepiece height scale to this Sky angle");
	    XtAddCallback (w, XmNactivateCallback, se_skyH_cb,(XtPointer)sa1_w);
	    XtManageChild (w);

	    /* formula #2 */

	    /* labels */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, fp_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 19); n++;
	    l_w = XmCreateLabel (eyep_w, "EL", args, n);
	    set_xmstring (l_w, XmNlabelString, "Apparent\nFOV, °");
	    XtManageChild (l_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, fp_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 21); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 38); n++; /*first includes 10*/
	    w = XmCreateLabel (eyep_w, "EL", args, n);
	    set_xmstring (w, XmNlabelString, "Eyepiece\nFL, mm");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, fp_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 40); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 58); n++;
	    w = XmCreateLabel (eyep_w, "EL", args, n);
	    set_xmstring (w, XmNlabelString, "Mirror FL\n(mm) m in");
	    XtManageChild (w);

	    /* TFs */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 19); n++;
	    XtSetArg (args[n], XmNcolumns, 8); n++;
	    afov_w = XmCreateTextField (eyep_w, "ApparentFOV", args, n);
	    wtip (afov_w,
	       "Enter apparent field of view through the eyepiece, in degrees");
	    sr_reg (afov_w, NULL, skyepcategory, 0);
	    XtManageChild (afov_w);
	    XtAddCallback (afov_w, XmNvalueChangedCallback, se_calc2_cb, NULL);
	    XtAddCallback (afov_w, XmNactivateCallback, se_calc2_cb, NULL);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 21); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 38); n++;
	    XtSetArg (args[n], XmNcolumns, 8); n++;
	    efl_w = XmCreateTextField (eyep_w, "EyepieceFL", args, n);
	    wtip (efl_w, "Enter focal length of eyepiece, in millimeters");
	    sr_reg (efl_w, NULL, skyepcategory, 0);
	    XtManageChild (efl_w);
	    XtAddCallback (efl_w, XmNvalueChangedCallback, se_calc2_cb, NULL);
	    XtAddCallback (efl_w, XmNactivateCallback, se_calc2_cb, NULL);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 40); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 58); n++;
	    XtSetArg (args[n], XmNcolumns, 8); n++;
	    mfl_w = XmCreateTextField (eyep_w, "MirrorFL", args, n);
	    wtip (mfl_w, "Enter focal length of primary mirror, in units of mm, m or inches");
	    sr_reg (mfl_w, NULL, skyepcategory, 0);
	    XtManageChild (mfl_w);
	    XtAddCallback (mfl_w, XmNvalueChangedCallback, se_calc2_cb, NULL);
	    XtAddCallback (mfl_w, XmNactivateCallback, se_calc2_cb, NULL);

	    /* formula #2 sky angle result label */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 60); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 88); n++;
	    sa2_w = XmCreateLabel (eyep_w, "SAL", args, n);
	    wtip (sa2_w, "Sky angle with given eyepiece and mirror");
	    set_xmstring (sa2_w, XmNlabelString, "xxx:xx:xx.x");
	    XtManageChild (sa2_w);

	    /* formula #2 set w and h */

	    n = 0;
	    XtSetArg (args[n],XmNbottomAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
	    XtSetArg (args[n], XmNbottomWidget, l_w); n++;
	    XtSetArg (args[n], XmNbottomOffset, 0); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 90); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    w = XmCreatePushButton (eyep_w, "W", args, n);
	    wtip (w, "Set eyepiece width scale to this Sky angle");
	    XtAddCallback (w, XmNactivateCallback, se_skyW_cb,(XtPointer)sa2_w);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNtopOffset, 1); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 90); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    w = XmCreatePushButton (eyep_w, "H", args, n);
	    wtip (w, "Set eyepiece height scale to this Sky angle");
	    XtAddCallback (w, XmNactivateCallback, se_skyH_cb,(XtPointer)sa2_w);
	    XtManageChild (w);

	/* favorites */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mfl_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNseparatorType, XmDOUBLE_LINE); n++;
	sep_w = XmCreateSeparator (eyep_w, "Sep", args, n);
	XtManageChild (sep_w);

	    /* title and "Add" */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    l_w = XmCreateLabel (eyep_w, "EL", args, n);
	    set_xmstring (l_w, XmNlabelString, "Eyepieces Definitions");
	    XtManageChild (l_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    pb_w = XmCreatePushButton (eyep_w, "Add current", args, n);
	    wtip (pb_w, "Add current settings to list of favorites");
	    XtAddCallback (pb_w, XmNactivateCallback, se_addfav_cb, NULL);
	    XtManageChild (pb_w);

	    /* all favorites in a RC, empty until favorites get defined */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, pb_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    favrc_w = XmCreateRowColumn (eyep_w, "EPRC", args, n);
	    XtManageChild (favrc_w);

	    /* load/save controls */

	    s[0] = eyep_suffix;
	    sfm_w = createFSM (eyep_w, s, 1, "auxil", se_loadd_cb);
	    wtip (sfm_w, "Select file of favorite eyepieces to load"); 
	    set_xmstring (sfm_w, XmNlabelString, "Load file: ");

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, favrc_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 15); n++;
	    XtSetValues (sfm_w, args, n);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sfm_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 15); n++;
	    pb_w = XmCreatePushButton (eyep_w, "ESPB", args, n);
	    set_xmstring (pb_w, XmNlabelString, "Save to:");
	    wtip (pb_w, "Save these Eyepieces definitions in the file named at right");
	    XtAddCallback (pb_w, XmNactivateCallback, se_saved_cb, NULL);
	    XtManageChild (pb_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sfm_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, pb_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 85); n++;
	    saved_w = XmCreateTextField (eyep_w, "EyePDefFile", args, n);
	    XtAddCallback (saved_w, XmNactivateCallback, se_saved_cb, NULL);
	    sr_reg (saved_w, NULL, skyepcategory, 0);
	    defaultTextFN (saved_w, 0, defeyepfn, NULL);
	    wtip (saved_w, "File in which to save these Eyepiece definitions");
	    XtManageChild (saved_w);

	/* eyepice locations */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, saved_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNseparatorType, XmDOUBLE_LINE); n++;
	sep_w = XmCreateSeparator (eyep_w, "Sep", args, n);
	XtManageChild (sep_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    l_w = XmCreateLabel (eyep_w, "Pos", args, n);
	    set_xmstring (l_w, XmNlabelString, "Current Sky View Eyepieces");
	    XtManageChild (l_w);

	    s[0] = eyel_suffix;
	    sfm_w = createFSM (eyep_w, s, 1, "auxil", se_loadp_cb);
	    wtip (sfm_w, "Select existing eyepiece positions file to load");
	    set_xmstring (sfm_w, XmNlabelString, "Load file: ");

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, l_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 15); n++;
	    XtSetValues (sfm_w, args, n);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sfm_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 15); n++;
	    pb_w = XmCreatePushButton (eyep_w, "ELS", args, n);
	    set_xmstring (pb_w, XmNlabelString, "Save to:");
	    wtip (pb_w, "Save current eyepiece positions in file named at right");
	    XtAddCallback (pb_w, XmNactivateCallback, se_savep_cb, NULL);
	    XtManageChild (pb_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sfm_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, pb_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 85); n++;
	    savep_w = XmCreateTextField (eyep_w, "EyePPosFile", args, n);
	    XtAddCallback (savep_w, XmNactivateCallback, se_savep_cb, NULL);
	    sr_reg (savep_w, NULL, skyepcategory, 0);
	    defaultTextFN (savep_w, 0, defeyelfn, NULL);
	    wtip (savep_w, "File in which to save current eyepiece positions");
	    XtManageChild (savep_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, savep_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 10); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 20); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 80); n++;
	    delep_w = XmCreatePushButton (eyep_w, "DelE", args, n);
	    XtAddCallback (delep_w, XmNactivateCallback, se_delall_cb, NULL);
	    wtip (delep_w, "Delete all eyepieces now on Sky View");
	    set_xmstring (delep_w,XmNlabelString,"Delete all placed eyepieces");
	    XtSetSensitive (delep_w, False);	/* works when there are some */
	    XtManageChild (delep_w);

	/* separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, delep_w); n++;
	XtSetArg (args[n], XmNtopOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (eyep_w, "Sep", args, n);
	XtManageChild (sep_w);

	/* a close button */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 20); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 40); n++;
	w = XmCreatePushButton (eyep_w, "Close", args, n);
	XtAddCallback (w, XmNactivateCallback, se_close_cb, NULL);
	wtip (w, "Close this dialog");
	XtManageChild (w);

	/* a help button */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 60); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 80); n++;
	w = XmCreatePushButton (eyep_w, "Help", args, n);
	XtAddCallback (w, XmNactivateCallback, se_help_cb, NULL);
	wtip (w, "More info about this dialog");
	XtManageChild (w);

	/* engage side effects if telrad on initially */
	if (XmToggleButtonGetState(telrad_w)) {
	    XmToggleButtonSetState(telrad_w, False, True);
	    XmToggleButtonSetState(telrad_w, True, True);
	}

	/* calculate sky angles from default values */
	se_calc1();
	se_calc2();

	/* load default favorites */
	se_loadEyep(NULL);
	se_loadPos(NULL);
}

/* read the given w or h scale and write it's value in the given label */
static void
se_scale_fmt (s_w, l_w)
Widget s_w, l_w;
{
	char buf[64];

	buf[0] = l_w == eyephl_w ? 'H' : 'W';
	fs_sexa (buf+1, raddeg(se_getScale(s_w)), 3, 60);
	set_xmstring (l_w, XmNlabelString, buf);
}

/* read the given pa scale and write it's value in the given label */
static void
se_pascale_fmt (s_w, l_w)
Widget s_w, l_w;
{
	char buf[64];

	sprintf (buf, "A  %4.0f", raddeg(se_getPScale(s_w)));
	set_xmstring (l_w, XmNlabelString, buf);
}

/* called when the telrad TB is activated */
static void
se_telrad_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int set = XmToggleButtonGetState (w);

	if (set) {
	    XtSetSensitive (eyef_w, False);
	    XtSetSensitive (eyeb_w, False);
	    XtSetSensitive (eyer_w, False);
	    XtSetSensitive (eyes_w, False);
	    XtSetSensitive (lock_w, False);
	    XtSetSensitive (eyephs_w, False);
	    XtSetSensitive (eyepws_w, False);
	    XtSetSensitive (eyepa_w, False);
	} else {
	    XtSetSensitive (eyef_w, True);
	    XtSetSensitive (eyeb_w, True);
	    XtSetSensitive (eyer_w, True);
	    XtSetSensitive (eyes_w, True);
	    XtSetSensitive (lock_w, True);
	    XtSetSensitive (eyephs_w, True);
	    XtSetSensitive (eyepws_w, True);
	    XtSetSensitive (eyepa_w, True);
	}
}

/* drag callback from the height scale */
static void
se_hscale_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	se_scale_fmt (eyephs_w, eyephl_w);

	/* slave the w scale to the h scale */
	if (XmToggleButtonGetState(lock_w)) {
	    se_setScale (eyepws_w, se_getScale (eyephs_w));
	    se_scale_fmt (eyepws_w, eyepwl_w);
	    XmToggleButtonSetState (telrad_w, False, True);
	}
}

/* drag callback from the width scale */
static void
se_wscale_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	se_scale_fmt (eyepws_w, eyepwl_w);

	/* slave the w scale to the h scale */
	if (XmToggleButtonGetState(lock_w)) {
	    se_setScale (eyephs_w, se_getScale (eyepws_w));
	    se_scale_fmt (eyephs_w, eyephl_w);
	    XmToggleButtonSetState (telrad_w, False, True);
	}
}

/* drag callback from the pa scale */
static void
se_pscale_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	se_pascale_fmt (eyepa_w, eyepal_w);
}

/* callback from the delete-all eyepieces control.
 */
/* ARGSUSED */
static void
se_delall_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (eyep) {
	    if (confirm()) {
		char msg[128];
		switch (neyep) {
		case 0:
		    printf ("Bug! tried to delete %d eyepieces\n", neyep);
		    abort();
		case 1:
		    sprintf (msg, "Delete the placed eyepiece?");
		    break;
		case 2:
		    sprintf (msg, "Delete both placed eyepieces?");
		    break;
		default:
		    sprintf (msg, "Delete all %d placed eyepieces?", neyep);
		    break;
		}
		query (eyep_w, msg, "Yes -- delete", "No -- cancel", NULL,
						    se_delall, NULL, NULL);
	    } else
		se_delall();
	}
}

/* delete memory for and display of all currently placed eyepieces.
 */
static void
se_delall()
{
	XtFree ((void *)eyep);
	eyep = NULL;
	neyep = 0;
	sv_all(mm_get_now());
	XtSetSensitive (delep_w, False);
}

/* callback from the close PB.
 */
/* ARGSUSED */
static void
se_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (eyep_w);
}

/* called when the help button is hit in the eyepiece dialog */
/* ARGSUSED */
static void
se_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
	    "Define eyepiece shapes and sizes."
	};

	hlp_dialog ("SkyView_eyepieces", msg, sizeof(msg)/sizeof(msg[0]));

}

/* called to take the sky angle from the calculator and set width scale.
 * client points to a label widget from which the angle is extracted.
 */
/* ARGSUSED */
static void
se_skyW_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Widget angle_w = (Widget)client;
	char *str;
	double a;

	get_xmstring (angle_w, XmNlabelString, &str);
	f_scansexa (str, &a);
	XtFree (str);
	se_setScale (eyepws_w, degrad(a));
	se_scale_fmt (eyepws_w, eyepwl_w);
	if (XmToggleButtonGetState (lock_w)) {
	    se_setScale (eyephs_w, degrad(a));
	    se_scale_fmt (eyephs_w, eyephl_w);
	}
}

/* called to take the sky angle from the calculator and set height scale.
 * client points to a label widget from which the angle is extracted.
 */
/* ARGSUSED */
static void
se_skyH_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Widget angle_w = (Widget)client;
	char *str;
	double a;

	get_xmstring (angle_w, XmNlabelString, &str);
	f_scansexa (str, &a);
	XtFree (str);
	se_setScale (eyephs_w, degrad(a));
	se_scale_fmt (eyephs_w, eyephl_w);
	if (XmToggleButtonGetState (lock_w)) {
	    se_setScale (eyepws_w, degrad(a));
	    se_scale_fmt (eyepws_w, eyepwl_w);
	}
}

/* called when any of the factors in formula 1 change */
/* ARGSUSED */
static void
se_calc1_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	se_calc1();
}

static void
se_calc1()
{
	char *tmp, *flstr, *fpstr, sastr[32];
	double fl, fp, sa;

	/* get focal length, convert to mm */
	flstr = XmTextFieldGetString (fl_w);
	fl = atof (flstr);
	if (fl == 0) {
	    XtFree (flstr);
	    return;
	}
	if ((tmp = strchr (flstr, 'm')) && strrchr (flstr, 'm') == tmp)
	    fl *= 1e3;		/* m to mm */
	else if (strchr (flstr, '"') || strchr (flstr, 'i'))
	    fl *= 25.4;		/* inches to mm */
	/* else assume mm */
	XtFree (flstr);

	/* get focal plane length, convert to um */
	fpstr = XmTextFieldGetString (fp_w);
	fp = atof (fpstr);
	if ((tmp = strchr (fpstr, 'm')) && strrchr(fpstr, 'm') != tmp)
	    fp *= 1e3;		/* mm to um */
	else if (strchr (fpstr, '"') || strchr (fpstr, 'i'))
	    fp *= 25.4e6;	/* inches to um */
	/* else assume um */
	XtFree (fpstr);

	/* compute and show sky angle */
	sa = 206*fp/fl;		/* arc seconds */
	fs_sexa (sastr, sa/3600., 3, 36000);
	set_xmstring (sa1_w, XmNlabelString, sastr);
}

/* called when any of the factors in formula 2 change */
/* ARGSUSED */
static void
se_calc2_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	se_calc2();
}

/* gather values and compute sky angle using formula 2 */
static void
se_calc2()
{
	char *tmp, *mflstr, sastr[32];
	double afov, efl, mfl, sa;

	/* get apparent field of view, assume degrees */
	tmp = XmTextFieldGetString (afov_w);
	afov = atof (tmp);
	XtFree (tmp);

	/* get eyepiece focal length, assume mm */
	tmp = XmTextFieldGetString (efl_w);
	efl = atof (tmp);
	XtFree (tmp);
	if (efl <= 0)
	    return;

	/* get focal length of primary, convert to mm */
	mflstr = XmTextFieldGetString (mfl_w);
	mfl = atof (mflstr);
	if ((tmp = strchr (mflstr, 'm')) && strrchr(mflstr, 'm') == tmp)
	    mfl *= 1e3;		/* m to mm */
	else if (strchr (mflstr, '"') || strchr (mflstr, 'i'))
	    mfl *= 25.4;	/* inches to mm */
	/* else assume mm */
	XtFree (mflstr);

	/* compute and show sky angle */
	sa = afov*efl/mfl;
	fs_sexa (sastr, sa, 3, 36000);
	set_xmstring (sa2_w, XmNlabelString, sastr);
}

/* read the given Scale widget and return it's current setting, in rads */
static double
se_getScale (w)
Widget w;
{
	int v;
	double a;

	XmScaleGetValue (w, &v);

	a = v*LOSCALE;
	if (a > CHSCALE)
	    a = CHSCALE + (a - CHSCALE)/LOSCALE*HISCALE;
	return (a);
}

/* set the given Scale widget to the given setting, in rads */
static void
se_setScale (w, a)
Widget w;
double a;
{
	int v;

	if (a > CHSCALE)
	    a = (a - CHSCALE)*LOSCALE/HISCALE + CHSCALE;
	v = (int)(a/LOSCALE+.5);
	if (v > SCALEMAX) {
	    xe_msg (1, "Sorry, scale only goes to %g", raddeg(MXSCALE));
	} else {
	    if (v < 1)
		v = 1;
	    XmScaleSetValue (w, v);
	}
}

/* read the given pos angle widget and return it's current setting, in rads */
static double
se_getPScale (w)
Widget w;
{
	int v;

	XmScaleGetValue (w, &v);
	return (degrad(v));
}

/* called to install "use" a FavEyeP.
 * client is index into favs[]
 */
/* ARGSUSED */
static void
se_usefav_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	FavEyeP *fp = &favs[(long int)client];

	se_setScale (eyepws_w, fp->w);
	se_scale_fmt (eyepws_w, eyepwl_w);
	se_setScale (eyephs_w, fp->h);
	se_scale_fmt (eyephs_w, eyephl_w);
	XmToggleButtonSetState (fp->isE ? eyer_w : eyes_w, True, True);
	XmToggleButtonSetState (fp->isS ? eyef_w : eyeb_w, True, True);
	XmToggleButtonSetState (telrad_w, False, True);
	XmToggleButtonSetState (lock_w, False, True);
}

/* called to delete a FavEyeP.
 * client is index into favs[]
 */
/* ARGSUSED */
static void
se_delfav_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	FavEyeP *fp = &favs[(long int)client];
	XtDestroyWidget (fp->row_w);
	fp->inuse = 0;
}

/* called to add a new FavEyeP */
/* ARGSUSED */
static void
se_addfav_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	FavEyeP newfav;
	double aunused;

	memset (&newfav, 0, sizeof(newfav));
	se_eyepsz (&newfav.w, &newfav.h, &aunused, &newfav.isE, &newfav.isS);
	sprintf (newfav.name, "My eyepiece #%d", nfavs+1);
	se_addFavEyeP (&newfav);
}

/* callback to load eyepiece defintions from file.
 * file name is label of this widget
 */
/* ARGSUSED */
static void
se_loadd_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *fn;
	get_xmstring (w, XmNlabelString, &fn);
	se_loadEyep(fn);
	XtFree (fn);
}

/* callback to Save the current eyepiece definitions to file named in saved_w
 * N.B. don't use call, this is used by TF and PB
 */
/* ARGSUSED */
static void
se_saved_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024], *fn;
	char *txt;

	/* get file name */
	fn = txt = XmTextFieldGetString (saved_w);
	if (!strstr (txt, eyep_suffix)) {
	    sprintf (fn = buf, "%s%s", txt, eyep_suffix);
	    XmTextFieldSetString (saved_w, fn);
	}

	/* save */
	se_saveFav(fn);

	/* clean up */
	XtFree (txt);
}

/* callback to load eyepiece positions list from file.
 * file name is label of this widget
 */
/* ARGSUSED */
static void
se_loadp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *fn;
	get_xmstring (w, XmNlabelString, &fn);
	se_loadPos(fn);
	XtFree (fn);
}

/* callback to Save the current eyepiece positions to file named in savep_w
 * N.B. don't use call, this is used by TF and PB
 */
/* ARGSUSED */
static void
se_savep_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024], *fn;
	char *txt;

	/* get file name */
	fn = txt = XmTextFieldGetString (savep_w);
	if (!strstr (txt, eyel_suffix)) {
	    sprintf (fn = buf, "%s%s", txt, eyel_suffix);
	    XmTextFieldSetString (savep_w, fn);
	}

	/* save */
	se_savePos(fn);

	/* clean up */
	XtFree (txt);
}

/* add (or reuse) a new (or unused) entry to favs[], create and show in favrc_w.
 */
static void
se_addFavEyeP (newfp)
FavEyeP *newfp;
{
	char buf[32], wstr[32], hstr[32];
	Widget w;
	Arg args[20];
	FavEyeP *fp;
	int n, fn;

	/* find/create a new entry */
	for (fn = 0; fn < nfavs; fn++)
	    if (!favs[fn].inuse)
		break;
	if (fn == nfavs)
	    favs = (FavEyeP *) XtRealloc ((char *)favs,
						    (++nfavs)*sizeof(FavEyeP));
	fp = &favs[fn];

	/* start filling with new */
	*fp = *newfp;
	fp->inuse = 1;

	/* add the widgets */
	n = 0;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNspacing, 3); n++;
	fp->row_w = XmCreateRowColumn (favrc_w, "EPF", args, n);
	XtManageChild (fp->row_w);

	    n = 0;
	    w = XmCreatePushButton (fp->row_w, "Del", args, n);
	    XtAddCallback (w, XmNactivateCallback, se_delfav_cb, (XtPointer)(long int)fn);
	    wtip (w, "Delete this eyepiece from the list of favorites");
	    XtManageChild (w);

	    n = 0;
	    w = XmCreatePushButton (fp->row_w, "Use", args, n);
	    XtAddCallback (w, XmNactivateCallback, se_usefav_cb, (XtPointer)(long int)fn);
	    wtip (w, "Install this eyepiece in the `next' settings above.");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNcolumns, MAXEPNMD); n++;
	    XtSetArg (args[n], XmNmaxLength, MAXEPNM-1); n++;
	    XtSetArg (args[n], XmNvalue, fp->name); n++;
	    w = XmCreateTextField (fp->row_w, "FTF", args, n);
	    wtip (w, "Type in a name for this eyepiece configuration");
	    XtManageChild (w);
	    fp->name_w = w;

	    n = 0;
	    w = XmCreateLabel (fp->row_w, "FL", args, n);
	    fs_sexa (wstr, raddeg(fp->w), 2, 60);
	    fs_sexa (hstr, raddeg(fp->h), 2, 60);
	    sprintf (buf, "%s %s  %c %c", wstr, hstr, 
				    fp->isE ? 'E' : 'R', fp->isS ? 'S' : 'O');
	    set_xmstring (w, XmNlabelString, buf);
	    wtip (w, "Coded definition for this eyepiece: W H Shape Style");
	    XtManageChild (w);
}

/* save the current (used) entries in favs[] to the given file */
static void
se_saveFav(char *fn)
{
	FILE *fp;
	int i;

	/* create */
	fp = fopend (fn, NULL, "w");
	if (!fp)
	    return;			/* already informed user */

	/* write */
	fprintf (fp, "<EyepieceDefinitions>\n");
	for (i = 0; i < nfavs; i++) {
	    FavEyeP *fvp = &favs[i];
	    char *name;

	    if (!fvp->inuse)
		continue;
	    name = XmTextFieldGetString (fvp->name_w);
	    fprintf (fp, "  <eyepiece width='%g' height='%g' isElliptical='%d' isSolid='%d'>%s</eyepiece>\n",
		    raddeg(fvp->w), raddeg(fvp->h), fvp->isE, fvp->isS, name);
	    XtFree (name);
	}
	fprintf (fp, "</EyepieceDefinitions>\n");

	/* finished */
	fclose (fp);

	if (confirm())
	    xe_msg (1, "Wrote %d definitions to %s", nfavs, fn);
}

/* read and replace the favorites list from the given file, or file named in
 * saved_w if !fn
 */
static void
se_loadEyep(char *fn)
{
	char msg[1024];
	char buf[1024];
	XMLEle *root, *ep;
	FILE *fp;
	LilXML *xp;
	int i;

	/* get file name */
	if (!fn) {
	    char *txt = XmTextFieldGetString (saved_w);
	    if (!strstr (txt, eyep_suffix))
		sprintf (buf, "%s%s", txt, eyep_suffix);
	    else
		strcpy (buf, txt);
	    XtFree (txt);
	    fn = buf;
	}

	/* open */
	fp = fopend (fn, "auxil", "r");
	if (!fp)
	    return;			/* already informed user */

	/* read */
	xp = newLilXML ();
	root = readXMLFile (fp, xp, msg);
	fclose (fp);
	delLilXML (xp);
	if (!root) {
	    xe_msg (1, "%s: %s", fn, msg[0] ? msg : "bad format");
	    return;
	}
	if (strcmp (tagXMLEle(root), "EyepieceDefinitions")) {
	    xe_msg (1, "%s: not an Eyepieces file", fn);
	    delXMLEle (root);
	    return;
	}

	/* clear the favs[] array */
	for (i = 0; i < nfavs; i++)
	    if (favs[i].inuse)
		XtDestroyWidget (favs[i].row_w);
	XtFree ((char *)favs);
	favs = NULL;
	nfavs = 0;

	/* build new */
	for (ep = nextXMLEle(root,1); ep != NULL; ep = nextXMLEle(root,0)) {
	    FavEyeP newf;

	    if (strcmp (tagXMLEle(ep), "eyepiece"))
		continue;

	    memset (&newf, 0, sizeof(newf));
	    strncpy (newf.name, pcdataXMLEle(ep), sizeof(newf.name)-1);
	    newf.isE = atoi(findXMLAttValu(ep,"isElliptical"));
	    newf.isS = atoi(findXMLAttValu(ep,"isSolid"));
	    newf.w = degrad(atof(findXMLAttValu(ep,"width")));
	    newf.h = degrad(atof(findXMLAttValu(ep,"height")));
	    se_addFavEyeP (&newf);
	}

	if (confirm())
	    xe_msg (1, "Read %d definitions from %s", nfavs, fn);

	/* finished */
	delXMLEle (root);
}

/* save eyep[] to the given file */
static void
se_savePos(char *fn)
{
	FILE *fp;
	int i;

	/* create */
	fp = fopend (fn, NULL, "w");
	if (!fp)
	    return;			/* already informed user */

	/* write */
	fprintf (fp, "<EyepiecePlacements>\n");
	for (i = 0; i < neyep; i++) {
	    EyePiece *ep = &eyep[i];

	    fprintf (fp, "  <placement altdec='%g' azra='%g' aamode='%d'",
					    ep->altdec, ep->azra, ep->aamode);
	    fprintf (fp, " width='%g' height='%g' angle='%g'",
					    ep->eyepw, ep->eyeph, ep->eyepa);
	    fprintf (fp, " isRound='%d' isSolid='%d' />\n",ep->round,ep->solid);
	}
	fprintf (fp, "</EyepiecePlacements>\n");

	if (confirm())
	    xe_msg (1, "Wrote %d eyepiece placements to %s", neyep, fn);

	/* finished */
	fclose (fp);
}

/* read and replace the list of eyepiece positions from the given file, or
 * file named in savep_w if !fn
 */
static void
se_loadPos(char *fn)
{
	char msg[1024];
	char buf[1024];
	XMLEle *root, *ep;
	FILE *fp;
	LilXML *xp;

	/* get file name */
	if (!fn) {
	    char *txt = XmTextFieldGetString (savep_w);
	    if (!strstr (txt, eyel_suffix))
		sprintf (buf, "%s%s", txt, eyel_suffix);
	    else
		strcpy (buf, txt);
	    XtFree (txt);
	    fn = buf;
	}

	/* open */
	fp = fopend (fn, "auxil", "r");
	if (!fp)
	    return;			/* already informed user */

	/* read */
	xp = newLilXML ();
	root = readXMLFile (fp, xp, msg);
	fclose (fp);
	delLilXML (xp);
	if (!root) {
	    xe_msg (1, "%s: %s", fn, msg[0] ? msg : "bad format");
	    return;
	}
	if (strcmp (tagXMLEle(root), "EyepiecePlacements")) {
	    xe_msg (1, "%s: not an Eyepiece Placements file", fn);
	    delXMLEle (root);
	    return;
	}

	/* clear the eyep[] array */
	XtFree ((char *)eyep);
	eyep = NULL;
	neyep = 0;

	/* build new list */
	for (ep = nextXMLEle(root,1); ep != NULL; ep = nextXMLEle(root,0)) {
	    EyePiece *nep;

	    if (strcmp (tagXMLEle(ep), "placement"))
		continue;

	    nep = se_addeyep ();
	    nep->altdec = atof(findXMLAttValu(ep,"altdec"));
	    nep->azra   = atof(findXMLAttValu(ep,"azra"));
	    nep->aamode = atof(findXMLAttValu(ep,"aamode"));
	    nep->eyepw  = atof(findXMLAttValu(ep,"width"));
	    nep->eyeph  = atof(findXMLAttValu(ep,"height"));
	    nep->eyepa  = atof(findXMLAttValu(ep,"angle"));
	    nep->round  = atoi(findXMLAttValu(ep,"isRound"));
	    nep->solid  = atoi(findXMLAttValu(ep,"isSolid"));
	}

	if (confirm())
	    xe_msg (1, "Read %d eyepiece placements from %s", neyep, fn);

	/* finished */
	delXMLEle (root);

	/* draw */
	sv_all (mm_get_now());
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: skyeyep.c,v $ $Date: 2011/05/16 02:32:23 $ $Revision: 1.32 $ $Name:  $"};

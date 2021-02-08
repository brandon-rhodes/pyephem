/* implement the manual sky coords conversion tool */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>

#include "xephem.h"

static void cc_create (void);
static void cc_point_cb (Widget w, XtPointer client, XtPointer call);
static void cc_getsky_cb (Widget w, XtPointer client, XtPointer call);
static void cc_eyep_cb (Widget w, XtPointer client, XtPointer call);
static void cc_canonFormat_cb (Widget w, XtPointer client, XtPointer call);
static void cc_vchg_cb (Widget w, XtPointer client, XtPointer call);
static void cc_help_cb (Widget w, XtPointer client, XtPointer call);
static void cc_close_cb (Widget w, XtPointer client, XtPointer call);
static void cc_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void cc_newval (Widget w);
static void canonAll (void);
static void canonFmt (char buf[], double v, int h);
static void reFormat (Widget w, int h);

static Widget ccshell_w;	/* main shell */
static Widget ccalt_w;		/* TF */
static Widget ccaz_w;		/* TF */
static Widget ccra_w;		/* TF */
static Widget ccdec_w;		/* TF */
static Widget ccglat_w;		/* TF */
static Widget ccglng_w;		/* TF */
static Widget cceclat_w;	/* TF */
static Widget cceclng_w;	/* TF */
static Widget ccdt_w;		/* time stamp label */
static Widget ccral_w;		/* RA epoch label */
static Widget cclra_w;		/* TF, epoch local to this window */
static Widget ccldec_w;		/* TF, RA local to this window */
static Widget ccle_w;		/* TF, Dec local to this window */
static Widget lastchg_w;	/* which ever TF was last edited */

static int block_vchg;		/* set to block valueChanged callback */

static char cccategory[] = "Coordinate converter";     /* Save category */

/* bring up the Manual dialog */
void
cc_manage ()
{
	if (!ccshell_w) {
	    cc_create();
	    cc_newval (ccra_w);
	    cc_canonFormat_cb (NULL, NULL, NULL);
	}

	XtPopup (ccshell_w, XtGrabNone);
	set_something (ccshell_w, XmNiconic, (XtArgVal)False);
}

/* new main Now: hold ra/dec and update alt/az */
void
cc_update (np, how_much)
Now *np;
int how_much;
{
	if (!isUp(ccshell_w))
	    return;

	cc_newval (lastchg_w);
}

/* called to put up or remove the watch cursor.  */
void
cc_cursor (c)
Cursor c;
{
	Window win;

	if (ccshell_w && (win = XtWindow(ccshell_w)) != 0) {
	    Display *dsp = XtDisplay(ccshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create the Manual entry dialog */
static void
cc_create()
{
	typedef struct {
	    char *label;	/* label */
	    char *def;		/* default value */
	    Widget *wp;		/* TF widget */
	    char *tip;		/* widget tip */
	} CCMField;
	static CCMField ccfields[] = {
	    {"RA @ ",         "  00:00:00.00", &cclra_w,
					    "RA, hours @ Equinox at left"},
	    {"Declination:",  "  00:00:00.00", &ccldec_w,
					    "Dec, degrees @ Equinox at left"},
	    {"RA @ XXXXXX",   "  00:00:00.00", &ccra_w,
					    "RA, hours @ Equinox in Main"},
	    {"Declination:",  "  00:00:00.00", &ccdec_w,
					    "Dec, degrees @ Equinox in Main"},
	    {"Altitude:",     "  00:00:00.00", &ccalt_w,
					    "Altitude, degrees"},
	    {"Azimuth:",      "  00:00:00.00", &ccaz_w,
					    "Azimuth, degrees E of N"},
	    {"Galactic lat:", "  00:00:00.00", &ccglat_w,
					    "Galactic latitude, degrees"},
	    {"Galactic lng:", "  00:00:00.00", &ccglng_w,
					    "Galactic longitude, degrees"},
	    {"Ecliptic lat:", "  00:00:00.00", &cceclat_w,
					    "Ecliptic latitude, degrees"},
	    {"Ecliptic lng:", "  00:00:00.00", &cceclng_w,
					    "Ecliptic longitude, degrees"},
	};
	Widget w;
	Widget cc_w, rc_w;
	Arg args[20];
	int i;
	int n;

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Coordinate converter"); n++;
	XtSetArg (args[n], XmNiconName, "Coords"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	ccshell_w = XtCreatePopupShell ("CoordsConv", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (ccshell_w);
	set_something (ccshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (ccshell_w, XmNpopdownCallback, cc_popdown_cb, 0);
	sr_reg (ccshell_w, "XEphem*CoordsConv.x", cccategory, 0);
	sr_reg (ccshell_w, "XEphem*CoordsConv.y", cccategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 20); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNfractionBase, 67); n++;
	cc_w = XmCreateForm (ccshell_w, "CCForm", args, n);
	XtAddCallback (cc_w, XmNhelpCallback, cc_help_cb, 0);
	XtManageChild (cc_w);

	/* main table in a RC */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNspacing, 8); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNnumColumns, (XtNumber(ccfields)+1)/2); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	rc_w = XmCreateRowColumn (cc_w, "CCMRC", args, n);
	XtManageChild (rc_w);

	    /* add the field entries */
	    for (i = 0; i < XtNumber(ccfields); i++) {
		CCMField *ccp = &ccfields[i];

		if (ccp->wp == &cclra_w) {
		    /* special case for local epoch */
		    Widget hrc_w;

		    n = 0;
		    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
		    XtSetArg (args[n], XmNmarginHeight, 0); n++;
		    hrc_w = XmCreateRowColumn (rc_w, "CHRC", args, n);
		    XtManageChild (hrc_w);

		    n = 0;
		    w = XmCreateLabel (hrc_w, "EL", args, n);
		    set_xmstring (w, XmNlabelString, ccp->label);
		    XtManageChild (w);

		    n = 0;
		    XtSetArg (args[n], XmNcolumns, 6); n++;
		    ccle_w = XmCreateTextField (hrc_w, "Equinox", args, n);
		    wtip (ccle_w,"Precession year or EOD for apparent");
		    XtAddCallback (ccle_w, XmNactivateCallback, cc_vchg_cb, 0);
		    sr_reg (ccle_w, NULL, cccategory, 1);
		    XtManageChild (ccle_w);

		    n = 0;
		    w = XmCreateLabel (hrc_w, "CCC", args, n);
		    set_xmstring (w, XmNlabelString, ":");
		    XtManageChild (w);

		} else {
		    n = 0;
		    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		    w = XmCreateLabel (rc_w, "CCM", args, n);
		    set_xmstring (w, XmNlabelString, ccp->label);
		    XtManageChild (w);
		}

		/* grab RA label so we can show epoch */
		if (ccp->wp == &ccra_w)
		    ccral_w = w;

		n = 0;
		XtSetArg (args[n], XmNcolumns, 13); n++;
		XtSetArg (args[n], XmNvalue, ccp->def); n++;
		w = XmCreateTextField (rc_w, "CCTF", args, n);
		XtAddCallback (w, XmNvalueChangedCallback, cc_vchg_cb, 0);
		XtAddCallback (w, XmNfocusCallback, cc_vchg_cb, 0);
		XtAddCallback (w, XmNactivateCallback, cc_canonFormat_cb, 0);
		wtip (w, ccp->tip);
		XtManageChild (w);
		*(ccp->wp) = w;
	    }

	/* time stamp */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	ccdt_w = XmCreateLabel (cc_w, "DT", args, n);
	wtip (ccdt_w, "Date and Time for which Alt/Az are computed");
	XtManageChild (ccdt_w);

	/* controls across bottom */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ccdt_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 1); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 11); n++;
	w = XmCreatePushButton (cc_w, "Close", args, n);
	wtip (w, "Close this window");
	XtAddCallback (w, XmNactivateCallback, cc_close_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ccdt_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 12); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 22); n++;
	w = XmCreatePushButton (cc_w, "Sky Point", args, n);
	wtip (w, "Center the Sky View at these coordinates"); 
	XtManageChild (w);
	XtAddCallback (w, XmNactivateCallback, cc_point_cb, 0);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ccdt_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 23); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 33); n++;
	w = XmCreatePushButton (cc_w, "Eyepiece", args, n);
	wtip (w, "Place an eyepiece at these coordinates on the Sky View"); 
	XtManageChild (w);
	XtAddCallback (w, XmNactivateCallback, cc_eyep_cb, 0);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ccdt_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 34); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 44); n++;
	w = XmCreatePushButton (cc_w, "Get Sky", args, n);
	wtip (w, "Load coordinates of Sky View center"); 
	XtAddCallback (w, XmNactivateCallback, cc_getsky_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ccdt_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 45); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 55); n++;
	w = XmCreatePushButton (cc_w, "Canonize", args, n);
	wtip (w, "Reformat all fields in a consistent manner");
	XtAddCallback (w, XmNactivateCallback, cc_canonFormat_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ccdt_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 56); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 66); n++;
	w = XmCreatePushButton (cc_w, "Help", args, n);
	wtip (w, "More information about this window");
	XtAddCallback (w, XmNactivateCallback, cc_help_cb, 0);
	XtManageChild (w);
}

/* callback to Point sky at current coords */
static void
cc_point_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Now *np = mm_get_now();
	double e = epoch == EOD ? mjd : epoch;
	double ra, dec;
	char *p;
	Obj o;

	p = XmTextFieldGetString(ccra_w);
	f_scansexa (p, &ra);
	XtFree(p);
	ra = hrrad(ra);

	p = XmTextFieldGetString(ccdec_w);
	f_scansexa (p, &dec);
	XtFree(p);
	dec = degrad(dec);

	if (epoch == EOD)
	    ap_as (np, mjd, &ra, &dec);

	memset (&o, 0, sizeof(o));
	strcpy (o.o_name, "Anonymous");
	o.o_type = FIXED;
	o.f_RA = (float)ra;
	o.f_dec = (float)dec;
	o.f_epoch = (float)e;

	obj_cir (np, &o);
	sv_point (&o);
}

/* how each field is formatted, depends on Hours or Degrees */
static void
canonFmt (buf, v, h)
char buf[];
double v;
int h;
{
	if (h)
	    fs_sexa (buf, v, 4, 360000);
	else {
	    fs_sexa (buf, v, 4, 36000);
	    strcat (buf, " ");
	}
}

/* callback to load from current Sky View center */
static void
cc_getsky_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	double fov, alt, az, ra, dec;
	char buf[32];
	int aa;

	sv_getcenter (&aa, &fov, &alt, &az, &ra, &dec);

	if (aa) {
	    /* user is driving via alt/az */
	    canonFmt (buf, raddeg(alt), 0);
	    XmTextFieldSetString (ccalt_w, buf);
	    canonFmt (buf, raddeg(az), 0);
	    XmTextFieldSetString (ccaz_w, buf);
	    cc_newval (ccaz_w);
	} else {
	    /* user is driving via RA/Dec */
	    canonFmt (buf, radhr(ra), 1);
	    XmTextFieldSetString (ccra_w, buf);
	    canonFmt (buf, raddeg(dec), 0);
	    XmTextFieldSetString (ccdec_w, buf);
	    cc_newval (ccdec_w);
	}
}

/* redisplay TF's sexagesimal value prettied up */
static void
reFormat (w, h)
Widget w;
int h;
{
	char buf[32], *p;
	double x;

	p = XmTextFieldGetString (w);
	f_scansexa (p, &x);
	XtFree(p);
	canonFmt (buf, x, h);
	XmTextFieldSetString (w, buf);
}

/* tidy up each coordinate display */
static void
canonAll()
{
	block_vchg++;
	reFormat (ccalt_w, 0);
	reFormat (ccaz_w, 0);
	reFormat (cclra_w, 1);
	reFormat (ccldec_w, 0);
	reFormat (ccra_w, 1);
	reFormat (ccdec_w, 0);
	reFormat (ccglat_w, 0);
	reFormat (ccglng_w, 0);
	reFormat (cceclat_w, 0);
	reFormat (cceclng_w, 0);
	block_vchg--;
}

/* called to place an eyepiece at these coords on the sky view */
static void
cc_eyep_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Now *np = mm_get_now();
	double fov, ra, dec, alt, az;
	int aamode;
	char *p;

	/* only if skyview up */
	if (!svshell_w) {
	    xe_msg (1, "Can not place eyepiece until Sky View window is up");
	    return;
	}

	/* get sky mode */
	sv_getcenter (&aamode, &fov, &alt, &az, &ra, &dec);

	/* define new eyepiece */
	if (aamode) {

	    p = XmTextFieldGetString(ccalt_w);
	    f_scansexa (p, &alt);
	    XtFree(p);
	    alt = degrad(alt);

	    p = XmTextFieldGetString(ccaz_w);
	    f_scansexa (p, &az);
	    XtFree(p);
	    az = degrad(az);

	    se_add (aamode, az, alt);

	} else {

	    p = XmTextFieldGetString(ccra_w);
	    f_scansexa (p, &ra);
	    XtFree(p);
	    ra = hrrad(ra);

	    p = XmTextFieldGetString(ccdec_w);
	    f_scansexa (p, &dec);
	    XtFree(p);
	    dec = degrad(dec);

	    if (epoch == EOD)
		ap_as (np, mjd, &ra, &dec);

	    se_add (aamode, ra, dec);

	}

	/* display on skyview */
	sv_all (np);
}

/* called from either the Reformat PB or Enter from any TF */
static void
cc_canonFormat_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	canonAll();
}

static void
cc_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* let popdown do all the work */
	XtPopdown (ccshell_w);
}

static void
cc_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
}

/* called when any character changes or focus is received
 * N.B. do not use call, used by different callbacks.
 */
static void
cc_vchg_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	cc_newval (w);
}

/* callback from the overall Help button.
 */
/* ARGSUSED */
static void
cc_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = { "Type in any field and the others are updated",};

	hlp_dialog ("Coord_conv", msg, sizeof(msg)/sizeof(msg[0]));
}

/* called to compute new values when the given TF widget changes.
 * remember in lastchg_w which widget was used to freeze it for next update.
 */
static void
cc_newval (w)
Widget w;
{
	Now *np = mm_get_now();
	double e = (epoch == EOD) ? mjd : epoch;
	char buf[32], *p;
	double ra, dec, lst;

	/* avoid recursion */
	if (block_vchg)
	    return;
	block_vchg++;

	/* save */
	lastchg_w = w;

	/* current time and eq epoch */
	timestamp (np, ccdt_w);
	if (epoch == EOD)
	    (void) strcpy (buf, "RA @ EOD:");
	else {
	    double tmp;
	    mjd_year (epoch, &tmp);
	    sprintf (buf, "RA @ %.1f:", tmp);
	}
	set_xmstring (ccral_w, XmNlabelString, buf);

	/* handy stuff */
	now_lst (np, &lst);
	lst = hrrad(lst);

	/* find astrometric ra/dec @ e from whatever changed */
	if (w == ccalt_w || w == ccaz_w) {
	    double alt, az, ha;

	    p = XmTextFieldGetString(ccalt_w);
	    f_scansexa (p, &alt);
	    XtFree(p);
	    alt = degrad(alt);
	    p = XmTextFieldGetString(ccaz_w);
	    f_scansexa (p, &az);
	    XtFree(p);
	    az = degrad(az);

	    unrefract (pressure, temp, alt, &alt);
	    aa_hadec (lat, alt, az, &ha, &dec);
	    ra = lst - ha;
	    range (&ra, 2*PI);
	    ap_as (np, e, &ra, &dec);

	} else if (w == ccglat_w || w == ccglng_w) {
	    double glat, glng;

	    p = XmTextFieldGetString(ccglat_w);
	    f_scansexa (p, &glat);
	    XtFree(p);
	    glat = degrad(glat);
	    p = XmTextFieldGetString(ccglng_w);
	    f_scansexa (p, &glng);
	    XtFree(p);
	    glng = degrad(glng);

	    gal_eq (e, glat, glng, &ra, &dec);

	} else if (w == cceclat_w || w == cceclng_w) {
	    double eclat, eclng;

	    p = XmTextFieldGetString(cceclat_w);
	    f_scansexa (p, &eclat);
	    XtFree(p);
	    eclat = degrad(eclat);
	    p = XmTextFieldGetString(cceclng_w);
	    f_scansexa (p, &eclng);
	    XtFree(p);
	    eclng = degrad(eclng);

	    ecl_eq (e, eclat, eclng, &ra, &dec);
	    if (epoch == EOD)
		ap_as (np, e, &ra, &dec);

	} else if (w == ccra_w || w == ccdec_w) {
	    p = XmTextFieldGetString(ccra_w);
	    f_scansexa (p, &ra);
	    XtFree(p);
	    ra = hrrad(ra);
	    p = XmTextFieldGetString(ccdec_w);
	    f_scansexa (p, &dec);
	    XtFree(p);
	    dec = degrad(dec);
	    if (epoch == EOD)
		ap_as (np, e, &ra, &dec);

	} else if (w == cclra_w || w == ccldec_w || w == ccle_w) {
	    p = XmTextFieldGetString(cclra_w);
	    f_scansexa (p, &ra);
	    XtFree(p);
	    ra = hrrad(ra);
	    p = XmTextFieldGetString(ccldec_w);
	    f_scansexa (p, &dec);
	    XtFree(p);
	    dec = degrad(dec);
	    p = XmTextFieldGetString(ccle_w);
	    if (strchr(p,'o') || strchr(p,'O')) {
		ap_as (np, e, &ra, &dec);
	    } else {
		double le;
		year_mjd (atof(p), &le);
		precess (le, e, &ra, &dec);
	    }
	    XtFree(p);

	} else {
	    printf ("Bogus coords conv widget\n");
	    abort();
	}

	/* find all others from ra/dec that were not set */
	if (w != ccalt_w && w != ccaz_w) {
	    double apra = ra, apdec = dec;	/* require apparent */
	    double alt, az, ha;

	    as_ap (np, e, &apra, &apdec);
	    ha = lst - apra;
	    hadec_aa (lat, ha, apdec, &alt, &az);
	    refract (pressure, temp, alt, &alt);
	    canonFmt (buf, raddeg(alt), 0);
	    XmTextFieldSetString (ccalt_w, buf);
	    canonFmt (buf, raddeg(az), 0);
	    XmTextFieldSetString (ccaz_w, buf);
	}
	if (w != ccglat_w && w != ccglng_w) {
	    double glat, glng;

	    eq_gal (e, ra, dec, &glat, &glng);
	    canonFmt (buf, raddeg(glat), 0);
	    XmTextFieldSetString (ccglat_w, buf);
	    canonFmt (buf, raddeg(glng), 0);
	    XmTextFieldSetString (ccglng_w, buf);
	}
	if (w != cceclat_w && w != cceclng_w) {
	    double eclat, eclng;
	    double era = ra, edec = dec;	/* want current epoch setting */

	    if (epoch == EOD)
		as_ap (np, e, &era, &edec);
	    eq_ecl (e, era, edec, &eclat, &eclng);
	    canonFmt (buf, raddeg(eclat), 0);
	    XmTextFieldSetString (cceclat_w, buf);
	    canonFmt (buf, raddeg(eclng), 0);
	    XmTextFieldSetString (cceclng_w, buf);
	}
	if (w != ccra_w && w != ccdec_w) {
	    double era = ra, edec = dec;	/* want current epoch setting */

	    if (epoch == EOD)
		as_ap (np, e, &era, &edec);
	    canonFmt (buf, radhr(era), 1);
	    XmTextFieldSetString (ccra_w, buf);
	    canonFmt (buf, raddeg(edec), 0);
	    XmTextFieldSetString (ccdec_w, buf);
	}
	if (w != cclra_w && w != ccldec_w && w != ccle_w) {
	    double lra = ra, ldec = dec;

	    p = XmTextFieldGetString(ccle_w);
	    if (strchr(p,'o') || strchr(p,'O')) {
		as_ap (np, e, &lra, &ldec);
	    } else {
		double le;
		year_mjd (atof(p), &le);
		precess (e, le, &lra, &ldec);
	    }
	    XtFree(p);
	    canonFmt (buf, radhr(lra), 1);
	    XmTextFieldSetString (cclra_w, buf);
	    canonFmt (buf, raddeg(ldec), 0);
	    XmTextFieldSetString (ccldec_w, buf);
	}

	block_vchg--;
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: coordsmenu.c,v $ $Date: 2011/05/16 02:32:23 $ $Revision: 1.20 $ $Name:  $"};

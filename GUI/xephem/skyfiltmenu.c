/* code to manage the stuff on the sky view filter display.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/Scale.h>

#include "xephem.h"


/* info for each filter option toggle */
typedef struct {
    char *label;	/* label on the toggle button */
    char *name;		/* name of toggle button, for resource setting */
    int type;		/* as in Obj.o_type */
    char class;		/* if type==FIXED then as in Obj.f.f_class */
    char state;		/* 0 or 1 depending on whether this class is on */
    Widget tbw;		/* the toggle button widget */
} FilterTB;

/* info for a set of filters forming a category */
typedef struct {
    char *label;	/* label for this category */
    char *ltip;		/* tip for this category */
    FilterTB *ftb;	/* array of toggle button filters in this class */
    int nftb;		/* entries in ftb[] */
    Widget pbw;		/* the category "toggle" pushbutton */
} FilterCat;

/* struct to hold automag settings */
typedef struct {
    double r;		/* max FOV radius for this mag, rads */
    int st;		/* star faintest mag */
    int ss;		/* sol sys faintest mag */
    int ds;		/* deep-sky faintest mag */
    int s;		/* mag step per dot size */
} AutoMag;

static void svf_create_filter (Widget mainrc, FilterCat *fcp);
static void svf_reset (void);
static void svf_apply (void);
static int svf_bright_ok (Obj *op);
static void svf_da_cb (Widget w, XtPointer client, XtPointer call);
static void svf_draw_symbol (FilterTB *ftbp, Widget daw);
static void svf_magdrag_cb (Widget w, XtPointer client, XtPointer call);
static void svf_apply_cb (Widget w, XtPointer client, XtPointer call);
static void svf_ok_cb (Widget w, XtPointer client, XtPointer call);
static void svf_close_cb (Widget w, XtPointer client, XtPointer call);
static void svf_help_cb (Widget w, XtPointer client, XtPointer call);
static void svf_toggle_cb (Widget w, XtPointer client, XtPointer call);
static void svf_all_cb (Widget w, XtPointer client, XtPointer call);
static void svf_reset_cb (Widget w, XtPointer client, XtPointer call);
static void svf_cat_toggle_cb (Widget w, XtPointer client, XtPointer call);
static int svf_initautomag (AutoMag **ampp);


static FilterTB solsys_f[] = {
    {"Planets",		"Planets",	PLANET,		0},
    {"Elliptical",	"Elliptical",	ELLIPTICAL,	0},
    {"Hyperbolic",	"Hyperbolic",	HYPERBOLIC,	0},
    {"Parabolic",	"Parabolic",	PARABOLIC,	0},
    {"Earth Sat",	"EarthSat",	EARTHSAT,	0}
};

static FilterTB other_f[] = {
    {"Q Quasars", 	"Quasars",	FIXED,		'Q'},
    {"L Pulsars", 	"Pulsars",	FIXED,		'L'},
    {"J Radio", 	"Radio",	FIXED,		'J'},
    {"Y Supernova", 	"Supernova",	FIXED,		'Y'},
    {"R S/N Rem", 	"SNRemnants",	FIXED,		'R'},
};

static FilterTB stars_f[] = {
    {"S Single",	"Stars",	FIXED,		'S'},
    {"B Binary",	"Binary",	BINARYSTAR,	 0},
       /* B as a subfield of FIXED merged into D as of BINARYSTAR */
    {"D Double",	"Double",	FIXED,		'D'},
    {"M Multiple",	"Multiple",	FIXED,		'M'},
    {"V Variable",	"Variable",	FIXED,		'V'},
    {"T Star-like",	"Stellar",	FIXED,		'T'}
};

static FilterTB nebulae_f[] = {
    {"N Bright",	"BrightNeb",	FIXED,		'N'},
    {"F Diffuse",	"DiffuseNeb",	FIXED,		'F'},
    {"K Dark",		"DarkNeb",	FIXED,		'K'},
    {"P Planetary",	"PlanetaryNeb",	FIXED,		'P'}
};

static FilterTB galaxies_f[] = {
    {"G Spiral",	"SpiralGal",	FIXED,		'G'},
    {"H Spherical",	"SphericalGal",	FIXED,		'H'},
    {"A Clusters",	"GalClusters",	FIXED,		'A'}
};

static FilterTB clusters_f[] = {
    {"C Globular",	"GlobularCl",	FIXED,		'C'},
    {"O Open",		"OpenCl",	FIXED,		'O'},
    {"U in Nebula",	"ClInNeb",	FIXED,		'U'}
};

static FilterCat filters[] = {
    {"Solar System:",
    	"Whether to display Planets, asteroids, comets, Earth satellites",
	solsys_f,	XtNumber(solsys_f)},
    {"Clusters:",
	"Whether to display various types of star clusters",
	clusters_f,	XtNumber(clusters_f)},
    {"Other:",		
	"Whether to display a variety of miscellaneous classes of objects",
	other_f,	XtNumber(other_f)},
    {"Galaxies:",
	"Whether to display galaxies and clusters of galaxies",
	galaxies_f,	XtNumber(galaxies_f)},
    {"Nebulae:",
	"Whether to display various types of nebulae, and planetary nebulae",
	nebulae_f,	XtNumber(nebulae_f)},
    {"Stars:",
	"Whether to display various classes of stars and star-like objects",
	stars_f,	XtNumber(stars_f)},
};

/* these form rapid lookup tables for the state of an object.
 * type_table can be directly indexed by Obj.o_type and, if type is FIXED,
 *   fclass_table by Obj.f.f_class.
 */
static char type_table[NOBJTYPES];
static char fclass_table[NCLASSES];

#define BMAGLIMIT       (-28)   /* brightest setting for the Mag scale */
#define FMAGLIMIT       30      /* faintest setting for the Mag scale */
#define MAXMAGSTP       30      /* mag mag step */
static int stmag, ssmag, dsmag;	/* current values of the mag limits */
static int magstp;		/* current value of mag step */
static Widget stmag_w, ssmag_w, dsmag_w;	/* mag scales */
static Widget magst_w;		/* mag steps scale */
static Widget filter_w;		/* main filter dialog */

static char skyfcategory[] = "Sky View -- Filter";	/* Save category */

/* create, but do not manage, the filter dialog.
 * we also apply its state info immediately to the initial resource
 * settings of the toggle buttons since they are used to set up the real filter
 * right away.
 */
void
svf_create(shell_w)
Widget shell_w;
{
	static struct {
	    char *name;
	    char *tip;
	    void (*cb)();
	} ctls[] = {
	    /* balance in half */
	    {"Ok", "Apply changes and close", svf_ok_cb},
	    {"Apply", "Apply changes and stay up", svf_apply_cb},
	    {"Toggle", "Toggle all selections on->off and off->on",
								svf_toggle_cb},
	    {"All", "Turn all selections on", svf_all_cb},
	    {"Reset", "Restore settings to what is now in effect",svf_reset_cb},
	    {"Close", "Make no changes and close", svf_close_cb},
	    {"Help", "Display additional information", svf_help_cb}
	};
	Arg args[20];
	XmString str;
	Widget w, f_w;
	Widget rc_w, lrc_w, rrc_w, mrc_w;
	int n;
	int i;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNverticalSpacing, 6); n++;
	XtSetArg (args[n], XmNmarginWidth, 8); n++;
	XtSetArg (args[n], XmNmarginHeight, 8); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	filter_w = XmCreateFormDialog (shell_w, "SkyFilter", args, n);
	set_something (filter_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (filter_w, XmNhelpCallback, svf_help_cb, NULL);
	sr_reg (XtParent(filter_w), "XEphem*SkyFilter.x", skyfcategory, 0);
	sr_reg (XtParent(filter_w), "XEphem*SkyFilter.y", skyfcategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Sky view Filter"); n++;
	XtSetValues (XtParent(filter_w), args, n);

	/* put the control buttons across the bottom in a horizontal RC
	 */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNadjustLast, True); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	XtSetArg (args[n], XmNspacing, 2); n++;
	rc_w = XmCreateRowColumn (filter_w, "CRC", args, n);
	XtManageChild (rc_w);

	for (i = 0; i < XtNumber(ctls); i++) {
	    str = XmStringCreateLtoR(ctls[i].name,XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	    XtSetArg (args[n], XmNmarginWidth, 1); n++;
	    XtSetArg (args[n], XmNmarginLeft, 0); n++;
	    XtSetArg (args[n], XmNmarginRight, 0); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreatePushButton (rc_w, "FilterCB", args, n);
	    wtip (w, ctls[i].tip);
	    XtManageChild(w);
	    XmStringFree (str);
	    XtAddCallback (w, XmNactivateCallback, ctls[i].cb, NULL);
	}

	/* pile all but the control buttons in a main rowcolumn */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, rc_w); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNnumColumns, 2); n++;
	mrc_w = XmCreateRowColumn (filter_w, "MRC", args, n);
	XtManageChild (mrc_w);

	/* make the left rc */
	n = 0;
	lrc_w = XmCreateRowColumn (mrc_w, "LRC", args, n);
	XtManageChild (lrc_w);

	/* add half the FilterCats */

	for (i = 0; i < XtNumber(filters)/2; i++)
	    svf_create_filter (lrc_w, &filters[i]);

	/* add sol sys limit scale in a frame */

	n = 0;
	f_w = XmCreateFrame (lrc_w, "SSF", args, n);
	XtManageChild (f_w);

	    str = XmStringCreate ("Sol Sys Lim Mag", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    XtSetArg (args[n], XmNminimum, BMAGLIMIT); n++;
	    XtSetArg (args[n], XmNmaximum, FMAGLIMIT); n++;
	    XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_LEFT); n++;
	    XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	    XtSetArg (args[n], XmNshowValue, True); n++;
	    XtSetArg (args[n], XmNtitleString, str); n++;
	    ssmag_w = XmCreateScale (f_w, "SolSysMag", args, n);
	    XtAddCallback (ssmag_w, XmNdragCallback, svf_magdrag_cb, 0);
	    XtAddCallback (ssmag_w, XmNvalueChangedCallback, svf_magdrag_cb, 0);
	    wtip (ssmag_w, "Faint limit for solar system objects");
	    XtManageChild (ssmag_w);
	    XmStringFree (str);
	    sr_reg (ssmag_w, NULL, skyfcategory, 0);

	/* add mag step scale in a frame */

	n = 0;
	f_w = XmCreateFrame (lrc_w, "SSF", args, n);
	XtManageChild (f_w);

	    str = XmStringCreate ("Mag dot step", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    XtSetArg (args[n], XmNminimum, 1); n++;
	    XtSetArg (args[n], XmNmaximum, MAXMAGSTP); n++;
	    XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	    XtSetArg (args[n], XmNshowValue, True); n++;
	    XtSetArg (args[n], XmNtitleString, str); n++;
	    magst_w = XmCreateScale (f_w, "MagStep", args, n);
	    wtip (magst_w, "Select magnitude change for each dot size step");
	    XtManageChild (magst_w);
	    XmStringFree (str);
	    XmScaleGetValue (magst_w, &magstp);
	    sr_reg (magst_w, NULL, skyfcategory, 0);

	/* make the right rc */
	n = 0;
	XtSetArg (args[n], XmNisAligned, False); n++;
	rrc_w = XmCreateRowColumn (mrc_w, "RRC", args, n);
	XtManageChild (rrc_w);

	/* add the other half of the FilterCat */

	for (i = XtNumber(filters)/2; i < XtNumber(filters); i++)
	    svf_create_filter (rrc_w, &filters[i]);

	/* add stars mag-scale in a frame */

	n = 0;
	f_w = XmCreateFrame (rrc_w, "STF", args, n);
	XtManageChild (f_w);

	    str = XmStringCreate ("Stars Lim Mag", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    XtSetArg (args[n], XmNminimum, BMAGLIMIT); n++;
	    XtSetArg (args[n], XmNmaximum, FMAGLIMIT); n++;
	    XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_LEFT); n++;
	    XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	    XtSetArg (args[n], XmNshowValue, True); n++;
	    XtSetArg (args[n], XmNtitleString, str); n++;
	    stmag_w = XmCreateScale (f_w, "StarMag", args, n);
	    XtAddCallback (stmag_w, XmNdragCallback, svf_magdrag_cb, 0);
	    XtAddCallback (stmag_w, XmNvalueChangedCallback, svf_magdrag_cb, 0);
	    wtip (stmag_w, "Faint limit for stars.");
	    XtManageChild (stmag_w);
	    XmStringFree (str);
	    sr_reg (stmag_w, NULL, skyfcategory, 0);

	n = 0;
	f_w = XmCreateFrame (rrc_w, "DSF", args, n);
	XtManageChild (f_w);

	    str = XmStringCreate ("Deep Sky Lim Mag", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    XtSetArg (args[n], XmNminimum, BMAGLIMIT); n++;
	    XtSetArg (args[n], XmNmaximum, FMAGLIMIT); n++;
	    XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_LEFT); n++;
	    XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	    XtSetArg (args[n], XmNshowValue, True); n++;
	    XtSetArg (args[n], XmNtitleString, str); n++;
	    dsmag_w = XmCreateScale (f_w, "DeepSkyMag", args, n);
	    XtAddCallback (dsmag_w, XmNdragCallback, svf_magdrag_cb, 0);
	    XtAddCallback (dsmag_w, XmNvalueChangedCallback, svf_magdrag_cb, 0);
	    wtip (dsmag_w, "Faint limit for deep sky objects.");
	    XtManageChild (dsmag_w);
	    XmStringFree (str);
	    sr_reg (dsmag_w, NULL, skyfcategory, 0);

	/* set the real filter according to the state of the toggle buttons */
	svf_apply();
}

/* return 1 if we are now up, else 0.
 */
int
svf_ismanaged()
{

	return (filter_w && XtIsManaged(filter_w));
}

/* called to manage the filter dialog.
 * whenever we are managed, reset the state of the toggle buttons back
 * to the way the real filter is.
 */
void
svf_manage()
{
	svf_reset();
	XtManageChild(filter_w);
}

/* called to manage the filter dialog.
 */
void
svf_unmanage()
{
	XtUnmanageChild(filter_w);
}

/* return a copy of the currently enforced type and class tables.
 * this is to support sky history.
 */
void
svf_gettables (tt, ct)
char tt[NOBJTYPES];
char ct[NCLASSES];
{
	(void) memcpy (tt, type_table, NOBJTYPES);
	(void) memcpy (ct, fclass_table, NCLASSES);
}

/* install a new copy of the type and class tables and update the buttons
 *   to match.
 * this is to support sky history.
 */
void
svf_settables (tt, ct)
char tt[NOBJTYPES];
char ct[NCLASSES];
{
	(void) memcpy (type_table, tt, NOBJTYPES);
	(void) memcpy (fclass_table, ct, NCLASSES);

	svf_reset();
}

/* return the current mag limits and mag step */
void
svf_getmaglimits (int *stmagp, int *ssmagp, int *dsmagp, int *magstpp)
{
	*stmagp = stmag;
	*ssmagp = ssmag;
	*dsmagp = dsmag;
	*magstpp = magstp;
}

/* set the current deep- and near-sky limit and mag step */
void
svf_setmaglimits (st, ss, ds, s)
int st, ss, ds, s;
{
	stmag = st;
	ssmag = ss;
	dsmag = ds;
	magstp = s;
	XmScaleSetValue (stmag_w, stmag);
	XmScaleSetValue (ssmag_w, ssmag);
	XmScaleSetValue (dsmag_w, dsmag);
	XmScaleSetValue (magst_w, s);
}

/* return 1 if p satisfies the stuff in the filter dialog, else 0.
 * N.B. because svf_apply() has to have already been called we do not need
 *   to range check the type and class.
 */
int
svf_filter_ok (op)
Obj *op;
{
	int tok;

	if (is_type(op,FIXEDM))
	    tok = fclass_table[((int)op->f_class) & (NCLASSES-1)];
	else
	    tok = type_table[op->o_type];

	if (tok)
	    return (svf_bright_ok (op));

	return (0);
}

/* called to put up or remove the watch cursor.  */
void
svf_cursor (c)
Cursor c;
{
	Window win;

	if (filter_w && (win = XtWindow(filter_w)) != 0) {
	    Display *dsp = XtDisplay(filter_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* establish faintest mags according to given fov */
void
svf_automag(fov)
double fov;
{
	static AutoMag amdef = {degrad(360.0), 6, 6, 8, 1};
	static AutoMag *amp;
	static int namp;
	int i;

	/* establish table sorted in ascending order by r if first time */
	if (namp == 0) {
	    namp = svf_initautomag (&amp);
	    if (namp <= 0) {
		namp = 1;
		amp = &amdef;
	    }
	}

	/* search for largest entry <= current fov */
	for (i = 0; i < namp && fov > amp[i].r; i++)
	    continue;
	i %= namp;
	svf_setmaglimits (amp[i].st, amp[i].ss, amp[i].ds, amp[i].s);
}

/* create one FilterCat filter category in the given RowColumn */
static void
svf_create_filter (mainrc, fcp)
Widget mainrc;
FilterCat *fcp;
{
	Arg args[20];
	Widget l_w;
	Widget rc_w;	/* rc for the label and controls */
	Widget fr_w, da_w;
	Widget w;
	int n;
	int i;

	/* make a rc within a frame for the tb rowcol */

	n = 0;
	fr_w = XmCreateFrame (mainrc, "Frame", args, n);
	XtManageChild (fr_w);
	n = 0;
	XtSetArg (args[n], XmNmarginWidth, 0); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	rc_w = XmCreateRowColumn (fr_w, "CategoryRC", args, n);
	XtManageChild (rc_w);

	/* make a label for the category */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	l_w = XmCreateLabel (rc_w, fcp->label, args, n);
	wtip (l_w, fcp->ltip);
	XtManageChild (l_w);

	/* make a pushbutton used to toggle all entries in this category */
	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	w = XmCreatePushButton (rc_w, "Toggle", args, n);
	wtip (w, "Toggle all selections in this category");
	XtAddCallback (w, XmNactivateCallback, svf_cat_toggle_cb,
								(XtPointer)fcp);
	XtManageChild (w);

	/* add each filter */

	for (i = 0; i < fcp->nftb; i++) {
	    Widget f_w;
	    Widget tb_w;
	    FilterTB *ftbp = &fcp->ftb[i];
	    XmString str;

	    n = 0;
	    f_w = XmCreateForm (rc_w, "FTBF", args, n);
	    XtManageChild (f_w);

		/* create a frame around a drawing area in which to show the
		 * object symbol
		 */
		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
		fr_w = XmCreateFrame (f_w, "SymbolF", args, n);
		XtManageChild (fr_w);
		n = 0;
		da_w = XmCreateDrawingArea (fr_w, "SymbolDA", args, n);
		XtAddCallback (da_w, XmNexposeCallback, svf_da_cb,
							    (XtPointer)ftbp);
		XtManageChild (da_w);

		/* create the filter selection toggle button */

		str = XmStringCreateLtoR (ftbp->label,XmSTRING_DEFAULT_CHARSET);
		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
		XtSetArg (args[n], XmNleftWidget, fr_w); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNlabelString, str); n++;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
		tb_w = XmCreateToggleButton (f_w, ftbp->name, args, n);
		XmStringFree(str);
		XtManageChild (tb_w);
		ftbp->tbw = tb_w;
		sr_reg (tb_w, NULL, skyfcategory, 0);
	}
}

/* set the real filter states and the toggle button states according to the
 *   rapid-access tables.
 * also, set the mag scales and dot scales from the real mag limits.
 */
static void
svf_reset()
{
	int i, j;

	for (i = 0; i < XtNumber(filters); i++) {
	    FilterCat *fcp = &filters[i];
	    for (j = 0; j < fcp->nftb; j++) {
		FilterTB *ftbp = &fcp->ftb[j];
		ftbp->state = (ftbp->type == FIXED)
		    ? fclass_table[(int)ftbp->class] : type_table[ftbp->type];
		XmToggleButtonSetState (ftbp->tbw, ftbp->state, True);
	    }
	}

	svf_setmaglimits (stmag, ssmag, dsmag, magstp);
}

/* set the real filter and rapid-access tables according to the present state
 *   of the toggle buttons.
 * also set the mag limits and dot scale from their controls.
 */
static void
svf_apply()
{
	int i, j;

	for (i = 0; i < XtNumber(filters); i++) {
	    FilterCat *fcp = &filters[i];
	    for (j = 0; j < fcp->nftb; j++) {
		FilterTB *ftbp = &fcp->ftb[j];
		int t = ftbp->type;
		if (t < 0 || t >= XtNumber(type_table)) {
		    printf ("svf_apply: type out of range: %d\n", t);
		    abort();
		}
		ftbp->state = XmToggleButtonGetState (ftbp->tbw);
		if (t == FIXED) {
		    int c = (int)ftbp->class;
		    if (c < 0 || c >= XtNumber(fclass_table)) {
			printf ("svf_apply: FIXED class out of range: %d\n", c);
			abort();
		    }
		    fclass_table[c] = ftbp->state;
		} else
		    type_table[t] = ftbp->state;
	    }
	}

	svtb_updateCTTT(fclass_table, type_table);

	XmScaleGetValue (stmag_w, &stmag);
	XmScaleGetValue (ssmag_w, &ssmag);
	XmScaleGetValue (dsmag_w, &dsmag);
	XmScaleGetValue (magst_w, &magstp);
}

/* return 1 if db object is ever possibly brighter than allowed range; else 0.
 * don't worry too much, this is just a first-stage culling. real mag cutoff
 * is done later.
 */
static int
svf_bright_ok(op)
Obj *op;
{
	switch (op->o_type) {
	case PLANET:
	    /* always just go for the planets */
	    return (1);
	    /* break; */
	case HYPERBOLIC: return (1);	/* and interlopers */
	case PARABOLIC:	 return (1);
	case ELLIPTICAL: {
	    double mag;		/* magnitude */
	    double per, aph;	/* perihelion and aphelion distance */

	    per = op->e_a*(1.0 - op->e_e);
	    aph = op->e_a*(1.0 + op->e_e);
	    if (per <= 1.1 && aph >= 0.9)
		return (1); /* might be blazing in the back yard some day */
	    if (op->e_mag.whichm == MAG_HG)
		 mag = op->e_mag.m1 + 5*log10(per*fabs(per-1.0));
	    else
		 gk_mag(op->e_mag.m1, op->e_mag.m2,
						per, fabs(per-1.0), &mag);
	    return (mag <= ssmag);
	    /* break; */
	    }
	case FIXED:		/* FALLTHRU */
	case BINARYSTAR:
	    return (get_mag(op) <= (is_deepsky(op) ? dsmag : stmag));
	    /* break; */
	case EARTHSAT:
	    /* TODO: work on satellite magnitudes someday */
	    return (1);
	default: 
	    printf ("sv_bright_ok(): bad type: %d\n", op->o_type);
	    abort();
	    return (0);	/* for lint */
	}
}

/* called when a symbol drawing area is exposed.
 * client is a pointer to the FilterTB.
 */
/* ARGSUSED */
static void
svf_da_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
	FilterTB *ftbp = (FilterTB *) client;

	switch (c->reason) {
	case XmCR_EXPOSE: {
	    /* turn off gravity so we get expose events for either shrink or
	     * expand.
	     */
	    static int before;
	    XExposeEvent *e = &c->event->xexpose;

	    if (!before) {
		XSetWindowAttributes swa;
		swa.bit_gravity = ForgetGravity;
		XChangeWindowAttributes (e->display, e->window, 
							    CWBitGravity, &swa);
		before = 1;
	    }
	    /* wait for the last in the series */
	    if (e->count != 0)
		return;
	    }
	    break;
	default:
	    return;
	}

	svf_draw_symbol(ftbp, w);
}

/* draw the symbol used for the object described by ftbp into the
 * DrawingArea widget daw.
 */
static void
svf_draw_symbol(ftbp, daw)
FilterTB *ftbp;
Widget daw;
{
	static GC gc;
	static Pixel fg_p, bg_p;
	Display *dsp = XtDisplay(daw);
	Window win = XtWindow(daw);
	Obj o;
	Dimension w, h;
	int diam;

	zero_mem ((void *)&o, sizeof(Obj));
	o.o_type = ftbp->type;
	switch (ftbp->type) {
	case BINARYSTAR:
	    o.b_nbp = 1;	/* just to avoid orb ele calc */
	    break;
	case FIXED:
	    o.f_class = ftbp->class;
	    set_ratio (&o, 2, 1);
	    set_pa (&o, PI/2);
	    break;
	case PLANET:
	    o.s_phase = 100;
	    break;
	}

	get_something (daw, XmNwidth, (XtArgVal)&w);
	get_something (daw, XmNheight, (XtArgVal)&h);
	diam = (w > h ? h : w) - 2; /* allow for a small border */

	if (!gc) {
	    get_something (daw, XmNforeground, (XtArgVal)&fg_p);
	    get_something (daw, XmNbackground, (XtArgVal)&bg_p);
	    gc = XCreateGC (dsp, win, 0L, NULL);
	}

	XSetForeground (dsp, gc, bg_p);
	XFillRectangle (dsp, win, gc, 0, 0, w, h);
	XSetForeground (dsp, gc, fg_p);
	sv_draw_obj (dsp, win, gc, &o, (int)w/2-1, (int)h/2-1, diam, 0);
	sv_draw_obj (dsp, win, gc, NULL, 0, 0, 0, 0);	/* flush */
}

/* called when either mag scale is dragged */
/* ARGSUSED */
static void
svf_magdrag_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	sv_amagoff();
}

/* called when Apply is pushed on the filter dialog */
/* ARGSUSED */
static void
svf_apply_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	svf_apply();
	sv_all(mm_get_now());
}

/* called when Ok is pushed on the filter dialog */
/* ARGSUSED */
static void
svf_ok_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	svf_apply();
	sv_all(mm_get_now());
	XtUnmanageChild (filter_w);
}

/* called when Close is pushed on the filter dialog */
/* ARGSUSED */
static void
svf_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (filter_w);
}

/* callback from the Help button.
 */
/* ARGSUSED */
static void
svf_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"This allows settings the types of objects to display."
};

	hlp_dialog ("SkyView_filter", msg, sizeof(msg)/sizeof(msg[0]));
}

/* called when Toggle is pushed on the filter dialog */
/* ARGSUSED */
static void
svf_toggle_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i, j;

	for (i = 0; i < XtNumber(filters); i++) {
	    FilterCat *fcp = &filters[i];
	    for (j = 0; j < fcp->nftb; j++) {
		FilterTB *ftbp = &fcp->ftb[j];
		XmToggleButtonSetState (ftbp->tbw, 
		    !XmToggleButtonGetState(ftbp->tbw), True);
	    }
	}
}

/* called when All is pushed on the filter dialog */
/* ARGSUSED */
static void
svf_all_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i, j;

	for (i = 0; i < XtNumber(filters); i++) {
	    FilterCat *fcp = &filters[i];
	    for (j = 0; j < fcp->nftb; j++) {
		FilterTB *ftbp = &fcp->ftb[j];
		XmToggleButtonSetState (ftbp->tbw, True, True);
	    }
	}
}

/* called when Reset is pushed on the filter dialog */
/* ARGSUSED */
static void
svf_reset_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	svf_reset();
}

/* called when the "Toggle" button is activated on a particular category.
 * client is the FilterCat pointer.
 */
/* ARGSUSED */
static void
svf_cat_toggle_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	FilterCat *fcp = (FilterCat *) client;
	int i;

	for (i = 0; i < fcp->nftb; i++) {
	    FilterTB *ftbp = &fcp->ftb[i];
	    XmToggleButtonSetState (ftbp->tbw, 
		!XmToggleButtonGetState(ftbp->tbw), True);
	}
}

/* given two pointers to MagAuto, return how they sort by r in qsort-style.
 */
static int
am_cf (const void * v1, const void * v2)
{
	AutoMag *am1 = (AutoMag *)v1;
	AutoMag *am2 = (AutoMag *)v2;
	double r1 = am1->r;
	double r2 = am2->r;

	if (r1 < r2)
	    return (-1);
	if (r1 > r2)
	    return (1);
	return (0);
}

/* digest the AutoMag resource.
 * the format is five values in each group, groups separated by commas.
 * each group is max fov, star mag, sol sys mag, deep sky mag, step-size.
 * return number of entries and put into *ampp.
 * array will be sorted into ascending order of fov radius.
 * N.B. don't set *ampp unless we return > 0.
 */
static int
svf_initautomag(ampp)
AutoMag **ampp;
{
	static char AMResource[] = "AutoMag";
	double r;
	int st, ss, ds, s;
	AutoMag *amp;
	int sam;
	int namp;
	char *am;
	int i;

	/* get the automag resource */
	am = getXRes (AMResource, NULL);
	if (!am) {
	    xe_msg (0, "No %s resource found", AMResource);
	    return (0);
	}

	/* scan for the quads -- build up amp as we go.
	 * stop at first sign of trouble.
	 */
	sam = sizeof(AutoMag);
	amp = (AutoMag *)0;
	namp = 0;
	while (sscanf (am, "%lf %d %d %d %d", &r, &st, &ss, &ds, &s) == 5) {
	    char *new = amp ? realloc ((void *)amp, (namp+1)*sam) : malloc(sam);
	    AutoMag *tamp;

	    if (!new) {
		xe_msg (0, "Insufficient memory for AutoMag table");
		break;
	    }
	    amp = (AutoMag *)new;
	    tamp = &amp[namp++];
	    tamp->r = degrad(r);
	    tamp->st = st;
	    tamp->ss = ss;
	    tamp->ds = ds;
	    tamp->s = s;

	    /* skip past next comma */
	    while (*am != ',' && *am != '\0')
	    	am++;
	    if (*am == '\0')
		break;
	    am++;
	}

	if (namp == 0) {
	    xe_msg (0, "No valid entries in AutoMag resource");
	    return (0);
	}

	/* sort into ascending order of fov radius */
	qsort ((void *)amp, namp, sizeof(AutoMag), am_cf);

	/* report results to message log because the format is a bit obscure */
	xe_msg(0, "Sky View found %d AutoMag table entries as follows:", namp);
	xe_msg(0, "     FOV  Stars    SolSys   DeepSky   DotSiz");
	for (i = 0; i < namp; i++) {
	    xe_msg (0, "  %6.2f    %3d      %3d    %3d       %3d",
		raddeg(amp[i].r), amp[i].st, amp[i].ss, amp[i].ds, amp[i].s);
	}

	*ampp = amp;
	return (namp);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: skyfiltmenu.c,v $ $Date: 2004/05/05 17:43:32 $ $Revision: 1.24 $ $Name:  $"};

/* code to manage the stuff on the "data" menu.
 * functions for the main data table are prefixed with dm.
 * functions for the setup menu are prefixed with ds.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/SelectioB.h>
#include <Xm/List.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>

#include "xephem.h"

static double dm_uhzndep (void);
static int ds_apply_selections (void);
static void dm_set_buttons (int whether);
static void dm_activate_cb (Widget w, XtPointer client, XtPointer call);
static void dm_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void dm_close_cb (Widget w, XtPointer client, XtPointer call);
static void dm_help_cb (Widget w, XtPointer client, XtPointer call);
static void dm_compute (int r, int force, Now *np);
static void dm_format (Now *np, Obj *op, RiseSet *rp, int c, Widget w);
static void dm_settags (void);
static void dm_showtim (Now *np, Widget w, double t);
static void dm_rs_hrsup (Now *np, Obj *op, Widget w, RiseSet *rp);
static void dm_build_cols (void);
static void dm_selname (Widget pb_w, int r, int c);
static void dm_resetuhzndep (void);
static void show_constellation (Now *np, Obj *op, Widget w);
static void ds_create_selection (Widget parent);
static void ds_buildfsl (void);
static void ds_setup_col_selections (void);
static void ds_ctl_cb (Widget w, XtPointer client, XtPointer call);
static void ds_help (void);

static void dm_create_flist_w (void);
static void dm_flistok_cb (Widget w, XtPointer client, XtPointer call);
static void dm_flist_cb (Widget w, XtPointer client, XtPointer call);
static void dm_xsel_cb (Widget w, XtPointer client, XtPointer call);
static void flistok_append_cb (void);
static void flistok_overwrite_cb (void);
static void make_flist (char *name, char *how);
static void dm_list_tofile (FILE *fp);
static void dm_list_get (char buf[]);

/* main sections in the Setup window */
typedef enum {
    MISC_COL, RISET_COL, SEP_COL
} ColType;

typedef struct {
    ColType type;	/* general class of this column */
    char *name;		/* name of column */
    char *tip;		/* widget tip text, or NULL */
    int on;		/* whether this column is currently to be on */
    Widget rcw;		/* RowColumn widget for this column */
    Widget lw;		/* label for this column's header (first in rcw) */
    Widget sw;		/* pushbutton widget for this col in selection menu */
} ColHdr;

/* column details.
 * N.B. these must match the DMCol order
 * this does not include the first column holding the object names.
 */
static ColHdr col[] = {
    {MISC_COL,	"Cns",	   "Constellation"},
    {MISC_COL,	"RA",	   "Right Ascension (to Main's settings)"},
    {MISC_COL,	"HA",	   "Hour Angle"},
    {MISC_COL,	"GHA",	   "Greenwich Hour Angle"},
    {MISC_COL,	"Dec",	   "Declination (to Main's settings)"},
    {MISC_COL,	"Az",	   "Azimuth, E of N"},
    {MISC_COL,	"Alt",	   "Angle above horizon"},
    {MISC_COL,	"Zenith",  "Angle down from zenith"},
    {MISC_COL,	"PA",      "Parallactic angle, +W"},
    {MISC_COL,	"JD",      "Julian date"},
    {MISC_COL,	"HJD",     "Heliocentric Julian date"},
    {MISC_COL,	"Air",     "Air mass"},

    {MISC_COL,	"VMag",	   "Apparent magnitude"},
    {MISC_COL,	"PMRA",	   "Proper motion in RA: Solsys as/hr, ESat deg/min, else mas/yr on sky"},
    {MISC_COL,	"PMDec",   "Proper motion in Dec: Solsys as/hr, ESat deg/min, else mas/yr"},
    {MISC_COL,	"Size",	   "Angular diameter, arc seconds"},
    {MISC_COL,	"Phase",   "Percent illumination seen from Earth"},
    {MISC_COL,	"Elong",   "Elongation: angular degrees from Sun, +E"},
    {MISC_COL,	"Spect",   "Spectral classification"},
    {MISC_COL,	"HeLat",   "Heliocentric latitude"},
    {MISC_COL,	"HeLong",  "Heliocentric longitude"},
    {MISC_COL,	"GLat",	   "Galactic latitude"},

    {MISC_COL,	"GLong",   "Galactic longitude"},
    {MISC_COL,	"EcLat",   "Ecliptic latitude"},
    {MISC_COL,	"EcLong",  "Ecliptic longitude"},
    {MISC_COL,	"EaDst",   "Distance from Earth, AU (moon is km)"},
    {MISC_COL,	"EaLght",  "Light travel time from Earth"},
    {MISC_COL,	"SnDst",   "Distance from Sun, AU"},
    {MISC_COL,	"SnLght",  "Light travel time from Sun"},
    {MISC_COL,	"Uranom",  "Volume and page number in Uranometria"},
    {MISC_COL,	"Uran2k",  "Volume and page number in Uranometria 2000"},
    {MISC_COL,	"MillSA",  "Volume and page number in Millenium Star Atlas"},

    {RISET_COL,	"RisTm",   "Rise time, today"},
    {RISET_COL,	"RisAz",   "Rise azimuth, today"},
    {RISET_COL,	"TrnTm",   "Transit time, today"},
    {RISET_COL,	"TrnAlt",  "Transit altitude, today"},
    {RISET_COL,	"TrnAz",   "Transit azimuth, today"},
    {RISET_COL,	"SetTm",   "Set time, today"},
    {RISET_COL,	"SetAz",   "Set azimuth, today"},
    {RISET_COL,	"HrsUp",   "Number of hours object is up, today"},

    /* N.B. assumed to be last */
    {SEP_COL,   "Sep",     "Angular separation from selected favorite"},
};

#define	NC	XtNumber(col)

/* tags for the various Data Selection control panel buttons */
typedef enum {
    OK, APPLY, CLALL, RESET, CANCEL, HELP
} DSCtrls;

typedef Widget RowW[NC];	/* one row's worth of widget ids */ 
static Widget datashell_w;	/* the overall table shell */
static Widget hdrcol_w;		/* RowColumn for object name column */
static Widget *hdr_w;		/* malloced list of object name labels */
static RowW *tblpb;		/* malloced list of nfavs RowW */
static Widget centric_w;	/* the centric label on the data table */
static Widget limbl_w;		/* the Limb label on the data table */
static Widget dt_w;		/* date/time stamp label widget */
static int nrows;		/* n rows in table, not counting header */

typedef enum {LIMB, CENTER} RSPosOpt;

static Widget selshell_w;	/* overall setup table shell */
static Widget hzn_w;		/* TF holding the user's horizon setting */
static Widget limb_w;		/* the Center/Limb toggle button */
static RSPosOpt limb;		/* one of CENTER or LIMB */
static Widget flist_w;		/* file list dialog widget */
static Widget fsl_w;		/* favorites scrolled list */

static int dm_selecting;	/* set while our fields are being selected */
static double hznd;		/* horizon displacement, rads */

static Obj **favs;		/* current list of loaded favorite objects */
static int nfavs;		/* n favs[] */
static Obj *sepop;		/* separation object */
static Widget sepl_w;		/* hidden label to save sep partner */

/* Save categories */
static char dscategory[] = "Data Table";

/* called when the data menu is activated via the main menu pulldown.
 * if never called before, create all the widgets.
 */
void
dm_manage ()
{
	if (!datashell_w)
	    dm_create_shell();

	dm_update (mm_get_now(), 1);
	dm_set_buttons (dm_selecting);

	XtPopup (datashell_w, XtGrabNone);
	set_something (datashell_w, XmNiconic, (XtArgVal)False);

	/* register we are now up */
	setXRes (dm_viewupres(), "1"); 
}

/* called to recompute and fill in values for the data menu.
 * don't bother if it doesn't exist or is unmanaged now or no one is logging.
 */
void
dm_update (np, how_much)
Now *np;
int how_much;
{
	int i;

	if (!datashell_w)
	    return;
	if (!isUp(datashell_w) && !any_ison() && !how_much)
	    return;

	watch_cursor (1);

	/* update each row */
	for (i = 0; i < nfavs; i++)
	    dm_compute (i, how_much, np);

	/* update the indicators */
	dm_settags();

	/* update the datestamp */
	timestamp (np, dt_w);

	watch_cursor (0);
}

/* called whenever the favorites list changes */
void
dm_newfavs()
{
	if (!datashell_w)
	    return;
	nfavs = fav_get_loaded (&favs);
	ds_buildfsl();
	dm_build_cols();
	dm_update(mm_get_now(), 1);
}

/* called by other menus as they want to hear from our buttons or not.
 * the "on"s and "off"s stack - only really redo the buttons if it's the
 * first on or the last off.
 */
void
dm_selection_mode (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	if (whether)
	    dm_selecting++;
	else if (dm_selecting > 0)
	    --dm_selecting;

	if (datashell_w)
	    if ((whether && dm_selecting == 1)     /* first one to want on */
		|| (!whether && dm_selecting == 0) /* last one to want off */)
		dm_set_buttons (whether);
}

/* return the name of the resource containing whether this view is up */
char *
dm_viewupres()
{
	return ("DataViewUp");
}

/* return the user's horizon displacement, in rads.
 * + means less sky.
 */
static double
dm_uhzndep()
{
	char *str = XmTextFieldGetString (hzn_w);
	double dis = degrad(atod(str));
	XtFree (str);
	return (dis);
}

/* set text field to hznd */
static void
dm_resetuhzndep()
{
	char buf[32];
	sprintf (buf, "%g", raddeg(hznd));
	XmTextFieldSetString (hzn_w, buf);
}

/* given the Now and Obj, fill in the RiseSet.
 * this takes into account the options currently in effect with the Data menu.
 */
void
dm_riset (np, op, rsp)
Now *np;
Obj *op;
RiseSet *rsp;
{
	double dis;	/* rads apparent horizon is above true */

	/* might get called before we have been managed the first time */
	if (!datashell_w)
	    dm_create_shell();

	/* user's displacement correction */
	dis = -hznd;

	/* add semi-diameter if want wrt limb */
        if (limb == LIMB)
	    dis += degrad(op->s_size/3600./2.0);

	riset_cir (np, op, dis, rsp);
}

/* called to put up or remove the watch cursor.  */
void
dm_cursor (c)
Cursor c;
{
	Window win;

	if (datashell_w && (win = XtWindow(datashell_w)) != 0) {
	    Display *dsp = XtDisplay(datashell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}

	if (selshell_w && (win = XtWindow(selshell_w)) != 0) {
	    Display *dsp = XtDisplay(selshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* given a DMCol, fill str[] with its column heading.
 * return 0 if column is on, else -1
 */
int
dm_colHeader (c, str)
DMCol c;
char str[];
{
	if (!col[c].on)
	    return (-1);
	strcpy (str, col[c].name);
	return (0);
}

/* given circumstances and DMCol, fill str[] with its string value.
 * return 0 if column is on, else -1 (but always fill in str).
 */
int
dm_colFormat (np, op, rp, c, str)
Now *np;
Obj *op;
RiseSet *rp;
DMCol c;
char str[];
{
	static Widget w;
	char *lstr;

	/* we get the value round-about via a widget because dm_format() can
	 * not be written to return a string (because it must use the funcs
	 * that call field_log())
	 */
	if (!w)
	    w = XmCreateLabel (toplevel_w, "DMFT", NULL, 0);
	dm_format (np, op, rp, c, w);
	get_xmstring (w, XmNlabelString, &lstr);
	(void) strcpy (str, lstr);
	XtFree (lstr);

	return (col[c].on ? 0 : -1);
}

/* N.B. might be called early if preloading certain .edb files
 * each column is a vertical RC, stacked into one horizontal RC.
 * the first children of each column is a label and is always there.
 * the remaining children area recreated each time Favorites changes.
 */
void
dm_create_shell()
{
	Widget ctlrc_w, w;
	Widget mb_w, cb_w, pd_w;
	Widget table_w;
	Widget dform_w;
	Arg args[20];
	int i, n;

	/* benign if called > 1 */
	if (datashell_w)
	    return;

	/* create the shell and main form */
	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Data Table"); n++;
	XtSetArg (args[n], XmNiconName, "Data"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	datashell_w = XtCreatePopupShell ("DataTable", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (datashell_w);
	set_something (datashell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (datashell_w, XmNpopdownCallback, dm_popdown_cb, 0);
	sr_reg (datashell_w, "XEphem*DataTable.x", dscategory, 0);
	sr_reg (datashell_w, "XEphem*DataTable.y", dscategory, 0);
	sr_reg (NULL, dm_viewupres(), dscategory, 0);

	n = 0;
	dform_w = XmCreateForm (datashell_w, "DTForm", args, n);
	XtAddCallback (dform_w, XmNhelpCallback, dm_help_cb, 0);
	XtManageChild (dform_w);

	/* make the menubar */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (dform_w, "MB", args, n);
	XtManageChild (mb_w);

	    /* make the Control pulldown */

	    n = 0;
	    pd_w = XmCreatePulldownMenu (mb_w, "CPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    XtSetArg (args[n], XmNmnemonic, 'C'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Control", args, n);
	    XtManageChild (cb_w);

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Setup", args, n);
	    XtAddCallback (w, XmNactivateCallback, dm_setup_cb, 0);
	    set_xmstring (w, XmNlabelString, "Setup...");
	    wtip (w, "Select desired columns for the table");
	    XtManageChild (w);

	    n = 0;
	    w = XmCreatePushButton (pd_w, "List", args, n);
	    XtAddCallback (w, XmNactivateCallback, dm_flist_cb, 0);
	    set_xmstring (w, XmNlabelString, "List...");
	    wtip (w, "Save current table to a file");
	    XtManageChild (w);

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Select", args, n);
	    XtAddCallback (w, XmNactivateCallback, dm_xsel_cb, 0);
	    set_xmstring (w, XmNlabelString, "X Select");
	    wtip (w, "Save current table to the X PRIMARY Selection buffer");
	    XtManageChild (w);

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Fav", args, n);
	    XtAddCallback (w, XmNactivateCallback,(XtCallbackProc)fav_manage,0);
	    set_xmstring (w, XmNlabelString, "Favorites...");
	    wtip (w, "Choose rows in table");
	    XtManageChild (w);

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, dm_close_cb, 0);
	    wtip (w, "Close this and all supporting dialogs");
	    XtManageChild (w);

	    /* make the Help pulldown */

	    n = 0;
	    pd_w = XmCreatePulldownMenu (mb_w, "HPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    XtSetArg (args[n], XmNmnemonic, 'H'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Help", args, n);
	    XtManageChild (cb_w);
	    set_something (mb_w, XmNmenuHelpWidget, (XtArgVal)cb_w);

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Help", args, n);
	    XtAddCallback (w, XmNactivateCallback, dm_help_cb, 0);
	    XtManageChild (w);

	/* make a rowcolumn for the bottom control panel */

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
	ctlrc_w = XmCreateRowColumn (dform_w, "DataTblRC", args, n);
	XtManageChild (ctlrc_w);

	    /* make the limb and centric indicators in frames.
	     * turn them on and off by managing the frames -- but not yet!
	     */

	    n = 0;
	    w = XmCreateFrame (ctlrc_w, "DLimblF", args, n);
	    n = 0;
	    limbl_w = XmCreateLabel (w, "DLimblL", args, n);
	    wtip (limbl_w, "Indicates whether Rise/Set data are for Limb or Center");
	    XtManageChild (limbl_w);

	    n = 0;
	    w = XmCreateFrame (ctlrc_w, "DCentricF", args, n);
	    n = 0;
	    centric_w = XmCreateLabel (w, "DCentricL", args, n);
	    wtip (centric_w, "Indicates Geocentric or Topocentric vantage, and equinox");
	    XtManageChild (centric_w);

	/* center a label for the date/time stamp in remaining space */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, ctlrc_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	dt_w = XmCreateLabel (dform_w, "DateStamp", args, n);
	wtip (dt_w, "Date and Time for which data are computed");
	timestamp (mm_get_now(), dt_w);	/* sets initial size correctly*/
	XtManageChild (dt_w);

	/* create the table - a bunch of V RCs in one H RC */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, ctlrc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	table_w = XmCreateRowColumn (dform_w, "DataTable", args, n);
	XtManageChild (table_w);

	    /* name column */

	    n = 0;
	    hdrcol_w = XmCreateRowColumn (table_w, "HC", args, n);
	    XtManageChild (hdrcol_w);

	    /* first child is just label for the upper left gap */

	    n = 0;
	    w = XmCreateLabel (hdrcol_w, "Gap", args, n);
	    set_xmstring (w, XmNlabelString, " ");
	    XtManageChild (w);

	    /* table columns */

	    for (i = 0; i < NC; i++) {
		n = 0;
		XtSetArg (args[n], XmNadjustMargin, False); n++;
		XtSetArg (args[n], XmNisAligned, False); n++;
		col[i].rcw = XmCreateRowColumn (table_w, "DC", args, n);
		XtManageChild (col[i].rcw);

		/* first child is label for column name */

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
		w = XmCreateLabel (col[i].rcw, "CH", args, n);
		set_xmstring (w, XmNlabelString, col[i].name);
		XtManageChild (w);
	    }

	    /* other entries added whenever favs change */

	/* define default separation target, if nothing else for dm_colFormat */
	sepop = db_basic (SUN);

	/* create the selection dialog.
	 * don't manage it yet but its state info is used right off.
	 */
	ds_create_selection(datashell_w);

	/* install initial favorites */
	dm_newfavs();
	ds_apply_selections();
}

/* callback to bring up the Data setup dialog */
/* ARGSUSED */
void
dm_setup_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (!selshell_w)
	    dm_create_shell();

	if (!XtIsManaged(selshell_w)) {
	    watch_cursor(1);
	    ds_setup_col_selections();
	    XtManageChild (selshell_w);
	    watch_cursor(0);
	}
}

/* return the separatoin between the given and the current table sep objects */
double
dm_sep (Obj *op)
{
	double sep;
	dm_separation (op, sepop, &sep);
	return (sep);
}

/* go through all the table buttons and set whether they
 * should appear to look like buttons or just flat labels.
 */
static void
dm_set_buttons (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	int r, c;

	for (c = 0; c < NC; c++)
	    for (r = 0; r < nfavs; r++)
		buttonAsButton (tblpb[r][c], whether);
}

/* make sure there are nfavs buttons in each col and tblpb[] points to them.
 * n rows in tblpb match n widgets in each col (+1 for column header)
 * more added as needed, unmanaged when not.
 * all cols will have the same number of rows.
 */
static void
dm_build_cols()
{
	int r, c;

	/* insure we have at least nfavs rows */
	if (nrows < nfavs) {
	    tblpb = (RowW *) XtRealloc ((char*)tblpb, nfavs*sizeof(RowW));
	    hdr_w = (Widget *) XtRealloc ((char*)hdr_w, nfavs*sizeof(Widget));
	    for (r = nrows; r < nfavs; r++) {
		/* object name column. use PB for size, not to ever look like */
		Widget w = XmCreatePushButton (hdrcol_w, "D", NULL, 0);
		buttonAsButton (w, 0);
		XtManageChild (w);
		hdr_w[r] = w;

		/* each data column */
		for (c = 0; c < NC; c++) {
		    XtPointer code = (XtPointer)(long int)((c<<8)|r);
		    w = XmCreatePushButton (col[c].rcw, "D", NULL, 0);
		    XtAddCallback (w, XmNactivateCallback, dm_activate_cb,code);
		    buttonAsButton (w, 0);
		    XtManageChild (w);
		    tblpb[r][c] = w;
		}
	    }

	    nrows = nfavs;
	}

	/* turn on exactly the first nfavs rows and set row names */
	for (r = 0; r < nrows; r++) {
	    if (r < nfavs) {
		set_xmstring (hdr_w[r], XmNlabelString, favs[r]->o_name);
		XtManageChild (hdr_w[r]);
	    } else
		XtUnmanageChild (hdr_w[r]);
	    for (c = 0; c < NC; c++) {
		if (r < nfavs) {
		    XtManageChild (tblpb[r][c]);
		    buttonAsButton (tblpb[r][c], dm_selecting);
		    dm_selname (tblpb[r][c], r, c);
		} else
		    XtUnmanageChild (tblpb[r][c]);
	    }
	}
}

/* set the userData name of table button pb_w knowing it is at [r,c] */
static void
dm_selname (Widget pb_w, int r, int c)
{
	char *name, *rname, *cname;
	char *userD;   /* Heller, pg 852, say's this is type Pointer?? */

	/* figure out our row and col name */
	rname = favs[r]->o_name;
	cname = col[c].name;

	/* combination is our plotting id */
	name = XtMalloc (strlen(rname) + strlen(cname) + 2); /* '.' '\0' */
	(void) sprintf (name, "%s.%s", rname, cname);

	/* set XmNuserData to be the name we want to go by */
	get_something (pb_w, XmNuserData, (XtArgVal)&userD);
	if (userD)
	    XtFree (userD);
	userD = name;
	set_something (pb_w, XmNuserData, (XtArgVal)userD);
}

/* callback from any of the data table buttons being activated.
 */
static void
/* ARGSUSED */
dm_activate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (dm_selecting) {
	    char *name;
	    get_something (w, XmNuserData, (XtArgVal)&name);
	    register_selection (name);
	}
}

/* callback when main dialog is popped down */
static void
/* ARGSUSED */
dm_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (selshell_w && XtIsManaged (selshell_w))
	    XtUnmanageChild (selshell_w);
	if (flist_w && XtIsManaged (flist_w))
	    XtUnmanageChild (flist_w);

	/* register we are now down */
	setXRes (dm_viewupres(), "0");
}

/* callback from the Data table Close button
 */
static void
/* ARGSUSED */
dm_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* let popdown do the real work */
	XtPopdown (datashell_w);
}

/* callback from the Data table Help button
 */
static void
/* ARGSUSED */
dm_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"This table displays various information about the planets and objects.",
"To reduce computation and save screen space, each row and column may be",
"individually turned off or on using the Select button."
};

	hlp_dialog ("Data_Table", msg, XtNumber(msg));
}

/* compute and print body info in data menu format */
/* ARGSUSED */
static void
dm_compute (r, force, np)
int r;		/* which row */
int force;	/* whether to print for sure or only if things have changed */
Now *np;
{
	int did_rs = 0;
	RiseSet rs;
	Obj *op;
	int c;

	op = favs[r];
	db_update (op);

	for (c = 0; c < NC; c++)
	    if (col[c].on) {
		if (col[c].type == RISET_COL && !did_rs) {
		    dm_riset (np, op, &rs);
		    did_rs = 1;
		}
		dm_format (np, op, &rs, c, tblpb[r][c]);
	    }
}

static void
dm_format (np, op, rp, c, w)
Now *np;
Obj *op;
RiseSet *rp;
int c;
Widget w;
{
	static char me[] = "dm_format()";
	double tmp;

	if (op->o_flags & NOCIRCUM) {
	    f_string (w, "no circ");
	    return;
	}

	switch (c) {
	case CONSTEL_ID:
	    show_constellation (np, op, w);
	    break;
	case RA_ID:
	    f_ra (w, op->s_ra);
	    break;
	case HA_ID:
	    radec2ha (np, op->s_ra, op->s_dec, &tmp);
	    tmp = radhr (tmp);
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		f_sexa (w, tmp, 3, 600);
	    else
		f_sexa (w, tmp, 3, 360000);
	    break;
	case GHA_ID: {
	    gha (np, op, &tmp);
	    tmp = radhr (tmp);
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		f_sexa (w, tmp, 3, 600);
	    else
		f_sexa (w, tmp, 3, 360000);
	    }
	    break;
	case DEC_ID:
	    f_prdec (w, op->s_dec);
	    break;
	case AZ_ID:
	    f_pangle (w, op->s_az);
	    break;
	case ALT_ID:
	    f_pangle (w, op->s_alt);
	    break;
	case ZEN_ID:
	    f_pangle (w, PI/2 - op->s_alt);
	    break;
	case Z_ID: /* airmass */
	    airmass (op->s_alt, &tmp);
	    f_double (w, "%7.4f", tmp);
	    break;
	case PA_ID: /* parallactic angle */
	    radec2ha (np, op->s_ra, op->s_dec, &tmp);
	    f_double (w, "%7.2f", raddeg(parallacticLHD (lat, tmp, op->s_dec)));
	    break;
	case JD_ID: /* julian date */
	    f_double (w, "%13.5f", mjd+MJD0);
	    break;
	case HJD_ID: /* heliocentric julian date */
	    if (epoch != J2000) {
		double ra = op->s_ra;
		double dec = op->s_dec;
		if (epoch == EOD)
		    ap_as (np, J2000, &ra, &dec);
		else
		    precess (epoch, J2000, &ra, &dec);
		heliocorr (mjd+MJD0, ra, dec, &tmp);
	    } else
		heliocorr (mjd+MJD0, op->s_ra, op->s_dec, &tmp);
	    f_double (w, "%13.5f", mjd+MJD0 - tmp);
	    break;
	case PMRA_ID: {	/* proper motion in RA on sky */
	    /* solsys as/hr, esat deg/min, else mas/yr */
	    int isss = is_ssobj(op);
	    int ises = is_type(op,EARTHSATM);
	    if (isss || ises) {
		Now n2 = *np;
		Obj o2 = *op;
		n2.n_mjd += isss ? 1./24. : 1./14400.;
		obj_cir (&n2, &o2);
		f_double (w, "%8.4g", cos(op->s_dec) * raddeg(o2.s_ra - op->s_ra) *
						(isss ? 3600.0 : 10.0));
	    } else {
		f_double (w, "%8.4g", cos(op->s_dec) * raddeg(op->f_pmRA)*3600000.*365.25);
	    }
	    }
	    break;
	case PMDEC_ID: { /* proper motion in Dec */
	    /* solsys as/hr, esat deg/min, else mas/yr */
	    int isss = is_ssobj(op);
	    int ises = is_type(op,EARTHSATM);
	    if (isss || ises) {
		Now n2 = *np;
		Obj o2 = *op;
		n2.n_mjd += isss ? 1./24. : 1./14400.;
		obj_cir (&n2, &o2);
		f_double (w, "%8.4g", raddeg(o2.s_dec - op->s_dec) *
						(isss ? 3600.0 : 10.0));
	    } else {
		f_double (w, "%8.4g", raddeg(op->f_pmdec)*3600000.*365.25);
	    }
	    }
	    break;
	case SPECT_ID:
	    if (is_type (op,FIXEDM) && op->f_class != 'G' && op->f_spect[0]) {
		char buf[10];
		sprintf (buf, " %2.2s  ", op->f_spect);
		f_string (w, buf);
	    } else {
		char buf[10];
		sprintf (buf, "     ");
		f_string (w, buf);
	    }
	    break;
	case HLONG_ID:
	    if (is_ssobj(op))
		f_pangle (w, op->s_hlong);
	    else {
		if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		    f_string (w, "      ");
		else
		    f_string (w, "         ");
	    }
	    break;
	case HLAT_ID:
	    if (is_ssobj(op))
		f_pangle (w, op->s_hlat);
	    else {
		if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		    f_string (w, "      ");
		else
		    f_string (w, "         ");
	    }
	    break;
	case GLONG_ID: {
	    double glat, glng, asra = op->s_ra, asdec = op->s_dec;
	    double e = epoch == EOD ? mjd : epoch;
	    if (epoch == EOD)
		ap_as (np, mjd, &asra, &asdec);
	    eq_gal (e, asra, asdec, &glat, &glng);
	    f_pangle (w, glng);
	    }
	    break;
	case GLAT_ID: {
	    double glat, glng, asra = op->s_ra, asdec = op->s_dec;
	    double e = epoch == EOD ? mjd : epoch;
	    if (epoch == EOD)
		ap_as (np, mjd, &asra, &asdec);
	    eq_gal (e, asra, asdec, &glat, &glng);
	    f_pangle (w, glat);
	    }
	    break;
	case ECLONG_ID: {
	    double eclat, eclng, asra = op->s_ra, asdec = op->s_dec;
	    double e = epoch == EOD ? mjd : epoch;
	    eq_ecl (e, asra, asdec, &eclat, &eclng);
	    f_pangle (w, eclng);
	    }
	    break;
	case ECLAT_ID: {
	    double eclat, eclng, asra = op->s_ra, asdec = op->s_dec;
	    double e = epoch == EOD ? mjd : epoch;
	    eq_ecl (e, asra, asdec, &eclat, &eclng);
	    f_pangle (w, eclat);
	    }
	    break;
	case EDST_ID:
	    if (is_planet(op, MOON)) {
		tmp = op->s_edist;
		if (pref_get(PREF_UNITS) == PREF_ENGLISH) {
		    /* s_edist is stored in au, want miles */
		    tmp *= MAU*FTPM/5280.0;
		} else {
		    /* s_edist is stored in au, want km */
		    tmp *= MAU/1000.0;
		}
		f_double (w, "%6.0f", tmp);
	    } else if (is_ssobj(op)) {
		/* show distance in au */
		f_double (w, op->s_edist >= 9.99995 ? "%6.3f" : "%6.4f",
								op->s_edist);
	    } else
		f_string (w, "      ");
	    break;
	case ELGHT_ID:
	    if (is_planet(op,MOON)) {
	        double m = op->s_edist*LTAU;	/* seconds */
		if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		    f_double (w, "%5.3f", m);
		else
		    f_double (w, "%8.5f", m);	/* 8.6 is ridiculous */
	    } else if (is_ssobj(op)) {
	        double m = (op->s_edist*LTAU)/3600.0;	/* hours */
		if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		    f_mtime (w, m);	/* hh:mm */
		else
		    f_time (w, m);	/* hh:mm:ss */
	    } else {
		if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		    f_string (w, "     ");
		else
		    f_string (w, "        ");
	    }
	    break;
	case SDST_ID:
	    if (is_ssobj(op) && !is_planet(op, SUN))
		f_double (w, op->s_sdist >= 9.99995 ? "%6.3f" : "%6.4f",
								op->s_sdist);
	    else
		f_string (w, "      ");
	    break;
	case SLGHT_ID:
	    if (is_ssobj(op) && !is_planet(op, SUN)) {
	        double m = (op->s_sdist*LTAU)/3600.0;	/* hours */
		if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		    f_mtime (w, m);	/* hh:mm */
		else
		    f_time (w, m);	/* hh:mm:ss */
	    } else {
		if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		    f_string (w, "     ");
		else
		    f_string (w, "        ");
	    }
	    break;
	case ELONG_ID:
	    if (op->o_type != EARTHSAT)
		f_double (w, "%6.1f", op->s_elong);
	    else
		f_string (w, "      ");
	    break;
	case URANOM_ID:
	    f_string (w, um_atlas(op->s_ra, op->s_dec));
	    break;
	case URAN2K_ID:
	    f_string (w, u2k_atlas(op->s_ra, op->s_dec));
	    break;
	case MILLSA_ID:
	    f_string (w, msa_atlas(op->s_ra, op->s_dec));
	    break;
	case SIZE_ID:
	    if (op->o_type != EARTHSAT) {
		if (op->s_size < 999.5)
		    f_double (w, "%5.1f", op->s_size);
		else
		    f_double (w, "%5.0f", op->s_size);
	    } else
		f_string (w, "     ");
	    break;
	case VMAG_ID:
	    if (op->o_type != EARTHSAT) {
	        double m = get_mag(op);
	        f_double (w, m <= -9.95 ? "%4.0f" : "%4.1f", m);
	    } else
		f_string (w, "    ");
	    break;
	case PHS_ID:
	    if (is_ssobj(op))
		f_double (w, " %3.0f ", op->s_phase);
	    else
		f_string (w, "     ");
	    break;

	case RSTIME_ID:
	    if (rp->rs_flags & RS_ERROR)
		f_string (w, "Error");
	    else if (rp->rs_flags & RS_CIRCUMPOLAR)
		f_string (w, "CirPl");
	    else if (rp->rs_flags & RS_NEVERUP)
		f_string (w, "NvrUp");
	    else if (rp->rs_flags & RS_NORISE)
		f_string (w, "NoRis");
	    else
		dm_showtim (np, w, rp->rs_risetm);	/* 5 chars wide */
	    break;

	case RSAZ_ID:
	    if (rp->rs_flags & RS_ERROR)
		f_string (w, " Error");
	    else if (rp->rs_flags & RS_CIRCUMPOLAR)
		f_string (w, "CirPol");
	    else if (rp->rs_flags & RS_NEVERUP)
		f_string (w, " NvrUp");
	    else if (rp->rs_flags & RS_NORISE)
		f_string (w, "NoRise");
	    else
		f_dm_angle (w, rp->rs_riseaz);	/* 6 chars wide */
	    break;

	case SETTIME_ID:
	    if (rp->rs_flags & RS_ERROR)
		f_string (w, "Error");
	    else if (rp->rs_flags & RS_CIRCUMPOLAR)
		f_string (w, "CirPl");
	    else if (rp->rs_flags & RS_NEVERUP)
		f_string (w, "NvrUp");
	    else if (rp->rs_flags & RS_NOSET)
		f_string (w, "NoSet");
	    else
		dm_showtim (np, w, rp->rs_settm);	/* 5 chars wide */
	    break;

	case SETAZ_ID:
	    if (rp->rs_flags & RS_ERROR)
		f_string (w, " Error");
	    else if (rp->rs_flags & RS_CIRCUMPOLAR)
		f_string (w, "CirPol");
	    else if (rp->rs_flags & RS_NEVERUP)
		f_string (w, " NvrUp");
	    else if (rp->rs_flags & RS_NOSET)
		f_string (w, " NoSet");
	    else
		f_dm_angle (w, rp->rs_setaz);	/* 6 chars wide */
	    break;

	case TRTIME_ID:
	    if (rp->rs_flags & RS_ERROR)
		f_string (w, "Error");
	    else if (rp->rs_flags & RS_NEVERUP)
		f_string (w, "NvrUp");
	    else if (rp->rs_flags & RS_NOTRANS)
		f_string (w, "NoTrn");
	    else
		dm_showtim (np, w, rp->rs_trantm);	/* 5 chars wide */
	    break;

	case TRALT_ID:
	    if (rp->rs_flags & RS_ERROR)
		f_string (w, " Error");
	    else if (rp->rs_flags & RS_NEVERUP)
		f_string (w, " NvrUp");
	    else if (rp->rs_flags & RS_NOTRANS)
		f_string (w, "NoTran");
	    else {
		f_dm_angle (w, rp->rs_tranalt);	/* 6 chars wide */
	    }
	    break;

	case TRAZ_ID:
	    if (rp->rs_flags & RS_ERROR)
		f_string (w, " Error");
	    else if (rp->rs_flags & RS_NEVERUP)
		f_string (w, " NvrUp");
	    else if (rp->rs_flags & RS_NOTRANS)
		f_string (w, "NoTran");
	    else {
		f_dm_angle (w, rp->rs_tranaz);	/* 6 chars wide */
	    }
	    break;

	case HRSUP_ID:
	    dm_rs_hrsup (np, op, w, rp);
	    break;

	case SEP_ID:
	    if (col[c].type != SEP_COL) {
		printf ("Bug: %s: col[%d].type = 0x%x\n", me, c, col[c].type);
		abort();
	    }
	    db_update(sepop);
	    dm_separation (op, sepop, &tmp);
	    f_pangle (w, tmp);
	    break;

	default:
	    printf ("Bug! %s: bogus column id %d\n", me, c);
	    abort();
	}
}

/* setup the limb and centric tag labels according to the current options */
static void
dm_settags()
{
	char str[1024], estr[64];
	Now *np = mm_get_now();

	if (epoch == EOD)
	    (void) strcpy (estr, "EOD");
	else {
	    double y;
	    mjd_year (epoch, &y);
	    (void) sprintf (estr, "%.1f", y);
	}
	(void) sprintf (str, "Equ: %s %s",
		    pref_get(PREF_EQUATORIAL) == PREF_GEO ? "Geo":"Topo", estr);
	f_showit (centric_w, str);

	limb = XmToggleButtonGetState (limb_w) ? LIMB : CENTER;
	f_showit (limbl_w, limb == LIMB ? "Limb" : "Center");
}

/* display the rise/set/transit mjd time t as hours in widget w.
 * convert tm to local time if tzpref && PREF_ZONE == PREF_LOCALTZ.
 */
static void
dm_showtim (np, w, t)
Now *np;
Widget w;
double t;
{
	if (pref_get(PREF_ZONE) == PREF_LOCALTZ)
	    t -= tz/24.0;

	f_mtime (w, mjd_hr(t));			/* 5 chars: "hh:mm" */
}

/* display the total hours this object has been up.
 * N.B. insure string length is always 5 chars wide.
 * N.B. Earth satellites are up from rise to set, period, and either or both
 *   of these may be tomorrow if they are later than the current time because
 *   we scan for earth sat rise/set events up to 24 hours in the future
 *   regardless of whether it crosses over into tomorrow. Also, since we can't
 *   assume a nominal diurnal period who's to say how much it's up if all we
 *   have is a rise time that is later than a set time,
 */
/* ARGSUSED */
static void
dm_rs_hrsup (np, op, w, rp)
Now *np;
Obj *op;
Widget w;
RiseSet *rp;
{
	double r, s, up;

	/* first some special cases */
	if (rp->rs_flags & RS_ERROR) {
	    f_string (w, "Error ");
	    return;
	}
	if (rp->rs_flags & RS_CIRCUMPOLAR) {
	    f_double (w, "%3.0f:00", 24.0); /* f_mtime() changes to 00:00 */
	    return;
	}
	if (rp->rs_flags & RS_NEVERUP) {
	    f_mtime (w, 0.0);
	    return;
	}
	if (rp->rs_flags & (RS_NORISE|RS_NOSET)) {
	    f_string (w, "      ");
	    return;
	}

	r = rp->rs_risetm;
	s = rp->rs_settm;
	up = s - r;
	 
	if (up < 0) {
	    /* we assume diurnal motion except for fast satellites */
	    if (op->o_type == EARTHSAT && op->es_n > FAST_SAT_RPD) {
		f_string (w, "      ");
		return;
	    }
	    up += 1.0;
	}

	f_mtime (w, up*24.0);			/* 5 chars: "hh:mm" */
}

static void
show_constellation (np, op, w)
Now *np;
Obj *op;
Widget w;
{
	char nm[10], *name;
	int id;

        id = cns_pick (op->s_ra, op->s_dec, epoch == EOD ? mjd : epoch);
	name = cns_name (id);
	(void) sprintf (nm, "%.3s", name);
	f_string(w, nm);
}

/* compute the separation between the two sky locations */
void
dm_separation (p, q, sp)
Obj *p, *q;
double *sp;
{
	double csep;

	solve_sphere (p->s_ra - q->s_ra, PI/2 - p->s_dec, sin(q->s_dec),
						    cos(q->s_dec), &csep, NULL);
	*sp = acos(csep);
}

/* create the setup dialog */
static void
ds_create_selection(parent)
Widget parent;
{
	static struct { /* info to streamline creation of control buttons */
	    DSCtrls id;
	    int lpos, rpos;
	    char *name;
	    char *tip;
	} ctlbtns[] = {
	    {APPLY,   0,  4, "Apply",   "Apply new settings and stay up"},
	    {CANCEL,  5,  9, "Close",   "Make no changes; just close"},
	    {CLALL,  10, 14, "None",    "Turn off all selections"},
	    {RESET,  15, 19, "Restore", "Return to active settings"},
	    {HELP,   20, 24, "Help",    "More detailed descriptions"}
	};
	Arg args[20];
	Widget lrc_w, crc_w, stb_w, sep_w, csep_w, ctlf_w;
	Widget w;
	Widget rb_w;
	int n;
	int i;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNallowOverlap, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	XtSetArg (args[n], XmNmarginHeight, 5); n++;
	XtSetArg (args[n], XmNmarginWidth, 5); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	selshell_w = XmCreateFormDialog (parent, "DataSetup", args, n);
	set_xmstring (selshell_w, XmNdialogTitle, "xephem Data Table setup");
	set_something (selshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (selshell_w, XmNhelpCallback, ds_ctl_cb, (XtPointer)HELP);
	sr_reg (XtParent(selshell_w), "XEphem*DataSetup.x", dscategory, 0);
	sr_reg (XtParent(selshell_w), "XEphem*DataSetup.y", dscategory, 0);

	/* make a form for bottom control panel */
	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNfractionBase, 24); n++;
	ctlf_w = XmCreateForm (selshell_w, "DataSelF", args, n);
	XtManageChild (ctlf_w);

	    /* make the control buttons */

	    for (i = 0; i < XtNumber(ctlbtns); i++) {
		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNtopOffset, 5); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomOffset, 5); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNleftPosition, ctlbtns[i].lpos); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNrightPosition, ctlbtns[i].rpos); n++;
		w = XmCreatePushButton (ctlf_w, ctlbtns[i].name, args, n);
		XtAddCallback (w, XmNactivateCallback, ds_ctl_cb,
						    (XtPointer)ctlbtns[i].id);
		wtip (w, ctlbtns[i].tip);
		XtManageChild (w);
	    }

	/* a sep above controls */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, ctlf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	csep_w = XmCreateSeparator (selshell_w, "CS", args, n);
	XtManageChild (csep_w);

	/* misc controls in one RC on left */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, csep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNadjustMargin, False); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNnumColumns, 3); n++;
	lrc_w = XmCreateRowColumn (selshell_w, "DS", args, n);
	XtManageChild (lrc_w);


	    for (i = 0; i < NC; i++) {
		if (col[i].type == MISC_COL) {
		    n = 0;
		    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);n++;
		    w = XmCreateToggleButton(lrc_w, col[i].name,args,n);
		    sr_reg (w, NULL, dscategory, 1);
		    XtManageChild (w);
		    col[i].sw = w;

		    if (col[i].tip)
			wtip (w, col[i].tip);
		}
	    }

	/* sep */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, csep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, lrc_w); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	sep_w = XmCreateSeparator (selshell_w, "S2", args, n);
	XtManageChild (sep_w);

	/* rise/set controls in one center column */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, csep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, sep_w); n++;
	XtSetArg (args[n], XmNadjustMargin, False); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	XtSetArg (args[n], XmNnumColumns, 1); n++;
	crc_w = XmCreateRowColumn (selshell_w, "DS", args, n);
	XtManageChild (crc_w);

	    /* start with displacement */

	    n = 0;
	    XtSetArg (args[n], XmNcolumns, 5); n++;
	    hzn_w = XmCreateTextField (crc_w, "Horizon", args, n);
	    sr_reg (hzn_w, NULL, dscategory, 1);
	    wtip (hzn_w, "Degrees the local horizon is above horizontal");
	    XtManageChild (hzn_w);
	    hznd = dm_uhzndep();

	    /* make the CENTER/LIMB radio box */

	    n = 0;
	    w = XmCreateFrame (crc_w, "DSLimbF", args, n);
	    XtManageChild (w);
	    n = 0;
	    rb_w = XmCreateRadioBox (w, "DSLimbRB", args, n);
	    XtManageChild (rb_w);

		n = 0;
		limb_w = XmCreateToggleButton (rb_w, "Limb", args, n);
		wtip (limb_w, "Compute Rise/Set events with respect to upper limb");
		sr_reg (limb_w, NULL, dscategory, 1);
		XtManageChild (limb_w);

		n = 0;
		XtSetArg(args[n], XmNset, !XmToggleButtonGetState(limb_w)); n++;
		w = XmCreateToggleButton (rb_w, "Center", args, n);
		wtip (w, "Compute Rise/Set events with respect to body center");
		XtManageChild (w);

	    /* now the rise/set buttons */

	    for (i = 0; i < NC; i++) {
		if (col[i].type == RISET_COL) {
		    n = 0;
		    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);n++;
		    w = XmCreateToggleButton(crc_w, col[i].name,args,n);
		    sr_reg (w, NULL, dscategory, 1);
		    XtManageChild (w);
		    col[i].sw = w;

		    if (col[i].tip)
			wtip (w, col[i].tip);
		}
	    }

	/* sep */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, csep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, crc_w); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	sep_w = XmCreateSeparator (selshell_w, "S3", args, n);
	XtManageChild (sep_w);

	/* sep controls built directly on right side */

	/* column control */

	i = NC-1;		/* N.B. assumes SEP_COL is last */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, sep_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	stb_w = XmCreateToggleButton(selshell_w, col[i].name, args, n);
	sr_reg (stb_w, NULL, dscategory, 1);
	XtManageChild (stb_w);
	col[i].sw = stb_w;

	if (col[i].tip)
	    wtip (stb_w, col[i].tip);

	/* make a hidden label resource for saving sep partner */
	sepl_w = XmCreateLabel (selshell_w, "SepPartner", NULL, 0);
	sr_reg (sepl_w, NULL, dscategory, 1);

	/* favorites scrolled list */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, stb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, csep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, sep_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	fsl_w = XmCreateScrolledList (selshell_w, "SSL", args, n);
	XtManageChild (fsl_w);
}

/* put the favs list into the scrolled list.
 * select one that matches sepl_w, if any
 */
static void
ds_buildfsl()
{
	XmString sepstr;
	int sepfound;
	int i;

	/* a little smoother if manage offline */
	set_something (fsl_w, XmNmappedWhenManaged, 0);
	XtUnmanageChild (fsl_w);

	get_something (sepl_w, XmNlabelString, (XtArgVal)&sepstr);
	XmListDeleteAllItems (fsl_w);
	sepfound = 0;
	for (i = 0; i < nfavs; i++) {
	    XmString str = XmStringCreateSimple (favs[i]->o_name);
	    XmListAddItemUnselected (fsl_w, str, 0);
	    if (XmStringCompare (str, sepstr)) {
		XmListSelectItem (fsl_w, str, False);
		sepfound = 1;
	    }
	    XmStringFree (str);
	}
	XmStringFree (sepstr);

	/* turn off sep column if partner no longer in list */
	if (!sepfound) {
	    XmToggleButtonSetState (col[SEP_ID].sw, False, False);
	    col[SEP_ID].on = 0;
	    XtUnmanageChild (col[SEP_ID].rcw);
	}

	set_something (fsl_w, XmNmappedWhenManaged, (XtArgVal)True);
	XtManageChild (fsl_w);
}

/* set up a Data setup col menu based on what is currently on and defined.
 */
static void
ds_setup_col_selections()
{
	int i;

	for (i = 0; i < NC; i++)
	    XmToggleButtonSetState (col[i].sw, col[i].on, False);

	XmToggleButtonSetState (limb_w, limb == LIMB, True);

	dm_resetuhzndep();
}


/* change the Data table according to what is now defined and set up in the
 * Setup menu.
 * N.B. can be called before we are managed.
 * return 0 if all ok, else -1.
 */
static int
ds_apply_selections()
{
	int n_riset;
	int n_sep;
	int i;

	watch_cursor(1);

	/* if activating sep, must selected a partner */
	if (XmToggleButtonGetState(col[SEP_ID].sw)) {
	    int *selpos, nsel, sel;
	    if (XmListGetSelectedPos (fsl_w, &selpos, &nsel) == False) {
		xe_msg (1, "Please select a Separation partner");
		watch_cursor(0);
		return (-1);
	    }
	    sel = selpos[0]-1;		/* List is 1-based */
	    if (sel < 0 || sel >= nfavs) {
		printf ("Bug! DS seli = %d but nfavs = %d\n", sel, nfavs);
		abort();
	    }
	    XtFree ((char*)selpos);
	    sepop = favs[sel];
	    set_xmstring (sepl_w, XmNlabelString, sepop->o_name);
	}

	/* scan each column choice */
	n_sep = n_riset = 0;
	for (i = 0; i < NC; i++) {
	    col[i].on = XmToggleButtonGetState(col[i].sw);
	    if (col[i].on) {
		XtManageChild (col[i].rcw);
		if (col[i].type == RISET_COL)
		    n_riset++;
		if (col[i].type == SEP_COL)
		    n_sep++;
	    } else
		XtUnmanageChild (col[i].rcw);
	}

	/* install user's horizon offset */
	hznd = dm_uhzndep();

	/* set state info labels */
	dm_settags();
	if (n_riset)
	    XtManageChild (XtParent(limbl_w));
	else
	    XtUnmanageChild (XtParent(limbl_w));

	if (col[RA_ID].on || col[DEC_ID].on || n_sep > 0)
	    XtManageChild (XtParent(centric_w));
	else
	    XtUnmanageChild (XtParent(centric_w));

	watch_cursor(0);

	return (0);
}

/* callback from any of the Data setup control panel buttons.
 * which is in client.
 */
static void
/* ARGSUSED */
ds_ctl_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	DSCtrls id = (DSCtrls) client;
	int i;

	switch (id) {
	case OK:
	    if (ds_apply_selections() == 0) {
		dm_update (mm_get_now(), 1);
		XtUnmanageChild (selshell_w);
	    }
	    break;
	case APPLY:
	    if (ds_apply_selections() == 0) {
		dm_update (mm_get_now(), 1);
		ng_update (mm_get_now(), 1);    /* cares about hzn offset */
	    }
	    break;
	case CANCEL:
	    XtUnmanageChild (selshell_w);
	    break;
	case CLALL:
	    for (i = 0; i < NC; i++)
		XmToggleButtonSetState (col[i].sw, False, False);
	    break;
	case RESET:
	    ds_setup_col_selections();
	    break;
	case HELP:
	    ds_help();
	    break;
	}
}

/* called from the Data selection table Help button
 */
static void
ds_help ()
{
	static char *msg[] = {
"This table lets you configure the rows and columns of the data table."
};

	hlp_dialog ("DataSelection_Table", msg, XtNumber(msg));
}

/* create the list filename prompt */
static void
dm_create_flist_w()
{
	Arg args[20];
	int n;

	n = 0;
        XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg(args[n], XmNmarginWidth, 20);  n++;
	XtSetArg(args[n], XmNmarginHeight, 20);  n++;
	flist_w = XmCreatePromptDialog (datashell_w, "DataSel", args,n);
	set_something (flist_w, XmNcolormap, (XtArgVal)xe_cm);
	set_xmstring (flist_w, XmNdialogTitle, "xephem Data list");
	set_xmstring (flist_w, XmNselectionLabelString, "File name:");
	defaultTextFN (XmSelectionBoxGetChild(flist_w, XmDIALOG_TEXT), 1,
							"datatbl.txt", NULL);
	XtUnmanageChild (XmSelectionBoxGetChild(flist_w, XmDIALOG_HELP_BUTTON));
	XtAddCallback (flist_w, XmNokCallback, dm_flistok_cb, NULL);
	XtAddCallback (flist_w, XmNmapCallback, prompt_map_cb, NULL);
}

/* called when the Ok button is hit in the file list prompt */
/* ARGSUSED */
static void
dm_flistok_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024];
	char *name;

	get_xmstring(w, XmNtextString, &name);

	if (strlen(name) == 0) {
	    xe_msg (1, "Please enter a file name.");
	    XtFree (name);
	    return;
	}

	if (existd (name,NULL) == 0 && confirm()) {
	    (void) sprintf (buf, "%s exists:\nAppend or Overwrite?", name);
	    query (flist_w, buf, "Append", "Overwrite", "Cancel",
				flistok_append_cb, flistok_overwrite_cb, NULL);
	} else
	    flistok_overwrite_cb();

	XtFree (name);
}

/* called when we want to append to a flist file */
static void
flistok_append_cb ()
{
	char *name;

	get_xmstring (flist_w, XmNtextString, &name);
	make_flist (name, "a");
	XtFree (name);
}

/* called when we want to ceate a new flist file */
static void
flistok_overwrite_cb ()
{
	char *name;

	get_xmstring (flist_w, XmNtextString, &name);
	make_flist (name, "w");
	XtFree (name);
}

/* open the named flist file "a" or "w" and fill it in. */
static void
make_flist (name, how)
char *name;
char *how;
{
	FILE *fp = fopend (name, NULL, how);

	if (fp) {
	    dm_list_tofile (fp);
	    (void) fclose (fp);
	}
}

/* callback from file List control button. */
/* ARGSUSED */
static void
dm_flist_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (!flist_w)
	    dm_create_flist_w();

	if (!XtIsManaged(flist_w))
	    XtManageChild (flist_w);
}

/* callback from x selection control button. */
/* ARGSUSED */
static void
dm_xsel_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *buf = XtMalloc ((nfavs+3)*(NC*20));	/* plenty big :-) */
	dm_list_get (buf);
	XSetSelectionOwner (XtD, XA_PRIMARY, None, CurrentTime);
	XStoreBytes (XtD, buf, strlen(buf));
	XtFree (buf);
}

/* write the current data table to the given file */
static void
dm_list_tofile(fp)
FILE *fp;
{
	char *buf = XtMalloc ((nfavs+3)*(NC*20));	/* plenty big :-) */
	dm_list_get (buf);
	fprintf (fp, "%s", buf);
	XtFree (buf);
}

/* fetch the current table into the given buffer.
 */
static void
dm_list_get (buf)
char buf[];
{
	Now *np = mm_get_now();
	int maxch[NC];
	int maxrh;
	char *str;
	char sbuf[32];
	int r, c;
	int bufl, l;

	/* init as legal string */
	buf[0] = '\0';
	bufl = 0;

	/* scan all the row headers to find longest */
	maxrh = 0;
	for (r = 0; r < nfavs; r++) {
	    l = strlen (favs[r]->o_name);
	    if (l > maxrh)
		maxrh = l;
	}

	/* scan all col headers and first row entry to find col widths.
	 * N.B. this assumes all data rows in a given column are the same
	 * length.
	 */
	for (c = 0; c < NC; c++) {
	    if (!col[c].on)
		continue;
	    get_xmstring (tblpb[0][c], XmNlabelString, &str);
	    maxch[c] = strlen (str);
	    XtFree (str);
	    l = strlen (col[c].name);
	    if (l > maxch[c])
		maxch[c] = l;
	}

	/* print the column headers */
	bufl += sprintf (buf+bufl, "%*s", maxrh, "");
	for (c = 0; c < NC; c++) {
	    if (!col[c].on)
		continue;
	    bufl += sprintf (buf+bufl, " %-*s", maxch[c], col[c].name);
	}
	bufl += sprintf (buf+bufl, "\n");

	/* now print the table */
	for (r = 0; r < nfavs; r++) {
	    bufl += sprintf (buf+bufl, "%-*s", maxrh, favs[r]->o_name);
	    for (c = 0; c < NC; c++) {
		if (!col[c].on)
		    continue;
		get_xmstring (tblpb[r][c], XmNlabelString, &str);
		bufl += sprintf (buf+bufl, " %-*s", maxch[c], str);
		XtFree (str);
	    }
	    bufl += sprintf (buf+bufl, "\n");
	}
	bufl += sprintf (buf+bufl, "\n");

	/* print the footer */
	if (XtIsManaged(XtParent(limbl_w))) {
	    get_xmstring (limbl_w, XmNlabelString, &str);
	    bufl += sprintf (buf+bufl, "%s ", str);
	    XtFree (str);
	}
	if (XtIsManaged(XtParent(centric_w))) {
	    get_xmstring (centric_w, XmNlabelString, &str);
	    bufl += sprintf (buf+bufl, "%s ", str);
	    XtFree (str);
	}
	get_xmstring (dt_w, XmNlabelString, &str);
	bufl += sprintf (buf+bufl, "%s\n", str);
	XtFree (str);

	/* add more details */
	if ((str = mm_getsite()) != NULL)
	    bufl += sprintf (buf+bufl, "Site: %s\n", str);
	bufl += sprintf (buf+bufl, "\n");
	fs_sexa (sbuf, raddeg(lat), 4, 3600);
	bufl += sprintf (buf+bufl, " Lat: %s\n", sbuf);
	fs_sexa (sbuf, -raddeg(lng), 4, 3600);
	bufl += sprintf (buf+bufl, "Long: %s\n", sbuf);
	bufl += sprintf (buf+bufl, "Elev: %gm\n", elev*ERAD);
	bufl += sprintf (buf+bufl, "Temp: %gC\n", temp);
	bufl += sprintf (buf+bufl, "Pres: %gmB\n", pressure);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: datamenu.c,v $ $Date: 2013/03/02 02:57:08 $ $Revision: 1.61 $ $Name:  $"};

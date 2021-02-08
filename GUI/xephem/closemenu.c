/* code to handle the close objects menu.
 *
 * the basic idea is to sort all objects into bands of dec (or alt).
 * then scan for pairs of close objects within one and adjacent bands.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/SelectioB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/List.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>

#include "xephem.h"


#define	MINSEP	degrad(.1)	/* smallest sep we allow.
				 * this is not a hard limit, just a sanity
				 * check. as sep shrinks, we make more and
				 * more tiny little malloced bands.
				 */

/* record of a band.
 * opp is malloced -- can hold nmem, nuse of which are in use.
 */
typedef struct {
    int nuse;		/* number of Obj * actually in use in opp now */
    int nmem;		/* number of Obj * there is room for in opp now */
    Obj **opp;		/* malloced list of Object pointers, or NULL */
} Band;

/* record of a pair.
 * used to collect pairs as the bands are scanned.
 */
typedef struct {
    Obj *op1, *op2;	/* two objects */
    float sep;		/* separation, rads (float to save memory) */
} Pair;

static void c_create_shell (void);
static void c_help_cb (Widget w, XtPointer client, XtPointer call);
static void c_helpon_cb (Widget w, XtPointer client, XtPointer call);
static void c_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void c_close_cb (Widget w, XtPointer client, XtPointer call);
static void c_flist_cb (Widget w, XtPointer client, XtPointer call);
static void c_actlist_cb (Widget w, XtPointer client, XtPointer call);
static void c_skypoint_cb (Widget w, XtPointer client, XtPointer call);
static void c_go_cb (Widget w, XtPointer client, XtPointer call);
static int sky_point (void);
static void do_search (void);
static int get_options (double *sp, double *mp, int *wp, int *op);
static void free_band (Band bp[], int nb);
static int init_bands (double sep, double mag, Band **bpp);
static int add_obj (Band *bp, Obj *op);
static int find_close (Band bp[], int nb, double maxsep, int wander, int
    ownmoons, Pair **ppp);
static int add_pair (Obj *op1, Obj *op2, double sep, Pair **ppp,
    int *mp, int *np);
static int add_pair (Obj *op1, Obj *op2, double sep, Pair **ppp,
    int *mp, int *np);
static int dspl_pairs (Pair p[], int np, double sep);

static void c_create_flist_w (void);
static void c_flistok_cb (Widget w, XtPointer client, XtPointer call);
static void flistok_append_cb (void);
static void flistok_overwrite_cb (void);
static void make_flist (char *name, char *how);
static void write_flist (FILE *fp);


static Widget cshell_w;		/* main shell */
static Widget sep_w;		/* text widget to hold max sep */
static Widget mag_w;		/* text widget to hold mag limit */
static Widget timestmp_w;	/* label for time stamp */
static Widget count_w;		/* label for count of objects in list */
static Widget list_w;		/* the list of close objects */
static Widget flist_w;		/* the file list dialog */
static Widget wander_w;		/* toggle to omit pairs of fixed objs*/
static Widget ownmoons_w;	/* toggle to omit a planet's own moons */
static Widget autoup_w;		/* toggle to automatically redo on updates */
static Widget autols_w;		/* toggle to automatically list on updates */

static char closecategory[] = "Close pairs"; 	/* Save category */

/* bring up Close pairs shell, creating if first time */
void
c_manage()
{
	if (!cshell_w) {
	    c_create_shell();
	    c_create_flist_w ();
	}
	XtPopup (cshell_w, XtGrabNone);
	set_something (cshell_w, XmNiconic, (XtArgVal)False);
}

/* called when time has advanced.
 * compute a new Close list if desired.
 */
/* ARGSUSED */
void
c_update (np, force)
Now *np;
int force;
{
	if (!isUp(cshell_w) || !XmToggleButtonGetState (autoup_w))
	    return;

	do_search();
}


/* called to put up or remove the watch cursor.  */
void
c_cursor (c)
Cursor c;
{
	Window win;

	if (cshell_w && (win = XtWindow(cshell_w)) != 0) {
	    Display *dsp = XtDisplay(cshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create a form in a shell to allow user to work with the list. */
static void
c_create_shell ()
{
	typedef struct {
	    char *label;	/* what goes on the help label */
	    char *key;		/* string to call hlp_dialog() */
	} HelpOn;
	static HelpOn helpon[] = {
	    {"Intro...",	"CloseObjects"},
	    {"on Control...",	"CloseObjects_control"},
	    {"on Options...",	"CloseObjects_options"},
	    {"Misc...",		"CloseObjects_misc"},
	};
	Widget mb_w, pd_w, cb_w;
	Widget s_w;
	XmString str;
	Widget cform_w;
	Widget w;
	Arg args[20];
	int n;
	int i;
	
	/* create outter shell and form */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Close Pairs"); n++;
	XtSetArg (args[n], XmNiconName, "Close"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	cshell_w = XtCreatePopupShell ("CloseObjs", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (cshell_w);
	set_something (cshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (cshell_w, XmNpopdownCallback, c_popdown_cb, NULL);
	sr_reg (cshell_w, "XEphem*CloseObjs.width", closecategory, 0);
	sr_reg (cshell_w, "XEphem*CloseObjs.height", closecategory, 0);
	sr_reg (cshell_w, "XEphem*CloseObjs.x", closecategory, 0);
	sr_reg (cshell_w, "XEphem*CloseObjs.y", closecategory, 0);

	n = 0;
	XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	cform_w = XmCreateForm(cshell_w, "CloseSh", args, n);
	XtAddCallback (cform_w, XmNhelpCallback, c_help_cb, NULL);
	XtManageChild (cform_w);

	/* create the menu bar across the top */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (cform_w, "MB", args, n);
	XtManageChild (mb_w);

	/* make the Control pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "ControlPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'C'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "ControlCB", args, n);
	    set_xmstring (cb_w, XmNlabelString, "Control");
	    XtManageChild (cb_w);

	    /* make the Go button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Run", args, n);
	    XtAddCallback (w, XmNactivateCallback, c_go_cb, 0);
	    wtip (w, "Perform one scan for close pairs");
	    XtManageChild (w);

	    /* make the sky point button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "SkyPt", args, n);
	    XtAddCallback (w, XmNactivateCallback, c_skypoint_cb, 0);
	    set_xmstring(w, XmNlabelString, "Sky point");
	    wtip (w, "Center the Sky View on the pair selected in the list");
	    XtManageChild (w);

	    /* make the file list button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "File", args, n);
	    XtAddCallback (w, XmNactivateCallback, c_flist_cb, 0);
	    set_xmstring(w, XmNlabelString, "List to file...");
	    wtip (w, "Save the current list to a file");
	    XtManageChild (w);

	    /* add a separator */

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep3", args, n);
	    XtManageChild (w);

	    /* add the close button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, c_close_cb, 0);
	    wtip (w, "Close this window");
	    XtManageChild (w);

	/* make the options pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "OptionsPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'O'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "OptionsCB", args, n);
	    set_xmstring (cb_w, XmNlabelString, "Options");
	    XtManageChild (cb_w);

	    /* make the auto update button */

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    autoup_w = XmCreateToggleButton (pd_w, "AutoUp", args, n);
	    set_xmstring(autoup_w, XmNlabelString, "Auto run");
	    wtip (autoup_w, "Whether to automatically perform a new scan after each Main menu Update.");
	    XtManageChild (autoup_w);
	    sr_reg (autoup_w, NULL, closecategory, 1);
	    
	    /* make the auto list button */

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    autols_w = XmCreateToggleButton (pd_w, "AutoList", args, n);
	    set_xmstring(autols_w, XmNlabelString, "Auto list");
	    wtip (autols_w,
	     "Whether to automatically append each new scan to the list file.");
	    XtManageChild (autols_w);
	    sr_reg (autols_w, NULL, closecategory, 1);

	    /* make the button to omit fixed pairs */

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    wander_w = XmCreateToggleButton (pd_w, "OmitFixedPairs", args, n);
	    set_xmstring(wander_w, XmNlabelString, "Omit fixed pairs");
	    wtip (wander_w, "Whether pairs of Fixed objects are not shown.");
	    XtManageChild (wander_w);
	    sr_reg (wander_w, NULL, closecategory, 1);

	    /* make the button to omit planets and their own satellites */

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    ownmoons_w = XmCreateToggleButton (pd_w, "OmitOwnMoons", args, n);
	    set_xmstring(ownmoons_w, XmNlabelString, "Omit planet's own moons");
	    wtip (ownmoons_w,
		"Whether to skip pairs that are a planet and it's own moons.");
	    XtManageChild (ownmoons_w);
	    sr_reg (ownmoons_w, NULL, closecategory, 1);

	/* make the help pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "HelpPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'H'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "HelpCB", args, n);
	    set_xmstring (cb_w, XmNlabelString, "Help");
	    XtManageChild (cb_w);
	    set_something (mb_w, XmNmenuHelpWidget, (XtArgVal)cb_w);

	    for (i = 0; i < XtNumber(helpon); i++) {
		HelpOn *hp = &helpon[i];

		str = XmStringCreate (hp->label, XmSTRING_DEFAULT_CHARSET);
		n = 0;
		XtSetArg (args[n], XmNlabelString, str); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		w = XmCreatePushButton (pd_w, "Help", args, n);
		XtAddCallback (w, XmNactivateCallback, c_helpon_cb,
							(XtPointer)(hp->key));
		XtManageChild (w);
		XmStringFree(str);
	    }

	/* make the separation label and entry field */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 20); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 47); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	w = XmCreateLabel (cform_w, "SepLabel", args, n);
	set_xmstring (w, XmNlabelString, "Max Sep (degs): ");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 80); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 53); n++;
	XtSetArg (args[n], XmNcolumns, 10); n++;
	sep_w = XmCreateTextField (cform_w, "Sep", args, n);
	wtip (sep_w, "Enter desired max separation to list");
	XtManageChild (sep_w);

	/* make the limiting mag label and entry field */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 20); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 47); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	w = XmCreateLabel (cform_w, "MagLabel", args, n);
	set_xmstring (w, XmNlabelString, "Mag limit: ");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 80); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 53); n++;
	XtSetArg (args[n], XmNcolumns, 10); n++;
	mag_w = XmCreateTextField (cform_w, "Mag", args, n);
	wtip (mag_w, "Enter desired limiting magnitude");
	XtManageChild (mag_w);

	/* make a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mag_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNseparatorType, XmDOUBLE_LINE); n++;
	s_w = XmCreateSeparator (cform_w, "Sep1", args, n);
	XtManageChild (s_w);

	/* make the list count label */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, s_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	count_w = XmCreateLabel (cform_w, "Count", args, n);
	set_xmstring (count_w, XmNlabelString, "0 Pairs");
	XtManageChild (count_w);

	/* make the time stamp label at the bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	timestmp_w = XmCreateLabel (cform_w, "TSL", args, n);
	set_xmstring (timestmp_w, XmNlabelString, " -- ");
	wtip (timestmp_w, "Date and Time for which data are computed");
	XtManageChild (timestmp_w);

	/* make the list between the count and the time */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, count_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, timestmp_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNselectionPolicy, XmBROWSE_SELECT); n++;
	XtSetArg (args[n], XmNlistSizePolicy, XmCONSTANT); n++;
	list_w = XmCreateScrolledList (cform_w, "List", args, n);
	XtAddCallback (list_w, XmNdefaultActionCallback, c_actlist_cb, NULL);
	wtip (list_w, "The list of close pairs");
	XtManageChild (list_w);
}

/* called when the general help key is pressed */
/* ARGSUSED */
static void
c_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"Select desired max separation and viewpoint, then Control->Start to find all",
"pairs of objects separated by less than the given angular distance.",
};
	
	hlp_dialog ("CloseObjects", msg, XtNumber(msg));
}

/* called when any of the individual help entries are selected.
 * client contains the string for hlp_dialog().
 */
/* ARGSUSED */
static void
c_helpon_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hlp_dialog ((char *)client, NULL, 0);
}

/* callback from file List control button. */
/* ARGSUSED */
static void
c_flist_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtManageChild (flist_w);
}

/* called when our toplevel shell is popped down */
/* ARGSUSED */
static void
c_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XtIsManaged(flist_w))
	    XtUnmanageChild (flist_w);
}

/* called when the Close pushbutton is activated */
/* ARGSUSED */
static void
c_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* let the popdown do the rest of the work */
	XtPopdown (cshell_w);
}

/* called when a list item is double-clicked */
/* ARGSUSED */
static void
c_actlist_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	(void) sky_point();
}

/* called when the Sky Point pushbutton is activated */
/* ARGSUSED */
static void
c_skypoint_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (sky_point() < 0)
	    xe_msg (1, "First select a line from the list.");
}

/* called when the Perform Search pushbutton is activated */
/* ARGSUSED */
static void
c_go_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	do_search();
}

/* tell sky view to point to the first object named in the current selection.
 * return 0 if ok, else -1.
 */
static int
sky_point()
{
	String sel;
	char objname[MAXNM];
	XmStringTable si;
	int sic;
	DBScan dbs;
	Obj *op;
	int i;

	/* get the current select item, if any */
	get_something (list_w, XmNselectedItemCount, (XtArgVal)&sic);
	if (sic < 1)
	    return (-1);
	get_something (list_w, XmNselectedItems, (XtArgVal)&si); /* don't free*/
	XmStringGetLtoR (si[0], XmSTRING_DEFAULT_CHARSET, &sel); /* free */

	/* extract the name of the first object -- see dspl_pairs's formating */
	(void) strncpy (objname, sel+7, sizeof(objname));
	XtFree (sel);
	objname[sizeof(objname)-1] = '\0';
	for (i = MAXNM-2; i >= 0; --i)
	    if (objname[i] == ' ')
		objname[i] = '\0';
	    else
		break;

	/* scan the db for the object */
	for (db_scaninit (&dbs, ALLM, NULL, 0); (op = db_scan(&dbs)) != NULL; )
	    if (strcmp (op->o_name, objname) == 0)
		break;

	/* point sky if found */
	if (op)
	    sv_point (op);
	else
	    xe_msg (1, "Can not find `%s' in memory!", objname);

	return (0);
}

/* given two pointers to Obj *, return how they sort by dec in qsort-style.
 */
static int
racmp_f (const void *v1, const void *v2)
{
	Obj *op1 = *((Obj **)v1);
	Obj *op2 = *((Obj **)v2);
	double ra1 = op1->s_ra;
	double ra2 = op2->s_ra;

	if (ra1 < ra2)
	    return (-1);
	if (ra1 > ra2)
	    return (1);
	return (0);
}

static void
do_search()
{
	int als = XmToggleButtonGetState(autols_w);
	double sep;	/* desired max separation, rads */
	double mag;	/* limiting magnitude */
	int wander;	/* 1 to limit list to at least one wanderer */
	int ownmoons;	/* 1 to omit planet's own moons */
	Band *bp = NULL;/* malloced array of nb bands */
	int nb;		/* number of dec (or alt) bands */
	Pair *pp = NULL;/* malloced list of close pairs */
	int np;		/* number of pairs */

	/* retrieve user option settings */
	if (get_options (&sep, &mag, &wander, &ownmoons) < 0)
	    return;

	/* ok, let's get to the real work */
	watch_cursor (1);

	/* compute band size, deal and sort objects into their own and adjacent
	 * bands.
	 */
	nb = init_bands (sep, mag, &bp);
	if (nb < 0) {
	    watch_cursor (0);
	    return;
	}

	/* look through bands for all pairs closer than sep.
	 * form sorted lists of them in malloced pp array.
	 */
	np = find_close (bp, nb, sep, wander, ownmoons, &pp);

	/* finished with the bands now */
	free_band (bp, nb);

	/* report what we found */
	if (np > 0)
	    (void) dspl_pairs (pp, np, sep);
	else if (!als)
	    xe_msg (1, "No such pairs found.");

	/* add to file if on */
	if (als)
	    flistok_append_cb ();

	/* finished */
	if (pp)
	    free ((char *)pp);
	watch_cursor (0);
}

/* retrieve the desired separation and other user option settings.
 * return 0 if ok, else -1
 */
static int
get_options (sp, mp, wp, op)
double *sp;	/* max separation, rads */
double *mp;	/* limiting magnitude */
int *wp;	/* wanderer flag */
int *op;	/* ownmoons flag */
{
	char *str;

	str = XmTextFieldGetString (sep_w);
	*sp = degrad(atod(str));
	XtFree (str);
	if (*sp < MINSEP) {
	    xe_msg (1, "Please specify a max separation >= %g degrees.",
	    							raddeg(MINSEP));
	    return (-1);
	}

	str = XmTextFieldGetString (mag_w);
	*mp = atod(str);
	XtFree (str);

	*wp = XmToggleButtonGetState (wander_w);

	*op = XmToggleButtonGetState (ownmoons_w);

	return (0);
}

/* free a list of nb Bands, including their opp.
 * allow for NULL pointers at any step.
 */
static void
free_band (bp, nb)
Band bp[];
int nb;
{
	Band *bpsav = bp;

	if (!bp)
	    return;

	for (bp += nb; --bp >= bpsav; )
	    if (bp->opp)
		free ((char *)bp->opp);

	free ((char *)bpsav);
}

/* given separation and other factors, build up the array of bands:
 *   find the number of bands and malloc array of bands at *bpp;
 *   deal each qualified object into its own and adjacent-up band;
 *   sort each band according to increasing ra (or az).
 * if all ok return number of bands created at *bpp, else -1.
 * N.B. caller must free_band(*bpp) if we return >= 0.
 */
static int
init_bands (sep, mag, bpp)
double sep;
double mag;
Band **bpp;
{
#define	BAND(x)		((int)floor(((x)+PI/2)/bandsiz))
	int topo = pref_get (PREF_EQUATORIAL) == PREF_TOPO;
	double bandsiz;
	Band *bp;
	DBScan dbs;
	int nobj;
	int nb;
	Obj *op;
	int n, i;

	/* compute number of bands and band size */
	nb = (int)ceil(PI/sep);
	if (nb < 1) {
	    xe_msg (1, "Please decrease max separation to < 180 degrees.");
	    return (-1);
	}
	bandsiz = PI/nb;

	/* malloc list of zero'd Bands */
	bp = (Band *) calloc (nb, sizeof(Band));
	if (!bp) {
	    xe_msg (1,
	    	"Could not malloc %d bands -- try increasing max separation.",
									nb);
	    return(-1);
	}

	/* scan db and fill in bands.
	 * each object goes in its own band and the one adjacent.
	 * we do not include undefined user objects nor those too dim or low.
	 * we also check for entries with bogus bands but should never be any.
	 */
	nobj = db_n();
	n = 0;
	for (db_scaninit(&dbs, ALLM, NULL, 0); (op = db_scan(&dbs)) != NULL; ) {
	    int b;

	    pm_set (33 * n++ / nobj);	/* about 0-33% of time here */

	    /* update object */
	    db_update (op);
	    if (get_mag(op) > mag)
		continue;
	    if (topo && op->s_alt < 0.0)
		continue;

	    /* find which band op fits into */
	    b = BAND(op->s_dec);
	    if (b < 0 || b > nb) {
		xe_msg (0, "%s is out of band: %d.", op->o_name, b);
		continue;
	    }

	    /* add op to its band */
	    if (add_obj (&bp[b], op) < 0) {
		free_band (bp, nb);
		return (-1);
	    }

	    /* unless we are at the top (N pole) add op to next band up too */
	    if (b < nb-1 && add_obj (&bp[b+1], op) < 0) {
		free_band (bp, nb);
		return (-1);
	    }
	}

	/* sort each band by ra (az)
	 */
	for (i = 0; i < nb; i++)
	    if (bp[i].nuse > 0)
		qsort ((void *)bp[i].opp, bp[i].nuse, sizeof (Obj *), racmp_f);

	/* report answers and return */
	*bpp = bp;
	return (nb);
}

/* add one more op to Band bp, growing opp as necessary.
 * return 0 if ok, else xe_msg() and -1.
 */
static int
add_obj (bp, op)
Band *bp;
Obj *op;
{
#define	BCHUNKS	32	/* grow the Obj * list at opp this much at a time */

	/* init the list if this is the first time. */
	if (!bp->opp) {
	    char *newmem;

	    newmem = malloc (BCHUNKS * sizeof(Obj *));
	    if (!newmem) {
		xe_msg (1, "Can not malloc temp Objects.");
		return (-1);
	    }

	    bp->opp = (Obj **) newmem;
	    bp->nmem = BCHUNKS;
	    bp->nuse = 0;
	}

	/* grow the list if necessary */
	if (bp->nuse == bp->nmem) {
	    char *newmem;

	    newmem = realloc((char *)bp->opp, (bp->nmem+BCHUNKS)*sizeof(Obj *));
	    if (!newmem) {
		xe_msg (1, "Can not realloc more temp Objects.");
		return (-1);
	    }

	    bp->opp = (Obj **) newmem;
	    bp->nmem += BCHUNKS;
	}

	bp->opp[bp->nuse++] = op;

	return (0);
}

/* malloc and fill *ppp with the close pairs in the sorted bands.
 * return count of new items in *ppp if ok else -1.
 * N.B. caller must free *ppp if it is != NULL no matter what we return.
 */
static int
find_close (bp, nb, maxsep, wander, ownmoons, ppp)
Band bp[];	/* list of bands to scan */
int nb;		/* number of bands */
double maxsep;	/* max sep we are looking for, rads */
int wander;	/* 1 if limit to pairs with at least one wanderer (ss object) */
int ownmoons;	/* 1 if limit planets with their own moons */
Pair **ppp;	/* return list of pairs we malloc */
{
	double stopsep = maxsep*2.3; /* stop scan at maxsep*sqrt(1**2 + 2**2) */
	int m = 0;		/* n pairs there are room for in *ppp */
	int n = 0;		/* n pairs in use in *ppp */
	int b;


	/* be sure everyone knows nothing has been malloced yet */
	*ppp = NULL;

	/* scan each band */
	for (b = 0; b < nb; b++, bp++) {
	    int nuse = bp->nuse;
	    int i;

	    pm_set (33 + 33 * b / nb);	/* about 33-66% more of time here */

	    /* compare against each object in this band */
	    for (i = 0; i < nuse; i++) {
		Obj *opi = bp->opp[i];
		int j;

		/* set up for finding sep -- see solve_sphere() -- i is c */
		double cc = sin(opi->s_dec);
		double sc = cos(opi->s_dec);

		/* scan forward for close objects in the same band.
		 * wrap, but not so far as to reach self again.
		 * since bands are sorted we can cut off when find one too far.
		 * but this _can_ lead to dups for very short band lists.
		 */
		for (j = (i+1)%nuse; j != i; j = (j+1)%nuse) {
		    Obj *opj = bp->opp[j];
		    double cb, sb;
		    double A, cA;
		    double sep, csep;

		    /* omit if both are fixed and want at least one wanderer */
		    if (wander && is_type(opi,FIXEDM) && is_type(opj,FIXEDM))
			continue;

		    /* omit if both are in the same planet system */
		    if (ownmoons && is_type(opi,PLANETM) && is_type(opj,PLANETM)
					&& opi->pl_code == opj->pl_code)
			continue;

		    /* find the exact separation */
		    cb = sin(opj->s_dec); /* j is b */
		    sb = cos(opj->s_dec);
		    A = opi->s_ra - opj->s_ra;
		    cA = cos(A);
		    csep = cb*cc + sb*sc*cA;
		    if (csep >  1.0) csep =  1.0;	/* just paranoid */
		    if (csep < -1.0) csep = -1.0;
		    sep = acos(csep);

		    /* stop when we're so far from opi there can be no more */
		    if (sep > stopsep)
			break;

		    if (sep <= maxsep)
			if (add_pair (opi, opj, sep, ppp, &m, &n) < 0)
			    return (-1);
		}
	    }
	}

	return (n);
}

/* add the two objects to the list of pairs.
 * we malloc/realloc as needed to grow *ppp, updating {m,n}p.
 * put the smaller of op1/2 in slot op1 for later dup checking.
 * return 0 if ok, else -1.
 * N.B. the caller must free *ppp if it is not NULL no matter what we return.
 */
static int
add_pair (op1, op2, sep, ppp, mp, np)
Obj *op1, *op2;	/* the two objects to report */
double sep;	/* separation, rads */
Pair **ppp;	/* pointer to malloced list of lines */
int *mp;	/* pointer to number of Pairs already in *ppp */
int *np;	/* pointer to number of *ppp actually in use */
{
#define	PCHUNKS	32	/* grow the report list this many at a time */
	Pair *newp;

	/* init the list if this is the first time. */
	if (!*ppp) {
	    char *newmem;

	    newmem = malloc (PCHUNKS * sizeof(Pair));
	    if (!newmem) {
		xe_msg (1, "Can not malloc any Pairs.");
		return (-1);
	    }

	    *ppp = (Pair *) newmem;
	    *mp = PCHUNKS;
	    *np = 0;
	}

	/* grow the list if necessary */
	if (*np == *mp) {
	    char *newmem;

	    newmem = realloc((char *)*ppp, (*mp+PCHUNKS) * sizeof(Pair));
	    if (!newmem) {
		xe_msg (1, "Can not realloc more Pairs.");
		return (-1);
	    }

	    *ppp = (Pair *) newmem;
	    *mp += PCHUNKS;
	}

	newp = &(*ppp)[*np];
	*np += 1;

	/* this lets us sort pairs of objects when their sep matches for
	 * later elimination of dups.
	 */
	if (op1 < op2) {
	    newp->op1 = op1;
	    newp->op2 = op2;
	} else {
	    newp->op1 = op2;
	    newp->op2 = op1;
	}
	newp->sep = (float)sep;

	return (0);
}

/* sort the two Pairs according to increasing separation in qsort() fashion
 * then by memory location of op1 then op2 -- this way two pairs of the same
 *   objects will sort adjacent, and is used later to elminate dups.
 */
static int
pair_cmpf (const void *v1, const void *v2)
{
	Pair *p1 = (Pair *)v1;
	Pair *p2 = (Pair *)v2;
	double s1 = (double)p1->sep;
	double s2 = (double)p2->sep;

	if (s1 < s2)
	    return (-1);
	else if (s1 > s2)
	    return (1);
	else {
	    Obj *opa, *opb;

	    opa = p1->op1;
	    opb = p2->op1;
	    if (opa < opb)
		return (-1);
	    else if (opa > opb)
		return (1);

	    opa = p1->op2;
	    opb = p2->op2;
	    if (opa < opb)
		return (-1);
	    else if (opa > opb)
		return (1);
	    else
		return (0);
	}
}

/* fill the list widget with the given set of pairs.
 * also update the attendent messages.
 * return 0 if ok else -1.
 */
static int
dspl_pairs (pp, np, sep)
Pair pp[];
int np;
double sep;
{
	XmString *xmstrtbl;
	char buf[128];
	char sepstr[64];
	int ns;
	int i;

	/* sort pairs by increasing separation then by object pointers */
	qsort ((void *)pp, np, sizeof(Pair), pair_cmpf);

	/* put into the list in one big push for best performance.
	 * beware of and remove any dups.
	 * ns ends up as the number of strings in xmstrtbl[].
	 */
	xmstrtbl = (XmString *) malloc (np * sizeof(XmString));
	if (!xmstrtbl) {
	    xe_msg (1, "No memory for %d XmStrings", np);
	    return (-1);
	}
	for (ns = i = 0; i < np; i++) {
	    Pair *ppi = &pp[i];

	    pm_set (66 + 29 * i / np);	/* about 66-95% time here */

	    /* can eliminate dups this way because of sort */
	    if (i > 0) {
		Pair *lpp = &pp[i-1];

		if (ppi->op1 == lpp->op1 && ppi->op2 == lpp->op2)
		    continue;
	    }

	    /* N.B. if you change this format, rework sky_point's extraction */

	    fs_pangle (sepstr, (double)ppi->sep);
	    (void) sprintf (buf, "%6.2f %-*.*s   %6.2f %-*.*s   %s",
					    get_mag(ppi->op1),
					    MAXNM-1, MAXNM-1, ppi->op1->o_name,
					    get_mag(ppi->op2),
					    MAXNM-1, MAXNM-1, ppi->op2->o_name,
					    sepstr);
	    xmstrtbl[ns++] = XmStringCreateSimple (buf);
	}

	/* reload the list */
	XtUnmanageChild (list_w);
	XmListDeleteAllItems (list_w);
	XmListAddItems (list_w, xmstrtbl, ns, 0);
	XtManageChild (list_w);

	/* finished with xmstrtbl */
	for (i = 0; i < ns; i++) 
	    XmStringFree (xmstrtbl[i]);
	free ((char *)xmstrtbl);

	/* update the messages */
	fs_pangle (sepstr, sep);
	(void) sprintf (buf, "Found %d %s Pair%s <=%s", ns,
	    pref_get(PREF_EQUATORIAL)==PREF_TOPO ? "Topocentric" : "Geocentric",
	    					ns==1 ? "" : "s", sepstr);
	set_xmstring (count_w, XmNlabelString, buf);
	timestamp (mm_get_now(), timestmp_w);

	pm_set (100);

	return (0);
}

/* create the list filename prompt */
static void
c_create_flist_w()
{
	Arg args[20];
	Widget tw;
	int n;

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg(args[n], XmNmarginWidth, 20);  n++;
	XtSetArg(args[n], XmNmarginHeight, 20);  n++;
	flist_w = XmCreatePromptDialog (cshell_w, "CloseList", args,n);
	set_something (flist_w, XmNcolormap, (XtArgVal)xe_cm);
	set_xmstring (flist_w, XmNdialogTitle, "xephem Close list");
	set_xmstring (flist_w, XmNselectionLabelString, "File name:");
	tw = XmSelectionBoxGetChild(flist_w, XmDIALOG_TEXT);
	defaultTextFN (tw, 1, "closelist.txt", NULL);
	sr_reg (tw, NULL, closecategory, 1);
	XtUnmanageChild (XmSelectionBoxGetChild(flist_w, XmDIALOG_HELP_BUTTON));
	XtAddCallback (flist_w, XmNokCallback, c_flistok_cb, NULL);
	XtAddCallback (flist_w, XmNmapCallback, prompt_map_cb, NULL);
}

/* called when the Ok button is hit in the file flist prompt */
/* ARGSUSED */
static void
c_flistok_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024];
	int icount;
	char *name;

	get_xmstring(w, XmNtextString, &name);

	if (strlen(name) == 0) {
	    xe_msg (1, "Please enter a file name.");
	    XtFree (name);
	    return;
	}

	get_something (list_w, XmNitemCount, (XtArgVal)&icount);
	if (icount == 0) {
	    xe_msg (1, "No items in list -- no file action.");
	    XtFree (name);
	    return;
	}

	if (existd (name,NULL) == 0 && confirm()) {
	    (void) sprintf (buf, "%s exists:\nAppend or Overwrite?", name);
	    query (cshell_w, buf, "Append", "Overwrite", "Cancel",
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
	    write_flist (fp);
	    (void) fclose (fp);
	} else {
	    XmToggleButtonSetState (autols_w, False, False);
	    XmToggleButtonSetState (autoup_w, False, False);
	}
}

/* write out all objects currently in the list, with header, to fp */
static void
write_flist (fp)
FILE *fp;
{
	XmStringTable items;
	int icount;
	char *p;
	int i;

	get_something (list_w, XmNitemCount, (XtArgVal)&icount);
	if (icount <= 0)
	    return;

	watch_cursor(1);

	get_xmstring (count_w, XmNlabelString, &p);
	(void) fprintf (fp, "%s", p);
	XtFree (p);
	get_xmstring (timestmp_w, XmNlabelString, &p);
	(void) fprintf (fp, " at %s\n", p);
	XtFree (p);

	get_something (list_w, XmNitems, (XtArgVal)&items);

	for (i = 0; i < icount; i++) {
	    XmStringGetLtoR (items[i], XmSTRING_DEFAULT_CHARSET, &p);
	    (void) fprintf (fp, "%s\n", p);
	    XtFree (p);
	}

	watch_cursor(0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: closemenu.c,v $ $Date: 2004/03/16 18:44:38 $ $Revision: 1.19 $ $Name:  $"};

/* code to manage the stuff on the "database" menu.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/Separator.h>

#include "xephem.h"

static void db_create_shell (void);
static void db_set_report (void);
static void db_openfifo_cb (Widget w, XtPointer client, XtPointer data);
static void db_index_cb (Widget w, XtPointer client, XtPointer data);
static void db_delall_cb (Widget w, XtPointer client, XtPointer data);
static void db_popdown_cb (Widget w, XtPointer client, XtPointer data);
static void db_relall_cb (Widget w, XtPointer client, XtPointer data);
static void db_help_cb (Widget w, XtPointer client, XtPointer data);
static void db_helpon_cb (Widget w, XtPointer client, XtPointer data);
static void db_loadpb_cb (Widget w, XtPointer client, XtPointer data);
static void db_close_cb (Widget w, XtPointer client, XtPointer data);
static void db_1catrow (Widget rc_w, DBCat *dbcp);
static void dbhdr_close_cb (Widget w, XtPointer client, XtPointer data);
static void db_showhdr(char *fn);

static Widget dbshell_w;	/* main shell */
static Widget catsw_w;		/* scrolled window managing the cat list */
static Widget report_w;		/* label with the dbstats */
static Widget load1_w;		/* TB whether to load Fav from .edb with 1 */
static Widget altnames_w;	/* TB whether to load alternate names */
static Widget ncats_w;		/* number of catalogs label */
static Widget nobjs_w;		/* number of objects label */
static Widget dbf_w;		/* form for displaying catalog header */
static Widget dbt_w;		/* T widget for display catalog header */

/* bring up the db menu, creating if first time */
void
db_manage()
{
	if (!dbshell_w)
	    db_create_shell();

	db_set_report();	/* get a fresh count */
	XtPopup (dbshell_w, XtGrabNone);
	set_something (dbshell_w, XmNiconic, (XtArgVal)False);
}

/* called when the database beyond the builtin ones has changed in any way.
 * as odd as this seems since *this* menu controls the db contents, this was
 *   added when the db fifo input path was added. it continued to be handy
 *   when initial db files could be loaded automatically and we introduced
 *   fields stars in 2.9.
 * all we do is update our tally, if we are up at all.
 */
/* ARGSUSED */
void
db_newdb (appended)
int appended;
{
	if (isUp(dbshell_w))
	    db_set_report();
}

/* return whether to check for alternate names when reading a catalog */
int
db_chkAltNames(void)
{
	if (!altnames_w)
	    db_create_shell();
	return (XmToggleButtonGetState (altnames_w));
}

/* update the list of catalogs.
 */
void
db_newcatmenu (dbcp, ndbcp)
DBCat dbcp[];
int ndbcp;
{
	char buf[128];
	Arg args[20];
	Widget ww;
	int n;

	/* create if not already */
	if (!dbshell_w)
	    db_create_shell();

	/* set count label */
	sprintf (buf, "%d Loaded Catalog%c", ndbcp, ndbcp==1?' ':'s');
	set_xmstring (ncats_w, XmNlabelString, buf);

	/* replace workWindow */
	get_something (catsw_w, XmNworkWindow, (XtArgVal)&ww);
	XtDestroyWidget (ww);
	n = 0;
	XtSetArg (args[n], XmNspacing, 0); n++;
	ww = XmCreateRowColumn (catsw_w, "CatRC", args, n);
	wtip (ww, "List of all catalogs now in memory");
	set_something (catsw_w, XmNworkWindow, (XtArgVal)ww);

	/* fill with each cat info */
	for (n = 0; n < ndbcp; n++)
	    db_1catrow (ww, dbcp+n);

	/* ok */
	XtManageChild (ww);
}

/* called to put up or remove the watch cursor.  */
void
db_cursor (c)
Cursor c;
{
	Window win;

	if (dbshell_w && (win = XtWindow(dbshell_w)) != 0) {
	    Display *dsp = XtDisplay(dbshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* return 1 if want to autoload favorite if read a .edb with 1 entry, else 0 */
int
db_load1()
{
	/* create if not already */
	if (!dbshell_w)
	    db_create_shell();

	return (XmToggleButtonGetState(load1_w));
}

/* create a shell to allow user to manage files . */
static void
db_create_shell ()
{
	typedef struct {
	    char *label;	/* what goes on the help label */
	    char *key;		/* string to call hlp_dialog() */
	    char *tip;		/* button tip text */
	} HelpOn;
	static HelpOn helpon[] = {
	    {"Intro...", "Database",
		"How to load and delete .edb catalogs files"},
	    {"on Control...", "Database_control",
		"Information about the Control pulldown menu"},
	    {"File format...", "Database_files",
		"Definition of the XEphem .edb file format"},
	    {"Notes...", "Database_notes",
		"Additional info about the XEphem data base"},
	};
	Widget mb_w, pd_w, cb_w;
	Widget rc_w;
	Widget dbform_w;
	XmString str;
	Widget w;
	char *s[2];
	Arg args[20];
	int n;
	int i;
	
	/* create outter shell and form */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Database Files"); n++;
	XtSetArg (args[n], XmNiconName, "DB"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	dbshell_w = XtCreatePopupShell ("DB", topLevelShellWidgetClass,
						    toplevel_w, args, n);
	setup_icon (dbshell_w);
	set_something (dbshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (dbshell_w, XmNpopdownCallback, db_popdown_cb, NULL);
	sr_reg (dbshell_w, "XEphem*DB.x", dbcategory, 0);
	sr_reg (dbshell_w, "XEphem*DB.y", dbcategory, 0);
	sr_reg (dbshell_w, "XEphem*DB.height", dbcategory, 0);
	sr_reg (dbshell_w, "XEphem*DB.width", dbcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, 5); n++;
	dbform_w = XmCreateForm (dbshell_w, "DBManage", args, n);
	XtAddCallback (dbform_w, XmNhelpCallback, db_help_cb, 0);
	XtManageChild(dbform_w);

	/* create the menu bar across the top */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (dbform_w, "MB", args, n);
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

	    /* handy Index */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Idx", args, n);
	    XtAddCallback (w, XmNactivateCallback, db_index_cb, 0);
	    set_xmstring(w, XmNlabelString, "Index...");
	    wtip (w, "Search and inspect loaded objects");
	    XtManageChild (w);

	    /* delete all */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "DelAll", args, n);
	    XtAddCallback (w, XmNactivateCallback, db_delall_cb, 0);
	    set_xmstring(w, XmNlabelString, "Delete all");
	    wtip (w, "Remove all files from memory");
	    XtManageChild (w);

	    /* reload all */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "RelAll", args, n);
	    XtAddCallback (w, XmNactivateCallback, db_relall_cb, 0);
	    set_xmstring(w, XmNlabelString, "Reload all");
	    wtip (w, "Reload all catalogs already in memory");
	    XtManageChild (w);

	    /* make the open fifo button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "OpenFIFO", args, n);
	    XtAddCallback (w, XmNactivateCallback, db_openfifo_cb, 0);
	    set_xmstring(w, XmNlabelString, "Open DB Fifo");
	    wtip (w, "Activate import path for Object definitions");
	    XtManageChild (w);

	    /* make the load-1 toggle button */

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True);  n++;
	    load1_w = XmCreateToggleButton (pd_w, "Load1", args, n);
	    set_xmstring(load1_w, XmNlabelString, "Make Favorite when read 1");
	    wtip (load1_w,
		"Reading a file containing 1 entry also adds it to Favorites");
	    XtManageChild (load1_w);
	    sr_reg (load1_w, NULL, dbcategory, 1);

	    /* make the Record duplicate names */

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True);  n++;
	    altnames_w = XmCreateToggleButton (pd_w, "CheckAltNames", args, n);
	    set_xmstring(altnames_w, XmNlabelString, "Check alternate names");
	    wtip (altnames_w,
		"Check and record alternate names when loading; nice but slow");
	    XtManageChild (altnames_w);
	    sr_reg (altnames_w, NULL, dbcategory, 1);

	    /* add a separator */

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    /* add the close button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, db_close_cb, 0);
	    wtip (w, "Close this window");
	    XtManageChild (w);

	/* make the Files pulldown */

	s[0] = ".edb";
	s[1] = ".tle";
	cb_w = createFSM (mb_w, s, 2, "catalogs", db_loadpb_cb);
	wtip (cb_w, "Select catalog file to load");

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
		XtAddCallback (w, XmNactivateCallback, db_helpon_cb,
							(XtPointer)(hp->key));
		if (hp->tip)
		    wtip (w, hp->tip);
		XtManageChild (w);
		XmStringFree(str);
	    }

	/* make the report output-only text under a label */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNtopOffset, 5); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	rc_w = XmCreateRowColumn (dbform_w, "RCO", args, n);
	XtManageChild (rc_w);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    nobjs_w = XmCreateLabel (rc_w, "NObjs", args, n);
	    XtManageChild (nobjs_w);

	    n = 0;
	    XtSetArg (args[n], XmNeditable, False); n++;
	    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	    XtSetArg (args[n], XmNblinkRate, 0); n++;
	    XtSetArg (args[n], XmNrows, 16); n++; /* must match db_set_report */
	    XtSetArg (args[n], XmNcolumns, 32); n++;
	    report_w = XmCreateText (rc_w, "Report", args, n);
	    wtip (report_w,
			"Breakdown of number and types of objects in memory");
	    XtManageChild (report_w);

	/* label for Catalogs */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNtopOffset, 5); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	ncats_w = XmCreateLabel (dbform_w, "NCats", args, n);
	set_xmstring (ncats_w, XmNlabelString, "0 Loaded Catalogs");
	XtManageChild (ncats_w);

	/* create a RC in a SW for showing and deleting loaded catalogs.
	 * the RC is replaced each time the list changes.
	 */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ncats_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	catsw_w = XmCreateScrolledWindow (dbform_w, "CatSW", args, n);
	XtManageChild (catsw_w);

	    n = 0;
	    w = XmCreateRowColumn (catsw_w, "CatRCDummy", args, n);
	    XtManageChild (w);
}

/* a PB in a file PD has been chosen.
 */
static void
db_loadpb_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *fn;

	watch_cursor(1);
	get_xmstring (w, XmNlabelString, &fn);
	db_read (fn);
	XtFree (fn);
	all_newdb(1);
	watch_cursor(0);
}

/* compile the stats report into report_w.
 * N.B. we do not include the planets nor the user objects.
 */
static void
db_set_report()
{
	DBScan dbs;
	char report[1024];
	Obj *op;
	int nes=0, ne=0, np=0, nh=0, npm=0, nbs=0;
	int nc=0, ng=0, nj=0, nn=0, npn=0, nl=0, nq=0, nr=0, ns=0, no=0, ny=0;
	int n=0;

	for (db_scaninit(&dbs, ALLM, NULL, 0); (op=db_scan(&dbs)) != NULL; ){
	    switch (op->o_type) {
	    case FIXED:
		switch (op->f_class) {
		case 'C': case 'U': case 'O':
		    nc++;
		    break;
		case 'G': case 'H': case 'A':
		    ng++;
		    break;
		case 'N': case 'F': case 'K':
		    nn++;
		    break;
		case 'J':
		    nj++;
		    break;
		case 'P':
		    npn++;
		    break;
		case 'L':
		    nl++;
		    break;
		case 'Q':
		    nq++;
		    break;
		case 'R':
		    nr++;
		    break;
		case 'T': case 'B': case 'D': case 'M': case 'S': case 'V': 
		    ns++;
		    break;
		case 'Y':
		    ny++;
		    break;
		default: 
		    no++;
		    break;
		}
		break;
	    case BINARYSTAR: nbs++; break;
	    case PLANET:     npm++; break;
	    case ELLIPTICAL: ne++;  break;
	    case HYPERBOLIC: nh++;  break;
	    case PARABOLIC:  np++;  break;
	    case EARTHSAT:   nes++; break;
	    case UNDEFOBJ:          break;
	    default:
		printf ("Unknown object type: %d\n", op->o_type);
		abort();
	    }
	    n++;
	}

	(void) sprintf (report, "\
%6d Solar -- planets and moons\n\
%6d Solar -- elliptical\n\
%6d Solar -- hyperbolic\n\
%6d Solar -- parabolic\n\
%6d Earth satellites\n\
%6d Clusters (C,U,O)\n\
%6d Galaxies (G,H,A)\n\
%6d Nebulae (N,F,K)\n\
%6d Planetary Nebulae (P)\n\
%6d Pulsars (L)\n\
%6d Quasars (Q)\n\
%6d Radio sources (J)\n\
%6d Supernovae (Y)\n\
%6d Supernova Remnants (R)\n\
%6d Stars (S,V,D,B,M,T)\n\
%6d Binary systems\n\
%6d Undefined\
",
	npm, ne, nh, np, nes, nc, ng, nn, npn, nl, nq, nj, ny, nr, ns, nbs, no);

	/* set report and count in label */
	set_something (report_w, XmNvalue, (XtArgVal)report);
	sprintf (report, "%d Loaded Objects", n);
	set_xmstring (nobjs_w, XmNlabelString, report);
}

static void
dbdelall()
{
	db_del_all();
	all_newdb(0);
}

/* callback when user wants to bring up the Index window
 */
/* ARGSUSED */
static void
db_index_cb (w, client, data)
Widget w;
XtPointer client;
XtPointer data;
{
	obj_manage();
}

/* callback when user wants to delete all files
 */
/* ARGSUSED */
static void
db_delall_cb (w, client, data)
Widget w;
XtPointer client;
XtPointer data;
{
	if (confirm())
	    query (dbshell_w, "Delete all files from memory?",
		   "Yes .. delete all", "No .. no change", NULL,
		   dbdelall, NULL, NULL);
	else 
	    dbdelall();
}

static void
dbrelall()
{
	watch_cursor(1);
	db_rel_all();
	all_newdb(0);
	watch_cursor(0);
}

/* callback when user wants to reload all files
 */
/* ARGSUSED */
static void
db_relall_cb (w, client, data)
Widget w;
XtPointer client;
XtPointer data;
{
	if (confirm())
	    query (dbshell_w, "Reload all files in memory?",
		   "Yes -- reload all", "No -- no change", NULL,
		   dbrelall, NULL, NULL);
	else 
	    dbrelall();
}

/* callback from the open fifo button */
/* ARGSUSED */
static void
db_openfifo_cb (w, client, data)
Widget w;
XtPointer client;
XtPointer data;
{
	db_connect_fifo();
}

/* ARGSUSED */
static void
db_help_cb (w, client, data)
Widget w;
XtPointer client;
XtPointer data;
{
	static char *msg[] = {
"This displays a count of the various types of objects currently in memory.",
"Database files may be read in to add to this list or the list may be deleted."
};

	hlp_dialog ("Database", msg, XtNumber(msg));
}

/* callback from a specific Help button.
 * client is a string to use with hlp_dialog().
 */
/* ARGSUSED */
static void
db_helpon_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hlp_dialog ((char *)client, NULL, 0);
}

/* ARGSUSED */
static void
db_popdown_cb (w, client, data)
Widget w;
XtPointer client;
XtPointer data;
{
	if (dbf_w)
	    XtUnmanageChild (dbf_w);
}

/* ARGSUSED */
static void
db_close_cb (w, client, data)
Widget w;
XtPointer client;
XtPointer data;
{
	XtPopdown (dbshell_w);
}

/* call from any of the Delete catalog PBs.
 * client is a DBCat *.
 */
/* ARGSUSED */
static void
catdel_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	watch_cursor(1);
	db_catdel ((DBCat *)client);
	all_newdb(0);
	watch_cursor(0);
}

/* call from any of the Header catalog PBs.
 * client is a DBCat *.
 */
/* ARGSUSED */
static void
cathdr_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	watch_cursor(1);
	db_showhdr (((DBCat *)client)->name);
	watch_cursor(0);
}

/* build one new catalog entry */
static void
db_1catrow (rc_w, dbcp)
Widget rc_w;
DBCat *dbcp;
{
	Widget f_w, pb_w;
	Widget nl_w, cl_w;
	char buf[32];
	Arg args[20];
	int n;
	int i, nobjs;

	n = 0;
	f_w = XmCreateForm (rc_w, "CatForm", args, n);
	XtManageChild (f_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNshadowThickness, 1); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	pb_w = XmCreatePushButton (f_w, "Delete", args, n);
	wtip (pb_w, "Delete all objects from this catalog");
	XtAddCallback (pb_w, XmNactivateCallback, catdel_cb, (XtPointer)dbcp);
	XtManageChild (pb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, pb_w); n++;
	XtSetArg (args[n], XmNleftOffset, 3); n++;
	XtSetArg (args[n], XmNshadowThickness, 1); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	pb_w = XmCreatePushButton (f_w, "Header", args, n);
	wtip (pb_w, "Display header info from this catalog");
	XtAddCallback (pb_w, XmNactivateCallback, cathdr_cb, (XtPointer)dbcp);
	XtManageChild (pb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, pb_w); n++;
	XtSetArg (args[n], XmNleftOffset, 3); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	nl_w = XmCreateLabel (f_w, "NObjs", args, n);
	XtManageChild (nl_w);

	for (nobjs = i = 0; i < NOBJTYPES; i++)
	    nobjs += dbcp->tmem[i].nuse;
	(void) sprintf (buf, "%6d", nobjs);
	set_xmstring (nl_w, XmNlabelString, buf);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, nl_w); n++;
	XtSetArg (args[n], XmNleftOffset, 3); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	cl_w = XmCreateLabel (f_w, "Name", args, n);
	XtManageChild (cl_w);
	set_xmstring (cl_w, XmNlabelString, dbcp->name);
}

/* display the header of the given catalog file */
static void
db_showhdr(char *fn)
{
	char buf[1024];
	FILE *fp;
	char *hdr;
	int n, l;

	/* create the text window first time */
	if (!dbf_w) {
	    Arg args[20];
	    Widget w;

	    n = 0;
	    XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	    XtSetArg (args[n], XmNmarginWidth, 10);  n++;
	    XtSetArg (args[n], XmNmarginHeight, 10);  n++;
	    dbf_w = XmCreateFormDialog (toplevel_w, "DBHeader", args, n);
	    set_something (dbf_w, XmNcolormap, (XtArgVal)xe_cm);
	    set_xmstring (dbf_w, XmNdialogTitle, "xephem Catalog header");

	    n = 0;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 10); n++;
	    w = XmCreatePushButton (dbf_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, dbhdr_close_cb, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNbottomWidget, w); n++;
	    XtSetArg (args[n], XmNbottomOffset, 10); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrows, 24); n++;
	    XtSetArg (args[n], XmNcolumns, 82); n++;
	    XtSetArg (args[n], XmNeditable, False); n++;
	    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	    XtSetArg (args[n], XmNblinkRate, 0); n++;
	    dbt_w = XmCreateScrolledText (dbf_w, "DBH", args, n);
	    XtManageChild (dbt_w);
	}

	fp = fopend (fn, "catalogs", "r");
	if (!fp)
	    return;

	hdr = XtMalloc(1);
	hdr[0] = '\0';
	for (n = l = 0; fgets (buf, sizeof(buf), fp) != NULL; l += n) {
	    if (dbline_candidate (buf) == 0)
		continue;
	    n = strlen (buf);
	    hdr = XtRealloc (hdr, n+l+1);
	    strcpy (hdr+l, buf);
	}
	XmTextSetString (dbt_w, hdr);
	XtFree (hdr);

	XtManageChild (dbf_w);
}

static void
dbhdr_close_cb (Widget w, XtPointer client, XtPointer call)
{
	if (dbf_w)
	    XtUnmanageChild (dbf_w);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: dbmenu.c,v $ $Date: 2005/04/27 17:04:59 $ $Revision: 1.48 $ $Name:  $"};

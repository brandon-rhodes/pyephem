/* support for AAVSO.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>

#include "xephem.h"

#define	LISTROWS	7	/* rows in list */
#define	DESCOLS		10	/* columns in avdb for designation */
#define	NAMCOLS		20	/* longest selection name */
#define	TFCOL		12	/* columns in text fields */
#define	AVEPOCH		1900	/* epoch of entries in avdb */
static char avdb[] = "starcensus.txt";		/* AAVSO star list */
static char avhost[] = "www.aavso.org";		/* host */
static char avcgi[] = "/cgi-bin/kstar.pl";	/* aavso's cgi-bin script */
static char default_str[] = "default";		/* knows to use nice interval */
static char latest_str[] = "latest";		/* knows to use latest */

static void av_create (void);
static void av_help_cb (Widget w, XtPointer client, XtPointer call);
static void av_close_cb (Widget w, XtPointer client, XtPointer call);
static void av_print_cb (Widget w, XtPointer client, XtPointer call);
static void av_list_cb (Widget w, XtPointer client, XtPointer call);
static void av_go_cb (Widget w, XtPointer client, XtPointer call);
static void av_name_cb (Widget w, XtPointer client, XtPointer call);
static void mkPrompt (Widget parent, char *prompt, Widget *text_wp, char *name);
static void mkTop (Widget rc_w);
static FILE * openDB (void);
static int fetchpm (void);
static void cleanLine (char *line);
static void fetchAndShow (void);
static void av_print (void);

static Widget av_w;		/* main shell */
static Widget avgrl_w;		/* Label for displaying graph */
static Widget avlist_w;		/* List for displaying closest choices */
static Widget avname_w;		/* TF for AAVSO star name */
static Widget avdes_w;		/* TF for AAVSO star designation */
static Widget avjd1_w;		/* TF for starting time */
static Widget avjd2_w;		/* TF for ending time */
static Widget avfit_w;		/* TF for running average period, days */
static Pixmap curve_pm;		/* the aavso light curve pixmap */

/* AAVSO binary query options */
typedef struct {
    char *name;			/* TB name */
    char *desc;			/* description */
    char *tstr;			/* true URL string */
    char *fstr;			/* false URL string */
    Widget w;			/* controlling TB */
} ABOpt;
/* N.B.: must be in order required by URL */
static ABOpt abopts[] = {
    { "FainterThan", "Fainter-than data", "yes", "no"},
    { "CCDV",        "CCDV data",         "yes", "no"},
    { "CCDI",        "CCDI data",         "yes", "no"},
    { "CCDR",        "CCDR data",         "yes", "no"},
    { "CCDB",        "CCDB data",         "yes", "no"},
    { "Visual",      "Visual data",       "yes", "no"},
    { "Discrepant",  "Discrepant data",   "yes", "no"},
};

static char aavsocategory[] = "AAVSO";	/* Save category */

/* create and bring up the AAVSO dialog */
void
av_manage ()
{
	if (!av_w)
	    av_create();

	XtPopup (av_w, XtGrabNone);
	set_something (av_w, XmNiconic, (XtArgVal)False);
}

/* find the closest LISTROWS stars to op in AAVSO database and load dialog */
void
av_load (op)
Obj *op;
{
	typedef struct {
	    char line[DESCOLS+NAMCOLS];	/* designation in avdb file */
	    double csep;		/* cos sep from op */
	} DBSort;
	Now *np = mm_get_now();
	double ra0, dec0;
	double cdec0, sdec0;
	double mjd0;
	double mjd1;
	DBSort dbs[LISTROWS];
	int ndbs;
	char line[DESCOLS+NAMCOLS];
	FILE *fp;
	int i;

	/* courtesy display */
	av_manage();

	/* open */
	fp = openDB();
	if (!fp)
	    return;

	watch_cursor(1);

	/* precess op's loc to AVEPOCH */
	ra0 = op->s_ra;
	dec0 = op->s_dec;
	mjd0 = epoch == EOD ? mjd : epoch;
	cal_mjd (1, 1.0, AVEPOCH, &mjd1);
	precess (mjd0, mjd1, &ra0, &dec0);
	cdec0 = cos(dec0);
	sdec0 = sin(dec0);

	/* search for closest and collect in dbs[] in ascending order.
	 * an exact match name always sorts highest.
	 */
	ndbs = 0;
	while (fgets (line, sizeof(line), fp)) {
	    int rah, ram, decd;
	    double csep;
	    char dsign;
	    int match;
	    char *namend;

	    if (sscanf (line, "%2d%2d%c%2d", &rah, &ram, &dsign, &decd) != 4)
		continue;
	    for (namend = line+strlen(line)-1; namend > line+10; namend--)
		if (!strchr (" \r\n", *namend))
		    break;
	    namend[1] = '\0';
	    match = !strcmp (op->o_name, line+10);
	    if (match) {
		csep = 1.0;
	    } else {
		double ra1 = hrrad(rah + ram/60.);
		double dec1 = degrad(decd) * (dsign == '-' ? -1 : 1);
		solve_sphere (ra0-ra1, PI/2-dec1, sdec0, cdec0, &csep, NULL);
	    }
	    if (ndbs == 0 || csep > dbs[ndbs-1].csep) {
		/* bubble into dbs[], best at [0] */
		for (i = ndbs; i > 0 && csep > dbs[i-1].csep; --i)
		    if (i < LISTROWS)
			dbs[i] = dbs[i-1];
		cleanLine (line);
		strcpy (dbs[i].line, line);
		dbs[i].csep = csep;
		if (ndbs < LISTROWS)
		    ndbs++;
	    }
	}

	fclose (fp);

	/* put into avlist_w */
	XmListDeleteAllItems (avlist_w);
	for (i = 0; i < LISTROWS; i++) {
	    XmString xmstr = XmStringCreateSimple (dbs[i].line);
	    XmListAddItem (avlist_w, xmstr, 0);	/* append */
	    XmStringFree (xmstr);
	}

	/* also put best into selections */
	XmTextFieldSetString (avname_w, &dbs[0].line[DESCOLS]);
	strcpy (line, dbs[0].line);
	line[DESCOLS] = '\0';
	XmTextFieldSetString (avdes_w, line);

	watch_cursor(0);
}

/* open the census file.
 * return a FILE * else NULL.
 */
static FILE *
openDB()
{
	char fn[1024];
	FILE *fp;

	/* open */
	(void) sprintf (fn, "%s/auxil/%s", getShareDir(), avdb);
	fp = fopen (fn, "r");
	if (!fp)
	    xe_msg (1, "%s:\n%s\n", fn, syserrstr());
	return (fp);
}

/* called to put up or remove the watch cursor.  */
void
av_cursor (c)
Cursor c;
{
	Window win;

	if (av_w && (win = XtWindow(av_w)) != 0) {
	    Display *dsp = XtDisplay(av_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create the AAVSO shell */
static void
av_create()
{
	Widget rc_w;
	Widget fr_w, f_w, w;
	Arg args[20];
	int n;

	/* create shell */
	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem AAVSO"); n++;
	XtSetArg (args[n], XmNiconName, "AAVSO"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	av_w = XtCreatePopupShell ("AAVSO", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (av_w);
	set_something (av_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (av_w, "XEphem*AAVSO.x", aavsocategory, 0);
	sr_reg (av_w, "XEphem*AAVSO.y", aavsocategory, 0);

	/* pile things up in a RC */
	n = 0;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNspacing, 5); n++;
	rc_w = XmCreateRowColumn (av_w, "AAVSORC", args, n);
	XtAddCallback (rc_w, XmNhelpCallback, av_help_cb, 0);
	XtManageChild (rc_w);

	/* credit heading */
	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	w = XmCreateLabel (rc_w, "MsgL", args, n);
	set_xmstring (w, XmNlabelString, "Courtesy http://www.aavso.org");
	XtManageChild (w);

	/* create top section */
	mkTop (rc_w);

	/* graph goes in a label in a frame */
	n = 0;
	fr_w = XmCreateFrame (rc_w, "Fr", args, n);
	XtManageChild (fr_w);
	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	    avgrl_w = XmCreateLabel (fr_w, "Graph", args, n);
	    set_xmstring (avgrl_w, XmNlabelString, " ");
	    XtManageChild (avgrl_w);

	/* bottom controls */
	n = 0;
	f_w = XmCreateForm (rc_w, "F", args, n);
	XtManageChild (f_w);

	    /* go */
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 4); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 24); n++;
	    w = XmCreatePushButton (f_w, "Go", args, n);
	    set_xmstring (w, XmNlabelString, "Get Curve");
	    XtAddCallback (w, XmNactivateCallback, av_go_cb, 0);
	    wtip (w, "Fetch light curve from AAVSO for Name.");
	    XtManageChild (w);

	    /* print */
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 28); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 48); n++;
	    w = XmCreatePushButton (f_w, "Pr", args, n);
	    set_xmstring (w, XmNlabelString, "Print");
	    XtAddCallback (w, XmNactivateCallback, av_print_cb, 0);
	    wtip (w, "Print light curve.");
	    XtManageChild (w);

	    /* close */
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 52); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 72); n++;
	    w = XmCreatePushButton (f_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, av_close_cb, 0);
	    wtip (w, "Close this dialog.");
	    XtManageChild (w);

	    /* help */
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 76); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 96); n++;
	    w = XmCreatePushButton (f_w, "Help", args, n);
	    wtip (w, "Obtain more information about this dialog.");
	    XtAddCallback (w, XmNactivateCallback, av_help_cb, 0);
	    XtManageChild (w);
}

/* construct the top section */
static void
mkTop (rc_w)
Widget rc_w;
{
	Widget lrc_w, rrc_w, orc_w;
	Widget f_w, w;
	Arg args[20];
	int i, n;

	/* 2 vertical RCs placed adjacent in a form */
	n = 0;
	f_w = XmCreateForm (rc_w, "F", args, n);
	XtManageChild (f_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 49); n++;
	lrc_w = XmCreateRowColumn (f_w, "LRC", args, n);
	XtManageChild (lrc_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 51); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	rrc_w = XmCreateRowColumn (f_w, "RRC", args, n);
	XtManageChild (rrc_w);

	/* stuff in left column */

	/* title to balance with right side */
	n = 0;
	w = XmCreateLabel (lrc_w, "LL", args, n);
	set_xmstring (w, XmNlabelString, "Setup:");
	XtManageChild (w);

	/* designation */
	mkPrompt (lrc_w, "Designation:", &avdes_w, "Designation");
	wtip (avdes_w, "Enter AAVSO designation, e.g., 0214-03");
	XtAddCallback (avdes_w, XmNactivateCallback, av_go_cb, NULL);

	/* name */
	mkPrompt (lrc_w, "Name:", &avname_w, "Name");
	wtip (avname_w, "Look up AAVSO name, e.g., OMI CET");
	XtAddCallback (avname_w, XmNactivateCallback, av_name_cb, NULL);

	/* date range */
	mkPrompt (lrc_w, "Start JD or Date:", &avjd1_w, "StartDate");
	wtip (avjd1_w, "Enter starting JD, or Date, or \"default\"");
	XtAddCallback (avjd1_w, XmNactivateCallback, av_go_cb, NULL);
	mkPrompt (lrc_w, "End JD or Date:", &avjd2_w, "EndDate");
	wtip (avjd2_w, "Enter ending JD, or Date, or \"latest\"");
	XtAddCallback (avjd2_w, XmNactivateCallback, av_go_cb, NULL);

	/* running average */
	mkPrompt (lrc_w, "Smooth fit, days:", &avfit_w, "SmoothDays");
	wtip (avfit_w, "Running average period, or 0 to disable");

	/* stuff in right column */

	n = 0;
	w = XmCreateLabel (rrc_w, "L", args, n);
	XtManageChild (w);
	set_xmstring (w, XmNlabelString, "AAVSO Stars near last Sky View pick:"); 

	n = 0;
	XtSetArg (args[n], XmNvisibleItemCount, LISTROWS); n++;
	XtSetArg (args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
	avlist_w = XmCreateList (rrc_w, "List", args, n);
	XtAddCallback (avlist_w, XmNsingleSelectionCallback, av_list_cb, NULL);
	XtAddCallback (avlist_w, XmNdefaultActionCallback, av_list_cb, NULL);
	wtip (avlist_w, "Stars nearest to last Sky View AAVSO pick");
	XtManageChild(avlist_w);

	/* binary options in RC below */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (rc_w, "AOL", args, n);
	set_xmstring (w, XmNlabelString, "Plot:");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNmarginWidth, 5); n++;
	XtSetArg (args[n], XmNspacing, 5); n++;
	XtSetArg (args[n], XmNnumColumns, 4); n++;
	orc_w = XmCreateRowColumn (rc_w, "Plot", args, n);
	XtManageChild (orc_w);

	for (i = 0; i < XtNumber(abopts); i++) {
	    ABOpt *ap = &abopts[i];

	    n = 0;
	    w = XmCreateToggleButton (orc_w, ap->name, args, n);
	    set_xmstring (w, XmNlabelString, ap->desc);
	    XtManageChild (w);
	    sr_reg (w, NULL, aavsocategory, 1);
	    ap->w = w;
	}
}

/* callback from the Close PB */
/* ARGSUSED */
static void
av_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (av_w);
}

/* callback from the Print PB */
/* ARGSUSED */
static void
av_print_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (curve_pm)
	    XPSAsk ("AAVSO", av_print);
	else
	    xe_msg (1, "No light curve to print");
}

/* generate a postscript file containing the curve_pm.
 * call XPSClose() when finished.
 */
static void
av_print()
{
	Display *dsp = XtDisplay(toplevel_w);
	unsigned int wid, hei, b, d;
	Window rootwin;
	int pw;
	int x, y;

	watch_cursor (1);

	XGetGeometry (dsp, curve_pm, &rootwin, &x, &y, &wid, &hei, &b, &d);
	pw = (int)(72*6.5*wid/hei+.5);	/* width on paper when 6.5 hi */

	XPSXBegin (curve_pm, 0, 0, wid, hei, (int)((8.5*72-pw)/2), 10*72, pw);
	XPSPixmap (curve_pm, wid, hei, xe_cm, 0, 1);
	XPSXEnd();
	XPSClose();

	watch_cursor (0);
}

/* callback for Help
 */
/* ARGSUSED */
static void
av_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"Fetch and display light curves from AAVSO."
};

	hlp_dialog ("AAVSO", msg, sizeof(msg)/sizeof(msg[0]));
}

/* callback when an item is selected in the list
 */
/* ARGSUSED */
static void
av_list_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmListCallbackStruct *lp = (XmListCallbackStruct *)call;
	char *str;

	/* load into selection */
	XmStringGetLtoR (lp->item, XmSTRING_DEFAULT_CHARSET, &str);
	XmTextFieldSetString (avname_w, str+DESCOLS);
	str[DESCOLS] = '\0';
	XmTextFieldSetString (avdes_w, str);
	XtFree (str);

	/* if double-click proceed with fetch */
	if (lp->reason == XmCR_DEFAULT_ACTION)
	    fetchAndShow();
}

/* callback from the Go PB or all TF except Name.
 */
/* ARGSUSED */
static void
av_go_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	fetchAndShow();
}

/* callback from the Name TF.
 * lookup Name and fill in Designation.
 * then Go.
 */
/* ARGSUSED */
static void
av_name_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	String name;
	char buf[1024];
	char nameuc[64];
	char c, *np, *ucp;
	FILE *fp;

	/* open right off -- else no point going further */
	fp = openDB();
	if (!fp)
	    return;

	/* copy name to nameuc, skipping blanks and promoting to upper case */
	name = XmTextFieldGetString (avname_w);
	for (ucp = nameuc, np = name; (c = *np) != '\0'; np++) {
	    if (ucp == nameuc && isspace(c))
		continue;
	    if (islower(c))
		c = toupper(c);
	    *ucp++ = c;
	}
	*ucp = '\0';
	while ((c = *--ucp) == ' ')
	    *ucp = '\0';
	XtFree (name);

	/* search db */
	while (fgets (buf, sizeof(buf), fp)) {
	    np = &buf[strlen(buf)-1];
	    while (np >= buf && ((c = *np) == '\r' || c == '\n' || c == ' '))
		*np-- = '\0';
	    if (strcmp (buf+DESCOLS, nameuc) == 0) {
		buf[DESCOLS] = '\0';
		XmTextFieldSetString (avdes_w, buf);
		fetchAndShow();
		fclose(fp);
		return;
	    }
	}
	fclose(fp);

	xe_msg (1, "Can not find '%s' in %s", nameuc, avdb);
}

/* create a prompt/text field pair */
static void
mkPrompt (parent, prompt, text_wp, txtname)
Widget parent;
char *prompt;
Widget *text_wp;
char *txtname;
{
	Widget l_w, t_w, f_w;
	Arg args[20];
	int n;

	/* form */
	n = 0;
	f_w = XmCreateForm (parent, "PF", args, n);
	XtManageChild (f_w);

	/* prompt label */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 48); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	l_w = XmCreateLabel (f_w, "PL", args, n);
	set_xmstring (l_w, XmNlabelString, prompt);
	XtManageChild (l_w);

	/* TF */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 52); n++;
	XtSetArg (args[n], XmNmaxLength, NAMCOLS); n++;
	XtSetArg (args[n], XmNcolumns, TFCOL); n++;
	t_w = XmCreateTextField (f_w, txtname, args, n);
	sr_reg (t_w, NULL, aavsocategory, 0);
	XtManageChild(t_w);

	*text_wp = t_w;
}

/* build the query date string from the user's string */
static void
buildDateString (ustr, dstr, def)
char *ustr, dstr[];
char *def;
{
	/* skip leading blanks */
	while (*ustr == ' ')
	    ustr++;

	if (strchr(ustr, '/')) {
	    /* treat as date */
	    int mo, yr;
	    double dy, Mjd;

	    f_sscandate (ustr, pref_get(PREF_DATE_FORMAT), &mo, &dy, &yr);
	    cal_mjd (mo, dy, yr, &Mjd);
	    sprintf (dstr, "%.2f", Mjd += MJD0);
	} else if (*ustr) {
	    /* no change .. just strip any trailing blanks.
	     * except web interfaec wants "defualt" instread of "latestest".
	     */
	    if (strcmp (ustr, latest_str) == 0)
		strcpy (dstr, default_str);
	    else {
		char *end;

		strcpy (dstr, ustr);
		for (end = &dstr[strlen(dstr)-1]; *end == ' '; end--)
		    *end = '\0';
	    }
	} else {
	    /* blank becomes "default" literal */
	    strcpy (dstr, def);
	}
}

/* fetch graph of Designation into curve_pm.
 * return 0 if ok else -1 if trouble.
 */
static int
fetchpm()
{
	static char fmt[] = "GET http://%s%s?%s?%s?%s?%g?%s?%s?%s?%s?%s?%s?%s?XEphem HTTP/1.0\r\nUser-Agent: xephem/%s\r\n\r\n";
	Display *dsp = XtDisplay(toplevel_w);
	int nrawgif;
	char buf[1024];
	char jd1[32], jd2[32];
	char *idp, id[NAMCOLS+1];
	char *abop[XtNumber(abopts)];
	double fit;
	char *str;
	int isgif;
	int length;
	int w, h;
	int nr;
	int fd;
	int i;
	unsigned char rawgif[250000];

	/* announce net activity and give user a way to stop */
	watch_cursor(1);
	stopd_up();

	/* form the two date range strings */
	str = XmTextFieldGetString (avjd1_w);
	buildDateString (str, jd1, default_str);
	XtFree (str);
	str = XmTextFieldGetString (avjd2_w);
	buildDateString (str, jd2, latest_str);
	XtFree (str);

	/* form designation string */
	idp = XmTextFieldGetString (avdes_w);
	for (str = idp; *str == ' '; str++)
	    continue;
	strcpy (id, str);
	XtFree(idp);
	for (str = &id[strlen(id)-1]; *str == ' '; --str)
	    *str = '\0';
	for (str = id; (str = strchr (str, ' ')) != NULL; str++)
	    *str = '+';

	/* get running average period */
	str = XmTextFieldGetString (avfit_w);
	fit = strtod (str, NULL);
	XtFree (str);

	/* gather options */
	for (i = 0; i < XtNumber(abopts); i++) {
	    ABOpt *ap = &abopts[i];
	    int set = XmToggleButtonGetState (ap->w);
	    abop[i] = set ? ap->tstr : ap->fstr;
	}

	/* query server */
	(void) sprintf (buf, fmt, avhost, avcgi, jd1, jd2, id, fit, abop[0],
					    abop[1], abop[2], abop[3], abop[4],
					    abop[5], abop[6], PATCHLEVEL);
	fd = httpGET (avhost, buf, buf);
	if (fd < 0) {
	    xe_msg (1, "Query: %s", buf);
	    stopd_down();
	    watch_cursor(0);
	    return (-1);
	}

	/* read header (everything to first blank line), looking for gif */
	isgif = 0;
	length = 0;
	while (recvline (fd, buf, sizeof(buf)) > 1) {
	    xe_msg (0, "Rcv: %s", buf);
	    if (strstr (buf, "Content-Type:") && strstr (buf, "image/gif"))
		isgif = 1;
	    if (strstr (buf, "Content-Length:"))
		length = atoi (buf+15);
	}
	if (!isgif) {
	    while (recvline (fd, buf, sizeof(buf)) > 0)
		xe_msg (0, "Rcv: %s", buf);
	    xe_msg (1, "Error talking to AAVSO .. see File->System log\n");
	    close (fd);
	    stopd_down();
	    watch_cursor(0);
	    return (-1);
	}
	if (length == 0) {
	    xe_msg (0, "No Content-Length in header");
	    length = sizeof(rawgif);	/* ?? */
	}

	/* remainder should be gif, read into rawgif[] */
	pm_up();
	for (nrawgif = 0; nrawgif < sizeof(rawgif); nrawgif += nr) {
	    pm_set (100*nrawgif/length);
	    nr = readbytes (fd, rawgif+nrawgif, 2048);
	    if (nr < 0) {
		xe_msg (1, "%s:\n%s", avhost, syserrstr());
		pm_down();
		close (fd);
		stopd_down();
		watch_cursor(0);
		return (-1);
	    }
	    if (nr == 0)
		break;
	}
	stopd_down();
	pm_down();
	close (fd);
	if (nr > 0) {
	    xe_msg (1, "File too large");
	    watch_cursor(0);
	    return (-1);
	}

	/* explode into pm */
	if (gif2pm (dsp, xe_cm, rawgif, nrawgif, &w, &h, &curve_pm, buf) < 0) {
	    xe_msg (1, "Gif error:\n%s", buf);
	    watch_cursor(0);
	    return (-1);
	}

	/* yes! */
	watch_cursor(0);
	return (0);
}

/* given a line from the avdb file strip of unwanted chars */
static void
cleanLine (line)
char *line;
{
	char *p;

	/* skip the designation.. it never needs cleaning */
	line += DESCOLS;

	/* remove \r \n and stuff after first + */
	if ((p = strchr (line, '\r')) != NULL)
	    *p = '\0';
	if ((p = strchr (line, '\n')) != NULL)
	    *p = '\0';
	if ((p = strchr (line, '+')) != NULL)
	    *p = '\0';
}

/* get the current selection and show its graph */
static void
fetchAndShow()
{
	Display *dsp = XtDisplay(toplevel_w);
	Pixmap oldpm;
	Arg args[20];
	int n;

	/* get graph in curve_pm */
	if (fetchpm() < 0)
	    return;

	/* display via avgrl_w label */
	get_something (avgrl_w, XmNlabelPixmap, (XtArgVal)&oldpm);
	if (oldpm != XmUNSPECIFIED_PIXMAP)
	    XFreePixmap (dsp, oldpm);
	n = 0;
	XtSetArg (args[n], XmNlabelType, XmPIXMAP); n++;
	XtSetArg (args[n], XmNlabelPixmap, curve_pm); n++;
	XtSetValues (avgrl_w, args, n);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: aavso.c,v $ $Date: 2010/10/06 21:12:16 $ $Revision: 1.21 $ $Name:  $"};

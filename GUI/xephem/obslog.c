/* provide a simple observer's log facility.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <fnmatch.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>

#include "lilxml.h"
#include "xephem.h"

typedef struct {
    int justlabel;			/* just a label not a field */
    char *label;			/* row title */
    char *xml;				/* xml tag, if different from label */
    Widget tfw;				/* TF for display/entry */
    Widget ktb;				/* TB whether to use as sort key */
    Widget ltb;				/* TB whether to list */
    Widget stb;				/* TB whether to use as search key */
    char *srchvalue;			/* fast access to search value */
    int colw;				/* col length */
} LogField;

/* definition of each log field.
 * N.B. must stay in sync with LogTags 
 */
static LogField logfields[] = {
    {1, "Automatically defined fields:"},
    {0, "Object"},
    {0, "Location"},
    {0, "Local Date", "Date"},
    {0, "Local Time", "Time"},

    {0, "UTC Date", "UTCDate"},
    {0, "UTC Time", "UTCTime"},
    {0, "JD", "JD"},
    {0, "H JD", "HJD"},
    {0, "Constel"},

    {0, "Latitude"},
    {0, "Longitude"},
    {0, "RA 2000", "RA2000"},
    {0, "Dec 2000", "Dec2000"},
    {0, "Altitude"},

    {0, "Azimuth"},
    {0, "Airmass"},
    {0, "Moon sep", "MoonSep"},
    {0, "Moon alt", "MoonAlt"},
    {0, "Moon phase", "MoonPhase"},

    {0, "Sun alt", "SunAlt"},
    {0, "Elongation"},

    {1, "User defined fields:"},
    {0, "Telescope"},
    {0, "Camera"},
    {0, "Eyepiece"},
    {0, "Filter"},

    {0, "Seeing"},
    {0, "Transparency"},
    {0, "User 1", "User1"},
    {0, "User 2", "User2"},
};

#define	NFLDS	XtNumber(logfields)

#define	TFCOL	20			/* n text field columns */
#define	TFROW	5			/* text field rows in notes field */

#define	KTBPOS	0			/* position % for sort key TB */
#define	LTBPOS	8			/* position % for list TB */
#define	STBPOS	16			/* position % for search TB */
#define	LPOS	24			/* position % for label */
#define	LTFPOS	55			/* position % of text field left side */

static int srchMatch (XMLEle *ep);
static int notesMatch (XMLEle *ep);
static int strscmp (char *s1, char *s2);
static void ol_create(void);
static void oll_create(void);
static void ol_key_cb (Widget w, XtPointer client, XtPointer call);
static void ol_help_cb (Widget w, XtPointer client, XtPointer call);
static void ol_close_cb (Widget w, XtPointer client, XtPointer call);
static void ol_list_cb (Widget w, XtPointer client, XtPointer call);
static void ol_add_cb (Widget w, XtPointer client, XtPointer call);
static void ol1row (Widget rc_w, LogField *lp);
static void fillNowFields (void);
static void fillObjFields (Obj *op);
static void ol_writeLB(void);
static void ol_listLB(void);
static char *xmltag (LogField *lfp);
static Widget logfld (char *tag);
static void printHeadings(XMLEle **mpp, int nmpp, char **txt, int *txtl);
static void printEntry (XMLEle *ep, char **txt, int *txtl);
static void sortMatches (XMLEle **mpp, int nmpp);
static void oll_save_cb (Widget w, XtPointer client, XtPointer call);
static void oll_save(void);
static void oll_close_cb (Widget w, XtPointer client, XtPointer call);
static void oll_info_eh (Widget w, XtPointer client, XEvent *ev, Boolean *ctd);

static Widget olshell_w;		/* main shell */
static Widget ollshell_w;		/* list shell */
static Widget ollst_w;			/* scrolled list */
static Widget ollstf_w;			/* save text field */
static Widget notes_w;			/* notes Text */
static Widget notesltb_w;		/* notes list TB */
static Widget notesstb_w;		/* notes search TB */

static XMLEle **matches;		/* malloced list of XMLEle* in list */
static int nmatches;			/* number in list */
static XMLEle *matchesroot;		/* root of entries in matches[] */

static char lbfn[] = "xelogbook.xml";	/* logbook file name */
static char lbtag[] = "xelogbook";	/* outter xml tag for logbook */
static char lbetag[] = "logentry";	/* tag for each entry */
static char ntag[] = "Notes";		/* tag for notes */
static double lbver = 1.1;		/* logbook version attr */

static char olcategory[] = "Logbook";	/* save category */

/* bring up observer's log, create if first time */
void
ol_manage()
{
	if (!olshell_w) {
	    ol_create();
	    oll_create();
	}

	XtPopup (olshell_w, XtGrabNone);
	set_something (olshell_w, XmNiconic, (XtArgVal)False);
}

/* return whether logbook is currently showing */
int
ol_isUp()
{
	return(isUp(olshell_w));
}

/* fill log fields from given object, and Now */
void
ol_setObj (Obj *op)
{
	fillNowFields();
	fillObjFields(op);
}

/* called to put up or remove the watch cursor.  */
void
ol_cursor (Cursor c)
{
	Window win;

	if (olshell_w && (win = XtWindow(olshell_w)) != 0) {
	    Display *dsp = XtDisplay(olshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create the observer log window */
static void
ol_create()
{
	Widget w, sep_w;
	Widget rc_w, fo_w;
	Arg args[20];
	int i, n;

	/* create the shell and form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Observer's log"); n++;
	XtSetArg (args[n], XmNiconName, "Logbook"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	olshell_w = XtCreatePopupShell ("Logbook", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (olshell_w);
	set_something (olshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (olshell_w, "XEphem*Logbook.x", olcategory, 0);
	sr_reg (olshell_w, "XEphem*Logbook.y", olcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNfractionBase, 17); n++;
	fo_w = XmCreateForm (olshell_w, "LF", args, n);
	XtAddCallback (fo_w, XmNhelpCallback, ol_help_cb, 0);
	XtManageChild (fo_w);

	/* controls attached across the bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 1); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 4); n++;
	w = XmCreatePushButton (fo_w, "Close", args, n);
	wtip (w, "Close this window");
	XtAddCallback (w, XmNactivateCallback, ol_close_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 5); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 8); n++;
	w = XmCreatePushButton (fo_w, "List", args, n);
	wtip (w, "List fields L of all logbook entries matching requirements selected with < > *, sorted by K");
	XtAddCallback (w, XmNactivateCallback, ol_list_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 9); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 12); n++;
	w = XmCreatePushButton (fo_w, "Add", args, n);
	wtip (w, "Add above fields as a new entry in the log book");
	XtAddCallback (w, XmNactivateCallback, ol_add_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 13); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 16); n++;
	w = XmCreatePushButton (fo_w, "Help", args, n);
	wtip (w, "Get more info about this window.");
	XtAddCallback (w, XmNactivateCallback, ol_help_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (fo_w, "LS", args, n);
	XtManageChild (sep_w);

	/* rc for the tags */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	rc_w = XmCreateRowColumn (fo_w, "LRC", args, n);
	XtManageChild (rc_w);

	    for (i = 0; i < NFLDS; i++)
		ol1row (rc_w, &logfields[i]);

	/* notes entry expands into remaining area between sep and rc */

	n =0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	w = XmCreateLabel (fo_w, "Notes", args, n);
	set_xmstring (w, XmNlabelString, "Notes:");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, w); n++;
	XtSetArg (args[n], XmNleftOffset, 5); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNmarginTop, 0); n++;
	XtSetArg (args[n], XmNmarginBottom, 0); n++;
	XtSetArg (args[n], XmNmarginWidth, 0); n++;
	XtSetArg (args[n], XmNmarginLeft, 0); n++;
	XtSetArg (args[n], XmNmarginRight, 0); n++;
	notesltb_w = XmCreateToggleButton (fo_w, "L", args, n);
	wtip (notesltb_w, "Include Notes in list");
	XtManageChild (notesltb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, notesltb_w); n++;
	XtSetArg (args[n], XmNleftOffset, 5); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNmarginTop, 0); n++;
	XtSetArg (args[n], XmNmarginBottom, 0); n++;
	XtSetArg (args[n], XmNmarginWidth, 0); n++;
	XtSetArg (args[n], XmNmarginLeft, 0); n++;
	XtSetArg (args[n], XmNmarginRight, 0); n++;
	notesstb_w = XmCreateToggleButton (fo_w, "S", args, n);
	wtip (notesstb_w, "Match notes against given glob pattern");
	XtManageChild (notesstb_w);

	n =0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, notesltb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, sep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg (args[n], XmNeditable, True); n++;
	XtSetArg (args[n], XmNrows, TFROW); n++;
	notes_w = XmCreateScrolledText (fo_w, "Value", args, n);
	XtManageChild (notes_w);
}

/* build up one row */
static void
ol1row (Widget rc_w, LogField *lp)
{
	Widget w, fo_w;
	Arg args[20];
	int n;

	/* main form for this row */
	n = 0;
	fo_w = XmCreateForm (rc_w, "LRF", args, n);
	XtManageChild (fo_w);

	/* sort key TB, unless just label */
	if (!lp->justlabel) {
	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, KTBPOS); n++;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNmarginTop, 0); n++;
	    XtSetArg (args[n], XmNmarginBottom, 0); n++;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    XtSetArg (args[n], XmNmarginLeft, 0); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    XtSetArg (args[n], XmNmarginRight, 0); n++;
	    w = XmCreateToggleButton (fo_w, "KTB", args, n);
	    XtAddCallback (w, XmNvalueChangedCallback, ol_key_cb, NULL);
	    set_xmstring (w, XmNlabelString, " ");
	    XtManageChild (w);
	    lp->ktb = w;
	}

	/* list TB, unless just label */
	if (!lp->justlabel) {
	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, LTBPOS); n++;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNmarginTop, 0); n++;
	    XtSetArg (args[n], XmNmarginBottom, 0); n++;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    XtSetArg (args[n], XmNmarginLeft, 0); n++;
	    XtSetArg (args[n], XmNmarginRight, 0); n++;
	    w = XmCreateToggleButton (fo_w, xmltag(lp), args, n);
	    set_xmstring (w, XmNlabelString, " ");
	    XtManageChild (w);
	    lp->ltb = w;
	    sr_reg (w, NULL, olcategory, 1);
	}

	/* search TB, if desired */
	if (!lp->justlabel) {
	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, STBPOS); n++;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNmarginTop, 0); n++;
	    XtSetArg (args[n], XmNmarginBottom, 0); n++;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    XtSetArg (args[n], XmNmarginLeft, 0); n++;
	    XtSetArg (args[n], XmNmarginRight, 0); n++;
	    w = XmCreateToggleButton (fo_w, "WTB", args, n);
	    set_xmstring (w, XmNlabelString, " ");
	    XtManageChild (w);
	    lp->stb = w;
	}

	/* field name */
	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, lp->justlabel ? 0 : LPOS); n++;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	w = XmCreateLabel (fo_w, "LRL", args, n);
	set_xmstring (w, XmNlabelString, lp->label);
	XtManageChild (w);

	if (lp->justlabel) {
	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (fo_w, "K", args, n);
	    wtip (w, "Select one field as the sort key for listings");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, LTBPOS); n++;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (fo_w, "L", args, n);
	    wtip (w, "Select the fields to be listed in a search result.");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, STBPOS); n++;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (fo_w, "S", args, n);
	    set_xmstring (w, XmNlabelString, "S");
	    wtip (w, "Select the fields to match for searching. Prefix numerical fields with < or >, other fields can use glob patterns");
	    XtManageChild (w);

	}

	/* text field, unless just a label */
	if (!lp->justlabel) {
	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, LTFPOS); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginTop, 0); n++;
	    XtSetArg (args[n], XmNmarginBottom, 0); n++;
	    XtSetArg (args[n], XmNcolumns, TFCOL); n++;
	    XtSetArg (args[n], XmNeditable, True); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, True); n++;
	    w = XmCreateTextField (fo_w, "LRTF", args, n);
	    fixTextCursor(w);
	    XtManageChild (w);
	    lp->tfw = w;
	}
}

/* called when a K key toggles */
/* ARGSUSED */
static void
ol_key_cb (Widget w, XtPointer client, XtPointer call)
{
	int i;

	/* do nothing when turning off */
	if (!XmToggleButtonGetState(w))
	    return;

	/* turn all others off, and turn on list for ths entry as a convenice */
	for (i = 0; i < NFLDS; i++) {
	    LogField *lfp = &logfields[i];
	    if (lfp->ktb) {
		if (lfp->ktb == w)
		    XmToggleButtonSetState (lfp->ltb, True, True);
		else
		    XmToggleButtonSetState (lfp->ktb, False, True);
	    }
	}
}

/* callback from the overall Help button.
 */
/* ARGSUSED */
static void
ol_help_cb (Widget w, XtPointer client, XtPointer call)
{
	static char *msg[] = {
	    "Save observing notes",
	};

	hlp_dialog ("ObsLog", msg, sizeof(msg)/sizeof(msg[0]));
}

/* callback from the Close PB */
/* ARGSUSED */
static void
ol_close_cb (Widget w, XtPointer client, XtPointer call)
{
	XtPopdown (olshell_w);
}   

/* callback from the List PB */
/* ARGSUSED */
static void
ol_list_cb (Widget w, XtPointer client, XtPointer call)
{
	/* fill in results */
	ol_listLB();

	/* display */
	XtPopup (ollshell_w, XtGrabNone);
	set_something (ollshell_w, XmNiconic, (XtArgVal)False);
}   

/* callback from the Add PB */
/* ARGSUSED */
static void
ol_add_cb (Widget w, XtPointer client, XtPointer call)
{
	ol_writeLB();
}   

/* set tags from Now */
static void
fillNowFields()
{
	Now *np = mm_get_now();
	double lmjd = mjd - tz/24.0;
	char buf[32];

	fs_date (buf, pref_get(PREF_DATE_FORMAT), mjd_day(lmjd));
	XmTextFieldSetString (logfld("Date"), buf);
	fs_time (buf, mjd_hr(lmjd));
	XmTextFieldSetString (logfld("Time"), buf);
	fs_date (buf, pref_get(PREF_DATE_FORMAT), mjd_day(mjd));
	XmTextFieldSetString (logfld("UTCDate"), buf);
	fs_time (buf, mjd_hr(mjd));
	XmTextFieldSetString (logfld("UTCTime"), buf);
	sprintf (buf, "%13.5f", mjd+MJD0);
	XmTextFieldSetString (logfld("JD"), buf);
	fs_dms_angle (buf, lat);
	XmTextFieldSetString (logfld("Latitude"), buf);
	fs_dms_angle (buf, -lng);       /* + west */
	XmTextFieldSetString (logfld("Longitude"), buf);
}

/* set tags from Obj op */
static void
fillObjFields (Obj *op)
{
	Now *np = mm_get_now();
	double e = epoch == EOD ? mjd : epoch;
	double ra, dec, sep;
	double hdiff, am;
	Obj *fop;
	char buf[32], *bp;

	XmTextFieldSetString (logfld("Object"), op->o_name);
	if ((bp = mm_getsite()) != NULL)
	    XmTextFieldSetString (logfld("Location"),  bp);

	/* TODO check for J2000 */


	bp = cns_name (cns_pick (op->s_ra, op->s_dec, e));
	bp += 5;       /* skip the abbreviation */
	XmTextFieldSetString (logfld("Constel"), bp);

	ra = op->s_ra;
	dec = op->s_dec;
	if (epoch == EOD)
	    ap_as (np, e, &ra, &dec);
	precess (e, J2000, &ra, &dec);

	fs_sexa (buf, radhr(ra), 4, 36000);
	XmTextFieldSetString (logfld("RA2000"), buf);
	fs_sexa (buf, raddeg(dec), 4, 3600);
	XmTextFieldSetString (logfld("Dec2000"), buf);

	fs_sexa (buf, raddeg(op->s_alt), 4, 3600);
	XmTextFieldSetString (logfld("Altitude"), buf);
	fs_sexa (buf, raddeg(op->s_az), 4, 3600);
	XmTextFieldSetString (logfld("Azimuth"), buf);

	heliocorr (mjd+MJD0, ra, dec, &hdiff);
	sprintf (buf, "%13.5f", mjd+MJD0 - hdiff);
	XmTextFieldSetString (logfld("HJD"), buf);

	airmass (op->s_alt, &am);
	sprintf (buf, "%.4f", am);
	XmTextFieldSetString (logfld("Airmass"), buf);

	fop = db_basic (MOON);
	dm_separation (op, fop, &sep);
	fs_sexa (buf, raddeg(sep), 4, 3600);
	XmTextFieldSetString (logfld("MoonSep"), buf);
	fs_sexa (buf, raddeg(fop->s_alt), 4, 3600);
	XmTextFieldSetString (logfld("MoonAlt"), buf);
	sprintf (buf, "%10.1f", fop->s_phase);
	XmTextFieldSetString (logfld("MoonPhase"), buf);

	fop = db_basic (SUN);
	fs_sexa (buf, raddeg(fop->s_alt), 4, 3600);
	XmTextFieldSetString (logfld("SunAlt"), buf);
	if (op->o_type != EARTHSAT) {
	    sprintf (buf, "%10.1f", op->s_elong);
	    XmTextFieldSetString (logfld("Elongation"), buf);
	} else
	    XmTextFieldSetString (logfld("Elongation"), " ");
}

/* read the existing log book and list all entries matching the selected fields.
 */
static void
ol_listLB()
{
	static LilXML *lp;
	XMLEle *root, *ep;
	char *txt = NULL;
	int txtl = 0;
	char fn[512];
	char buf[2048];
	FILE *fp;
	int i;

	/* open and read in the xml log */
	sprintf (fn, "%s/%s", getPrivateDir(), lbfn);
	fp = fopenh (fn, "r");
	if (!fp) {
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	    return;
	}
	if (!lp)
	    lp = newLilXML();
	root = readXMLFile (fp, lp, buf);
	fclose (fp);
	if (!root) {
	    xe_msg (1, "%s:\n%s", fn, buf);
	    return;
	}
	if (strcmp (tagXMLEle(root), lbtag)) {
	    delXMLEle (root);
	    xe_msg (1, "%s:\nNot an XEphem log file", fn);
	    return;
	}

	/* collect values from the desired search keys */
	for (i = 0; i < NFLDS; i++) {
	    LogField *lfp = &logfields[i];
	    if (!lfp->justlabel && XmToggleButtonGetState(lfp->stb))
		lfp->srchvalue = XmTextFieldGetString (lfp->tfw); /* XtFree! */
	    else
		lfp->srchvalue = NULL;
	}

	/* scan each entry to create new matches list */
	if (matches) {
	    delXMLEle (matchesroot);
	    XtFree ((char*)matches);
	}
	matches = NULL;
	nmatches = 0;
	matchesroot = root;
	for (ep = nextXMLEle (root,1); ep != NULL; ep = nextXMLEle(root,0)) {
	    if (srchMatch(ep)) {
		matches = (XMLEle **) XtRealloc ((char*)matches,
					    (nmatches+1)*sizeof(XMLEle *));
		matches[nmatches++] = ep;
	    }
	}

	/* sort */
	sortMatches (matches, nmatches);

	/* print headings and figure column widths */
	printHeadings (matches, nmatches, &txt, &txtl);

	/* print each entry */
	for (i = 0; i < nmatches; i++)
	    printEntry (matches[i], &txt, &txtl);

	/* display */
	XmTextSetString (ollst_w, txt);

	/* clean up */
	for (i = 0; i < NFLDS; i++)
	    if (logfields[i].srchvalue)
		XtFree (logfields[i].srchvalue);
	XtFree (txt);
}

/* append sl chars of src to *dstp, growing *dstp as required.
 * update *dstp and return new final length, sans trailing '\0'.
 */
static int
mstrcat (char **dstp, int dl, char *src, int sl)
{
	int newl = dl+sl;
	char *newdst = XtRealloc (*dstp, newl+1);
	strncpy (newdst+dl, src, sl);
	newdst[newl] = '\0';
	*dstp = newdst;
	return (newl);
}

/* print list header.
 * N.B. number of lines added here must agree with num removed in oll_info_eh()
 */
static void
printHeadings(XMLEle **mpp, int nmpp, char **txt, int *txtl)
{
	static char dashes[] = "----------------------------------------------";
	char buf[1024];
	time_t t;
	int i, j, l;

	/* init colw to label length as a min width */
	for (i = 0; i < NFLDS; i++)
	    logfields[i].colw = strlen(logfields[i].label);

	/* one pass to find max col length */
	for (i = 0; i < nmpp; i++) {
	    XMLEle *ep, *e = mpp[i];
	    for (j = 0; j < NFLDS; j++) {
		LogField *lfp = &logfields[j];
		if (!lfp->justlabel && XmToggleButtonGetState(lfp->ltb)) {
		    for (ep=nextXMLEle(e,1); ep!=NULL; ep=nextXMLEle(e,0)) {
			if (!strcmp (xmltag(lfp), tagXMLEle(ep))) {
			    int l = strlen (pcdataXMLEle(ep));
			    if (l > lfp->colw)
				lfp->colw = l;
			}
		    }
		}
	    }
	}

	/* print col heading */
	time (&t);
	l = sprintf (buf, "XEphem %s Observers Log Book printed %s\n",
							PATCHLEVEL, ctime(&t));
	*txtl = mstrcat (txt, *txtl, buf, l);
	for (i = 0; i < NFLDS; i++) {
	    LogField *lfp = &logfields[i];
	    if (!lfp->justlabel && XmToggleButtonGetState(lfp->ltb)) {
		l = sprintf (buf, "%-*.*s ", lfp->colw, lfp->colw, lfp->label);
		*txtl = mstrcat (txt, *txtl, buf, l);
	    }
	}
	*txtl = mstrcat (txt, *txtl, "\n", 1);
	for (i = 0; i < NFLDS; i++) {
	    LogField *lfp = &logfields[i];
	    if (!lfp->justlabel && XmToggleButtonGetState(lfp->ltb)) {
		l = sprintf (buf, "%-*.*s ", lfp->colw, lfp->colw, dashes);
		*txtl = mstrcat (txt, *txtl, buf, l);
	    }
	}
	*txtl = mstrcat (txt, *txtl, "\n", 1);
}

/* add one entry from ep to txt */
static void
printEntry (XMLEle *ep, char **txt, int *txtl)
{
	XMLEle *e, *notes = NULL;
	int i;

	/* show each selected list field, but just find notes for later */
	for (e = nextXMLEle(ep,1); e != NULL; e = nextXMLEle(ep,0)) {
	    if (!strcmp (ntag, tagXMLEle(e))) {
		notes = e;
		continue;
	    }
	    for (i = 0; i < NFLDS; i++) {
		LogField *lfp = &logfields[i];
		if (!lfp->justlabel && !strcmp (xmltag(lfp), tagXMLEle(e))
					&& XmToggleButtonGetState(lfp->ltb)) {
		    char buf[1024];
		    int l = sprintf (buf, "%-*.*s ", lfp->colw, lfp->colw,
							    pcdataXMLEle(e));
		    *txtl = mstrcat (txt, *txtl, buf, l);
		}
	    }
	}
	*txtl = mstrcat (txt, *txtl, "\n", 1);

	/* append notes if exists and wanted */
	if (XmToggleButtonGetState (notesltb_w) && notes &&
						pcdatalenXMLEle(notes) > 0) {
	    /* preceed each line with indent and # */
	    int l = pcdatalenXMLEle(notes);
	    char *np = pcdataXMLEle(notes);
	    *txtl = mstrcat (txt, *txtl, "  # ", 4);
	    for (i = 0; i < l; i++) {
		*txtl = mstrcat (txt, *txtl, &np[i], 1);
		if (i < l-1 && np[i] == '\n')
		    *txtl = mstrcat (txt, *txtl, "  # ", 4);
	    }
	    if (np[l-1] != '\n')
		*txtl = mstrcat (txt, *txtl, "\n", 1);
	}
}

/* return 1 if the given logentry satisfies all the search keys else 0 */
static int
srchMatch (XMLEle *ep)
{
	XMLEle *eli;
	int i;

	for (eli = nextXMLEle (ep,1); eli != NULL; eli = nextXMLEle(ep,0))
	    for (i = 0; i < NFLDS; i++)
		if (logfields[i].srchvalue &&
			    !strcmp (tagXMLEle(eli), xmltag(&logfields[i])) &&
		    	    !strscmp(logfields[i].srchvalue, pcdataXMLEle(eli)))
		    return (0);

	/* all ok so far, now depends on notes if using it */
	if (XmToggleButtonGetState(notesstb_w))
	    return (notesMatch (ep));

	return (1);
}

/* return 1 if the notes field and ep match else 0 */
static int
notesMatch (XMLEle *ep)
{
	char *pattern, *string;
	XMLEle *nep;
	int flags, ok;

	/* find notes entry, fetch pattern and string */
	nep = findXMLEle (ep, ntag);
	if (!nep)
	    return (0);
	string = pcdataXMLEle (nep);
	pattern = XmTextGetString (notes_w);

	/* test */
	flags = 0;
#if defined(FNM_CASEFOLD)	/* GNU extension only */
	flags |= FNM_CASEFOLD;
#endif
	ok = !fnmatch (pattern, string, flags);

	/* clean up and return */
	XtFree (pattern);
	return (ok);
}

/* add the current logbook settings to logbook in the private dir */
static void
ol_writeLB()
{
	FILE *fp;
	char buf[1024];
	char cltag[sizeof(lbtag)+10];
	char *str;
	int i;

	/* need closing tag */
	sprintf (cltag, "</%s>\n", lbtag);

	/* open or create */
	sprintf (buf, "%s/%s", getPrivateDir(), lbfn);
	fp = fopenh (buf, "r+");
	if (fp) {
	    /* found existing log, skip to just before closing tag */
	    long offset = ftell (fp);
	    while (fgets (buf, sizeof(buf), fp)) {
		if (strcmp (buf, cltag) == 0)
		    break;
		offset = ftell (fp);
	    }
	    if (feof(fp) || ferror(fp)) {
		xe_msg (1, "Can not find %s", cltag);
		return;
	    }
	    fseek (fp, offset, SEEK_SET);
	} else {
	    /* no log, create */
	    if (errno != ENOENT || !(fp = fopenh (buf, "w"))) {
		xe_msg (1, "%s:\n %s", buf, syserrstr());
		return;
	    }
	    fprintf (fp, "<?xml version='1.0'?>\n");
	    fprintf (fp, "<%s version='%g'>\n", lbtag, lbver);
	}

	/* write new entry, notes are special */
	fprintf (fp, "    <%s>\n", lbetag);
	for (i = 0; i < NFLDS; i++) {
	    LogField *lfp = &logfields[i];
	    if (lfp->justlabel)
		continue;
	    str = XmTextFieldGetString (lfp->tfw);
	    fprintf (fp, "        <%s>%s</%s>\n", xmltag(lfp),str,xmltag(lfp));
	    XtFree (str);
	}
	str = XmTextGetString (notes_w);
	fprintf (fp, "        <%s>\n%s\n        </%s>\n", ntag, str, ntag);
	XtFree (str);
	fprintf (fp, "    </%s>\n", lbetag);

	/* finished */
	fprintf (fp, "%s", cltag);
	xe_msg (1, "Added new log entry");
	fclose (fp);
}

/* return str as a double, noting especially dates and sexagesimal angles
 */
static double
numeric (char *str)
{
	char *lslash = strchr (str, '/');
	char *rslash = strrchr (str, '/');

	if (lslash && rslash && lslash != rslash) {
	    /* 2 distinct /'s so figure it's a date in user's preferred format*/
	    double d; int m, y;
	    f_sscandate (str, pref_get(PREF_DATE_FORMAT), &m, &d, &y);
	    return (y + (m*32 + d)/416);	/* ordinal */
	} else {
	    double v;
	    f_scansexa (str, &v);
	    return (v);
	}
}

/* return whether str matches pattern.
 * if pattern starts with < or > treat things numerically, else treat as
 * a wildcard match.
 */
static int
strscmp (char *pattern, char *str)
{
	int flags;

	if (pattern[0] == '<')
	    return (numeric (str) < numeric (pattern+1));
	if (pattern[0] == '>')
	    return (numeric (str) > numeric (pattern+1));

	flags = 0;
#if defined(FNM_CASEFOLD)	/* GNU extension only */
	flags |= FNM_CASEFOLD;
#endif
	return (!fnmatch (pattern, str, flags));
}

/* return xml tag for the given logfield */
static char *
xmltag (LogField *lfp)
{
	return (lfp->xml ? lfp->xml : lfp->label);
}

/* find a TextField for a field given its xml tag */
static Widget
logfld (char *tag)
{
	int i;

	for (i = 0; i < NFLDS; i++)
	    if (strcmp (tag, xmltag(&logfields[i])) == 0)
		return (logfields[i].tfw);

	printf ("Obslog bug: no such tag: %s\n", tag);
	abort();
}

static char *srchtag;

static int
cmp_qsort (const void *v1, const void *v2)
{
	XMLEle *ep1 = *((XMLEle **)v1);
	XMLEle *ep2 = *((XMLEle **)v2);
	char *sv1 = NULL, *sv2 = NULL;
	XMLEle *ep;

	for (ep = nextXMLEle (ep1,1); ep != NULL; ep = nextXMLEle(ep1,0)) {
	    if (!strcmp (srchtag, tagXMLEle(ep))) {
		sv1 = pcdataXMLEle (ep);
		break;
	    }
	}

	for (ep = nextXMLEle (ep2,1); ep != NULL; ep = nextXMLEle(ep2,0)) {
	    if (!strcmp (srchtag, tagXMLEle(ep))) {
		sv2 = pcdataXMLEle (ep);
		break;
	    }
	}

	if (!sv1)
	    return (-1);
	if (!sv2)
	    return (1);
	return (strnncmp (sv1, sv2));
}

/* sort the array of XML log file elements by the selected key */
static void
sortMatches (XMLEle **mpp, int nmpp)
{
	int (*sortfp) (const void *v1, const void *v2);
	int i;

	/* find the key, use Object if none set */
	srchtag = NULL;
	for (i = 0; i < NFLDS; i++) {
	    LogField *lfp = &logfields[i];
	    if (lfp->ktb && XmToggleButtonGetState(lfp->ktb)) {
		srchtag = xmltag (lfp);
		break;
	    }
	}
	if (!srchtag) {
	    /* use Object */
	    for (i = 0; i < NFLDS; i++) {
		LogField *lfp = &logfields[i];
		if (!strcmp (xmltag(lfp), "Object")) {
		    srchtag = xmltag (lfp);
		    break;
		}
	    }
	}

	/* sort by the key */
	sortfp = cmp_qsort;
	qsort (mpp, nmpp, sizeof(XMLEle*), sortfp);
}

/* create the observer log list window */
static void
oll_create()
{
	Widget w, fo_w;
	Arg args[20];
	int n;

	/* create the shell and form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Observer's log list"); n++;
	XtSetArg (args[n], XmNiconName, "Log list"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	ollshell_w = XtCreatePopupShell ("LogList", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (ollshell_w);
	set_something (olshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (ollshell_w, "XEphem*LogList.x", olcategory, 0);
	sr_reg (ollshell_w, "XEphem*LogList.y", olcategory, 0);
	sr_reg (ollshell_w, "XEphem*LogList.width", olcategory, 0);
	sr_reg (ollshell_w, "XEphem*LogList.height", olcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	fo_w = XmCreateForm (ollshell_w, "LL", args, n);
	XtManageChild (fo_w);

	/* controls attached across the bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 10); n++;
	w = XmCreatePushButton (fo_w, "OLS", args, n);
	set_xmstring (w, XmNlabelString, "Save:");
	XtAddCallback (w, XmNactivateCallback, oll_save_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, w); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 75); n++;
	ollstf_w = XmCreateTextField (fo_w, "filename", args, n);
	XtAddCallback (ollstf_w, XmNactivateCallback, oll_save_cb, NULL);
	XtManageChild (ollstf_w);
	sr_reg (ollstf_w, NULL, olcategory, 1);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 90); n++;
	w = XmCreatePushButton (fo_w, "OLS", args, n);
	set_xmstring (w, XmNlabelString, "Close");
	XtAddCallback (w, XmNactivateCallback, oll_close_cb, NULL);
	XtManageChild (w);

	/* scrolled text */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, ollstf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrows, 24); n++;
	XtSetArg (args[n], XmNcolumns, 82); n++;
	XtSetArg (args[n], XmNeditable, False); n++;
	XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	XtSetArg (args[n], XmNblinkRate, 0); n++;
	ollst_w = XmCreateScrolledText (fo_w, "OLST", args, n);
	XtAddEventHandler (ollst_w, ButtonPressMask, False, oll_info_eh, 0);
	XtManageChild (ollst_w);
}

/* save the list text in file named in ollstf_w
 * N.B. don't use call, this is called by a PB and a TF
 */
/* ARGSUSED */
static void
oll_save_cb (Widget w, XtPointer client, XtPointer call)
{
	char *fn;

	/* get fn */
	fn = XmTextFieldGetString (ollstf_w);
	if (strlen (fn) == 0) {
	    xe_msg (1, "Please enter a file name");
	    XtFree (fn);
	    return;
	}

	/* go, perhaps cautiously */
	if (existd (fn,NULL) == 0 && confirm()) {
	    char buf[1024];
	    (void) sprintf (buf, "%s exists:\nOverwrite?", fn);
	    query (ollshell_w, buf, "Yes - overwrite", "No - cancel", NULL,
							oll_save, NULL, NULL);
	} else
	    oll_save();	/* just hammer it */

	/* done with name */
	XtFree (fn);
}

/* write ollst_w to file named in ollstf_w */
static void
oll_save()
{
	char *fn = XmTextFieldGetString (ollstf_w);
	FILE *fp = fopend (fn, NULL, "w");
	char *txt = XmTextGetString (ollst_w);
	fprintf (fp, "%s", txt);
	if (ferror(fp))
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	else
	    xe_msg (1, "Wrote %s", fn);
	fclose (fp);
	XtFree (txt);
	XtFree (fn);
}

/* callback from the Close PB in the list window */
/* ARGSUSED */
static void
oll_close_cb (Widget w, XtPointer client, XtPointer call)
{
	XtPopdown (ollshell_w);
}   

/* extra ButtonPress handler added to scrolled text.
 * used to provide few more options based on text under pointer.
 * N.B. header offset must agree with printHeadings()
 */
static void
oll_info_eh (Widget w, XtPointer client, XEvent *ev, Boolean *c_to_dispatch)
{
	XMLEle *me, *ep;
	char *txt, *rastr, *decstr;
	double ra, dec;
	Obj o;
	int pos;
	int r;

	/* only care about button3 press */
	if (!(ev->type == ButtonPress && ev->xbutton.button == Button3))
	    return;

	/* TODO accommodate notes */
	if (XmToggleButtonGetState(notesltb_w))
	    return;

	/* where are we? */
	pos = XmTextXYToPos (w, ev->xbutton.x, ev->xbutton.y);
	if (pos < 0)
	    return;

	/* find row number */
	txt = XmTextGetString (w);
	for (r = 0; pos > 0; --pos)
	    if (txt[pos] == '\n')
		r++;
	XtFree (txt);

	/* find match, account for header */
	r -= 4;
	if (r < 0 || r >= nmatches)
	    return;
	me = matches[r];

	/* pull out ra/dec, build Obj for mark */
	ep = findXMLEle (me, "RA2000");
	if (!ep)
	    return;
	rastr = pcdataXMLEle (ep);
	ep = findXMLEle (me, "Dec2000");
	if (!ep)
	    return;
	decstr = pcdataXMLEle (ep);
	f_scansexa (rastr, &ra);
	memset (&o, 0, sizeof(o));
	o.o_type = FIXED;
	o.f_RA = (float)hrrad(ra);
	f_scansexa (decstr, &dec);
	o.f_dec = (float)degrad(dec);
	o.f_epoch = J2000;

	obj_cir (mm_get_now(), &o);
	sv_point(&o);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: obslog.c,v $ $Date: 2015/08/09 21:47:00 $ $Revision: 1.24 $ $Name:  $"};

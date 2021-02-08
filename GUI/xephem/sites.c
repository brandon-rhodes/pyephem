/* code to read and manipulate the shared and private site files.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <fnmatch.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/SelectioB.h>
#include <Xm/List.h>
#include <Xm/TextF.h>

#include "xephem.h"

extern char maincategory[];

static void read_files (void);
static int sites_cmpf (const void * v1, const void * v2);
static void create_sq_w (void);
static void fill_list (void);
static void buildNew (Widget w);
static void buildDST (Widget mb_w, char *prompt, Widget *m_w, Widget *w_w,
    Widget *d_w, Widget *at_w);
static void addButtons (Widget pd_w, char **names, int nnames);
static void setMenuHistory (Widget w, int i);
static void read_file (FILE *fp);
static void scroll_sites (int i);
static void sq_set_cb (Widget w, XtPointer client, XtPointer call);
static void sq_search_cb (Widget w, XtPointer client, XtPointer call);
static void sq_dblclick_cb (Widget w, XtPointer client, XtPointer call);
static void sq_click_cb (Widget w, XtPointer client, XtPointer call);
static void sq_help_cb (Widget w, XtPointer client, XtPointer call);
static void sq_cancel_cb (Widget w, XtPointer client, XtPointer call);
static void sq_create_cb (Widget w, XtPointer client, XtPointer call);
static void sq_save_cb (Widget w, XtPointer client, XtPointer call);
static void sq_setmain_cb (Widget w, XtPointer client, XtPointer call);
static int buildNewSite (Site *sp);
static char *nows (char *bp);
static char *getTF (Widget w, char buf[], int bl);
static int getPDmH (Widget pd);
static Site *moreSites(void);

static Widget sq_w;		/* sites query form dialog */
static Widget sql_w;		/* sites query scrolled list */
static Widget srtf_w;		/* search text field */
static Widget settf_w;		/* set text field */
static Widget newf_w;		/* form to un/manage to show create */
static Widget nsn_w;		/* new site name TF*/
static Widget nlt_w;		/* new site lat TF */
static Widget nlg_w;		/* new site long TF */
static Widget nel_w;		/* new site elev TF */
static Widget nzn_w;		/* new site tz name TF */
static Widget nzo_w;		/* new site tz offset TF */
static Widget ndn_w;		/* new site dst name */
static Widget ndo_w;		/* new site dst offset TF */
static Widget dbm_w;		/* dst beg month pulldown */
static Widget dbw_w;		/* dst beg week pulldown */
static Widget dbd_w;		/* dst beg day-of-week pulldown */
static Widget dbat_w;		/* dst beg time TF */
static Widget dem_w;		/* dst end month pulldown */
static Widget dew_w;		/* dst end week pulldown */
static Widget ded_w;		/* dst end day-of-week pulldown */
static Widget deat_w;		/* dst end time TF */

#define	MAXSITELLEN	512	/* maximum site file line length */
static Site *sites;		/* malloced list of Sites */
static int nsites;		/* number of entries in sites array */
static char sfn[] = "xephem_sites";/* xephem site file */

/* strings in dst begin/end pulldown
 * N.B. we don't use the string values, we presume to use children[] index
 */
static char *months[] = {
   "January  ", "February ", "March    ", "April    ", "May      ", "June     ",
   "July     ", "August   ", "September", "October  ", "November ", "December "
};
static char *weeks[] = {
    "1", "2", "3", "4", "Last"
};
static char *dow[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static char twoam[] = "  2:00:00";


/* let user choose a site from a scrolled list */
void
sites_manage()
{
	if (!sites)
	    read_files();
	if (!sq_w) {
	    create_sq_w();
	    fill_list();
	}

	XtManageChild (sq_w);
}

/* give caller our list of sites, if wanted, but always return count */
int
sites_get_list (Site **sipp)
{
	if (!sites)
	    read_files();
	if (sipp)
	    *sipp = sites;
	return (nsites);
}

/* search the sites list for one containing glob p.
 * start after the current selection in the list, if any.
 * return index into sites[] if find, else return -1.
 */
int
sites_search (char *p)
{
	int *poslist, poscount;
	int startpos;
	int flags;
	int i, n;

	/* read files if first time */
	if (!sites)
	    read_files();

	/* decide where to start - resume if gui up, else just at front */
	if (sql_w && XmListGetSelectedPos (sql_w, &poslist, &poscount)==True) {
	    startpos = poslist[0]%nsites;	/* 1-based so already +1 */
	    XtFree ((char *)poslist);
	} else
	    startpos = 0;

	/* check each possible offset location starting at startpos.
	 */
	flags = 0;
#if defined(FNM_CASEFOLD)	/* GNU extension only */
	flags |= FNM_CASEFOLD;
#endif
	for (n = 0, i = startpos; n < nsites; n++, i = (i+1)%nsites) {
	    Site *sip = &sites[i];
	    if (!fnmatch (p, sip->si_name, flags))
		return (i);
	}

	return (-1);
}

/* fill ab[maxn] with an abbreviated version of full.
 * N.B. allow for full == NULL or full[0] == '\0'.
 */
void
sites_abbrev (full, ab, maxn)
char *full, ab[];
int maxn;
{
	int fl;
	int n;

	/* check edge conditions */
	if (!full || (fl = strlen(full)) == 0)
	    return;

	/* just copy if it all fits ok */
	if (fl < maxn-1) {
	    (void) strcpy (ab, full);
	    return;
	}

	/* clip off words from the right until short enough.
	 * n is an index, not a count.
	 */
	for (n = fl-1; n >= maxn-4; ) {
	    while (n > 0 && isalnum(full[n]))
		n--;
	    while (n > 0 && (ispunct(full[n]) || isspace(full[n])))
		n--;
	}
	(void) sprintf (ab, "%.*s...", n+1, full);
}

/* make the site selection dialog */
static void
create_sq_w()
{
	Widget w, cl_w, fr_w, add_w;
	Arg args[20];
	int n;
	
	/* create outter form dialog */

	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False);  n++;
	XtSetArg (args[n], XmNverticalSpacing, 7); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 7); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	sq_w = XmCreateFormDialog(toplevel_w, "Sites", args, n);
	set_something (sq_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (sq_w, XmNhelpCallback, sq_help_cb, NULL);
	sr_reg (XtParent(sq_w), "XEphem*Sites.x", maincategory, 0);
	sr_reg (XtParent(sq_w), "XEphem*Sites.y", maincategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Site Selection"); n++;
	XtSetValues (XtParent(sq_w), args, n);

	/* make Close Add and Help across the bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 30); n++;
	cl_w = XmCreatePushButton (sq_w, "Close", args, n);
	wtip (cl_w, "Close this window");
	XtAddCallback (cl_w, XmNactivateCallback, sq_cancel_cb, NULL);
	XtManageChild (cl_w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 40); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 60); n++;
	add_w = XmCreateToggleButton (sq_w, "Create", args, n);
	wtip (add_w, "Build a new site definition");
	wtip (add_w, "Show fields to build a new site entry");
	XtAddCallback (add_w, XmNvalueChangedCallback, sq_create_cb, NULL);
	XtManageChild (add_w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 70); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 90); n++;
	w = XmCreatePushButton (sq_w, "Help", args, n);
	wtip (w, "Get more info about this window");
	XtAddCallback (w, XmNactivateCallback, sq_help_cb, NULL);
	XtManageChild (w);

	/* build new section */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, cl_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	fr_w = XmCreateFrame (sq_w, "New", args, n);
	XtManageChild (fr_w);

	buildNew (fr_w);

	/* make a PB and TF to enter the set string above that */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, fr_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 20); n++;
	w = XmCreatePushButton (sq_w, "Set", args, n);
	XtAddCallback (w, XmNactivateCallback, sq_set_cb, NULL);
	wtip (w, "Set Main site to string at right");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, fr_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNcolumns, 30); n++;
	settf_w = XmCreateTextField (sq_w, "SetTF", args, n);
	wtip (settf_w, "Candidate site");
	XtAddCallback (settf_w, XmNactivateCallback, sq_set_cb, NULL);
	XtManageChild (settf_w);

	/* make a PB and TF to enter the search string above that */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, settf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 20); n++;
	w = XmCreatePushButton (sq_w, "Search", args, n);
	XtAddCallback (w, XmNactivateCallback, sq_search_cb, NULL);
	wtip (w, "Search for next Site matching glob at right");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, settf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNcolumns, 30); n++;
	srtf_w = XmCreateTextField (sq_w, "SearchTF", args, n);
	wtip (srtf_w, "Candidate glob pattern");
	XtAddCallback (srtf_w, XmNactivateCallback, sq_search_cb, NULL);
	XtManageChild (srtf_w);

	/* make the scrolled list at the top */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, srtf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNselectionPolicy, XmBROWSE_SELECT); n++;
	sql_w = XmCreateScrolledList (sq_w, "SiteSL", args, n);
	wtip (sql_w, "Sites.. click to copy below, double-click to Set too");
	XtAddCallback (sql_w, XmNdefaultActionCallback, sq_dblclick_cb, NULL);
	XtAddCallback (sql_w, XmNbrowseSelectionCallback, sq_click_cb, NULL);
	XtManageChild (sql_w);
}

/* build the section that lets the user build a new entry */
static void
buildNew (Widget frame_w)
{
	typedef struct {
	    char *prompt;		/* prompt label, or NULL for gap */
	    char *defstr;		/* default string */
	    Widget *wp;			/* text widget */
	    char *tip;			/* tool tip */
	} NewRC;
	static NewRC newrc[] = {
	    {"Latitude:",  "  40:00:00", &nlt_w, "Latitude, D:M:S +N"},
	    {"Longitude:", "  75:00:00", &nlg_w, "Longitude, D:M:S +W"},
	    {"Elevation:", "         0", &nel_w, "Elevation, m above MSL"},
	    {NULL},
	    {NULL},
	    {"Zone name:", "       EST", &nzn_w, "Local time zone name"},
	    {"Offset:",    "         5", &nzo_w, "Local hours west of UTC"},
	    {"DST name:",  "       EDT", &ndn_w, "Time zone name when DST"},
	    {"Offset:",    "         4", &ndo_w, "Hours west when DST"},
	};
	Widget rc_w, mrc_w, w;
	Arg args[20];
	int i, n;

	/* master form -- managed elsewhere */

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 7); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	newf_w = XmCreateForm (frame_w, "NF", args, n);

	/* site name */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 20); n++;
	w = XmCreateLabel (newf_w, "SN", args, n);
	set_xmstring (w, XmNlabelString, "Site name:");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	nsn_w = XmCreateTextField (newf_w, "SN", args, n);
	wtip (nsn_w, "Enter name for new site");
	XtManageChild (nsn_w);

	/* next set consistent enough to use a RC */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, nsn_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNnumColumns, 4); n++;
	XtSetArg (args[n], XmNspacing, 7); n++;
	rc_w = XmCreateRowColumn (newf_w, "RC", args, n);
	XtManageChild (rc_w);

	/* build fields */

	for (i = 0; i < XtNumber(newrc); i++) {
	    NewRC *p = &newrc[i];

	    if (p->prompt) {
		n = 0;
		w = XmCreateLabel (rc_w, "NSL", args, n);
		set_xmstring (w, XmNlabelString, p->prompt);
		XtManageChild (w);
		n = 0;
		XtSetArg (args[n], XmNvalue, p->defstr); n++;
		XtSetArg (args[n], XmNcolumns, 10); n++;
		w = XmCreateTextField (rc_w, "NST", args, n);
		XtManageChild (w);
		wtip (w, p->tip);
		*p->wp = w;
	    } else {
		n = 0;
		w = XmCreateLabel (rc_w, "NSG", args, n);
		set_xmstring (w, XmNlabelString, " ");
		XtManageChild (w);
	    }
	}

	/* build DST begins and end controls */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	mrc_w = XmCreateRowColumn (newf_w, "BMB", args, n);
	wtip (mrc_w, "Rules for when summer savings time begins");
	XtManageChild (mrc_w);
	buildDST (mrc_w, "DST Beg", &dbm_w, &dbw_w, &dbd_w, &dbat_w);
	setMenuHistory (dbm_w, 3);
	setMenuHistory (dbw_w, 0);
	setMenuHistory (dbd_w, 0);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mrc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	mrc_w = XmCreateRowColumn (newf_w, "BMB", args, n);
	wtip (mrc_w, "Rules for when summer savings time ends");
	XtManageChild (mrc_w);
	buildDST (mrc_w, "DST End", &dem_w, &dew_w, &ded_w, &deat_w);
	setMenuHistory (dem_w, 9);
	setMenuHistory (dew_w, 4);
	setMenuHistory (ded_w, 0);

	/* Set main and Save PBs */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mrc_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 15); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 40); n++;
	w = XmCreatePushButton (newf_w, "S", args, n);
	wtip (w, "Install new site definition into Main");
	set_xmstring (w, XmNlabelString, "Set main");
	XtAddCallback (w, XmNactivateCallback, sq_setmain_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mrc_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 60); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 85); n++;
	w = XmCreatePushButton (newf_w, "S", args, n);
	wtip (w, "Save new site definition to private xephem_sites file");
	set_xmstring (w, XmNlabelString, "Save");
	XtAddCallback (w, XmNactivateCallback, sq_save_cb, NULL);
	XtManageChild (w);
}

static void
buildDST (Widget p_w, char *prompt, Widget *m_w, Widget *w_w, Widget *d_w,
Widget *at_w)
{
	Widget w, om_w;
	char buf[32];
	Arg args[20];
	int n;

	n = 0;
	w = XmCreateLabel (p_w, "DL", args, n);
	set_xmstring (w, XmNlabelString, prompt);
	XtManageChild (w);

	n = 0;
	*m_w = XmCreatePulldownMenu (p_w, "MPD", args, n);
	addButtons (*m_w, months, XtNumber(months));
	n = 0;
	XtSetArg (args[n], XmNsubMenuId, *m_w); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	om_w = XmCreateOptionMenu (p_w, "MCB", args, n);
	XtManageChild (om_w);
	w = XmOptionButtonGadget (om_w);
	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetValues (w, args, n);

	n = 0;
	*w_w = XmCreatePulldownMenu (p_w, "WPD", args, n);
	addButtons (*w_w, weeks, XtNumber(weeks));
	n = 0;
	XtSetArg (args[n], XmNsubMenuId, *w_w); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	om_w = XmCreateOptionMenu (p_w, "WCB", args, n);
	XtManageChild (om_w);
	w = XmOptionButtonGadget (om_w);
	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetValues (w, args, n);

	n = 0;
	*d_w = XmCreatePulldownMenu (p_w, "DPD", args, n);
	addButtons (*d_w, dow, XtNumber(dow));
	n = 0;
	XtSetArg (args[n], XmNsubMenuId, *d_w); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	om_w = XmCreateOptionMenu (p_w, "DCB", args, n);
	XtManageChild (om_w);
	w = XmOptionButtonGadget (om_w);
	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetValues (w, args, n);

	n = 0;
	w = XmCreateLabel (p_w, "AT", args, n);
	set_xmstring (w, XmNlabelString, "at");
	XtManageChild (w);
	n = 0;
	XtSetArg (args[n], XmNcolumns, 10); n++;
	sprintf (buf, "%10s", twoam);
	XtSetArg (args[n], XmNvalue, buf); n++;
	*at_w = XmCreateTextField (p_w, "ATT", args, n);
	XtManageChild (*at_w);
}

static void
addButtons (Widget pd_w, char **names, int nnames)
{
	Widget w;
	Arg args[20];
	int i, n;

	for (i = 0; i < nnames; i++) {
	    n = 0;
	    w = XmCreatePushButton (pd_w, "SPB", args, n);
	    set_xmstring (w, XmNlabelString, names[i]);
	    XtManageChild (w);
	}
}

/* menuHistory in the given option menu to widget number i */
static void
setMenuHistory (Widget w, int i)
{
	WidgetList children;

	get_something (w, XmNchildren, (XtArgVal)&children);
	set_something (w, XmNmenuHistory, (XtArgVal)children[i]);
}

/* shift the scrolled list so sites[i] is selected and visible */
static void
scroll_sites (i)
int i;
{
	i += 1;					/* List is 1-based */
	XmListSetPos (sql_w, i);	 	/* scroll it to top */
	XmListSelectPos (sql_w, i, False);	/* just highlight it */
}

/* reset the current sites list, read shared then merge private and sort */
static void
read_files ()
{
	char fn[1024];
	FILE *fp;

	/* reset current list */
	XtFree ((char *)sites);
	sites = NULL;
	nsites = 0;

	/* try share then private */
	sprintf (fn, "%s/auxil/%s", getShareDir(), sfn);
	fp = fopenh (fn, "r");
	if (fp) {
	    read_file (fp);
	    fclose (fp);
	} else {
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	}
	sprintf (fn, "%s/%s", getPrivateDir(), sfn);
	fp = fopenh (fn, "r");
	if (fp) {
	    read_file (fp);
	    fclose (fp);
	} else if (errno != ENOENT) {	/* ok if just absent */
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	}

	/* sort by name */
	qsort ((void *)sites, nsites, sizeof(Site), sites_cmpf);
}

/* read the given sites file and append to sites list.
 * caller checks if nsites increased to gauge success
 */
static void
read_file (FILE *fp)
{
	char buf[MAXSITELLEN];

	/* read each entry, building up list */
	while (fgets (buf, sizeof(buf), fp) != NULL) {
	    char name[MAXSITELLEN];
	    char tzdefn[128];
	    int latd, latm;
	    int lngd, lngm;
	    double lats, lngs;
	    char latNS, lngEW;
	    double lt, lg;
	    double ele;
	    Site *sp;
	    int l;
	    int nf;

	    /* read line.. skip if not complete. tz is optional */
	    tzdefn[0] = '\0';
	    nf = sscanf (buf, "%[^;]; %3d %2d %lf %c   ; %3d %2d %lf %c   ;%lf ; %s",
				    name, &latd, &latm, &lats, &latNS,
				    &lngd, &lngm, &lngs, &lngEW, &ele, tzdefn);
	    if (nf < 10)
		continue;

	    /* strip trailing blanks off name */
	    for (l = strlen (name); --l >= 0; )
		if (isspace(name[l]))
		    name[l] = '\0';
		else
		    break;

	    /* crack location */
	    lt = degrad (latd + latm/60.0 + lats/3600.0);
	    if (latNS == 'S')
		lt = -lt;
	    lg = degrad (lngd + lngm/60.0 + lngs/3600.0);
	    if (lngEW == 'W')
		lg = -lg;

	    /* extend sites array */
	    sp = moreSites();

	    /* fill a new Site record */
	    memset ((void *)sp, 0, sizeof(Site));
	    sp->si_lat = (float)lt;
	    sp->si_lng = (float)lg;
	    sp->si_elev = (float)ele;
	    (void) strncpy (sp->si_tzdefn, tzdefn, sizeof(sp->si_tzdefn)-1);
	    (void) strncpy (sp->si_name, name, sizeof(sp->si_name)-1);
	}
}

/* (re)fill the scrolled list sql_w with the set of sites.
 * also tell earth view to redraw.
 */
static void
fill_list ()
{
	XmString *xms;
	int i;

	/* build array of XmStrings for fast updating of the ScrolledList */
	xms = (XmString *) XtMalloc (nsites * sizeof(XmString));
	for (i = 0; i < nsites; i++)
	    xms[i] = XmStringCreateSimple (sites[i].si_name);
	XmListDeleteAllItems (sql_w);
	XmListAddItems (sql_w, xms, nsites, 0);

	/* finished with the XmStrings table */
	for (i = 0; i < nsites; i++)
	    XmStringFree (xms[i]);
	XtFree ((void *)xms);

	/* update earth list */
	e_update (mm_get_now(), 1);
}

/* compare name portions of two pointers to Sites in qsort fashion.
 */
static int
sites_cmpf (const void * v1, const void * v2)
{
	char *name1 = ((Site *)v1)->si_name;
	char *name2 = ((Site *)v2)->si_name;

	return (strcmp (name1, name2));
}

/* called when CR is hit in the Set text field /or/ from Set PB.
 */
/* ARGSUSED */
static void
sq_set_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *str = XmTextFieldGetString (settf_w);
	int i;

	/* if text field matches a site, use the full definition, else just
	 * use the simple string.
	 */
	for (i = 0; i < nsites; i++) {
	    if (!strcmp (sites[i].si_name, str)) {
		scroll_sites (i);
		mm_setsite(&sites[i], 0);
		break;
	    }
	}
	if (i == nsites)
	    mm_sitename (str);

	XtFree (str);
}

/* called when CR is hit in the search text field /or/ from Search PB */
/* ARGSUSED */
static void
sq_search_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *str;
	int i;

	str = XmTextFieldGetString (srtf_w);
	i = sites_search (str);
	if (i < 0)
	    xe_msg (1, "No matching site found.");
	else {
	    /* Set and scroll */
	    XmTextFieldSetString (settf_w, sites[i].si_name);
	    scroll_sites (i);
	}
	XtFree (str);
}

/* called when an item in the scrolled list is double-clicked */
/* ARGSUSED */
static void
sq_dblclick_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int *pos, npos;
	int free;

	/* put into Set field and install in Main, all at once */
	if ((free = XmListGetSelectedPos (sql_w, &pos, &npos)) && 
			npos == 1 && pos[0] > 0 && pos[0] <= nsites) {
	    Site *sip = &sites[pos[0]-1]; /* pos is 1-based */
	    XmTextFieldSetString (settf_w, sip->si_name);
	    mm_setsite(sip, 0);
	} else {
	    xe_msg (1, "Bogus list selection");
	}

	if (free)
	    XtFree ((char *)pos);
}

/* called when an item in the scrolled list is single-clicked */
/* ARGSUSED */
static void
sq_click_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int *pos, npos;
	int free;

	/* copy to set field */
	if ((free = XmListGetSelectedPos (sql_w, &pos, &npos)) && 
			npos == 1 && pos[0] > 0 && pos[0] <= nsites) {
	    Site *sip = &sites[pos[0]-1]; /* pos is 1-based */
	    XmTextFieldSetString (settf_w, sip->si_name);
	} else {
	    xe_msg (1, "Bogus list selection");
	}

	if (free)
	    XtFree ((char *)pos);
}

/* callback from the Help button
 */
/* ARGSUSED */
static void
sq_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"Load and select from a list of sites."
};

	hlp_dialog ("MainMenu_sites_dialog", msg, XtNumber(msg));
}

/* called when the Cancel button is hit */
/* ARGSUSED */
static void
sq_cancel_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (sq_w);
}

/* called when the Add TB is hit */
/* ARGSUSED */
static void
sq_create_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XmToggleButtonGetState (w))
	    XtManageChild (newf_w);
	else
	    XtUnmanageChild (newf_w);
}

/* called when the Set main PB is hit */
/* ARGSUSED */
static void
sq_setmain_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Site ns;

	if (buildNewSite(&ns) < 0)
	    return;
	mm_setsite (&ns, 0);
}

/* called when the Save PB is hit */
/* ARGSUSED */
static void
sq_save_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char fn[1024];
	Site ns;
	FILE *fp;

	if (buildNewSite(&ns) < 0)
	    return;
	
	/* add to display set */
	memcpy (moreSites(), &ns, sizeof(ns));
	qsort ((void *)sites, nsites, sizeof(Site), sites_cmpf);
	fill_list();

	/* append to private file */
	sprintf (fn, "%s/%s", getPrivateDir(), sfn);
	fp = fopenh (fn, "a+");
	if (fp) {
	    /* 123:45:67 */
	    char latbuf[32], lngbuf[32];
	    fs_sexa (latbuf, raddeg(fabs(ns.si_lat)), 3, 3600);
	    fs_sexa (lngbuf, raddeg(fabs(ns.si_lng)), 3, 3600);
	    fprintf (fp,
		"%-40s; %.3s %.2s %.2s %c   ; %.3s %.2s %.2s %c   ;%6.1f ; %s\n",
			ns.si_name,
			latbuf, latbuf+4, latbuf+7, ns.si_lat < 0 ? 'S' : 'N',
			lngbuf, lngbuf+4, lngbuf+7, ns.si_lng < 0 ? 'W' : 'E',
			ns.si_elev, ns.si_tzdefn);
	    fclose (fp);
	} else {
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	}
}

/* gather widget settings and build a new Site.
 * return 0 if looks ok, else -1
 */
static int
buildNewSite (Site *sp)
{
	char buf[128], *bp;
	int bl = sizeof(buf);
	double tmp;
	int zl;

	/* init */

	memset (sp, 0, sizeof(*sp));

	/* always gather name lat long elev */

	strncpy (sp->si_name, getTF(nsn_w,buf,bl), sizeof(sp->si_name)-1);
	if (strlen(sp->si_name) == 0) {
	    xe_msg (1, "Please specify a site name");
	    return (-1);
	}

	f_scansexa (getTF(nlt_w,buf,bl), &tmp);
	sp->si_lat = (float) degrad (tmp);

	f_scansexa (getTF(nlg_w,buf,bl), &tmp);
	sp->si_lng = (float) degrad (-tmp);

	sp->si_elev = (float) strtod (getTF(nel_w,buf,bl), NULL);

	zl = 0;

	zl += sprintf (sp->si_tzdefn+zl, "%s", nows(getTF(nzn_w,buf,bl)));

	f_scansexa (getTF(nzo_w,buf,bl), &tmp);
	if (tmp - (int)tmp != 0)
	    fs_sexa (buf, tmp, 2, 60);
	zl += sprintf (sp->si_tzdefn+zl, "%s", nows(buf));

	/* only do the rest if dst name is non-blank */

	bp = nows(getTF(ndn_w,buf,bl));
	if (strlen(bp) > 0) {
	    zl += sprintf (sp->si_tzdefn+zl, "%s", bp);

	    f_scansexa (getTF(ndo_w,buf,bl), &tmp);
	    if (tmp - (int)tmp != 0)
		fs_sexa (buf, tmp, 2, 60);
	    zl += sprintf (sp->si_tzdefn+zl, "%s", nows(buf));


	    zl += sprintf (sp->si_tzdefn+zl, ",M%d", getPDmH (dbm_w)+1);
	    zl += sprintf (sp->si_tzdefn+zl, ".%d", getPDmH (dbw_w)+1);
	    zl += sprintf (sp->si_tzdefn+zl, ".%d", getPDmH (dbd_w));
	    f_scansexa (getTF(dbat_w,buf,bl), &tmp);
	    fs_sexa (buf, tmp, 3, 3600);
	    if (strcmp(buf,twoam))
		zl += sprintf (sp->si_tzdefn+zl, "/%s", nows(buf));

	    zl += sprintf (sp->si_tzdefn+zl, ",M%d", getPDmH (dem_w)+1);
	    zl += sprintf (sp->si_tzdefn+zl, ".%d", getPDmH (dew_w)+1);
	    zl += sprintf (sp->si_tzdefn+zl, ".%d", getPDmH (ded_w));
	    f_scansexa (getTF(deat_w,buf,bl), &tmp);
	    fs_sexa (buf, tmp, 3, 3600);
	    if (strcmp(buf,twoam))
		zl += sprintf (sp->si_tzdefn+zl, "/%s", nows(buf));
	}

	/*
	fprintf (stderr, "'%s' '%s'\n", sp->si_name, sp->si_tzdefn);
	*/

	return (0);
}

/* extract up to bl-1 chars of text from given text field widget into buf,
 * return buf which always includes EOS.
 */
static char *
getTF (Widget w, char buf[], int bl)
{
	char *txt;

	txt = XmTextFieldGetString (w);
	strncpy (buf, txt, bl);
	XtFree (txt);
	buf[bl-1] = '\0';
	return (buf);
}

/* remove all white space from bp IN PLACE and return first non-white char */
static char *
nows (char *bp)
{
	char *start = NULL, *dest;

	for (dest = bp; *bp; bp++) {
	    if (*bp != ' ') {
		if (!start)
		    start = dest;
		*dest++ = *bp;
	    }
	}
	*dest = '\0';

	return (start ? start : bp);
}

/* scan the children of the given pulldown menu and return the children[] 
 * index of the one matching menuHistory
 */
static int
getPDmH (Widget pd)
{
	WidgetList children;
	Cardinal numChildren;
	Widget mh;
	int i;

	get_something (pd, XmNchildren, (XtArgVal)&children);
	get_something (pd, XmNnumChildren, (XtArgVal)&numChildren);
	get_something (pd, XmNmenuHistory, (XtArgVal)&mh);

	for (i = 0; i < numChildren; i++)
	    if (children[i] == mh)
		return (i);

	printf ("Bug! no menuHistory for %s\n", XtName(pd));
	abort();
}

/* extend sites[] by one and return pointer to new entry */
static Site *
moreSites()
{
	sites = (Site *) XtRealloc ((void *)sites, (nsites+1)*sizeof(Site));
	return (&sites[nsites++]);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: sites.c,v $ $Date: 2006/08/21 20:04:30 $ $Revision: 1.27 $ $Name:  $"};

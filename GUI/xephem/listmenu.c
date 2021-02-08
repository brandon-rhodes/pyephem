/* code to manage the stuff on the "listing" menu.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>

#include "xephem.h"
#include "lilxml.h"

static void lst_select (int whether);
static void lst_create_shell (void);
static void lst_activate_cb (Widget w, XtPointer client, XtPointer call);
static void lst_close_cb (Widget w, XtPointer client, XtPointer call);
static void lst_loadcfg_cb (Widget w, XtPointer client, XtPointer call);
static void lst_savecfg_cb (Widget w, XtPointer client, XtPointer call);
static void lst_help_cb (Widget w, XtPointer client, XtPointer call);
static void lst_undo_cb (Widget w, XtPointer client, XtPointer call);
static void lst_reset (void);
static void lst_add (char *name);
static void lst_stop_selecting (void);
static void lst_turn_off (void);
static void lst_try_append (void);
static void lst_try_overwrite (void);
static void lst_try_cancel (void);
static void lst_try_turn_on (void);
static void lst_turn_on (char *how);
static void lst_hdr (void);

#define	COMMENT	'*'		/* comment character */

#define MAXLSTSTR	32	/* longest string we can list */
#define MAXFLDNAM	32	/* longest allowed field name */
#define	INDENT		15	/* file info indent, pixel */

static Widget lstshell_w;	/* main shell */
static Widget select_w;		/* select mode TB */
static Widget active_w;		/* creating list TB */
static Widget prompt_w;		/* label to inform what to do next */
static Widget colhdr_w;		/* include col hdr TB */
static Widget latex_w;		/* latex option TB */
static Widget title_w;		/* title text field */
static Widget filename_w;	/* file name text field */
static Widget undo_w;		/* undo PB */
static Widget lrc_w;		/* RC for list entries */
static Widget cfn_w;		/* TF with name of file to save config too */

static FILE *lst_fp;            /* the listing file; == 0 means don't plot */
static int lst_new;		/* 1 when open until first set are printed */
static int lst_latex;		/* 1 when want latex format */

/* lst_activate_cb client values. */
typedef enum {
    SELECT, ACTIVE, COLHDR, LATEX
} Options;

/* store the name, string value and widget for each field to track.
 * we get the label straight from the Text widget in the table as needed.
 */
typedef struct {
    char l_name[MAXFLDNAM];	/* name of field we are listing */
    char l_str[MAXLSTSTR];	/* last know string value of field */
    int l_cw;			/* column width -- 0 until known */
    Widget l_w;			/* table entry in lrc_w */
} LstFld;
static LstFld *lstflds;		/* malloced list */
static int nlstflds;		/* number of lstflds[] in actual use */
static int mlstflds;		/* number of lstflds[] in existance */ 
static char config_suffix[] = ".lsc";	/* file ext for list config files */

static char listcategory[] = "Tools -- List"; /* Save category */
static char xml_listele[] = "XEphemListConfig";

/* called when the list menu is activated via the main menu pulldown.
 * if never called before, create and manage all the widgets as a child of a
 * form. otherwise, go for it.
 */
void
lst_manage ()
{
	if (!lstshell_w)
	    lst_create_shell();
	
	XtPopup (lstshell_w, XtGrabNone);
	set_something (lstshell_w, XmNiconic, (XtArgVal)False);
}

/* called by the other menus (data, etc) as their buttons are
 * selected to inform us that that button is to be included in a listing.
 */
void
lst_selection (name)
char *name;
{
	if (!isUp(lstshell_w) || !XmToggleButtonGetState(select_w))
	    return;

	lst_add (name);
}

/* called as each different field is written -- just save in lstflds[]
 * if we are potentially interested.
 */
void
lst_log (name, str)
char *name;
char *str;
{
	if (listing_ison()) {
	    LstFld *lp;
	    for (lp = lstflds; lp < &lstflds[nlstflds]; lp++)
		if (strcmp (name, lp->l_name) == 0) {
		    (void) strncpy (lp->l_str, str, MAXLSTSTR-1);
		    break;
		}
	}
}

/* called when all fields have been updated and it's time to
 * write the active listing to the current listing file, if one is open.
 */
void
listing()
{
	if (lst_fp) {
	    /* list in order of original selection */
	    LstFld *lp;

	    /* print headings if this the first time */
	    if (lst_new) {
		lst_hdr();
		lst_new = 0;
	    }

	    /* now print the fields */
	    for (lp = lstflds; lp < &lstflds[nlstflds]; lp++) {
		(void) fprintf (lst_fp, "  %-*s", lp->l_cw, lp->l_str);
		if (lst_latex && lp < &lstflds[nlstflds-1])
		    (void) fprintf (lst_fp, "&");
	    }
	    if (lst_latex)
	    	(void) fprintf (lst_fp, "\\\\");
	    (void) fprintf (lst_fp, "\n");
	    fflush (lst_fp);	/* to allow monitoring */
	}
}

int
listing_ison()
{
	return (lst_fp != 0);
}

/* called to put up or remove the watch cursor.  */
void
lst_cursor (c)
Cursor c;
{
	Window win;

	if (lstshell_w && (win = XtWindow(lstshell_w)) != 0) {
	    Display *dsp = XtDisplay(lstshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* inform the other menues whether we are setting up for them to tell us
 * what fields to list.
 */
static void
lst_select(whether)
int whether;
{
	all_selection_mode(whether);
}

static void
lst_create_shell()
{
	typedef struct {
	    int indent;		/* amount to indent, pixels */
	    char *iname;	/* instance name, if Saveable */
	    char *title;
	    int cb_data;
	    Widget *wp;
	    char *tip;
	} TButton;
	static TButton tbs[] = {
	    {0, NULL, "Select fields to list", SELECT, &select_w,
		"When on, data fields eligible for listing are selectable buttons"},
	    {0, NULL, "Create list file", ACTIVE, &active_w,
		"When on, selected fields are written to the named file at each main Update"},
	    {INDENT, "Headings", "Include column headings", COLHDR, &colhdr_w,
		"Whether file format will include a heading over each column"},
	    {INDENT, "LaTex", "List in LaTeX format", LATEX, &latex_w,
		"Whether to separate columns with `&' and end lines with `\\\\'"},
	};
	XmString str;
	Widget w, rc_w, f_w, oms_w;
	Widget lstform_w;
	Arg args[20];
	char *s[1];
	int i, n;

	/* create form dialog */
	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Listing Control"); n++;
	XtSetArg (args[n], XmNiconName, "List"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	lstshell_w = XtCreatePopupShell ("List", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (lstshell_w);
	set_something (lstshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (lstshell_w, "XEphem*List.x", listcategory, 0);
	sr_reg (lstshell_w, "XEphem*List.y", listcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNresizePolicy, XmRESIZE_ANY); n++;
        XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	lstform_w = XmCreateForm(lstshell_w, "ListF", args, n);
	XtAddCallback (lstform_w, XmNhelpCallback, lst_help_cb, 0);
	XtManageChild (lstform_w);

	/* make a RowColumn to hold everything */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNtopOffset, 10); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNadjustMargin, False); n++;
	XtSetArg (args[n], XmNspacing, 5); n++;
	rc_w = XmCreateRowColumn (lstform_w, "ListRC", args, n);
	XtManageChild (rc_w);

	/* control to allow selecting a config file to load */

	s[0] = config_suffix;
	oms_w = createFSM (rc_w, s, 1, "auxil", lst_loadcfg_cb);
	set_xmstring (oms_w, XmNlabelString, "Load config file:   ");

	/* save config TB and its TF */

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, 0); n++;
	f_w = XmCreateForm (rc_w, "LCF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    w = XmCreateToggleButton(f_w, "LSTB", args, n);
	    XtAddCallback(w, XmNvalueChangedCallback, lst_savecfg_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Save config file: ");
	    wtip (w, "Save current list definition to file");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    cfn_w = XmCreateTextField (f_w, "ConfigFile", args, n);
	    defaultTextFN (cfn_w, 0, "myconfig.lsc", NULL);
	    wtip (cfn_w,
		"Enter name of file in which to save current list definition");
	    XtManageChild (cfn_w);
	    sr_reg (cfn_w, NULL, listcategory, 0);

	/* make the control toggle buttons */

	for (i = 0; i < XtNumber(tbs); i++) {
	    TButton *tbp = &tbs[i];

	    str = XmStringCreate(tbp->title, XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNmarginWidth, tbp->indent); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateToggleButton(rc_w, tbp->iname ? tbp->iname : "ListTB",
								    args, n);
	    XmStringFree (str);
	    XtAddCallback(w, XmNvalueChangedCallback, lst_activate_cb,
						    (XtPointer)(long int)(tbp->cb_data));
	    if (tbp->wp)
		*tbp->wp = w;
	    if (tbp->tip)
		wtip (w, tbp->tip);
	    XtManageChild (w);
	    if (tbp->iname)
		sr_reg (w, NULL, listcategory, 1);
	}

	/* create filename text area and its label in a form */

	n = 0;
	f_w = XmCreateForm (rc_w, "FNF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, INDENT); n++;
	    w = XmCreateLabel (f_w, "ListFnL", args, n);
	    set_xmstring (w, XmNlabelString, "Name:  ");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNcolumns, 40); n++;
	    filename_w = XmCreateTextField (f_w, "Filename", args, n);
	    wtip (filename_w, "Enter name of file to create with list");
	    XtManageChild (filename_w);
	    sr_reg (filename_w, NULL, listcategory, 0);

	/* create title text area and its label in a form */

	n = 0;
	f_w = XmCreateForm (rc_w, "TF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, INDENT); n++;
	    w = XmCreateLabel (f_w, "ListTL", args, n);
	    set_xmstring (w, XmNlabelString, "Title: ");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNcolumns, 40); n++;
	    title_w = XmCreateTextField (f_w, "Title", args, n);
	    wtip (title_w, "Enter a title to be written to the file");
	    XtManageChild (title_w);

	/* create prompt line -- it will be managed as necessary */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	prompt_w = XmCreateLabel (rc_w, "ListPrompt", args, n);
	set_xmstring(prompt_w,XmNlabelString,"Choose field for next column...");

	/* make an RC for the table */
	n = 0;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
	XtSetArg (args[n], XmNisAligned, True); n++;
	lrc_w = XmCreateRowColumn (rc_w, "LRC", args, n);
	XtManageChild (lrc_w);

	/* create a separator */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep", args, n);
	XtManageChild (w);

	/* make a form to hold the bottom buttons evenly */

	n = 0;
	XtSetArg (args[n], XmNfractionBase, 10); n++;
	f_w = XmCreateForm (rc_w, "ListCF", args, n);
	XtManageChild(f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 1); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 3); n++;
	    w = XmCreatePushButton (f_w, "Close", args, n);
	    wtip (w, "Close this dialog (but continue listing if active)");
	    XtAddCallback (w, XmNactivateCallback, lst_close_cb, 0);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 4); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 6); n++;
	    XtSetArg (args[n], XmNsensitive, False); n++;
	    undo_w = XmCreatePushButton (f_w, "Undo", args, n);
	    wtip (undo_w, "Undo last column choice");
	    XtAddCallback (undo_w, XmNactivateCallback, lst_undo_cb, 0);
	    XtManageChild (undo_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 7); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 9); n++;
	    w = XmCreatePushButton (f_w, "Help", args, n);
	    wtip (w, "More detailed usage information");
	    XtAddCallback (w, XmNactivateCallback, lst_help_cb, 0);
	    XtManageChild (w);
}

/* callback from any of the listing menu toggle buttons being activated.
 */
/* ARGSUSED */
static void
lst_activate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *t = (XmToggleButtonCallbackStruct *) call;
	Options op = (Options) client;

	switch (op) {
	case SELECT:
	    if (t->set) {
		/* first turn off listing, if on, while we change things */
		if (XmToggleButtonGetState(active_w))
		    XmToggleButtonSetState(active_w, False, True);
		lst_reset();	/* reset lstflds array and unmanage the table*/
		lst_select(1);	/* inform other menus to inform us of fields */
		XtManageChild (prompt_w);
		XtSetSensitive (undo_w, True);
	    } else
		lst_stop_selecting();
	    break;

	case ACTIVE:
	    if (t->set) {
		/* first turn off selecting, if on */
		if (XmToggleButtonGetState(select_w))
		    XmToggleButtonSetState(select_w, False, True);
		if (nlstflds > 0)
		    lst_try_turn_on();
		else {
		    XmToggleButtonSetState(w, False, True);
		    xe_msg (1, "Please select at least one field to list");
		}
	    } else
		lst_turn_off();
	    break;

	case COLHDR:
	    break;	/* toggle state is sufficient */

	case LATEX:
	    lst_latex = t->set;
	    break;
	}
}

/* callback to load a new config file. 
 * file name is label of this widget
 */
/* ARGSUSED */
static void
lst_loadcfg_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024];
	FILE *fp;
	char *fn;

	/* turn off listing and selecting, if on */
	if (XmToggleButtonGetState(active_w))
	    XmToggleButtonSetState(active_w, False, True);
	if (XmToggleButtonGetState(select_w))
	    XmToggleButtonSetState(select_w, False, True);

	get_xmstring (w, XmNlabelString, &fn);
	fp = fopend (fn, NULL, "r");
	if (fp) {
	    LilXML *lp = newLilXML();
	    XMLEle *pele = readXMLFile (fp, lp, buf);

	    if (!pele)
		xe_msg (1, "%s:\n%s", fn, buf);
	    else if (strcmp (tagXMLEle(pele), xml_listele)) {
		xe_msg (1, "Not a valid list config file");
	    } else {
		XMLEle *ep;

		lst_reset();
		for (ep= nextXMLEle (pele,1); ep != NULL; ep=nextXMLEle(pele,0))
		    if (strcmp (tagXMLEle(ep), "list") == 0)
			lst_add (findXMLAttValu (ep, "name"));
		XmTextFieldSetString (title_w,
				pcdataXMLEle(findXMLEle(pele,"title")));
		XmToggleButtonSetState (colhdr_w,
				atoi(findXMLAttValu (pele, "headings")), True);
		XmToggleButtonSetState (latex_w,
				atoi(findXMLAttValu (pele, "latex")), True);
		lst_stop_selecting();
	    }
	    delXMLEle (pele);
	    delLilXML (lp);
	    fclose (fp);
	}

	XtFree (fn);
}

/* callback to save config to the file named by cfn_w 
 */
/* ARGSUSED */
static void
lst_savecfg_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	FILE *fp;
	char buf[1024], *txt;
	char *fn;
	int i;

	if (!XmToggleButtonGetState(w))
	    return;

	/* stop selecting if on */
	if (XmToggleButtonGetState (select_w))
	    XmToggleButtonSetState (select_w, False, True);

	/* set up file name */
	fn = txt = XmTextFieldGetString (cfn_w);
	if (!strstr (txt, config_suffix)) {
	    sprintf (fn = buf, "%s%s", txt, config_suffix);
	    XmTextFieldSetString (cfn_w, fn);
	}

	/* create config file, save each entry */
	fp = fopend (fn, NULL, "w");
	if (fp) {
	    char *title = XmTextFieldGetString (title_w);
	    fprintf (fp, "<%s headings='%d' latex='%d'>\n", xml_listele,
					XmToggleButtonGetState (colhdr_w),
					XmToggleButtonGetState (latex_w));
	    fprintf (fp, "  <title>%s</title>\n", title);
	    for (i = 0; i < nlstflds; i++)
		fprintf (fp, "  <list name='%s'/>\n", lstflds[i].l_name);
	    fprintf (fp, "</%s>\n", xml_listele);
	    XtFree (title);
	    fclose (fp);
	}

	/* act like a push button */
	XmToggleButtonSetState (w, False, True);
	if (confirm())
	    xe_msg (1, "%s saved", fn);

	XtFree (txt);
}

/* callback from the Close button.
 */
/* ARGSUSED */
static void
lst_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (lstshell_w);
}

/* callback from the Help
 */
/* ARGSUSED */
static void
lst_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"Select fields to become each column of a listing, then run xephem. Each step",
"will yield one line in the output file. The filename may be specified in the",
"text area provided."
};

	hlp_dialog ("Listing", msg, sizeof(msg)/sizeof(msg[0]));
}

/* callback from the Undo button.
 */
/* ARGSUSED */
static void
lst_undo_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (nlstflds > 0)
	    XtUnmanageChild (lstflds[--nlstflds].l_w);
}

/* forget the list, and unmanage the table.
 */
static void
lst_reset()
{
	int i;

	for (i = 0; i < nlstflds; i++)
	    XtUnmanageChild (lstflds[i].l_w);
	nlstflds = 0;
}

/* add and display a new name to list */
static void
lst_add (char *name)
{
	Widget tw;

	if (nlstflds == mlstflds) {
	    lstflds = (LstFld *) XtRealloc ((char *)lstflds,
						(++mlstflds)*sizeof(LstFld));
	    lstflds[nlstflds].l_w = XmCreateLabel(lrc_w, "ListLabel", NULL, 0);
	}

	tw = lstflds[nlstflds].l_w;
	set_xmstring (tw, XmNlabelString, name);
	XtManageChild (tw);
	(void) strncpy (lstflds[nlstflds].l_name, name, MAXFLDNAM);
	nlstflds++;
}

/* stop selecting: tell everybody else to drop their buttons, make sure toggle
 * is off.
 */
static void
lst_stop_selecting()
{
	XmToggleButtonSetState (select_w, False, False);
	lst_select(0);
	XtUnmanageChild (prompt_w);
	XtSetSensitive (undo_w, False);
}

static void
lst_turn_off ()
{
	if (lst_fp) {
	    (void) fclose (lst_fp);
	    lst_fp = 0;
	}
}

/* called from the query routine when want to append to an existing list file.*/
static void
lst_try_append()
{
	lst_turn_on("a");
}

/* called from the query routine when want to overwrite to an existing list
 * file.
 */
static void
lst_try_overwrite()
{
	lst_turn_on("w");
}

/* called from the query routine when decided not to make a listing file.  */
static void
lst_try_cancel()
{
	XmToggleButtonSetState (active_w, False, False);
}

/* attempt to open file for use as a listing file.
 * if it doesn't exist, then go ahead and make it.
 * but if it does, first ask wheher to append or overwrite.
 */
static void
lst_try_turn_on()
{
	char *txt = XmTextFieldGetString (filename_w);
	char pn[1024];

	sprintf (pn, "%s/%s", getPrivateDir(), txt);

	if (confirm() && existsh (pn) == 0) {
	    char buf[1024];
	    (void) sprintf (buf, "%s exists:\nAppend or Overwrite?", txt);
	    query (lstshell_w, buf, "Append", "Overwrite", "Cancel",
			    lst_try_append, lst_try_overwrite, lst_try_cancel);
	} else
	    lst_try_overwrite();

	XtFree (txt);
}

/* turn on listing facility.
 * establish a file to use (and thereby set lst_fp, the "listing-is-on" flag).
 */
static void
lst_turn_on (how)
char *how;	/* fopen how argument */
{
	char *txt = XmTextFieldGetString (filename_w);
	char pn[1024];

	sprintf (pn, "%s/%s", getPrivateDir(), txt);

	/* listing is on if file opens ok */
	lst_fp = fopenh (pn, how);
	if (!lst_fp) {
	    XmToggleButtonSetState (active_w, False, False);
	    xe_msg (1, "%s:\n%s", txt, syserrstr());
	}
	XtFree (txt);

	lst_new = 1;	/* trigger fresh column headings */
	/* TODO: not when appending? */
}

/* print the title.
 * then set each l_w. if column headings are enabled, use and also print them.
 *   else just use l_str.
 */
static void
lst_hdr ()
{
	LstFld *lp;
	int col;
	char *txt;

	/* add a title if desired */
	txt = XmTextFieldGetString (title_w);
	if (txt[0] != '\0')
	    (void) fprintf (lst_fp, "%c %s\n", COMMENT, txt);
	XtFree (txt);

	col = XmToggleButtonGetState (colhdr_w);

	/* set lp->l_cw to max of str, prefix and suffix lengths */
	for (lp = lstflds; lp < &lstflds[nlstflds]; lp++) {
	    int l = strlen (lp->l_str);
	    if (col) {
		int nl = strlen(lp->l_name);
		char *dp;

		for (dp = lp->l_name; *dp && *dp != '.'; dp++)
		    continue;
		if (*dp) {
		    int pl = dp - lp->l_name;	/* prefix */
		    int sl = nl - pl - 1; 	/* suffix */
		    if (pl > l) l = pl;
		    if (sl > l) l = sl;
		} else {
		    if (nl > l) l = nl;
		}
	    }
	    lp->l_cw = l;
	}

	if (col) {
	    int printed_anything;

	    /* print first row of column headings */
	    for (lp = lstflds; lp < &lstflds[nlstflds]; lp++) {
		char cmt = lp == lstflds && !lst_latex ? COMMENT : ' ';
		char *dp;

		for (dp = lp->l_name; *dp && *dp != '.'; dp++)
		    continue;
		if (*dp)
		    fprintf (lst_fp, "%c %-*.*s", cmt, lp->l_cw,
						    (int)(dp-lp->l_name), lp->l_name);
		else
		    fprintf (lst_fp, "%c %-*s", cmt, lp->l_cw, lp->l_name);
		if (lst_latex && lp < &lstflds[nlstflds-1])
		    fprintf (lst_fp, "&");
	    }
	    if (lst_latex)
	    	fprintf (lst_fp, "\\\\");
	    fprintf (lst_fp, "\n");

	    /* print second row of column headings */
	    printed_anything = 0;
	    for (lp = lstflds; lp < &lstflds[nlstflds]; lp++) {
		char cmt = lp == lstflds && !lst_latex ? COMMENT : ' ';
		char *dp;

		for (dp = lp->l_name; *dp && *dp != '.'; dp++)
		    continue;
		if (*dp) {
		    fprintf (lst_fp, "%c %-*s", cmt, lp->l_cw, dp+1);
		    printed_anything = 1;
		} else
		    fprintf (lst_fp, "%c %-*s", cmt, lp->l_cw, "");
		if (lst_latex && printed_anything && lp < &lstflds[nlstflds-1])
		    fprintf (lst_fp, "&");
	    }
	    if (lst_latex && printed_anything)
	    	fprintf (lst_fp, "\\\\");
	    fprintf (lst_fp, "\n");
	}

	fflush (lst_fp); /* to allow monitoring */
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: listmenu.c,v $ $Date: 2009/01/05 20:55:54 $ $Revision: 1.23 $ $Name:  $"};

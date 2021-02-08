/* handle the history feature of skyview */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/TextF.h>
#include <Xm/ScrolledW.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/ArrowB.h>
#include <Xm/CascadeB.h>

#include "xephem.h"

extern char skycategory[];

static char skyhfn[]= "xephem_skyhist";	/* file name to store history */
#define	VERSIONN		5	/* version number */

/* info to manage any number of history entries and their controls.
 * we destroy and recreate all the entries in the Dialog each time the
 * history changes, but we keep reusing the PBs in the Pulldown menu.
 */
typedef struct {
    SvHistory h;			/* history */
    char *ulbl;				/* malloced user label */
    Widget ltf_w;			/* TF widget showing ulbl */
} SVHEntry;
static SVHEntry *hist;			/* malloced list of SVHEntry */
static int nhist;			/* number of entries in nhist[] */
static Widget *pdpb_w;			/* malloced list of PBs for pulldown */
static int npdpb;			/* number in pdpb_w[] */

static Widget svhshell_w;		/* main form shell */
static Widget pd_w;			/* pulldown menu */
static Widget sw_w;			/* list scrolled window */

static int autonum;			/* for building a unique def ulbl */

static void svh_manage_cb (Widget w, XtPointer client, XtPointer call);
static void svh_add_cb (Widget w, XtPointer client, XtPointer call);
static void svh_del_cb (Widget w, XtPointer client, XtPointer call);
static void svh_rep_cb (Widget w, XtPointer client, XtPointer call);
static void svh_go_cb (Widget w, XtPointer client, XtPointer call);
static void svh_up_cb (Widget w, XtPointer client, XtPointer call);
static void svh_down_cb (Widget w, XtPointer client, XtPointer call);
static void svh_cascading_cb (Widget w, XtPointer client, XtPointer call);
static void svh_save_cb (Widget w, XtPointer client, XtPointer call);
static void svh_load_cb (Widget w, XtPointer client, XtPointer call);
static void svh_ltfc_cb (Widget w, XtPointer client, XtPointer call);
static void svh_help_cb (Widget w, XtPointer client, XtPointer call);
static void svh_close_cb (Widget w, XtPointer client, XtPointer call);
static void show_entries (void);
static void setup_1row (Widget rc_w, int i);
static void setup_label (Widget lbl_w, SvHistory *hp);
static FILE *openhist (char *buf, char *how);
static int loadhist (FILE *histfp, char *fn, char msg[]);

/* create the History pulldown off the given menubar;
 * and create the history control dialog;
 * and Apply the first history entry, if any.
 */
void
svh_create (Widget mb_w)
{
	Widget w;
	Arg args[20];
	FILE *hfp;
	char buf[1024];
	char fn[1024];
	Widget svhform_w;
	int n;

	/* create the pulldown menu */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "HPD", args, n);

	n = 0;
	XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	XtSetArg (args[n], XmNmnemonic, 'H'); n++;
	w = XmCreateCascadeButton (mb_w, "History", args, n);
	XtAddCallback (w, XmNcascadingCallback, svh_cascading_cb, 0);
	wtip (w, "Manage a collection of Sky View settings");
	XtManageChild (w);

	/* add the "Edit" PB */

	n = 0;
	w = XmCreatePushButton (pd_w, "WPB", args, n);
	XtAddCallback (w, XmNactivateCallback, svh_manage_cb, NULL);
	wtip(w,"Open a window to manage the History list");
	set_xmstring (w, XmNlabelString, "Edit...");
	XtManageChild (w);

	/* add a separator to set off the list of entries */

	n = 0;
	w = XmCreateSeparator (pd_w, "HSep", args, n);
	XtManageChild (w);

	/* remaining PBs are added as needed when cascading */

	/* create the History shell and form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "Sky history"); n++;
	XtSetArg (args[n], XmNiconName, "Sky"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	svhshell_w = XtCreatePopupShell ("SkyHist", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (svhshell_w);
	set_something (svhshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (svhshell_w, "XEphem*SkyHist.width", skycategory, 0);
	sr_reg (svhshell_w, "XEphem*SkyHist.height", skycategory, 0);
	sr_reg (svhshell_w, "XEphem*SkyHist.x", skycategory, 0);
	sr_reg (svhshell_w, "XEphem*SkyHist.y", skycategory, 0);

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNfractionBase, 30); n++;
	svhform_w = XmCreateForm (svhshell_w, "SVHForm", args, n);
	XtAddCallback (svhform_w, XmNhelpCallback, svh_help_cb, 0);
	XtManageChild (svhform_w);

	/* controls attached across the bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 4); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 6); n++;
	w = XmCreatePushButton (svhform_w, "Add", args, n);
	wtip (w, "Add the current Sky View settings to the History list");
	XtAddCallback (w, XmNactivateCallback, svh_add_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 9); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 11); n++;
	w = XmCreatePushButton (svhform_w, "Save", args, n);
	(void) sprintf (buf, "Save the History list to %s/%s", getPrivateDir(),
								    skyhfn);
	wtip (w, XtNewString(buf));	/* must be permanent memory */
	XtAddCallback (w, XmNactivateCallback, svh_save_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 14); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 16); n++;
	w = XmCreatePushButton (svhform_w, "Load", args, n);
	(void) sprintf(buf,"Load the History list from %s/%s else %s/auxil/%s",
			    getPrivateDir(), skyhfn, getShareDir(), skyhfn);
	wtip (w, XtNewString(buf));	/* must be permanent memory */
	XtAddCallback (w, XmNactivateCallback, svh_load_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 19); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 21); n++;
	w = XmCreatePushButton (svhform_w, "Close", args, n);
	wtip (w, "Close this window");
	XtAddCallback (w, XmNactivateCallback, svh_close_cb, 0);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 24); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 26); n++;
	w = XmCreatePushButton (svhform_w, "Help", args, n);
	wtip (w, "Get more info about this window.");
	XtAddCallback (w, XmNactivateCallback, svh_help_cb, 0);
	XtManageChild (w);

	/* SW for RC of entries stretches between top and row of controls */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	sw_w = XmCreateScrolledWindow (svhform_w, "SW", args, n);
	XtManageChild (sw_w);

	    /* add a dummy RC just so it can be replaced when add entries */

	    n = 0;
	    w = XmCreateRowColumn (sw_w, "Dummy", args, n);
	    XtManageChild (w);

	/* init from history file if opens ok */
	hfp = openhist(fn, "r");
	if (hfp) {
	    char buf[1024];
	    if (loadhist (hfp, fn, buf) < 0) {
		xe_msg (1, "%s", buf);
	    } else {
		show_entries();
		xe_msg (0, "%s", buf);	/* just log */
	    }
	    fclose (hfp);
	}
}

/* called to put up or remove the watch cursor.  */
void
svh_cursor (c)
Cursor c;
{
	Window win;

	if (svhshell_w && (win = XtWindow(svhshell_w)) != 0) {
	    Display *dsp = XtDisplay(svhshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* close the History control dialog, if any */
void
svh_unmanage()
{
	if (svhshell_w)
	    XtPopdown (svhshell_w);
}

/* get current settings and assign to next position */
void
svh_add_current()
{
	char buf[32];

	hist = (SVHEntry *) XtRealloc ((char*)hist, (nhist+1)*sizeof(SVHEntry));
	zero_mem ((void *)(&hist[nhist]), sizeof(hist[0]));
	svh_get (&hist[nhist].h);
	(void) sprintf (buf, "New entry %d", ++autonum);
	hist[nhist].ulbl = XtNewString (buf);
	nhist++;
	show_entries ();
}

/* return the current number of history entries */
int
svh_nhist()
{
	return (nhist);
}

/* display each of the nhist SvHistory entries in the dialog. */
static void
show_entries()
{
	Widget ww;
	Arg args[20];
	int i, n;

	/* replace workWindow with a fresh RC */
	get_something (sw_w, XmNworkWindow, (XtArgVal)&ww);
	XtDestroyWidget (ww);	/* destroys all the ltf_w too */
	n = 0;
	XtSetArg (args[n], XmNspacing, 0); n++;
	ww = XmCreateRowColumn (sw_w, "CatRC", args, n);
	set_something (sw_w, XmNworkWindow, (XtArgVal)ww);

	/* fill with info */
	for (i = 0; i < nhist; i++)
	    setup_1row (ww, i);

	/* ok */
	XtManageChild (ww);
}

/* using info from hist[i] create its row in the history window */
static void
setup_1row (rc_w, i)
Widget rc_w;
int i;
{
	Widget f_w, del_w, rep_w, go_w, up_w, down_w, lbl_w, tf_w;
	Arg args[20];
	int n;

	/* create the row's form */
	n = 0;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	f_w = XmCreateForm (rc_w, "HF", args, n);
	XtManageChild (f_w);

	/* PB to Apply */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 2); n++;
	go_w = XmCreatePushButton (f_w, "Apply", args, n);
	XtAddCallback (go_w, XmNactivateCallback, svh_go_cb, (XtPointer)(long int)i);
	wtip (go_w, "Put this History entry into effect");
	XtManageChild (go_w);

	/* PB to move up in list */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, go_w); n++;
	XtSetArg (args[n], XmNarrowDirection, XmARROW_UP); n++;
	up_w = XmCreateArrowButton (f_w, "Up", args, n);
	XtAddCallback (up_w, XmNactivateCallback, svh_up_cb, (XtPointer)(long int)i);
	wtip (up_w, "Move this History entry up one row in the list");
	if (i == 0)
	    XtSetSensitive (up_w, False);
	XtManageChild (up_w);

	/* PB to move down in list */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, up_w); n++;
	XtSetArg (args[n], XmNarrowDirection, XmARROW_DOWN); n++;
	down_w = XmCreateArrowButton (f_w, "Down", args, n);
	XtAddCallback (down_w, XmNactivateCallback, svh_down_cb, (XtPointer)(long int)i);
	wtip (down_w, "Move this History entry down one row in the list");
	if (i == nhist-1)
	    XtSetSensitive (down_w, False);
	XtManageChild (down_w);

	/* PB to delete */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, down_w); n++;
	del_w = XmCreatePushButton (f_w, "Delete", args, n);
	XtAddCallback (del_w, XmNactivateCallback, svh_del_cb, (XtPointer)(long int)i);
	wtip (del_w, "Delete this History entry");
	XtManageChild (del_w);

	/* PB to replace */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, del_w); n++;
	rep_w = XmCreatePushButton (f_w, "Replace", args, n);
	XtAddCallback (rep_w, XmNactivateCallback, svh_rep_cb, (XtPointer)(long int)i);
	wtip (rep_w, "Replace this History entry with current settings");
	XtManageChild (rep_w);

	/* label for info */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, rep_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 70); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	lbl_w = XmCreateLabel (f_w, "TL", args, n);
	setup_label (lbl_w, &hist[i].h);
	XtManageChild (lbl_w);

	/* TF for label */
	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, lbl_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 100); n++;
	XtSetArg (args[n], XmNvalue, hist[i].ulbl); n++;
	tf_w = XmCreateTextField (f_w, "Label", args, n);
	XtAddCallback (tf_w, XmNvalueChangedCallback, svh_ltfc_cb,(XtPointer)(long int)i);
	wtip (tf_w, "Your label for this entry");
	XtManageChild (tf_w);
	hist[i].ltf_w = tf_w;
	XmTextFieldSetString (tf_w, hist[i].ulbl);
}

/* called just as the pulldown menu is cascading */
/* ARGSUSED */
static void
svh_cascading_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i;

	/* assign a PB to each hist[], creating as necessary */
	for (i = 0; i < nhist; i++) {
	    Widget pb;

	    if (i >= npdpb) {
		Arg args[20];
		int n;

		n = 0;
		pb = XmCreatePushButton (pd_w, "HPB", args, n);
		XtAddCallback (pb,XmNactivateCallback, svh_go_cb, (XtPointer)(long int)i);
		wtip (pb, "Apply this History entry");
		pdpb_w = (Widget *) XtRealloc ((char *)pdpb_w,
							++npdpb*sizeof(Widget));
		pdpb_w[i] = pb;
	    } else
		pb = pdpb_w[i];

	    set_xmstring (pb, XmNlabelString, hist[i].ulbl);
	    XtManageChild (pb);
	}

	/* unmanage any excess pulldown buttons */
	while (i < npdpb)
	    XtUnmanageChild (pdpb_w[i++]);
}

/* callback each time a char is typed in a ltf_w.
 * client is index into hist[]
 */
/* ARGSUSED */
static void
svh_ltfc_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i = (long int)client;

	XtFree(hist[i].ulbl);
	hist[i].ulbl = XmTextFieldGetString (hist[i].ltf_w);
}

/* callback from the overall Help button.
 */
/* ARGSUSED */
static void
svh_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"Collects and Installs collections of Sky View settings",
};

	hlp_dialog ("SkyView_history", msg, sizeof(msg)/sizeof(msg[0]));
}

/* callback from the manage PB in the pulldown
 */
/* ARGSUSED */
static void
svh_manage_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopup (svhshell_w, XtGrabNone);
	set_something (svhshell_w, XmNiconic, (XtArgVal)False);
}

/* callback from the Close PB */
/* ARGSUSED */
static void
svh_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (svhshell_w);
}

/* called when the Add button is hit in the history window */
/* ARGSUSED */
static void
svh_add_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	svh_add_current();
}

/* used to delete history entry deli, possibly after confirming */
static int deli;
static void
del_i()
{
	/* free ulbl */
	XtFree (hist[deli].ulbl);

	/* remove deli from hist */
	memcpy (&hist[deli], &hist[deli+1], (nhist-(deli+1))*sizeof(SVHEntry));
	nhist--;

	/* show list now */
	show_entries();
}

/* called when a Delete button is hit in the history window.
 * client is index into hist[]
 */
/* ARGSUSED */
static void
svh_del_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* save index */
	deli = (long int)client;

	/* confirm */
	if (confirm()) {
	    char buf[1024];

	    (void) sprintf (buf, "Delete \"%s\"?", hist[deli].ulbl);
	    query (svhshell_w, buf, "Yes -- delete", "No -- keep it", NULL,
							    del_i, NULL, NULL);
	} else
	    del_i();
}

/* used to replace history entry repi, possibly after confirming */
static int repi;
static void
rep_i()
{
	char *ulbl = hist[repi].ulbl;

	zero_mem ((void *)(&hist[repi]), sizeof(hist[0]));
	svh_get (&hist[repi].h);
	hist[repi].ulbl = ulbl;
	show_entries ();
}

/* called when a Replace button is hit in the history window.
 * client is index into hist[]
 */
/* ARGSUSED */
static void
svh_rep_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* save index */
	repi = (long int)client;

	/* confirm */
	if (confirm()) {
	    char buf[1024];

	    (void) sprintf (buf, "Replace \"%s\"?", hist[repi].ulbl);
	    query (svhshell_w, buf, "Yes -- replace", "No -- keep it", NULL,
							    rep_i, NULL, NULL);
	} else
	    rep_i();
}

/* called when a Go button is hit in the history window.
 * client is index into hist[]
 */
/* ARGSUSED */
static void
svh_go_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i = (long int)client;
	svh_goto (&hist[i].h);
}

/* called when an Up button is hit in the history window.
 * client is index into hist[]
 */
/* ARGSUSED */
static void
svh_up_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i = (long int)client;
	SVHEntry tmp;

	/* nothing to do if chose top already */
	if (i == 0)
	    return;

	/* swap up */
	tmp = hist[i-1];
	hist[i-1] = hist[i];
	hist[i] = tmp;

	/* show list now */
	show_entries();
}

/* called when a Down button is hit in the history window.
 * client is index into hist[]
 */
/* ARGSUSED */
static void
svh_down_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i = (long int)client;
	SVHEntry tmp;

	/* nothing to do if chose end already */
	if (i >= nhist-1)
	    return;

	/* swap down */
	tmp = hist[i+1];
	hist[i+1] = hist[i];
	hist[i] = tmp;

	/* show list now */
	show_entries();
}

/* setup the label lbl_w from the given SvHistory */
static void
setup_label (lbl_w, hp)
Widget lbl_w;
SvHistory *hp;
{
	char *projstr = hp->cyl_proj ? "Cyl" : "Sph";
	char str1[32], str2[32];
	char adstr[32];
	char fovstr[32];
	char buf[128];

	fs_sexa (fovstr, raddeg(hp->fov), 3, 60);

	if (hp->aa_mode) {
	    fs_sexa (str1, raddeg(hp->altdec), 3, 60);
	    fs_sexa (str2, raddeg(hp->azra), 3, 60);
	    strcpy (adstr, "Alt/Az");
	} else {
	    fs_sexa (str1, radhr(hp->azra), 3, 60);
	    fs_sexa (str2, raddeg(hp->altdec), 3, 60);
	    strcpy (adstr, "RA/Dec");
	}

	(void) sprintf (buf,"%s %s %s %s %s", projstr, adstr, str1,str2,fovstr);
	set_xmstring (lbl_w, XmNlabelString, buf);
}

/* called when the Save button is hit */
/* ARGSUSED */
static void
svh_save_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	FILE *histfp;
	SvHistory *hp;
	int i, j;
	char fn[1024];

	histfp = openhist(fn, "w");
	if (!histfp) {
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	    return;
	}

	fprintf(histfp, "# skyview history file\n");
	fprintf(histfp, "Version %d\n", VERSIONN);
	for (i = 0; i < nhist; i++) {
	    hp = &hist[i].h;
	    fprintf(histfp, ":%d\n", i);
	    fprintf(histfp, "%s\n", hist[i].ulbl);
	    fprintf(histfp, "%g %g %g %d %d %d %d %d %d %d %d\n",
		hp->fov, hp->azra, hp->altdec,
		hp->stmag, hp->ssmag, hp->dsmag, hp->magstp,
		hp->grid, hp->autogrid, hp->aagrid, hp->gridlbl);
	    fprintf(histfp, "%*s\n", GRIDLEN, hp->vgrid);
	    fprintf(histfp, "%*s\n", GRIDLEN, hp->hgrid);
	    fprintf(histfp,
		"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		hp->winw, hp->winh, hp->cyl_proj, hp->aa_mode,
		hp->flip_lr, hp->flip_tb,
		hp->justd, hp->eclip, hp->galac, hp->eq,
		hp->hznmap, hp->conf, hp->conb, hp->conn, hp->cona, 
		hp->eyep, hp->magscale, hp->automag,
		hp->lbl_lst, hp->lbl_lfs, hp->lbl_lss, hp->lbl_lds,
		hp->lbl_bst, hp->lbl_bfs, hp->lbl_bss, hp->lbl_bds,
		hp->conr, hp->hznclipping);
	    for (j=0; j < (int)NOBJTYPES; j++) 
		fprintf(histfp, "%c", hp->type_table[j]?'1':'0');
	    fprintf(histfp, "\n");
	    for (j=0; j < (int)NCLASSES; j++)
		fprintf(histfp, "%c", hp->fclass_table[j]?'1':'0');
	    fprintf(histfp, "\n");
	}
	fprintf(histfp, "# end\n");
	(void)fclose(histfp);

	if (confirm())
	    xe_msg (1, "%s saved", fn);
}

/* called when the Load button is hit in the history pulldown */
/* ARGSUSED */
static void
svh_load_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char fn[1024];
	char buf[1024];
	FILE *hfp;

	hfp = openhist(fn, "r");
	if (hfp) {
	    if (loadhist(hfp, fn, buf) < 0)
		xe_msg (1, "%s", buf);
	    else
		show_entries();
	    fclose (hfp);
	} else {
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	}
}

/* load the history file named fn[] already open on histfp.
 * return 0 if ok, -1 if trouble.
 * some sort of message is also always in msg[].
 */
static int
loadhist (histfp, fn, msg)
FILE *histfp;
char *fn;
char *msg;
{
	char buf[1024];
	char *ulbl;
	SvHistory hp;
	int i, j, n;
	int v;

	/* first look for Version */
	while (fgets(buf, sizeof(buf), histfp))
	    if (buf[0] != '#' && buf[0] != '\n')
		break;
	if (sscanf(buf, "Version %d", &v) != 1) {
	    sprintf (msg, "%s:\nNo Version line:\n%s\n", fn, buf);
	    goto err;
	} else if (!(v <= VERSIONN && v >= VERSIONN-2)) {
	    sprintf (msg, "%s:\nWrong version, must be %d.", fn, VERSIONN);
	    goto err;
	}

	/* now loop over each group of lines defining one History entry */
	nhist = 0;
	ulbl = 0;
	while (fgets(buf, sizeof(buf), histfp)) {
	    if (buf[0] == '#' || buf[0] == '\n')
		continue;			/* initial comment */

	    if (sscanf(buf, ":%d", &i) != 1) {
		sprintf (msg, "%s:\nBogus \":<n>\" line:\n%s", fn, buf);
		goto err;
	    }

	    if (!fgets (buf, sizeof(buf), histfp))
		goto err;
	    buf[strlen(buf)-1] = '\0';		/* no \n */
	    ulbl = XtNewString (buf);	/* malloced copy */

	    if (!fgets(buf, sizeof(buf), histfp))
		goto err;
	    if (sscanf(buf, "%lg %lg %lg %d %d %d %d %d %d %d %d",
			&hp.fov, &hp.azra, &hp.altdec,
			&hp.stmag, &hp.ssmag, &hp.dsmag, &hp.magstp,
			&hp.grid, &hp.autogrid, &hp.aagrid, &hp.gridlbl)!=11) {
		sprintf (msg, "%s:\nBogus fov .. \n%s", fn, buf);
		goto err;
	    }

	    if (!fgets(buf, sizeof(buf), histfp))
		goto err;
	    (void) sprintf (hp.vgrid, "%*.*s", GRIDLEN, GRIDLEN, buf);

	    if (!fgets(buf, sizeof(buf), histfp))
		goto err;
	    (void) sprintf (hp.hgrid, "%*.*s", GRIDLEN, GRIDLEN, buf);

	    if (!fgets(buf, sizeof(buf), histfp))
		goto err;
	    n = sscanf(buf,
		"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		    &hp.winw, &hp.winh, &hp.cyl_proj, &hp.aa_mode,
		    &hp.flip_lr, &hp.flip_tb,
		    &hp.justd, &hp.eclip, &hp.galac, &hp.eq,
		    &hp.hznmap, &hp.conf, &hp.conb, &hp.conn, &hp.cona,
		    &hp.eyep, &hp.magscale, &hp.automag,
		    &hp.lbl_lst, &hp.lbl_lfs, &hp.lbl_lss, &hp.lbl_lds,
		    &hp.lbl_bst, &hp.lbl_bfs, &hp.lbl_bss, &hp.lbl_bds,
		    &hp.conr, &hp.hznclipping );
	    if (v == 4 && n == 27) {
		hp.hznclipping = 1;
	    } else if (v == 3 && n == 24) {
		hp.lbl_bds = hp.lbl_bfs;
		hp.lbl_bss = hp.lbl_bst;
		hp.lbl_bst = hp.lbl_lds;
		hp.lbl_lds = hp.lbl_lss;
		hp.lbl_lss = hp.lbl_lfs;

		hp.lbl_bss = 10;
		hp.lbl_bds = 10;
		hp.conr = 0;
		hp.hznclipping = 1;
	    } else if (!(v == VERSIONN && n == 28)) {
		sprintf (msg, "%s:\nBogus switches line .. \n%s", fn, buf);
		goto err;
	    }

	    if (!fgets(buf, sizeof(buf), histfp))
		goto err;
	    for (j = 0; j < sizeof(hp.type_table); j++) {
		switch (buf[j]) {
		case '0': hp.type_table[j] = 0; break;
		case '1': hp.type_table[j] = 1; break;
		default: goto err;
		}
	    }

	    if (!fgets(buf, sizeof(buf), histfp))
		goto err;
	    for (j = 0; j < sizeof(hp.fclass_table); j++) {
		switch (buf[j]) {
		case '0': hp.fclass_table[j] = 0; break;
		case '1': hp.fclass_table[j] = 1; break;
		default: goto err;
		}
	    }

	    /* got one! */
	    hist = (SVHEntry *) XtRealloc ((char *)hist,
						    (nhist+1)*sizeof(SVHEntry));
	    memcpy(&hist[nhist].h, &hp, sizeof(hp));
	    hist[nhist].ulbl = ulbl;
	    nhist++;
	}

	sprintf (msg, "%s:\nLoaded %d skyhistory entries.", fn, nhist);
	return (0);

    err:
	/* overwrite err if something more basic is wrong */
	if (ferror(histfp))
	    sprintf (msg, "%s:\n%s", fn, syserrstr());
	else if (feof(histfp))
	    sprintf (msg, "%s:\nUnexpected EOF\n", fn);

	for (i = 0; i < nhist; i++)
	    XtFree (hist[i].ulbl);
	nhist = 0;
	return (-1);
}

/* fill fn with full history file name and try to open.
 * if opening for write try to create in PrivateDir.
 */
static FILE *
openhist(fn, how)
char *fn;
char *how;
{
	FILE *fp;

	/* first try local copy, r or w */
	(void) sprintf (fn, "%s/%s", getPrivateDir(), skyhfn);
	fp = fopenh (fn, how);

	/* if fails try global copy if for r */
	if (!fp && how[0] == 'r') {
	    (void) sprintf (fn, "%s/auxil/%s", getShareDir(), skyhfn);
	    fp = fopenh (fn, how);
	}

	return (fp);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: skyhist.c,v $ $Date: 2012/07/07 18:04:42 $ $Revision: 1.40 $ $Name:  $"};

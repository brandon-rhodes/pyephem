/* code to manage the stuff on the "plot" menu.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/FileSB.h>
#include <Xm/Label.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>

#include "xephem.h"
#include "lilxml.h"

static void plt_select (int whether);
static void plot_create_shell (void);
static void plt_active_cb (Widget w, XtPointer client, XtPointer call);
static void plt_select_cb (Widget w, XtPointer client, XtPointer call);
static void plt_show_cb (Widget w, XtPointer client, XtPointer call);
static void plt_savecfg_cb (Widget w, XtPointer client, XtPointer call);
static void plt_loadcfg_cb (Widget w, XtPointer client, XtPointer call);
static void plt_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void plt_close_cb (Widget w, XtPointer client, XtPointer call);
static void plt_help_cb (Widget w, XtPointer client, XtPointer call);
static void plt_undo_cb (Widget w, XtPointer client, XtPointer call);
static void plt_reset (void);
static void plt_stop_selecting (void);
static void plt_turn_off (void);
static void set_x (char *name);
static void set_y (char *name);
static void add_field (char *name);
static void init_row (void);
static void plt_try_append (void);
static void plt_try_overwrite (void);
static void plt_try_cancel (void);
static void plt_try_turn_on (void);
static void plt_turn_on (char *how);
static void plt_da_manage (char *fn);
static void plt_da_destroy (Widget da_w);
static void plt_print (void);
static void plt_da_print_cb (Widget w, XtPointer client, XtPointer call);
static void plt_da_mloop_cb (Widget w, XtPointer client, XtPointer call);
static void plt_da_close_cb (Widget w, XtPointer client, XtPointer call);
static void plt_da_unmap_cb (Widget w, XtPointer client, XtPointer call);
static void plt_da_flipx_cb (Widget w, XtPointer client, XtPointer call);
static void plt_da_flipy_cb (Widget w, XtPointer client, XtPointer call);
static void plt_da_grid_cb (Widget w, XtPointer client, XtPointer call);
static void plt_da_xyr_asdate_cb (Widget w, XtPointer client, XtPointer call);
static void plt_da_xjd_asdate_cb (Widget w, XtPointer client, XtPointer call);
static void plt_da_exp_cb (Widget da_w, XtPointer client, XtPointer call);
static void plt_da_motion_eh (Widget da_w, XtPointer client, XEvent *ev, Boolean *continue_to_dispatch);
static void plt_drawall (Widget da_w);
static int plt_ano (double *sx, double *sy, int *xp, int *yp, int w2x,int arg);

#define	MAXFLDNAM	32	/* longest allowed field name */

/* column indeces */
typedef enum {
    T=0		/* tag label text field */,
    X		/* X label */,
    Y		/* Y label */,
    FORM	/* the form that holds the three above */,
    NPC		/* number of columns */
} PCols;

static Widget plotshell_w;	/* main shell */
static Widget select_w;		/* TB for selecting plot fields */
static Widget active_w;		/* TB for building plot file */
static Widget prompt_w;		/* what to do next label */
static Widget title_w;		/* TF for user's plot title */
static Widget pfn_w;		/* TF for new plot file name */
static Widget cfn_w;		/* TF for new config file name */
static Widget undo_w;		/* PB for undoing a field selection */
static int selecting_xy;	/* one of X or Y */

static FILE *plt_fp;            /* the plot file; == 0 means don't plot */

static Widget da_w_save;	/* used to set up for a print action */
static char config_suffix[] = ".ptc";	/* file ext for plot config files */
static char plot_suffix[] = ".plt";	/* file ext for plot files */

static char plotcategory[] = "Tools -- Plot";	/* Save category */
static char xml_plotele[] = "XEphemPlotConfig";

/* store the name of each x and y line to track and their values.
 * we get the tag straight from the Text widget in the table as needed.
 */
typedef struct {
    char pl_xn[MAXFLDNAM];	/* name of x field, or 0 if none */
    double pl_xv;		/* last known value of x field */
    char pl_yn[MAXFLDNAM];	/* name of y field, or 0 if none */
    double pl_yv;		/* last known value of x field */
    Widget c_w[NPC];		/* column displays */
} PltLine;
static PltLine pltlines[MAXPLTLINES];
static int npltlines;		/* number of completely defined plots */

/* called when the plot menu is activated via the main menu pulldown.
 * if never called before, create and manage all the widgets as a child of a
 * form. otherwise, just get going.
 */
void
plot_manage ()
{
	if (!plotshell_w)
	    plot_create_shell();
	
	XtPopup (plotshell_w, XtGrabNone);
	set_something (plotshell_w, XmNiconic, (XtArgVal)False);
}

/* called by the other menus (data, etc) as their buttons are
 * selected to inform us that that button is to be included in a plot.
 */
void
plt_selection (name)
char *name;
{
	if (!isUp(plotshell_w) || !XmToggleButtonGetState(select_w))
	    return;

	add_field (name);
}

/* called as each different field is written -- just save in pltlines[]
 * if we are interested in it.
 * might have the same field listed more than once so can't stop if find one.
 */
void
plt_log (name, value)
char *name;
double value;
{
	if (plt_fp) {
	    PltLine *plp;
	    for (plp = pltlines; plp < &pltlines[npltlines]; plp++) {
		if (strcmp (name, plp->pl_xn) == 0)
		    plp->pl_xv = value;
		if (strcmp (name, plp->pl_yn) == 0)
		    plp->pl_yv = value;
	    }
	}
}

/* called when all fields have been updated and it's time to
 * write the active plotfields to the current plot file, if one is open.
 */
void
plot()
{
	if (plt_fp) {
	    /* plot in order of original selection */
	    PltLine *plp;
	    for (plp = pltlines; plp < &pltlines[npltlines]; plp++) {
		char *lbl = XmTextFieldGetString(pltlines[plp-pltlines].c_w[T]);
		(void) fprintf (plt_fp, "%s,%.12g,%.12g\n", lbl,
						    plp->pl_xv, plp->pl_yv);
		XtFree (lbl);
	    }
	}
}

int
plot_ison()
{
	return (plt_fp != 0);
}

/* called to put up or remove the watch cursor.  */
void
plt_cursor (c)
Cursor c;
{
	Window win;

	if (plotshell_w && (win = XtWindow(plotshell_w)) != 0) {
	    Display *dsp = XtDisplay(plotshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* inform the other menues whether we are setting up for them to tell us
 * what fields to plot.
 */
static void
plt_select(whether)
int whether;
{
	all_selection_mode(whether);
}

static void
plot_create_shell()
{
	XmString str;
	Widget f_w, rc_w;
	Widget pf_w, oms_w, omc_w;
	Widget w, cfg_w;
	Arg args[20];
	char *s[1];
	int i, n;

	/* create the main dialog */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Plot Control"); n++;
	XtSetArg (args[n], XmNiconName, "Plot"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	plotshell_w = XtCreatePopupShell ("Plot", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (plotshell_w);
	set_something (plotshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (plotshell_w, XmNpopdownCallback, plt_popdown_cb, NULL);
	sr_reg (plotshell_w, "XEphem*Plot.x", plotcategory, 0);
	sr_reg (plotshell_w, "XEphem*Plot.y", plotcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNresizePolicy, XmRESIZE_ANY); n++;
	pf_w = XmCreateForm (plotshell_w, "PlotF", args, n);
	XtAddCallback (pf_w, XmNhelpCallback, plt_help_cb, 0);
	XtManageChild (pf_w);

	/* control to allow selecting an existing plot file to display */

	s[0] = plot_suffix;
	oms_w = createFSM (pf_w, s, 1, "auxil", plt_show_cb);
	wtip (oms_w, "Select a plot configuration file to load");
	set_xmstring (oms_w, XmNlabelString, "Show plot file:     ");
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetValues (oms_w, args, n);

	/* control to allow selecting a config file to load */

	s[0] = config_suffix;
	omc_w = createFSM (pf_w, s, 1, "auxil", plt_loadcfg_cb);
	set_xmstring (omc_w, XmNlabelString, "Load config file:   ");
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, oms_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetValues (omc_w, args, n);

	/* save config TB and its TF */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, omc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	cfg_w = XmCreateToggleButton(pf_w, "PSTB", args, n);
	XtAddCallback(cfg_w, XmNvalueChangedCallback, plt_savecfg_cb, NULL);
	set_xmstring (cfg_w, XmNlabelString, "Save config file: ");
	wtip (cfg_w, "Save current plot definition to file");
	XtManageChild (cfg_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, omc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, cfg_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	cfn_w = XmCreateTextField (pf_w, "ConfigFile", args, n);
	defaultTextFN (cfn_w, 0, "myconfig.ptc", NULL);
	wtip (cfn_w,
		"Enter name of file in which to save current plot definition");
	XtManageChild (cfn_w);
	sr_reg (cfn_w, NULL, plotcategory, 0);

	/* field Select PB */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, cfn_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	select_w = XmCreateToggleButton(pf_w, "PSTB", args, n);
	XtAddCallback(select_w, XmNvalueChangedCallback, plt_select_cb, NULL);
	set_xmstring (select_w, XmNlabelString, "Select fields to plot");
	wtip (select_w, "Build plot functions from selectable buttons");
	XtManageChild (select_w);

	/* create plot TB and its TF */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, select_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	active_w = XmCreateToggleButton(pf_w, "PCTB", args, n);
	XtAddCallback(active_w, XmNvalueChangedCallback, plt_active_cb, NULL);
	set_xmstring (active_w, XmNlabelString, "Create plot file: ");
	wtip (active_w,
		"Selected fields are written to the named file at each Update");
	XtManageChild (active_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, select_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, active_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	pfn_w = XmCreateTextField (pf_w, "PlotFile", args, n);
	defaultTextFN (pfn_w, 0, "myplot.plt", NULL);
	wtip (pfn_w, "Enter name of plot file to create");
	XtManageChild (pfn_w);
	sr_reg (pfn_w, NULL, plotcategory, 0);

	/* make a RowColumn to everything else */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, pfn_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNspacing, 6); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNadjustMargin, False); n++;
	rc_w = XmCreateRowColumn(pf_w, "PlotRC", args, n);
	XtManageChild (rc_w);

	/* create title text area and its label */

	n = 0;
	str = XmStringCreate("Title:", XmSTRING_DEFAULT_CHARSET);
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	XtSetArg (args[n], XmNlabelString, str); n++;
	w = XmCreateLabel (rc_w, "PlotTL", args, n);
	XtManageChild (w);
	XmStringFree (str);

	n = 0;
	XtSetArg (args[n], XmNcolumns, 40); n++;
	title_w = XmCreateTextField (rc_w, "PlotTitle", args, n);
	wtip (title_w, "Enter a title to be written to the file");
	XtManageChild (title_w);

	/* create prompt line -- it will be managed as necessary */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	prompt_w = XmCreateLabel (rc_w, "PlotPrompt", args, n);

	/* create the table.
	 * each row is in a form to control its shape.
	 * loop index of -1 is used to make the column headings.
	 * the table entries are all managed but the forms are not at this time.
	 */

	for (i = -1; i < MAXPLTLINES; i++) {
	    n = 0;
	    f_w = XmCreateForm (rc_w, "PlotTF", args, n);

	    /* save the form unless we are just making the column headings */
	    if (i == -1)
		XtManageChild (f_w);
	    else
		pltlines[i].c_w[FORM] = f_w;

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 0); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 20); n++;
	    if (i == -1) {
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
		w = XmCreateLabel (f_w, "Tag", args, n);
		wtip (w, "Column of tags for each X/Y pair to plot");
		XtManageChild (w);
	    } else {
		XtSetArg (args[n], XmNmaxLength, MAXTAG); n++;
		XtSetArg (args[n], XmNcolumns, MAXTAG); n++;
		pltlines[i].c_w[T] = XmCreateTextField (f_w, "PlotTag", args,n);
		XtManageChild (pltlines[i].c_w[T]);
	    }

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 30); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 65); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    if (i == -1) {
		w = XmCreateLabel (f_w, "_X_", args, n);
		wtip (w, "Column of X fields for each X/Y pair to plot");
		XtManageChild (w);
	    } else {
		pltlines[i].c_w[X] = XmCreateLabel (f_w, "PlotX", args, n);
		XtManageChild (pltlines[i].c_w[X]);
	    }

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 65); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 100); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    if (i == -1) {
		w = XmCreateLabel (f_w, "_Y_", args, n);
		wtip (w, "Column of Y fields for each X/Y pair to plot");
		XtManageChild (w);
	    } else {
		pltlines[i].c_w[Y] = XmCreateLabel (f_w, "PlotY", args, n);
		XtManageChild (pltlines[i].c_w[Y]);
	    }
	}

	/* create a separator */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep", args, n);
	XtManageChild (w);

	/* make a form to hold the close and help buttons evenly */

	n = 0;
	XtSetArg (args[n], XmNfractionBase, 10); n++;
	f_w = XmCreateForm (rc_w, "PlotCF", args, n);
	XtManageChild(f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 1); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 3); n++;
	    w = XmCreatePushButton (f_w, "Close", args, n);
	    wtip (w, "Close this dialog ((but continue plotting if active)");
	    XtAddCallback (w, XmNactivateCallback, plt_close_cb, 0);
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
	    wtip (undo_w, "Undo last choice)");
	    XtAddCallback (undo_w, XmNactivateCallback, plt_undo_cb, 0);
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
	    XtAddCallback (w, XmNactivateCallback, plt_help_cb, 0);
	    XtManageChild (w);
}

/* callback from one of the file names in the Show list.
 * file name is label of this widget
 */
/* ARGSUSED */
static void
plt_show_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *fn;

	/* turn off plotting, if on, to make sure file is complete. */
	if (XmToggleButtonGetState(active_w))
	    XmToggleButtonSetState(active_w, False, True);

	get_xmstring (w, XmNlabelString, &fn);
	plt_da_manage(fn);
	XtFree (fn);
}

/* callback from one of the config file names in load.
 * file name is label of this widget
 */
/* ARGSUSED */
static void
plt_loadcfg_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024];
	FILE *fp;
	char *fn;

	/* turn off plotting and selecting, if on */
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
	    else if (strcmp (tagXMLEle(pele), xml_plotele)) {
		xe_msg (1, "Not a valid plot config file");
	    } else {
		XMLEle *ep;

		plt_reset();
		init_row();	/* set first tag to something and show it */
		selecting_xy = X;
		for (ep= nextXMLEle(pele,1); ep != NULL; ep=nextXMLEle(pele,0)){
		    if (strcmp (tagXMLEle(ep), "plot") == 0) {
			XmTextFieldSetString (pltlines[npltlines].c_w[T],
					findXMLAttValu (ep, "tag"));
			add_field (findXMLAttValu (ep, "x"));
			add_field (findXMLAttValu (ep, "y"));
		    }
		}

		XmTextFieldSetString (title_w,
					pcdataXMLEle(findXMLEle(pele,"title")));
		plt_stop_selecting();
	    }
	    delXMLEle (pele);
	    delLilXML (lp);
	    fclose (fp);
	}

	XtFree (fn);
}

/* callback to save current configuration to file named by cfn_w. */
/* ARGSUSED */
static void
plt_savecfg_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	FILE *fp;
	char *txt, *fn;
	char buf[1024];
	int i;

	/* ignore if turning off */
	if (!XmToggleButtonGetState(w))
	    return;

	/* stop selecting if on */
	if (XmToggleButtonGetState (select_w))
	    XmToggleButtonSetState (select_w, False, True);

	/* get config file name, add suffix if not already */
	fn = txt = XmTextFieldGetString (cfn_w);
	if (!strstr (txt, config_suffix)) {
	    sprintf (fn = buf, "%s%s", txt, config_suffix);
	    XmTextFieldSetString (cfn_w, fn);
	}

	/* save each entry */
	fp = fopend (fn, NULL, "w");
	if (fp) {
	    char *title = XmTextFieldGetString (title_w);
	    fprintf (fp, "<%s>\n", xml_plotele);
	    fprintf (fp, "  <title>%s</title>\n", title);
	    for (i = 0; i < npltlines; i++) {
		PltLine *p = &pltlines[i];
		char *tn = XmTextFieldGetString (p->c_w[T]);
		fprintf (fp, "  <plot tag='%s' x='%s' y='%s' />\n", tn,
							p->pl_xn, p->pl_yn);
		XtFree (tn);
	    }
	    fprintf (fp, "</%s>\n", xml_plotele);
	    XtFree (title);
	    fclose (fp);
	}

	/* act like a push button - give chance to see change. */
	if (confirm())
	    xe_msg (1, "%s saved", fn);
	XmToggleButtonSetState(w, False, True);

	/* finished */
	XtFree (txt);
}

/* callback to allow selecting plot fields */
/* ARGSUSED */
static void
plt_select_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XmToggleButtonGetState(w)) {
	    /* first turn off plotting, if on, while we change things */
	    if (XmToggleButtonGetState(active_w))
		XmToggleButtonSetState(active_w, False, True);
	    plt_reset();	/* reset pltlines array and unmanage the table*/
	    plt_select(1);	/* inform other menus to inform us of fields */
	    init_row();		/* set first tag to something and show it */
	    selecting_xy = X;
	} else
	    plt_stop_selecting();
}

/* callback when user wants to start building a plot file */
/* ARGSUSED */
static void
plt_active_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XmToggleButtonGetState(w)) {
	    /* first turn off selecting, if on */
	    if (XmToggleButtonGetState(select_w))
		XmToggleButtonSetState(select_w, False, True);
	    if (npltlines > 0)
		plt_try_turn_on();
	    else {
		xe_msg (1, "Please select at least one pair of values to plot");
		XmToggleButtonSetState (w, False, False);
	    }
	} else
	    plt_turn_off();
}

/* callback when the main plot window is closed */
/* ARGSUSED */
static void
plt_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
}

/* callback from the Close button.
 */
/* ARGSUSED */
static void
plt_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (plotshell_w);
}

/* callback from the Help button.
 */
/* ARGSUSED */
static void
plt_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
static char *help_msg[] = {
"This menu controls the plot generation and display functionality of xephem.",
"Select fields to form x/y pairs, enable plotting to write them to a file on",
"each xephem iteration step, then view. Each file may be titled, as desired."
};
	hlp_dialog ("Plot", help_msg, sizeof(help_msg)/sizeof(help_msg[0]));
}

/* callback from the Undo button.
 */
/* ARGSUSED */
static void
plt_undo_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (selecting_xy == X) {
	    if (npltlines > 0) {
		/* delete row and previous y */
		XtUnmanageChild (pltlines[npltlines--].c_w[FORM]);
		set_y ("");
		selecting_xy = Y;
	    }
	} else if (selecting_xy == Y) {
	    /* clear x */
	    set_x ("");
	    selecting_xy = X;
	}
}

/* forget our list, and unmanage the table.
 */
static void
plt_reset()
{
	int i;

	for (i = 0; i < npltlines; i++)
	    XtUnmanageChild (pltlines[i].c_w[FORM]);

	npltlines = 0;
}

/* stop selecting: unmanage a partitially filled in line specs; tell
 * everybody else to drop their buttons, make sure toggle is off.
 */
static void
plt_stop_selecting()
{
	if (npltlines < MAXPLTLINES)
	    XtUnmanageChild (pltlines[npltlines].c_w[FORM]);

	XmToggleButtonSetState (select_w, False, False);
	plt_select(0);
	XtUnmanageChild (prompt_w);
	XtSetSensitive (undo_w, False);
}

static void
plt_turn_off ()
{
	if (plt_fp) {
	    (void) fclose (plt_fp);
	    plt_fp = 0;
	}
}

/* set name as X coord on current row and change prompt to ask for Y */
static void
set_x (name)
char *name;
{
	(void) strncpy (pltlines[npltlines].pl_xn, name, MAXFLDNAM-1);
	set_xmstring (pltlines[npltlines].c_w[X], XmNlabelString, name);
	set_xmstring (prompt_w, XmNlabelString, "Choose field for Y ..");
	XtManageChild (prompt_w);
}

/* set name as Y coord on current row */
static void
set_y (name)
char *name;
{
	(void) strncpy (pltlines[npltlines].pl_yn, name, MAXFLDNAM-1);
	set_xmstring (pltlines[npltlines].c_w[Y], XmNlabelString, name);
}

/* init npltlines'th row and change prompt to ask for X */
static void
init_row ()
{
	char buf[100];

	(void) sprintf (buf, "P%02d", npltlines+1);
	XmTextFieldSetString (pltlines[npltlines].c_w[T], buf);

	set_xmstring (prompt_w, XmNlabelString, "Choose field for X ..");
	XtManageChild (prompt_w);

	set_xmstring (pltlines[npltlines].c_w[X], XmNlabelString, " ");
	set_xmstring (pltlines[npltlines].c_w[Y], XmNlabelString, " ");

	XtManageChild (pltlines[npltlines].c_w[FORM]);

	XtSetSensitive (undo_w, True);
}

/* add the next field to the new plot configuration being build */
static void
add_field (char *name)
{
	if (selecting_xy == X) {
	    set_x (name);
	    selecting_xy = Y;
	} else {
	    set_y (name);
	    ++npltlines;
	    if (npltlines == MAXPLTLINES)
		plt_stop_selecting();
	    else {
		init_row();
		selecting_xy = X;
	    }
	}
}

/* called from the query routine when want to append to an existing plot file.*/
static void
plt_try_append()
{
	plt_turn_on("a");
}

/* called from the query routine when want to overwrite to an existing plot
 * file.
 */
static void
plt_try_overwrite()
{
	plt_turn_on("w");
}

/* called from the query routine when want decided not to make a plot file.  */
static void
plt_try_cancel()
{
	XmToggleButtonSetState (active_w, False, False);
}

/* attempt to open file for use as a plot file.
 * if it doesn't exist, then go ahead and make it.
 * but if it does, first ask wheher to append or overwrite.
 */
static void
plt_try_turn_on()
{
	char *txt;
	char buf[1024], *fn;

	/* add suffix if not already */
	fn = txt = XmTextFieldGetString (pfn_w);
	if (!strstr (txt, plot_suffix))
	    sprintf (fn = buf, "%s%s", txt, plot_suffix);

	/* create, or modify if ok */
	if (existd (fn,NULL) == 0 && confirm()) {
	    char buf[256];
	    (void) sprintf (buf, "%s exists:\nAppend or Overwrite?", fn);
	    query (plotshell_w, buf, "Append", "Overwrite", "Cancel",
			    plt_try_append, plt_try_overwrite, plt_try_cancel);
	} else
	    plt_try_overwrite();

	XtFree (txt);
}

/* turn on plotting.
 * establish a file to use (and thereby set plt_fp, the plotting_is_on flag).
 */
static void
plt_turn_on (how)
char *how;	/* fopen how argument */
{
	char buf[1024], *fn;
	char *txt;

	/* add suffix if not already */
	fn = txt = XmTextFieldGetString (pfn_w);
	if (!strstr (txt, plot_suffix)) {
	    sprintf (fn = buf, "%s%s", txt, plot_suffix);
	    XmTextFieldSetString (pfn_w, fn);
	}

	/* plotting is on if file opens ok */
	plt_fp = fopend (fn, NULL, how);
	if (!plt_fp)
	    XmToggleButtonSetState (active_w, False, False);
	XtFree (txt);
	
	if (plt_fp) {
	    /* add a title if it's not null */
	    txt = XmTextFieldGetString (title_w);
	    if (txt[0] != '\0')
		(void) fprintf (plt_fp, "* %s\n", txt);
	    XtFree (txt);
	}
}

/* make a new drawing area widget and manage it. it's unmanaged and destroyed
 *   from the Close button or if something goes wrong during plotting.
 * open the plot file and save it, the current state of the flipx/flipy/grid
 *   buttons and the filename in a DrawInfo struct in the userData resource
 *   for the FormDialog where the drawingarea callback can get at it each time.
 * this way, we can have lots of different plots up at once yet we don't
 *   have to keep track of them.
 * by leaving the file open, we gain some protection against it being removed
 *   or renamed.
 */
static void
plt_da_manage(char *fn)
{
	Widget daform_w;
	Widget da_w, w;
	Widget mb_w, pd_w, cb_w;
	XmString str;
	EventMask mask;
	Arg args[20];
	char titlebuf[64];
	int n;
	DrawInfo *di;
	FILE *fp;

	/* first make sure we can open the plot file */
	fn = XtNewString (fn);
	fp = fopend (fn, "auxil", "r");
	if (!fp) {
	    XtFree (fn);
	    return;
	}

	/* create the form dialog parent. */
	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	daform_w = XmCreateFormDialog (plotshell_w, "PlotDisplay", args, n);
	set_something (daform_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (daform_w, XmNmapCallback, prompt_map_cb, NULL);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	(void) sprintf (titlebuf, "xephem Plot of `%.*s'",
						(int)sizeof(titlebuf)-20, fn);
	n = 0;
	XtSetArg (args[n], XmNtitle, titlebuf); n++;
	XtSetValues (XtParent(daform_w), args, n);

	/* make the DrawInfo structure and save it in the userData of the Form.
	 * the memory gets freed when the dialog is closed/unmanaged.
	 * the options get inited from their toggle buttons.
	 */
	di = (DrawInfo *) XtMalloc (sizeof(DrawInfo));
	di->filename = fn;
	di->fp = fp;
	set_something (daform_w, XmNuserData, (XtArgVal)di);

	/* create the menu bar across the top */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (daform_w, "MB", args, n);
	XtManageChild (mb_w);
	
	/* create the drawing area and connect plt_da_exp_cb().
	 * N.B. be sure this guys parent is the FormDialog so exp_cb can find
	 *   the DrawInfo by looking there at its userData.
	 * make this as high as it is wide when it is first mapped.
	 * N.B. if ever want this in a frame beware that other functions
	 *   assume that the daform_w is the parent of the DrawingArea.
	 */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 2); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNmarginWidth, 0); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	da_w = XmCreateDrawingArea (daform_w, "PlotMap", args, n);
	XtAddCallback (da_w, XmNexposeCallback, plt_da_exp_cb, NULL);
	mask = Button1MotionMask | PointerMotionMask | PointerMotionHintMask;
	XtAddEventHandler (da_w, mask, False, plt_da_motion_eh, 0);
	XtManageChild (da_w);

	/* when unmap the form, pass the da to the unmap callback */
	XtAddCallback (daform_w, XmNunmapCallback, plt_da_unmap_cb,
							    (XtPointer)da_w);

	/* make the Control pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "ControlPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'C'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "ControlCB", args, n);
	    set_xmstring (cb_w, XmNlabelString, "Control");
	    XtManageChild (cb_w);

	    /* make the print button */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "PltPrint", args, n);
	    XtAddCallback (w, XmNactivateCallback, plt_da_print_cb, da_w);
	    set_xmstring (w, XmNlabelString, "Print...");
	    wtip (w, "Print this plot");
	    XtManageChild (w);

	    /* make the annot button */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "Ann", args, n);
	    XtAddCallback (w, XmNactivateCallback, ano_cb, NULL);
	    set_xmstring (w, XmNlabelString, "User annotation...");
	    wtip (w, "Open window to create and manage your own annotation");
	    XtManageChild (w);

	    /* make the movie loop button */
	    n = 0;
	    n += ml_addacc (args, n);
	    w = XmCreatePushButton (pd_w, "ML", args, n);
	    XtAddCallback (w, XmNactivateCallback, plt_da_mloop_cb, da_w);
	    set_xmstring (w, XmNlabelString, "Add to movie...");
	    wtip (w, "Add this plot to the movie loop");
	    XtManageChild (w);

	    /* add a separator */
	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    /* make the close button.
	     * it destroys the dialog and frees the DrawInfo struct.
	     */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "Close", args, n);
	    wtip (w, "Close this plot display");
	    XtAddCallback (w, XmNactivateCallback, plt_da_close_cb, daform_w);
	    XtManageChild (w);

	/* make the View pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "ViewPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'V'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "ViewCB", args, n);
	    set_xmstring (cb_w, XmNlabelString, "View");
	    XtManageChild (cb_w);

	    str = XmStringCreate("Flip X", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateToggleButton(pd_w, "FlipX", args, n);
	    di->flipx = XmToggleButtonGetState (w);
	    di->fx_w = w;
	    XmStringFree (str);
	    XtAddCallback (w, XmNvalueChangedCallback, plt_da_flipx_cb, da_w);
	    wtip (w, "Flip left-to-right");
	    XtManageChild (w);
	    /* TODO - can not use until fix findWFB()
	     * sr_reg (w, NULL, plotcategory, 1);
	     */

	    str = XmStringCreate("Flip Y", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateToggleButton(pd_w, "FlipY", args, n);
	    di->flipy = XmToggleButtonGetState (w);
	    di->fy_w = w;
	    XmStringFree (str);
	    XtAddCallback (w, XmNvalueChangedCallback, plt_da_flipy_cb, da_w);
	    wtip (w, "Flip top-to-bottom");
	    XtManageChild (w);
	    /* TODO - can not use until fix findWFB()
	     * sr_reg (w, NULL, plotcategory, 1);
	     */

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    w = XmCreateToggleButton(pd_w, "Grid", args, n);
	    di->grid = XmToggleButtonGetState (w);
	    di->g_w = w;
	    XtAddCallback (w, XmNvalueChangedCallback, plt_da_grid_cb, da_w);
	    wtip (w, "Overlay plot with calibrated grid");
	    XtManageChild (w);
	    /* TODO - can not use until fix findWFB()
	     * sr_reg (w, NULL, plotcategory, 1);
	     */

	    str = XmStringCreate("Show X-Axis Years as Dates",
						    XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateToggleButton(pd_w, "XYrAsDate", args, n);
	    di->xyr_asdate = XmToggleButtonGetState (w);
	    di->yas_w = w;
	    XmStringFree (str);
	    XtAddCallback (w, XmNvalueChangedCallback, plt_da_xyr_asdate_cb, da_w);
	    wtip (w, "Assume ordinate values are decimal years and display as dates");
	    XtManageChild (w);
	    /* TODO - can not use until fix findWFB()
	     * sr_reg (w, NULL, plotcategory, 1);
	     */

	    str = XmStringCreate("Show X-Axis JDs as Dates",
						    XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateToggleButton(pd_w, "XJDAsDate", args, n);
	    di->xjd_asdate = XmToggleButtonGetState (w);
	    di->jas_w = w;
	    XmStringFree (str);
	    XtAddCallback (w, XmNvalueChangedCallback, plt_da_xjd_asdate_cb, da_w);
	    wtip (w, "Assume ordinate values are Julian dates and display as dates");
	    XtManageChild (w);
	    /* TODO - can not use until fix findWFB()
	     * sr_reg (w, NULL, plotcategory, 1);
	     */

	/* go. the expose will do the actual plotting */
	XtManageChild (daform_w);
}

/* called with the DrawingArea when finished with a plot.
 * use it to get the FormDialog parent and reclaim all memory in its userData
 *   DrawInfo and destroy the Form and hence everything in it.
 */
static void
plt_da_destroy (da_w)
Widget da_w;
{
	Widget daform_w = XtParent(da_w);
	DrawInfo *di;

	get_something (daform_w, XmNuserData, (XtArgVal)&di);
	(void) fclose (di->fp);
	XtFree(di->filename);
	XtFree((char *)di);
	/* TODO: don't do these until fix findWFB() 
	sr_unreg (di->fx_w);
	sr_unreg (di->fy_w);
	sr_unreg (di->g_w);
	sr_unreg (di->as_w);
	*/

	/* destroy the shell, not just the form */
	while (!XtIsWMShell (daform_w))
	    daform_w = XtParent(daform_w);
	XtDestroyWidget(daform_w);

	/* invalidate the saved print widget if it matches */
	if (da_w_save == da_w)
	    da_w_save = 0;
}

/* proceed to generate a postscript file from da_w_save if it is still up.
 * reset da_w_save and call XPSClose() when finished.
 */
static void
plt_print()
{
	if (da_w_save) {
	    Dimension w, h;
	    int iw, ih;

	    get_something (da_w_save, XmNwidth, (XtArgVal)&w);
	    get_something (da_w_save, XmNheight, (XtArgVal)&h);
	    iw = (int)w;
	    ih = (int)h;

	    /* draw in an area 6.5w x 8h centered 1in down from top */
	    if (16*iw >= 13*ih)
		XPSXBegin (XtWindow(da_w_save), 0, 0, iw, ih, 1*72, 10*72,
							    (int)(6.5*72));
	    else {
		int pw = 72*8*iw/ih;	/* width on paper when 8 hi */
		XPSXBegin (XtWindow(da_w_save), 0, 0, iw, ih,
					    (int)((8.5*72-pw)/2), 10*72, pw);
	    }

	    plt_drawall (da_w_save);
	    XPSXEnd();
	    XPSClose();
	    da_w_save = 0;
	} else {
	    xe_msg (1, "Can not print after plot has been dismissed.");
	    XPSClose();
	}
}

/* called when the Print button is pushed on a plot.
 * store the da_w (passed in client) and ask for print setup info.
 */
/* ARGSUSED */
static void
plt_da_print_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Widget da_w = (Widget)client;

	da_w_save = da_w;
	XPSAsk ("Plot", plt_print);
}

/* called to add plot to the movie loop.
 * use the window of the da_w (passed in client)
 */
/* ARGSUSED */
static void
plt_da_mloop_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Widget da_w = (Widget)client;
	XmUpdateDisplay (da_w);		/* handle expose after pulldown menu */
	ml_add (XtWindow(da_w), NULL);
}

/* called when the Close button is pushed on a plot.
 * just unmap the dialog (passed as client).
 */
/* ARGSUSED */
static void
plt_da_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild ((Widget)client);
}

/* called when a plot is unmapped, either via a WM command or the Close button.
 * free the DrawInfo and destroy the DrawingArea (passed as client).
 */
/* ARGSUSED */
static void
plt_da_unmap_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	plt_da_destroy ((Widget)client);
}

/* callback from the Flip X toggle button within the drawing FormDiag itself.
 * toggle the x bit in the parent's DrawInfo structure and fake an expose.
 * client is the DrawingArea widget.
 */
/* ARGSUSED */
static void
plt_da_flipx_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *t = (XmToggleButtonCallbackStruct *) call;
	Widget da_w = (Widget) client;
	Widget daform_w = XtParent(da_w);
	Display *dsp = XtDisplay(da_w);
	Window win = XtWindow(da_w);
	DrawInfo *di;
	XExposeEvent ev;
	Window root;
	int x, y;
	unsigned int nx, ny, bw, d;

	get_something (daform_w, XmNuserData, (XtArgVal)&di);
	di->flipx = t->set;

	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);

	ev.type = Expose;
	ev.send_event = 1;	/* gets set anyways */
	ev.display = dsp;
	ev.window = win;
	ev.x = ev.y = 0;
	ev.width = nx;
	ev.height = ny;
	ev.count = 0;

	XSendEvent (dsp, win, /*propagate*/False, ExposureMask, (XEvent *)&ev);
}

/* callback from the Flip Y toggle button within the drawing FormDiag itself.
 * toggle the y bit in the parent's DrawInfo structure and fake an expose.
 * client is the DrawingArea widget.
 */
/* ARGSUSED */
static void
plt_da_flipy_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *t = (XmToggleButtonCallbackStruct *) call;
	Widget da_w = (Widget) client;
	Widget daform_w = XtParent(da_w);
	Display *dsp = XtDisplay(da_w);
	Window win = XtWindow(da_w);
	DrawInfo *di;
	XExposeEvent ev;
	Window root;
	int x, y;
	unsigned int nx, ny, bw, d;

	get_something (daform_w, XmNuserData, (XtArgVal)&di);
	di->flipy = t->set;

	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);

	ev.type = Expose;
	ev.send_event = 1;	/* gets set anyways */
	ev.display = dsp;
	ev.window = win;
	ev.x = ev.y = 0;
	ev.width = nx;
	ev.height = ny;
	ev.count = 0;

	XSendEvent (dsp, win, /*propagate*/False, ExposureMask, (XEvent *)&ev);
}

/* callback from the grid toggle button within the drawing FormDiag itself.
 * toggle the grid flag in the parent's DrawInfo structure and fake an expose.
 * client is the DrawingArea widget.
 */
/* ARGSUSED */
static void
plt_da_grid_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *t = (XmToggleButtonCallbackStruct *) call;
	Widget da_w = (Widget) client;
	Widget daform_w = XtParent(da_w);
	Display *dsp = XtDisplay(da_w);
	Window win = XtWindow(da_w);
	DrawInfo *di;
	XExposeEvent ev;
	Window root;
	int x, y;
	unsigned int nx, ny, bw, d;

	get_something (daform_w, XmNuserData, (XtArgVal)&di);
	di->grid = t->set;

	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);

	ev.type = Expose;
	ev.send_event = 1;	/* gets set anyways */
	ev.display = dsp;
	ev.window = win;
	ev.x = ev.y = 0;
	ev.width = nx;
	ev.height = ny;
	ev.count = 0;

	XSendEvent (dsp, win, /*propagate*/False, ExposureMask, (XEvent *)&ev);
}

/* callback from the X-as-Year-is-date toggle button within the drawing FormDiag itself.
 * toggle the xisdate flag in the parent's DrawInfo structure and fake an
 * expose. client is the DrawingArea widget.
 */
/* ARGSUSED */
static void
plt_da_xyr_asdate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *t = (XmToggleButtonCallbackStruct *) call;
	Widget da_w = (Widget) client;
	Widget daform_w = XtParent(da_w);
	Display *dsp = XtDisplay(da_w);
	Window win = XtWindow(da_w);
	DrawInfo *di;
	XExposeEvent ev;
	Window root;
	int x, y;
	unsigned int nx, ny, bw, d;

	/* honor this, and toggle off the other if on */
	get_something (daform_w, XmNuserData, (XtArgVal)&di);
	di->xyr_asdate = t->set;
	if (!di->xyr_asdate && di->xjd_asdate)
	    return;	/* we were toggled off */
	if (di->xyr_asdate && di->xjd_asdate) {
	    di->xjd_asdate = 0;
	    XmToggleButtonSetState (di->jas_w, False, True);
	}

	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);

	ev.type = Expose;
	ev.send_event = 1;	/* gets set anyways */
	ev.display = dsp;
	ev.window = win;
	ev.x = ev.y = 0;
	ev.width = nx;
	ev.height = ny;
	ev.count = 0;

	XSendEvent (dsp, win, /*propagate*/False, ExposureMask, (XEvent *)&ev);
}

/* callback from the X-as-JD-is-date toggle button within the drawing FormDiag itself.
 * toggle the xisdate flag in the parent's DrawInfo structure and fake an
 * expose. client is the DrawingArea widget.
 */
/* ARGSUSED */
static void
plt_da_xjd_asdate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *t = (XmToggleButtonCallbackStruct *) call;
	Widget da_w = (Widget) client;
	Widget daform_w = XtParent(da_w);
	Display *dsp = XtDisplay(da_w);
	Window win = XtWindow(da_w);
	DrawInfo *di;
	XExposeEvent ev;
	Window root;
	int x, y;
	unsigned int nx, ny, bw, d;

	/* honor this, and toggle off the other if on */
	get_something (daform_w, XmNuserData, (XtArgVal)&di);
	di->xjd_asdate = t->set;
	if (!di->xjd_asdate && di->xyr_asdate)
	    return;	/* we were toggled off */
	if (di->xjd_asdate && di->xyr_asdate) {
	    di->xyr_asdate = 0;
	    XmToggleButtonSetState (di->yas_w, False, True);
	}

	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);

	ev.type = Expose;
	ev.send_event = 1;	/* gets set anyways */
	ev.display = dsp;
	ev.window = win;
	ev.x = ev.y = 0;
	ev.width = nx;
	ev.height = ny;
	ev.count = 0;

	XSendEvent (dsp, win, /*propagate*/False, ExposureMask, (XEvent *)&ev);
}

/* plot drawing area's expose callback.
 * redraw the graph to the (new?) size.
 * get a DrawInfo from our parent's userData.
 */
/* ARGSUSED */
static void
plt_da_exp_cb (da_w, client, call)
Widget da_w;
XtPointer client;
XtPointer call;
{
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;

	switch (c->reason) {
	case XmCR_EXPOSE: {
	    XExposeEvent *e = &c->event->xexpose;
	    XSetWindowAttributes swa;
	    unsigned long mask = CWBitGravity;

	    /* turn off gravity so we get fresh exposes when grow and shrink.
	     * do it every time because this expose handler is used for all
	     * the windows -- yes, it is over kill but not worth remembering
	     * which windows have already been set.
	     */
	    swa.bit_gravity = ForgetGravity;
	    XChangeWindowAttributes (e->display, e->window, mask, &swa);

	    /* wait for the last in the series */
	    if (e->count != 0)
		return;
	    break;
	    }
	default:
	    printf ("Unexpected da_w event. type=%d\n", c->reason);
	    abort();
	}

	/* draw the plot from scratch */
	plt_drawall (da_w);
}

/* plot drawing area's motion handler.
 * called when pointer moves over a plot.
 * get a DrawInfo from our parent's userData.
 * report data at that location
 */
static void
plt_da_motion_eh (da_w, client, ev, continue_to_dispatch)
Widget da_w;
XtPointer client;
XEvent *ev;
Boolean *continue_to_dispatch;
{
	Display *dsp = XtDisplay(da_w);
	Window win = XtWindow(da_w);
	Window root, child;
	int rx, ry, wx, wy;
	unsigned mask;
	Widget daform_w;
	DrawInfo *di;

	daform_w = XtParent(da_w);
	get_something (daform_w, XmNuserData, (XtArgVal)&di);

	XQueryPointer (dsp, win, &root, &child, &rx, &ry, &wx, &wy, &mask);

	plot_coords (da_w, di, wx, wy);
}


/* given a drawing area, draw the plot.
 * remember: the parent of the da is the form and it's userData is the di info.
 */
static void
plt_drawall (da_w)
Widget da_w;
{
	Dimension w, h;
	Widget daform_w;
	DrawInfo *di;

	watch_cursor (1);

	get_something (da_w, XmNwidth, (XtArgVal)&w);
	get_something (da_w, XmNheight, (XtArgVal)&h);

	daform_w = XtParent(da_w);
	get_something (daform_w, XmNuserData, (XtArgVal)&di);
	XClearWindow (XtDisplay(da_w), XtWindow(da_w));
	rewind (di->fp);
	if (plot_cartesian (di, da_w, w, h) < 0) {
	    /* had trouble, so done with this FormDialog.
	     */
	    xe_msg (0, "Error plotting `%s'", di->filename);
	    plt_da_destroy (da_w);
	}

	/* user annotation */
	ano_draw (da_w, XtWindow(da_w), plt_ano, 0);

	watch_cursor (0);
}

/* support simple pixel based annotation
 */
static int
plt_ano (double *sx, double *sy, int *xp, int *yp, int w2x, int arg)
{
	if (w2x) {
	    *xp = (int)*sx;
	    *yp = (int)*sy;
	} else {
	    *sx = (double)*xp;
	    *sy = (double)*yp;
	}

	return (1);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: plotmenu.c,v $ $Date: 2012/12/30 17:01:02 $ $Revision: 1.38 $ $Name:  $"};

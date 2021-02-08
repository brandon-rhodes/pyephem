/* this file contains functions to support iterative xephem searches.
 * we support several kinds of searching and solving algorithms.
 * the expressions being evaluated are compiled and executed from compiler.c.
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
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/SelectioB.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>

#include "xephem.h"
#include "lilxml.h"

static void srch_create_shell (void);
static int srch_isup (void);
static void get_tmlimit(void);
static int srching_now (void);
static void compile_func (void);
static void srch_set_buttons (int whether);
static void srch_use_cb (Widget w, XtPointer client, XtPointer call);
static void srch_loadfn_cb (Widget w, XtPointer client, XtPointer call);
static void srch_savefn_cb (Widget w, XtPointer client, XtPointer call);
static void srch_newfunc_cb (Widget w, XtPointer client, XtPointer call);
static void srch_compile_cb (Widget w, XtPointer client, XtPointer call);
static void srch_help_cb (Widget w, XtPointer client, XtPointer call);
static void srch_clear_cb (Widget w, XtPointer client, XtPointer call);
static void srch_fields_cb (Widget w, XtPointer client, XtPointer call);
static void srch_close_cb (Widget w, XtPointer client, XtPointer call);
static void srch_acc_cb (Widget wid, XtPointer client, XtPointer call);
static void srch_goal_cb (Widget w, XtPointer client, XtPointer call);
static void srch_on_off_cb (Widget w, XtPointer client, XtPointer call);
static int srch_min (double Mjd, double v, double *tmincp);
static int srch_max (double Mjd, double v, double *tmincp);
static int srch_solve0 (double Mjd, double v, double *tmincp);
static int srch_binary (double Mjd, double v, double *tmincp);


/* the widgets we need direct access to */
static Widget srchshell_w;	/* main shell */
static Widget acc_w;		/* TF to enter desired search accuracy */
static Widget field_w;		/* TB whether to set buttons to define func */
static Widget func_w;		/* T contains the search function */
static Widget err_w;		/* label to show compile errors */
static Widget valu_w;		/* label to display calulated value */
static Widget on_w;		/* TB whether searching is active */
static Widget save_w;		/* TF naming file to save */

/* name is it appears when valu_w is selected for plotting or lising */
static char srchvname[] = "SolveValue";
static char solvecategory[] = "Tools -- Solve";	/* Save category */
static char xml_srchele[] = "XEphemSolver";

static int (*srch_f)();		/* 0 or pointer to one of the search functions*/
static int srch_tmscalled;	/* number of iterations so far */
static double tmlimit;		/* search accuracy, hrs */
static char config_suffix[] = ".svc";	/* file ext for solver config files */

static int srch_selecting;	/* whether our value is currently selectable */
static int srch_self_selection_mode;	/* flag to prevent self-selection */

typedef struct {
    char *title;
    char *name;
    XtPointer cb_data;
    char *tip;
    Widget w;
} Goal;
static Goal goals[] = {
    {"Find Maximum", "FindMax", (XtPointer)srch_max,
	"Solve for a time at which the Function experiences a local maxima"},
    {"Find Minimum", "FindMin", (XtPointer)srch_min,
	"Solve for a time at which the Function experiences a local minima"},
    {"Find 0", "Find0", (XtPointer)srch_solve0,
	"Solve for a time at which the Function evaluates to 0"},
    {"Binary", "Binary", (XtPointer)srch_binary,
	"Step time along until the truth value of the Function changes"},
};

/* called when the search menu is activated via the main menu pulldown.
 * if never called before, create and manage all the widgets as a child of a
 * form. otherwise, just go for it.
 */
void
srch_manage ()
{
	if (!srchshell_w)
	    srch_create_shell();
	
	XtPopup (srchshell_w, XtGrabNone);
	set_something (srchshell_w, XmNiconic, (XtArgVal)False);
	srch_set_buttons(srch_selecting);
}

/* called by other menus as they want to hear from our buttons or not.
 * the "on"s and "off"s stack - only really redo the buttons if it's the
 * first on or the last off.
 * N.B. we cooperate with a flag from the Enable pushbutton to prevent
 *   being able use the search function result as a term in the search funtion.
 */
void
srch_selection_mode (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	if (srch_self_selection_mode)
	    return;

	if (whether)
	    srch_selecting++;
	else if (srch_selecting > 0)
	    --srch_selecting;

	if (isUp(srchshell_w))
	    if ((whether && srch_selecting == 1)     /* first one to want on */
		|| (!whether && srch_selecting == 0) /* last one to want off */)
		srch_set_buttons (whether);
}

/* called when other modules, such as data menu, have a button pushed
 * and we have asked (by enabling field buttons) that they inform us that
 * that button is to be included in the search function.
 * other modules that use buttons, such as plotting and listing, might ask
 * too so it might not really be us that wants to use it.
 */
void
srch_selection (name)
char *name;
{
	int ins;
	char *qname;

	if (!isUp(srchshell_w) || !XmToggleButtonGetState(field_w))
	    return;

	ins = XmTextGetInsertionPosition (func_w);
	qname = XtMalloc (strlen(name) + 3);	/* two '"' and \0 */
	(void) sprintf (qname, "\"%s\"", name);
	XmTextReplace (func_w, /* from */ ins, /* to */ ins, qname);
	XtFree (qname);

	/* move the focus right back to the search menu so the insertion point
	 * remains visible.
	 */
	XSetInputFocus (XtDisplay(srchshell_w), XtWindow(srchshell_w),
						RevertToParent, CurrentTime);
}

/* if searching is in effect call the search type function.
 * it might modify *tmincp according to where it next wants to eval.
 * (remember tminc is in hours, not days).
 * if searching ends for any reason it is also turned off.
 * if we are not searching but are plotting or listing we still execute the
 *   search function (if it is ok) and flog and display it.
 * return 0 if caller can continue or -1 if it is time to stop.
 */
int
srch_eval(Mjd, tmincp)
double Mjd;
double *tmincp;
{
	int s;

	if (prog_isgood() && any_ison()) {
	    char errbuf[128];
	    double v;
	    s = execute_expr (&v, errbuf);
	    if (s == 0) {
		f_double (valu_w, "%g", v);
		if (srching_now()) {
		    xe_msg (0, "Solver step %3d @ %13.5f %g", srch_tmscalled,
		    						Mjd+MJD0, v);
		    s = (*srch_f)(Mjd, v, tmincp);
		    srch_tmscalled++;
		}
	    } else {
		xe_msg (0, "Solver evaluation error: %.200s", errbuf);
	    }
	} else
	    s = 0;

	if (s < 0)
	    XmToggleButtonSetState(on_w, False, /*invoke cb*/True);

	return (s);
}

/* called by other systems to decide whether it is worth computing and
 *   logging values to the search system.
 * we say True whenever there is a validly compiled function and either we are
 *   searching (obviously) or the srch control menu is up.
 */
int
srch_ison()
{
	return (prog_isgood() && (srching_now() || srch_isup()));
}

/* called as each different field is written -- just tell the compiler
 * if we are interested in it.
 * we have to check if *anything* is on because we might be plotting/listing
 *   the srch function itself.
 */
void
srch_log (name, value)
char *name;
double value;
{
	if (any_ison())
	    compiler_log (name, value);
}

/* called to put up or remove the watch cursor.  */
void
srch_cursor (c)
Cursor c;
{
	Window win;

	if (srchshell_w && (win = XtWindow(srchshell_w)) != 0) {
	    Display *dsp = XtDisplay(srchshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

static void
srch_create_shell()
{
	XmString str;
	Widget w, trc_w, brc_w, f_w, rb_w;
	Widget oms_w, compile_w; 
	Widget help_w;
	Widget srchform_w;
	Arg args[20];
	char *s[1];
	int i, n;

	/* create form dialog */
	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNiconName, "Solve"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	XtSetArg (args[n], XmNtitle, "xephem Solver Control"); n++;
	srchshell_w = XtCreatePopupShell ("Solve", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (srchshell_w);
	set_something (srchshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (srchshell_w, "XEphem*Solve.x", solvecategory, 0);
	sr_reg (srchshell_w, "XEphem*Solve.y", solvecategory, 0);

	n = 0;
	XtSetArg (args[n], XmNresizePolicy, XmRESIZE_GROW); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	srchform_w = XmCreateForm (srchshell_w, "SrchF", args, n);
	XtAddCallback (srchform_w, XmNhelpCallback, srch_help_cb, 0);
	XtManageChild (srchform_w);

	/* create a RowColumn to hold stuff on top of the text */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNspacing, 5); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNadjustMargin, False); n++;
	trc_w = XmCreateRowColumn (srchform_w, "TSrchRC", args, n);
	XtManageChild (trc_w);

	/* searching on/off toggle button */

	n = 0;
	on_w = XmCreateToggleButton (trc_w, "SAct", args, n);
	XtAddCallback (on_w, XmNvalueChangedCallback, srch_on_off_cb, 0);
	set_xmstring (on_w, XmNlabelString, "Solver is Active");
	wtip (on_w, "When on, solver controls Main's step size and searches for goal");
	XtManageChild (on_w);

	/* create a separator */

	n = 0;
	w = XmCreateSeparator (trc_w, "Sep1", args, n);
	XtManageChild (w);

	/* compiler area title */

	n = 0;
	f_w = XmCreateForm (trc_w, "SrchAF", args, n);
	XtManageChild (f_w);

	    str = XmStringCreate ("Function:", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (f_w, "FuncL", args, n);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomOffset, 2); n++;
	    w = XmCreatePushButton (f_w, "Clear", args, n);
	    XtAddCallback (w, XmNactivateCallback, srch_clear_cb, NULL);
	    XtManageChild (w);

	/* create another RC for the bottom stuff.
	 * text will connect them together.
	 * this is so text follows user resizing.
	 */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNspacing, 5); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNadjustMargin, False); n++;
	brc_w = XmCreateRowColumn (srchform_w, "BSrchRC", args, n);
	XtManageChild (brc_w);

	/* compiler message label */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	err_w = XmCreateLabel (brc_w, "SrchErrsL", args, n);
	set_xmstring (err_w, XmNlabelString, " ");
	wtip (err_w, "Result from compiling the search Function");
	XtManageChild (err_w);

	/* use-fields button */

	str = XmStringCreate("Enable field buttons", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	field_w = XmCreateToggleButton (brc_w, "SrchFEnable", args, n);
	XtAddCallback (field_w, XmNvalueChangedCallback, srch_fields_cb, 0);
	wtip (field_w, "When on, data fields eligible for use in Function are selectable buttons");
	XtManageChild (field_w);
	XmStringFree(str);

	/* COMPILE push button */

	n = 0;
	XtSetArg (args[n], XmNshowAsDefault, True); n++;
	compile_w = XmCreatePushButton (brc_w, "Compile", args, n);
	XtAddCallback (compile_w, XmNactivateCallback, srch_compile_cb, 0);
	set_something (srchform_w, XmNdefaultButton, (XtArgVal)w);
	wtip (compile_w, "Press to compile the Function");
	XtManageChild (compile_w);

	/* create a separator */

	n = 0;
	w = XmCreateSeparator (brc_w, "Sep2", args, n);
	XtManageChild (w);

	/* config file load */

	s[0] = config_suffix;
	oms_w = createFSM (brc_w, s, 1, "auxil", srch_loadfn_cb);
	wtip (oms_w, "Select existing plot configuration file to load");
	set_xmstring (oms_w, XmNlabelString, "Load file: ");
	n = 0;

	/* config file save */

	n = 0;
	f_w = XmCreateForm (brc_w, "CFF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    w = XmCreatePushButton (f_w, "CFL", args, n);
	    XtAddCallback (w, XmNactivateCallback, srch_savefn_cb, NULL);
	    wtip (w,"Save search function and settings to file named at right");
	    set_xmstring (w, XmNlabelString, "Save to:");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNleftOffset, 5); n++;
	    save_w = XmCreateTextField (f_w, "ConfigFile", args, n);
	    sr_reg (save_w, NULL, solvecategory, 0);
	    defaultTextFN (save_w, 0, "mysolver.svc", NULL);
	    XtAddCallback (save_w, XmNactivateCallback, srch_savefn_cb, NULL);
	    wtip (save_w, "File name in which to save search function");
	    XtManageChild (save_w);

	/* create a separator */

	n = 0;
	w = XmCreateSeparator (brc_w, "Sep2", args, n);
	XtManageChild (w);

	/* create goal radio box and its toggle buttons */

	str = XmStringCreate ("Goal:", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	XtSetArg (args[n], XmNlabelString, str); n++;
	w = XmCreateLabel (brc_w, "SrchRBL", args, n);
	XtManageChild(w);
	XmStringFree(str);

	n = 0;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	rb_w = XmCreateRadioBox (brc_w, "SrchGoalRadioBox", args, n);
	XtManageChild (rb_w);

	    for (i = 0; i < XtNumber(goals); i++) {
		n = 0;
		str = XmStringCreate(goals[i].title, XmSTRING_DEFAULT_CHARSET);
		XtSetArg (args[n], XmNlabelString, str); n++;
		w = XmCreateToggleButton (rb_w, goals[i].name, args, n);
		XtAddCallback (w, XmNvalueChangedCallback, srch_goal_cb,
						(XtPointer)goals[i].cb_data);
		XmStringFree(str);
		if (goals[i].tip)
		    wtip (w, goals[i].tip);
		XtManageChild (w);
		goals[i].w = w;
		sr_reg (w, NULL, solvecategory, 1);
	    }

	/* create a separator */

	n = 0;
	w = XmCreateSeparator (brc_w, "Sep3", args, n);
	XtManageChild (w);

	/* Accuracy label and its push button in a form */

	n = 0;
	f_w = XmCreateForm (brc_w, "SrchAF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 8); n++;
	    acc_w = XmCreateTextField (f_w, "Accuracy", args, n);
	    XtAddCallback (acc_w, XmNactivateCallback, srch_acc_cb, 0);
	    wtip (acc_w, "Set desired time accuracy of solution, hours");
	    XtManageChild (acc_w);
	    sr_reg (acc_w, NULL, solvecategory, 1);
	    get_tmlimit();

	    str = XmStringCreate("Desired accuracy:", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, acc_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 5); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (f_w, "SrchAccL", args, n);
	    XtManageChild (w);
	    XmStringFree (str);

	/* current search value in a form */

	n = 0;
	f_w = XmCreateForm (brc_w, "SrchVF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNuserData, (XtArgVal)srchvname); n++;
	    valu_w = XmCreatePushButton (f_w, "CurValue", args, n);
	    set_xmstring (valu_w, XmNlabelString, "0.0");
	    buttonAsButton (valu_w, False);
	    XtAddCallback (valu_w, XmNactivateCallback, srch_use_cb, 0);
	    wtip (valu_w, "Value of Function evaluated at current time");
	    XtManageChild (valu_w);

	    str = XmStringCreate ("Current value: ", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, valu_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 5); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (f_w, "SrchValuL", args, n);
	    XtManageChild (w);
	    XmStringFree (str);

	/* create a separator */

	n = 0;
	w = XmCreateSeparator (brc_w, "Sep4", args, n);
	XtManageChild (w);

	/* form to hold bottom control buttons */

	n = 0;
	XtSetArg (args[n], XmNfractionBase, 7); n++;
	f_w = XmCreateForm (brc_w, "SrchCF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 1); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 3); n++;
	    w = XmCreatePushButton(f_w, "Close", args,n);
	    XtAddCallback(w, XmNactivateCallback, srch_close_cb, 0);
	    wtip (w, "Close this dialog (but continue solving if active)");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 4); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 6); n++;
	    help_w = XmCreatePushButton(f_w, "Help", args, n);
	    XtAddCallback(help_w, XmNactivateCallback, srch_help_cb, 0);
	    wtip (help_w, "More detailed usage information");
	    XtManageChild (help_w);

	/* function text connected in between the rc's.
	 * arrange for Return to activate the Compile button.
	 */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, trc_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, brc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg (args[n], XmNwordWrap, True); n++;
	func_w = XmCreateText (srchform_w, "Function", args, n);
	XtAddCallback (func_w, XmNvalueChangedCallback, srch_newfunc_cb, NULL);
	wtip(func_w,"Function to solve: enter fields manually, or via buttons");
	XtManageChild (func_w);
	sr_reg (func_w, NULL, solvecategory, 1);
}

/* set tmlimit from user's widget */
static void
get_tmlimit()
{
	char *str = XmTextFieldGetString (acc_w);
	f_scansexa (str, &tmlimit);
	XtFree (str);
}

/* return True whenever the srch control menu is up */
static int
srch_isup()
{
	return (isUp(srchshell_w));
}

/* return True whenever we are actually in the midst of controlling a search.
 */
static int
srching_now()
{
	return (on_w && XmToggleButtonGetState(on_w));
}

/* go through all the buttons pickable for plotting and set whether they
 * should appear to look like buttons.
 */
static void
srch_set_buttons (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	buttonAsButton(valu_w, whether);
}

/* callback from the value button when it is to be used for
 * plotting or listing. if we have been put us in selecting mode, we look like
 * a button and we should inform them we have been picked.
 * otherwise, we do nothing (we didn't look like a button anyway).
 */
/* ARGSUSED */
static void
srch_use_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (srch_selecting) {
	    if (prog_isgood()) {
		plt_selection (srchvname);
		lst_selection (srchvname);
	    } else
		xe_msg (0, "You must first successfully compile a search\nfunction before value may be selected.");
	}
}

/* callback when the function definition changes
 */
/* ARGSUSED */
static void
srch_newfunc_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	f_showit (err_w, "New function not compiled");
}

/* callback to load a function from a file.
 * file name is label of this widget.
 */
/* ARGSUSED */
static void
srch_loadfn_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char msg[1024];
	char *g, *f, *a;
	char *fn;
	FILE *fp;

	get_xmstring (w, XmNlabelString, &fn);
	fp = fopend (fn, NULL, "r");
	if (fp) {
	    LilXML *lp = newLilXML ();
	    XMLEle *sele = readXMLFile (fp, lp, msg);
	    int i;

	    if (!sele)
		xe_msg (1, "%s", msg);
	    else if (strcmp (tagXMLEle(sele), xml_srchele)
	    		|| !(g = pcdataXMLEle (findXMLEle (sele, "goal")))
	    		|| !(f = pcdataXMLEle (findXMLEle (sele, "function")))
	    		|| !(a = pcdataXMLEle (findXMLEle (sele, "accuracy"))))
		xe_msg (1, "Not a valid solver file");
	    else {
		for (i = 0; i < XtNumber(goals); i++) {
		    if (!strcmp(goals[i].title, g)) {
			XmToggleButtonSetState (goals[i].w, True, True);
			break;
		    }
		}
		if (i == XtNumber(goals)) {
		    xe_msg (1, "Unknown goal: %s", g);
		} else {
		    /* ok, all looks good */
		    XmTextSetString (func_w, f);
		    XmTextFieldSetString (acc_w, a);
		    get_tmlimit();
		    compile_func();
		}
	    }

	    delXMLEle (sele);
	    delLilXML (lp);
	    fclose (fp);
	}

	XtFree (fn);
}

/* save the current search function to a file.
 * N.B. don't use call, used by both TF and PB 
 */
/* ARGSUSED */
static void
srch_savefn_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024], *txt;
	char *goal;
	char *fn;
	FILE *fp;
	int i;

	/* find current goal setting */
	goal = NULL;
	for (i = 0; i < XtNumber(goals); i++) {
	    if (XmToggleButtonGetState(goals[i].w)) {
		goal = goals[i].title;
		break;
	    }
	}
	if (!goal) {
	    xe_msg (1, "Goal must be selected before saving");
	    return;
	}

	/* get file name */
	fn = txt = XmTextFieldGetString (save_w);
	if (!strstr(txt, config_suffix)) {
	    sprintf (fn = buf, "%s%s", txt, config_suffix);
	    XmTextFieldSetString (save_w, fn);
	}

	/* create file */
	fp = fopend (fn, NULL, "w");
	if (fp) {
	    char *func = XmTextGetString (func_w);
	    char *acc = XmTextFieldGetString (acc_w);
	    fprintf (fp, "<%s>\n", xml_srchele);
		fprintf (fp, "  <function>%s</function>\n", entityXML(func));
		fprintf (fp, "  <accuracy>%s</accuracy>\n", acc);
		fprintf (fp, "  <goal>%s</goal>\n", goal);
	    fprintf (fp, "</%s>\n", xml_srchele);
	    XtFree (func);
	    XtFree (acc);
	    fclose (fp);
	}

	/* clean up */
	if (confirm())
	    xe_msg (1, "%s saved", fn);
	XtFree (txt);
}

/* callback from the compile button.
 */
/* ARGSUSED */
static void
srch_compile_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	compile_func();
}

/* callback from the help button.
 */
/* ARGSUSED */
static void
srch_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
static char *help_msg[] = {
"This menu controls the automatic searching facility. You define an arithmetic",
"or boolean function, using most of the fields xephem displays, then xephem",
"will automatically evaluate the function and adjust the time on each",
"iteration to search for the goal.",
"",
"To perform a search:",
"   enter a function,",
"   compile it,",
"   select a goal,",
"   set the desired accuracy,",
"   enable searching,",
"   perform the search by stepping xephem."
};

	hlp_dialog ("Search", help_msg, sizeof(help_msg)/sizeof(help_msg[0]));
}

/* callback from the "clear" function puch button.
 */
/* ARGSUSED */
static void
srch_clear_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *txt = XmTextGetString (func_w);
	XmTextReplace (func_w, 0, strlen(txt), "");
	XtFree (txt);
}

/* callback from the "field enable" push button.
 * inform the other menues whether we are setting up for them to tell us
 * what fields to plot.
 */
/* ARGSUSED */
static void
srch_fields_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int whether = XmToggleButtonGetState(w);

	/* don't use our own result to srch with */
	srch_self_selection_mode = 1;
	all_selection_mode(whether);
	srch_self_selection_mode = 0;
}

/* callback from the Close button.
 */
/* ARGSUSED */
static void
srch_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (srchshell_w);
}

/* user typed Return in accuracy field. get his new value and use it */
/* ARGSUSED */
static void
srch_acc_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	get_tmlimit();
}

/* callback from the search goal selection radio buttons.
 * same callback used for all of them.
 * client is pointer to desired search function.
 */
/* ARGSUSED */
static void
srch_goal_cb(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int (*sfp)()= (int (*)())client;

	if (XmToggleButtonGetState(w)) {
	    /* better turn off searching if changing the search function! */
	    if (srch_f != sfp && srching_now())
		XmToggleButtonSetState(on_w, False, True /* invoke cb */);
	    srch_f = sfp;
	}
}

/* callback from the on/off toggle button activate.
 */
/* ARGSUSED */
static void
srch_on_off_cb(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg =
	    "You must first successfully compile a search Function and\n select a Goal algorithm before searching may be enabled.";

	if (XmToggleButtonGetState(w)) {
	    /* insure there is a valid function and goal algorithm selected
	     * if turning searching on.
	     */
	    if (!prog_isgood()) {
		XmToggleButtonSetState(on_w, False, True/* invoke cb */);
		xe_msg (1, "%s", msg);
		 
	    } else if (srch_f == 0) {
		XmToggleButtonSetState(on_w, False, True/* invoke cb */);
		xe_msg (1, "%s", msg);
	    } else {
		srch_tmscalled = 0;
		/* turning on searhing so as a courtesy turn off the
		 * field selection mechanism if it's on now.
		 * get fresh desired acc.
		 */
		if (XmToggleButtonGetState(field_w))
		    XmToggleButtonSetState(field_w, False, True);
		get_tmlimit();
	    }
	}
}

static void
compile_func()
{
	char *newexp;
	char errbuf[256];

	newexp = XmTextGetString (func_w);
	if (compile_expr (newexp, errbuf) <  0)
	    set_xmstring (err_w, XmNlabelString, errbuf);
	else
	    set_xmstring (err_w, XmNlabelString, "Function compiled successfully");

	XtFree (newexp);

	/* as a courtesy, turn off searching if it's currently active */
	if (XmToggleButtonGetState(on_w))
	    XmToggleButtonSetState(on_w, False, True/*invoke cb*/);

	/* compute the new function.
	 * must update everything else first because our function uses that
	 *   stuff in all likelihood.
	 */
	if (prog_isgood()) {
	    Now *np = mm_get_now();
	    redraw_screen(1);
	    (void) srch_eval (mjd, (double *)NULL);
	}
}

/* search for time when expression is at a local maximum.
 * uses globals srch_tmscalled and tmlimit.
 * return -1 when finished.
 */
static int
srch_max(Mjd, v, tmincp)
double Mjd;
double v;
double *tmincp;
{
	static double v0, lastv0, j0, v1, lastv1, j1;
	static double lastv;

	/* save first time and keep going */
	if (srch_tmscalled == 0) {
	    j0 = -1e100;
	    j1 =  1e100;
	    lastv = v;
	    return (0);
	}

	/* record and turn around at half speed whenever value shrinks again */
	if (v < lastv) {
	    if (*tmincp > 0) {
		j1 = Mjd;
		lastv1 = v1;
		v1 = v;
	    } else {
		j0 = Mjd;
		lastv0 = v0;
		v0 = v;
	    }
	    if (lastv0 == v0 && lastv1 == v1) {
		xe_msg (1, "Function is too broad to find max so precisely");
		return (-1);
	    }

	    *tmincp *= -0.5;
	}

	lastv = v;

	return (24*(j1 - j0) < tmlimit ? -1 : 0);
}

/* search for time when expression is at a local minimum.
 * uses globals srch_tmscalled and tmlimit.
 * return -1 when finished.
 */
static int
srch_min(Mjd, v, tmincp)
double Mjd;
double v;
double *tmincp;
{
	static double v0, lastv0, j0, v1, lastv1, j1;
	static double lastv;

	/* save first time and keep going */
	if (srch_tmscalled == 0) {
	    j0 = -1e100;
	    j1 =  1e100;
	    lastv = v;
	    return (0);
	}

	/* record and turn around at half speed whenever value grows again */
	if (v > lastv) {
	    if (*tmincp > 0) {
		j1 = Mjd;
		lastv1 = v1;
		v1 = v;
	    } else {
		j0 = Mjd;
		lastv0 = v0;
		v0 = v;
	    }
	    if (lastv0 == v0 && lastv1 == v1) {
		xe_msg (1, "Function is too broad to find min so precisely");
		return (-1);
	    }

	    *tmincp *= -0.5;
	}

	lastv = v;

	return (24*(j1 - j0) < tmlimit ? -1 : 0);
}

/* use secant method to solve for time when expression passes through 0.
 * uses globals srch_tmscalled and tmlimit.
 */
static int
srch_solve0(Mjd, v, tmincp)
double Mjd;
double v;
double *tmincp;
{
	static double x0, x_1;	/* x(n-1) and x(n) */
	static double y_0, y_1;	/* y(n-1) and y(n) */
	double x_2;		/* x(n+1) */
	double df;		/* y(n) - y(n-1) */

	switch (srch_tmscalled) {
	case 0: x0 = Mjd; y_0 = v; return(0);
	case 1: x_1 = Mjd; y_1 = v; break;
	default: x0 = x_1; y_0 = y_1; x_1 = Mjd; y_1 = v; break;
	}

	df = y_1 - y_0;
	if (fabs(df) < 1e-10) {
	    /* near-0 zero denominator, ie, curve is pretty flat here,
	     * so assume we are done enough.
	     * signal this by forcing a 0 tminc.
	     */
	    *tmincp = 0.0;
	    return (-1);
	}
	x_2 = x_1 - y_1*(x_1-x0)/df;
	*tmincp = (x_2 - Mjd)*24.0;
	return (fabs (*tmincp) < tmlimit ? -1 : 0);
}

/* binary search for time when expression changes from its initial state.
 * uses globals srch_tmscalled and tmlimit.
 * if the change is outside the initial tminc range, then keep searching in that
 *    direction by tminc first before starting to divide down.
 */
static int
srch_binary(Mjd, v, tmincp)
double Mjd;
double v;
double *tmincp;
{
	static double lb, ub;		/* lower and upper bound */
	static int initial_state;
	int this_state = v >= 0.5;

#define	FLUNDEF	-9e10

	if (srch_tmscalled == 0) {
	    if (*tmincp >= 0.0) {
		/* going forwards in time so first Mjd is lb and no ub yet */
		lb = Mjd;
		ub = FLUNDEF;
	    } else {
		/* going backwards in time so first Mjd is ub and no lb yet */
		ub = Mjd;
		lb = FLUNDEF;
	    }
	    initial_state = this_state;
	    return (0);
	}

	if (ub != FLUNDEF && lb != FLUNDEF) {
	    if (this_state == initial_state)
		lb = Mjd;
	    else
		ub = Mjd;
	    *tmincp = ((lb + ub)/2.0 - Mjd)*24.0;
#ifdef TRACEBIN
	    xe_msg (0, "lb=%g ub=%g tminc=%g Mjd=%g is=%d ts=%d",
			    lb, ub, *tmincp, Mjd, initial_state, this_state);
#endif
	    /* signal to stop if asking for time change less than TMLIMIT */
	    return (fabs (*tmincp) < tmlimit ? -1 : 0);
	} else if (this_state != initial_state) {
	    /* gone past; turn around half way */
	    if (*tmincp >= 0.0)
		ub = Mjd;
	    else
		lb = Mjd;
	    *tmincp /= -2.0;
	    return (0);
	} else {
	    /* just keep going, looking for first state change but we keep
	     * learning the lower (or upper, if going backwards) bound.
	     */
	    if (*tmincp >= 0.0)
		lb = Mjd;
	    else
		ub = Mjd;
	    return (0);
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: srchmenu.c,v $ $Date: 2008/01/31 16:40:52 $ $Revision: 1.25 $ $Name:  $"};

 /* code to handle setting up and generating trails.
 *
 * to use: let user define a trail with tr_setup().
 * when user hits Ok we call your function and you build the actual trails
 *   and do the drawing with tr_draw().
 *
 * ok to have several of these at once; they each maintain their own context.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>

#include "xephem.h"

#define	SMALL_MAG	1.0		/* small char mag */
#define	MEDIUM_MAG	1.4		/* medium char mag */
#define	LARGE_MAG	2.0		/* large char mag */
#define	HUGE_MAG	2.8		/* huge char mag */
#define	GAP		4		/* space from line to text, pixels */

static char fontxr[] = "trailsFont";	/* X resource naming font to use */
static XFontStruct *tr_fs;		/* font for trails */

/* context information per dialog.
 * one of these gets malloced and saved in client callback.
 */
typedef struct {
    Widget sh_w;		/* overall shell */
    Widget nb_w, na_w;		/* Scales setting n ticks before and after */
    Widget l_w[TRLR_N];		/* TBs for time stamp label rate options */
    Widget i_w[TRI_N];		/* TBs for interval options */
    Widget f_w[TRF_N];		/* TBs for format options */
    Widget r_w[TRR_N];		/* TBs for the rounding options */
    Widget s_w[TRS_N];		/* TBs for the size options */
    Widget o_w[TRO_N];		/* TBs for the orientation options */
    Widget custom_w;		/* TextField for custom interval */
    TrCB cb;			/* saved caller's callback function */
    XtPointer client;		/* saved in tr_setup() and sent back in callbk*/
} TrContext;

/* make an array of these to create a row/col for the options */
typedef struct {
    char *name;			/* widget instance name */
    char *label;		/* widget's label */
    char *tip;			/* tip text */
} RCOption;

/* N.B. must correspond to the entries in TrLR */
static RCOption l_options[TRLR_N] = {
    {"every1",	"Every 1",   "Label each tick mark"},
    {"every2",	"Every 2",   "Label every other tick mark"},
    {"every5",	"Every 5",   "Label every 5th tick mark"},
    {"every10",	"Every 10",  "Label every 10th tick mark"},
    {"fl",	"First+Last","Label only the first and last tick mark"},
    {"ml",	"Mid+Last",  "Label only the middle and last tick mark"},
    {"fm",	"First+Mid", "Label only the first and middle tick mark"},
    {"fml",	"F+M+L",     "Label only the first, middle and last tick mark"},
    {"none",	"None",      "Do not label any tick marks"},
};

/* N.B. must correspond to the entries in TrInt */
static RCOption i_options[TRI_N] = {
    {"1min",	"1 Minute",  "One tick mark every minute"},
    {"5mins",	"5 Minutes", "One tick mark every 5 minutes"},
    {"hour",	"1 Hour",    "One tick mark each hour"},
    {"day",	"1 Day",     "One tick mark each day"},
    {"week",	"1 Week",    "One tick mark each 7 days"},
    {"month",	"1 Month",   "One tick mark per month"},
    {"year",	"1 Year",    "One tick mark per year"},
    {"custom",	"Custom",    "One tick mark for each interval, below"},
};

/* N.B. must correspond to the entries in TrFormat */
static RCOption f_options[TRF_N] = {
    {"time",	"H:M:S","Label tick marks as Hour:Minutes:Seconds"},
    {"time",	"H:M",  "Label tick marks as Hour:Minutes"},
    {"date",	"Date", "Label tick marks according to the current Date format Preference"},
};

/* N.B. must correspond to the entries in TrRound */
static RCOption r_options[TRR_N] = {
    {"min",	"Whole min", "Begin tick marks at the next whole minute"},
    {"day",	"Whole day", "Begin tick marks at the next whole day"},
    {"whole",	"Whole interval", "Begin tick marks at the next whole interval selection"},
    {"now",	"Now", "Begin tick marks at the current Main menu time"},
};

/* N.B. must correspond to the entries in TrOrient */
static RCOption o_options[TRO_N] = {
    {"up",   "Up",   "Labels written up from tick mark, tilt head left to read"},
    {"down", "Down", "Labels written down from tick mark, tilt head right to read"},
    {"left", "Left", "Labels written to left of tick mark"},
    {"right","Right","Labels written to right of tick mark"},
    {"above","Above","Labels centered above tick mark"},
    {"below","Below","Labels centered below tick mark"},
    {"upr",  "Up 45","Labels at 45 degree angle up and right of tick mark"},
    {"downr","Down 45","Labels at 45 degree angle down and right of tick mark"},
    {"pathl","Path-Left","Labels would be to your left if you walked the trail"},
    {"pathr","Path-Right","Labels would be to your right if you walked the trail"},
};

/* N.B. must correspond to the entries in TrSize */
static RCOption s_options[TRS_N] = {
    {"small",	"Small",  "Tick mark labels are small"},
    {"medium",	"Medium", "Tick mark labels are medium size"},
    {"large",	"Large",  "Tick mark labels are large"},
    {"huge",	"Huge",   "Tick mark labels are huge"},
};

static Widget create_form (char *title, char *hdr, TrState *init, TrCB cb,
    XtPointer client);
static void make_rb (Widget rc_w, char *title, char *name, RCOption *op,
    int nop, Widget savew[]);
static void customint_cb (Widget w, XtPointer client, XtPointer call);
static void ok_cb (Widget w, XtPointer client, XtPointer call);
static void apply_cb (Widget w, XtPointer client, XtPointer call);
static int do_trails (TrContext *tcp);
static void tStep (double *tp, TrState *statep, int dir);
static int get_options (TrContext *tcp, TrState *statep);
static void popdown_cb (Widget w, XtPointer client, XtPointer call);
static void destroy_cb (Widget w, XtPointer client, XtPointer call);
static void close_cb (Widget w, XtPointer client, XtPointer call);
static void help_cb (Widget w, XtPointer client, XtPointer call);
static void draw_stamp (Display *dsp, Drawable win, GC gc, TrTS *tp,
    TrState *sp, int mark, int ticklen, int lx, int ly, int x, int y);

/* put up a window to ask how to make a trail. OK will call (*cb)().
 * ok to have several at once -- they keep their own context in client callback.
 */
void
tr_setup(
char *title,		/* window manager title */
char *hdr,		/* label across top */
TrState *init,		/* default setup */
TrCB cb,		/* callback when/if user hits Ok */
XtPointer client)	/* saved and passed back to (*cb)() */
{
	static char me[] = "tr_setup()";
	Widget w;

	/* do some sanity checks */
	if (init->l >= TRLR_N) {
	    printf ("%s: Bogus label rate: %d\n", me, init->l);
	    abort();
	}
	if (init->i >= TRI_N) {
	    printf ("%s: Bogus interval: %d\n", me, init->i);
	    abort();
	}
	if (init->f >= TRF_N) {
	    printf ("%s: Bogus format: %d\n", me, init->f);
	    abort();
	}
	if (init->r >= TRR_N) {
	    printf ("%s: Bogus round: %d\n", me, init->r);
	    abort();
	}
	if (init->o >= TRO_N) {
	    printf ("%s: Bogus orientation: %d\n", me, init->o);
	    abort();
	}
	if (init->s >= TRS_N) {
	    printf ("%s: Bogus size: %d\n", me, init->s);
	    abort();
	}
	if (!cb) {
	    printf ("%s: No cb()\n", me);
	    abort();
	}

	/* ok, go for it */
	w = create_form (title, hdr, init, cb, client);
	if (w)
	    XtPopup (w, XtGrabNone);
}

/* called when basic resources change.
 * rebuild and redraw.
 * TODO: reclaim old stuff when called again.
 */
void
tr_newres()
{
	tr_fs = NULL;
}

/* draw a line from [lx,ly] to [x,y]. iff tp, draw a tickmark (in parens if
 * mark) of halfsize ticklen pixels and draw a time stamp according to tp and
 * sp at [x,y]. iff ltp, do the same according to ltp and sp at [lx,ly].
 */
void
tr_draw (Display *dsp, Drawable win, GC gc, int mark, int ticklen, TrTS *tp,
TrTS *ltp, TrState *sp, int lx, int ly, int x, int y)
{
	/* the base line for sure */
        XPSDrawLine (dsp, win, gc, lx, ly, x, y);

	/* if tp, draw tick mark and stamp at [x,y] */
	if (tp) {
	    XPSFillArc (dsp, win, gc, x-ticklen, y-ticklen, 2*ticklen+1,
						    2*ticklen+1, 0, 360*64);
	    draw_stamp (dsp, win, gc, tp, sp, mark, ticklen, lx, ly, x, y);
	}

	/* same for other end if ltp */
	if (ltp) {
	    XPSFillArc (dsp, win, gc, lx-ticklen, ly-ticklen, 2*ticklen+1,
						    2*ticklen+1, 0, 360*64);
	    draw_stamp (dsp, win, gc, ltp, sp, mark, ticklen,
					    x-2*(x-lx), y-2*(y-ly), lx, ly);
	}

}

/* set a trail state resource
 */
void
tr_setres (name, state)
char *name;
TrState *state;
{
	char val[50];

	sprintf (val, "%d %d %d %d %d %d %d %d %.9f",
		 (int) state->l, (int) state->i, (int) state->f,
		 (int) state->r, (int) state->o, (int) state->s,
		 state->nticks, state->nbefore, state->customi);
	setXRes (name, val);
}

/* get a trail state from a resource
 */
void
tr_getres (name, state)
char *name;
TrState *state;
{
	char *val;
	int l, i, f, r, o, s, n, b;
	double c;

	val = getXRes (name, "");
	if (*val) {
	    if (sscanf (val, "%d %d %d %d %d %d %d %d %lf",
			&l, &i, &f, &r, &o, &s, &n, &b, &c) == 9) {
		state->l = (TrLR) l;
		state->i = (TrInt) i;
		state->f = (TrFormat) f;
		state->r = (TrRound) r;
		state->o = (TrOrient) o;
		state->s = (TrSize) s;
		state->nticks = n;
		state->nbefore = b;
		state->customi = c;
	    } else
		xe_msg (0, "Trail state resource value of %s invalid.", name);
	}
}


/* create a trail control window.
 * A pointer to malloc'd TrContext is kept in client destroy callback, be sure
 * to free this whenever the window is destroyed.
 */
static Widget
create_form (title, hdr, init, cb, client)
char *title;
char *hdr;
TrState *init;
TrCB cb;
XtPointer client;
{
	TrContext *tcp;
	Widget ok_w, apply_w, close_w, help_w;
	Widget sep_w, lbl_w, n_w;
	Widget sh_w, mf_w, hdr_w, rc_w, f_w, fr_w;
	Widget ofr_w, ifr_w, lrfr_w;
	Widget w;
	XmString str;
	Arg args[20];
	int n;

	/* make a fresh trails context */
	tcp = (TrContext *) malloc (sizeof(TrContext));
	if (!tcp) {
	    xe_msg (1, "Can not malloc for trail setup context.");
	    return (NULL);
	}

	/* save caller's callback function and client data */
	tcp->cb = cb;
	tcp->client = client;

	/* make the shell */
	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNtitle, title); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNiconName, "Trail"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	sh_w = XtCreatePopupShell ("Trails", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (sh_w);
	set_something (sh_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (sh_w, XmNpopupCallback, prompt_map_cb, NULL);
	XtAddCallback (sh_w, XmNpopdownCallback, popdown_cb, NULL);
	XtAddCallback (sh_w, XmNdestroyCallback, destroy_cb, (XtPointer)tcp);

	/* make the form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNfractionBase, 16); n++;
	XtSetArg (args[n], XmNverticalSpacing, 8); n++;
	mf_w = XmCreateForm (sh_w, "TF", args, n);
	XtAddCallback (mf_w, XmNhelpCallback, help_cb, NULL);
	XtManageChild (mf_w);

	/* save shell in tcp for later destruction */
	tcp->sh_w = sh_w;

	/* create the bottons along the bottom */

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 1); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 3); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	ok_w = XmCreatePushButton (mf_w, "Ok", args, n);
	XtAddCallback (ok_w, XmNactivateCallback, ok_cb, (XtPointer)tcp);
	wtip (ok_w, "Use settings to create trail, close dialog");
	XtManageChild (ok_w);

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 5); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 7); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	apply_w = XmCreatePushButton (mf_w, "Apply", args, n);
	XtAddCallback (apply_w, XmNactivateCallback, apply_cb, (XtPointer)tcp);
	wtip (apply_w, "Use settings to create trail, leave dialog up");
	XtManageChild (apply_w);

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 9); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 11); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	close_w = XmCreatePushButton (mf_w, "Close", args, n);
	XtAddCallback (close_w, XmNactivateCallback, close_cb, (XtPointer)tcp);
	wtip (close_w, "Close dialog, make no trails");
	XtManageChild (close_w);

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 13); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 15); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	help_w = XmCreatePushButton (mf_w, "Help", args, n);
	XtAddCallback (help_w, XmNactivateCallback, help_cb, 0);
	wtip (help_w, "More detailed information");
	XtManageChild (help_w);

	/* separator above bottom controls */

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, help_w); n++;
	sep_w = XmCreateSeparator (mf_w, "Sep", args, n);
	XtManageChild (sep_w);

	/* labels and fields for before and after counts above separator */

	n = 0;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 5); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, sep_w); n++;
	lbl_w = XmCreateLabel (mf_w, "LB", args, n);
	set_xmstring (lbl_w, XmNlabelString, "Ticks before Start:");
	XtManageChild (lbl_w);

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 5); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 9); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, sep_w); n++;
	XtSetArg (args[n], XmNshowValue, True); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNmaximum, 256); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_LEFT); n++;
	n_w = XmCreateScale (mf_w, "TicksBefore", args, n);
	wtip (n_w, "Set number of desired tick marks before Start time");
	XtManageChild (n_w);

	/* save counts-before widget id for later retrieval */
	tcp->nb_w = n_w;

	/* set default */
	XmScaleSetValue (n_w, init->nbefore);

	n = 0;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 11); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, sep_w); n++;
	lbl_w = XmCreateLabel (mf_w, "LA", args, n);
	set_xmstring (lbl_w, XmNlabelString, "after:");
	XtManageChild (lbl_w);

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 11); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 15); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, sep_w); n++;
	XtSetArg (args[n], XmNshowValue, True); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNmaximum, 256); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	n_w = XmCreateScale (mf_w, "TicksAfter", args, n);
	wtip (n_w, "Set number of desired tick marks after Start time");
	XtManageChild (n_w);

	/* save counts-after widget id for later retrieval */
	tcp->na_w = n_w;

	/* set desired default tick mark count */
	XmScaleSetValue (n_w, init->nticks - init->nbefore);

	/* add tables at the top in their own form */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, n_w); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	f_w = XmCreateForm (mf_w, "TF1", args, n);
	XtManageChild (f_w);

	/* make the header label across the top */
	str = XmStringCreateSimple (hdr);
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	XtSetArg (args[n], XmNlabelString, str); n++;
	hdr_w = XmCreateLabel (f_w, "Header", args, n);
	XtManageChild (hdr_w);
	XmStringFree (str);


	/* make a radio box for orientations in a frame. */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, hdr_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 2); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 33); n++;
	ofr_w = XmCreateFrame (f_w, "OFR", args, n);
	XtManageChild (ofr_w);

	    n = 0;
	    XtSetArg (args[n], XmNadjustMargin, False); n++;
	    rc_w = XmCreateRowColumn (ofr_w, "ORC", args, n);
	    XtManageChild (rc_w);

	    /* make rb for orientation options */
	    make_rb(rc_w, "Orientation:", "Orientation", o_options, TRO_N,
								    tcp->o_w);

	    /* set desired default orientation */
	    XmToggleButtonSetState (tcp->o_w[init->o], True, True);


 	/* make a radio box for intervals in a frame.
	 * also put the custom TF and "whole" tb in the rc.
	 */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, hdr_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 35); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 66); n++;
	ifr_w = XmCreateFrame (f_w, "IFR", args, n);
	XtManageChild (ifr_w);

	    n = 0;
	    XtSetArg (args[n], XmNadjustMargin, False); n++;
	    rc_w = XmCreateRowColumn (ifr_w, "IRC", args, n);
	    XtManageChild (rc_w);

	    /* make rb for interval options */
	    make_rb (rc_w, "Interval:", "Interval", i_options, TRI_N, tcp->i_w);

	    /* set desired default interval */
	    XmToggleButtonSetState (tcp->i_w[init->i], True, True);

	    /* add the TextField for entering custom interval,
	     * add callback to custom to let it change TextField sense,
	     * and init TF sens here now.
	     */
	    n = 0;
	    XtSetArg (args[n], XmNcolumns, 10); n++;
	    w = XmCreateTextField (rc_w, "CustomInterval", args, n);
	    wtip (w, "Enter a custom tick mark interval, h:m:s  <x>d  <x>y");
	    XtManageChild (w);
	    tcp->custom_w = w;
	    XtSetSensitive (w, XmToggleButtonGetState(tcp->i_w[TRI_CUSTOM]));
	    XtAddCallback (tcp->i_w[TRI_CUSTOM], XmNvalueChangedCallback,
						customint_cb, tcp->custom_w);
	    if (init->i == TRI_CUSTOM) {
		char buf[64];
		fs_sexa (buf, init->customi*24.0, 2, 3600);
		XmTextFieldSetString (tcp->custom_w, buf);
	    }

 	/* make a radio box for label rates in a frame. */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, hdr_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 68); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 98); n++;
	lrfr_w = XmCreateFrame (f_w, "LRFR", args, n);
	XtManageChild (lrfr_w);

	    n = 0;
	    XtSetArg (args[n], XmNadjustMargin, False); n++;
	    rc_w = XmCreateRowColumn (lrfr_w, "LRRC", args, n);
	    XtManageChild (rc_w);

	    /* make rb for label rates options */
	    make_rb(rc_w, "Label:", "TimeStamp",l_options,TRLR_N,tcp->l_w);

	    /* set desired default label rate */
	    XmToggleButtonSetState (tcp->l_w[init->l], True, True);



 	/* make a radio box for format options in a frame */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ofr_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNbottomPosition, 95); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 2); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 33); n++;
	fr_w = XmCreateFrame (f_w, "FFR", args, n);
	XtManageChild (fr_w);

	    n = 0;
	    XtSetArg (args[n], XmNadjustMargin, False); n++;
	    rc_w = XmCreateRowColumn (fr_w, "FRC", args, n);
	    XtManageChild (rc_w);

	    /* make rb for format options */
	    make_rb (rc_w, "Format:", "Format", f_options, TRF_N, tcp->f_w);

	    /* set desired default format */
	    XmToggleButtonSetState (tcp->f_w[init->f], True, True);

 	/* make a radio box for size options in a frame */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ifr_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNbottomPosition, 95); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 35); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 66); n++;
	fr_w = XmCreateFrame (f_w, "SFR", args, n);
	XtManageChild (fr_w);

	    n = 0;
	    XtSetArg (args[n], XmNadjustMargin, False); n++;
	    rc_w = XmCreateRowColumn (fr_w, "SRC", args, n);
	    XtManageChild (rc_w);

	    /* make rb for size options */
	    make_rb (rc_w, "Font:", "Size", s_options, TRS_N, tcp->s_w);

	    /* set desired default rounding */
	    XmToggleButtonSetState (tcp->s_w[init->s], True, True);


 	/* make a radio box for rounding options in a frame */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, lrfr_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNbottomPosition, 95); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 68); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 98); n++;
	fr_w = XmCreateFrame (f_w, "RFR", args, n);
	XtManageChild (fr_w);

	    n = 0;
	    XtSetArg (args[n], XmNadjustMargin, False); n++;
	    rc_w = XmCreateRowColumn (fr_w, "RRC", args, n);
	    XtManageChild (rc_w);

	    /* make rb for rounding options */
	    make_rb (rc_w, "Start:", "Start", r_options, TRR_N, tcp->r_w);

	    /* set desired default rounding */
	    XmToggleButtonSetState (tcp->r_w[init->r], True, True);


	return (sh_w);
}

/* make a title and radio box inside rc.
 * save each toggle button widget in savew[] array.
 * would use frame's new title child but don't want to lock into 1.2 (yet)
 */
static void
make_rb (rc_w, title, name, op, nop, savew)
Widget rc_w;
char *title;
char *name;
RCOption *op;
int nop;
Widget savew[];
{
	Widget l_w, rb_w, w;
	XmString str;
	Arg args[20];
	int n;
	int i;

	str = XmStringCreateSimple (title);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	l_w = XmCreateLabel (rc_w, "L", args, n);
	XtManageChild (l_w);
	XmStringFree (str);

	n = 0;
	rb_w = XmCreateRadioBox (rc_w, name, args, n);
	XtManageChild (rb_w);

	    for (i = 0; i < nop; i++, op++) {
		str = XmStringCreateSimple (op->label);
		n = 0;
		XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
		XtSetArg (args[n], XmNlabelString, str); n++;
		w = XmCreateToggleButton (rb_w, op->name, args, n);
		XtManageChild (w);
		XmStringFree (str);
		savew[i] = w;
		if (op->tip)
		    wtip (w, op->tip);
	    }
}

/* called when the Custom time interval TB changes.
 * client is the TextField widget used to enter custom intervals.
 */
/* ARGSUSED */
static void
customint_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtSetSensitive ((Widget)client, XmToggleButtonGetState(w));
}

/* called when the Ok is hit.
 * client is pointer to TrContext.
 * pull out desired trail parameters then call the user's callback function.
 * close if went ok.
 */
/* ARGSUSED */
static void
ok_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	TrContext *tcp;

	/* fetch the TrContext */
	tcp = (TrContext *)client;

	/* do the work, close unless err */
	if (do_trails (tcp) == 0)
	    XtPopdown (tcp->sh_w);
}

/* called when the Apply is hit.
 * client is pointer to TrContext.
 * pull out desired trail parameters then call the user's callback function.
 */
/* ARGSUSED */
static void
apply_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	TrContext *tcp;

	/* fetch the TrContext */
	tcp = (TrContext *)client;

	/* do the work, stay up regardless */
	(void) do_trails (tcp);
}

/* perform the actual trails work.
 * return 0 if ok, else -1.
 */
static int
do_trails (tcp)
TrContext *tcp;
{
	Now *np = mm_get_now();
	TrState state;
	TrTS *ttp;
	int dow;
	int m, y;
	double d;
	double t;
	int cbret;
	int i;

	/* gather trail configuration */
	if (get_options(tcp, &state) < 0)
	    return (-1);

	/* get memory for list of time stamps */
	ttp = (TrTS *) malloc (state.nticks * sizeof(TrTS));
	if (!ttp) {
	    xe_msg (1, "Can not malloc tickmarks array.");
	    return (-1);
	}

	/* set initial time, t, as UTC according to rounding options.
	 */
	t = mjd;
	if (pref_get(PREF_ZONE) == PREF_LOCALTZ)
	    t -= tz/24.;			/* rounding in local time */
	switch (state.r) {
	case TRR_MIN:
	    t = ceil(t*24*60)/(24*60);
	    break;

	case TRR_DAY:
	    t = ceil(t-0.5)+0.5;
	    break;

	case TRR_INTER:
	    switch (state.i) {
	    case TRI_MIN:
		t = ceil(t*24*60)/(24*60);
		break;
	    case TRI_5MIN:
		t = ceil(t*24*(60/5))/(24*(60/5));
		break;
	    case TRI_HOUR:
		t = ceil(t*24)/24;
		break;
	    case TRI_DAY:
		t = ceil(t-0.5)+0.5;
		break;
	    case TRI_WEEK:
		t = mjd_day (t);
		while (mjd_dow (t, &dow) == 0 && dow != 0)
		    t += 1;
		break;
	    case TRI_MONTH:
		mjd_cal (t, &m, &d, &y);
		d = 1;
		if (++m > 12) {
		    m = 1;
		    y += 1;
		}
		cal_mjd (m, d, y, &t);
		break;
	    case TRI_YEAR:
		mjd_cal (t, &m, &d, &y);
		d = 1;
		m = 1;
		y++;
		cal_mjd (m, d, y, &t);
		break;
	    case TRI_CUSTOM:
		/* round to next nearest multiple of custom */
		t = ceil(t*(1.0/state.customi))/(1.0/state.customi);
		break;
	    default:
		break;
	    }
	    break;

	case TRR_NONE:
	    /* stay with current moment */
	    break;

	default:
	    printf ("Bogus trail rounding code: %d\n", state.r);
	    abort();
	}
	if (pref_get(PREF_ZONE) == PREF_LOCALTZ)
	    t += tz/24.;			/* back to UT */

	/* move back by nbefore */
	for (i = 0; i < state.nbefore; i++)
	    tStep (&t, &state, -1);

	/* now fill in the time stamp times and formats */
	for (i = 0; i < state.nticks; i++) {
	    TrTS *tp = &ttp[i];

	    /* time is just t */
	    tp->t = t;

	    /* determine whether to stamp this tickmark */
	    switch (state.l) {
	    case TRLR_1:
		tp->lbl = 1;
		break;
	    case TRLR_2:
		tp->lbl = !(i % 2);
		break;
	    case TRLR_5:
		tp->lbl = !(i % 5);
		break;
	    case TRLR_10:
		tp->lbl = !(i % 10);
		break;
	    case TRLR_FL:
		tp->lbl = (i == 0 || i == state.nticks-1);
		break;
	    case TRLR_FM:
		tp->lbl = (i == 0 || i == state.nticks/2);
		break;
	    case TRLR_ML:
		tp->lbl = (i == state.nticks/2 || i == state.nticks-1);
		break;
	    case TRLR_FML:
		tp->lbl = (i==0 || i==state.nticks/2 || i==state.nticks-1);
		break;
	    case TRLR_NONE:
		tp->lbl = 0;
		break;
	    default:
		printf ("Bogus trail label rate code=%d\n", state.l);
		abort();
	    }

	    /* advance time */
	    tStep (&t, &state, 1);
	}

	/* now call the users callback */
	cbret = (*tcp->cb) (ttp, &state, tcp->client);

	/* clean up what we malloced except
	 * TrContext isn't destroyed until user closes dialog.
	 */
	free ((char *)ttp);

	/* ok */
	return (cbret);
}

/* advance t forward or backward based on state */
static void
tStep (tp, statep, sign)
double *tp;
TrState *statep;
int sign;
{
	int m, y;
	double d;

	if (abs(sign) != 1) {
	    printf ("tStep called with %d\n", sign);
	    abort();
	}

	switch (statep->i) {
	case TRI_CUSTOM: *tp += sign*statep->customi;	break;
	case TRI_MIN:    *tp += sign*1./(24.*60.);      break;
	case TRI_5MIN:   *tp += sign*5./(24.*60.);	break;
	case TRI_HOUR:   *tp += sign*1./24.;		break;
	case TRI_DAY:    *tp += sign*1.0;		break;
	case TRI_WEEK:   *tp += sign*7.0;		break;
	case TRI_MONTH:
	    mjd_cal (*tp, &m, &d, &y);
	    m += sign;
	    if (m > 12) {
		m = 1;
		y += 1;
	    } else if (m < 1) {
		m = 12;
		y -= 1;
	    }
	    cal_mjd (m, d, y, tp);
	    break;
	case TRI_YEAR:
	    mjd_cal (*tp, &m, &d, &y);
	    y += sign;
	    cal_mjd (m, d, y, tp);
	    break;
	default:
	    printf ("Bogus interval: %d\n", statep->i);
	    abort();
	}
}

/* pull the options from tcp.
 * return 0 if ok, else xe_msg() wny not and return -1.
 */
static int
get_options(tcp, statep)
TrContext *tcp;
TrState *statep;
{
	int i, j;

	/* find label rate */
	for (i = 0; i < TRLR_N; i++) {
	    if (XmToggleButtonGetState(tcp->l_w[i]))
		break;
	}
	if (i == TRLR_N) {
	    xe_msg (1, "Please select a tick mark label rate.");
	    return(-1);
	}
	statep->l = (TrLR)i;

		
	/* find interval -- might be custom */
	for (i = 0; i < TRI_N; i++) {
	    if (XmToggleButtonGetState(tcp->i_w[i]))
		break;
	}
	if (i == TRI_N) {
	    xe_msg (1, "Please select a tick mark interval.");
	    return(-1);
	}
	statep->i = (TrInt)i;
	if (statep->i == TRI_CUSTOM) {
	    char *str;
	    double days;

	    str = XmTextFieldGetString (tcp->custom_w);
	    if (strchr (str, 'y'))
		days = strtod (str, NULL)*365.0;
	    else if (strchr (str, 'd'))
		days = strtod (str, NULL);
	    else if (f_scansexa (str, &days) == 0)
		days /= 24.0;
	    else {
		xe_msg (1, "Format must be one of h:m:s  <x>d  <x>y");
		XtFree (str);
		return (-1);
	    }
	    XtFree (str);

	    if (days <= 0.0) {
		xe_msg (1, "Please specify a positive custom interval.");
		return(-1);
	    }

	    statep->customi = days;
	}


	/* find rounding mode */
	for (i = 0; i < TRR_N; i++) {
	    if (XmToggleButtonGetState(tcp->r_w[i]))
		break;
	}
	if (i == TRR_N) {
	    xe_msg (1, "Please select a rounding mode.");
	    return(-1);
	}
	statep->r = (TrRound)i;


	/* find format mode */
	for (i = 0; i < TRF_N; i++) {
	    if (XmToggleButtonGetState(tcp->f_w[i]))
		break;
	}
	if (i == TRF_N) {
	    xe_msg (1, "Please select a format mode.");
	    return(-1);
	}
	statep->f = (TrFormat)i;

	/* find size */
	for (i = 0; i < TRS_N; i++) {
	    if (XmToggleButtonGetState(tcp->s_w[i]))
		break;
	}
	if (i == TRS_N) {
	    xe_msg (1, "Please select a size.");
	    return(-1);
	}
	statep->s = (TrSize)i;

	/* find orientation */
	for (i = 0; i < TRO_N; i++) {
	    if (XmToggleButtonGetState(tcp->o_w[i]))
		break;
	}
	if (i == TRO_N) {
	    xe_msg (1, "Please select an orientation.");
	    return(-1);
	}
	statep->o = (TrOrient)i;


	/* get total number of tickmarks */
	XmScaleGetValue (tcp->nb_w, &i);
	XmScaleGetValue (tcp->na_w, &j);
	if (i + j < 2) {
	    xe_msg (1, "Please specify at least 2 tick marks.");
	    return(-1);
	}
	statep->nbefore = i;
	statep->nticks = i+j;

	/* ok */
	return (0);
}

/* called when unmapped */
/* ARGSUSED */
static void
popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* N.B. don't free the userData here -- wait for destroy.
	 * turns out unmap callbacks run before ok_cb!
	 */
	XtDestroyWidget (w);
}

/* called when destroyed, client is TrContext to be free'd */
/* ARGSUSED */
static void
destroy_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	free ((char *)client);
}

/* called when the Close button is hit.
 * client is a TrContext.
 */
/* ARGSUSED */
static void
close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	TrContext *tcp;

	/* fetch the TrContext */
	tcp = (TrContext *)client;

	XtPopdown (tcp->sh_w);
}

/* called when the Help button is hit */
/* ARGSUSED */
static void
help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
        hlp_dialog ("Trails", NULL, 0);
}

/* draw a timestamp (in parens if mark) for the given trail object at x, y.
 * direction, size and content depend on tp, sp and the line endpoints.
 */
static void
draw_stamp (dsp, win, gc, tp, sp, mark, ticklen, lx, ly, x, y)
Display *dsp;
Drawable win;
GC gc;
TrTS *tp;
TrState *sp;
int mark;
int ticklen;
int lx, ly;
int x, y;
{
	char buf[64], *bp;
	double mag = 1.0;
	double a = 0, ca, sa;
	int align = 0;
	int gap = GAP + ticklen;
	Now *np = mm_get_now();
	double t = tp->t;

	/* do nothing if no labeling at all or none for this stamp */
	if (sp->l == TRLR_NONE || !tp->lbl)
	    return;

	/* make sure the fonts are ready */
	if (!tr_fs)
	    tr_fs = getXResFont (fontxr);

	/* establish character size */
	switch (sp->s) {
	case TRS_SMALL:	 mag = SMALL_MAG;  break;
	case TRS_MEDIUM: mag = MEDIUM_MAG; break;
	case TRS_LARGE:	 mag = LARGE_MAG;  break;
	case TRS_HUGE:	 mag = HUGE_MAG;   break;
	case TRS_N:	 break;
	}

	/* fill buf with appropriate message and trim leading blanks */
	if (pref_get(PREF_ZONE) == PREF_LOCALTZ) t -= tz/24.;
	switch (sp->f) {
	case TRF_HMS:
	    fs_time (buf, mjd_hr(t));
	    break;
	case TRF_TIME:
	    fs_mtime (buf, mjd_hr(t));
	    break;
	case TRF_DATE:
	    fs_date (buf, pref_get(PREF_DATE_FORMAT), t);
	    break;
	case TRF_N:
	    buf[0] = '\0';
	    break;
	}
        for (bp = buf; *bp == ' '; bp++)
	    continue;
	if (mark) {
	    int l = strlen(bp);
	    memmove (bp+1, bp, l);
	    bp[0] = '[';
	    bp[++l] = ']';
	    bp[++l] = '\0';
	}

	/* set up angle and alignment to rotate and translate into position.
	 * one rule is to never ask the user to look upside down even a little.
	 */
	switch (sp->o) {
	case TRO_UP:
	    y -= gap; align = MLEFT; a = 90;
	    break;

	case TRO_DOWN:
	    y += gap; align = MLEFT; a = -90;
	    break;

	case TRO_LEFT:
	    x -= gap; align = MRIGHT; a = 0;
	    break;

	case TRO_RIGHT:
	    x += gap; align = MLEFT; a = 0;
	    break;

	case TRO_ABOVE:
	    y -= gap; align = BCENTRE; a = 0;
	    break;

	case TRO_BELOW:
	    y += gap; align = TCENTRE; a = 0;
	    break;

	case TRO_UPR:
	    x += gap; y -= gap; align = MLEFT; a = 45;
	    break;

	case TRO_DOWNR:
	    x += gap; y += gap; align = MLEFT; a = -45;
	    break;

	case TRO_PATHL:
	    if (x-lx == 0)
		a = ly-y < 0 ? -PI/2 : PI/2;
	    else
		a = atan2((double)(ly-y),(double)(x-lx));	/* -PI .. +PI */
	    a += PI/2;
	    ca = cos(a);
	    sa = sin(a);
	    x += (int)(ca*gap);
	    y -= (int)(sa*gap);
	    if (a > PI/2) {
		a -= PI;
		align = MRIGHT;
	    } else if (a < -PI/2) {
		a += PI;
		align = MRIGHT;
	    } else {
		align = MLEFT;
	    }
	    a = raddeg(a);
	    break;

	case TRO_PATHR:
	    if (x-lx == 0)
		a = ly-y < 0 ? -PI/2 : PI/2;
	    else
		a = atan2((double)(ly-y),(double)(x-lx));	/* -PI .. +PI */
	    a -= PI/2;
	    ca = cos(a);
	    sa = sin(a);
	    x += (int)(ca*gap);
	    y -= (int)(sa*gap);
	    if (a > PI/2) {
		a -= PI;
		align = MRIGHT;
	    } else if (a < -PI/2) {
		a += PI;
		align = MRIGHT;
	    } else {
		align = MLEFT;
	    }
	    a = raddeg(a);
	    break;

	case TRO_N:
	    break;
	}

	XPSRotDrawAlignedString (dsp, tr_fs, a, mag, win, gc, x, y, bp, align);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: trailmenu.c,v $ $Date: 2008/03/25 01:16:16 $ $Revision: 1.29 $ $Name:  $"};

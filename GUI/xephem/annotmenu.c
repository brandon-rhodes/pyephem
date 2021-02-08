/* system-wide annotation facility */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledW.h>

#include "xephem.h"
#include "lilxml.h"


typedef struct {
    int active;				/* set when in use, 0 can reuse */
    int hide;				/* set to hide this entry */
    char win[64];			/* widget for which annot was created */
    Widget man_w;			/* manager for one row */
    Widget hide_w;			/* hide TB */
    Widget win_w;			/* window name label */
    Widget tf_w;			/* text field */
    int tdx, tdy;			/* relative TF location */
    int ldx, ldy;			/* relative ending coords of line */
    double wa, wb;			/* world coords of target */
    int worldok;			/* whether a and b are set */
    int x0, y0;				/* target X coords(just until worldok)*/
    int arg;				/* user's ano_draw arg */
} AnnInfo;
static AnnInfo *anninfo;		/* malloced array of ann info */
static int nanninfo;			/* total n entries in anninfo */

static Widget anoshell_w;		/* main shell */
static Widget anorc_w;			/* main RC of individual items */
static Widget save_w;			/* save file name TF */
static GC xgc;				/* xor GC while rubber-banding */
static GC agc;				/* annotation GC */
static XFontStruct *afsp;		/* annotation font */
#define	ALW	1			/* line linewidth */
#define	ATB	3			/* text border to start of line */

static int ano_newEntry(void);
static void ano_createshell(void);
static void ano_mkgcs (void);
static void ano_del_cb (Widget w, XtPointer client, XtPointer call);
static void ano_hide_cb (Widget w, XtPointer client, XtPointer call);
static void ano_new_cb (Widget w, XtPointer client, XtPointer call);
static void ano_togglehides_cb (Widget w, XtPointer client, XtPointer call);
static void ano_hideall_cb (Widget w, XtPointer client, XtPointer call);
static void ano_place_cb (Widget w, XtPointer client, XtPointer call);
static void ano_text_cb (Widget w, XtPointer client, XtPointer call);
static void ano_close_cb (Widget w, XtPointer client, XtPointer call);
static void ano_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void ano_help_cb (Widget w, XtPointer client, XtPointer call);
static void ano_load_cb (Widget w, XtPointer client, XtPointer call);
static void ano_save_cb (Widget w, XtPointer client, XtPointer call);
static void ano_load (char *fn);
static void ano_save (char *fn);
static void interact (AnnInfo *ap);
static void lineStart (int bw, int bh, int fx, int fy, int *xp, int *yp);
static void textBB (char *txt, int *lx, int *rx, int *ay, int *dy);
static Widget findDAW (Display *dsp, Window cw);

static char anocategory[] = "Annotation";

static char defanofn[] = "annotation.ano";	/* default file name */
static char ano_suffix[] = ".ano";		/* file name suffix */


/* bring up the annotation tool */
void
ano_manage()
{
	if (!anoshell_w) {
	    ano_createshell();
	    ano_mkgcs();

	    /* seed so we can always use realloc */
	    anninfo = (AnnInfo *) malloc (1);
	    nanninfo = 0;

	    /* load default set */
	    ano_load (NULL);
	}

	XtPopup (anoshell_w, XtGrabNone);
	set_something (anoshell_w, XmNiconic, (XtArgVal)False);

}

/* handy callback may be used by anyone wanting to bring up Annotation window */
void
ano_cb (Widget w, XtPointer client, XtPointer call)
{
	ano_manage();
}

/* draw all annotations assigned to Widget w using drawable dr.
 * dr allows clients to use their private pixmaps if desired.
 * use convwx to convert world to/from X coords depending on w2x.
 * a is angle above equator (such as lat/alt/dec), b is rotation (long/az/ra).
 * arg is just stored and forwarded to caller.
 * on first call after defining a new annotation, worldok is false and we find
 *   the world coords from x0 and y0, from then on we find x and y from the
 *   world coords.
 */
void
ano_draw (Widget w, Drawable dr, int convwx(double *ap, double *bp,
    int *xp, int *yp, int w2x, int arg), int arg)
{
	Display *dsp = XtDisplay (w);
	Window win = XtWindow (w);
	char *wname = XtName(w);
	int i;

	if (!win)
	    return;

	for (i = 0; i < nanninfo; i++) {
	    AnnInfo *ap = &anninfo[i];
	    if (ap->active && !ap->hide && !strcmp(ap->win,wname)) {
		int x, y, v;

		if (ap->worldok)
		    v = convwx (&ap->wa, &ap->wb, &x, &y, 1, ap->arg);
		else {
		    v = convwx (&ap->wa, &ap->wb, &ap->x0, &ap->y0, 0, arg);
		    ap->worldok = 1;
		    ap->arg = arg;
		    x = ap->x0;
		    y = ap->y0;
		}
		
		if (v) {
		    char *txt = XmTextFieldGetString (ap->tf_w);
		    XPSDrawString (dsp, dr, agc, x + ap->tdx, y + ap->tdy, txt,
								strlen(txt));
		    XtFree(txt);

		    XPSDrawLine (dsp, dr, agc, x, y, x + ap->ldx, y + ap->ldy);
		}
	    }
	}
}

/* called whenever an annotation resource is changed, such as font or color.
 * N.B. might be called before ever being opened.
 * TODO: remove old first
 */
void
ano_newres()
{
	if (!anoshell_w)
	    return;
	ano_mkgcs();
	all_update (mm_get_now(), 1);
}

void
ano_cursor(c)
Cursor c;
{
	Window win;

	if (anoshell_w && (win = XtWindow(anoshell_w)) != 0) {
	    Display *dsp = XtDisplay(anoshell_w);
	    if (c)
		XDefineCursor(dsp, win, c);
	    else
		XUndefineCursor(dsp, win);
	}
}

/* create the main annotation shell */
static void
ano_createshell()
{
	Widget w, f_w, pb_w, afs_w, sw_w;
	Arg args[20];
	char *s[1];
	int n;

	/* create shell and form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Annotation"); n++;
	XtSetArg (args[n], XmNiconName, "Annotation"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	anoshell_w = XtCreatePopupShell ("Annotation", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (anoshell_w);
	set_something (anoshell_w, XmNcolormap, (XtArgVal)xe_cm);
        XtAddCallback (anoshell_w, XmNpopdownCallback, ano_popdown_cb, 0);
	sr_reg (anoshell_w, "XEphem*Annotation.x", anocategory, 0);
	sr_reg (anoshell_w, "XEphem*Annotation.y", anocategory, 0);
	sr_reg (anoshell_w, "XEphem*Annotation.width", anocategory, 0);
	sr_reg (anoshell_w, "XEphem*Annotation.height", anocategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNfractionBase, 26); n++;
	f_w = XmCreateForm (anoshell_w, "AnForm", args, n);
	XtAddCallback (f_w, XmNhelpCallback, ano_help_cb, 0);
	XtManageChild (f_w);

	/* controls along bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 1); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 5); n++;
	w = XmCreatePushButton (f_w, "Close", args, n);
	wtip (w, "Close this window");
	XtAddCallback (w, XmNactivateCallback, ano_close_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 6); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 10); n++;
	w = XmCreatePushButton (f_w, "New", args, n);
	wtip (w, "Create a new annotation");
	XtAddCallback (w, XmNactivateCallback, ano_new_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 11); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 15); n++;
	w = XmCreatePushButton (f_w, "HideAll", args, n);
	wtip (w, "Temporarily hide all existing annotation");
	set_xmstring (w, XmNlabelString, "Hide all");
	XtAddCallback (w, XmNactivateCallback, ano_hideall_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 16); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 20); n++;
	w = XmCreatePushButton (f_w, "TogHide", args, n);
	wtip (w, "Toggle which annotations are hiding");
	set_xmstring (w, XmNlabelString, "Toggle");
	XtAddCallback (w, XmNactivateCallback, ano_togglehides_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 21); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 25); n++;
	w = XmCreatePushButton (f_w, "Help", args, n);
	wtip (w, "Get more information about annotation");
	XtAddCallback (w, XmNactivateCallback, ano_help_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	w = XmCreateSeparator (f_w, "Sep", args, n);
	XtManageChild (w);

	/* load/save controls */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 4); n++;
	pb_w = XmCreatePushButton (f_w, "ASPB", args, n);
	set_xmstring (pb_w, XmNlabelString, "Save to: ");
	wtip (pb_w, "Save these Annotations to file named at right");
	XtAddCallback (pb_w, XmNactivateCallback, ano_save_cb, NULL);
	XtManageChild (pb_w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, pb_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 22); n++;
	save_w = XmCreateTextField (f_w, "File", args, n);
	sr_reg (save_w, NULL, anocategory, 0);
	defaultTextFN (save_w, 0, defanofn, NULL);
	XtAddCallback (save_w, XmNactivateCallback, ano_save_cb, NULL);
	wtip (save_w, "File in which to save above Annotations");
	XtManageChild (save_w);

	s[0] = ano_suffix;
	afs_w = createFSM (f_w, s, 1, "auxil", ano_load_cb);
	wtip (afs_w, "Select file of annotations to load");
	set_xmstring (afs_w, XmNlabelString, "Load file: ");

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, save_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 4); n++;
	XtSetValues (afs_w, args, n);

	/* main RC in a SW to contain each entry */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, afs_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	XtSetArg (args[n], XmNvisualPolicy, XmVARIABLE); n++;
	sw_w = XmCreateScrolledWindow (f_w, "RC", args, n);
	XtManageChild (sw_w);

	    n = 0;
	    XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	    XtSetArg (args[n], XmNspacing, 5); n++;
	    anorc_w = XmCreateRowColumn (sw_w, "RC", args, n);
	    XtManageChild (anorc_w);

	    /* SW assumes work is its child but just to be tidy about it .. */
	    set_something (sw_w, XmNworkWindow, (XtArgVal)anorc_w);
}

/* add one entry to anorc_w, return index */
static int
ano_newEntry()
{
	Widget d_w, p_w;
	AnnInfo *ap;
	Widget w, f_w;
	Arg args[20];
	int ai;
	int n;

	/* find new or expand anninfo */

	for (ai = 0; ai < nanninfo; ai++)
	    if (!anninfo[ai].active)
		break;
	if (ai == nanninfo)
	    anninfo = (AnnInfo *)realloc(anninfo, ++nanninfo * sizeof(AnnInfo));
	ap = &anninfo[ai];
	memset (ap, 0, sizeof(*ap));
	ap->active = 1;

	/* put in a form */

	n = 0;
	XtSetArg (args[n], XmNhorizontalSpacing, 2); n++;
	f_w = XmCreateForm (anorc_w, "EF", args, n);
	XtManageChild (f_w);
	ap->man_w = f_w;

	/* del, text, place */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	p_w = XmCreatePushButton (f_w, "Place", args, n);
	wtip (p_w, "(Re)position this annotation");
	XtAddCallback (p_w, XmNactivateCallback, ano_place_cb, (XtPointer)(long int)ai);
	XtManageChild (p_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	d_w = XmCreatePushButton (f_w, "Delete", args, n);
	wtip (d_w, "Delete this annotation");
	XtAddCallback (d_w, XmNactivateCallback, ano_del_cb, (XtPointer)(long int)ai);
	XtManageChild (d_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, d_w); n++;
	ap->hide_w = XmCreateToggleButton (f_w, "Hide", args, n);
	wtip (ap->hide_w, "Hide this annotation");
	XtAddCallback (ap->hide_w, XmNvalueChangedCallback, ano_hide_cb,
								(XtPointer)(long int)ai);
	XtManageChild (ap->hide_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, ap->hide_w); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	ap->win_w = XmCreateLabel (f_w, "WindowName", args, n);
	XtManageChild (ap->win_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, p_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, ap->win_w); n++;
	w = XmCreateTextField (f_w, "TF", args, n);
	wtip (w, "Text to be used in annotation");
	XtAddCallback (w, XmNactivateCallback, ano_text_cb, (XtPointer)(long int)ai);
	XtManageChild (w);
	ap->tf_w = w;

	return (ai);
}

static void
ano_mkgcs()
{
	Display *dsp = XtDisplay (anoshell_w);
	Window root = RootWindow(dsp, DefaultScreen(dsp));
	Pixel black = BlackPixel (dsp, DefaultScreen (dsp));
	unsigned long gcm;
	XGCValues gcv;
	Pixel p;

	afsp = getXResFont ("AnnoFont");
	get_color_resource (anoshell_w, "AnnoColor", &p);

	/* make another that uses xor for rubber banding */
	gcm = GCFunction | GCForeground | GCLineWidth | GCFont;
	gcv.function = GXxor;
	gcv.foreground = black ^ p;
	gcv.line_width = ALW;
	gcv.font = afsp->fid;
	xgc = XCreateGC (XtD, root, gcm, &gcv);

	/* make one of solid color */
	gcm = GCForeground | GCLineWidth | GCFont;
	gcv.foreground = p;
	gcv.line_width = ALW;
	gcv.font = afsp->fid;
	agc = XCreateGC (XtD, root, gcm, &gcv);
}

/* callback from deleting one entry.
 * client is index into anoinfo[]
 */
/* ARGSUSED */
static void
ano_del_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int ai = (long int)client;
	AnnInfo *ap = &anninfo[ai];

	ap->active = 0;
	XtDestroyWidget (ap->man_w);
	all_update (mm_get_now(), 1);
}

/* callback from hiding one entry.
 * client is index into anoinfo[]
 */
/* ARGSUSED */
static void
ano_hide_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int set = XmToggleButtonGetState (w);
	int ai = (long int)client;
	AnnInfo *ap = &anninfo[ai];

	ap->hide = set;
	all_update (mm_get_now(), 1);
}

/* callback from adding one entry.
 */
/* ARGSUSED */
static void
ano_new_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	(void) ano_newEntry();
}

/* callback from toggling all entries.
 */
/* ARGSUSED */
static void
ano_togglehides_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i;

	for (i = 0; i < nanninfo; i++) {
	    AnnInfo *ap = &anninfo[i];
	    if (ap->active)
		XmToggleButtonSetState (ap->hide_w, !!(ap->hide ^= 1), False);
	}
	all_update (mm_get_now(), 1);
}

/* callback from hiding all entries.
 */
/* ARGSUSED */
static void
ano_hideall_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i;

	for (i = 0; i < nanninfo; i++) {
	    AnnInfo *ap = &anninfo[i];
	    if (ap->active)
		XmToggleButtonSetState (ap->hide_w, !!(ap->hide = 1), False);
	}
	all_update (mm_get_now(), 1);
}

/* callback from placing one entry.
 * client is index into anoinfo[]
 */
/* ARGSUSED */
static void
ano_place_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int ai = (long int)client;
	AnnInfo *ap = &anninfo[ai];

	interact (ap);
}

/* callback from entering text.
 * client is index into anoinfo[]
 */
/* ARGSUSED */
static void
ano_text_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int ai = (long int)client;
	AnnInfo *ap = &anninfo[ai];

	/* if not defined, same as Place, otherwise just changing text */
	if (!ap->win[0])
	    interact (ap);
	else
	    all_update (mm_get_now(), 1);
}

/* callback from the main Close button */
/* ARGSUSED */
static void
ano_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* let popdown do all he work */
	XtPopdown (anoshell_w);
}

/* called to load annotations from file.
 * file name is label of this widget
 */
/* ARGSUSED */
static void
ano_load_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *fn;
	get_xmstring (w, XmNlabelString, &fn);
	ano_load(fn);
	XtFree (fn);
}

/* called to Save the current annotations to a file named in save_w
 * N.B. don't use call, used by both a TF and a PB
 */
/* ARGSUSED */
static void
ano_save_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024], *fn;
	char *txt;

	/* get file name */
	fn = txt = XmTextFieldGetString (save_w);
	if (!strstr (txt, ano_suffix)) {
	    sprintf (fn = buf, "%s%s", txt, ano_suffix);
	    XmTextFieldSetString (save_w, fn);
	}

	/* save */
	ano_save(fn);

	/* confirm */
	if (confirm())
	    xe_msg (1, "%s saved", fn);

	/* clean up */
	XtFree (txt);
}

/* callback from closing the main annotation shell.
 */
/* ARGSUSED */
static void
ano_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* TODO erase all ? */
}

/* callback from the overall Help button.
 */
/* ARGSUSED */
static void
ano_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"Enter text, click Place, move cursor to position text, click and hold,",
"position end of line, release."
};

	hlp_dialog ("Annotation", msg, sizeof(msg)/sizeof(msg[0]));
}

/* compute bounding box around txt rendered with afsp font.
 * return left x, right x, ascent and descent from text origin plus small gap;
 */
static void
textBB (char *txt, int *lx, int *rx, int *ay, int *dy)
{
	int txtl = strlen(txt);
	XCharStruct cs;
	int dir, as, de;

	XTextExtents (afsp, txt, txtl, &dir, &as, &de, &cs);
	*lx = cs.lbearing + ATB;
	*rx = cs.rbearing + ATB;
	*ay = cs.ascent + ATB;
	*dy = cs.descent + ATB;
}

/* user interaction to place text and define line */
static void
interact (AnnInfo *ap)
{
	static Cursor tc, lc;
	Display *dsp = XtD;
	Window root = RootWindow(dsp, DefaultScreen(dsp));
	Window annow = 0;
	Widget daw = NULL;
	char *txt;
	int txtl;
	int tlx, trx, tay, tdy;		/* bb of text wrt it's [0,0] pos */
	int lrx, lry;			/* last root x, to detect no motion */
	int lx, ly;			/* last cursor position, wrt window */
	int x0=0, y0=0;			/* line start, wrt window */
	unsigned int lm;
	int nc;

	/* make text and line drawing cursors */
	if (!tc) {
	    tc = XCreateFontCursor (dsp, XC_draft_small);
	    lc = XCreateFontCursor (dsp, XC_pencil);
	}

	/* if reusing an existing annotation, erase old */
	if (ap->win[0]) {
	    ap->win[0] = '\0';
	    all_update (mm_get_now(), 1);
	}

	/* get text to place -- N.B. must XtFree(txt) */
	txt = XmTextFieldGetString (ap->tf_w);
	txtl = strlen (txt);

	/* compute text bounding box */
	textBB (txt, &tlx, &trx, &tay, &tdy);

	/* grab pointer to start placing text */
	XmUpdateDisplay (toplevel_w);	/* helps insure button pops back up */
	if (XGrabPointer (dsp, root, False, ButtonPressMask, GrabModeAsync,
			GrabModeSync, None, tc, CurrentTime) != GrabSuccess) {
	    xe_msg (1, "Could not grab pointer");
	    XtFree (txt);
	    return;
	}

	/* first click places text and begins line, next release ends line */
	lrx = lry = lx = ly = -1000;
	lm = 0;
	for (nc = 0; nc < 2; ) {
	    int x = 0, y = 0, rx, ry, cx, cy;
	    Window rw, cw;
	    unsigned int m;
	    int click;

	    /* get cursor pos .. nothing to do if just over pure root win */
	    if (!XQueryPointer (dsp, root, &rw, &cw, &rx, &ry, &cx, &cy, &m)) {
		xe_msg (1, "XQueryPointer error");
		break;
	    }
	    if (cw == None)
		continue;

	    /* avoid needless work while idle */
	    click = (m ^ lm) & Button1Mask;
	    if (!click && rx == lrx && ry == lry)
		continue;
	    lm = m;
	    lrx = rx;
	    lry = ry;

	    if (nc == 0) {
		/* place text label if in any DA */
		daw = findDAW (dsp, cw);

		if (daw) {
		    /* in one of xephem's DA */
		    Window dawin = XtWindow (daw);

		    /* erase last if in same as before */
		    if (annow == dawin)
			XDrawString (dsp, annow, xgc, lx, ly, txt, txtl);
		    annow = dawin;

		    /* draw in new pos */
		    XTranslateCoordinates (dsp, root, annow, rx, ry, &x,&y,&cw);
		    XDrawString (dsp, annow, xgc, x, y, txt, txtl);

		    if (click) {
			/* draw text in desired color for sure */
			XDrawString (dsp, annow, agc, x, y, txt, txtl);

			/* change cursor for line drawing */
			XUngrabPointer (dsp, CurrentTime);
			if (XGrabPointer (dsp, root, False, ButtonReleaseMask,
					GrabModeAsync, GrabModeSync, None, lc,
					    CurrentTime) != GrabSuccess) {
			    xe_msg (1, "Regrab failed");
			    break;
			}

			/* save text loc, use as "last" for first line erase */
			ap->x0 = x;
			ap->y0 = y;
			x0 = x;
			y0 = y;

			/* move on to line phase */
			nc++;
		    }
		} else {
		    /* not in a DA, erase if just left, check for abort */
		    if (annow) {
			XDrawString (dsp, annow, xgc, lx, ly, txt, txtl);
			annow = 0;
			daw = NULL;
		    }
		    if (click)
			break;
		}

	    } else {
		int dx, dy;

		/* erase last line */
		XDrawLine (dsp, annow, xgc, x0, y0, lx, ly);

		/* find new starting loc */
		XTranslateCoordinates (dsp, root, annow, rx, ry, &x, &y, &cw);
		dx = ap->x0 - tlx;
		dy = ap->y0 - tay;
		lineStart (tlx+trx, tdy+tay, x-dx, y-dy, &x0, &y0);
		x0 += dx;
		y0 += dy;

		/* draw new line */
		XDrawLine (dsp, annow, xgc, x0, y0,  x,  y);

		if (click) {
		    /* done .. save and refresh */
		    ap->tdx = ap->x0 - x;
		    ap->tdy = ap->y0 - y;
		    ap->ldx = x0 - x;
		    ap->ldy = y0 - y;
		    ap->x0 = x;
		    ap->y0 = y;
		    strcpy (ap->win, XtName(daw));
		    set_xmstring (ap->win_w, XmNlabelString, ap->win);
		    ap->active = 1;

		    /* redraw and fix worldok */
		    ap->worldok = 0;
		    all_update (mm_get_now(), 1);

		    break;
		}
	    }

	    /* history */
	    lx = x;
	    ly = y;
	}

	XUngrabPointer (dsp, CurrentTime);

	XtFree (txt);
}

/* given a box and the far end of a line, find a nice place to start the line.
 * all coords are with respect to upper left corner of box, +y down.
 */
static void
lineStart (int bw, int bh, int fx, int fy, int *xp, int *yp)
{
	int x, y;

	if (fx >= 0 && fx <= bw) {
	    /* always use center when between ends, or no-op if inside */
	    x = bw/2;
	    if (fy < 0)
		y = 0;			/* top */
	    else if (fy < bh)
		x = fx, y = fy;		/* freeze inside text */
	    else
		y = bh;			/* bottom */
	} else {
	    int angle;			/* +cw from +x */
	    int dx, dy;

	    /* find angle from center */
	    dx = fx - bw/2;
	    dy = fy - bh/2;
	    angle = dx ? raddeg(atan2((double)dy,(double)dx)) : (dy>0?90:-90);
	    if (angle < 0)
		angle += 360;

	    /* assign nice starting place around edge */
	    switch (angle/20) {
	    case 0: case 17:
		x = bw; y = bh/2;	/* right center */
		break;
	    case 1: case 2:
		x = bw; y = bh;		/* lower right */
		break;
	    case 3: case 4: case 5:
		x = bw/2; y = bh;	/* lower center */
		break;
	    case 6: case 7:
		x = 0; y = bh;		/* lower left */
		break;
	    case 8: case 9:
		x = 0; y = bh/2;	/* left center */
		break;
	    case 10: case 11:
		x = 0; y = 0;		/* upper left */
		break;
	    case 12: case 13: case 14:
		x = bw/2; y = 0;	/* top center */
		break;
	    case 15: case 16:
		x = bw; y = 0;		/* top right */
		break;
	    default:
		printf ("Impossible annot angle %d\n", angle);
		abort();
	    }
	}

	/* done */
	*xp = x;
	*yp = y;
}

/* starting at cw drill down to find a leaf DrawingArea widget.
 * return its pointer else NULL
 */
static Widget
findDAW (Display *dsp, Window cw)
{
	Window par, *chil;
	Window root;
	Widget daw;
	int i;
	unsigned int nchil;

	/* check whether found goal */
	daw = XtWindowToWidget(dsp, cw);
	if (daw && XmIsDrawingArea(daw)) {
	    /* scrolled window uses a DA as a clipping parent for app's DA */
	    Cardinal nwc;
	    get_something (daw, XmNnumChildren, (XtArgVal)&nwc);
	    if (nwc == 0)
		return (daw);		/* yes! */
	}

	/* get children of cw */
	chil = NULL;
	if (!XQueryTree (dsp, cw, &root, &par, &chil, &nchil))
	    return (NULL);

	/* search down for leaf DA */
	daw = NULL;
	for (i = 0; i < nchil && !daw; i++)
	    daw = findDAW (dsp, chil[i]);

	/* clean up and report */
	if (chil)
	    XFree(chil);
	return (daw);
}

/* build a new anninfo[] list from the given file, or file named in save_w
 * if !fn
 */
static void
ano_load (char *fn)
{
	char msg[1024];
	char buf[1024];
	XMLEle *root, *ep;
	FILE *fp;
	LilXML *xp;
	int i;

	/* get file name */
	if (!fn) {
	    char *txt = XmTextFieldGetString (save_w);
	    if (!strstr (txt, ano_suffix))
		sprintf (buf, "%s%s", txt, ano_suffix);
	    else
		strcpy (buf, txt);
	    XtFree (txt);
	    fn = buf;
	}

	/* open */
	fp = fopend (fn, "auxil", "r");
	if (!fp)
	    return;			/* already informed user */

	/* read */
	xp = newLilXML ();
	root = readXMLFile (fp, xp, msg);
	fclose (fp);
	delLilXML (xp);
	if (!root) {
	    xe_msg (1, "%s: %s", fn, msg[0] ? msg : "bad format");
	    return;
	}
	if (strcmp (tagXMLEle(root), "Annotations")) {
	    xe_msg (1, "%s: not an Annotations file", fn);
	    delXMLEle (root);
	    return;
	}

	/* clear the anninfo[] array */
	for (i = 0; i < nanninfo; i++)
	    if (anninfo[i].active)
		XtDestroyWidget (anninfo[i].man_w);
	free ((char *)anninfo);
	anninfo = (AnnInfo *) malloc (1);	/* seed for realloc */
	nanninfo = 0;

	/* build new */
	for (ep = nextXMLEle(root,1); ep != NULL; ep = nextXMLEle(root,0)) {
	    AnnInfo *ap;
	    int ai;

	    if (strcmp (tagXMLEle(ep), "annotation"))
		continue;

	    ai = ano_newEntry();
	    ap = &anninfo[ai];

	    XmTextFieldSetString (ap->tf_w, pcdataXMLEle(ep));
	    XmToggleButtonSetState (ap->hide_w, atoi(findXMLAttValu(ep,"hide")),
	    								False);
	    strcpy (ap->win, findXMLAttValu(ep,"window"));
	    set_xmstring (ap->win_w, XmNlabelString, ap->win);
	    ap->tdx = atoi(findXMLAttValu(ep,"textdx"));
	    ap->tdy = atoi(findXMLAttValu(ep,"textdy"));
	    ap->ldx = atoi(findXMLAttValu(ep,"linedx"));
	    ap->ldy = atoi(findXMLAttValu(ep,"linedy"));
	    ap->arg = atoi(findXMLAttValu(ep,"clientarg"));
	    ap->wa = atof(findXMLAttValu(ep,"worldx"));
	    ap->wb = atof(findXMLAttValu(ep,"worldy"));
	    ap->worldok = 1;
	}

	/* finished */
	delXMLEle (root);

	/* draw */
	all_update (mm_get_now(), 1);
}

/* save the current active entries in anninfo[] to the given file */
static void
ano_save (char *fn)
{
	FILE *fp;
	int i;

	/* create */
	fp = fopend (fn, NULL, "w");
	if (!fp)
	    return;			/* already informed user */

	/* write */
	fprintf (fp, "<Annotations>\n");
	for (i = 0; i < nanninfo; i++) {
	    AnnInfo *ap = &anninfo[i];
	    if (ap->active) {
		char *string = XmTextFieldGetString (ap->tf_w);

		fprintf (fp, "  <annotation");
		fprintf (fp, " hide='%d'", XmToggleButtonGetState(ap->hide_w));
		fprintf (fp, " window='%s'", ap->win);
		fprintf (fp, " textdx='%d'", ap->tdx);
		fprintf (fp, " textdy='%d'", ap->tdy);
		fprintf (fp, " linedx='%d'", ap->ldx);
		fprintf (fp, " linedy='%d'", ap->ldy);
		fprintf (fp, " clientarg='%d'", ap->arg);
		fprintf (fp, " worldx='%g'", ap->wa);
		fprintf (fp, " worldy='%g'", ap->wb);
		fprintf (fp, ">\n    %s\n", string);
		fprintf (fp, "  </annotation>\n");

		XtFree (string);
	    }
	}
	fprintf (fp, "</Annotations>\n");

	/* finished */
	fclose (fp);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: annotmenu.c,v $ $Date: 2015/04/09 00:19:20 $ $Revision: 1.22 $ $Name:  $"};

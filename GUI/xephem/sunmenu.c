/* code to manage the sun display 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/DrawingA.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>

#include "xephem.h"

static void sun_create_shell (void);
static void sun_help_cb (Widget w, XtPointer client, XtPointer data);
static void sun_close_cb (Widget w, XtPointer client, XtPointer data);
static void sun_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void sun_fpd_cb (Widget w, XtPointer client, XtPointer data);
static void sun_load_cb (Widget w, XtPointer client, XtPointer data);
static void sun_exp_cb (Widget w, XtPointer client, XtPointer data);
static void sun_download_cb (Widget w, XtPointer client, XtPointer data);
static void sun_save_cb (Widget w, XtPointer client, XtPointer data);
static void sun_load1(void);
static void sun_print_cb (Widget w, XtPointer client, XtPointer data);
static void sun_mloop_cb (Widget w, XtPointer client, XtPointer data);
static void sun_motion_eh (Widget w, XtPointer client, XEvent *ev,
    Boolean *continue_to_dispatch);
static int readSOHOImage(void);
static int displayPic (char fn[], unsigned char *pic, int picl);
static void storePicFN (char fn[], int w);
static int getTS (int *tp, int *sp);
static int scanDir (char *dir, char ***files, int nfiles);
static void sun_refresh(void);
static int sun_ano (double *fracX, double *fracY, int *xp, int *yp, int w2x,
    int arg);
static void sun_print(void);
static void sun_ps_annotate(void);
static void location (int x, int dx, int y, int dy, double scale, double *rap,
    double *decp);
static int sun_gather (char ***files);
static void sun_display (FILE *fp, char *name);
static double carrington(Now *np);

#define	SUNPOLERA	degrad(286.13000)	/* RA of n pole */
#define	SUNPOLEDEC	degrad(63.87000)	/* dec of n pole */

#define	NSREAD	2048		/* size of socket read */

static Widget sunshell_w;	/* main shell */
static Widget sunda_w;		/* main drawing area */
static Widget ssw_w;		/* scrolled window for sun */
static Widget lpd_w;		/* Load pulldown */
static Widget bytype_w;		/* whether to filter by Type */
static Widget bysize_w;		/* whether to filter by Size */
static Widget ts_w;		/* timestamp label */
static Widget save_w;		/* save PB */
static Widget coords_w;		/* coordinates label */
static Widget crn_w;		/* Carrington rotation number label */
static Pixmap sun_pm;		/* staging area */
static unsigned char *rawpic;	/* current raw picture file, if any */
static long rawpicl;		/* length of rawpic */
static char *picfn;		/* malloced name of current picture */

/* info about each image format choice */
typedef struct {
    char *name;			/* widget name of this choice */
    char *label;		/* widget label of this choice */
    char *file;			/* string in file name for this type */
    int dx, dy;			/* shift from image center to sun @ 1024 */
    float scale;		/* image scale, rads/pixel @ 1024 */
    Widget w;			/* controlling TB */
} SOHOType;
/* N.B. order must agree with switch in sun_print() */
static SOHOType stype[] = {
    {"EIT171",	"EIT 171",         "eit_171",	  0,  0, degrad(0.00075)},
    {"EIT195",	"EIT 195",         "eit_195",	  0,  0, degrad(0.00075)},
    {"EIT284",	"EIT 284",         "eit_284",	  0,  0, degrad(0.00075)},
    {"EIT304",	"EIT 304",         "eit_304",	  0,  0, degrad(0.00075)},
    {"HMICon",	"HMI Continuum",   "hmi_igr",	  0,  0, degrad(0.00055)},
    {"HMIMag",	"HMI Magnetogram", "hmi_mag",	  0,  0, degrad(0.00055)},
    {"LASC2",	"LASCO C2",        "c2",	-16, 20, degrad(0.00365)},
    {"LASC3",	"LASCO C3",        "c3",	-16, 20, degrad(0.00775)},
};

/* info about size of image to get/display */
typedef struct {
    char *name;			/* widget name of this choice */
    char *label;		/* widget label of this choice */
    char *file;			/* string in file name for this size */
    int scale;			/* wrt 256 */
    Widget w;			/* controlling TB */
} SOHOSize;
static SOHOSize ssize[] = {
    {"512x512",		"512x512",         "512",	2},
    {"1024x1024",	"1024x1024",       "1024",	1},
};

static char *sohohost;			/* SOHO web site */
static char suncategory[] = "Sun";

/* bring up the sun menu, creating if first time */
void
sun_manage()
{
	if (!sunshell_w) {
	    sun_create_shell();
	    sohohost = getXRes ("SOHOhost", NULL);
	}

	XtPopup (sunshell_w, XtGrabNone);
	set_something (sunshell_w, XmNiconic, (XtArgVal)False);
	/* rely on expose to do fresh update */

	/* register we are now up */
	setXRes (sun_viewupres(), "1");
}

/* called to update our scene */
void
sun_update (Now *np, int how_much)
{
	if (!isUp(sunshell_w))
	    return;

	timestamp (np, ts_w);
	f_double (crn_w, "CRN: %.6f", carrington(np));
	if (sun_pm)
	    sun_refresh();
}

/* called to put up or remove the watch cursor.  */
void
sun_cursor (Cursor c)
{
	Window win;

	if (sunshell_w && (win = XtWindow(sunshell_w)) != 0) {
	    Display *dsp = XtDisplay(sunshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* called when basic resources change */
void
sun_newres()
{
	if (!sunshell_w)
	    return;
	sun_update (mm_get_now(), 1);
}

/* return the name of the resource containing whether this view is up */
char *
sun_viewupres()
{
	return ("SunViewUp");
}

/* create main shell */
static void
sun_create_shell ()
{
	Widget pd_w, cb_w, mb_w;
	Widget w, f_w;
	Arg args[20];
	char buf[64];
	int i;
	int n;
	
	/* create outter shell and form */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Sun"); n++;
	XtSetArg (args[n], XmNiconName, "Sun"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	sunshell_w = XtCreatePopupShell ("Sun", topLevelShellWidgetClass,
							toplevel_w, args, n);
	set_something (sunshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (sunshell_w, XmNpopdownCallback, sun_popdown_cb, 0);
	setup_icon (sunshell_w);
	sr_reg (sunshell_w, "XEphem*Sun.x", suncategory, 0);
	sr_reg (sunshell_w, "XEphem*Sun.y", suncategory, 0);
	sr_reg (sunshell_w, "XEphem*Sun.height", suncategory, 0);
	sr_reg (sunshell_w, "XEphem*Sun.width", suncategory, 0);
	sr_reg (NULL, sun_viewupres(), suncategory, 0);

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	f_w = XmCreateForm (sunshell_w, "SunF", args, n);
	XtAddCallback (f_w, XmNhelpCallback, sun_help_cb, 0);
	XtManageChild(f_w);

	/* menu bar */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (f_w, "SMB", args, n);
	XtManageChild (mb_w);

	    /* Control */

	    n = 0;
	    pd_w = XmCreatePulldownMenu (mb_w, "CPD", args, n);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'D'); n++;
		w = XmCreatePushButton (pd_w, "Download", args, n);
		set_xmstring (w, XmNlabelString, "Download latest");
		wtip (w, "Download latest SOHO image of given Type and Size");
		XtAddCallback (w, XmNactivateCallback, sun_download_cb, 0);
		XtManageChild (w);

		/* save not sensitive until image has been downloaded */
		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'S'); n++;
		XtSetArg (args[n], XmNsensitive, False); n++;
		save_w = XmCreatePushButton (pd_w, "Save", args, n);
		set_xmstring (save_w, XmNlabelString, "Save downloaded image");
		wtip (save_w, "Save newly downloaded image to local disk");
		XtAddCallback (save_w, XmNactivateCallback, sun_save_cb, 0);
		XtManageChild (save_w);

		n = 0;
		w = XmCreateSeparator (pd_w, "S1", args, n);
		XtManageChild (w);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'T'); n++;
		XtSetArg (args[n], XmNindicatorOn, True); n++;
		XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		bytype_w = XmCreateToggleButton (pd_w, "ByType", args, n);
		set_xmstring (bytype_w, XmNlabelString, "Filter Files by Type");
		wtip (bytype_w, "Whether Files restricts choices to Type");
		sr_reg (bytype_w, NULL, suncategory, 0);
		XtManageChild (bytype_w);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'F'); n++;
		XtSetArg (args[n], XmNindicatorOn, True); n++;
		XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		bysize_w = XmCreateToggleButton (pd_w, "BySize", args, n);
		set_xmstring (bysize_w, XmNlabelString, "Filter Files by Size");
		wtip (bysize_w, "Whether Files restricts choices to Size");
		sr_reg (bysize_w, NULL, suncategory, 0);
		XtManageChild (bysize_w);

		n = 0;
		w = XmCreateSeparator (pd_w, "S1", args, n);
		XtManageChild (w);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'P'); n++;
		w = XmCreatePushButton (pd_w, "Print", args, n);
		XtAddCallback (w, XmNactivateCallback, sun_print_cb, NULL);
		wtip (w, "Print the current SOHO image");
		set_xmstring (w, XmNlabelString, "Print...");
		XtManageChild (w);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'U'); n++;
		w = XmCreatePushButton (pd_w, "Ann", args, n);
		XtAddCallback (w, XmNactivateCallback, ano_cb, NULL);
		set_xmstring (w, XmNlabelString, "User annotation...");
		wtip (w, "Draw text and lines on curret SOHO image");
		XtManageChild (w);

		n = 0;
		n += ml_addacc (args, n);
		XtSetArg (args[n], XmNmnemonic, 'm'); n++;
		w = XmCreatePushButton (pd_w, "Movie", args, n);
		XtAddCallback (w, XmNactivateCallback, sun_mloop_cb, NULL);
		set_xmstring (w, XmNlabelString, "Add to movie...");
		wtip (w, "Add current image to movie loop");
		XtManageChild (w);

		n = 0;
		w = XmCreateSeparator (pd_w, "S1", args, n);
		XtManageChild (w);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'C'); n++;
		w = XmCreatePushButton (pd_w, "Close", args, n);
		wtip (w, "Close this SOHO window");
		XtAddCallback (w, XmNactivateCallback, sun_close_cb, NULL);
		XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNmnemonic, 'C'); n++;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Control", args, n);
	    XtManageChild (cb_w);

	    /* Load */

	    n = 0;
	    lpd_w = XmCreatePulldownMenu (mb_w, "FPD", args, n);
	    XtAddCallback (lpd_w, XmNmapCallback, sun_fpd_cb, NULL);

	    n = 0;
	    XtSetArg (args[n], XmNmnemonic, 'L'); n++;
	    XtSetArg (args[n], XmNsubMenuId, lpd_w); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Files", args, n);
	    wtip (cb_w, "Select a local SOHO file to display");
	    XtManageChild (cb_w);

	    /* Type */

	    n = 0;
	    XtSetArg (args[n], XmNradioBehavior, True); n++;
	    XtSetArg (args[n], XmNradioAlwaysOne, True); n++;
	    pd_w = XmCreatePulldownMenu (mb_w, "TPD", args, n);

		for (i = 0; i < XtNumber(stype); i++) {
		    SOHOType *tp = &stype[i];
		    n = 0;
		    XtSetArg (args[n], XmNindicatorOn, True); n++;
		    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		    tp->w = XmCreateToggleButton (pd_w, tp->name, args, n);
		    set_xmstring (tp->w, XmNlabelString, tp->label);
		    sprintf (buf, "XEphem*Sun*%s.set", tp->name);
		    sr_reg (tp->w, buf, suncategory, 0);
		    XtManageChild (tp->w);
		}

	    n = 0;
	    XtSetArg (args[n], XmNmnemonic, 'T'); n++;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Type", args, n);
	    wtip (cb_w, "Specify SOHO image type to download");
	    XtManageChild (cb_w);

	    /* Size */

	    n = 0;
	    XtSetArg (args[n], XmNradioBehavior, True); n++;
	    XtSetArg (args[n], XmNradioAlwaysOne, True); n++;
	    pd_w = XmCreatePulldownMenu (mb_w, "TPD", args, n);

		for (i = 0; i < XtNumber(ssize); i++) {
		    SOHOSize *sp = &ssize[i];
		    n = 0;
		    XtSetArg (args[n], XmNindicatorOn, True); n++;
		    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		    sp->w = XmCreateToggleButton (pd_w, sp->name, args, n);
		    set_xmstring (sp->w, XmNlabelString, sp->label);
		    sprintf (buf, "XEphem*Sun*%s.set", sp->name);
		    sr_reg (sp->w, buf, suncategory, 0);
		    XtManageChild (sp->w);
		}

	    n = 0;
	    XtSetArg (args[n], XmNmnemonic, 'S'); n++;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Size", args, n);
	    wtip (cb_w, "Specify SOHO image size to download");
	    XtManageChild (cb_w);

	    /* help */

	    n = 0;
	    pd_w = XmCreatePulldownMenu (mb_w, "TPD", args, n);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'H'); n++;
		w = XmCreatePushButton (pd_w, "Help", args, n);
		XtAddCallback (w, XmNactivateCallback, sun_help_cb, 0);
		XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNmnemonic, 'H'); n++;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Help", args, n);
	    set_something (mb_w, XmNmenuHelpWidget, (XtArgVal)cb_w);
	    XtManageChild (cb_w);

	/* time stamp and CRN at bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 50); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	ts_w = XmCreateLabel (f_w, "TS", args, n);
	XtManageChild (ts_w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 50); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	crn_w = XmCreateLabel (f_w, "CRN", args, n);
	XtManageChild (crn_w);

	/* cursor coords above */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, ts_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	coords_w = XmCreateLabel (f_w, "Coordinates", args, n);
	XtManageChild (coords_w);

	/* remainder is scrolled window for image */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, coords_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	XtSetArg (args[n], XmNshadowThickness, 1); n++;
	ssw_w = XmCreateScrolledWindow (f_w, "SunSW", args, n);
	XtManageChild (ssw_w);

	    /* workarea is a drawing area */

	    n = 0;
	    XtSetArg (args[n], XmNwidth, 1024); n++;	/* not critical */
	    XtSetArg (args[n], XmNheight, 1024); n++;	/* not critical */
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    sunda_w = XmCreateDrawingArea (ssw_w, "SunMap", args, n);
	    XtAddEventHandler (sunda_w,PointerMotionMask,False,sun_motion_eh,0);
	    XtAddCallback (sunda_w, XmNexposeCallback, sun_exp_cb, NULL);
	    XtManageChild (sunda_w);

	    /* SW assumes work is its child but just to be tidy about it .. */
	    set_something (ssw_w, XmNworkWindow, (XtArgVal)sunda_w);
}

/* called when the main scrolled area needs refreshing */
/* ARGSUSED */
static void
sun_exp_cb (Widget w, XtPointer client, XtPointer call)
{
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
	XExposeEvent *e = &c->event->xexpose;
	Now *np = mm_get_now();

	watch_cursor (1);

	switch (c->reason) {
	case XmCR_EXPOSE: {
	    /* turn off gravity so we get expose events for either shrink or
	     * expand.
	     * also center the scroll bars initially.
	     */
	    static int before;

	    if (!before) {
		XSetWindowAttributes swa;
		unsigned long mask = CWBitGravity | CWBackingStore;

		swa.bit_gravity = ForgetGravity;
		swa.backing_store = NotUseful;
		XChangeWindowAttributes (e->display, e->window, mask, &swa);

		before = 1;
	    }
	    break;
	    }
	default:
	    printf ("Unexpected mda_w event. type=%d\n", c->reason);
	    abort();
	}

	if (!sun_pm)
	    sun_load1();	/* load initial if possible */
	if (sun_pm)
	    sun_refresh();
	timestamp (np, ts_w);
	f_double (crn_w, "CRN: %.6f", carrington(np));

	watch_cursor (0);
}

/* ARGSUSED */
static void
sun_help_cb (Widget w, XtPointer client, XtPointer data)
{
	static char *msg[] = {
	    "This tool opens and displays solar images"
	};

	hlp_dialog ("Sun", msg, XtNumber(msg));
}

static void
sun_popdown_cb (Widget w, XtPointer client, XtPointer data)
{
	/* register we are now down */
	setXRes (sun_viewupres(), "0");
}

/* ARGSUSED */
static void
sun_close_cb (Widget w, XtPointer client, XtPointer data)
{
	/* let popdown do the real work */
	XtPopdown (sunshell_w);
}

static int
fn_cmp (const void *s1, const void *s2)
{
	return (strcmp (*(char **)s1, *(char **)s2));
}

/* ARGSUSED */
static void
sun_print_cb (Widget w, XtPointer client, XtPointer data)
{
	XPSAsk ("Sun View", sun_print);
}

/* ARGSUSED */
static void
sun_mloop_cb (Widget w, XtPointer client, XtPointer data)
{
	ml_add (sun_pm, ts_w);
}

/* proceed to generate a postscript file.
 * call XPSClose() when finished.
 */
static void
sun_print ()
{
	Display *dsp = XtDisplay(sunda_w);
	Window win = XtWindow(sunda_w);
	Window root;
	Pixmap drawpm;
	int rx, ry;
	unsigned int bw, dep;
	unsigned int wid, hei;
	int s, t;

	/* must be up and showing */
	if (!isUp(sunshell_w) || !sun_pm) {
	    xe_msg (1, "Sun image be open to print.");
	    XPSClose();
	    return;
	}

	watch_cursor(1);

	/* get size */
	getTS (&t, &s);
	s = 1024/ssize[s].scale;

	/* fit view in square across the top and prepare to capture X calls */
	XPSXBegin (sun_pm, 0, 0, s, s, 1*72, 10*72, (int)(6.5*72));

	/* since our normal expose refresh draws annotation directly into the
	 * sunda_w window (which in turn is because we don't want to burn it
	 * into the sun_pm which we create only once when the image is read)
	 * we have to build another temp pixmap here just to add annotation
	 * Also, it would look nice to block out the black background of
	 * the MDI images but they use the same color in the dark sunspots.
	 */
	XGetGeometry (dsp, win, &root, &rx, &ry, &wid, &hei, &bw, &dep);
	drawpm = XCreatePixmap (dsp, win, wid, hei, dep);
	XCopyArea (XtD, sun_pm, drawpm, DefaultGC(XtD, DefaultScreen(XtD)),
							0, 0, wid, hei, 0, 0);
	ano_draw (sunda_w, drawpm, sun_ano, 0);
	XPSPixmap (drawpm, s, s, xe_cm, 0, 0);
	XFreePixmap (dsp, drawpm);

        /* no more X captures */
	XPSXEnd();

	/* add some extra info */
	sun_ps_annotate ();

	/* finished */
	XPSClose();

	watch_cursor(0);
}

static void
sun_ps_annotate()
{
	Now *np = mm_get_now();
        char dir[128];
	char buf[128];
	int ctr = 306;  /* = 8.5*72/2 */
	int lx = ctr-5;
	int y;

	/* caption */
	y = AROWY(13);
	(void) strcpy (buf, "XEphem Solar View");
	(void) sprintf (dir, "(%s) %d %d cstr", buf, ctr, y);
	XPSDirect (dir);

	y = AROWY(9);
	fs_date (buf, pref_get(PREF_DATE_FORMAT), mjd_day(mjd));
	(void) sprintf (dir, "(UTC Date:) %d %d rstr (%s) %d %d lstr\n",
							lx, y, buf, lx+10, y);
	XPSDirect (dir);

	y = AROWY(8);
	fs_time (buf, mjd_hr(mjd));
	(void) sprintf (dir, "(UTC Time:) %d %d rstr (%s) %d %d lstr\n",
							lx, y, buf, lx+10, y);
	XPSDirect (dir);

	y = AROWY(7);
	sprintf (buf, "%.6f", carrington(np));
	(void) sprintf (dir,"(Carrington number:) %d %d rstr (%s) %d %d lstr\n",
							lx, y, buf, lx+10, y);
	XPSDirect (dir);
}

/* load one file initially for fun */
static void
sun_load1()
{
	char **files;
	int nfiles;

	nfiles = sun_gather (&files);
	if (nfiles > 0) {
	    FILE *fp = fopend (files[0], "auxil", "r");
	    sun_display (fp, files[0]);
	    fclose (fp);
	    XtFree ((char *) files);
	}
}

/* gather and sort all qualified file names.
 * N.B. caller must free *files
 */
static int
sun_gather (char ***files)
{
	char sdir[1024];
	int nfiles = 0;

	*files = NULL;
	sprintf (sdir, "%s/auxil", getShareDir());
	nfiles += scanDir (sdir, files, nfiles);
	nfiles += scanDir (getPrivateDir(), files, nfiles);
	qsort (*files, nfiles, sizeof(char *), fn_cmp);

	return (nfiles);
}

/* display the given picture file stream */
static void
sun_display (FILE *fp, char *name)
{
	unsigned char *pic;
	int picl;

	fseek (fp, 0L, SEEK_END);
	picl = (int) ftell(fp);
	pic = (unsigned char *) XtMalloc (picl);
	fseek (fp, 0L, SEEK_SET);
	fread (pic, picl, 1, fp);
	if (displayPic (name, pic, picl) < 0)
	    XtFree ((char *)pic);		/* already told user why */
}

/* called when the Files pulldown is about to come up */
/* ARGSUSED */
static void
sun_fpd_cb (Widget w, XtPointer client, XtPointer data)
{
	WidgetList children;
	Cardinal numChildren;
	char **files;
	int i, nfiles;

	/* gather and sort all file names */
	nfiles = sun_gather (&files);

	/* load into pulldown menu */
	get_something (lpd_w, XmNchildren, (XtArgVal)&children);
	get_something (lpd_w, XmNnumChildren, (XtArgVal)&numChildren);
	for (i = 0; i < nfiles; i++) {
	    Widget w;
	    if (i < numChildren) {
		w = children[i];
	    } else {
		w = XmCreatePushButton (lpd_w, "LPDPB", NULL, 0);
		XtAddCallback (w, XmNactivateCallback, sun_load_cb, NULL);
	    }
	    set_xmstring (w, XmNlabelString, files[i]);
	    XtManageChild (w);
	}

	/* turn off unused buttons */
	for (; i < numChildren; i++)
	    XtUnmanageChild (children[i]);

	/* set to multi-column if lots of files */
	if (nfiles >= 5)
	    set_something (w,XmNnumColumns,(XtArgVal)((int)(sqrt(nfiles/5.0))));
	else
	    set_something (w, XmNnumColumns, (XtArgVal)1);

	/* finished with list */
	XtFree ((char *) files);
}

/* called to load a local file when a Load PB is clicked.
 * local file name is our labelString
 */
/* ARGSUSED */
static void
sun_load_cb (Widget w, XtPointer client, XtPointer data)
{
	char *name;
	FILE *fp;
	int i;

	get_xmstring (w, XmNlabelString, &name);

	fp = fopend (name, "auxil", "r");
	if (fp) {
	    /* display */
	    sun_display (fp, name);
	    fclose (fp);

	    /* set type and size in menus to match based on fn */
	    for (i = 0; i < XtNumber(stype); i++) {
		if (strstr (name, stype[i].file)) {
		    XmToggleButtonSetState (stype[i].w, True, True);
		    break;
		}
	    }
	    for (i = 0; i < XtNumber(ssize); i++) {
		if (strstr (name, ssize[i].file)) {
		    XmToggleButtonSetState (ssize[i].w, True, True);
		    break;
		}
	    }

	    /* can not save a file loaded from disk */
	    XtSetSensitive (save_w, False);
	}

	XtFree (name);
}

/* called to download the latest image of the current type and size */
/* ARGSUSED */
static void
sun_download_cb (Widget w, XtPointer client, XtPointer data)
{
	watch_cursor (1);

	/* read image */
	if (readSOHOImage() < 0) {
	    stopd_down();
	    watch_cursor (0);
	    return;
	}

	/* ok to save */
	XtSetSensitive (save_w, True);

	stopd_down();
	watch_cursor (0);
}

/* called to save giffn[], if any, in private dir */
/* ARGSUSED */
static void
sun_save_cb (Widget w, XtPointer client, XtPointer data)
{
	char fn[1024];
	FILE *fp;

	/* never allowed again until new download */
	XtSetSensitive (save_w, False);

	/* sanity check */
	if (!picfn || !rawpic) {
	    xe_msg (1, "No image to save");
	    return;
	}

	/* create file */
	sprintf (fn, "%s/%s", getPrivateDir(), picfn);
	fp = fopenh (fn, "w");
	if (!fp) {
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	    return;
	}

	/* write */
	if (fwrite (rawpic, rawpicl, 1, fp) != 1) {
	    xe_msg (1, "%s:\n%s", picfn, syserrstr());
	    fclose (fp);
	    return;
	}

	/* ok */
	if (confirm())
	    xe_msg (1, "%s:\nWritten successfully", picfn);
	fclose (fp);
}

/* called to display coords from cursor roaming */
static void
sun_motion_eh (Widget w, XtPointer client, XEvent *ev, Boolean *dispatch)
{
	Display *dsp = XtDisplay(w);
	Window win = XtWindow(w);
	SOHOType *tp;
	SOHOSize *sp;
	int t, s;
	Window root, child;
	int rx, ry, wx, wy;
	unsigned mask;
	unsigned int bw, dep;
	unsigned int wid, hei;
	double r, d;
	char cbuf[64], rbuf[32], dbuf[32];

	if (!picfn || !rawpic)
	    return;

	XGetGeometry (dsp, win, &root, &rx, &ry, &wid, &hei, &bw, &dep);
	XQueryPointer (dsp, win, &root, &child, &rx, &ry, &wx, &wy, &mask);

	getTS (&t, &s);
	tp = &stype[t];
	sp = &ssize[s];
	location (wx-wid/2, tp->dx/sp->scale, wy-hei/2, tp->dy/sp->scale,
						tp->scale*sp->scale, &r, &d);

	fs_sexa (rbuf, radhr(r), 3, 600);
	fs_sexa (dbuf, raddeg(d), 3, 60);
	sprintf (cbuf, "RA, Dec: %s %s", rbuf, dbuf);
	set_xmstring (coords_w, XmNlabelString, cbuf);
}

/* read picture from sohohost and display.
 * store picture in rawpic[] and its filename in *picfn.
 * return 0 if ok else xe_msg and -1
 */
static int
readSOHOImage()
{
	char fn[1024];
	int t, s;
	char get[1024];
	char buf[1024];
	char filetime[100];
	char filetype[100];
	int isjpeg, jpegl;
	int njpeg;
	unsigned char *jpeg;
	int fd, nr;
	struct tm tm;

	memset(&tm, 0, sizeof(struct tm));

	/* get desired type and size */
	if (getTS (&t, &s) < 0)
	    return (-1);

	/* build latest fn */
	sprintf (filetype, "%s", stype[t].file);
	sprintf (fn, "/data/realtime/%s/%s/latest.jpg", filetype, ssize[s].file);

	/* build GET command */
	sprintf (get, "GET http://%s%s HTTP/1.0\r\nUser-Agent: xephem/%s\r\n\r\n", sohohost, fn, PATCHLEVEL);

	/* query server */
	fd = httpGET (sohohost, get, buf);
	if (fd < 0) {
	    xe_msg (1, "http get: %s", buf);
	    return (-1);
	}

	/* read header (everything to first blank line), looking for jpeg */
	isjpeg = 0;
	jpegl = 0;
	while (recvline (fd, buf, sizeof(buf)) > 1) {
	    xe_msg (0, "Rcv: %s", buf);
	    if (strstr (buf, "Content-Type:") && strstr (buf, "image/jpeg"))
		isjpeg = 1;
	    if (strstr (buf, "Content-Length:"))
		jpegl = atoi (buf+15);
	    if (strstr (buf, "Last-Modified:")) {
		strptime(buf+20, "%d %b %Y %H:%M:%S %Z", &tm);
		strftime(filetime, sizeof(filetime), "%Y%m%d_%H%M%S", &tm);
		xe_msg (0, "Rcv: Filetime: %s", filetime);
	    }
	}
	if (!isjpeg) {
	    while (recvline (fd, buf, sizeof(buf)) > 0)
		xe_msg (0, "Rcv: %s", buf);
	    xe_msg (1, "Error talking to SOHO .. see File->System log\n");
	    close (fd);
	    return (-1);
	}
	if (jpegl == 0) {
	    xe_msg (1, "No Content-Length in header");
	    close (fd);
	    return (-1);
	}

	/* remainder should be a jpeg file, read into jpeg[] */
	pm_up();
	jpeg = NULL;
	for (njpeg = 0; njpeg < jpegl; njpeg += nr) {
	    pm_set (100*njpeg/jpegl);
	    jpeg = (unsigned char *) XtRealloc ((char*)jpeg, njpeg+NSREAD);
	    nr = readbytes (fd, jpeg+njpeg, NSREAD);
	    if (nr < 0) {
		xe_msg (1, "%s:\n%s", sohohost, syserrstr());
		pm_down();
		close (fd);
		return (-1);
	    }
	    if (nr == 0)
		break;
	}
	pm_down();
	close (fd);

        sprintf (fn, "/%s_%s.jpg", filetime, filetype);
	/* display jpeg */
	if (displayPic (fn, jpeg, njpeg) < 0)
	    return (-1);

	/* clean up */
	return (0);
}

/* display the picture file named fn and install as the "current" file.
 * return 0 if ok, else -1 and why xe_msg()
 * N.B. picture must be malloced memory and user should /not/ free, let us manage it
 */
static int
displayPic (char fn[], unsigned char *pic, int picl)
{
	static XColor xcol[256];
	static int xcolset;
	Pixmap pm;
	char buf[1024];
	int w, h;
	int ok = 0;

	/* explode into pm */
	if (strstr (fn, ".gif")) {
	    ok = 1;
	    if (gif2pm (XtD, xe_cm, pic, picl, &w, &h, &pm, buf) < 0) {
		xe_msg (1, "Display GIF: %s:\n%s", fn, buf);
		return (-1);
	    }
	}
	if (strstr (fn, ".jpg")) {
	    char tmpfn[2048];
	    FILE *tmpfp;

	    ok = 1;

	    /* free last batch of colors */
	    if (xcolset) {
		freeXColors (XtD, xe_cm, xcol, XtNumber(xcol));
		xcolset = 0;
	    }

	    /* convert JPEG to pixmap */
	    (void) tempfilename (tmpfn, "suntmp", ".jpg");
	    tmpfp = fopen (tmpfn, "wb+");
	    if (!tmpfp) {
		xe_msg (1, "Can not create tmp file: %s", tmpfn);
		return (-1);
	    }
	    fwrite(pic, picl, 1, tmpfp);
	    rewind(tmpfp);
	    if (jpeg2pm (XtD, xe_cm, tmpfp, &w, &h, &pm, xcol, buf) < 0) {
		xe_msg (1, "Display JPEG: %s:\n%s", fn, buf);
		fclose (tmpfp);
		unlink (tmpfn);
		return (-1);
	    }
	    xcolset = 1;
	    fclose (tmpfp);
	    unlink (tmpfn);
	}

	if (ok == 0) {
	    xe_msg (1, "Unknown file type: %s", fn);
	    return (-1);
	}

	/* replace pixmap and center */
	if (sun_pm)
	    XFreePixmap (XtD, sun_pm);
	sun_pm = pm;
	set_something (sunda_w, XmNwidth, (XtArgVal)(Dimension)w);
	set_something (sunda_w, XmNheight, (XtArgVal)(Dimension)h);
	centerScrollBars(ssw_w);
	sun_refresh();
	if (rawpic)
	    XtFree ((char *)rawpic);
	rawpic = pic;
	rawpicl = picl;
	storePicFN (fn, w);

	return (0);
}

/* store base of fn[] in picfn[], include width w in name somewhere */
static void
storePicFN (char fn[], int w)
{
	char buf[1024];
	char *sp, *lsp;

	/* find last slash in fn */
	lsp = NULL;
	for (sp = fn; sp; sp = strchr (sp+1, '/'))
	    lsp = sp;
	if (!lsp) {
	    printf ("Bug! no / in filename '%s'\n", fn);
	    abort();
	}

	sprintf (buf, "SOHO_%d_%s", w, lsp+1);
	if (picfn)
	    XtFree (picfn);
	picfn = XtNewString (buf);
}

/* get indices into user's current type and size settings.
 * either can be NULL if not interested.
 */
static int
getTS (int *tp, int *sp)
{
	int i;

	if (tp) {
	    for (i = 0; i < XtNumber(stype); i++) {
		if (XmToggleButtonGetState (stype[i].w)) {
		    *tp = i;
		    break;
		}
	    }
	    if (i == XtNumber(stype)) {
		xe_msg (1, "No SOHO type");
		return (-1);
	    }
	}

	if (sp) {
	    for (i = 0; i < XtNumber(ssize); i++) {
		if (XmToggleButtonGetState (ssize[i].w)) {
		    *sp = i;
		    break;
		}
	    }
	    if (i == XtNumber(ssize)) {
		xe_msg (1, "No SOHO size");
		return (-1);
	    }
	}

	return (0);
}

static int
scanDir (char *dir, char ***files, int nfiles)
{
	int bytype = XmToggleButtonGetState (bytype_w);
	int bysize = XmToggleButtonGetState (bysize_w);
	int t, s;
	DIR *dirp;
	struct dirent *dp;
	int n;

	/* get type and size settings if currently interested */
	if ((bytype || bysize) && getTS (&t, &s) < 0)
	    return (0);

	/* open and scan the directory for matching names */
	dirp = opendir (dir);
	if (!dirp)
	    return (0);
	for (n = 0; (dp = readdir (dirp)) != NULL; ) {
	    if (strncmp (dp->d_name, "SOHO", 4) == 0 &&
			    (!bytype || strstr (dp->d_name, stype[t].file)) &&
			    (!bysize || strstr (dp->d_name, ssize[s].file))) {
		*files = (char **) XtRealloc ((char *)(*files),
					    (nfiles + n + 1) * sizeof(char *));
		(*files)[nfiles + n++] = XtNewString (dp->d_name);
	    }
	}

	/* finished */
	closedir (dirp);
	return (n);
}

static void
sun_refresh()
{
	Dimension w, h;

	get_something (sunda_w, XmNwidth, (XtArgVal)&w);
	get_something (sunda_w, XmNheight, (XtArgVal)&h);
	XCopyArea (XtD, sun_pm, XtWindow(sunda_w),
			DefaultGC(XtD, DefaultScreen(XtD)), 0, 0, w, h, 0, 0);

	/* draw annotation directly onto window so we don't dirty up the
	 * pixmap (which we only load with the image once)
	 */
	ano_draw (sunda_w, XtWindow(sunda_w), sun_ano, 0);
}

/* convert image proportions to/from X windows coords depending on w2x.
 * return whether visible.
 */
static int
sun_ano (double *fracX, double *fracY, int *xp, int *yp, int w2x, int arg)
{
	Display *dsp = XtDisplay (sunda_w);
	unsigned int nx, ny, bw, d;
	Window root;
	int x, y;

	XGetGeometry(dsp, sun_pm, &root, &x, &y, &nx, &ny, &bw, &d);

	if (w2x) {
	    *xp = (int)floor(*fracX*nx + 0.5);
	    *yp = (int)floor(*fracY*ny + 0.5);
	} else {
	    *fracX = (double)*xp/nx;
	    *fracY = (double)*yp/ny;
	}

	return (*xp>=0 && *xp<nx && *yp>=0 && *yp<ny);
}

/* given location in X pixels from center of image, scale in rads/pix,
 * dy is the amount by which the sun is below the center of the image.
 * return ra/dec at current moment.
 */
static void
location (int x, int dx, int y, int dy, double scale, double *rap, double *decp)
{
	Obj *sop = db_basic (SUN);
	double sd = sin (sop->s_dec);
	double cd = cos (sop->s_dec);
	double xp, yp, p, cp, sp;

	/* SOHO image is always rotated with N solar axis vertical.
	 * find rotation, p, to bring equatoral N up.
	 */
	solve_sphere (sop->s_ra-SUNPOLERA, PI/2-SUNPOLEDEC, sd, cd, NULL, &p);

	/* rotate screen coords so eq N is up */
	cp = cos(p);
	sp = sin(p);
	y -= dy;
	x -= dx; 
	xp =  x*cp - y*sp;
	yp =  x*sp + y*cp;

	*decp = sop->s_dec - yp*scale;
	*rap = sop->s_ra - xp*scale/cos(*decp);
	range (rap, 2*PI);
}

/* display carrington coord for current time */
static double
carrington(Now *np)
{
	return ((1./27.2753)*((mjd+MJD0)-2398167.0) + 1.0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: sunmenu.c,v $ $Date: 2012/03/07 01:10:15 $ $Revision: 1.23 $ $Name:  $"};


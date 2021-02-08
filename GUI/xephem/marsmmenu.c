/* code to manage the mars moon stuff.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Separator.h>
#include <Xm/Scale.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>

#include "xephem.h"

extern char marscategory[];


#define	MINR	10	/* min distance for picking, pixels */

/* local record of what is now on screen for easy id from mouse picking.
 */
typedef struct {
    Obj o;		/* copy of object info.
    			 * copy from DB or, if mars moon, we fill in just
			 * o_name and s_mag/ra/dec
			 */
    int x, y;		/* screen coord */
} ScreenObj;

/* malloced arrays for each view */
typedef enum {
    SO_MAIN, SO_SHADOW, SO_TOP, SO_N
} SOIdx;
static ScreenObj *screenobj[SO_N];
static int nscreenobj[SO_N];

static void marsm_create_shell_w (void);
static void marsm_create_tvform_w (void);
static void marsm_create_msform_w (void);
static void marsm_set_buttons (int whether);
static void marsm_sstats_close_cb (Widget w, XtPointer client,
    XtPointer call);
static void marsm_sstats_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_option_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_cpdmapping_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_scale_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_activate_cb (Widget w, XtPointer client, XtPointer call);
static void mt_unmap_cb (Widget w, XtPointer client, XtPointer call);
static void mt_map_cb (Widget w, XtPointer client, XtPointer call);
static void mt_track_size (void);
static void mt_da_exp_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_close_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_mloop_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_anim_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_da_exp_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_da_input_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_create_popup (void);
static void marsm_fill_popup (ScreenObj *sop, int justname);
static void marsm_help_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_helpon_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_goto_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_print_cb (Widget w, XtPointer client, XtPointer call);
static void marsm_print (void);
static void make_gcs (Widget w);
static void marsm_ps_annotate (Now *np);
static void marsm_calibline (Display *dsp, Drawable win, GC gc, int xc,
    int yc, char *tag, int tw, int th, int l);
static void marsm_draw_map (Widget w, Obj *mop, double msize,
    MoonData md[M_NMOONS]);
static int marsm_ano (double *mX, double *mZ, int *xp, int *yp, int w2x,
    int arg);
static int mtm_ano (double *mX, double *mZ, int *xp, int *yp, int w2x, int arg);

static void add_screenobj (SOIdx, Obj *op, int x, int y);
static void reset_screenobj (SOIdx);
static ScreenObj *close_screenobj (SOIdx, int x, int y);

static void sky_background (Drawable win, unsigned w, unsigned h, int fmag,
    double ra0, double dec0, double scale, double rad, int fliptb, int fliplr);

static Widget mmshell_w;	/* main shell */
static Widget msform_w;		/* statistics form */
static Widget mmframe_w;	/* main frame */
static Widget mtform_w;		/* top-view form */
static Widget mtframe_w;	/* top-view frame */
static Widget mda_w;		/* main drawing area */
static Widget mtda_w;		/* top-view drawing area */
static Widget scale_w;		/* size scale */
static Widget limmag_w;		/* limiting magnitude scale */
static Widget dt_w;		/* main date/time stamp widget */
static Widget sdt_w;		/* statistics date/time stamp widget */
static Widget skybkg_w;		/* toggle for controlling sky background */
static Widget topview_w;	/* toggle for controlling top view */
static Widget tel_w;            /* PB to send position to telescope */
static Pixmap marsm_pm;		/* main pixmap */
static Pixmap mt_pm;		/* top-view pixmap */
static GC marsm_fgc, marsm_bgc;	/* various colors and operators */
static GC marsm_xgc;		/* xor for moons and labels */
static XFontStruct *marsm_fs;	/* font for labels */
static int marsm_cw, marsm_ch;	/* size of label font */
static int marsm_selecting;	/* set while our fields are being selected */
static int brmoons;		/* whether we want to brightten the moons */
static int tags;		/* whether we want tags on the drawing */
static int flip_tb;		/* whether we want to flip top/bottom */
static int flip_lr;		/* whether we want to flip left/right */
static int skybkg;		/* whether we want sky background */
static int topview;		/* whether we want the top view */

static Widget mmpu_w;		/* main popup */
static Widget mmpu_name_w;	/* popup name label */
static Widget mmpu_ra_w;	/* popup RA label */
static Widget mmpu_dec_w;	/* popup Dec label */
static Widget mmpu_mag_w;	/* popup Mag label */

#define	MAXSCALE	10.0	/* max scale mag factor */
#define	NORM		100.0	/* max radius; used to normalize */
#define	MAPSCALE(r)	((r)*((int)nx)/NORM/2*scale)
#define	XCORD(x)	((int)(((int)nx)/2.0 + ew*MAPSCALE(x) + 0.5))
#define	YCORD(y)	((int)(((int)ny)/2.0 - ns*MAPSCALE(y) + 0.5))
#define ZCORD(z)        ((int)(((int)ny)/2.0 +    MAPSCALE(z) + 0.5))
#define	MOVIE_STEPSZ	(1./12.)/* movie step size, hours */
#define PRTR            20      /* printing table row, up from bottom */
#define PRCW            7       /* printing char width */

/* field star support */
static ObjF *fstars;		/* malloced list of field stars, or NULL */
static int nfstars;		/* number of entries in fstars[] */
static double fsdec, fsra;	/* location when field stars were loaded */
#define	FSFOV	degrad(1.0)	/* size of FOV to fetch, rads */
#define	FSMAG	20.0		/* limiting mag for fetch */
#define	FSMOVE	degrad(.2)	/* reload when mars has moved this far, rads */
static void marsm_loadfs (double ra, double dec);

enum {CEV, CSV, CPS, CTR, CX, CY, CZ, CRA, CDEC, CMAG, CNum};/*marsm_w col idx*/
static Widget	marsm_w[M_NMOONS][CNum];	/* the data display widgets */

static double pole_ra, pole_dec;

/* called when the mars moon view is activated from the mars pulldown.
 * then we begin a life of our own as a toplevel shell.
 * if never called before, create and manage all the widgets as a child of a
 * form. otherwise, just insure up.
 */
void
marsm_manage ()
{
	if (!mmshell_w) {
	    marsm_create_shell_w();
	    marsm_create_msform_w();
	    marsm_create_tvform_w();
	}
	
	XtPopup (mmshell_w, XtGrabNone);
	set_something (mmshell_w, XmNiconic, (XtArgVal)False);
	marsm_set_buttons(marsm_selecting);
}

/* called to recompute and fill in values for the Mars moon menu.
 * don't bother if it doesn't exist or is unmanaged now or no one is logging.
 */
void
marsm_update (np, how_much)
Now *np;
int how_much;
{
	static char fmt[] = "%7.3f";
	Obj *eop = db_basic (SUN);
	Obj *mop = db_basic (MARS);
	MoonData md[M_NMOONS];
	char *dir, buf[1024];
	int wantstats;
	double msize;
	int i;

	/* see if we should bother */
	if (!mmshell_w)
	    return;
	wantstats = XtIsManaged(msform_w) || any_ison() || how_much;
	if (!isUp(mmshell_w) && !wantstats)
	    return;

	watch_cursor (1);

        /* compute md[0].x/y/z/mag/ra/dec and md[1..NM-1].x/y/z/mag info */
	sprintf (buf, "%s/auxil", getShareDir());
	dir = expand_home (buf);
        marsm_data (mjd, dir, eop, mop, &msize, &pole_ra, &pole_dec, md);

	if (wantstats) {
	    for (i = 0; i < M_NMOONS; i++) {
		if (i > 0) {
		    f_double (marsm_w[i][CEV], "%1.0f", (double)md[i].evis);
		    f_double (marsm_w[i][CSV], "%1.0f", (double)md[i].svis);
		    f_double (marsm_w[i][CPS], "%1.0f", (double)md[i].pshad);
		    f_double (marsm_w[i][CTR], "%1.0f", (double)md[i].trans);
		    f_double (marsm_w[i][CX], fmt, md[i].x);
		    f_double (marsm_w[i][CY], fmt, md[i].y);
		    f_double (marsm_w[i][CZ], fmt, md[i].z);
		}
		f_double (marsm_w[i][CMAG], "%4.1f", md[i].mag);
		f_ra (marsm_w[i][CRA], md[i].ra);
		f_prdec (marsm_w[i][CDEC], md[i].dec);
	    }

	    timestamp (np, sdt_w);
	}

	if (isUp(mmshell_w)) {
	    marsm_draw_map (mda_w, mop, msize, md);
	    timestamp (np, dt_w);
	}

	watch_cursor (0);
}

/* called when basic resources change.
 * rebuild and redraw.
 */
void
marsm_newres()
{
	if (!mmshell_w)
	    return;
	make_gcs (mda_w);
	marsm_update (mm_get_now(), 1);
}

/* called when the database has changed.
 * if we are drawing background, we'd best redraw everything.
 */
/* ARGSUSED */
void
marsm_newdb (appended)
int appended;
{
	if (skybkg)
	    marsm_update (mm_get_now(), 1);
}

int
marsm_ison()
{
	return (isUp(mmshell_w));
}

/* called by other menus as they want to hear from our buttons or not.
 * the "on"s and "off"s stack - only really redo the buttons if it's the
 * first on or the last off.
 */
void
marsm_selection_mode (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	if (whether)
	    marsm_selecting++;
	else if (marsm_selecting > 0)
	    --marsm_selecting;

	if (mmshell_w)
	    if ((whether && marsm_selecting == 1)     /* first one to want on */
		|| (!whether && marsm_selecting == 0) /* last one to want off */)
		marsm_set_buttons (whether);
}

/* called to put up or remove the watch cursor.  */
void
marsm_cursor (c)
Cursor c;
{
	Window win;

	if (mmshell_w && (win = XtWindow(mmshell_w)) != 0) {
	    Display *dsp = XtDisplay(mmshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}

	if (msform_w && (win = XtWindow(msform_w)) != 0) {
	    Display *dsp = XtDisplay(msform_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}

	if (mtform_w && (win = XtWindow(mtform_w)) != 0) {
	    Display *dsp = XtDisplay(mtform_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create the main moons shell */
static void
marsm_create_shell_w()
{
	typedef struct {
	    char *name;		/* toggle button instance name, or NULL */
	    char *label;	/* label */
	    int *flagp;		/* pointer to global flag, or NULL if none */
	    Widget *wp;		/* widget to save, or NULL */
	    char *tip;		/* widget tip */
	} Option;
	static Option options[] = {
	    {NULL,	   "Top view",		&topview, &topview_w,
	    	"When on, show view looking from high above N pole"},
	    {"SkyBkg",	   "Sky background",	&skybkg,  &skybkg_w,
	    	"When on, sky will include database objects and Field Stars"},
	    {"BrightMoons","Bright moons",	&brmoons, NULL,
		"Display moons disproportionately bright, even if below limit"},
	    {"Tags",	   "Tags",		&tags,    NULL,
		"Label each moon with its Roman Numeral sequence number"},
	    {"FlipTB",	   "Flip T/B",		&flip_tb, NULL,
		"Flip the display top-to-bottom"},
	    {"FlipLR",	   "Flip L/R",		&flip_lr, NULL,
		"Flip the display left-to-right"},
	};
	typedef struct {
	    char *label;	/* what goes on the help label */
	    char *key;		/* string to call hlp_dialog() */
	} HelpOn;
	static HelpOn helpon[] = {
	    {"Intro...",	"MarsMoons"},
	    {"on Mouse...",	"MarsMoons_mouse"},
	    {"on Control...",	"MarsMoons_control"},
	    {"on View...",	"MarsMoons_view"},
	};
	Widget w;
	Widget mb_w, pd_w, cb_w;
	Widget mmform_w;
	XmString str;
	Arg args[20];
	int n;
	int i;

	/* create form and its shell */

	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Mars moon view"); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNiconName, "MarsMoon"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	mmshell_w = XtCreatePopupShell ("MarsMoons", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (mmshell_w);
	set_something (mmshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (mmshell_w, XmNpopdownCallback, marsm_popdown_cb, 0);
	sr_reg (mmshell_w, "XEphem*MarsMoons.width", marscategory, 0);
	sr_reg (mmshell_w, "XEphem*MarsMoons.height", marscategory, 0);
	sr_reg (mmshell_w, "XEphem*MarsMoons.x", marscategory, 0);
	sr_reg (mmshell_w, "XEphem*MarsMoons.y", marscategory, 0);

	n = 0;
	mmform_w = XmCreateForm (mmshell_w, "MarsMoons", args, n);
	XtAddCallback (mmform_w, XmNhelpCallback, marsm_help_cb, 0);
	XtManageChild (mmform_w);

	/* create the menu bar across the top */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (mmform_w, "MB", args, n);
	XtManageChild (mb_w);

	/* make the Control pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "ControlPD", args, n);
	XtAddCallback (pd_w, XmNmapCallback, marsm_cpdmapping_cb, NULL);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'C'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "ControlCB", args, n);
	    set_xmstring (cb_w, XmNlabelString, "Control");
	    XtManageChild (cb_w);

	    /* add the "Print" push button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "P", args, n);
	    set_xmstring (w, XmNlabelString, "Print...");
	    XtAddCallback (w, XmNactivateCallback, marsm_print_cb, 0);
	    wtip (w, "Print front view and detail table");
	    XtManageChild (w);

	    /* add the "Anno" push button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "A", args, n);
	    set_xmstring (w, XmNlabelString, "User annotation...");
	    XtAddCallback (w, XmNactivateCallback, ano_cb, 0);
	    wtip (w, "Open window to create and manage your own annotation");
	    XtManageChild (w);

	    /* add the "Field stars" push button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "FS", args, n);
	    set_xmstring (w, XmNlabelString, "Field Stars...");
	    XtAddCallback (w, XmNactivateCallback, (XtCallbackProc)fs_manage,0);
	    wtip (w, "Define where GSC and PPM catalogs are to be found");
	    XtManageChild (w);

	    /* add the "GOTO" push button */

	    n = 0;
	    tel_w = XmCreatePushButton (pd_w, "GOTO", args, n);
	    set_xmstring (tel_w, XmNlabelString, "Telescope GoTo");
	    XtAddCallback (tel_w, XmNactivateCallback, marsm_goto_cb, 0);
	    wtip (tel_w, "Send position to external application");
	    XtManageChild (tel_w);

	    /* add the "movie" push button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Anim", args, n);
	    set_xmstring (w, XmNlabelString, "Animation demo");
	    XtAddCallback (w, XmNactivateCallback, marsm_anim_cb, 0);
	    wtip (w, "Start/Stop a fun time-lapse animation");
	    XtManageChild (w);

	    /* add the "Movie" push button */

	    n = 0;
	    n += ml_addacc (args, n);
	    w = XmCreatePushButton (pd_w, "ML", args, n);
	    set_xmstring (w, XmNlabelString, "Add to movie...");
	    XtAddCallback (w, XmNactivateCallback, marsm_mloop_cb, 0);
	    wtip (w, "Add this scene to the movie loop");
	    XtManageChild (w);

	    /* add the "close" push button beneath a separator */

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, marsm_close_cb, 0);
	    wtip (w, "Close this and all supporting dialogs");
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

	    for (i = 0; i < XtNumber(options); i++) {
		Option *op = &options[i];

		n = 0;
		XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
		w = XmCreateToggleButton (pd_w, op->name ? op->name : "MMTB",
								    args, n);
		XtAddCallback (w, XmNvalueChangedCallback, marsm_option_cb, 
						    (XtPointer)(op->flagp));
		set_xmstring (w, XmNlabelString, op->label);
		if (op->flagp)
		    *(op->flagp) = XmToggleButtonGetState(w);
		if (op->wp)
		    *op->wp = w;
		if (op->tip)
		    wtip (w, op->tip);
		XtManageChild (w);
		if (op->name)
		    sr_reg (w, NULL, marscategory, 1);
	    }

	    /* add a separator */

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    /* add the More Info control */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Stats", args, n);
	    set_xmstring (w, XmNlabelString, "More info...");
	    XtAddCallback (w, XmNactivateCallback, marsm_sstats_cb, NULL);
	    wtip (w, "Display additional details");
	    XtManageChild (w);

	/* make the help pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "HelpPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'H'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "HelpCB", args, n);
	    set_xmstring (cb_w, XmNlabelString, "Help");
	    XtManageChild (cb_w);
	    set_something (mb_w, XmNmenuHelpWidget, (XtArgVal)cb_w);

	    for (i = 0; i < XtNumber(helpon); i++) {
		HelpOn *hp = &helpon[i];

		str = XmStringCreate (hp->label, XmSTRING_DEFAULT_CHARSET);
		n = 0;
		XtSetArg (args[n], XmNlabelString, str); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		w = XmCreatePushButton (pd_w, "Help", args, n);
		XtAddCallback (w, XmNactivateCallback, marsm_helpon_cb,
							(XtPointer)(hp->key));
		XtManageChild (w);
		XmStringFree(str);
	    }

	/* make the date/time stamp label */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	dt_w = XmCreateLabel (mmform_w, "DateStamp", args, n);
	timestamp (mm_get_now(), dt_w);	/* establishes size */
	wtip (dt_w, "Date and Time for which map is computed");
	XtManageChild (dt_w);

	/* make the scale widget.
	 * attach both top and bottom so it's the one to follow resizing.
	 */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, dt_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNmaximum, 100); n++;
	XtSetArg (args[n], XmNminimum, 0); n++;
	XtSetArg (args[n], XmNscaleMultiple, 10); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
	scale_w = XmCreateScale (mmform_w, "Scale", args, n);
	XtAddCallback (scale_w, XmNdragCallback, marsm_scale_cb, 0);
	XtAddCallback (scale_w, XmNvalueChangedCallback, marsm_scale_cb, 0);
	wtip (scale_w, "Zoom in and out");
	XtManageChild (scale_w);
	sr_reg (scale_w, NULL, marscategory, 0);

	/* make the limiting mag scale widget.
	 * attach both top and bottom so it's the one to follow resizing.
	 */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, dt_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNmaximum, 20); n++;
	XtSetArg (args[n], XmNminimum, 0); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
	XtSetArg (args[n], XmNshowValue, True); n++;
	limmag_w = XmCreateScale (mmform_w, "LimMag", args, n);
	XtAddCallback (limmag_w, XmNdragCallback, marsm_scale_cb, 0);
	XtAddCallback (limmag_w, XmNvalueChangedCallback, marsm_scale_cb, 0);
	wtip (limmag_w, "Adjust the brightness and limiting magnitude");
	XtManageChild (limmag_w);
	sr_reg (limmag_w, NULL, marscategory, 0);

	/* make a frame for the drawing area.
	 * attach both top and bottom so it's the one to follow resizing.
	 */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, dt_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, scale_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, limmag_w); n++;
	XtSetArg (args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
	mmframe_w = XmCreateFrame (mmform_w, "UFrame", args, n);
	XtManageChild (mmframe_w);

	    /* make a drawing area for drawing the little map */

	    n = 0;
	    mda_w = XmCreateDrawingArea (mmframe_w, "MarsMoons", args, n);
	    XtAddCallback (mda_w, XmNexposeCallback, marsm_da_exp_cb, 0);
	    XtAddCallback (mda_w, XmNinputCallback, marsm_da_input_cb, 
	    						(XtPointer)SO_MAIN);
	    XtManageChild (mda_w);
}

/* make the statistics form dialog */
static void
marsm_create_msform_w()
{
	typedef struct {
	    int col;		/* C* column code */
	    int moononly;	/* applies to moons only */
	    char *collabel;	/* column label */
	    char *suffix;	/* suffix for plot/list/search name */
	    char *tip;		/* widget tip */
	} MoonColumn;
	static MoonColumn mc[] = {
	    {CEV,   1,   "E",     "EVis",
		"Whether geometrically visible from Earth"},
	    {CSV,   1,   "S",     "SVis",
		"Whether in Sun light"},
	    {CPS,   1,   "P",     "PShad",
		"Whether shadow falls on planet"},
	    {CTR,   1,   "T",     "Transit",
		"Whether moon is transitting face of planet"},
	    {CX,    1,   "X (+E)",     "X",
		"Apparent displacement east of planetary center"},
	    {CY,    1,   "Y (+S)",     "Y",
		"Apparent displacement south of planetary center"},
	    {CZ,    1,   "Z (+front)", "Z",
		"Mars radii towards Earth from planetary center"},
	    {CRA,   0,   "RA",         "RA",
		"Right Ascension (to Main's settings)"},
	    {CDEC,  0,   "Dec",        "Dec",
		"Declination (to Main's settings)"},
	    {CMAG,  0,   "Mag",        "Mag",
		"Apparent visual magnitude"},
	};
	MoonData md[M_NMOONS];
	Widget w;
	Widget rc_w, title_w, sep_w;
	Arg args[20];
	int n;
	int i;

	/* just get moon names */
	marsm_data (0.0, NULL, NULL, NULL, NULL, NULL, NULL, md);

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	msform_w = XmCreateFormDialog (mmshell_w, "MarsMoonStats", args, n);
	set_something (msform_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg(XtParent(msform_w), "XEphem*MarsMoonStats.x", marscategory, 0);
	sr_reg(XtParent(msform_w), "XEphem*MarsMoonStats.y", marscategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Mars Moon info"); n++;
	XtSetValues (XtParent(msform_w), args, n);

	/* make table title label */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNtopOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	title_w = XmCreateLabel (msform_w, "UML", args, n);
	XtManageChild (title_w);
	set_xmstring (title_w, XmNlabelString,"Moon Positions (Martian radii)");

	/* make the moon table, one column at a time */

	/* moon designator column */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, title_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_BEGINNING); n++;
	XtSetArg (args[n], XmNisAligned, True); n++;
	rc_w = XmCreateRowColumn (msform_w, "UDes", args, n);
	XtManageChild (rc_w);

	    n = 0;
	    w = XmCreateLabel (rc_w, "TL", args, n);
	    set_xmstring (w, XmNlabelString, "Tag");
	    wtip (w, "Roman Numeral sequence designation of moon");
	    XtManageChild (w);

	    for (i = 0; i < M_NMOONS; i++) {
		char *tag = md[i].tag;

		n = 0;
		w = XmCreatePushButton (rc_w, "UTag", args, n); /*PB for sz */
		set_xmstring (w, XmNlabelString, tag ? tag : " ");
		buttonAsButton (w, False);
		XtManageChild (w);
	    }

	/* moon name column */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, title_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, rc_w); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_BEGINNING); n++;
	XtSetArg (args[n], XmNisAligned, True); n++;
	rc_w = XmCreateRowColumn (msform_w, "UName", args, n);
	XtManageChild (rc_w);

	    n = 0;
	    w = XmCreateLabel (rc_w, "NL", args, n);
	    set_xmstring (w, XmNlabelString, "Name");
	    wtip (w, "Common name of body");
	    XtManageChild (w);

	    for (i = 0; i < M_NMOONS; i++) {
		n = 0;
		w = XmCreatePushButton (rc_w, "UName", args, n); /*PB for sz*/
		set_xmstring (w, XmNlabelString, md[i].full);
		buttonAsButton (w, False);
		XtManageChild (w);
	    }

	/* make each of the X/Y/Z/Mag/RA/DEC information columns */

	for (i = 0; i < XtNumber (mc); i++) {
	    MoonColumn *mp = &mc[i];
	    int j;

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, title_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, rc_w); n++;
	    XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
	    XtSetArg (args[n], XmNisAligned, True); n++;
	    rc_w = XmCreateRowColumn (msform_w, "MMIRC", args, n);
	    XtManageChild (rc_w);

	    n = 0;
	    w = XmCreateLabel (rc_w, "ULab", args, n);
	    set_xmstring (w, XmNlabelString, mp->collabel);
	    if (mp->tip)
		wtip (w, mp->tip);
	    XtManageChild (w);

	    for (j = 0; j < M_NMOONS; j++) {
		if (!mp->moononly || j > 0) {
		    char *sel;

		    sel = XtMalloc (strlen(md[j].full) + strlen(mp->suffix)+2);
		    (void) sprintf (sel, "%s.%s", md[j].full, mp->suffix);

		    n = 0;
		    XtSetArg (args[n], XmNuserData, sel); n++;
		    w = XmCreatePushButton(rc_w, "UPB", args, n);
		    XtAddCallback(w, XmNactivateCallback, marsm_activate_cb, 0);
		    marsm_w[j][mp->col] = w;
		} else {
		    n = 0;
		    w = XmCreateLabel(rc_w, "UPB", args, n);
		    set_xmstring (w, XmNlabelString, " ");
		}
		XtManageChild (w);
	    }
	}

	/* make a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (msform_w, "Sep1", args, n);
	XtManageChild (sep_w);

	/* make a date/time stamp */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sdt_w = XmCreateLabel (msform_w, "JDateStamp", args, n);
	wtip (sdt_w, "Date and Time for which data are computed");
	XtManageChild (sdt_w);

	/* make a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sdt_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (msform_w, "Sep2", args, n);
	XtManageChild (sep_w);

	/* make the close button */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
        XtSetArg (args[n], XmNbottomOffset, 5); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 30); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 70); n++;
	w = XmCreatePushButton (msform_w, "Close", args, n);
	XtAddCallback (w, XmNactivateCallback, marsm_sstats_close_cb, 0);
	wtip (w, "Close this dialog");
	XtManageChild (w);
}

/* create mtform_w, the top view dialog */
static void
marsm_create_tvform_w()
{
	Arg args[20];
	int n;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	mtform_w = XmCreateFormDialog (mmshell_w, "MarsMoonsTV", args, n);
	set_something (mtform_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (mtform_w, XmNunmapCallback, mt_unmap_cb, 0);
	XtAddCallback (mtform_w, XmNmapCallback, mt_map_cb, NULL);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Mars Moons top view"); n++;
	XtSetValues (XtParent(mtform_w), args, n);

	/* fill with a drawing area in a frame */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mtframe_w = XmCreateFrame (mtform_w, "MTVF", args, n);
	XtManageChild (mtframe_w);

	    n = 0;
	    mtda_w = XmCreateDrawingArea (mtframe_w, "MarsTop", args, n);
	    XtAddCallback (mtda_w, XmNexposeCallback, mt_da_exp_cb, NULL);
	    XtAddCallback (mtda_w, XmNinputCallback, marsm_da_input_cb, 
							(XtPointer)SO_TOP);
	    XtManageChild (mtda_w);
}

/* go through all the buttons pickable for plotting and set whether they
 * should appear to look like buttons or just flat labels.
 */
static void
marsm_set_buttons (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	int i, j;

	for (i = 0; i < M_NMOONS; i++)
	    for (j = 0; j < CNum; j++)
		if (marsm_w[i][j])
		    buttonAsButton (marsm_w[i][j], whether);
}

/* callback when the Close button is activated on the stats menu */
/* ARGSUSED */
static void
marsm_sstats_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (msform_w);
}

/* callback when the the More Info button is activated */
/* ARGSUSED */
static void
marsm_sstats_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtManageChild (msform_w);
	marsm_set_buttons(marsm_selecting);
}

/* callback when the control menu is becoming visible 
 */
/* ARGSUSED */
static void
marsm_cpdmapping_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtSetSensitive (tel_w, telIsOn());
}

/* callback from any of the option buttons.
 * client points to global flag to set; some don't have any.
 * in any case then just redraw everything.
 */
/* ARGSUSED */
static void
marsm_option_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (client) {
	    int *flagp = (int *)client;
	    *flagp = XmToggleButtonGetState(w);
	    if (flagp == &topview) {
		if (topview) {
		    if (!mtform_w)
			marsm_create_tvform_w();
		    XtManageChild (mtform_w);
		} else
		    XtUnmanageChild (mtform_w);
	    }
	}

	marsm_update (mm_get_now(), 1);
}

/* callback from the scales changing.
 * just redraw everything.
 */
/* ARGSUSED */
static void
marsm_scale_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	marsm_update (mm_get_now(), 1);
}

/* callback from any of the data menu buttons being activated.
 */
/* ARGSUSED */
static void
marsm_activate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (marsm_selecting) {
	    char *name;
	    get_something (w, XmNuserData, (XtArgVal)&name);
	    register_selection (name);
	}
}

/* callback from either expose or resize of the topview.
 */
/* ARGSUSED */
static void
mt_da_exp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static int last_nx, last_ny;
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
        Window win = XtWindow(w);
	Display *dsp = XtDisplay(w);
	unsigned int nx, ny, bw, d;
	Window root;
	int x, y;

	/* filter out a few oddball cases */
	switch (c->reason) {
	case XmCR_EXPOSE: {
	    /* turn off gravity so we get expose events for either shrink or
	     * expand.
	     */
	    static int before;
	    XExposeEvent *e = &c->event->xexpose;

	    if (!before) {
		XSetWindowAttributes swa;
                unsigned long mask = CWBitGravity | CWBackingStore;

		swa.bit_gravity = ForgetGravity;
		swa.backing_store = NotUseful; /* we use a pixmap */
		XChangeWindowAttributes (dsp, win, mask, &swa);
		before = 1;
	    }

	    /* wait for the last in the series */
	    if (e->count != 0)
		return;
	    break;
	    }
	default:
	    printf ("Unexpected mmform_w event. type=%d\n", c->reason);
	    abort();
	}

        XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);
	if (!mt_pm || (int)nx != last_nx || (int)ny != last_ny) {
	    if (mt_pm)
		XFreePixmap (dsp, mt_pm);
	    mt_pm = XCreatePixmap (dsp, win, nx, ny, d);
	    last_nx = nx;
	    last_ny = ny;
	    mt_track_size();
	    marsm_update (mm_get_now(), 1);
	} else
	    XCopyArea (dsp, mt_pm, win, marsm_fgc, 0, 0, nx, ny, 0, 0);
}

/* called whenever the topview scene is mapped. */
/* ARGSUSED */
static void
mt_map_cb (wid, client, call)
Widget wid;
XtPointer client;
XtPointer call;
{
	mt_track_size();
}

/* set the width of the topview DrawingArea the same as the main window's.
 * we also try to center it just above, but it doesn't always work.
 */
static void
mt_track_size()
{
	Dimension w, h;
	Position mfx, mfy, mdx, mdy;
	Position sdy;
	Arg args[20];
	int n;

	/* set widths equal */
	n = 0;
	XtSetArg (args[n], XmNwidth, &w); n++;
	XtGetValues (mda_w, args, n);

	n = 0;
	XtSetArg (args[n], XmNwidth, w); n++;
	XtSetValues (mtda_w, args, n);

	n = 0;
	XtSetArg (args[n], XmNheight, &h); n++;
	XtGetValues (mtda_w, args, n);

	/* set locations -- allow for different stuff on top of drawingareas */
	n = 0;
	XtSetArg (args[n], XmNx, &mfx); n++;
	XtSetArg (args[n], XmNy, &mfy); n++;
	XtGetValues (mmshell_w, args, n);
	n = 0;
	XtSetArg (args[n], XmNx, &mdx); n++;
	XtSetArg (args[n], XmNy, &mdy); n++;
	XtGetValues (mmframe_w, args, n);
	n = 0;
	XtSetArg (args[n], XmNy, &sdy); n++;
	XtGetValues (mtframe_w, args, n);

	n = 0;
	XtSetArg (args[n], XmNx, mfx+mdx); n++;
	XtSetArg (args[n], XmNy, mfy - h - sdy - mdy - 20); n++;
	XtSetValues (mtform_w, args, n);
}

/* callback when topview dialog is unmapped */
/* ARGSUSED */
static void
mt_unmap_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonSetState (topview_w, False, True);
}

/* callback when main shell is popped down */
/* ARGSUSED */
static void
marsm_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (msform_w);
	XtUnmanageChild (mtform_w);

        if (marsm_pm) {
	    XFreePixmap (XtDisplay(mda_w), marsm_pm);
	    marsm_pm = (Pixmap) NULL;
	}

	/* free any field stars */
	if (fstars) {
	    free ((void *)fstars);
	    fstars = NULL;
	    nfstars = 0;
	}

	/* stop movie that might be running */
	mm_movie (0.0);
}

/* callback from the main Close button */
/* ARGSUSED */
static void
marsm_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* let popdown do all the work */
	XtPopdown (mmshell_w);
}

/* callback to add scene to movie loop */
/* ARGSUSED */
static void
marsm_mloop_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	ml_add (marsm_pm, dt_w);
}

/* callback from the Movie button
 */
/* ARGSUSED */
static void
marsm_anim_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* best effect if turn off worrying about the sky background */
	skybkg = 0;
	XmToggleButtonSetState (skybkg_w, False, False);

	mm_movie (MOVIE_STEPSZ);
}

/* callback from either expose or resize of the drawing area.
 */
/* ARGSUSED */
static void
marsm_da_exp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static int last_nx, last_ny;
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
	Window win = XtWindow(w);
	Display *dsp = XtDisplay(w);
	unsigned int nx, ny, bw, d;
	Window root;
	int x, y;

	/* filter out a few oddball cases */
	switch (c->reason) {
	case XmCR_EXPOSE: {
	    /* turn off gravity so we get expose events for either shrink or
	     * expand.
	     */
	    static int before;
	    XExposeEvent *e = &c->event->xexpose;

	    if (!before) {
		XSetWindowAttributes swa;
		unsigned long mask = CWBitGravity | CWBackingStore;

		swa.bit_gravity = ForgetGravity;
		swa.backing_store = NotUseful; /* we use a pixmap */
		XChangeWindowAttributes (dsp, win, mask, &swa);
		before = 1;
	    }

	    /* wait for the last in the series */
	    if (e->count != 0)
		return;
	    break;
	    }
	default:
	    printf ("Unexpected mmform_w event. type=%d\n", c->reason);
	    abort();
	}

	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);
	if (!marsm_pm || (int)nx != last_nx || (int)ny != last_ny) {
	    if (marsm_pm)
		XFreePixmap (dsp, marsm_pm);
	    marsm_pm = XCreatePixmap (dsp, win, nx, ny, d);
	    last_nx = nx;
	    last_ny = ny;
	    if (topview)
		mt_track_size();
	    marsm_update (mm_get_now(), 1);
	} else
	    XCopyArea (dsp, marsm_pm, win, marsm_fgc, 0, 0, nx, ny, 0, 0);
}

/* callback from mouse or keyboard input over either drawing area.
 * client is one of SOIdx.
 */
/* ARGSUSED */
static void
marsm_da_input_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
	ScreenObj *sop;
	SOIdx soidx;
	XEvent *ev;
	int x, y;

	/* get x and y value iff it was from mouse button 3 */
	if (c->reason != XmCR_INPUT)
	    return;
	ev = c->event;
	if (ev->xany.type != ButtonPress || ev->xbutton.button != 3)
	    return;
	x = ev->xbutton.x;
	y = ev->xbutton.y;

	/* find something close on the screen */
	soidx = (SOIdx) client;
	sop = close_screenobj (soidx, x, y);
	if (!sop && soidx == SO_MAIN) {
	    /* try shadows */
	    soidx = SO_SHADOW;
	    sop = close_screenobj (soidx, x, y);
	}
	if (!sop)
	    return;

	/* put info in popup, mmpu_w, creating it first if necessary  */
	if (!mmpu_w)
	    marsm_create_popup();
	marsm_fill_popup (sop, soidx == SO_SHADOW);

	/* put it on screen */
	XmMenuPosition (mmpu_w, (XButtonPressedEvent *)ev);
	XtManageChild (mmpu_w);
}

/* create the (unmanaged for now) popup menu in mmpu_w. */
static void
marsm_create_popup()
{
	static Widget *puw[] = {
	    &mmpu_name_w,
	    &mmpu_ra_w,
	    &mmpu_dec_w,
	    &mmpu_mag_w,
	};
	Widget w;
	Arg args[20];
	int n;
	int i;

	n = 0;
	XtSetArg (args[n], XmNisAligned, True); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
	mmpu_w = XmCreatePopupMenu (mda_w, "UPU", args, n);

	/* stack everything up in labels */
	for (i = 0; i < XtNumber(puw); i++) {
	    n = 0;
	    w = XmCreateLabel (mmpu_w, "SPUL", args, n);
	    XtManageChild (w);
	    *puw[i] = w;
	}
}

/* put up a popup at ev with info about sop */
static void
marsm_fill_popup (ScreenObj *sop, int justname)
{
	char *name;
	double ra, dec, mag;
	char buf[64], buf2[64];

	name = sop->o.o_name;
	(void) sprintf (buf2, "%.*s", MAXNM, name);
	set_xmstring (mmpu_name_w, XmNlabelString, buf2);

	if (justname) {
	    XtUnmanageChild (mmpu_mag_w);
	    XtUnmanageChild (mmpu_ra_w);
	    XtUnmanageChild (mmpu_dec_w);
	} else {
	    mag = get_mag(&sop->o);
	    (void) sprintf (buf2, " Mag: %.3g", mag);
	    set_xmstring (mmpu_mag_w, XmNlabelString, buf2);
	    XtManageChild (mmpu_mag_w);

	    ra = sop->o.s_ra;
	    fs_ra (buf, ra);
	    (void) sprintf (buf2, "  RA: %s", buf);
	    set_xmstring (mmpu_ra_w, XmNlabelString, buf2);
	    XtManageChild (mmpu_ra_w);

	    dec = sop->o.s_dec;
	    fs_prdec (buf, dec);
	    (void) sprintf (buf2, " Dec: %s", buf);
	    set_xmstring (mmpu_dec_w, XmNlabelString, buf2);
	    XtManageChild (mmpu_dec_w);

	}
}

/* callback from the Help all button
 */
/* ARGSUSED */
static void
marsm_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"This is a simple schematic depiction of Mars and its moons.",
};

	hlp_dialog ("MarsMoons", msg, XtNumber(msg));
}

/* callback from a specific Help button.
 * client is a string to use with hlp_dialog().
 */
/* ARGSUSED */
static void
marsm_helpon_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hlp_dialog ((char *)client, NULL, 0);
}

/* callback from the goto control.
 */
/* ARGSUSED */
static void
marsm_goto_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Obj *op = db_basic (MARS);
	telGoto (op);
}

/* add one entry to the given global screenobj array.
 */
static void
add_screenobj (soidx, op, x, y)
SOIdx soidx;
Obj *op;
int x, y;
{
	char *mem = (char *) screenobj[soidx];
	int nmem = nscreenobj[soidx];
	ScreenObj *sop;

	/* grow screenobj by one */
	if (mem)
	    mem = realloc ((char *)mem, (nmem+1)*sizeof(ScreenObj));
	else
	    mem = malloc (sizeof(ScreenObj));
	if (!mem) {
	    xe_msg (0, "Out of memory -- %s will not be pickable", op->o_name);
	    return;
	}
	screenobj[soidx] = (ScreenObj *) mem;

	sop = &screenobj[soidx][nscreenobj[soidx]++];

	/* fill new entry */
	sop->o = *op;
	sop->x = x;
	sop->y = y;
}

/* reclaim any existing screenobj entries from the given collection */
static void
reset_screenobj(soidx)
SOIdx soidx;
{
	if (screenobj[soidx]) {
	    free ((char *)screenobj[soidx]);
	    screenobj[soidx] = NULL;
	}
	nscreenobj[soidx] = 0;
}

/* find the entry in the given screenobj closest to [x,y] within MINR.
 * if found return the ScreenObj *, else NULL.
 */
static ScreenObj *
close_screenobj (soidx, x, y)
SOIdx soidx;
int x, y;
{
	ScreenObj *scop = screenobj[soidx];
	ScreenObj *minsop = NULL;
	int nobj = nscreenobj[soidx];
	int minr = 2*MINR;
	int i;

	for (i = 0; i < nobj; i++) {
	    ScreenObj *sop = &scop[i];
	    int r = abs(x - sop->x) + abs(y - sop->y);
	    if (r < minr) {
		minr = r;
		minsop = sop;
	    }
	}
	if (minr <= MINR)
	    return (minsop);
	else
	    return (NULL);
}

/* callback from Print control.
 */
/* ARGSUSED */
static void
marsm_print_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XPSAsk ("Martian moons", marsm_print);
}

/* proceed to generate a postscript file.
 * call XPSClose() when finished.
 */
static void
marsm_print ()
{
	Display *dsp = XtDisplay (mda_w);
	Now *np = mm_get_now();
	unsigned int w, h, bw, d;
	Window root;
	int x, y;

	if (!marsm_ison()) {
	    xe_msg (1, "Mars moons must be open to save a file.");
	    XPSClose();
	    return;
	}

	watch_cursor(1);

	/* get size of the rendering area */
	XGetGeometry(dsp, marsm_pm, &root, &x, &y, &w, &h, &bw, &d);

	/* draw in an area 5.5w x 5.5h centered 1in down from top */
	if (w >= h)
	    XPSXBegin (marsm_pm, 0, 0, w, h, 1*72, 10*72, (int)(5.5*72));
	else {
	    int pw = (int)(72*5.5*w/h+.5);	/* width on paper when 5.5 hi */
	    XPSXBegin (marsm_pm, 0, 0, w, h, (int)((8.5*72-pw)/2), 10*72, pw);
	}

	/* redraw */
	marsm_update (np, 1);

	/* no more X captures */
	XPSXEnd();

	/* add some extra info */
	marsm_ps_annotate (np);

	/* finished */
	XPSClose();

	watch_cursor(0);
}

static void
marsm_ps_annotate (np)
Now *np;
{
	int ctrx = (int)(8.5*72/2);
	Obj *eop = db_basic (SUN);
	Obj *mop = db_basic (MARS);
	MoonData md[M_NMOONS];
	double msize;
	double tzmjd;
	char *bp, buf[256], buf2[32], dir[256];
	int n, x, y, i;

	/* compute all info */
	sprintf (buf, "%s/auxil", getShareDir());
	bp = expand_home (buf);
        marsm_data (mjd, dir, eop, mop, &msize, &pole_ra, &pole_dec, md);

	/* border */
	y = (int)AROWY(PRTR+.5);
	sprintf (buf, "newpath 72 %d moveto 540 %d lineto stroke\n", y, y);
	XPSDirect (buf);

	/* title */
	y = AROWY(PRTR-2);
	if (pref_get(PREF_ZONE) == PREF_UTCTZ) {
	    tzmjd = mjd;
	    bp = "UTC";
	} else {
	    tzmjd = mjd-tz/24;
	    bp = tznm;
	}
	fs_time (buf, mjd_hr(tzmjd));
	fs_date (buf2, pref_get(PREF_DATE_FORMAT), mjd_day(tzmjd));
	sprintf (dir,"(XEphem Mars View at %s %s %s, JD %13.5f) %d %d cstr\n",
					buf, buf2, bp, mjd+MJD0, ctrx, y);
	XPSDirect (dir);

	/* location if topocentric */
	if (pref_get(PREF_EQUATORIAL) == PREF_TOPO) {
	    char s, *site;

	    bp = buf;
	    bp += sprintf (bp, "From ");

	    site = mm_getsite();
	    if (site)
		bp += sprintf (bp, "%s at ", XPSCleanStr(site,strlen(site)));

	    /* N.B. without the s= business the sign is printed wrong!! */

	    fs_sexa (buf2, raddeg(fabs(lat)), 3, 3600);
	    bp += sprintf (bp, "Latitude %s %c ", buf2, s=(lat<0 ? 'S' : 'N'));

	    fs_sexa (buf2, raddeg(fabs(lng)), 4, 3600);
	    bp += sprintf (bp, "Longitude %s %c", buf2, s=(lng<0 ? 'W' : 'E'));

	    sprintf (dir, "(%s) %d %d cstr\n", buf, ctrx, AROWY(PRTR-4));
	    XPSDirect (dir);
	}

	/* table heading */
	bp = buf;
	bp += sprintf (bp, "%s ", pref_get(PREF_EQUATORIAL) == PREF_TOPO ?
						"Topocentric" : "Geocentric");
	if (epoch == EOD)
	    bp += sprintf (bp, "Apparent EOD");
	else {
	    double tmp;
	    mjd_year (epoch, &tmp);
	    bp += sprintf (bp, "Astrometric %.2f", tmp);
	}
	y = AROWY(PRTR-5);
	sprintf (dir, "(%s Moon Positions, XYZ in Martian radii) %d %d cstr\n",
								buf, ctrx,y);
	XPSDirect (dir);

	/* stats for each row */
	for (i = 0; i < M_NMOONS; i++) {
	    y = AROWY(PRTR-7-i);
	    x = 65;

	    if (i == 0) {
		sprintf (dir, "(Tag) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    if (md[i].tag) {
		n = sprintf (buf, "%-4s", md[i].tag);
		sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
		XPSDirect (dir);
	    } else
		n = 4;
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "(Name) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    n = sprintf (buf, "%-9s", md[i].full);
	    sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
	    XPSDirect (dir);
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "(E) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    n = sprintf (buf, "%1.0f ", (double)md[i].evis);
	    if (i > 0) {
		sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
		XPSDirect (dir);
	    }
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "(S) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    n = sprintf (buf, "%1.0f ", (double)md[i].svis);
	    if (i > 0) {
		sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
		XPSDirect (dir);
	    }
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "(P) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    n = sprintf (buf, "%1.0f ", (double)md[i].pshad);
	    if (i > 0) {
		sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
		XPSDirect (dir);
	    }
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "(T) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    n = sprintf (buf, "%1.0f ", (double)md[i].trans);
	    if (i > 0) {
		sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
		XPSDirect (dir);
	    }
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "( X +E) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    n = sprintf (buf, "%7.3f ", md[i].x);
	    if (i > 0) {
		sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
		XPSDirect (dir);
	    }
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "( Y +S) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    n = sprintf (buf, "%7.3f ", md[i].y);
	    if (i > 0) {
		sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
		XPSDirect (dir);
	    }
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "( Z +front) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    n = sprintf (buf, "%7.3f ", md[i].z);
	    if (i > 0) {
		sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
		XPSDirect (dir);
	    }
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "(   RA) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    fs_sexa (buf, radhr(md[i].ra), 2, 360000);
	    n = 11;
	    sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
	    XPSDirect (dir);
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "(   Dec) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    fs_sexa (buf, raddeg(md[i].dec), 3, 36000);
	    n = 11;
	    sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
	    XPSDirect (dir);
	    x += n*PRCW;

	    if (i == 0) {
		sprintf (dir, "(Mag) %d %d lstr\n", x, AROWY(PRTR-6-i));
		XPSDirect (dir);
	    }
	    sprintf (buf, "%4.1f ", md[i].mag);
	    sprintf (dir, "(%s) %d %d lstr\n", buf, x, y);
	    XPSDirect (dir);
	}
}

/* given the loc of the moons, draw a nifty little picture.
 * also save resulting screen locs of everything in the screenobj array.
 * scale of the locations is in terms of Mars radii == 1.
 * unflipped view is S up, E right.
 * planet itself is in md[0], moons in md[1..NM-1].
 *     +md[].x: East, mars radii
 *     +md[].y: South, mars radii
 *     +md[].z: in front, mars radii
 */
static void
marsm_draw_map (w, mop, msize, md)
Widget w;
Obj *mop;
double msize;
MoonData md[M_NMOONS];
{
	Display *dsp = XtDisplay(w);
	Window win;
	Window root;
	double scale;
	int sv;
	int fmag;
	char c;
	int x, y;
	unsigned int nx, ny, bw, d;
	int ns = flip_tb ? -1 : 1;
	int ew = flip_lr ? -1 : 1;
	int scale1;
	int i;

	/* first the main graphic */
	win = XtWindow (mda_w);

	if (!marsm_fgc)
	    make_gcs(w);

	XmScaleGetValue (limmag_w, &fmag);
	XmScaleGetValue (scale_w, &sv);
	scale = pow(MAXSCALE, sv/100.0);

	/* get size and erase */
	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);
	XFillRectangle (dsp, marsm_pm, marsm_bgc, 0, 0, nx, ny);
	scale1 = (int)floor(MAPSCALE(1)+0.5);

	/* prepare for a new list of things on the screen */
	reset_screenobj(SO_MAIN);
	reset_screenobj(SO_SHADOW);

	/* draw Mars in center with unit radius */
	XPSFillArc(dsp, marsm_pm, marsm_fgc, nx/2-scale1, ny/2-scale1,
					    2*scale1-1, 2*scale1-1, 0, 360*64);
	add_screenobj (SO_MAIN, mop, nx/2, ny/2);

	/* draw background objects, if desired
	 * go out plenty wide to pick up moon during occultations.
	 */
	if (skybkg)
	    sky_background(marsm_pm, nx, ny, fmag, md[0].ra, md[0].dec,
			msize/MAPSCALE(2), degrad(.5), flip_tb, flip_lr);

	/* draw labels */
	c = flip_lr ? 'W' : 'E';
	XPSDrawString(dsp, marsm_pm, marsm_fgc, nx-marsm_cw-1, ny/2-2, &c, 1);
	c = flip_tb ? 'N' : 'S';
	XPSDrawString(dsp, marsm_pm, marsm_fgc, (nx-marsm_cw)/2-1,
						    marsm_fs->ascent, &c, 1);

	/* draw 1' calibration line */
	if (tags)
	    marsm_calibline (dsp, marsm_pm, marsm_fgc, nx/2, 5*ny/6, "1'",
				marsm_cw*2, marsm_ch+4,
				(int)MAPSCALE(degrad(1.0/60.0)/(msize/2)));

	/* draw each moon and its shadow
	 */
	for (i = 1; i < M_NMOONS; i++) {
	    double mx = md[i].x;
	    double my = md[i].y;
	    double mag = md[i].mag;
	    int diam;
	    Obj o;

	    /* skip if behind or in shadow */
	    if (!md[i].evis || !md[i].svis)
		continue;

	    diam = magdiam (fmag, 1, msize/(2*scale1), mag, 0.0);
	    if (brmoons)
		diam += 3;
	    if (diam <= 0)
		continue;	/* too faint */

	    /* moon */
	    x = XCORD(mx);
	    y = YCORD(my);
	    if (diam == 1)
		XPSDrawPoint(dsp, marsm_pm, marsm_xgc, x, y);
	    else
		XPSDrawStar (dsp, marsm_pm, marsm_xgc, x-diam/2, y-diam/2,
									diam);

	    /* add object to list of screen objects drawn */
	    memset (&o, 0, sizeof(0));
	    (void) strcpy (o.o_name, md[i].full);
	    set_smag (&o, mag);
	    o.s_ra = (float)md[i].ra;
	    o.s_dec = (float)md[i].dec;
	    add_screenobj (SO_MAIN, &o, x, y);

	    /* shadow, if in front */
	    if (md[i].pshad) {
		int scx = XCORD(md[i].sx);
		int scy = YCORD(md[i].sy);

		XPSDrawStar (dsp, marsm_pm, marsm_bgc, scx-diam/2, scy-diam/2,
									diam);
		sprintf (o.o_name, "%s shadow",  md[i].full);
		add_screenobj (SO_SHADOW, &o, scx, scy);
	    }

	    if (tags && md[i].tag)
		XDrawString(dsp, marsm_pm, marsm_xgc, x-marsm_cw/2,
			    y+2*marsm_ch, md[i].tag, strlen(md[i].tag));
	}

	/* user annotation */
	ano_draw (mda_w, marsm_pm, marsm_ano, 0);

	XCopyArea (dsp, marsm_pm, win, marsm_fgc, 0, 0, nx, ny, 0, 0);

	/* then the top view, if desired */

	if (!topview || !mt_pm)	/* let expose make new pixmap */
	    return;

	/* get size and erase */
	win = XtWindow (mtda_w);
	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);
	XFillRectangle (dsp, mt_pm, marsm_bgc, 0, 0, nx, ny);

	/* draw Mars in center with unit radius */
	i = (int)(ew*64*raddeg(asin(sin(degrad(mop->s_elong))/mop->s_sdist)));
	XPSFillArc(dsp, mt_pm, marsm_fgc, nx/2-scale1, ny/2-scale1, 2*scale1-1,
						    2*scale1-1, -i, -180*64);
	reset_screenobj(SO_TOP);
	add_screenobj (SO_TOP, mop, nx/2, ny/2);

	/* draw each moon
	 */
	for (i = 1; i < M_NMOONS; i++) {
	    double mx = md[i].x;
	    double mz = md[i].z;
	    double mag = md[i].mag;
	    int diam;
	    Obj o;

	    if (!md[i].svis)
		continue;

	    diam = magdiam (fmag, 1, msize/(2*scale1), mag, 0.0);
	    if (brmoons)
		diam += 3;
	    if (diam <= 0)
		continue;	/* too faint */

	    x = XCORD(mx);
	    y = ZCORD(mz);
	    if (diam == 1)
		XDrawPoint (dsp, mt_pm, marsm_xgc, x, y);
	    else
		XFillArc (dsp, mt_pm, marsm_xgc, x-diam/2, y-diam/2, diam,
							    diam, 0, 360*64);

	    /* add object to list of screen objects drawn */
	    memset (&o, 0, sizeof(0));
	    (void) strcpy (o.o_name, md[i].full);
	    set_smag (&o, mag);
	    o.s_ra = (float)md[i].ra;
	    o.s_dec = (float)md[i].dec;
	    add_screenobj (SO_TOP, &o, x, y);

	    if (tags && md[i].tag)
		XDrawString(dsp, mt_pm, marsm_xgc, x-marsm_cw/2, y+2*marsm_ch,
					md[i].tag, strlen(md[i].tag));
	}

	/* draw label towards earth */
	if (tags) {
	    XPoint xp[5];

	    XDrawString(dsp, mt_pm, marsm_fgc, nx/2, ny-marsm_ch, "Earth", 5);
	    xp[0].x = nx/2 - 15; xp[0].y = ny - 2*marsm_ch;
	    xp[1].x = 0;         xp[1].y = 3*marsm_ch/2;
	    xp[2].x = -5;        xp[2].y = -3*marsm_ch/4;
	    xp[3].x = 10;        xp[3].y = 0;
	    xp[4].x = -5;        xp[4].y = 3*marsm_ch/4;
	    XDrawLines (dsp, mt_pm, marsm_fgc, xp, 5, CoordModePrevious);
	}

	/* user annotation */
	ano_draw (mtda_w, mt_pm, mtm_ano, 0);

	XCopyArea (dsp, mt_pm, win, marsm_fgc, 0, 0, nx, ny, 0, 0);
}

/* convert mars X/Y to/from X windows coords depending on w2x.
 * return whether visible.
 */
static int
marsm_ano (double *mX, double *mY, int *xp, int *yp, int w2x, int arg)
{
	Display *dsp = XtDisplay (mda_w);
	Window win = XtWindow (mda_w);
	int ns = flip_tb ? -1 : 1;
	int ew = flip_lr ? -1 : 1;
	unsigned int nx, ny, bw, d;
	Window root;
	double scale;
	int x, y;
	int sv;

	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);
	XmScaleGetValue (scale_w, &sv);
	scale = pow(MAXSCALE, sv/100.0);

	if (w2x) {
	    *xp = XCORD(*mX);
	    *yp = YCORD(*mY);
	} else {
	    *mX = 2*NORM*(*xp-(int)nx/2.0)/(ew*(int)nx*scale);
	    *mY = 2*NORM*((int)ny/2.0-*yp)/(ns*(int)ny*scale);
	}

	return (*xp>=0 && *xp<nx && *yp>=0 && *yp<ny);
}

/* convert mars X/Z to/from X windows coords depending on w2x.
 * return whether visible.
 */
static int
mtm_ano (double *mX, double *mZ, int *xp, int *yp, int w2x, int arg)
{
	Display *dsp = XtDisplay (mtda_w);
	Window win = XtWindow (mtda_w);
	int ew = flip_lr ? -1 : 1;
	unsigned int nx, ny, bw, d;
	Window root;
	double scale;
	int x, y;
	int sv;

	XGetGeometry(dsp, win, &root, &x, &y, &nx, &ny, &bw, &d);
	XmScaleGetValue (scale_w, &sv);
	scale = pow(MAXSCALE, sv/100.0);

	if (w2x) {
	    *xp = XCORD(*mX);
	    *yp = ZCORD(*mZ);
	} else {
	    *mX = 2*NORM*(*xp-(int)nx/2.0)/(ew*(int)nx*scale);
	    *mZ = 2*NORM*(*yp-(int)ny/2.0)/((int)ny*scale);
	}

	return (*xp>=0 && *xp<nx && *yp>=0 && *yp<ny);
}

static void
make_gcs (w)
Widget w;
{
	Display *dsp = XtDisplay(w);
	Window win = XtWindow(w);
	XGCValues gcv;
	unsigned int gcm;
	Pixel fg, bg;

	marsm_fs = getXResFont ("moonsFont");
	(void) get_color_resource (w, "marsColor", &fg);
	gcm = GCForeground | GCFont;
	gcv.foreground = fg;
	gcv.font = marsm_fs->fid;
	marsm_fgc = XCreateGC (dsp, win, gcm, &gcv);
	marsm_cw = marsm_fs->max_bounds.width;
	marsm_ch = marsm_fs->max_bounds.ascent + marsm_fs->max_bounds.descent;

	gcm = GCForeground;
	(void) get_color_resource (w, "MarsBackground", &bg);
	gcv.foreground = bg;
	marsm_bgc = XCreateGC (dsp, win, gcm, &gcv);

	gcm = GCForeground | GCFunction | GCFont;
	gcv.foreground = fg ^ bg;
	gcv.function = GXxor;
	gcv.font = marsm_fs->fid;
	marsm_xgc = XCreateGC (dsp, win, gcm, &gcv);
}

/* draw a calibration line l pixels long centered at [xc,yc] marked with tag
 * in a bounding box tw x th.
 */
static void
marsm_calibline (dsp, win, gc, xc, yc, tag, tw, th, l)
Display *dsp;
Drawable win;
GC gc;
int xc, yc;
char *tag;
int tw, th;
int l;
{
	int lx = xc - l/2;
	int rx = lx + l;
	int i;

	XDrawLine (dsp, win, gc, lx, yc, rx, yc);

	for (i = 0; i <= 6; i++) {
	    int x = lx+i*l/6;
	    int dy = 3 + ((i%6)?0:3);

	    XDrawLine (dsp, win, gc, x, yc-dy, x, yc+dy);
	}

	XDrawString (dsp, win, gc, xc-tw/2, yc+th, tag, strlen(tag));
}

/* draw all database objects in a small sky patch about the given center.
 * get field stars too if enabled and we have moved enough.
 * save objects and screen locs in the global screenobj array for picking.
 * this is used to draw the backgrounds for the planet closeups.
 * Based on work by: Dan Bruton <WDB3926@acs.tamu.edu>
 */
static void
sky_background (win, w, h, fmag, ra0,dec0,scale,rad,fliptb,fliplr)
Drawable win;		/* window to draw on */
unsigned w, h;		/* window size */
int fmag;		/* faintest magnitude to display */
double ra0, dec0;	/* center of patch, rads */
double scale;		/* rads per pixel */
double rad;		/* maximum radius to draw away from ra0/dec0, rads */
int fliptb, fliplr;	/* flip direction; default is S up E right */
{
	static int before;
	double cdec0 = cos(dec0);
	DBScan dbs;
	Obj *op;
	double hfov = w * scale;
	double vfov = h * scale;

	if (!before && pref_get(PREF_EQUATORIAL) == PREF_GEO) {
	    xe_msg (1, "Equatorial preference should probably be set to Topocentric");
	    before = 1;
	}

        /* update field star list */
	marsm_loadfs (ra0, dec0);

	/* scan the database and draw whatever is near */
	for (db_scaninit(&dbs, ALLM, fstars, nfstars);
					    (op = db_scan (&dbs)) != NULL; ) {
	    double dra, ddec;
	    GC gc;
	    int dx, dy, x, y;
	    int diam;

	    if (is_planet (op, MARS)) {
		/* we draw it elsewhere */
		continue;
	    }

	    db_update (op);

	    /* find size, in pixels. */
	    diam = magdiam (fmag, 1, scale, get_mag(op),
						degrad(op->s_size/3600.0));
	    if (diam <= 0)
		continue;

	    /* find x/y location if it's in the general area */
	    dra = op->s_ra - ra0;	/* + E */
	    ddec = dec0 - op->s_dec;	/* + S */
	    if (fabs(ddec) > rad || delra(dra)*cdec0 > rad)
		continue;
	    dx = (int)(dra/scale);
	    dy = (int)(ddec/scale);
	    x = fliplr ? (int)w/2-dx : (int)w/2+dx;
	    y = fliptb ? (int)h/2+dy : (int)h/2-dy;

	    /* pick a gc */
	    obj_pickgc(op, toplevel_w, &gc);

	    /* draw 'er */
	    sv_draw_obj_x (XtD, win, gc, op, x, y, diam, 0, fliptb, !fliplr, 0, 1, dec0, ra0, vfov, hfov, w, h);

	    /* save 'er */
	    add_screenobj (SO_MAIN, op, x, y);
	}

	sv_draw_obj (XtD, win, (GC)0, NULL, 0, 0, 0, 0);	/* flush */
}

/* load field stars around the given location, unless current set is
 * already close enough.
 */
static void
marsm_loadfs (ra, dec)
double ra, dec;
{
	Now *np = mm_get_now();

	if (fstars && fabs(dec-fsdec)<FSMOVE && cos(dec)*delra(ra-fsra)<FSMOVE)
	    return;

	if (fstars) {
	    free ((void *)fstars);
	    fstars = NULL;
	    nfstars = 0;
	}

        nfstars = fs_fetch (np, ra, dec, FSFOV, FSMAG, &fstars);

	if (nfstars > 0) {
            xe_msg (0, "Mars Moon View added %d field stars",nfstars);
	    fsdec = dec;
	    fsra = ra;
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: marsmmenu.c,v $ $Date: 2012/07/07 18:04:42 $ $Revision: 1.45 $ $Name:  $"};

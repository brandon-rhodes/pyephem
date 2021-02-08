/* code to manage the stuff on the moon display.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Scale.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledW.h>

#include "xephem.h"
#include "lilxml.h"

typedef struct {
    double curlt, curlg;	/* lat/long under cursor */
    double cursunalt;		/* sun alt under cursor now */
    double curnsrt;		/* next sunrise under cursor, tz mjd */
    double curnsst;		/* next sunset under cursor, tz mjd */
    double srlng;		/* longitude of the rising sun */
    double sslat;		/* lat of subsolar point */
    double llat, llong;		/* current libration in lat and long */
    double limb;		/* limb angle, rads ccw from up */
    double tilt;		/* angle of limb tilt, degrees towards earth */
    double age;			/* days since new moon */
} MoonInfo;

typedef struct {
    Obj *op;			/* points into real db for more info */
    int x, y;			/* X location on mda_w */
} SkyObj;

static SkyObj *skyobjs;		/* malloced list of objects in sky background */
static int nskyobjs;		/* number of objects in skyobjs[] list */
static char mooncategory[] = "Moon";	/* Save category */

#define	MAXR	10		/* max gap when picking sky objects, pixels */	

static void resetSkyObj (void);
static void addSkyObj (Obj *op, int x, int y);
static FILE *mlo_open (char *base);
static int mlo_installed(void);
static Obj *closeSkyObj (int x, int y);
static void fill_skypopup (Obj *op);
static void m_create_skypopup (void);

static void m_create_shell (void);
static void mlo_create_shell (void);
static void m_create_msform (void);
static void m_create_esform (void);
static void m_set_buttons (int whether);
static void m_eshine_cb (Widget w, XtPointer client, XtPointer call);
static void m_mloop_cb (Widget w, XtPointer client, XtPointer call);
static void m_eshineclose_cb (Widget w, XtPointer client, XtPointer call);
static void m_eshineup_cb (Widget w, XtPointer client, XtPointer call);
static void m_anim_cb (Widget w, XtPointer client, XtPointer call);
static void m_elab_cb (Widget w, XtPointer client, XtPointer call);
static void m_select_cb (Widget w, XtPointer client, XtPointer call);
static void m_mstats_cb (Widget w, XtPointer client, XtPointer call);
static void m_mstats_close_cb (Widget w, XtPointer client, XtPointer call);
static void m_activate_cb (Widget w, XtPointer client, XtPointer call);
static void m_print_cb (Widget w, XtPointer client, XtPointer call);
static void m_print (void);
static void m_ps_annotate (void);
static void m_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void mlo_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void mlo_select_cb (Widget w, XtPointer client, XtPointer call);
static void mlo_ann_cb (Widget w, XtPointer client, XtPointer call);
static void m_close_cb (Widget w, XtPointer client, XtPointer call);
static void mlo_close_cb (Widget w, XtPointer client, XtPointer call);
static void m_init_gcs (void);
static void m_init_gcs (void);
static void m_help_cb (Widget w, XtPointer client, XtPointer call);
static void m_helpon_cb (Widget w, XtPointer client, XtPointer call);
static void m_pl_cb (Widget w, XtPointer client, XtPointer call);
static void m_copy_cb (Widget w, XtPointer client, XtPointer call);
static void m_showlo_cb (Widget w, XtPointer client, XtPointer call);
static void m_option_cb (Widget w, XtPointer client, XtPointer call);
static void m_shrink_cb (Widget w, XtPointer client, XtPointer call);
static void m_sizewidgets (void);
static int xy2ll (int x, int y, double *ltp, double *lgp);
static int ll2xy (double lt, double lg, int *xp, int *yp);
static int overMoon (int x, int y);
static void m_pointer_eh (Widget w, XtPointer client, XEvent *ev,
    Boolean *continue_to_dispatch);
static void m_exp_cb (Widget w, XtPointer client, XtPointer call);
static void m_redraw (void);
static void m_refresh (XExposeEvent *ep);
static void m_getsize (Drawable d, unsigned *wp, unsigned *hp,unsigned *dp);
static void m_info (Now *np);
static void m_pinfo (void);
static void m_draw (void);
static int mxim_create (void);
static void image_setup (void);
static int m_esedges (int y, double sinsrl, int right, int *lxp, int *rxp);
static void m_clip_mask_setup (void);
static void m_free_clip_mask (void);
static int m_shrink (int f);
static void mBWdither (void);
static void m_resize (unsigned char *in, int nr, int nc, int f,
    unsigned char *out);
static void m_umbra (void);
static void fliptb (void);
static void fliplr (void);
static void m_fillSL(void);
static void mi_draw (void);
static int m_ano (double *latp, double *lngp, int *xp, int *yp, int w2x,
    int arg);
static void m_orientation (void);
static void m_labels (void);
static void m_sky (void);
static void m_grid (void);
static void m_sub (void);
static void m_mark_libr (void);
static void m_reportloc (Display *dsp, Window win, int x, int y);

static void makeGlassImage (Display *dsp);
static void makeGlassGC (Display *dsp, Window win);
static void doGlass (Display *dsp, Window win, int b1p, int m1,
    int b1r, int wx, int wy);
static void fillGlass (int wx, int wy);
static void m_readdb (void);
static void m_freedb (void);
static void m_popup (XEvent *ep);

/* stuff for the moon image.
 * obviously, size and centering metrics are tuned to a particular image.
 */
#define	XEVERS		3	/* required version in header */
#define MOONRAD		311	/* radius of moon image, pixels */
#define TOPMARG		25	/* top margin of moon image, pixels */
#define LEFTMARG	25	/* left margin of moon image, pixels */
#define	MNROWS		670	/* number of rows in moon image */
#define	MNCOLS		670	/* number of columns in moon image */
#define	BORD		150	/* extra drawing area border for labels/stars */
#define	CLNG	degrad(-4.6)	/* longitude of image center, rads east */
#define	CLAT	degrad(1.0)	/* latitude of image center, rads north */
#define MOVIE_STEPSZ    (2.)	/* movie step size without stars, hours */
#define MOVIE_SSTEPSZ   (1./60)	/* movie step size with stars, hours */

static unsigned char *mimage;	/* malloced array of full-size moon image */
static Pixel *mgray;		/* gray-scale ramp for drawing image */
static int nmgray;		/* number of pixels usable in mgray[] */
static Pixel mbg;		/* background color for image */
static int mdepth;		/* depth of image, in bits */
static int mbpp;		/* bits per pixel in image: 1, 8, 16 or 32 */
static int topmarg, leftmarg;	/* current TOPMARG/LEFTMARG after resize */
static int moonrad;		/* current MOONRAD after resize, pixels */
static int mnrows, mncols;	/* current size of image after resize, pixels */
static XImage *m_xim;		/* XImage of moon now at current size */
static int mshrink;		/* current shrink factor: 1 .. MAXSHRINK */
static MoonInfo minfo;		/* stats about the current situation */
static Pixmap m_clip_mask;	/* clipping mask used for creating m_pm from m_xim */

#define	MAXSHRINK	6	/* max shrink factor */
#define	SYNP		29.53	/* average synodic period, days */
#define	SIDP		27.32	/* average sidereal period, days */
#define	MINKM		125	/* smallest feature we ever label, km */
#define	ESHINEF		10	/* earthshine factor range: 0 .. this */
#define	LMFRAC		40	/* libr marker is 1/x this of total size */
#define	LGAP		20	/* gap between NSEW labels and image edge */
#define	FMAG		12	/* faintest mag of sky background object */

/* main's widgets */
static Widget mshell_w;		/* main moon shell */
static Widget msform_w;		/* statistics form dialog */
static Widget msw_w;		/* main scrolled window */
static Widget msl_w;		/* main scrolled list */
static Widget mda_w;		/* image view DrawingArea */
static Pixmap m_pm;		/* image view staging pixmap */
static Widget dt_w;		/* main date/time stamp widget */
static Widget sdt_w;		/* statistics date/time stamp widget */

/* "More info" widgets */
static Widget srlng_w;		/* sunrise longitude PB widget */
static Widget sslat_w;		/* subsolar latitude PB widget */
static Widget llat_w;		/* lib in lat PB widget */
static Widget llong_w;		/* lib in long PB widget */
static Widget limb_w;		/* limb angle PB widget */
static Widget lib_w;		/* total lib PB widget */
static Widget age_w;		/* days since new moon PB widget */
static Widget sunalt_w;		/* sun altitude under cursor */
static Widget infot_w;		/* info title label */
static Widget lat_w, lng_w;	/* lat/long under cursor */
static Widget nsrt_w, nsrd_w;	/* next sunrise time/date under cursor */
static Widget nsst_w, nssd_w;	/* next sunset time/date under cursor */

/* lo shell widgets */
static Widget mlo_shell_w;	/* main shell */
static Widget mlo_sw_w;		/* scrolled window for pixmap image */
static Widget mlo_pml_w;	/* L for pixmap image */
static Widget mlo_sl_w;		/* scrolled list to show choices */
static Widget mlo_ann_w;	/* TB whether to show annotated version */

/* earthshine widgets */
static Widget esform_w;		/* main dialog */
static Widget eshine_w;		/* earthshine factor scale */

/* lunar surface popup's widgets */
static Widget pu_w;		/* main popup */
static Widget pu_name_w;	/* popup name label */
static Widget pu_type_w;	/* popup type label */
static Widget pu_lat_w;		/* popup latitude label */
static Widget pu_lng_w;		/* popup longitude label */
static Widget pu_alt_w;		/* popup sun altitude label */
static Widget pu_pl_w;		/* popup label TB */
static Widget pu_copy_w;	/* popup copy PB */
static Widget pu_showlo_w;	/* popup show lunar orbiter image PB */

/* sky background object's widgets */
static Widget skypu_w;		/* main popup */
static Widget skypu_name_w;	/* popup name label */
static Widget skypu_mag_w;	/* popup mag label */
static Obj *skypu_op;		/* current object being referred to */

/* field star support */
static ObjF *fstars;            /* malloced list of field stars, or NULL */
static int nfstars;             /* number of entries in fstars[] */
static double fsdec, fsra;      /* location when field stars were loaded */
#define FSFOV   degrad(3.0)     /* size of FOV to fetch, rads */
#define FSMAG   ((double)FMAG)	/* limiting mag for fetch */
#define FSMOVE  degrad(.2)      /* reload when moon has moved this far, rads */
static void m_loadfs (Now *np, double ra, double dec);

static GC m_fgc, m_bgc, m_agc;	/* various GCs */
static XFontStruct *m_fsp;	/* label font info */

static int m_selecting;		/* set while our fields are being selected */

static XImage *glass_xim;	/* glass XImage -- 0 means new or can't */
static GC glassGC;		/* GC for glass border */

#define	GLASSSZ		50	/* mag glass width and heigth, pixels */
#define	GLASSMAG	2	/* mag glass factor (may be any integer > 0) */

/* options list */
typedef enum {
    SPACECRAFT_OPT, LABELS_OPT, GRID_OPT, FLIPLR_OPT, FLIPTB_OPT, SKY_OPT,
    UMBRA_OPT, N_OPT
} Option;
static int option[N_OPT];
static Widget option_w[N_OPT];

/* return the effective x or y values, allowing for flip options.
 * N.B. this is with respect to the image, not the drawing area.
 */
#define	FX(x)	(option[FLIPLR_OPT] ? mncols-1-(x) : (x))
#define	FY(y)	(option[FLIPTB_OPT] ? mnrows-1-(y) : (y))

/* lunar database */
typedef struct {
    char *name;			/* malloced name */
    char *type;			/* malloced type (may be shared) */
    double lt, lg;		/* lunar coords, lat N and long E, rads */
    int x, y;			/* image location, X pixels coords */
    char pl;			/* whether want label */
    char **lofiles;		/* malloced list of lunar orbiter file names */
    int nlofiles;		/* number of lofiles */
} MoonDB;
static MoonDB *moondb;		/* malloced array from file */
static int nmoondb;		/* entries in moondb[] */
static int m_wantlabel (MoonDB *mp);
static void fill_popup (MoonDB *mp, int x, int y);
static void m_create_popup (void);
static void mlo_load (MoonDB *ep);
static void mlo_load_image (char *fn);
static MoonDB *closeMoonDB (int x, int y);

/* called when the moon view is activated via the main menu pulldown.
 * if first time, build everything, else just up we go.
 * allow for retrying to read the image file each time until find it.
 */
void
m_manage ()
{
	if (!mshell_w) {
	    /* menu one-time-only work */

	    /* build dialogs */
	    m_create_shell();
	    m_create_msform();
	    m_create_esform();
	    mlo_create_shell();

	    /* establish depth, colors and bits per pixel */
	    get_something (mda_w, XmNdepth, (XtArgVal)&mdepth);
	    m_init_gcs();
	    mbpp = (mdepth == 1 || nmgray == 2) ? 1 :
				    (mdepth>=17 ? 32 : (mdepth >= 9 ? 16 : 8));

	    /* read moon database */
	    m_readdb();
	    m_fillSL();
	}

	if (!mimage) {
	    /* read image and display at initial size */
	    if (m_shrink(mshrink) < 0)
		return;
	}
	
	XtPopup (mshell_w, XtGrabNone);
	set_something (mshell_w, XmNiconic, (XtArgVal)False);
	centerScrollBars (msw_w);

	/* register we are now up */
	setXRes (m_viewupres(), "1");
}

int
m_ison()
{
	return (isUp(mshell_w));
}

/* commanded from main to update with a new set of circumstances */
void
m_update (np, how_much)
Now *np;
int how_much;
{
	if (!mshell_w)
	    return;
	if (!isUp(mshell_w) && !any_ison() && !how_much)
	    return;

	watch_cursor (1);
	m_redraw();
	watch_cursor (0);
}

/* called when basic resources change.
 * rebuild and redraw.
 */
void
m_newres()
{
	if (!mshell_w)
	    return;
	m_init_gcs();
	m_update (mm_get_now(), 1);
}

/* called when the database has changed.
 * if we are drawing background, we'd best redraw everything.
 */
void
m_newdb (appended)
int appended;
{
	if (option[SKY_OPT] && isUp(mshell_w)) {
	    if (!appended)
		resetSkyObj();	/* still ok if just appended */
	    m_redraw();
	}
}

/* called by other menus as they want to hear from our buttons or not.
 * the "on"s and "off"s stack - only really redo the buttons if it's the
 * first on or the last off.
 */
void
m_selection_mode (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	if (whether)
	    m_selecting++;
	else if (m_selecting > 0)
	    --m_selecting;

	if (mshell_w) {
	    if ((whether && m_selecting == 1)     /* first one to want on */
		|| (!whether && m_selecting == 0) /* last one to want off */)
		m_set_buttons (whether);
	}
}

/* called to put up or remove the watch cursor.  */
void
m_cursor (c)
Cursor c;
{
	Window win;

	if (mshell_w && (win = XtWindow(mshell_w)) != 0) {
	    Display *dsp = XtDisplay(mshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}

	if (mlo_shell_w && (win = XtWindow(mlo_shell_w)) != 0) {
	    Display *dsp = XtDisplay(mlo_shell_w);
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
}

/* return the name of the resource containing whether this view is up */
char *
m_viewupres()
{
	return ("MoonViewUp");
}

static void
m_create_shell()
{
	typedef struct {
	    Option o;		/* which option */
	    char *name;		/* name of TB */
	    char *title;	/* title string of option */
	    char *tip;		/* widget tip */
	} OpSetup;
	static OpSetup ops[] = {
	    {SPACECRAFT_OPT,	"Spacecraft",	"Spacecraft",
	    	"Label each Spacecraft landing site"},
	    {LABELS_OPT,	"Labels",	"Labels",
	    	"Whether to display selected labels"},
	    {SKY_OPT,		"SkyBkg",	"Sky background",
	    	"When on, sky will include database objects and Field Stars"},
	    {UMBRA_OPT,		"Umbra",	"{Pen}Umbra",
	    	"Display edges of Earth shadow (if near a lunar eclipse)"},
	    {FLIPTB_OPT,	"FlipTB",	"Flip T/B",
	    	"Flip map top-to-bottom"},
	    {FLIPLR_OPT,	"FlipLR",	"Flip L/R",
	    	"Flip map left-to-right"},
	    {GRID_OPT,		"Grid",		"Grid",
	    	"Overlay 15 degree grid and mark Sub-Earth location"},
	};
	typedef struct {
	    char *label;	/* what goes on the help label */
	    char *key;		/* string to call hlp_dialog() */
	} HelpOn;
	static HelpOn helpon[] = {
	    {"Intro...",	"Moon"},
	    {"on Mouse...",	"Moon_mouse"},
	    {"on Control...",	"Moon_control"},
	    {"on View...",	"Moon_view"},
	    {"on Scale...",	"Moon_scale"},
	    {"on Lunar Orbiter...",	"Moon_lo"},
	};
	Widget mb_w, pd_w, cb_w;
	Widget w;
	Widget mform_w;
	XmString str;
	unsigned long mask;
	Arg args[20];
	int i;
	int n;

	/* create master form */
	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Moon view"); n++;
	XtSetArg (args[n], XmNiconName, "Moon"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	mshell_w = XtCreatePopupShell ("Moon", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (mshell_w);
	set_something (mshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (mshell_w, XmNpopdownCallback, m_popdown_cb, 0);
	sr_reg (mshell_w, "XEphem*Moon.width", mooncategory, 0);
	sr_reg (mshell_w, "XEphem*Moon.height", mooncategory, 0);
	sr_reg (mshell_w, "XEphem*Moon.x", mooncategory, 0);
	sr_reg (mshell_w, "XEphem*Moon.y", mooncategory, 0);
	sr_reg (NULL, m_viewupres(), mooncategory, 0);

	n = 0;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	mform_w = XmCreateForm (mshell_w, "MoonF", args, n);
	XtAddCallback (mform_w, XmNhelpCallback, m_help_cb, 0);
	XtManageChild (mform_w);

	/* create the menu bar across the top */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (mform_w, "MB", args, n);
	XtManageChild (mb_w);

	/* make the Control pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "ControlPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'C'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "ControlCB", args, n);
	    set_xmstring (cb_w, XmNlabelString, "Control");
	    XtManageChild (cb_w);

	    /* the "Print" push button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "PrintPB", args, n);
	    set_xmstring (w, XmNlabelString, "Print...");
	    XtAddCallback (w, XmNactivateCallback, m_print_cb, 0);
	    wtip (w, "Print the current Moon map");
	    XtManageChild (w);

	    /* the "Annot" push button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Ann", args, n);
	    set_xmstring (w, XmNlabelString, "User annotation...");
	    XtAddCallback (w, XmNactivateCallback, ano_cb, 0);
	    wtip (w, "Open window to create and manage your own annotation");
	    XtManageChild (w);

	    /* the "Field stars" push button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "FS", args, n);
	    set_xmstring (w, XmNlabelString, "Field Stars...");
	    XtAddCallback (w, XmNactivateCallback, (XtCallbackProc)fs_manage,0);
	    wtip (w, "Define where GSC and PPM catalogs are to be found");
	    XtManageChild (w);

	    /* button to bring up the earthshine dialog */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "ESPB", args, n);
	    XtAddCallback (w, XmNactivateCallback, m_eshineup_cb, 0);
	    set_xmstring (w, XmNlabelString, "Set Earthshine...");
	    wtip (w, "Display a dialog to set brightness of dark half of moon");
	    XtManageChild (w);

	    /* button for Movie demo */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "EAnim", args, n);
	    XtAddCallback (w, XmNactivateCallback, m_anim_cb, 0);
	    set_xmstring (w, XmNlabelString, "Animation demo");
	    wtip (w, "Start/Stop a fun time-lapse animation");
	    XtManageChild (w);

	    /* button to erase labels */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "MEL", args, n);
	    XtAddCallback (w, XmNactivateCallback, m_elab_cb, 0);
	    set_xmstring (w, XmNlabelString, "Forget labels");
	    wtip (w, "Forget all non-spacecraft labels added to the map");
	    XtManageChild (w);

	    /* the "Movie" push button */

	    n = 0;
	    n += ml_addacc (args, n);
	    w = XmCreatePushButton (pd_w, "MovieL", args, n);
	    set_xmstring (w, XmNlabelString, "Add to movie...");
	    XtAddCallback (w, XmNactivateCallback, m_mloop_cb, 0);
	    wtip (w, "Add lunar image to movie loop");
	    XtManageChild (w);

	    /* add a separator */
	    n = 0;
	    w = XmCreateSeparator (pd_w, "CtS", args, n);
	    XtManageChild (w);

	    /* add the close button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, m_close_cb, 0);
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

	    /* add options */

	    for (i = 0; i < XtNumber(ops); i++) {
		OpSetup *osp = &ops[i];
		Option o = osp->o;

		n = 0;
		XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		w = XmCreateToggleButton (pd_w, osp->name, args, n);
		XtAddCallback(w, XmNvalueChangedCallback, m_option_cb,
								(XtPointer)o);
		set_xmstring (w, XmNlabelString, osp->title);
		option[o] = XmToggleButtonGetState (w);
		option_w[o] = w;
		if (osp->tip)
		    wtip (w, osp->tip);
		XtManageChild (w);
		sr_reg (w, NULL, mooncategory, 1);
	    }

	    /* add a separator */

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    /* add the More Info control */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Stats", args, n);
	    set_xmstring (w, XmNlabelString, "More info...");
	    XtAddCallback (w, XmNactivateCallback, m_mstats_cb, 0);
	    wtip (w, "Display additional information");
	    XtManageChild (w);

	/* make the Scale pulldown */

	n = 0;
	XtSetArg (args[n], XmNradioBehavior, True); n++;
	pd_w = XmCreatePulldownMenu (mb_w, "ScalePD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'S'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "ScaleCB", args, n);
	    set_xmstring (cb_w, XmNlabelString, "Scale");
	    wtip (cb_w, "Set the image size");
	    XtManageChild (cb_w);

	    for (i = 1; i <= MAXSHRINK; i++) {
		char name[64], title[64];
		int shrink;

		/* just advertise whole values */
		if (MAXSHRINK%i)
		    continue;
		shrink = MAXSHRINK/i;

		(void) sprintf (name, "Scale%dX", shrink);
		n = 0;
		w = XmCreateToggleButton (pd_w, name, args, n);
		XtAddCallback(w, XmNvalueChangedCallback, m_shrink_cb,
								(XtPointer)(long int)i);
		(void) sprintf (title, " Scale %d X", shrink);
		set_xmstring (w, XmNlabelString, title);
		XtManageChild (w);

		/* pick up user's default */
		if (XmToggleButtonGetState(w)) {
		    if (mshrink != 0)
			xe_msg (0, "Multiple setting: ignoring Moon*%s", name);
		    else
			mshrink = i;
		}

		/* if done and none set, default to last one */
		if (i == MAXSHRINK && mshrink == 0) {
		    xe_msg (0, "No Moon*Scale<n>X resource -- defaulting to 1");
		    XmToggleButtonSetState (w, True, False);
		    mshrink = MAXSHRINK;
		}

		sr_reg (w, NULL, mooncategory, 1);
	    }

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
		XtAddCallback (w, XmNactivateCallback, m_helpon_cb,
							(XtPointer)(hp->key));
		XtManageChild (w);
		XmStringFree(str);
	    }

	/* make a label for the date stamp */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	dt_w = XmCreateLabel (mform_w, "DateStamp", args, n);
	timestamp (mm_get_now(), dt_w);	/* sets initial size */
	wtip (dt_w, "Date and Time for which map is computed");
	XtManageChild (dt_w);

	/* make a scrolled list on the right for the names */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, dt_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNselectionPolicy, XmBROWSE_SELECT); n++;
	msl_w = XmCreateScrolledList (mform_w, "MoonSL", args, n);
	XtAddCallback (msl_w, XmNbrowseSelectionCallback, m_select_cb,0);
	XtAddCallback (msl_w, XmNdefaultActionCallback, m_select_cb,0);
	XtManageChild (msl_w);

	/* make a drawing area in a scrolled window for the image view */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, dt_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, XtParent(msl_w)); n++;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	XtSetArg (args[n], XmNvisualPolicy, XmVARIABLE); n++;
	msw_w = XmCreateScrolledWindow (mform_w, "MoonSW", args, n);
	XtManageChild (msw_w);

	    n = 0;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    mda_w = XmCreateDrawingArea (msw_w, "MoonMap", args, n);
	    XtAddCallback (mda_w, XmNexposeCallback, m_exp_cb, NULL);
	    mask = Button1MotionMask | ButtonPressMask | ButtonReleaseMask
						    | PointerMotionHintMask;
	    XtAddEventHandler (mda_w, mask, False, m_pointer_eh, 0);
	    XtManageChild (mda_w);

	    /* SW assumes work is its child but just to be tidy about it .. */
	    set_something (msw_w, XmNworkWindow, (XtArgVal)mda_w);

	/* match SW background to DA */
	get_something (msw_w, XmNclipWindow, (XtArgVal)&w);
	if (w) {
	    Pixel p;
	    get_something (mda_w, XmNbackground, (XtArgVal)&p);
	    set_something (w, XmNbackground, (XtArgVal)p);
	}
}

static void
m_create_msform()
{
	typedef struct {
	    int islabel;
	    char *label;
	    Widget *wp;
	    char *tip;
	} DItem;
	static DItem citems[] = {
	    {1,"Latitude +N:",  &lat_w, "Selenographic latitude under cursor"},
	    {1,"Longitude +E:", &lng_w, "Selenographic longitude under cursor"},
	    {0,"Sun altitude:", &sunalt_w,
	    	"Sun angle above horizon as seen from location under cursor"},
	    {1,"Next Sunrise:", &nsrt_w,
	    	"Time of next sunrise as seen from location under cursor"},
	    {1," ",             &nsrd_w,
	    	"Date of next sunrise as seen from location under cursor"},
	    {1,"Next Sunset:",  &nsst_w,
	    	"Time of next sunset as seen from location under cursor"},
	    {1," ",             &nssd_w,
	    	"Date of next sunset as seen from location under cursor"},
	};
	Widget rc_w;
	Widget sep_w;
	Widget w;
	XmString str;
	Arg args[20];
	int n;
	int i;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	msform_w = XmCreateFormDialog (mshell_w, "MoonStats", args, n);
	set_something (msform_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (XtParent(msform_w), "XEphem*MoonStats.x", mooncategory,0);
	sr_reg (XtParent(msform_w), "XEphem*MoonStats.y", mooncategory,0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Moon info"); n++;
	XtSetValues (XtParent(msform_w), args, n);

	/* label for title */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	infot_w = XmCreateLabel (msform_w, "MIL", args, n);
	set_xmstring (infot_w, XmNlabelString, " ");
	XtManageChild (infot_w);

	/* make a rowcolumn to hold the cursor tracking info */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, infot_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNspacing, 5); n++; /* matches RC below better */
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNnumColumns, XtNumber(citems)); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	rc_w = XmCreateRowColumn (msform_w, "SRC", args, n);
	XtManageChild (rc_w);

	    for (i = 0; i < XtNumber(citems); i++) {
		DItem *dp = &citems[i];

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
		w = XmCreateLabel (rc_w, "CLbl", args, n);
		set_xmstring (w, XmNlabelString, dp->label);
		XtManageChild (w);

		n = 0;
		XtSetArg (args[n], XmNrecomputeSize, False); n++;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		if (dp->islabel) {
		    w = XmCreateLabel (rc_w, "CVal", args, n);
		} else {
		    XtSetArg (args[n], XmNuserData, "Moon.SunAlt"); n++;
		    w = XmCreatePushButton (rc_w, "CSA", args, n);
		    XtAddCallback (w, XmNactivateCallback, m_activate_cb, NULL);
		}
		set_xmstring (w, XmNlabelString, " ");

		if (dp->wp)
		    *(dp->wp) = w;
		if (dp->tip)
		    wtip (w, dp->tip);
		XtManageChild (w);
	    }

	/* make a separator between the 2 data sets */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (msform_w, "Sep1", args, n);
	XtManageChild(sep_w);

	/* make a rowcolumn to hold the labels and info buttons */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNnumColumns, 8); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	rc_w = XmCreateRowColumn (msform_w, "SRC", args, n);
	XtManageChild (rc_w);

	    /* make the srlng, sslat and libration in lat/long rows */

	    str = XmStringCreate ("Sunrise Long:", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (rc_w, "MLCoL", args, n);
	    XtManageChild (w);
	    XmStringFree (str);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	    XtSetArg (args[n], XmNuserData, "Moon.SunRiseLng"); n++;
	    srlng_w = XmCreatePushButton (rc_w, "MLCoLPB", args, n);
	    XtAddCallback (srlng_w, XmNactivateCallback, m_activate_cb, NULL);
	    wtip (srlng_w, "Selenographic longitude where sun is now rising");
	    XtManageChild (srlng_w);

	    str = XmStringCreate ("Subsolar Lat:", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (rc_w, "MLSL", args, n);
	    XtManageChild (w);
	    XmStringFree (str);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	    XtSetArg (args[n], XmNuserData, "Moon.SubSolLat"); n++;
	    sslat_w = XmCreatePushButton (rc_w, "MLSLPB", args, n);
	    XtAddCallback (sslat_w, XmNactivateCallback, m_activate_cb, NULL);
	    wtip (sslat_w, "Selenographic latitude at which sun is directly overhead");
	    XtManageChild (sslat_w);

	    str = XmStringCreate ("Libr in Lat:", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (rc_w, "MLLatL", args, n);
	    XtManageChild (w);
	    XmStringFree (str);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	    XtSetArg (args[n], XmNuserData, "Moon.LibrLat"); n++;
	    llat_w = XmCreatePushButton (rc_w, "MLLatPB", args, n);
	    XtAddCallback (llat_w, XmNactivateCallback, m_activate_cb, NULL);
	    wtip (llat_w, "Current tilt in latitude from nominal `face-on' direction, + Lunar N");
	    XtManageChild (llat_w);

	    str = XmStringCreate ("Libr in Long:", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (rc_w, "MLLongL", args, n);
	    XtManageChild (w);
	    XmStringFree (str);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	    XtSetArg (args[n], XmNuserData, "Moon.LibrLong"); n++;
	    llong_w = XmCreatePushButton (rc_w, "MLLongPB", args, n);
	    XtAddCallback (llong_w, XmNactivateCallback, m_activate_cb, NULL);
	    wtip (llong_w, "Current tilt in longitude from nominal `face-on' direction, + Lunar E");
	    XtManageChild (llong_w);

	    /* make the limb/tilt rows */

	    str = XmStringCreate ("Limb angle:", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (rc_w, "MLimbL", args, n);
	    XtManageChild (w);
	    XmStringFree (str);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	    XtSetArg (args[n], XmNuserData, "Moon.LibrLimb"); n++;
	    limb_w = XmCreatePushButton (rc_w, "MLimbPB", args, n);
	    XtAddCallback (limb_w, XmNactivateCallback, m_activate_cb, NULL);
	    wtip (limb_w, "Limb angle nearest Earth, + Lunar W of N");
	    XtManageChild (limb_w);

	    str = XmStringCreate ("Tilt:", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (rc_w, "MLibL", args, n);
	    XtManageChild (w);
	    XmStringFree (str);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	    XtSetArg (args[n], XmNuserData, "Moon.LibrTilt"); n++;
	    lib_w = XmCreatePushButton (rc_w, "MLibPB", args, n);
	    XtAddCallback (lib_w, XmNactivateCallback, m_activate_cb, NULL);
	    wtip(lib_w,"Degrees by which nominal face is tilted towards Earth");
	    XtManageChild (lib_w);

	    str = XmStringCreate ("Age:", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateLabel (rc_w, "MAge", args, n);
	    XtManageChild (w);
	    XmStringFree (str);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	    XtSetArg (args[n], XmNuserData, "Moon.Age"); n++;
	    age_w = XmCreatePushButton (rc_w, "MLibPB", args, n);
	    XtAddCallback (age_w, XmNactivateCallback, m_activate_cb, NULL);
	    wtip (age_w, "Days since new moon");
	    XtManageChild (age_w);

	/* add a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (msform_w, "Sep2", args, n);
	XtManageChild (sep_w);

	/* add a label for the current date/time stamp */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	sdt_w = XmCreateLabel (msform_w, "SDTstamp", args, n);
	wtip (sdt_w, "Date and Time for which data are computed");
	XtManageChild (sdt_w);

	/* add a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sdt_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (msform_w, "Sep3", args, n);
	XtManageChild (sep_w);

	/* add a close button */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 20); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 80); n++;
	w = XmCreatePushButton (msform_w, "Close", args, n);
	XtAddCallback (w, XmNactivateCallback, m_mstats_close_cb, 0);
	wtip (w, "Close this dialog");
	XtManageChild (w);
}

/* create the little earthshine scale factor dialog */
static void
m_create_esform()
{
	Arg args[20];
	Widget sep_w;
	Widget w;
	int min, max, v;
	int n;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNfractionBase, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	esform_w = XmCreateFormDialog (mshell_w, "MoonEShine", args, n);
	set_something (esform_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (XtParent(esform_w), "XEphem*MoonEShine.x",mooncategory,0);
	sr_reg (XtParent(esform_w), "XEphem*MoonEShine.y",mooncategory,0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Moon Earthshine"); n++;
	XtSetValues (XtParent(esform_w), args, n);

	/* make a scale */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNtopOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNshowValue, True); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNminimum, 0); n++;
	XtSetArg (args[n], XmNmaximum, ESHINEF); n++;
	eshine_w = XmCreateScale (esform_w, "Earthshine", args, n);
	wtip (eshine_w, "Set to desired relative Earthshine brightness");
	XtManageChild (eshine_w);
	sr_reg (eshine_w, NULL, mooncategory, 1);

	get_something (eshine_w, XmNminimum, (XtArgVal)&min);
	get_something (eshine_w, XmNmaximum, (XtArgVal)&max);
	get_something (eshine_w, XmNvalue, (XtArgVal)&v);

	if (min >= max || v < min || v > max || v == 0) {
	    xe_msg (0, "Bogus moon Earthshine values -- setting defaults");
	    set_something (eshine_w, XmNminimum, (XtArgVal)1);
	    set_something (eshine_w, XmNmaximum, (XtArgVal)10);
	    set_something (eshine_w, XmNvalue, (XtArgVal)4);
	}

	/* make a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, eshine_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	sep_w = XmCreateSeparator (esform_w, "Sep", args, n);
	XtManageChild(sep_w);

	/* make the buttons at the bottom */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 2); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 4); n++;
	w = XmCreatePushButton (esform_w, "Apply", args, n);
	XtAddCallback (w, XmNactivateCallback, m_eshine_cb, NULL);
	wtip (w, "Make it so");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 6); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 8); n++;
	w = XmCreatePushButton (esform_w, "Close", args, n);
	XtAddCallback (w, XmNactivateCallback, m_eshineclose_cb, NULL);
	wtip (w, "Close this dialog");
	XtManageChild (w);
}

/* go through all the buttons pickable for plotting and set whether they
 * should appear to look like buttons or just flat labels.
 */
static void
m_set_buttons (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	buttonAsButton (sunalt_w, whether);
	buttonAsButton (srlng_w, whether);
	buttonAsButton (sslat_w, whether);
	buttonAsButton (llat_w, whether);
	buttonAsButton (llong_w, whether);
	buttonAsButton (limb_w, whether);
	buttonAsButton (lib_w, whether);
	buttonAsButton (age_w, whether);
}

/* callback from the Apply button in the earthshine dialog */
/* ARGSUSED */
static void
m_eshine_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	m_redraw();
}

/* callback to add the current scene to the movie loop */
/* ARGSUSED */
static void
m_mloop_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	ml_add (m_pm, dt_w);
}

/* callback from the Close button in the earthshine dialog */
/* ARGSUSED */
static void
m_eshineclose_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (esform_w);
}

/* called to toggle whether the earthshine eshineactor is dialog */
/* ARGSUSED */
static void
m_eshineup_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XtIsManaged(esform_w))
	    XtUnmanageChild (esform_w);
	else
	    XtManageChild (esform_w);
}


/* callback from the Close button on the stats menu.
 */
/* ARGSUSED */
static void
m_mstats_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (msform_w);
}

/* callback from the More Info button on the main menu.
 */
/* ARGSUSED */
static void
m_mstats_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtManageChild (msform_w);
	m_set_buttons(m_selecting);
}

/* callback from the Print PB */
/* ARGSUSED */
static void
m_print_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
        XPSAsk ("Moon View", m_print);
}

/* proceed to generate a postscript file.
 * call XPSClose() when finished.
 */
static void
m_print ()
{
	/* must be up */
	if (!m_ison()) {
	    xe_msg (1, "Moon must be open to print.");
	    XPSClose();
	    return;
	}

	watch_cursor(1);

	/* fit view in square across the top and prepare to capture X calls */
	XPSXBegin (m_pm, BORD, BORD, mncols, mnrows, 1*72, 10*72,
								(int)(6.5*72));

	/* redraw everything now */
	m_redraw();

        /* no more X captures */
	XPSXEnd();

	/* add some extra info */
	m_ps_annotate ();

	/* finished */
	XPSClose();

	watch_cursor(0);
}

static void
m_ps_annotate ()
{
	Now *np = mm_get_now();
        char dir[128];
	char buf[128];
	int ctr = 306;  /* = 8.5*72/2 */
	int lx = 180, rx = 460;
	int y;

	/* caption */
	y = AROWY(13);
	if (option[SKY_OPT])
	    (void) sprintf (buf, "XEphem %s Moon View",
		    pref_get(PREF_EQUATORIAL)==PREF_GEO ? "Geocentric"
							: "Topocentric");
	else
	    (void) strcpy (buf, "XEphem Moon View");
	(void) sprintf (dir, "(%s) %d %d cstr\n", buf, ctr, y);
	XPSDirect (dir);

	y = AROWY(9);
	fs_date (buf, pref_get(PREF_DATE_FORMAT), mjd_day(mjd));
	(void) sprintf (dir, "(UTC Date:) %d %d rstr (%s) %d %d lstr\n",
							lx, y, buf, lx+10, y);
	XPSDirect (dir);

	fs_time (buf, mjd_hr(mjd));
	(void) sprintf (dir, "(UTC Time:) %d %d rstr (%s) %d %d lstr\n",
							rx, y, buf, rx+10, y);
	XPSDirect (dir);

	(void) sprintf (dir, "(%6.3f days old) %d %d cstr\n", minfo.age, ctr,y);
	XPSDirect (dir);

	y = AROWY(8);
	fs_dm_angle (buf, minfo.srlng);
	(void) sprintf (dir,"(Sunrise Longitude:) %d %d rstr (%s) %d %d lstr\n",
							lx, y, buf, lx+10, y);
	XPSDirect (dir);

	fs_dm_angle (buf, minfo.sslat);
	(void) sprintf (dir,"(Subsolar Latitude:) %d %d rstr (%s) %d %d lstr\n",
							rx, y, buf, rx+10, y);
	XPSDirect (dir);

	y = AROWY(7);
	fs_dm_angle (buf, minfo.llat);
	(void) sprintf (dir,
			"(Libration in Latitude:) %d %d rstr (%s) %d %d lstr\n",
							lx, y, buf, lx+10, y);
	XPSDirect (dir);

	fs_dm_angle (buf, minfo.llong);
	(void) sprintf (dir,
		    "(Libration in Longitude:) %d %d rstr (%s) %d %d lstr\n",
							rx, y, buf, rx+10, y);
	XPSDirect (dir);

	y = AROWY(6);
	fs_dm_angle (buf, minfo.limb);
	(void) sprintf (dir,
		    "(Limb Angle:) %d %d rstr (%s) %d %d lstr\n",
							lx, y, buf, lx+10, y);
	XPSDirect (dir);

	fs_dm_angle (buf, minfo.limb);
	(void) sprintf (dir,
		    "(Limb Tilt Angle:) %d %d rstr (%6.3f) %d %d lstr\n",
						rx, y, minfo.tilt, rx+10, y);
	XPSDirect (dir);

	/* add site/lat/long if showing real stars and topocentric */
	if (option[SKY_OPT] && pref_get(PREF_EQUATORIAL) == PREF_TOPO) {
	    char *site;

	    /* put site name under caption */
	    site = mm_getsite();
	    if (site) {
		y = AROWY(12);
		(void) sprintf (dir, "(%s) %d %d cstr\n",
	    				XPSCleanStr(site,strlen(site)), ctr, y);
		XPSDirect (dir);
	    }

	    /* then add lat/long */
	    y = AROWY(10);

	    fs_sexa (buf, raddeg(fabs(lat)), 3, 3600);
	    (void) sprintf (dir, "(Latitude:) %d %d rstr (%s %c) %d %d lstr\n",
				    lx, y, buf, lat < 0 ? 'S' : 'N', lx+10, y);
	    XPSDirect (dir);

	    fs_sexa (buf, raddeg(fabs(lng)), 4, 3600);
	    (void) sprintf (dir,"(Longitude:) %d %d rstr (%s %c) %d %d lstr\n",
				    rx, y, buf, lng < 0 ? 'W' : 'E', rx+10, y);
	    XPSDirect (dir);
	}
}

/* callback from any of the data menu buttons being activated.
 */
/* ARGSUSED */
static void
m_activate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (m_selecting) {
	    char *name;
	    get_something (w, XmNuserData, (XtArgVal)&name);
	    register_selection (name);
	}
}

/* callback from mshell_w being popped down.
 */
/* ARGSUSED */
static void
m_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (esform_w);
	XtUnmanageChild (msform_w);

	/* freeing the pixmap will prevent any useless updates while off and
	 * insure a fresh update on the next expose.
	 */
	if (m_pm) {
	    XFreePixmap (XtD, m_pm);
	    m_pm = 0;
	}

	/* free any field stars */
	if (fstars) {
	    free ((void *)fstars);
	    fstars = NULL;
	    nfstars = 0;
	}

	/* stop movie that might be running */
	mm_movie (0.0);

	/* register we are now down */
	setXRes (m_viewupres(), "0");
}

/* called from Close button */
static void
m_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (mshell_w);
	/* let popdown do all the work */
}

/* callback from the any of the option TBs.
 * Option enum is in client.
 */
/* ARGSUSED */
static void
m_option_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Option opt = (Option)client;
	int set;

	watch_cursor (1);

	/* toggle the option */
	option[opt] = set = XmToggleButtonGetState (w);

	switch (opt) {
	case GRID_OPT:
	    if (set) {
		m_grid();
		m_refresh (NULL);
	    } else
		m_redraw();
	    break;

	case SKY_OPT:
	    m_redraw();
	    break;

	case UMBRA_OPT:
	    if (set) {
		m_umbra();
		m_sub();
		m_refresh (NULL);
	    } else
		m_redraw();
	    break;

	case SPACECRAFT_OPT:	/* FALLTHRU */
	case LABELS_OPT:
	    if (set) {
		m_labels();
		m_refresh (NULL);
	    } else
		m_redraw();
	    break;

	case FLIPTB_OPT:
	    m_free_clip_mask();
	    fliptb();
	    m_redraw();
	    break;

	case FLIPLR_OPT:
	    m_free_clip_mask();
	    fliplr();
	    m_redraw();
	    break;

	case N_OPT:
	    break;
	}

	watch_cursor (0);
}

/* callback from any of the Scale TBs.
 * client is shrink factor: 1..MAXSHRINK
 */
/* ARGSUSED */
static void
m_shrink_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int f = (long int)client;
	int set = XmToggleButtonGetState (w);

	if (!set || f == mshrink)
	    return;
	if (m_shrink (f) < 0)
	    return;
	mshrink = f;
}


/* callback to erase all labels */
/* ARGSUSED */
static void
m_elab_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	MoonDB *mp;

	/* turn off all persistent label flags then redraw to erase */
	for (mp = moondb; mp < &moondb[nmoondb]; mp++)
	    mp->pl = 0;
	m_redraw();
}

/* callback for when the Movie button is activated. */
/* ARGSUSED */
static void
m_anim_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
        mm_movie (option[SKY_OPT] ? MOVIE_SSTEPSZ : MOVIE_STEPSZ);
}

/* callback from the Help all button
 */
/* ARGSUSED */
static void
m_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
	    "This is a depiction of the Moon.",
	};

	hlp_dialog ("Moon", msg, XtNumber(msg));
}

/* callback from a specific Help button.
 * client is a string to use with hlp_dialog().
 */
/* ARGSUSED */
static void
m_helpon_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hlp_dialog ((char *)client, NULL, 0);
}


/* given an [x,y] on m_xim, find latitude, rads north, and longitude, rads e.
 * N.B. [x,y] must be with respect to the _original_ image size and orientation.
 * return 0 if ok (inside image) else -1.
 */
static int
xy2ll (x, y, ltp, lgp)
int x, y;
double *ltp, *lgp;
{
	double cx, cy;
	double lt, lg;

	/* convert to image center, scaled to radius */
	cx = (double)(x - (leftmarg + moonrad))/moonrad;	/* + right */
	cy = (double)((topmarg + moonrad)  - y)/moonrad;	/* + up */

	if (cx*cx + cy*cy > 1.0)
	    return (-1);

	lt = asin (cy);
	lg = asin(cx/cos(lt));

	lt += CLAT;
	lg += CLNG;
	if (lt > PI/2) {
	    lt = PI - lt;
	    lg += PI;
	} else if (lt < -PI/2) {
	    lt = -PI - lt;
	    lg += PI;
	}

	*ltp = lt;
	*lgp = lg;
	range (lgp, 2*PI);

	return (0);
}

/* given a lat(+N)/long(+E) find its [x,y] location on the resized imae.
 * N.B. the returned [x,y] is with respect to the image at its current size
 *   but does not include flipping or mda_w border.
 * return 0 if ok (on front side) else -1, but always return x and y.
 */
static int
ll2xy (lt, lg, xp, yp)
double lt, lg;
int *xp, *yp;
{
	double cx, cy, cz;
	double coslt, sinlt;

	lt -= CLAT;
	lg -= CLNG;
	if (lt > PI/2) {
	    lt = PI - lt;
	    lg += PI;
	} else if (lt < -PI/2) {
	    lt = -PI - lt;
	    lg += PI;
	}

	coslt = cos(lt);
	cz = moonrad*coslt*cos(lg);

	sinlt = sin(lt);
	cx = moonrad*coslt*sin(lg);
	cy = moonrad*sinlt;

	*xp = (int)(cx + (leftmarg + moonrad) + 0.5);
	*yp = (int)((topmarg + moonrad) -  cy + 0.5);

	return (cz < 0 ? -1 : 0);
}

/* return True if the given [x,y], relative to mda_w, refers to a spot over the
 * lunar image, else return 0.
 */
static int
overMoon (x, y)
int x, y;
{
	/* convert to image center */
	x -= BORD + leftmarg + moonrad;
	y -= BORD + topmarg  + moonrad;

	return (x*x + y*y <= moonrad*moonrad);
}

/* event handler from all Button events on the mda_w */
static void
m_pointer_eh (w, client, ev, continue_to_dispatch)
Widget w;
XtPointer client;
XEvent *ev;
Boolean *continue_to_dispatch;
{
	Display *dsp = ev->xany.display;
	Window win = ev->xany.window;
	int evt = ev->type;
	Window root, child;
	int rx, ry, x, y;
	unsigned mask;
	int m1, b1p, b1r, b3p;

	/* what happened? */
	m1  = evt == MotionNotify  && ev->xmotion.state   & Button1Mask;
	b1p = evt == ButtonPress   && ev->xbutton.button == Button1;
	b1r = evt == ButtonRelease && ev->xbutton.button == Button1;
	b3p = evt == ButtonPress   && ev->xbutton.button == Button3;

	/* do we care? */
	if (!m1 && !b1p && !b1r && !b3p)
	    return;

	/* where are we? */
	XQueryPointer (dsp, win, &root, &child, &rx, &ry, &x, &y, &mask);

	/* dispatch */
	if (b3p)
	    m_popup (ev);
	if (b1p || m1 || b1r) {
	    doGlass (dsp, win, b1p, m1, b1r, x, y);
	    m_reportloc (dsp, win, x, y);
	}
}

/* expose (or reconfig) of moon image view drawing area.
 */
/* ARGSUSED */
static void
m_exp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
	XExposeEvent *e = &c->event->xexpose;
	Display *dsp = e->display;
	Window win = e->window;

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


	/* insure pixmap exists -- it gets destroyed whenever mda_w is resized
	 * or the dialog is unmanaged.
	 */
	if (!m_pm) {
	    unsigned wid, hei, d;

	    m_getsize (win, &wid, &hei, &d);
	    if ((int)wid != mncols+2*BORD || (int)hei != mnrows+2*BORD) {
		printf ("m_da: Bad size: wid=%d mncols=%d hei=%d mnrows=%d\n",
					wid, mncols, hei, mnrows);
		abort();
	    }

	    m_pm = XCreatePixmap (dsp, win, wid, hei, d);
	    m_draw();
	}

	/* update exposed area */
	m_refresh (e);

	watch_cursor (0);
}

/* redraw the current scene */
static void
m_redraw()
{
	watch_cursor (1);

	m_draw ();
	m_refresh(NULL);

	watch_cursor (0);
}

/* copy the m_pm pixmap to the drawing area mda_w.
 * if ep just copy that much, else copy all.
 */
static void
m_refresh(ep)
XExposeEvent *ep;
{
	Display *dsp = XtDisplay(mda_w);
	Window win = XtWindow (mda_w);
	Pixmap pm = m_pm;
	unsigned w, h;
	int x, y;

	/* ignore if no pixmap now */
	if (!pm)
	    return;

	if (ep) {
	    x = ep->x;
	    y = ep->y;
	    w = ep->width;
	    h = ep->height;
	} else {
	    w = mncols+2*BORD;
	    h = mnrows+2*BORD;
	    x = y = 0;
	}

	XCopyArea (dsp, pm, win, m_fgc, x, y, w, h, x, y);
}

/* get the width, height and depth of the given drawable */
static void
m_getsize (d, wp, hp, dp)
Drawable d;
unsigned *wp, *hp, *dp;
{
	Window root;
	int x, y;
	unsigned int bw;

	XGetGeometry (XtD, d, &root, &x, &y, wp, hp, &bw, dp);
}

/* make the various gcs, handy pixel values and fill in mgray[].
 * N.B. just call this once.
 * TODO: reclaim old stuff if called again.
 */
static void
m_init_gcs()
{
	Display *dsp = XtD;
	Window win = XtWindow(toplevel_w);
	Colormap cm = xe_cm;
	XGCValues gcv;
	unsigned int gcm;
	Pixel fg;
	Pixel p;

	/* fg and bg */
	get_something (mda_w, XmNforeground, (XtArgVal)&fg);
	(void) get_color_resource (mda_w, "MoonBackground", &mbg);

	gcm = GCForeground | GCBackground;
	gcv.foreground = fg;
	gcv.background = mbg;
	m_fgc = XCreateGC (dsp, win, gcm, &gcv);

	gcv.foreground = mbg;
	gcv.background = fg;
	m_bgc = XCreateGC (dsp, win, gcm, &gcv);

	/* make the label marker gc */
	(void) get_color_resource (mda_w, "MoonAnnotColor", &p);
	gcm = GCForeground | GCBackground;
	gcv.foreground = p;
	gcv.background = mbg;
	m_agc = XCreateGC (dsp, win, gcm, &gcv);
	get_views_font (dsp, &m_fsp);

	/* build gray-scale ramp for image */
	nmgray = gray_ramp (dsp, cm, &mgray);

	/* unless we will be using a bitmap, force color 0 to background. */
	if (nmgray > 2)
	    mgray[0] = mbg;
}

/* given curlt/lg and Now compute supporting moon info in minfo */
static void
m_info(np)
Now *np;
{
	double tzo = pref_get(PREF_ZONE)==PREF_LOCALTZ ? -tz/24.0 : 0;
	double srlng;
	double t;
	double nfmjd, nm, fm;

	moon_colong (mjd+MJD0, minfo.curlt, minfo.curlg, &srlng, NULL,
						&minfo.cursunalt, &minfo.sslat);

	/* convert colong to sunrise longitude */
	srlng = -srlng;
	range (&srlng, 2*PI);
	minfo.srlng = srlng;

	t = srlng-minfo.curlg;
	range (&t, 2*PI);
	minfo.curnsrt = mjd + tzo + t/(2*PI)*SYNP;

	t = srlng-minfo.curlg+PI;
	range (&t, 2*PI);
	minfo.curnsst = mjd + tzo + t/(2*PI)*SYNP;

	/* libration info */
	llibration (mjd+MJD0, &minfo.llat, &minfo.llong);
	minfo.limb = atan2 (-minfo.llong, minfo.llat);
	if (minfo.limb < 0.0)
	    minfo.limb += 2*PI;	/* limb angle is traditionally 0..360 */
	minfo.tilt =
		raddeg(sqrt(minfo.llat*minfo.llat + minfo.llong*minfo.llong));

	/* age, days from prev new moon */
	for (nfmjd = mjd, nm = mjd+1; nm > mjd; nfmjd -= 7)
	    moonnf (nfmjd, &nm, &fm);
	minfo.age = mjd - nm;
}

/* update "more info" labels.
 * then, if m_pm is defined, compute a scene onto it.
 */
static void
m_draw ()
{
	Now *np = mm_get_now();

	/* update and print the latest info */
	m_info (np);
	m_pinfo ();

	/* update time stamps too */
	timestamp (np, dt_w);
	timestamp (np, sdt_w);

	if (m_pm)
	    mi_draw ();
}

/* draw moon onto m_pm, using minfo.
 * N.B. this just fills the pixmap; call m_refresh() to copy to the screen.
 */
static void
mi_draw ()
{
	XGCValues gcv;
	unsigned int gcm;

	/* check assumptions -- even graphic uses image under the glass */
	if (!mimage) {
	    printf ("No moon mimage!\n");
	    abort();
	}
	if (!m_xim) {
	    printf ("No m_xim!\n");
	    abort();
	}
	if (!m_pm) {
	    printf ("No m_pm Pixmap!\n");
	    abort();
	}

	/* fill in m_xim from mimage */
	image_setup ();

	/* create clip_mask from mimage when needed */
	m_clip_mask_setup ();

	/* clear m_pm */
	XPSPaperColor (mbg);
	XPSFillRectangle (XtD, m_pm, m_bgc, 0, 0, mncols+2*BORD, mnrows+2*BORD);

	/* add sky background objects */
	resetSkyObj();
	if (option[SKY_OPT])
	    m_sky();

	/* apply clipping mask to m_fgc */
	gcm = GCClipXOrigin | GCClipYOrigin | GCClipMask;
	gcv.clip_mask = m_clip_mask;
	gcv.clip_x_origin = BORD;
	gcv.clip_y_origin = BORD;
	XChangeGC(XtD, m_fgc, gcm, &gcv);

	/* copy m_xim within BORDER */
	XPutImage (XtD, m_pm, m_fgc, m_xim, 0, 0, BORD, BORD, mncols, mnrows);

	/* remove clipping mask from m_fgc */
	gcm = GCClipMask;
	gcv.clip_mask = None;
	XChangeGC(XtD, m_fgc, gcm, &gcv);

	XPSPixmap (m_pm, mncols+2*BORD, mnrows+2*BORD, xe_cm, m_bgc, 1);

	/* add eclipse pen/umbra boundaries and sub points, if enabled */
	if (option[UMBRA_OPT]) {
	    m_umbra();
	    m_sub ();
	}

	/* add the libration marker */
	m_mark_libr ();

	/* add grid, if enabled */
	if (option[GRID_OPT])
	    m_grid();

	/* add any desired feature labels */
	m_labels();

	/* add orientation markings */
	m_orientation();

	/* user annotation */
	ano_draw (mda_w, m_pm, m_ano, 0);
}

/* convert relative moon center to/from X Windows coords depending on w2x.
 * return whether visible.
 */
static int
m_ano (double *fracx, double *fracy, int *xp, int *yp, int w2x, int arg)
{
	if (w2x) {
	    *xp = (int)floor(FX(*fracx*moonrad+leftmarg+moonrad) + BORD + 0.5);
	    *yp = (int)floor(FY(*fracy*moonrad+topmarg+moonrad)  + BORD + 0.5);
	} else {
	    *fracx = (double)(FX(*xp - BORD) - (leftmarg+moonrad))/moonrad;
	    *fracy = (double)(FY(*yp - BORD) -  (topmarg+moonrad))/moonrad;
	}

	return (1);
}

/* label the database entries marked for name display */
static void
m_labels()
{
	MoonDB *mp;

	XSetFont (XtD, m_agc, m_fsp->fid);

	for (mp = moondb; mp < &moondb[nmoondb]; mp++)
	    if (m_wantlabel(mp)) {
		char *n = mp->name;
		int l = strlen (n);
		int x = FX(mp->x)+BORD;
		int y = FY(mp->y)+BORD;

		if (strstr (mp->type, "Landing")) {
		    XPSDrawArc (XtD, m_pm, m_agc, x-2, y-2, 5, 5, 0, 360*64);
		    XPSDrawString (XtD, m_pm, m_agc, x, y-5, n, l);
		} else 
		    XPSDrawString (XtD, m_pm, m_agc, x, y, n, l);
	    }
}

/* add background sky objects to m_pm and to skyobjs[] list */
static void
m_sky()
{
	static int before;
	Now *np = mm_get_now();
	double scale;		/* image scale, rads per pixel */
	double mra, mdec;	/* moon's ra and dec */
	double cmdec;		/* cos mdec */
	double mlat, mlng;	/* moon's ecliptic coords */
	double cmlat;		/* cos mlat*/
	double maxr;		/* max dst from center we want to draw, rads */
	DBScan dbs;
	Obj *moonop;
	Obj *op;

	if (!before && pref_get(PREF_EQUATORIAL) == PREF_GEO) {
	    xe_msg (1, "Equatorial preference should probably be set to Topocentric");
	    before = 1;
	}

	/* get current moon info and derive scale, etc */
	moonop = db_basic (MOON);
	db_update (moonop);
	mra = moonop->s_ra;
	mdec = moonop->s_dec;
	cmdec = cos(mdec);
	eq_ecl (mjd, mra, mdec, &mlat, &mlng);
	cmlat = cos(mlat);
	scale = (degrad(moonop->s_size/3600.0)/2.0) / moonrad;
	maxr = 2*(sqrt((double)mnrows*mnrows + mncols*mncols) + BORD)*scale;

	int w = mncols + 2 * BORD;
	int h = mnrows + 2 * BORD;
	double hfov = w * scale;
	double vfov = h * scale;

	/* load field stars */
	m_loadfs (np, mra, mdec);

	/* scan the database and draw whatever is near */
	for (db_scaninit(&dbs, ALLM, fstars, nfstars);
					    (op = db_scan (&dbs)) != NULL; ) {

	    double ra, dec;	/* object's ra/dec */
	    double olat, olng;	/* object's ecliptic lat/lng */
	    double dlat, dlng;	/* diff from moon's */
	    int dx, dy;
	    int x, y;
	    int diam;
	    GC gc;

	    if (is_planet (op, MOON)) {
		/* we draw it elsewhere :-) */
		continue;
	    }

	    db_update (op);
	    ra = op->s_ra;
	    dec = op->s_dec;

	    /* find size, in pixels. */
	    diam = magdiam (FMAG, 2, scale, get_mag(op),
					    degrad(op->s_size/3600.0));
	    /* reject if too faint */
	    if (diam <= 0)
		continue;

	    /* or if it's obviously outside field of view */
	    if (fabs(mdec - dec) > maxr || delra(mra - ra)*cmdec > maxr)
		continue;

	    /* ecliptic coords match moons tilt far better than equatorial */
	    eq_ecl (mjd, ra, dec, &olat, &olng);

	    /* find [x,y] relative to image center */
	    dlat = mlat - olat;
	    dlng = mlng - olng;
	    dx = (int)floor((dlng*cmlat)/scale + 0.5);	/* + right */
	    dy = (int)floor(dlat/scale + 0.5);		/* + down */

	    /* allow for flipping and shift to find window coords */
	    x = FX(dx+mncols/2) + BORD;
	    y = FY(dy+mnrows/2) + BORD;

	    /* pick a gc */
	    obj_pickgc(op, toplevel_w, &gc);

	    /* draw 'er */
	    sv_draw_obj_x (XtD, m_pm, gc, op, x, y, diam, 1, option[FLIPTB_OPT], option[FLIPLR_OPT], 0, 1, mdec, mra, vfov, hfov, w, h);

	    /* add to skyobjs[] list */
	    addSkyObj (op, x, y);
	}

	sv_draw_obj (XtD, m_pm, (GC)0, NULL, 0, 0, 0, 0);	/* flush */
}

/* load field stars around the given location, unless current set is
 * already close enough.
 */
static void
m_loadfs (np, ra, dec)
Now *np;
double ra, dec;
{

	if (fstars && fabs(dec-fsdec)<FSMOVE && cos(dec)*delra(ra-fsra)<FSMOVE)
	    return;

	if (fstars) {
	    free ((void *)fstars);
	    fstars = NULL;
	    nfstars = 0;
	}

        nfstars = fs_fetch (np, ra, dec, FSFOV, FSMAG, &fstars);

	if (nfstars > 0) {
            xe_msg (0, "Moon View added %d field stars", nfstars);
	    fsdec = dec;
	    fsra = ra;
	}
}

/* given a MoonDB entry, return 1 if we want to show its label now, else 0. */
static int
m_wantlabel (mp)
MoonDB *mp;
{
	/* always show if marked as persistent and labels are on */
	if (option[LABELS_OPT] && mp->pl)
	    return (1);

	/* or if want spacecraft and it appears to be one and
	 * we are at full size or this is an Apollo site
	 */
	if (option[SPACECRAFT_OPT] && strstr (mp->type, "Landing")
		    && (mshrink == 1 || strstr (mp->name, "Apollo")))
	    return (1);

	return (0);
}

/* draw the N/S E/W labels on the four edges of the m_pm
 * we are showing lunar coords.
 */
static void
m_orientation()
{
	int fw, fa, fd;
	int dir, asc, des;
	XCharStruct xcs;

	XQueryTextExtents (XtD, m_fsp->fid, "W", 1, &dir, &asc, &des, &xcs);
	fw = xcs.width;
	fa = xcs.ascent;
	fd = xcs.descent;

	XSetFont (XtD, m_agc, m_fsp->fid);
	XPSDrawString (XtD, m_pm, m_agc, BORD+(mncols-fw)/2, BORD-fd-LGAP,
					    option[FLIPTB_OPT] ? "S" : "N", 1);
	XPSDrawString (XtD, m_pm, m_agc, BORD+(mncols-fw)/2,BORD+mnrows+fa+LGAP,
					    option[FLIPTB_OPT] ? "N" : "S", 1);
	XPSDrawString (XtD, m_pm, m_agc, BORD-fw-LGAP, BORD+(mnrows+fa)/2,
					    option[FLIPLR_OPT] ? "E" : "W", 1);
	XPSDrawString (XtD, m_pm, m_agc, BORD+mncols+LGAP, BORD+(mnrows+fa)/2,
					    option[FLIPLR_OPT] ? "W" : "E", 1);
}

/* draw a coordinate grid over the image moon on m_pm */
static void
m_grid()
{
#define	GSP	degrad(15.0)	/* grid spacing */
#define	FSP	(GSP/4.)	/* fine spacing */
	Display *dsp = XtDisplay (mda_w);
	Window win = m_pm;
	double lt, lg;
	int x, y;

	/* lines of equal lat */
	for (lt = PI/2 - GSP; lt >= -PI/2 + GSP; lt -= GSP) {
	    XPoint xpt[(int)(PI/FSP)+1];
	    int npts = 0;

	    for (lg = -PI/2; lg <= PI/2; lg += FSP) {
		if (ll2xy(lt, lg, &x, &y) < 0)
		    continue;
		if (npts >= XtNumber(xpt)) {
		    printf ("Moon lat grid overflow\n");
		    abort();
		}
		xpt[npts].x = FX(x) + BORD;
		xpt[npts].y = FY(y) + BORD;
		npts++;
	    }
	    XPSDrawLines (dsp, win, m_agc, xpt, npts, CoordModeOrigin);
	}

	/* lines of equal longitude */
	for (lg = -PI/2; lg <= PI/2; lg += GSP) {
	    XPoint xpt[(int)(PI/FSP)+1];
	    int npts = 0;

	    for (lt = -PI/2; lt <= PI/2; lt += FSP) {
		if (ll2xy(lt, lg, &x, &y) < 0)
		    continue;
		if (npts >= XtNumber(xpt)) {
		    printf ("Moon lng grid overflow\n");
		    abort();
		}
		xpt[npts].x = FX(x) + BORD;
		xpt[npts].y = FY(y) + BORD;
		npts++;
	    }
	    XPSDrawLines (dsp, win, m_agc, xpt, npts, CoordModeOrigin);
	}
}

/* draw an X at the subearth and a circle at the subsolar spot using minfo */
static void
m_sub ()
{
#define	SUBR	4
	Display *dsp = XtDisplay (mda_w);
	Window win = m_pm;
	int x, y;

	/* subearth point */
	if (ll2xy (minfo.llat, minfo.llong, &x, &y) == 0) {
	    x = FX(x) + BORD;
	    y = FY(y) + BORD;
	    XPSDrawLine (dsp, win, m_agc, x-SUBR, y-SUBR, x+SUBR, y+SUBR);
	    XPSDrawLine (dsp, win, m_agc, x-SUBR, y+SUBR, x+SUBR, y-SUBR);
	}

	/* subsolar point -- open circle */
	if (ll2xy (minfo.sslat, minfo.srlng + PI/2, &x, &y) == 0) {
	    x = FX(x) + BORD;
	    y = FY(y) + BORD;
	    XPSDrawArc (dsp, win, m_agc, x-SUBR/2, y-SUBR/2,SUBR,SUBR,0,360*64);
	}

	/* anti-subsolar point -- filled circle */
	if (ll2xy (-minfo.sslat, minfo.srlng - PI/2, &x, &y) == 0) {
	    x = FX(x) + BORD;
	    y = FY(y) + BORD;
	    XPSFillArc (dsp, win, m_agc, x-SUBR/2, y-SUBR/2,SUBR,SUBR,0,360*64);
	}
}

/* create m_xim of size mnrowsXmncols, depth mdepth and bit-per-pixel mbpp.
 * make a Bitmap if only have 1 bit per pixel, otherwise a Pixmap.
 * return 0 if ok else -1 and xe_msg().
 */
static int
mxim_create ()
{
	Display *dsp = XtDisplay (mda_w);
	int nbytes = (mnrows+7)*(mncols+7)*mbpp/8;
	char *data;

	/* get memory for image pixels.  */
	data = (char *) malloc (nbytes);
	if (!data) {
	    xe_msg(1, "Can not get %d bytes for shadow pixels", nbytes);
	    return (-1);
	}

	/* create the XImage */
	m_xim = XCreateImage (dsp, XDefaultVisual (dsp, DefaultScreen(dsp)),
	    /* depth */         mbpp == 1 ? 1 : mdepth,
	    /* format */        mbpp == 1 ? XYBitmap : ZPixmap,
	    /* offset */        0,
	    /* data */          data,
	    /* width */         mncols,
	    /* height */        mnrows,
	    /* pad */           mbpp < 8 ? 8 : mbpp,
	    /* bpl */           0);
	if (!m_xim) {
	    xe_msg (1, "Can not create shadow XImage");
	    free ((void *)data);
	    return (-1);
	}

        m_xim->bitmap_bit_order = LSBFirst;
	m_xim->byte_order = LSBFirst;

	/* ok */
	return (0);
}

/* compute a scene in m_xim, knowledge of the sunrise longitude and
 * earthshine.
 */
static void
image_setup ()
{
	double esrlng = minfo.srlng-CLNG; /* correct for earth viewpoint */
	int right = cos(esrlng) < 0;	  /*whther shadw reaches to right limb*/
	double sinsrl = sin(esrlng);
	unsigned char *mp;
	int x, y;
	int esf;

	get_something (eshine_w, XmNvalue, (XtArgVal)&esf);

	/* copy intensities through mgray[] map to get pixels */
	for (y = 0; y < mnrows; y++) {
	    int lx, rx;		/* left and right edge of scan line to darken */
	    int fy;		/* (possibly) flipped y */

	    (void) m_esedges (y, sinsrl, right, &lx, &rx);
	    fy = FY(y);

	    /* scan across the whole row, drawing shadow between lx and rx */
	    mp = &mimage[fy*mncols];
	    for (x = 0; x < mncols; x++) {
		int i = (int)(*mp++);
		int v;

		if (x >= lx && x <= rx)
		    v = i*nmgray*esf/ESHINEF/256; /* shadow */
		else
		    v = i*nmgray/256;

		XPutPixel (m_xim, x, fy, mgray[v]);
	    }
	}
}

/* compute the left and right edge of earthshine for the given y.
 * all coords wrt to m_xim and current orientation.
 * sinsrl is sin of longitude of the rising sun.
 */
static int
m_esedges (y, sinsrl, right, lxp, rxp)
int y;		/* image y */
double sinsrl;	/* sin of longitude of the rising sun */
int right;	/* whether shadow reaches to right limb */
int *lxp, *rxp;	/* left and right x of edges of earthshine */
{
	int lx, rx;	/* left and right edge of scan line to darken */
	int yc;		/* y with respect to moon center */
	int ret;	/* return value */

	yc = y - (topmarg + moonrad);

	/* find left and right edge of shadow */
	if (abs(yc) <= moonrad) {
	    int r;		/* pixels to limb at this y */

	    r = (int)(sqrt((double)(moonrad*moonrad - yc*yc))+1); /* round up */

	    /* compute shadow edges with respect to center */
	    if (right) {
		rx = r;
		lx = -(int)(r * sinsrl);
	    } else {
		lx = -r;
		rx = (int)(r * sinsrl);
	    }

	    /* convert to X coords */
	    lx += leftmarg + moonrad;
	    rx += leftmarg + moonrad;

	    ret = 0;
	} else {
	    /* above or below moon image so no shadow */
	    lx = -1;
	    rx = mncols;
	    ret = -1;
	}

	/* allow for flipping */
	if (option[FLIPLR_OPT]) {
	    int tmp = lx;
	    lx = FX(rx);
	    rx = FX(tmp);
	}

	*lxp = lx;
	*rxp = rx;

	return (ret);
}

/* create a clipping mask for applying m_xim to m_pm */
static void
m_clip_mask_setup ()
{
	if (!m_clip_mask)
	{
	    XGCValues gcv;
	    Window win = RootWindow(XtD, DefaultScreen(XtD));
	    GC sky_gc, moon_gc;

	    unsigned char *mp;
	    int x, y;

	    m_clip_mask = XCreatePixmap(XtD, win, mncols, mnrows, 1);

	    /* setup clipping mask for m_fgc */
	    gcv.foreground = 0;
	    sky_gc = XCreateGC(XtD, m_clip_mask, GCForeground, &gcv);
	    gcv.foreground = 1;
	    moon_gc = XCreateGC(XtD, m_clip_mask, GCForeground, &gcv);

	    XFillRectangle(XtD, m_clip_mask, moon_gc, 0, 0, mncols, mnrows);

	    for (y = 0; y < mnrows; y++) {
		/* scan across the whole row */
		mp = &mimage[y*mncols];
		for (x = 0; x < mncols; x++) {
		    int i = (int)(*mp++);

		    if (!i)
			XDrawPoint (XtD, m_clip_mask, sky_gc, x, y);
		}
	    }
	    XFreeGC(XtD, sky_gc);
	    XFreeGC(XtD, moon_gc);
	}
}

/* delete clipping mask so new one will get built when needed */
static void
m_free_clip_mask ()
{
	if (m_clip_mask) {
	    XFreePixmap (XtD, m_clip_mask);
	    m_clip_mask = 0;
	}
}

/* go through database and set screen loc given lat/long */
static void
m_dbloc()
{
	MoonDB *mp;

	for (mp = moondb; mp < &moondb[nmoondb]; mp++)
	    ll2xy (mp->lt, mp->lg, &mp->x, &mp->y);
}

/* do everything necessary to see a fresh image, shrunk by factor f:
 * read full moon file; shrink by a factor of f; replace mimage with result;
 * resize mda_w and m_xim; recompute image [x,y] values match new scale.
 * when done, set mimage, m_xim, mncols, mnrows, topmarg, leftmarg and moonrad.
 * return 0 if all ok, else xe_msg() and return -1.
 */
static int
m_shrink (f)
int f;
{
	char msg[1024];
	char fn[1024];
	FImage moonfits;
	unsigned char *newim, *im;
	int imnr, imnc;
	int newnr, newnc;
	int v;
	int fd;

	/* open moon image */
	(void) sprintf (fn, "%s/auxil/moon.fts",  getShareDir());
	fd = openh (fn, 0);
	if (fd < 0) {
	    xe_msg (1, "%s: %s\n", fn, syserrstr());
	    return (-1);
	}

	/* read moon file into moonfits */
	if (readFITS (fd, &moonfits, msg) < 0) {
	    xe_msg (1, "%s: %s", fn, msg);
	    (void) close (fd);
	    return (-1);
	}
	(void) close (fd);

	/* make sure it's the new flipped version */
	if (getIntFITS (&moonfits, "XEVERS", &v) < 0 || v != XEVERS) {
	    xe_msg (1, "%s: Incorrect version", fn);
	    return (-1);
	}

	/* make some local shortcuts */
	im = (unsigned char *) moonfits.image;
	imnr = moonfits.sh;
	imnc = moonfits.sw;

	/* make some sanity checks */

	if (moonfits.bitpix != 8 || imnr != MNROWS || imnc != MNCOLS) {
	    xe_msg (1, "%s: Expected %d x %d but found %d x %d",
						fn, MNROWS, MNCOLS, imnr, imnc);
	    resetFImage (&moonfits);
	    return (-1);
	}

	/* set newim to resized version of im (it _is_ im if f is just 1) */
	if (f != 1) {
	    unsigned char *newmem;

	    newnr = imnr / f;
	    newnc = imnc / f;

	    /* get memory for resized copy */
	    newmem = (unsigned char *) malloc (newnr * newnc);
	    if (!newmem) {
		xe_msg (1, "No mem for 1/%dX %d x %d -> %d x %d resize",
						f, imnr, imnc, newnr, newnc);
		free ((void *)im);
		return (-1);
	    }

	    m_resize (im, MNROWS, MNCOLS, f, newmem);
	    newim = newmem;
	    free ((void *)im);
	} else {
	    newim = im;
	    newnr = MNROWS;
	    newnc = MNCOLS;
	}

	/* commit newim to mimage -- set global metrics */
	if (mimage) {
	    free ((void *)mimage);
	    mimage = NULL;
	}
	mimage = newim;
	mnrows = newnr;
	mncols = newnc;
	topmarg = (int)((double)TOPMARG/f + 0.5);
	leftmarg = (int)((double)LEFTMARG/f + 0.5);
	moonrad = (int)((double)MOONRAD/f + 0.5);

	/* dither mimage if we only have 2 colors to work with */
	if (mbpp == 1) 
	    mBWdither();

	/* flip mimage, as desired */
	if (option[FLIPTB_OPT])
	    fliptb();
	if (option[FLIPLR_OPT])
	    fliplr();

	/* (re)create the X image */
	if (m_xim) {
	    free ((void *)m_xim->data);
	    m_xim->data = NULL;
	    XDestroyImage (m_xim);
	    m_xim = NULL;
	}
	if (mxim_create() < 0) {
	    resetFImage (&moonfits);
	    mimage = NULL;
	    return (-1);
	}

	/* delete clip_mask so new one will get built when needed */
	m_free_clip_mask ();

	/* delete pixmap so new one will get built on next expose */
	if (m_pm) {
	    XFreePixmap (XtD, m_pm);
	    m_pm = 0;
	}

	/* recompute location of db on image at this scale */
	m_dbloc();

	/* size the drawing area to hold the new image plus a border.
	 * this will give us an expose event to show the new image.
	 * also center the scrollbars.
	 */
	m_sizewidgets ();
	centerScrollBars(msw_w);

	return(0);
}

/* set size of mda_w as desired based on mnrows and mncols */
static void
m_sizewidgets ()
{
	int neww = mncols + 2*BORD;
	int newh = mnrows + 2*BORD;

	/* mda_w should be image size + BORD all around */
	set_something (mda_w, XmNwidth, (XtArgVal)neww);
	set_something (mda_w, XmNheight, (XtArgVal)newh);
}

/* copy image in, of size nr x nr, to out by shrinking a factor f.
 * N.B. we assume out is (nr/f) * (nr/f) bytes.
 */
static void
m_resize(in, nr, nc, f, out)
unsigned char *in, *out;
int nr, nc;
int f;
{
	int outx, outy;
	int noutx, nouty;

	noutx = nc/f;
	nouty = nr/f;
	for (outy = 0; outy < nouty; outy++) {
	    unsigned char *inrp = &in[outy*f*nr];
	    for (outx = 0; outx < noutx; outx++) {
		*out++ = *inrp;
		inrp += f;
	    }
	}
}

/* dither mimage into a 2-intensity image: 0 and 255.
 * form 2x2 tiles whose pattern depends on intensity peak and spacial layout.
 */
static void
mBWdither()
{
	int idx[4];
	int y;

	idx[0] = 0;
	idx[1] = 1;
	idx[2] = mncols;
	idx[3] = mncols+1;

	for (y = 0; y < mnrows - 1; y += 2) {
	    unsigned char *mp = &mimage[y*mncols];
	    unsigned char *lp;

	    for (lp = mp + mncols - 1; mp < lp; mp += 2) {
		int sum, numon;
		int i;

		sum = 0;
		for (i = 0; i < 4; i++)
		    sum += (int)mp[idx[i]];
		numon = sum*5/1021;	/* 1021 is 255*4 + 1 */

		for (i = 0; i < 4; i++)
		    mp[idx[i]] = 0;

		switch (numon) {
		case 0:
		    break;
		case 1:
		case 2:
		    mp[idx[0]] = 255;
		    break;
		case 3:
		    mp[idx[0]] = 255;
		    mp[idx[1]] = 255;
		    mp[idx[3]] = 255;
		    break;
		case 4:
		    mp[idx[0]] = 255;
		    mp[idx[1]] = 255;
		    mp[idx[2]] = 255;
		    mp[idx[3]] = 255;
		    break;
		default:
		    printf ("Bad numon: %d\n", numon);
		    abort();
		}
	    }
	}
}

/* draw umbra and penumbra boundaries */
static void
m_umbra()
{
	Now *np = mm_get_now();
	Obj *mop = db_basic (MOON);
	Obj *sop = db_basic (SUN);
	double scale;		/* image scale, rads per pixel */
	double mrad;		/* moon radius, rads */
	double ara, adec;	/* anti-solar loc, equatorial coords, rads */
	double alat, alng;	/* anti-solar loc, ecliptic coords, rads */
	double mlat, mlng;	/* moon loc, ecliptic coords, rads */
	double dlat, dlng;	/* difference, rads */
	double cmlat;		/* cos moon's lat */
	double sed, med;	/* sun-earth and moon-earth dist, au */
	double u, p;		/* umbra and penumbra radius, m at moon dist*/
	double maxr;
	int dx, dy;
	int x, y, r;

	/* get basic info */
	db_update (mop);
	db_update (sop);
	mrad = degrad(mop->s_size/3600.0)/2.0;
	scale = mrad / moonrad;
	ara = sop->s_gaera + PI;
	adec = -sop->s_gaedec;
	eq_ecl (mjd, ara, adec, &alat, &alng);
	eq_ecl (mjd, mop->s_gaera, mop->s_gaedec, &mlat, &mlng);
	dlat = mlat - alat;
	dlng = mlng - alng;
	cmlat = cos (mlat);

	/* skip if obviously outside fov */
	maxr = 2*(sqrt((double)mnrows*mnrows + mncols*mncols) + BORD)*scale;
	if (fabs(dlat) > maxr || delra(dlng)*cmlat > maxr)
	    return;

	/* find center of circle */
	dx = (int)floor((dlng*cmlat)/scale + 0.5);	/* + right */
	dy = (int)floor(dlat/scale + 0.5);		/* + down */
	x = FX(dx+mncols/2) + BORD;
	y = FY(dy+mnrows/2) + BORD;

	/* compute diameters at moon (based on similar triangles) */
	med = mop->s_edist;
	sed = sop->s_edist;
	u = ERAD - med/sed*(SRAD-ERAD);
	r = (int)floor(u/MRAD*moonrad + 0.5);
	XPSDrawArc (XtD, m_pm, m_fgc, x-r, y-r, 2*r, 2*r, 0, 360*64);
	p = ERAD + med/sed*(SRAD-ERAD);
	r = (int)floor(p/MRAD*moonrad + 0.5);
	XSetLineAttributes (XtD, m_fgc, 0, LineOnOffDash, CapButt, JoinMiter);
	XPSDrawArc (XtD, m_pm, m_fgc, x-r, y-r, 2*r, 2*r, 0, 360*64);
	XSetLineAttributes (XtD, m_fgc, 0, LineSolid, CapButt, JoinMiter);

	/* tried to draw a line along path by computing ecl loc +- a few
	 * hours from now but doesn't work since the perspective changes.
	 */
}

/* flip mimage left/right */
static void
fliplr()
{
	int x, y;

	for (y = 0; y < mnrows; y++) {
	    unsigned char *rp = &mimage[y*mncols];
	    for (x = 0; x < mncols/2; x++) {
		unsigned char tmp = rp[x];
		rp[x] = rp[mncols-x-1];
		rp[mncols-x-1] = tmp;
	    }
	}
}

/* flip mimage top/bottom.
 * N.B. will flip back option if can't do it for some reason.
 */
static void
fliptb()
{
	char buf[2048];		/* plenty :-) */
	int y;

	if (mncols > sizeof(buf)) {
	    xe_msg (1, "Can not flip -- rows are longer than %ld",
							    (long)sizeof(buf));
	    XmToggleButtonSetState (option_w[FLIPTB_OPT],
						option[FLIPTB_OPT] ^= 1, False);
	    return;
	}
	    

	for (y = 0; y < mnrows/2; y++) {
	    unsigned char *r0 = &mimage[y*mncols];
	    unsigned char *r1 = &mimage[(mnrows-y-1)*mncols];

	    (void) memcpy (buf, (void *)r0, mncols);
	    (void) memcpy ((void *)r0, (void *)r1, mncols);
	    (void) memcpy ((void *)r1, buf, mncols);
	}
}

/* draw a marker on m_pm to show the limb angle favored by libration */
static void
m_mark_libr ()
{
	int r;		/* radius of marker */
	int x, y;	/* center of marker */

	r = moonrad/LMFRAC;
	x = leftmarg + (int)(moonrad*(1 - sin(minfo.limb)));
	y = topmarg  + (int)(moonrad*(1 - cos(minfo.limb)));

	x = FX(x);
	y = FY(y);

	XPSFillArc (XtD, m_pm, m_agc, BORD+x-r, BORD+y-r, 2*r, 2*r, 0, 360*64);
}

/* report the location of x,y, which are with respect to mda_w.
 * N.B. allow for flipping and the borders.
 */
static void
m_reportloc (dsp, win, x, y)
Display *dsp;
Window win;
int x, y;
{
	double lt, lg;

	/* convert from mda_w coords to m_xim coords */
	x -= BORD;
	y -= BORD;

	if (xy2ll (FX(x), FY(y), &lt, &lg) == 0) {
	    Now *np = mm_get_now();

	    f_showit (infot_w, "User cursor:");
	    minfo.curlt = lt;
	    minfo.curlg = lg;
	    m_info (np);
	    m_pinfo ();

	} else {
	    set_xmstring (lat_w, XmNlabelString, " ");
	    set_xmstring (lng_w, XmNlabelString, " ");
	    set_xmstring (sunalt_w, XmNlabelString, " ");
	    set_xmstring (nsrd_w, XmNlabelString, " ");
	    set_xmstring (nsrt_w, XmNlabelString, " ");
	    set_xmstring (nssd_w, XmNlabelString, " ");
	    set_xmstring (nsst_w, XmNlabelString, " ");
	}
}

/* print minfo */
static void
m_pinfo()
{
	f_dm_angle (lat_w, minfo.curlt);
	f_dm_angle (lng_w, minfo.curlg);
	f_dm_angle (sunalt_w, minfo.cursunalt);
	f_date (nsrd_w, mjd_day(minfo.curnsrt));
	f_mtime (nsrt_w, mjd_hr(minfo.curnsrt));
	f_date (nssd_w, mjd_day(minfo.curnsst));
	f_mtime (nsst_w, mjd_hr(minfo.curnsst));

	/* update cursor indep info - always on */
	f_dm_angle (srlng_w, minfo.srlng);
	f_dm_angle (sslat_w, minfo.sslat);
	f_dm_angle (llat_w, minfo.llat);
	f_dm_angle (llong_w, minfo.llong);
	f_dm_angle (limb_w, minfo.limb);
	f_double (lib_w, "%6.3f", minfo.tilt);
	f_double (age_w, "%6.3f", minfo.age);
}

/* make glass_xim of size GLASSSZ*GLASSMAG and same genre as m_xim.
 * leave glass_xim NULL if trouble.
 */
static void
makeGlassImage (dsp)
Display *dsp;
{
	int nbytes = (GLASSSZ*GLASSMAG+7) * (GLASSSZ*GLASSMAG+7) * mbpp/8;
	char *glasspix = (char *) malloc (nbytes);

	if (!glasspix) {
	    xe_msg (0, "Can not malloc %d for Glass pixels", nbytes);
	    return;
	}

	glass_xim = XCreateImage (dsp, XDefaultVisual (dsp, DefaultScreen(dsp)),
	    /* depth */         m_xim->depth,
	    /* format */        m_xim->format,
	    /* offset */        0,
	    /* data */          glasspix,
	    /* width */         GLASSSZ*GLASSMAG,
	    /* height */        GLASSSZ*GLASSMAG,
	    /* pad */           mbpp < 8 ? 8 : mbpp,
	    /* bpl */           0);

	if (!glass_xim) {
	    free ((void *)glasspix);
	    xe_msg (0, "Can not make Glass XImage");
	    return;
	}

        glass_xim->bitmap_bit_order = LSBFirst;
	glass_xim->byte_order = LSBFirst;
}

/* make glassGC */
static void
makeGlassGC (dsp, win)
Display *dsp;
Window win;
{
	XGCValues gcv;
	unsigned int gcm;
	Pixel p;

	(void) get_color_resource (mda_w, "GlassBorderColor", &p);
	gcm = GCForeground;
	gcv.foreground = p;
	glassGC = XCreateGC (dsp, win, gcm, &gcv);
}

/* handle the operation of the magnifying glass.
 * this is called whenever there is left button activity over the image.
 */
static void
doGlass (dsp, win, b1p, m1, b1r, wx, wy)
Display *dsp;
Window win;
int b1p, m1, b1r;	/* button/motion state */
int wx, wy;		/* window coords of cursor */
{
	static int lastwx, lastwy;
	int rx, ry, rw, rh;		/* region */

	/* check for first-time stuff */
	if (!glass_xim)
	    makeGlassImage (dsp);
	if (!glass_xim)
	    return; /* oh well */
	if (!glassGC)
	    makeGlassGC (dsp, win);

	if (m1) {

	    /* motion: put back old pixels that won't just be covered again */

	    /* first the vertical strip that is uncovered */

	    rh = GLASSSZ*GLASSMAG;
	    ry = lastwy - (GLASSSZ*GLASSMAG/2);
	    if (ry < 0) {
		rh += ry;
		ry = 0;
	    }
	    if (wx < lastwx) {
		rw = lastwx - wx;	/* cursor moved left */
		rx = wx + (GLASSSZ*GLASSMAG/2);
	    } else {
		rw = wx - lastwx;	/* cursor moved right */
		rx = lastwx - (GLASSSZ*GLASSMAG/2);
	    }
	    if (rx < 0) {
		rw += rx;
		rx = 0;
	    }

	    if (rw > 0 && rh > 0)
		XCopyArea (dsp, m_pm, win, m_fgc, rx, ry, rw, rh, rx, ry);

	    /* then the horizontal strip that is uncovered */

	    rw = GLASSSZ*GLASSMAG;
	    rx = lastwx - (GLASSSZ*GLASSMAG/2);
	    if (rx < 0) {
		rw += rx;
		rx = 0;
	    }
	    if (wy < lastwy) {
		rh = lastwy - wy;	/* cursor moved up */
		ry = wy + (GLASSSZ*GLASSMAG/2);
	    } else {
		rh = wy - lastwy;	/* cursor moved down */
		ry = lastwy - (GLASSSZ*GLASSMAG/2);
	    }
	    if (ry < 0) {
		rh += ry;
		ry = 0;
	    }

	    if (rw > 0 && rh > 0)
		XCopyArea (dsp, m_pm, win, m_fgc, rx, ry, rw, rh, rx, ry);
	}

	if (b1p || m1) {

	    /* start or new location: show glass and save new location */

	    fillGlass (wx-BORD, wy-BORD);
	    XPutImage (dsp, win, m_fgc, glass_xim, 0, 0,
			wx-(GLASSSZ*GLASSMAG/2), wy-(GLASSSZ*GLASSMAG/2),
			GLASSSZ*GLASSMAG, GLASSSZ*GLASSMAG);
	    lastwx = wx;
	    lastwy = wy;

	    /* kinda hard to tell boundry of glass so draw a line around it */
	    XDrawRectangle (dsp, win, glassGC,
			wx-(GLASSSZ*GLASSMAG/2), wy-(GLASSSZ*GLASSMAG/2),
			GLASSSZ*GLASSMAG-1, GLASSSZ*GLASSMAG-1);
	}

	if (b1r) {

	    /* end: restore all old pixels */

	    rx = lastwx - (GLASSSZ*GLASSMAG/2);
	    rw = GLASSSZ*GLASSMAG;
	    if (rx < 0) {
		rw += rx;
		rx = 0;
	    }

	    ry = lastwy - (GLASSSZ*GLASSMAG/2);
	    rh = GLASSSZ*GLASSMAG;
	    if (ry < 0) {
		rh += ry;
		ry = 0;
	    }

	    if (rw > 0 && rh > 0)
		XCopyArea (dsp, m_pm, win, m_fgc, rx, ry, rw, rh, rx, ry);
	}
}

/* fill glass_xim with GLASSSZ*GLASSMAG view of m_xim centered at coords
 * xc,yc. take care at the edges (m_xim is mnrows x mncols)
 */
static void
fillGlass (xc, yc)
int xc, yc;
{
	int sx, sy;	/* coords in m_xim */
	int gx, gy;	/* coords in glass_xim */
	int i, j;

	gy = 0;
	gx = 0;
	for (sy = yc-GLASSSZ/2; sy < yc+GLASSSZ/2; sy++) {
	    for (sx = xc-GLASSSZ/2; sx < xc+GLASSSZ/2; sx++) {
		Pixel p;

		if (sx < 0 || sx >= mncols || sy < 0 || sy >= mnrows)
		    p = XGetPixel (m_xim, 0, 0);
		else
		    p = XGetPixel (m_xim, sx, sy);
		for (i = 0; i < GLASSMAG; i++)
		    for (j = 0; j < GLASSMAG; j++)
			XPutPixel (glass_xim, gx+i, gy+j, p);
		gx += GLASSMAG;
	    }
	    gx = 0;
	    gy += GLASSMAG;
	}
}

/* read Moon database: fill up moodb[] and set nmoondb.
 * N.B. to fill in x,y we assume the lat/long match the image orientation in
 *   its original form, ie, up N right E.
 * if fail, leave moondb NULL.
 */
static void
m_readdb()
{
#define	NDBCHUNKS	32	/* malloc room for these many more each time */
	LilXML *xp;
	XMLEle *root, *ep;
	char buf[1024];
	char fn[1024];
	int ndb;
	FILE *fp;

	/* open file */
	(void) sprintf (fn, "%s/lo/lodb.xml",  getShareDir());
	fp = fopenh (fn, "r");
	if (!fp) {
	    xe_msg (1, "%s: %s", fn , syserrstr());
	    return;
	}

	/* reset moondb[] */
	m_freedb();
	ndb = 0;

	watch_cursor (1);

	/* prep and read the data file */
	xp = newLilXML();
	root = readXMLFile (fp, xp, buf);
	fclose (fp);
	delLilXML (xp);
	if (!root) {
	    xe_msg (1, "%s:\n%s", fn, buf);
	    return;
	}
	if (strcmp (tagXMLEle(root), "XEphemLunarDB")) {
	    xe_msg (1, "%s:\nNot XEphem Lunar database", fn);
	    delXMLEle (root);
	    return;
	}

	/* read each feature and add each to list */
	for (ep = nextXMLEle (root, 1); ep != NULL; ep = nextXMLEle(root,0)) {
	    XMLEle *fe;
	    MoonDB *mp;
	    char *type;
	    int x, y;
	    int i;

	    /* confirm feature tag */
	    if (strcmp (tagXMLEle(ep), "feature"))
		continue;

	    /* grow moondb if now filled */
	    if (nmoondb == ndb) {
		moondb = (MoonDB *) XtRealloc ((char *)moondb,
					    (ndb+NDBCHUNKS)*sizeof(MoonDB));
		ndb += NDBCHUNKS;
	    }
	    mp = &moondb[nmoondb];
	    memset (mp, 0, sizeof(*mp));

	    /* build new entry at mp */

	    /* lat/long location, x/y gets set once we know the image scale */
	    mp->lt = degrad(atof(findXMLAttValu(ep, "lat")));
	    mp->lg = degrad(atof(findXMLAttValu(ep, "long")));
	    if (ll2xy (mp->lt, mp->lg, &x, &y) < 0)
		continue;	/* far side */

	    /* copy name */
	    mp->name = XtNewString(findXMLAttValu(ep, "name"));

	    /* copy type, but try to reuse */
	    type = findXMLAttValu(ep, "type");
	    for (i = 0; i < nmoondb; i++)
		if (strcmp (moondb[i].type, type) == 0) {
		    mp->type = moondb[i].type;
		    break;	/* reuse type string */
		}
	    if (i == nmoondb) {
		/* ok, first time we've seen this type */
		mp->type = XtNewString (type);
	    }

	    /* add each file basename */
	    for (fe = nextXMLEle (ep, 1); fe != NULL; fe = nextXMLEle(ep,0)) {
		if (strcmp(tagXMLEle(fe), "file"))
		    continue;
		mp->lofiles = (char **) XtRealloc ((char *)mp->lofiles,
						(mp->nlofiles+1)*sizeof(char*));
		mp->lofiles[mp->nlofiles++] =
				    XtNewString (findXMLAttValu(fe, "base"));
	    }

	    /* initialy not persistent label */
	    mp->pl = 0;

	    /* all ok */
	    nmoondb++;
	}

	/* finished with xml */
	delXMLEle (root);

	/* warn if nothing found */
	if (nmoondb == 0)
	    xe_msg (0, "%s:\nNo moon database entries found", fn);

	/* ok */
	watch_cursor(0);
}

/* compare two pointers to string, qsort-style */
static int
ncmp_qsort (const void *p1, const void *p2)
{
	return (strcmp (*(char**)p1, *(char**)p2));
}

/* fill the scrolled list with db names */
static void
m_fillSL()
{
	char **names;
	int i;

	/* build the list of names in sorted order */
	names = (char **) XtMalloc (nmoondb * sizeof(char *));
	for (i = 0; i < nmoondb; i++)
	    names[i] = moondb[i].name;
	qsort (names, nmoondb, sizeof(char *), ncmp_qsort);

	/* put in scrolled list */
	XmListDeleteAllItems (msl_w);
	for (i = 0; i < nmoondb; i++) {
	    XmString xms = XmStringCreateSimple (names[i]);
	    XmListAddItemUnselected (msl_w, xms, 0);
	    XmStringFree (xms);
	}

	/* finished with sorted list */
	XtFree ((char *)names);
}

/* free moondb[] and the names and types.
 * N.B. beware that type strings may not all be unique.
 * N.B. we allow for moondb being already NULL.
 */
static void
m_freedb()
{
	int i, j;

	if (moondb == NULL)
	    return;

	for (i = 0; i < nmoondb; i++) {
	    MoonDB *mp = &moondb[i];

	    /* free name for sure -- each is unique */
	    XtFree (mp->name);

	    /* free type once -- pointer might be reused */
	    if (mp->type) {
		XtFree (mp->type);
		for (j = i+1; j < nmoondb; j++)
		    if (moondb[j].type == mp->type)
			moondb[j].type = NULL;
	    }

	    /* free lo images */
	    if (mp->lofiles) {
		for (j = 0; j < mp->nlofiles; j++)
		    XtFree (mp->lofiles[j]);
		XtFree ((char *)mp->lofiles);
	    }
	}

	/* now free the array itself */
	XtFree ((char *)moondb);
	moondb = NULL;
	nmoondb = 0;
}

/* free the list of sky objects, if any */
static void
resetSkyObj ()
{
	if (skyobjs) {
	    free ((void *) skyobjs);
	    skyobjs = NULL;
	}
	nskyobjs = 0;
}

/* add op at [x,y] to the list of background sky objects */
static void
addSkyObj (op, x, y)
Obj *op;
int x, y;
{
	char *newmem;

	if (skyobjs)
	    newmem = realloc ((void *)skyobjs, (nskyobjs+1) * sizeof(SkyObj));
	else
	    newmem = malloc (sizeof(SkyObj));
	if (!newmem) {
	    xe_msg (1, "No mem for more sky objects");
	    return;
	}

	skyobjs = (SkyObj *) newmem;
	skyobjs[nskyobjs].op = op;
	skyobjs[nskyobjs].x = x;
	skyobjs[nskyobjs].y = y;
	nskyobjs++;
}

/* find the object in skyobjs[] that is closest to [x,y].
 * return NULL if none within MAXR.
 */
static Obj *
closeSkyObj (x, y)
int x, y;
{
	SkyObj *closesp = NULL;
	int mind = 0;
	int i;

	for (i = 0; i < nskyobjs; i++) {
	    SkyObj *sp = &skyobjs[i];
	    int d = abs(sp->x - x) + abs(sp->y - y);

	    if (!closesp || d < mind) {
		mind = d;
		closesp = sp;
	    }
	}

	if (closesp && mind <= MAXR)
	    return (closesp->op);
	return (NULL);
}

/* called when hit button 3 over image */
static void
m_popup (ep)
XEvent *ep;
{
	XButtonEvent *bep;
	int x, y;

	bep = &ep->xbutton;
	x = bep->x;
	y = bep->y;

	if (overMoon (x, y)) {
	    MoonDB *mp = closeMoonDB (x-BORD, y-BORD);
	    fill_popup (mp, x-BORD, y-BORD);
	    XmMenuPosition (pu_w, (XButtonPressedEvent *)ep);
	    XtManageChild (pu_w);
	} else {
	    Obj *op = closeSkyObj (x, y);
	    if (op) {
		fill_skypopup (op);
		XmMenuPosition (skypu_w, (XButtonPressedEvent *)ep);
		XtManageChild (skypu_w);
	    }
	}
}

/* create the popup menu */
static void
m_create_popup()
{
	Widget w;
	Arg args[20];
	int n;

	n = 0;
	XtSetArg (args[n], XmNisAligned, True); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
	pu_w = XmCreatePopupMenu (mda_w, "MPU", args, n);

	n = 0;
	pu_name_w = XmCreateLabel (pu_w, "MPN", args, n);
	wtip (pu_name_w, "Name of feature");
	XtManageChild (pu_name_w);

	n = 0;
	pu_type_w = XmCreateLabel (pu_w, "MPT", args, n);
	wtip (pu_type_w, "Basic type of feature");
	XtManageChild (pu_type_w);

	n = 0;
	pu_lat_w = XmCreateLabel (pu_w, "MPLat", args, n);
	wtip (pu_lat_w, "Selenographic latitude of this feature");
	XtManageChild (pu_lat_w);

	n = 0;
	pu_lng_w = XmCreateLabel (pu_w, "MPLng", args, n);
	wtip (pu_lng_w, "Selenographic longitude of this feature");
	XtManageChild (pu_lng_w);

	n = 0;
	pu_alt_w = XmCreateLabel (pu_w, "MPAlt", args, n);
	wtip (pu_alt_w, "Altitude of Sun above horizon at this feature");
	XtManageChild (pu_alt_w);

	n = 0;
	w = XmCreateSeparator (pu_w, "MPS", args, n);
	XtManageChild (w);

	/* persistent label TB -- managed as needed */
	n = 0;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
	pu_pl_w = XmCreateToggleButton (pu_w, "MPPL", args, n);
	XtAddCallback (pu_pl_w, XmNvalueChangedCallback, m_pl_cb, 0);
	set_xmstring (pu_pl_w, XmNlabelString, "Label");
	wtip (pu_pl_w, "Whether to label this object on the map.");

	/* copy to Info table PB -- managed as needed */
	n = 0;
	pu_copy_w = XmCreatePushButton (pu_w, "MPC", args, n);
	XtAddCallback (pu_copy_w, XmNactivateCallback, m_copy_cb, 0);
	set_xmstring (pu_copy_w, XmNlabelString, "Set info table");
	wtip (pu_copy_w, "Copy this feature location to the Moon info table");

	/* show LO PB -- managed as needed */
	n = 0;
	pu_showlo_w = XmCreatePushButton (pu_w, "MLO", args, n);
	XtAddCallback (pu_showlo_w, XmNactivateCallback, m_showlo_cb, 0);
	set_xmstring (pu_showlo_w, XmNlabelString, "Lunar Orbiter image");
	wtip (pu_showlo_w, "Show feature in lunar orbiter images");
}

/* fill in popup widgets from mp. if mp == NULL use x,y and skip name.
 * x and y are in image coords, (as in m_xim), not mda_w.
 * N.B. since we assume the database matches the original image orientation
 *   there is no need to correct mp->{x,y} for flipping.
 */
static void
fill_popup (mp, x, y)
MoonDB *mp;
int x, y;
{
	Now *np = mm_get_now();
	double lt, lg;
	double a;
	char str[64];

	/* create popup first time */
	if (!pu_w)
	    m_create_popup();

	if (mp) {
	    XtManageChild (pu_name_w);
	    f_showit (pu_name_w, mp->name);
	    XtManageChild (pu_type_w);
	    f_showit (pu_type_w, mp->type);
	    XtManageChild (pu_pl_w);
	    XmToggleButtonSetState (pu_pl_w, mp->pl, False);
	    set_something (pu_pl_w, XmNuserData, (XtArgVal)mp);
	    XtManageChild (pu_copy_w);
	    set_something (pu_copy_w, XmNuserData, (XtArgVal)mp);
	    lt = mp->lt;
	    lg = mp->lg;
	    if (mlo_installed() && mp->nlofiles > 0) {
		XtManageChild (pu_showlo_w);
		set_something (pu_showlo_w, XmNuserData, (XtArgVal)mp);
	    } else
		XtUnmanageChild (pu_showlo_w);
	} else {
	    XtUnmanageChild (pu_name_w);
	    XtUnmanageChild (pu_type_w);
	    XtUnmanageChild (pu_pl_w);
	    XtUnmanageChild (pu_copy_w);
	    XtUnmanageChild (pu_showlo_w);
	    xy2ll (FX(x), FY(y), &lt, &lg);
	}

	(void) strcpy (str, "Lat: ");
	if (lt < 0) {
	    fs_dm_angle (str+5, -lt);
	    (void) strcat (str, " S");
	} else {
	    fs_dm_angle (str+5, lt);
	    (void) strcat (str, " N");
	}
	f_showit (pu_lat_w, str);

	(void) strcpy (str, "Long: ");
	if (lg > PI) {
	    fs_dm_angle (str+6, 2*PI-lg);
	    (void) strcat (str, " W");
	} else {
	    fs_dm_angle (str+6, lg);
	    (void) strcat (str, " E");
	}
	f_showit (pu_lng_w, str);

	moon_colong (mjd+MJD0, lt, lg, NULL, NULL, &a, NULL);
	(void) strcpy (str, "Sun alt: ");
	fs_dm_angle (str+9, a);
	f_showit (pu_alt_w, str);
}

/* find the closest entry in moondb[] to [x,y].
 * if find one return its *MoonDB else NULL.
 * x and y are in image coords (as in m_xim), not mda_w.
 * N.B. we allow for flipping.
 */
static MoonDB *
closeMoonDB (x, y)
int x, y;
{
	MoonDB *mp, *smallp;
	double lt, lg;		/* location of [x,y] */
	double forsh;		/* foreshortening (cos of angle from center) */
	int minr2;

	/* allow for flipping */
	x = FX(x);
	y = FY(y);

	/* find forshortening */
	if (xy2ll (x, y, &lt, &lg) < 0)
	    return (NULL);
	solve_sphere (lg - CLNG, PI/2-CLAT, sin(lt), cos(lt), &forsh, NULL);

	watch_cursor(1);

	minr2 = 100000000;
	smallp = NULL;
	for (mp = moondb; mp < &moondb[nmoondb]; mp++) {
	    int dx = mp->x - x;
	    int dy = mp->y - y;
	    int r2 = dx*dx + dy*dy;

	    if (r2 < minr2) {
		smallp = mp;
		minr2 = r2;
	    }
	}

	watch_cursor(0);

	return (smallp);
}

/* clicked or double-clicked a name in the main db list */
static void
m_select_cb (Widget w, XtPointer client, XtPointer call)
{
	XmListCallbackStruct *lcb = (XmListCallbackStruct *)call;
	int dblclick = lcb->reason == XmCR_DEFAULT_ACTION;
	XmString sel = lcb->item;
	char *name;
	MoonDB *mp;

	XmStringGetLtoR (sel, XmSTRING_DEFAULT_CHARSET, &name);

	/* toggle label, and display if double-clicked */
	for (mp = moondb; mp < &moondb[nmoondb]; mp++) {
	    if (strcmp (mp->name, name) == 0) {

		if (dblclick) {
		    /* display lunar orbiter image if any */
		    if (mlo_installed()) {
			mlo_load (mp);
			XtPopup (mlo_shell_w, XtGrabNone);
		    }
		} else {
		    /* toggle label */
		    mp->pl ^= 1;
		    if (mp->pl) {
			if (!option[LABELS_OPT]) {
			    /* turn on all labels and draw */
			    XmToggleButtonSetState (option_w[LABELS_OPT],1,1);
			} else {
			    /* draw just this one */
			    m_labels();
			    m_refresh (NULL);
			}
		    } else {
			/* redraw to erase */
			m_redraw();
		    }
		}

		break;
	    }
	}

	XtFree (name);
}

/* called when the persistent label TB is activated in the popup.
 * userData is the MoonDB entry whose pl we toggle, then redraw.
 */
/* ARGSUSED */
static void
m_pl_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	MoonDB *mp;

	get_something (w, XmNuserData, (XtArgVal)&mp);
	mp->pl = XmToggleButtonGetState (w);

	/* draw just labels if adding or redraw all to erase */
	if (mp->pl) {
	    m_labels();
	    m_refresh (NULL);
	} else
	    m_redraw();
}

/* called when to copy the current feature to the moon info table.
 * userData is the MoonDB entry to copy.
 */
/* ARGSUSED */
static void
m_copy_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Now *np = mm_get_now();
	MoonDB *mp;

	get_something (w, XmNuserData, (XtArgVal)&mp);
	xy2ll (mp->x, mp->y, &minfo.curlt, &minfo.curlg);
	m_info (np);
	set_xmstring (infot_w, XmNlabelString, mp->name);
	m_pinfo ();

	/* display too */
	XtManageChild (msform_w);
	m_set_buttons(m_selecting);
}

/* called to display lunar orbiter image from popup.
 * userData is the MoonDB entry to show.
 */
/* ARGSUSED */
static void
m_showlo_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	MoonDB *mp;

	get_something (w, XmNuserData, (XtArgVal)&mp);

	/* if there /is/ an image for this feature... */
	if (mp->nlofiles > 0) {
	    /* display */
	    mlo_load (mp);
	    XtPopup (mlo_shell_w, XtGrabNone);

	    /* set label just to be helpful */
	    mp->pl = 1;
	    m_labels();
	    m_refresh (NULL);
	}
}

/* assign the object pointed to by skypu_op to Favorites
 */
static void
m_assign_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	fav_add (skypu_op);
}

/* create the sky background object popup menu */
static void
m_create_skypopup()
{
	Widget w;
	Arg args[20];
	int n;

	n = 0;
	XtSetArg (args[n], XmNisAligned, True); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
	skypu_w = XmCreatePopupMenu (mda_w, "MSKYPU", args, n);

	n = 0;
	skypu_name_w = XmCreateLabel (skypu_w, "MSKYPN", args, n);
	wtip (skypu_name_w, "Name of object");
	XtManageChild (skypu_name_w);

	n = 0;
	skypu_mag_w = XmCreateLabel (skypu_w, "MSKYPM", args, n);
	wtip (skypu_mag_w, "Nominal magnitude");
	XtManageChild (skypu_mag_w);

	/* sep */
	n = 0;
	w = XmCreateSeparator (skypu_w, "MSKYS", args, n);
	XtManageChild (w);

	/* a PB to add to Favorites */
	n = 0;
	w = XmCreatePushButton (skypu_w, "ABPB", args, n);
	XtAddCallback (w, XmNactivateCallback, m_assign_cb, 0);
	set_xmstring (w, XmNlabelString, "Add to Favorites");
	XtManageChild (w);
}

/* fill skypu_w with info from op */
static void
fill_skypopup (op)
Obj *op;
{
	char buf[32];

	/* create popup first time */
	if (!skypu_w)
	    m_create_skypopup();

	/* save in case it's assigned */
	skypu_op = op;

	/* show name and mag */
	f_showit (skypu_name_w, op->o_name);
	(void) sprintf (buf, "Mag: %.2f", get_mag(op));
	f_showit (skypu_mag_w, buf);
}

/* create the shell to display lunar orbiter images */
static void
mlo_create_shell()
{
	Widget w, credit_w;
	Widget mform_w;
	Arg args[20];
	int n;

	/* create master form */
	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Lunar Orbiter view"); n++;
	XtSetArg (args[n], XmNiconName, "LOrbiter"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	mlo_shell_w = XtCreatePopupShell ("MoonLO", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (mlo_shell_w);
	set_something (mlo_shell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (mlo_shell_w, XmNpopdownCallback, mlo_popdown_cb, 0);
	sr_reg (mlo_shell_w, "XEphem*MoonLO.width", mooncategory, 0);
	sr_reg (mlo_shell_w, "XEphem*MoonLO.height", mooncategory, 0);
	sr_reg (mlo_shell_w, "XEphem*MoonLO.x", mooncategory, 0);
	sr_reg (mlo_shell_w, "XEphem*MoonLO.y", mooncategory, 0);

	n = 0;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	mform_w = XmCreateForm (mlo_shell_w, "MoonLOF", args, n);
	XtManageChild (mform_w);

	/* scrolled list in lower right to display possible file choices */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNvisibleItemCount, 4); n++;
	XtSetArg (args[n], XmNselectionPolicy, XmBROWSE_SELECT); n++;
	mlo_sl_w = XmCreateScrolledList (mform_w, "MLOSL", args, n);
	XtAddCallback (mlo_sl_w, XmNbrowseSelectionCallback, mlo_select_cb,0);
	XtManageChild (mlo_sl_w);

	/* annotation and close buttons */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 15); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 15); n++;
	w = XmCreatePushButton (mform_w, "Close", args, n);
	XtAddCallback (w, XmNactivateCallback, mlo_close_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 15); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 30); n++;
	mlo_ann_w = XmCreateToggleButton (mform_w, "Overlay", args, n);
	XtAddCallback (mlo_ann_w, XmNvalueChangedCallback, mlo_ann_cb, NULL);
	set_xmstring (mlo_ann_w, XmNlabelString, "Overlay annotation");
	sr_reg (mlo_ann_w, NULL, mooncategory, 0);
	XtManageChild (mlo_ann_w);

	/* credit */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, XtParent(mlo_sl_w)); n++;
	XtSetArg (args[n], XmNbottomOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	credit_w = XmCreateLabel (mform_w, "LOC", args, n);
	set_xmstring (credit_w, XmNlabelString, "Lunar Orbiter images used by permission of\nLunar and Planetary Institute, http://www.lpi.usra.edu");
	XtManageChild (credit_w);

	/* label in a scrolled window for image */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, credit_w); n++;
	XtSetArg (args[n], XmNbottomOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	mlo_sw_w = XmCreateScrolledWindow (mform_w, "MLOSW", args, n);
	XtManageChild (mlo_sw_w);

	    n = 0;
	    XtSetArg (args[n], XmNrecomputeSize, True); n++;
	    XtSetArg (args[n], XmNlabelType, XmPIXMAP); n++;
	    mlo_pml_w = XmCreateLabel (mlo_sw_w, "MLOPML", args, n);
	    XtManageChild (mlo_pml_w);
}

/* callback from mlo_shell_w being popped down.
 */
/* ARGSUSED */
static void
mlo_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
}

/* called from Close button in lunar orbiter window */
static void
mlo_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (mlo_shell_w);
	/* let popdown do all the work */
}

/* load the files in the Lunar Orbiter scrolled list and load the first image */
static void
mlo_load (MoonDB *mp)
{
	int i;

	/* load scrolled list */
	XmListDeleteAllItems (mlo_sl_w);
	for (i = 0; i < mp->nlofiles; i++) {
	    XmString xms = XmStringCreateSimple (mp->lofiles[i]);
	    XmListAddItemUnselected (mlo_sl_w, xms, 0);
	    XmStringFree (xms);
	}

	/* select first image and show it */
	XmListSelectPos (mlo_sl_w, 1, True);
}

/* a file in the lunar orbiter scrolled list has been selected for display */
static void
mlo_select_cb (Widget w, XtPointer client, XtPointer call)
{
	XmString sel = ((XmListCallbackStruct *)call)->item;
	char *base;

	XmStringGetLtoR (sel, XmSTRING_DEFAULT_CHARSET, &base);
	mlo_load_image (base);
	XtFree (base);
	centerScrollBars(mlo_sw_w);
}

/* user has changed whether to show annotated or not */
static void
mlo_ann_cb (Widget w, XtPointer client, XtPointer call)
{
	XmStringTable xmst;
	char *base;

	get_something (mlo_sl_w, XmNselectedItems, (XtArgVal)&xmst);
	XmStringGetLtoR (xmst[0], XmSTRING_DEFAULT_CHARSET, &base);
	mlo_load_image (base);
	XtFree (base);
}

static void
mlo_load_image (char *base)
{
	static XColor xcol[256];
	static int xcolset;
	Pixmap oldpm, pm; 
	FILE *fp;
	char msg[1024];
	int w, h;

	fp = mlo_open (base);
	if (!fp) {
	    xe_msg (1, "%s.jpg:\n%s", base, syserrstr());
	    return;
	}

	/* free last batch of colors */
	if (xcolset) {
	    freeXColors (XtD, xe_cm, xcol, XtNumber(xcol));
	    xcolset = 0;
	}

	/* create the expose pixmap */
	if (jpeg2pm (XtD, xe_cm, fp, &w, &h, &pm, xcol, msg) < 0) {
	    xe_msg (1, "%s:\n%s", base, msg);
	    fclose (fp);
	    return;
	}
	xcolset = 1;

	/* replace label pixmap */
	get_something (mlo_pml_w, XmNlabelPixmap, (XtArgVal)&oldpm);
	if (oldpm != XmUNSPECIFIED_PIXMAP)
	    XFreePixmap (XtD, oldpm);
	set_something (mlo_pml_w, XmNlabelPixmap, (XtArgVal)pm);

	/* clean up */
	fclose (fp);
}

/* return 1 if lunar orbiter images appear to be installed, else 0 */
static int
mlo_installed()
{
	FILE *fp = mlo_open ("IV-130-H1");
	if (fp) {
	    fclose (fp);
	    return (1);
	}
	return (0);
}

/* given the base name open a lunar orbiter image.
 */
static FILE *
mlo_open (char *base)
{
	int ann = XmToggleButtonGetState (mlo_ann_w);
	char fn[1024];

	sprintf (fn, "%s/lo/%s/%s.jpg", getShareDir(), ann?"amg":"img", base);
	return (fopen (fn, "r"));
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: moonmenu.c,v $ $Date: 2012/07/07 18:04:42 $ $Revision: 1.64 $ $Name:  $"};

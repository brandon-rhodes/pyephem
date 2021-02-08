/* code to manage the stuff on the mars display.
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Scale.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledW.h>

#include "xephem.h"

/* shared by marsmmenu.c */
char marscategory[] = "Mars -- View and Info";	/* Save category */

#define	POLE_RA		degrad(317.61)
#define	POLE_DEC	degrad(52.85)

static void m_popup (XEvent *ep);
static void m_create_popup (void);

static void m_create_shell (void);
static void m_create_msform (void);
static void m_create_mfform (void);
static void m_set_buttons (int whether);
static void m_mstats_close_cb (Widget w, XtPointer client, XtPointer call);
static void m_mstats_cb (Widget w, XtPointer client, XtPointer call);
static void m_features_ctrl_cb (Widget w, XtPointer client,XtPointer call);
static void m_features_cb (Widget w, XtPointer client, XtPointer call);
static void m_feasel_cb (Widget w, XtPointer client, XtPointer call);
static void m_moons_cb (Widget w, XtPointer client, XtPointer call);
static void m_selection_cb (Widget w, XtPointer client, XtPointer call);
static void m_cml_cb (Widget w, XtPointer client, XtPointer call);
static void m_slt_cb (Widget w, XtPointer client, XtPointer call);
static void m_see_cb (Widget w, XtPointer client, XtPointer call);
static void m_apply_cb (Widget w, XtPointer client, XtPointer call);
static void m_aim_cb (Widget w, XtPointer client, XtPointer call);
static void m_print_cb (Widget w, XtPointer client, XtPointer call);
static void m_print (void);
static void m_ps_annotate (void);
static void m_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void m_close_cb (Widget w, XtPointer client, XtPointer call);
static void m_mloop_cb (Widget w, XtPointer client, XtPointer call);
static void m_list_cb (Widget w, XtPointer client, XtPointer call);
static void m_init_gcs (void);
static void m_help_cb (Widget w, XtPointer client, XtPointer call);
static void m_helpon_cb (Widget w, XtPointer client, XtPointer call);
static void m_option_cb (Widget w, XtPointer client, XtPointer call);
static void m_exp_cb (Widget w, XtPointer client, XtPointer call);
static void m_pointer_eh (Widget w, XtPointer client, XEvent *ev,
    Boolean *continue_to_dispatch);
static void m_redraw (int newim);
static void m_refresh (XExposeEvent *ep);
static void m_stats (void);
static void m_drawpm (void);
static void m_setsize(void);
static void m_drFeatures (void);
static void mxim_setup (void);
static void m_orientation (void);
static void m_sizecal (void);
static void m_grid (void);
static void m_reportloc (int x, int y);
static void m_eqproject (Now *np, double ra, double dec, int *xp, int *yp);
static void m_setselected(void);
static void m_setnewpos (double l, double L);
static void mars_cml (Now *np, double *cmlp, double *sltp, double *pap);
static void swap_colors (void);
static int mxim_create (void);
static int m_readmap (void);
static int m_readFeatures (void);
static int xy2ll (int x, int y, double *ltp, double *lgp);
static int ll2xy (double l, double L, int *xp, int *yp);
static int mfsa_qsort (const void *p1, const void *p2);
static int mf_qsort (const void *p1, const void *p2);
static int m_ano (double *ixp, double *iyp, int *xp, int *yp, int w2x, int arg);

/* the map image, read into mimage, is a IMHx(2*IMH) 8bit file of relief in
 * square degree steps starting at longitude LONG0 on the left moving to the
 * west, south latitude on the bottom. use it to build a window winszXwinsz of
 * the visible face in m_xim.
 */
#define	XEVERS		5	/* required version in header */
#define	LONG0		0	/* long of left side, degs */
#define	IMH		1440	/* n rows in file map */
#define	IMW	(IMH*2)		/* n columns in file map */
#define	WINR	(winsz/2)	/* handy window radius */
#define	REDCOLORS	50	/* n red ramp colors in map */
#define	BORD		80	/* extra drawing area border for labels/stars */
#define	LGAP		20	/* gap between NSEW labels and image edge */
#define	FMAG		16	/* faintest mag of sky background object */
#define	MAXR		10	/* max gap when picking sky objects, pixels */	
#define	GSP	degrad(15.0)	/* grid spacing */
#define	FSP	(GSP/5.)	/* fine spacing */
#define	XRAD		5	/* radius of central mark, pixels */
#define	LANDSR 		3	/* radius of landing site symbol */
#define	MARSD		6776.	/* mars diam, km */

static unsigned char *mimage;	/* malloced array of raw mars image */
static Pixel mcolors[REDCOLORS];/* color-scale ramp for drawing image */
static Pixel mbg;		/* background color for image */
static int mdepth;		/* depth of image, in bits */
static int mbpp;		/* bits per pixel in image: 1, 8, 16 or 32 */
static int winsz;		/* n rows&cols in X window image */
static XImage *m_xim;		/* XImage of mars now at current size */
static double m_cml;		/* current central meridian longitude, rads */
static double m_slt;		/* current subearth latitude, rads */
static double m_sslt, m_cslt;	/*   " handy sin/cos */
static double m_pa;		/* current N pole position angle, rads */
static double m_spa, m_cpa;	/*   " handy sin/cos */
static int m_seeing;		/* seeing, tenths of arc seconds */
static Obj *marsop;		/* current mars info */
static double cm_dec, sm_dec;	/* handy cos and sin of mars' dec */

/* main's widgets */
static Widget mshell_w;		/* main mars shell */
static Widget msw_w;            /* main scrolled window */
static Widget mda_w;		/* image view DrawingArea */
static Pixmap m_pm;		/* image view staging pixmap */
static Widget dt_w;		/* main date/time stamp widget */

/* "More info" stats widgets */
static Widget msform_w;		/* statistics form dialog */
static Widget sdt_w;		/* statistics date/time stamp widget */
static Widget lat_w, lng_w;	/* lat/long under cursor */
static Widget cml_w;		/* central merdian longitude PB */
static Widget cmls_w;		/* central merdian longitude scale */
static Widget slt_w;		/* subearth latitude PB */
static Widget slts_w;		/* subearth latitude scale */
static Widget see_w;		/* seeing label */
static Widget sees_w;		/* seeing scale */
static Widget apply_w;		/* the Apply PB -- we fiddle with sensitive */
static int fakepos;		/* set when cml/slt/pa etc are not true */

/* surface object's widgets */
static Widget pu_w;		/* popup */
static Widget pu_name_w;        /* popup name label */
static Widget pu_type_w;        /* popup type label */
static Widget pu_size_w;        /* popup size label */
static Widget pu_l_w;		/* popup lat label */
static Widget pu_L_w;		/* popup Long label */
static Widget pu_aim_w;		/* popup Point PB */
static double pu_l;		/* latitude if Point PB is activated */
static double pu_L;		/* longitude if Point PB is activated */

static GC m_fgc, m_bgc, m_agc;	/* various GCs */
static XFontStruct *m_fsp;	/* label font */

static int m_selecting;		/* set while our fields are being selected */
static char marsfcategory[] = "Mars -- Features";	/* Save category */

static XImage *glass_xim;       /* glass XImage -- 0 means new or can't */
static GC glassGC;              /* GC for glass border */

#define GLASSSZ         50      /* mag glass width and heigth, pixels */
#define GLASSMAG        2       /* mag glass factor (may be any integer > 0) */

/* options list */
typedef enum {
    HALFSIZ_OPT, GRID_OPT, FLIPLR_OPT, FLIPTB_OPT, 
    N_OPT
} Option;
static int option[N_OPT];

/* Image to X Windows coord converter macros, including flipping.
 * image coords have 0 in the center +x/right +y/down of the m_xim,
 * X Windows coords have upper left +x/right +y/down of the mda_w.
 */
#define	IX2XX(x)	(BORD + WINR + (option[FLIPLR_OPT] ? -(x) : (x)))
#define	IY2XY(y)	(BORD + WINR + (option[FLIPTB_OPT] ? -(y) : (y)))
#define	XX2IX(x)	(((x) - (BORD + WINR)) * (option[FLIPLR_OPT] ? -1 : 1))
#define	XY2IY(y)	(((y) - (BORD + WINR)) * (option[FLIPTB_OPT] ? -1 : 1))

/* manage the feature labeling */
typedef enum {
    MFC_OK,
    MFC_APPLY,
    MFC_ALL,
    MFC_NONE,
    MFC_TOGGLE,
    MFC_CLOSE,
    MFC_HELP
} MFCtrl;			/* feature controls */
static Widget mfform_w;		/* feature type form dialog */
static Widget flist_w;		/* feature scrolled list */
typedef struct {
    Widget tb;			/* selection toggle button */
    int set;			/* whether currently set */
    char type[128];		/* type */
    int n;			/* count */
} MFSel;
static MFSel *mfsa;		/* malloced array of MFSel's */
static int nmfsa;		/* n entries in mfsp[] */
typedef struct {
    char name[32];		/* name */
    double lt, lg;		/* lat/long, rads +N/+W */
    double dia;			/* dia or largest dimenstion, km */
    int mfsai;			/* feature type index into mfsa */
} MFeature;
static MFeature *mf;		/* malloced list of features */
static int nmf;			/* entries in mf[] */

/* called when the mars view is activated via the main menu pulldown.
 * if first time, build everything, else just toggle whether we are mapped.
 * allow for retrying to read the image file each time until find it.
 */
void
mars_manage ()
{
	if (!mshell_w) {
	    /* one-time-only work */

	    /* all for nothing without the file and features db. */
	    if (m_readmap() < 0 || m_readFeatures() < 0)
		return;

	    /* build dialogs */
	    m_create_shell();
	    m_create_msform();
	    m_create_mfform();

	    /* establish depth, colors and bits per pixel */
	    get_something (mda_w, XmNdepth, (XtArgVal)&mdepth);
	    m_init_gcs();
	    mbpp = (mdepth==1) ? 1 : (mdepth>=17 ? 32 : (mdepth >= 9 ? 16 : 8));

	    /* establish initial mars circumstances */
	    m_stats();
	    m_setsize();
	}

	XtPopup (mshell_w, XtGrabNone);
	set_something (mshell_w, XmNiconic, (XtArgVal)False);
	centerScrollBars (msw_w);

	/* register we are now up */
	setXRes (mars_viewupres(), "1");
}

/* commanded from main to update with a new set of circumstances */
void
mars_update (np, how_much)
Now *np;
int how_much;
{
	if (!mshell_w)
	    return;
	if (!isUp(mshell_w) && !any_ison())
	    return;

	/* new mars stats */
	m_stats();

	/* new image */
	m_redraw(1);
}

/* called when basic resources change.
 * we also take care of mars moons.
 * rebuild and redraw.
 */
void
mars_newres()
{
	marsm_newres();
	if (!mshell_w)
	    return;
	m_init_gcs();
	mars_update (mm_get_now(), 1);
}

int
mars_ison()
{
	return (isUp(mshell_w));
}

/* called by other menus as they want to hear from our buttons or not.
 * the "on"s and "off"s stack - only really redo the buttons if it's the
 * first on or the last off.
 */
void
mars_selection_mode (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	if (whether)
	    m_selecting++;
	else if (m_selecting > 0)
	    --m_selecting;

	if (mars_ison()) {
	    if ((whether && m_selecting == 1)     /* first one to want on */
		|| (!whether && m_selecting == 0) /* last one to want off */)
		m_set_buttons (whether);
	}
}

/* called to put up or remove the watch cursor.  */
void
mars_cursor (c)
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

	if (msform_w && (win = XtWindow(msform_w)) != 0) {
	    Display *dsp = XtDisplay(msform_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}

	if (mfform_w && (win = XtWindow(mfform_w)) != 0) {
	    Display *dsp = XtDisplay(mfform_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* return the name of the resource containing whether this view is up */
char *
mars_viewupres()
{
	return ("MarsViewUp");
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
	    {HALFSIZ_OPT,	"HalfSize",	"Half size",
		"Display map at half size"},
	    {FLIPTB_OPT,	"FlipTB",	"Flip T/B",
	    	"Flip the map top-to-bottom"},
	    {FLIPLR_OPT,	"FlipLR",	"Flip L/R",
	    	"Flip the map left-to-right"},
	    {GRID_OPT,		"Grid",		"Grid",
	    	"Overlay a 15 degree grid and mark Sub-Earth location"},
	};
	typedef struct {
	    char *label;	/* what goes on the help label */
	    char *key;		/* string to call hlp_dialog() */
	} HelpOn;
	static HelpOn helpon[] = {
	    {"Intro...",	"Mars"},
	    {"on Mouse...",	"Mars_mouse"},
	    {"on Control...",	"Mars_control"},
	    {"on View...",	"Mars_view"},
	};
	Widget mb_w, pd_w, cb_w;
	Widget mform_w;
	Widget w;
	unsigned long mask;
	XmString str;
	Arg args[20];
	int i;
	int n;

	/* create master form in its shell */
	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Mars view"); n++;
	XtSetArg (args[n], XmNiconName, "Mars"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	mshell_w = XtCreatePopupShell ("Mars", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (mshell_w);
	set_something (mshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (mshell_w, XmNpopdownCallback, m_popdown_cb, 0);
	sr_reg (mshell_w, "XEphem*Mars.width", marscategory, 0);
	sr_reg (mshell_w, "XEphem*Mars.height", marscategory, 0);
	sr_reg (mshell_w, "XEphem*Mars.x", marscategory, 0);
	sr_reg (mshell_w, "XEphem*Mars.y", marscategory, 0);
	sr_reg (NULL, mars_viewupres(), marscategory, 0);

	n = 0;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	mform_w = XmCreateForm (mshell_w, "MarsForm", args, n);
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
	    w = XmCreatePushButton (pd_w, "MPrint", args, n);
	    set_xmstring (w, XmNlabelString, "Print...");
	    XtAddCallback (w, XmNactivateCallback, m_print_cb, 0);
	    wtip (w, "Print the current Mars map");
	    XtManageChild (w);

	    /* the "Annot" push button */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "MAnn", args, n);
	    set_xmstring (w, XmNlabelString, "User annotation...");
	    XtAddCallback (w, XmNactivateCallback, ano_cb, 0);
	    wtip (w, "Open window to create and manage your own annotation");
	    XtManageChild (w);

	    /* the "movie loop" push button */

	    n = 0;
	    n += ml_addacc (args, n);
	    w = XmCreatePushButton (pd_w, "MML", args, n);
	    set_xmstring (w, XmNlabelString, "Add to movie...");
	    XtAddCallback (w, XmNactivateCallback, m_mloop_cb, 0);
	    wtip (w, "Add this scene to the movie loop");
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
		XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
		w = XmCreateToggleButton (pd_w, osp->name, args, n);
		XtAddCallback(w, XmNvalueChangedCallback, m_option_cb,
								(XtPointer)o);
		set_xmstring (w, XmNlabelString, osp->title);
		option[o] = XmToggleButtonGetState (w);
		if (osp->tip)
		    wtip (w, osp->tip);
		XtManageChild (w);
		sr_reg (w, NULL, marscategory, 1);
	    }

	    /* add a separator */

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    /* add the Feature control */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Features", args, n);
	    set_xmstring (w, XmNlabelString, "Features...");
	    XtAddCallback (w, XmNactivateCallback, m_features_cb, NULL);
	    wtip (w, "Display labeled features");
	    XtManageChild (w);

	    /* add the More Info control */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Stats", args, n);
	    set_xmstring (w, XmNlabelString, "More info...");
	    XtAddCallback (w, XmNactivateCallback, m_mstats_cb, NULL);
	    wtip (w, "Display additional information and controls");
	    XtManageChild (w);

	    /* add the Moons control */

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Moons", args, n);
	    set_xmstring (w, XmNlabelString, "Moon view...");
	    XtAddCallback (w, XmNactivateCallback, m_moons_cb, NULL);
	    wtip (w, "Display schematic view of Mars and its moons");
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

	/* make a drawing area in a scrolled window for the image view */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, dt_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	XtSetArg (args[n], XmNvisualPolicy, XmVARIABLE); n++;
	msw_w = XmCreateScrolledWindow (mform_w, "MarsSW", args, n);
	XtManageChild (msw_w);

	    /* size gets changed to match the display later */

	    n = 0;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    mda_w = XmCreateDrawingArea (msw_w, "MarsMap", args, n);
	    XtAddCallback (mda_w, XmNexposeCallback, m_exp_cb, NULL);
            mask = Button1MotionMask | ButtonPressMask | ButtonReleaseMask |
							PointerMotionHintMask;
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

/* create the "more info" stats dialog */
static void
m_create_msform()
{
	typedef struct {
	    char *label;
	    Widget *wp;
	    char *tip;
	} DItem;
	static DItem citems[] = {
	    {"Under Cursor:", NULL, NULL},
	    {"Latitude +N:",  &lat_w, "Martian Latitude under cursor"},
	    {"Longitude +W:", &lng_w, "Martian Longitude under cursor"},
	};
	Widget rc_w;
	Widget sep_w;
	Widget f_w;
	Widget w;
	char str[32];
	Arg args[20];
	int n;
	int i;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	msform_w = XmCreateFormDialog (mshell_w, "MarsStats", args, n);
	set_something (msform_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (XtParent(msform_w), "XEphem*MarsStats.x", marscategory,0);
	sr_reg (XtParent(msform_w), "XEphem*MarsStats.y", marscategory,0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Mars info"); n++;
	XtSetValues (XtParent(msform_w), args, n);

	/* make a rowcolumn to hold the cursor tracking info */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNtopOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNspacing, 5); n++;
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
		w = XmCreateLabel (rc_w, "CVal", args, n);
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
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	sep_w = XmCreateSeparator (msform_w, "Sep1", args, n);
	XtManageChild(sep_w);

	/* make the slt row */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (msform_w, "SLTL", args, n);
	set_xmstring (w, XmNlabelString, "Sub Earth Lat (+N):");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	XtSetArg (args[n], XmNuserData, "Mars.SubLat"); n++;
	slt_w = XmCreatePushButton (msform_w, "SLTVal", args, n);
	XtAddCallback (slt_w, XmNactivateCallback, m_selection_cb, NULL);
	wtip (slt_w, "Martian latitude at center of map");
	XtManageChild (slt_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, slt_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNminimum, -90); n++;
	XtSetArg (args[n], XmNmaximum, 90); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNshowValue, False); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	slts_w = XmCreateScale (msform_w, "SLTS", args, n);
	XtAddCallback (slts_w, XmNvalueChangedCallback, m_slt_cb, NULL);
	XtAddCallback (slts_w, XmNdragCallback, m_slt_cb, NULL);
	wtip (slts_w, "Set arbitrary central latitude, then use Apply");
	XtManageChild (slts_w);

	/* make the cml row */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, slts_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (msform_w, "CMLL", args, n);
	set_xmstring (w, XmNlabelString, "Central M Long (+W):");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, slts_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	XtSetArg (args[n], XmNuserData, "Mars.CML"); n++;
	cml_w = XmCreatePushButton (msform_w, "CMLVal", args, n);
	XtAddCallback (cml_w, XmNactivateCallback, m_selection_cb, NULL);
	wtip (cml_w, "Martian longitude at center of map");
	XtManageChild (cml_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, cml_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNminimum, 0); n++;
	XtSetArg (args[n], XmNmaximum, 359); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNshowValue, False); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	cmls_w = XmCreateScale (msform_w, "CMLS", args, n);
	XtAddCallback (cmls_w, XmNvalueChangedCallback, m_cml_cb, NULL);
	XtAddCallback (cmls_w, XmNdragCallback, m_cml_cb, NULL);
	wtip (cmls_w, "Set arbitrary central longitude, then use Apply");
	XtManageChild (cmls_w);

	/* make the seeing row */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, cmls_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (msform_w, "SeeingL", args, n);
	set_xmstring (w, XmNlabelString, "Seeing (arc seconds):");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, cmls_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	see_w = XmCreateLabel (msform_w, "SeeingV", args, n);
	wtip (see_w, "Image is blurred to simulate this seeing value");
	XtManageChild (see_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, see_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNshowValue, False); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNminimum, 0); n++;
	XtSetArg (args[n], XmNmaximum, 50); n++;	/* 5 arcsecs */
	sees_w = XmCreateScale (msform_w, "Seeing", args, n);
	XtAddCallback (sees_w, XmNvalueChangedCallback, m_see_cb, NULL);
	XtAddCallback (sees_w, XmNdragCallback, m_see_cb, NULL);
	wtip (sees_w, "Set desired seeing, then use Apply");
	XtManageChild (sees_w);
	sr_reg (sees_w, NULL, marscategory, 1);

	/* pick up initial value */
	XmScaleGetValue (sees_w, &m_seeing);
	(void) sprintf (str, "%.1f", m_seeing/10.0);
	set_xmstring (see_w, XmNlabelString, str);

	/* add a label for the current date/time stamp */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sees_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	sdt_w = XmCreateLabel (msform_w, "SDTstamp", args, n);
	wtip (sdt_w, "Date and Time for which data are computed");
	XtManageChild (sdt_w);

	/* add a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sdt_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	sep_w = XmCreateSeparator (msform_w, "Sep3", args, n);
	XtManageChild (sep_w);

	/* put the bottom controls in their own form */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 10); n++;
	XtSetArg (args[n], XmNfractionBase, 13); n++;
	f_w = XmCreateForm (msform_w, "ACH", args, n);
	XtManageChild (f_w);

	    /* the apply button */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 1); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 4); n++;
	    apply_w = XmCreatePushButton (f_w, "Apply", args, n);
	    XtAddCallback (apply_w, XmNactivateCallback, m_apply_cb, NULL);
	    wtip (apply_w, "Apply the new values");
	    XtManageChild (apply_w);

	    /* the close button */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 5); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 8); n++;
	    w = XmCreatePushButton (f_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, m_mstats_close_cb, NULL);
	    wtip (w, "Close this dialog");
	    XtManageChild (w);

	    /* the help button */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 9); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 12); n++;
	    w = XmCreatePushButton (f_w, "Help", args, n);
	    XtAddCallback (w, XmNactivateCallback, m_helpon_cb,
						(XtPointer)"Mars_moreinfo");
	    wtip (w, "More info about this dialog");
	    XtManageChild (w);
}

/* given two pointers to MFSel pointers sort by type, qsort-style */
static int
mfsa_qsort (const void *p1, const void *p2)
{
	MFSel *m1 = *(MFSel**)p1;
	MFSel *m2 = *(MFSel**)p2;
	return (strcmp (m1->type, m2->type));
}

/* create the "features" stats dialog.
 * N.B. we assume mfsa[] and mf[] are all set up.
 */
static void
m_create_mfform()
{
	typedef struct {
	    MFCtrl mfce;		/* which feature control */
	    char *label;		/* label */
	    char *tip;			/* helpfull tip */
	} MFC;
	static MFC mfc[] = {
	    {MFC_OK, "Ok", "Draw the chosen features and close this dialog"},
	    {MFC_APPLY, "Apply", "Draw the chosen features"},
	    {MFC_TOGGLE, "Toggle", "Swap features on/off"},
	    {MFC_ALL, "All", "Turn all features on"},
	    {MFC_NONE, "None", "Turn all features off"},
	    {MFC_CLOSE, "Close", "Close this dialog"},
	    {MFC_HELP, "Help", "More info about this dialog"},
	};
	Widget rc_w, sep_w, f_w;
	MFSel **sortmfsa;
	XmString *flist;
	Widget w;
	Arg args[20];
	int n;
	int i;

	/* create form */

	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	mfform_w = XmCreateFormDialog (mshell_w, "MarsFeatures", args, n);
	set_something (mfform_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg(XtParent(mfform_w), "XEphem*MarsFeatures.x", marsfcategory, 0);
	sr_reg(XtParent(mfform_w), "XEphem*MarsFeatures.y", marsfcategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Mars features"); n++;
	XtSetValues (XtParent(mfform_w), args, n);

	/* make a rowcolumn and add the feature type TBs sorted by type */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNtopOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNspacing, 1); n++;
	rc_w = XmCreateRowColumn (mfform_w, "SRC", args, n);
	XtManageChild (rc_w);

	    sortmfsa = (MFSel **) XtMalloc (nmfsa * sizeof(MFSel*));
	    for (i = 0; i < nmfsa; i++)
		sortmfsa[i] = mfsa + i;
	    qsort (sortmfsa, nmfsa, sizeof(MFSel*), mfsa_qsort);

	    for (i = 0; i < nmfsa; i++) {
		MFSel *mfsp = sortmfsa[i];
		char buf[1024];
		int j;

		/* widget name is first word in type */
		for (j = 0; isalpha(mfsp->type[j]); j++)
		    buf[j] = mfsp->type[j];
		buf[j] = '\0';

		n = 0;
		XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
		w = XmCreateToggleButton (rc_w, buf, args, n);
		XtAddCallback(w, XmNvalueChangedCallback, m_feasel_cb, mfsp);
		(void) sprintf (buf, "%4d %s", mfsp->n, mfsp->type);
		set_xmstring (w, XmNlabelString, buf);
		mfsp->set = XmToggleButtonGetState (w);
		mfsp->tb = w;
		XtManageChild (w);
		sr_reg (w, NULL, marsfcategory, 1);
	    }
	    XtFree ((char*)sortmfsa);


	/* add a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	sep_w = XmCreateSeparator (mfform_w, "Sep1", args, n);
	XtManageChild (sep_w);

	/* add feature list */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNtopOffset, 10); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 10); n++;
	XtSetArg (args[n], XmNselectionPolicy, XmMULTIPLE_SELECT); n++;
	flist_w = XmCreateScrolledList (mfform_w, "FL", args, n);
	XtAddCallback (flist_w, XmNmultipleSelectionCallback, m_list_cb, 0);
	XtManageChild (flist_w);

	    flist = (XmString *) XtMalloc (nmf * sizeof(XmString));
	    for (i = 0; i < nmf; i++)
		flist[i] = XmStringCreateSimple (mf[i].name);
	    XmListAddItemsUnselected (flist_w, flist, nmf, 0);
	    for (i = 0; i < nmf; i++)
		XmStringFree (flist[i]);
	    XtFree ((char*)flist);
	    m_setselected();

	/* add the controls in their own form */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNfractionBase, 3*XtNumber(mfc)+1); n++;
	f_w = XmCreateForm (mfform_w, "MFTF", args, n);
	XtManageChild (f_w);

	    for (i = 0; i < XtNumber(mfc); i++) {
		MFC *mp = &mfc[i];

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNleftPosition, 1+3*i); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNrightPosition, 3+3*i); n++;
		w = XmCreatePushButton (f_w, "MFTPB", args, n);
		XtAddCallback (w, XmNactivateCallback, m_features_ctrl_cb,
							(XtPointer)mp->mfce);
		set_xmstring (w, XmNlabelString, mp->label);
		wtip (w, mp->tip);
		XtManageChild (w);
	    }
}

/* go through all the buttons pickable for plotting and set whether they
 * should appear to look like buttons or just flat labels.
 */
static void
m_set_buttons (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	buttonAsButton (cml_w, whether);
	buttonAsButton (slt_w, whether);
}

/* callback when an item is de/selected in the feature list */
/* ARGSUSED */
static void
m_list_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmListCallbackStruct *lp = (XmListCallbackStruct *)call;

	if (lp->selected_item_count == 1) {
	    int i = lp->selected_item_positions[0]-1;
	    m_setnewpos (mf[i].lt, mf[i].lg);
	} else
	    m_redraw(0);
}

/* callback from the Moons button */
/* ARGSUSED */
static void
m_moons_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	marsm_manage();
}

/* callback from the Close button on the stats menu */
/* ARGSUSED */
static void
m_mstats_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (msform_w);
}

/* callback when want stats menu up */
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

/* callback from any of the Features dialog bottom controls.
 * client is one of MFCtrl.
 */
/* ARGSUSED */
static void
m_features_ctrl_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	MFSel *mfsp;

	switch ((MFCtrl)client) {
	case MFC_OK:
	    m_setselected();
	    m_redraw(0);
	    XtUnmanageChild (mfform_w);
	    break;
	case MFC_APPLY:
	    m_setselected();
	    m_redraw(0);
	    break;
	case MFC_ALL:
	    for (mfsp = mfsa; mfsp < &mfsa[nmfsa]; mfsp++)
		XmToggleButtonSetState (mfsp->tb, mfsp->set=1, False);
	    break;
	case MFC_NONE:
	    for (mfsp = mfsa; mfsp < &mfsa[nmfsa]; mfsp++)
		XmToggleButtonSetState (mfsp->tb, mfsp->set=0, False);
	    break;
	case MFC_TOGGLE:
	    for (mfsp = mfsa; mfsp < &mfsa[nmfsa]; mfsp++)
		XmToggleButtonSetState (mfsp->tb,
		    mfsp->set = !XmToggleButtonGetState(mfsp->tb), False);
	    break;
	case MFC_CLOSE:
	    XtUnmanageChild (mfform_w);
	    break;
	case MFC_HELP:
	    hlp_dialog ("Mars_features", NULL, 0);
	    break;
	default:
	    printf ("Bad MFCtrl: %d\n", (int)(long int)client);
	    abort();
	}
}

/* callback when want features dialog up */
/* ARGSUSED */
static void
m_features_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtManageChild (mfform_w);
}

/* callback when want a features TB changes.
 * client is pointer to MFSel whose set state is being changed.
 */
/* ARGSUSED */
static void
m_feasel_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	MFSel *mfsp = (MFSel *) client;
	mfsp->set = XmToggleButtonGetState(w);
}

/* callback from the Print PB */
/* ARGSUSED */
static void
m_print_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
        XPSAsk ("Mars View", m_print);
}

/* proceed to generate a postscript file.
 * call XPSClose() when finished.
 */
static void
m_print ()
{
	/* must be up */
	if (!mars_ison()) {
	    xe_msg (1, "Mars must be open to print.");
	    XPSClose();
	    return;
	}

	watch_cursor(1);

	/* fit view in square across the top and prepare to capture X calls */
	XPSXBegin (m_pm, BORD, BORD, winsz, winsz, 1*72, 10*72, (int)(6.5*72));

	/* redraw everything into m_pm */
	m_redraw(1);

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
	int lx = 145, rx = 460;
	int y;

	/* caption */
	y = AROWY(13);
	(void) strcpy (buf, "XEphem Mars View");
	(void) sprintf (dir, "(%s) %d %d cstr", buf, ctr, y);
	XPSDirect (dir);

	y = AROWY(9);
	fs_date (buf, pref_get(PREF_DATE_FORMAT), mjd_day(mjd));
	(void) sprintf (dir, "(UTC Date:) %d %d rstr (%s) %d %d lstr\n",
							lx, y, buf, lx+10, y);
	XPSDirect (dir);

	fs_dm_angle (buf, m_slt);
	(void) sprintf (dir,"(%s Latitude:) %d %d rstr (%s) %d %d lstr\n",
			fakepos ? "Center" : "Sub Earth", rx, y, buf, rx+10, y);
	XPSDirect (dir);

	y = AROWY(8);
	fs_time (buf, mjd_hr(mjd));
	(void) sprintf (dir, "(UTC Time:) %d %d rstr (%s) %d %d lstr\n",
							lx, y, buf, lx+10, y);
	XPSDirect (dir);

	fs_dm_angle (buf, m_cml);
	(void) sprintf (dir,"(%s Longitude:) %d %d rstr (%s) %d %d lstr\n",
			fakepos ? "Center" : "Sub Earth", rx, y, buf, rx+10, y);
	XPSDirect (dir);

	/* add site/lat/long if topocentric */
	if (pref_get(PREF_EQUATORIAL) == PREF_TOPO) {
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

	/* add seeing if > 0 */
	if (m_seeing) {
	    y = AROWY(6);
	    (void) sprintf(dir,"(Simulated %.1f Arcsecond Seeing) %d %d cstr\n",
							m_seeing/10.0, ctr, y);
	    XPSDirect (dir);
	}
    }

/* callback from CML or SLT button being activated.
 */
/* ARGSUSED */
static void
m_selection_cb (w, client, call)
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

/* callback from the CML scale */
/* ARGSUSED */
static void
m_cml_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmScaleCallbackStruct *sp = (XmScaleCallbackStruct *)call;
	int v = sp->value;

	f_dm_angle (cml_w, degrad((double)v));

	/* Apply button is now useful */
	XtSetSensitive (apply_w, True);
}

/* callback from the SLT scale */
/* ARGSUSED */
static void
m_slt_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmScaleCallbackStruct *sp = (XmScaleCallbackStruct *)call;
	int v = sp->value;

	f_dm_angle (slt_w, degrad((double)v));

	/* Apply button is now useful */
	XtSetSensitive (apply_w, True);
}

/* callback from the Seeing scale */
/* ARGSUSED */
static void
m_see_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmScaleCallbackStruct *sp = (XmScaleCallbackStruct *)call;
	char str[32];
	int v;

	v = sp->value;
	(void) sprintf (str, "%4.1f", v/10.0);
	set_xmstring (see_w, XmNlabelString, str);

	/* Apply button is now useful */
	XtSetSensitive (apply_w, True);
}

/* callback from the Apply PB */
/* ARGSUSED */
static void
m_apply_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int v;

	/* gather the new values */
	XmScaleGetValue (cmls_w, &v);
	m_cml = degrad(v);
	f_dm_angle (cml_w, m_cml);

	XmScaleGetValue (slts_w, &v);
	m_slt = degrad(v);
	m_sslt = sin(m_slt);
	m_cslt = cos(m_slt);
	f_dm_angle (slt_w, m_slt);

	XmScaleGetValue (sees_w, &v);
	m_seeing = v;

	/* force a redraw */
	fakepos = 1;
	m_redraw(1);
}

/* callback from the Point PB */
/* ARGSUSED */
static void
m_aim_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	m_setnewpos (pu_l, pu_L);
}

/* install a new lat/long with pa 0 */
static void
m_setnewpos (double l, double L)
{
	m_slt = l;
	m_sslt = sin(m_slt);
	m_cslt = cos(m_slt);
	f_dm_angle (slt_w, m_slt);
	XmScaleSetValue (slts_w, (int)(raddeg(m_slt)));

	m_cml = L;
	f_dm_angle (cml_w, m_cml);
	XmScaleSetValue (cmls_w, (int)(raddeg(m_cml)));

	m_pa = 0.0;
	m_spa = 0.0;
	m_cpa = 1.0;

	fakepos = 1;
	m_redraw(1);
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
	XtUnmanageChild (msform_w);
	XtUnmanageChild (mfform_w);

	/* register we are now down */
	setXRes (mars_viewupres(), "0");
}

/* called from Close button */
static void
m_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (mshell_w);
	/* popdown will do all the real work */
}

/* called to add scene to the movie loop */
static void
m_mloop_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	ml_add (m_pm, dt_w);
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

	case HALFSIZ_OPT:
	    m_setsize();
	    m_redraw(1);
	    break;

	case GRID_OPT:
	    if (set) {
		m_grid();
		m_refresh (NULL);
	    } else
		m_redraw(0);
	    break;

	case FLIPTB_OPT:
	    /* swap color map when flip top/bottom */
	    swap_colors();
	    m_redraw(1);
	    break;

	case FLIPLR_OPT:
	    m_redraw(1);
	    break;

	case N_OPT:
	    break;
	}


	watch_cursor (0);
}

/* swap dark-bright color map.
 * this is used to keep mountains looking like mountains when flipping t/b
 */
static void
swap_colors()
{
	int i;

	for (i = 0; i < REDCOLORS/2; i++) {
	    int tmp = mcolors[i];
	    mcolors[i] = mcolors[REDCOLORS-1-i];
	    mcolors[REDCOLORS-1-i] = tmp;
	}
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
	    "This is a map of Mars.",
	};

	hlp_dialog ("Mars", msg, XtNumber(msg));
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

/* set winsz from HALFSIZ_OPT then size of the scrolling window, image and pm.*/
static void
m_setsize()
{
	Window win = RootWindow(XtD, DefaultScreen(XtD));
	Dimension wid, hei;
	int fullsz;

	/* full size is IMH/(PI/2) because of wrapping cyl onto sphere */
	fullsz = (int)(IMH/(PI/2));
	winsz = option[HALFSIZ_OPT] ? fullsz/2 : fullsz;

	mxim_create();

	wid = 2*BORD+winsz;
	hei = 2*BORD+winsz;
	set_something (mda_w, XmNwidth, (XtArgVal)wid);
	set_something (mda_w, XmNheight, (XtArgVal)hei);

	if (m_pm)
	    XFreePixmap (XtD, m_pm);
	m_pm = XCreatePixmap (XtD, win, wid, hei, mdepth);
}

/* expose (or reconfig) of mars image view drawing area.
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

	watch_cursor (1);

	switch (c->reason) {
	case XmCR_EXPOSE: {
	    static int before;

	    if (!before) {
		/* turn off backing store, do first full draw */
		XSetWindowAttributes swa;
		unsigned long mask = CWBackingStore;

		swa.backing_store = NotUseful;
		XChangeWindowAttributes (e->display, e->window, mask, &swa);

		m_redraw(1);

		before = 1;
	    }
	    break;
	    }
	default:
	    printf ("Unexpected mars mda_w event. type=%d\n", c->reason);
	    abort();
	}

	/* update exposed area */
	m_refresh (e);

	watch_cursor (0);
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

	if (get_color_resource (mda_w, "GlassBorderColor", &p) < 0) {
	    xe_msg (0, "Can not get GlassBorderColor -- using White");
	    p = WhitePixel(dsp, DefaultScreen(dsp));
	}
	gcm = GCForeground;
	gcv.foreground = p;
	glassGC = XCreateGC (dsp, win, gcm, &gcv);
}

/* fill glass_xim with GLASSSZ*GLASSMAG view of m_xim centered at coords
 * xc,yc. take care at the edges (m_xim is winsz x winsz)
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

		if (sx < 0 || sx >= winsz || sy < 0 || sy >= winsz)
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
	    m_reportloc (x, y);
	}
}

/* establish mimage and m_xim and return 0 else xe_msg() and return -1 */
static int
m_readmap()
{
	unsigned char r[256], g[256], b[256];
	char why[256];
	char fn[1024];
	FILE *fp;
	int w, h;
	int i;

	/* open mars map */
	(void) sprintf (fn, "%s/auxil/marsmap.jpg", getShareDir());
	fp = fopenh (fn, "r");
	if (!fp) {
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	    return (-1);
	}

	/* read mars file into mimage */
	mimage = jpegRead (fp, &w, &h, r, g, b, why);
	fclose (fp);
	if (!mimage) {
	    xe_msg (1, "%s:\n%s", fn, why);
	    return (-1);
	}

	/* make some sanity checks */
	if (w != 2*IMH || h != IMH) {
	    xe_msg (1, "%s:\nExpected %d x %d but found %d x %d", fn,
							    2*IMH, IMH, w, h);
	    free (mimage);
	    mimage = NULL;
	    return (-1);
	}

	/* convert colormap index to gray using SMPTE convention */
	for (i = 0; i < w*h; i++) {
	    int c = mimage[i];
	    mimage[i] = (unsigned char)(.33*r[c] + .5*g[c] + .17*b[c]);
	}

	return(0);
}

/* create m_xim of size winszxwinsz, depth mdepth and bit-per-pixel mbpp.
 * make a Bitmap if only have 1 bit per pixel, otherwise a Pixmap.
 * return 0 if ok else -1 and xe_msg().
 */
static int
mxim_create ()
{
	Display *dsp = XtDisplay (mda_w);
	int nbytes = winsz*winsz*mbpp/8;
	char *data;

	/* delete if old exists */
	if (m_xim) {
	    free ((void *)m_xim->data);
	    m_xim->data = NULL;
	    XDestroyImage (m_xim);
	    m_xim = NULL;
	}

	/* get memory for image pixels.  */
	data = (char *) malloc (nbytes);
	if (!data) {
	    xe_msg (1, "Can not get %d bytes for shadow pixels", nbytes);
	    return (-1);
	}

	/* create the XImage */
	m_xim = XCreateImage (dsp, DefaultVisual (dsp, DefaultScreen(dsp)),
	    /* depth */         mbpp == 1 ? 1 : mdepth,
	    /* format */        mbpp == 1 ? XYBitmap : ZPixmap,
	    /* offset */        0,
	    /* data */          data,
	    /* width */         winsz,
	    /* height */        winsz,
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

/* select each entry in the scrolled list if and only if it is implied
 * from the current list of chosen types.
 */
static void
m_setselected()
{
	int i;

	/* unselect all entries */
	XmListDeselectAllItems (flist_w);

	/* select each entry whose type is set */
	for (i = 0; i < nmf; i++)
	    if (mfsa[mf[i].mfsai].set)
		XmListSelectPos (flist_w, i+1, False);
}

/* full redraw takes three steps: fill image, fill pixmap, copy to screen.
 * this does the first step if desired, then always the next 2.
 */
static void
m_redraw(newim)
int newim;
{
	watch_cursor (1);

	XmUpdateDisplay (toplevel_w);
	if (newim)
	    mxim_setup ();
	m_drawpm ();
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

	/* ignore of no pixmap now */
	if (!pm)
	    return;

	if (ep) {
	    x = ep->x;
	    y = ep->y;
	    w = ep->width;
	    h = ep->height;
	} else {
	    w = winsz+2*BORD;
	    h = winsz+2*BORD;
	    x = y = 0;
	}

	XCopyArea (dsp, pm, win, m_fgc, x, y, w, h, x, y);
}

/* make the various gcs, handy pixel values and fill in mcolors[].
 * N.B. just call this once.
 * TODO: reclaim old stuff if called again
 */
static void
m_init_gcs()
{
	Display *dsp = XtD;
	Window win = XtWindow(toplevel_w);
	Colormap cm = xe_cm;
	XGCValues gcv;
	Pixel reds[REDCOLORS];
	int nreds;
	XColor xc;
	unsigned int gcm;
	Pixel fg;
	Pixel p;
	int i;

	/* fg and bg */
	get_color_resource (mda_w, "marsColor", &fg);
	(void) get_color_resource (mda_w, "MarsBackground", &mbg);

	gcm = GCForeground | GCBackground;
	gcv.foreground = fg;
	gcv.background = mbg;
	m_fgc = XCreateGC (dsp, win, gcm, &gcv);

	gcv.foreground = mbg;
	gcv.background = fg;
	m_bgc = XCreateGC (dsp, win, gcm, &gcv);

	/* make the label marker gc */
	(void) get_color_resource (mda_w, "MarsAnnotColor", &p);
	gcm = GCForeground | GCBackground;
	gcv.foreground = p;
	gcv.background = mbg;
	m_agc = XCreateGC (dsp, win, gcm, &gcv);

	get_views_font (dsp, &m_fsp);

	/* build red ramp for image.
	 * base the scale off the foreground color.
	 */
	xc.pixel = fg;
	XQueryColor (dsp, cm, &xc);
	nreds = alloc_ramp (dsp, &xc, cm, reds, REDCOLORS);
	if (nreds < REDCOLORS) {
	    xe_msg (0, "Wanted %d but only found %d colors for Mars.",
							REDCOLORS, nreds);
	}

	/* set mcolors[] */
	for (i = 0; i < REDCOLORS; i++)
	    mcolors[i] = reds[nreds*i/REDCOLORS];
	if (!option[FLIPTB_OPT])
	    swap_colors();
}

/* update mars info and draw the stat labels */
static void
m_stats ()
{
	Now *np = mm_get_now();

	/* get fresh mars info */
	marsop = db_basic (MARS);
	db_update (marsop);
	cm_dec = cos(marsop->s_gaedec);
	sm_dec = sin(marsop->s_gaedec);

	/* compute and display the CML and SLT and polar position angle */
	mars_cml (np, &m_cml, &m_slt, &m_pa);
	m_sslt = sin(m_slt);
	m_cslt = cos(m_slt);
	m_spa = sin(m_pa);
	m_cpa = cos(m_pa);

	f_dm_angle (cml_w, m_cml);
	XmScaleSetValue (cmls_w, (int)(raddeg(m_cml)));
	f_dm_angle (slt_w, m_slt);
	XmScaleSetValue (slts_w, (int)(raddeg(m_slt)));

	/* reset fake flag */
	fakepos = 0;

	/* update time stamps too */
	timestamp (np, dt_w);
	timestamp (np, sdt_w);
}

/* fill the pixmap, m_pm.
 * N.B. if mars image changed, call mxim_setup() before this.
 * N.B. if want to draw, call m_refresh() after this.
 */
static void
m_drawpm ()
{
	/* check assumptions */
	if (!m_pm) {
	    printf ("No mars m_pm Pixmap!\n");
	    abort();
	}

	/* Apply button is no longer useful */
	XtSetSensitive (apply_w, False);

	/* clear m_pm, and copy m_xim within BORDER */
	XPSPaperColor (mbg);
	XPSFillRectangle (XtD, m_pm, m_bgc, 0, 0, winsz+2*BORD, winsz+2*BORD);
	XPutImage (XtD, m_pm, m_fgc, m_xim, 0, 0, BORD, BORD, winsz, winsz);
	XPSPixmap (m_pm, winsz+2*BORD, winsz+2*BORD, xe_cm, m_bgc, 1);

	/* add grid, if enabled */
	if (option[GRID_OPT])
	    m_grid();

	/* add feature labels, as enabled */
	m_drFeatures();

	/* add orientation markings */
	m_orientation();

	/* and the size calibration */
	m_sizecal();

	/* user annotation */
	ano_draw (mda_w, m_pm, m_ano, 0);
}

static int
m_ano (double *latp, double *lngp, int *xp, int *yp, int w2x, int arg)
{
	int x, y;

	if (w2x) {
	    if (ll2xy (*latp, *lngp, &x, &y) < 0)
		return (0);
	    *xp = IX2XX(x);
	    *yp = IY2XY(y);
	} else {
	    x = XX2IX(*xp);
	    y = XY2IY(*yp);
	    if (xy2ll (x, y, latp, lngp) < 0)
		return (0);
	}

	return (1);
}

/* discard trailing whitespace in name IN PLACE */
static void
noTrWhite (name)
char *name;
{
	int l;

	for (l = strlen(name)-1; l >= 0 && isspace(name[l]); --l)
	    name[l] = '\0';
}

/* look through mfsa[] for type.
 * if first time for this type, add to list.
 * increment count.
 * return index into mfsa.
 */
static int
findMFSel (type)
char *type;
{
	MFSel *mfsp;

	/* check for existing */
	for (mfsp = mfsa; mfsp < &mfsa[nmfsa]; mfsp++)
	    if (!strcmp (type, mfsp->type)) {
		mfsp->n++;
		return (mfsp-mfsa);
	    }

	/* add one */
	mfsa = (MFSel *) XtRealloc ((char*)mfsa, (nmfsa+1)*sizeof(MFSel));
	mfsp = &mfsa[nmfsa++];
	strcpy (mfsp->type, type);
	mfsp->n = 1;
	return (mfsp-mfsa);
}

/* given two pointers to MFeature sort by name, qsort-style */
static int
mf_qsort (const void *p1, const void *p2)
{
	MFeature *f1 = (MFeature*)p1;
	MFeature *f2 = (MFeature*)p2;
	return (strcmp (f1->name, f2->name));
}

/* read in the mars_db features list.
 * build malloced lists mf and mfsa.
 * features in mf[] will be sorted by name.
 * N.B. mf[] point into mfsa[] so don't move them.
 * return 0 if ok, else -1.
 */
static int
m_readFeatures()
{
	char buf[1024];
	char fn[1024];
	FILE *fp;

	/* open the file */
	(void) sprintf (fn, "%s/auxil/mars_db",  getShareDir());
	fp = fopenh (fn, "r");
	if (!fp) {
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	    return (-1);
	}

	/* prepare lists.
	 * really +1 to allow always using realloc, and as staging for next.
	 */
	if (mf)
	    XtFree ((void*)mf);
	mf = (MFeature *) XtMalloc (sizeof(MFeature));
	nmf = 0;
	if (mfsa)
	    XtFree ((void*)mfsa);
	mfsa = (MFSel *) XtMalloc (sizeof(MFSel));
	nmfsa = 0;

	/* read and add each feature and unique feature type */
	while (fgets (buf, sizeof(buf), fp)) {
	    MFeature *mfp = &mf[nmf];
	    char type[sizeof(((MFSel*)0)->type)];
	    char name[128];
	    int nf;

	    /* ignore all lines that do not follow the pattern */
	    nf = sscanf(buf,"%[^|]| %lf | %lf | %lf | %[^\n]", name,
					&mfp->lt, &mfp->lg, &mfp->dia, type);
	    if (nf != 5)
		continue;
	    mfp->lt = degrad(mfp->lt);
	    mfp->lg = degrad(mfp->lg);

	    /* remove trailing white space */
	    noTrWhite(type);
	    noTrWhite(name);
	    strncpy(mfp->name, name, sizeof(mfp->name));

	    /* find type, creating new if first time seen */
	    mfp->mfsai = findMFSel (type);

	    /* add */
	    nmf++;
	    mf = (MFeature *) XtRealloc ((void*)mf, (nmf+1)*sizeof(MFeature));
	}
	fclose(fp);

	/* sort features by name */
	qsort (mf, nmf, sizeof(MFeature), mf_qsort);

	/* ok */
	xe_msg (0, "Read %d features from mars_db", nmf);
	return (0);
}

/* draw the mf list, based on what is selected in flist_w */
static void
m_drFeatures ()
{
	Display *dsp = XtDisplay (mda_w);
	int *selected, nselected;
	int i;

	if (!XmListGetSelectedPos (flist_w, &selected, &nselected))
	    return;

	XSetFont (dsp, m_agc, m_fsp->fid);

	for (i = 0; i < nselected; i++) {
	    MFeature *mfp = &mf[selected[i]-1];
	    int dir, asc, des;
	    XCharStruct all;
	    int r, x, y;
	    int l;

	    /* find map location in X windows coords */
	    if (ll2xy (mfp->lt, mfp->lg, &x, &y) < 0)
		continue;
	    x = IX2XX(x);
	    y = IY2XY(y);

	    /* find radius if crater or single mountain or 0, in X pixels */
	    if (!strncmp (mfsa[mfp->mfsai].type, "Crater", 6)
					    || strstr (mfp->name, "Mons"))
		r = (int)(winsz*mfp->dia/MARSD)/2;
	    else if (!strncmp (mfsa[mfp->mfsai].type, "Landing", 7))
		r = LANDSR;
	    else
		r = 0;

	    /* center and display the name */
	    l = strlen(mfp->name);
	    XTextExtents (m_fsp, mfp->name, l, &dir, &asc, &des, &all);
	    XPSDrawString (dsp, m_pm, m_agc, x-all.width/2,y-(r+2),mfp->name,l);
	    if (r > 0)
		XPSDrawArc (dsp, m_pm, m_agc, x-r, y-r, 2*r, 2*r, 0, 360*64);
	}

	XtFree ((char *)selected);
}

/* draw the N/S E/W labels on m_pm */
static void
m_orientation()
{
	Now *np = mm_get_now();
	double mr, mra, mdec;
	double ra, dec;
	int x, y;

	/* celestial plane has not meaning at arbitrary orientations */
	if (fakepos)
	    return;

	XSetFont (XtD, m_agc, m_fsp->fid);
	mr = degrad(marsop->s_size/3600.0)/2 * 1.1;
	mra = marsop->s_gaera;
	mdec = marsop->s_gaedec;

	ra = mra + mr/cm_dec;
	dec = mdec;
	m_eqproject (np, ra, dec, &x, &y);
	x = IX2XX(x);
	y = IY2XY(y);
	XPSDrawString (XtD, m_pm, m_agc, x, y, "E", 1);

	ra = mra - mr/cm_dec;
	m_eqproject (np, ra, dec, &x, &y);
	x = IX2XX(x);
	y = IY2XY(y);
	XPSDrawString (XtD, m_pm, m_agc, x, y, "W", 1);

	ra = mra;
	dec = mdec + mr;
	m_eqproject (np, ra, dec, &x, &y);
	x = IX2XX(x);
	y = IY2XY(y);
	XPSDrawString (XtD, m_pm, m_agc, x, y, "N", 1);

	dec = mdec - mr;
	m_eqproject (np, ra, dec, &x, &y);
	x = IX2XX(x);
	y = IY2XY(y);
	XPSDrawString (XtD, m_pm, m_agc, x, y, "S", 1);
}

/* draw the size calibration */
static void
m_sizecal()
{
	int dir, asc, des;
	XCharStruct xcs;
	char buf[64];
	int l;

	(void) sprintf (buf, "%.1f\"", marsop->s_size);
	l = strlen (buf);
	XQueryTextExtents (XtD, m_fsp->fid, buf, l, &dir, &asc, &des, &xcs);

	XSetFont (XtD, m_agc, m_fsp->fid);
	XPSDrawLine (XtD, m_pm, m_agc, BORD, winsz+3*BORD/2, winsz+BORD,
							    winsz+3*BORD/2);
	XPSDrawLine (XtD, m_pm, m_agc, BORD, winsz+3*BORD/2-3, BORD,
							    winsz+3*BORD/2+3);
	XPSDrawLine (XtD, m_pm, m_agc, BORD+winsz, winsz+3*BORD/2-3, BORD+winsz,
							    winsz+3*BORD/2+3);
	XPSDrawString (XtD, m_pm, m_agc, BORD+winsz/2-xcs.width/2,
					winsz+3*BORD/2+xcs.ascent+6, buf, l);
}

/* draw a coordinate grid over the image already on m_pm */
static void
m_grid()
{
	Display *dsp = XtDisplay (mda_w);
	double fsp = FSP;
	double lt, lg;
	int x, y;

	/* set current font */
	XSetFont (dsp, m_agc, m_fsp->fid);

	/* lines of constant lat */
	for (lt = -PI/2 + GSP; lt < PI/2; lt += GSP) {
	    XPoint xpt[(int)(2*PI/FSP)+2];
	    int npts = 0;

	    for (lg = 0; lg <= 2*PI+fsp; lg += fsp) {
		if (ll2xy(lt, lg, &x, &y) < 0) {
		    if (npts > 0) {
			XPSDrawLines (dsp, m_pm,m_agc,xpt,npts,CoordModeOrigin);
			npts = 0;
		    }
		    continue;
		}

		if (npts >= XtNumber(xpt)) {
		    printf ("Mars lat grid overflow\n");
		    abort();
		}
		xpt[npts].x = IX2XX(x);
		xpt[npts].y = IY2XY(y);
		npts++;
	    }

	    if (npts > 0)
		XPSDrawLines (dsp, m_pm, m_agc, xpt, npts, CoordModeOrigin);
	}

	/* lines of constant longitude */
	for (lg = 0; lg < 2*PI; lg += GSP) {
	    XPoint xpt[(int)(2*PI/FSP)+1];
	    int npts = 0;

	    for (lt = -PI/2; lt <= PI/2; lt += fsp) {
		if (ll2xy(lt, lg, &x, &y) < 0) {
		    if (npts > 0) {
			XPSDrawLines (dsp, m_pm,m_agc,xpt,npts,CoordModeOrigin);
			npts = 0;
		    }
		    continue;
		}

		if (npts >= XtNumber(xpt)) {
		    printf ("Mars lng grid overflow\n");
		    abort();
		}
		xpt[npts].x = IX2XX(x);
		xpt[npts].y = IY2XY(y);
		npts++;
	    }

	    if (npts > 0)
		XPSDrawLines (dsp, m_pm, m_agc, xpt, npts, CoordModeOrigin);
	}

	/* X marks the center, unless rotated by hand */
	if (!fakepos) {
	    XPSDrawLine (dsp, m_pm, m_agc, IX2XX(-XRAD), IY2XY(-XRAD),
						    IX2XX(XRAD), IY2XY(XRAD));
	    XPSDrawLine (dsp, m_pm, m_agc, IX2XX(-XRAD), IY2XY(XRAD),
						    IX2XX(XRAD), IY2XY(-XRAD));
	}
}

/* fill in m_xim from mimage and current circumstances.
 * m_xim is winszxwinsz, mimage is 2*IMH wide x IMH high.
 */
static void
mxim_setup ()
{
#define	SQR(x)	((x)*(x))
	int pixseeing = (int)(winsz*m_seeing/marsop->s_size/10);
	int tb = option[FLIPTB_OPT];
	int lr = option[FLIPLR_OPT];
	unsigned char *see;			/* seeing temp array */
	unsigned char *pict;			/* working temp copy */
	double csh;
	int lsh;
	int x, y;

	/* check assumptions */
	if (!m_xim) {
	    printf ("No mars m_xim!\n");
	    abort();
	}
	if (!mimage) {
	    printf ("No mars mimage!\n");
	    abort();
	}

	/* make working copy -- background is 0 */
	pict = (unsigned char *) XtCalloc (winsz*winsz, 1);

	/* make seeing temp array if non-0 seeing */
	see = pixseeing > 0 ? (unsigned char *) XtMalloc (winsz) : NULL;

	/* scan to build up the morphed relief map, and allow for flipping */
	for (y = -WINR; y < WINR; y++) {
	    int iy = (tb ? -y-1 : y) + WINR;
	    unsigned char *prow = &pict[iy*winsz];

	    pm_set ((y+WINR)*50/winsz);

	    for (x = -WINR; x < WINR; x++) {
		int ix = (lr ? -x-1 : x) + WINR;
		unsigned char *px = &prow[ix];

		if (x*x + y*y < WINR*WINR) {
		    double l, L;
		    int mx, my;

		    if (xy2ll (x, y, &l, &L) == 0) {
			/* find the mimage pixel at l/L */
			my = (int)(IMH*(PI/2-l)/PI);
			L = degrad(LONG0) - L;
			range (&L, 2*PI);
			mx = (int)(IMW*L/(2*PI));
			*px = mimage[my*IMW+mx];
			if (!*px)
			    *px = 1;
		    }
		}
	    }
	}

	/* find cos of shadow foreshortening angle based on planar
	 * sun-mars-earth triangle and earth-sun == 1.
	 * if we are faking it, turn off the shadow.
	 */
	csh = fakepos ? 1.0 : (SQR(marsop->s_sdist) + SQR(marsop->s_edist) - 1)
					/ (2*marsop->s_sdist*marsop->s_edist);

	/* shadow is on left if elongation is positive and flipped lr
	 * or elongation is negative and not flipped lr.
	 */
	lsh = (marsop->s_elong > 0.0 && lr) || (marsop->s_elong < 0.0 && !lr);

	/* scan again to blur, add shadow and place real pixels */
	for (y = -WINR; y < WINR; y++) {
	    int iy = y + WINR;
	    unsigned char *prow = &pict[iy*winsz];
	    int lx, rx;

	    pm_set (50+(y+WINR)*50/winsz);

	    if (lsh) {
		lx = (int)(-csh*sqrt((double)(WINR*WINR - y*y)) + .5);
		rx = WINR;
	    } else {
		lx = -WINR;
		rx = (int)(csh*sqrt((double)(WINR*WINR - y*y)) + .5);
	    }

	    /* fill in seeing table for this row */
	    if (pixseeing > 0) {
		for (x = -WINR; x < WINR; x++) {
		    int ix = x + WINR;
		    int s = (pixseeing+1)/2;
		    int nsc = 0;
		    int sc = 0;
		    int dx, dy;
		    int step;

		    /* is center within mars circle? */
		    if (x*x + y*y > WINR*WINR)
			continue;

		    /* establish sample step size, some fraction of seeing */
		    step = s/7;
		    if (step < 1)
			step = 1;

		    /* average a sampled circle about ix,iy */
		    for (dy = -s; dy <= s; dy += step) {
			int sy = y + dy;
			int maxdxdx = s*s - dy*dy;
			int maxsxsx = WINR*WINR - sy*sy;
			unsigned char *py = &pict[(sy+WINR)*winsz];
			for (dx = -s; dx <= s; dx += step) {
			    if (dx*dx <= maxdxdx) {  /* within seeing circle? */
				int sx = x + dx;
				if (sx*sx <= maxsxsx) {	/* within mars ? */
				    sc += py[sx+WINR];
				    nsc++;
				}
			    }
			}
		    }

		    see[ix] = nsc > 0 ? sc/nsc : 0;
		}
	    }

	    for (x = -WINR; x < WINR; x++) {
		int ix = x + WINR;
		unsigned long p;
		int c;

		if (x < lx || x > rx || x*x + y*y >= WINR*WINR)
		    c = 0;
		else if (pixseeing > 0)
		    c = see[ix];
		else
		    c = prow[ix];

		p = c ? mcolors[c*REDCOLORS/256] : mbg;
		XPutPixel (m_xim, ix, iy, p);
	    }
	}

	XtFree ((char *)pict);
	if (see)
	    XtFree ((char *)see);
}

/* convert [x,y] to true mars lat/long, in rads.
 *   x: centered, +right, -WINR .. x .. WINR
 *   y: centered, +down,  -WINR .. y .. WINR
 * return 0 if x,y are really over the planet, else -1.
 * caller can be assured -PI/2 .. l .. PI/2 and 0 .. L .. 2*PI.
 * N.B. it is up to the caller to deal wth flipping.
 */
static int
xy2ll (x, y, lp, Lp)
int x, y;
double *lp, *Lp;
{
	double R = sqrt ((double)(x*x + y*y));
	double a;
	double ca, B;

	if (R >= WINR)
	    return (-1);

	if (y == 0)
	    a = x < 0 ? -PI/2 : PI/2;
	else 
	    a = atan2((double)x,(double)y);
	solve_sphere (a, asin(R/WINR), m_sslt, m_cslt, &ca, &B);

	*lp = PI/2 - acos(ca);
	*Lp = m_cml + B;
	range (Lp, 2*PI);

	return (0);
}

/* convert true mars lat/long, in rads, to [x,y].
 *   x: centered, +right, -WINR .. x .. WINR
 *   y: centered, +down,  -WINR .. y .. WINR
 * return 0 if loc is on the front face, else -1 (but still compute x,y);
 * N.B. it is up to the caller to deal wth flipping of the resulting locs.
 */
static int
ll2xy (l, L, xp, yp)
double l, L;
int *xp, *yp;
{
	double sR, cR;
	double A, sA, cA;

	solve_sphere (L - m_cml, PI/2 - l, m_sslt, m_cslt, &cR, &A);
	sR = sqrt(1.0 - cR*cR);
	sA = sin(A);
	cA = cos(A);

	*xp = (int)floor(WINR*sR*sA + 0.5);
	*yp = (int)floor(WINR*sR*cA + 0.5);

	return (cR > 0 ? 0 : -1);
}

/* report the location of x,y, which are with respect to mda_w.
 * N.B. allow for flipping and the borders.
 */
static void
m_reportloc (x, y)
int x, y;
{
	double lt, lg;

	/* convert from mda_w X Windows coords to centered m_xim coords */
	x = XX2IX(x);
	y = XY2IY(y);

	if (xy2ll (x, y, &lt, &lg) == 0) {
	    f_dm_angle (lat_w, lt);
	    f_dm_angle (lng_w, lg);
	} else {
	    set_xmstring (lat_w, XmNlabelString, " ");
	    set_xmstring (lng_w, XmNlabelString, " ");
	}
}

/* find the smallest entry in mf[] that [x,y] is within.
 * if find one return its *MFeature else NULL.
 * x and y are in image coords.
 * N.B. we allow for flipping.
 */
static MFeature *
closeFeature (x, y)
int x, y;
#define	RSLOP	5
{
	MFeature *mfp, *smallp;
	double lt, lg;		/* location of [x,y] */
	double forsh;		/* foreshortening (cos of angle from center) */
	double minr;

	/* find forshortening */
	if (xy2ll (x, y, &lt, &lg) < 0)
	    return (NULL);
	solve_sphere (lg - m_cml, PI/2-m_slt, sin(lt), cos(lt), &forsh, NULL);

	watch_cursor(1);

	minr = 1e6;
	smallp = NULL;
	for (mfp = mf; mfp < &mf[nmf]; mfp++) {
	    int sz;			/* radius, in pixels */
	    int dx, dy;
	    int fx, fy;
	    double r;

	    /* find pixels from cursor */
	    if (ll2xy (mfp->lt, mfp->lg, &fx, &fy) < 0)
		continue;
	    dx = fx - x;
	    dy = fy - y;
	    r = sqrt((double)dx*dx + (double)dy*dy);

	    /* it's a candidate if we are inside its (foreshortened) size
	     * or we are within RSLOP pixel of it.
	     */
	    sz = (int)(mfp->dia*(winsz/MARSD/2));
	    if (r <= sz*forsh && r < minr) {
		smallp = mfp;
		minr = r;
	    }
	}

	watch_cursor(0);

	return (smallp);
}

/* called when hit button 3 over image */
static void
m_popup (ep)
XEvent *ep;
{
	XButtonEvent *bep;
	int overmars;
	int x, y;

	/* get m_da convert to image coords */
	bep = &ep->xbutton;
	x = XX2IX(bep->x);
	y = XY2IY(bep->y);

	overmars = xy2ll (x, y, &pu_l, &pu_L) == 0;

	if (overmars) {
	    MFeature *mfp = closeFeature (x, y);
	    char buf[32];

	    /* create popup menu first time */
	    if (!pu_w)
		m_create_popup();

	    if (mfp) {
		char *type = mfsa[mfp->mfsai].type;

		set_xmstring (pu_name_w, XmNlabelString, mfp->name);
		XtManageChild (pu_name_w);

		(void) sprintf (buf, "%.*s", (int)strcspn (type, " ,-"), type);
		set_xmstring (pu_type_w, XmNlabelString, buf);
		XtManageChild (pu_type_w);

		(void) sprintf (buf, "%.1f km", mfp->dia);
		set_xmstring (pu_size_w, XmNlabelString, buf);
		XtManageChild (pu_size_w);

		/* show feature's coords */
		fs_sexa (buf, raddeg(mfp->lt), 3, 60);
		(void) strcat (buf, " N");
		set_xmstring (pu_l_w, XmNlabelString, buf);
		fs_sexa (buf, raddeg(mfp->lg), 3, 60);
		(void) strcat (buf, " W");
		set_xmstring (pu_L_w, XmNlabelString, buf);
	    } else {
		XtUnmanageChild (pu_name_w);
		XtUnmanageChild (pu_type_w);
		XtUnmanageChild (pu_size_w);

		/* show cursors coords */
		fs_sexa (buf, raddeg(pu_l), 3, 3600);
		(void) strcat (buf, " N");
		set_xmstring (pu_l_w, XmNlabelString, buf);
		fs_sexa (buf, raddeg(pu_L), 3, 3600);
		(void) strcat (buf, " W");
		set_xmstring (pu_L_w, XmNlabelString, buf);
	    }

	    XmMenuPosition (pu_w, (XButtonPressedEvent *)ep);
	    XtManageChild (pu_w);
	}
}

/* create the surface popup menu */
static void
m_create_popup()
{
	Widget w;
	Arg args[20];
	int n;

	n = 0;
	XtSetArg (args[n], XmNisAligned, True); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
	pu_w = XmCreatePopupMenu (mda_w, "MSKYPU", args, n);

	n = 0;
	pu_name_w = XmCreateLabel (pu_w, "MName", args, n);
	wtip (pu_name_w, "Name");

	n = 0;
	pu_type_w = XmCreateLabel (pu_w, "MType", args, n);
	wtip (pu_type_w, "Type");

	n = 0;
	pu_size_w = XmCreateLabel (pu_w, "MSizeat", args, n);
	wtip (pu_size_w, "Size");

	n = 0;
	pu_l_w = XmCreateLabel (pu_w, "MLat", args, n);
	wtip (pu_l_w, "Latitude");
	XtManageChild (pu_l_w);

	n = 0;
	pu_L_w = XmCreateLabel (pu_w, "MLong", args, n);
	wtip (pu_L_w, "Longitude");
	XtManageChild (pu_L_w);

	n = 0;
	w = XmCreateSeparator (pu_w, "MSKYPS", args, n);
	XtManageChild (w);

	n = 0;
	pu_aim_w = XmCreatePushButton (pu_w, "Point", args, n);
	XtAddCallback (pu_aim_w, XmNactivateCallback, m_aim_cb, NULL);
	wtip (pu_aim_w, "Center this location in the view");
	XtManageChild (pu_aim_w);
}

/* given geocentric ra and dec find image [xy] on martian equitorial projection.
 * [0,0] is center, +x martian right/west (celestial east) +y down/north.
 * N.B. we do *not* allow for flipping here.
 * N.B. this only works when ra and dec are near mars.
 * N.B. this uses m_pa.
 */
static void
m_eqproject (np, ra, dec, xp, yp)
Now *np;
double ra, dec;
int *xp, *yp;
{
	double scale = winsz/degrad(marsop->s_size/3600.0); /* pix/rad */
	double xr, yr;
	double x, y;

	/* find x and y so +x is celestial right/east and +y is down/north */
	x = scale*(ra - marsop->s_gaera)*cm_dec;
	y = scale*(dec - marsop->s_gaedec);

	/* rotate by position angle, m_pa.
	 */
	xr = x*m_cpa - y*m_spa;
	yr = x*m_spa + y*m_cpa;

	*xp = (int)floor(xr + 0.5);
	*yp = (int)floor(yr + 0.5);
}

/* return:
 *   *cmlp: Martian central meridian longitude;
 *   *sltp: subearth latitude
 *   *pap:  position angle of N pole (ie, rads E of N)
 * all angles in rads.
 */

#define M_CML0  degrad(325.845)         /* Mars' CML towards Aries at M_MJD0 */
#define M_MJD0  (2418322.0 - MJD0)      /* mjd date of M_CML0 */
#define M_PER   degrad(350.891962)      /* Mars' rotation period, rads/day */

static void
mars_cml(np, cmlp, sltp, pap)
Now *np;
double *cmlp;
double *sltp;
double *pap;
{
	Obj *sp;
	double a;	/* angle from Sun ccw to Earth seen from Mars, rads */
	double Ae;	/* planetocentric longitude of Earth from Mars, rads */
	double cml0;	/* Mar's CML towards Aries, rads */
	double lc;	/* Mars rotation correction for light travel, rads */
	double tmp;

	sp = db_basic (SUN);
	db_update (sp);

	a = asin (sp->s_edist/marsop->s_edist*sin(marsop->s_hlong-sp->s_hlong));
	Ae = marsop->s_hlong + PI + a;
	cml0 = M_CML0 + M_PER*(mjd-M_MJD0) + PI/2;
	range(&cml0, 2*PI);
	lc = LTAU * marsop->s_edist/SPD*M_PER;
	*cmlp = cml0 - Ae - lc;
	range (cmlp, 2*PI);

	solve_sphere (POLE_RA - marsop->s_gaera, PI/2-POLE_DEC, sm_dec, cm_dec,
								    &tmp, pap);

	/* from Green (1985) "Spherical Astronomy", Cambridge Univ. Press,
	 * p.428. Courtesy Jim Bell.
	 */
        *sltp = asin (-sin(POLE_DEC)*sin(marsop->s_gaedec) -
					cos(POLE_DEC)*cos(marsop->s_gaedec)
						* cos(marsop->s_gaera-POLE_RA));
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: marsmenu.c,v $ $Date: 2009/01/05 20:55:54 $ $Revision: 1.78 $ $Name:  $"};

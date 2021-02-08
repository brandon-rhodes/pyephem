/* code to manage the stuff on the "earth view" display.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/Scale.h>
#include <Xm/CascadeB.h>

#include "xephem.h"

/* these are from earthmap.c */
extern MRegion ereg[];
extern int nereg;

/* info to keep track of object trails.
 */
typedef struct {
    Now t_now;			/* Now for object at a given time */
    Obj t_obj;			/* it's Obj at that time */
    double t_sublat, t_sublng;	/* sub lat/lng of object at that time */
    int t_lbl;			/* whether this tickmark gets labeled */
    short lvis, lx, ly;		/* if land mark visible: screen x and y */
} Trail;

/* one row -- one per object */
#define	PNL	(MAXNM+16)	/* size of room for userData pick name */
/* N.B. use malloced space for _pn because this struct gets realloced around */
typedef struct {
    Obj *op;			/* handy pointer from favs */
    TrState trstate;		/* initial and retaining trails options */
    Trail *trail;		/* malloced list, current pos always at [0] */
    int ntrail;			/* entries in trail[] */
    Widget name_w;		/* label to show current object */
    Widget show_w;		/* TB whether to show at all */
    Widget wantlbl_w;		/* TB whether to add label */
    Widget wantfoot_w;		/* TB whether to show footprint */
    Widget wantorb_w;		/* TB whether to show earth satellite orbit */
    Widget wantnoorb_w;		/* label spacer when orbit not appropriate */
    Widget showtr_w;		/* TB whether to show trail (if any) */
    Widget track_w;		/* TB whether to keep centered  */
    Widget pick_w;		/* TB mouse pick info refers to this object */
    Widget slat_w;		/* Label for Sub latitude */
    char *slat_pn;		/* persistent location for pick name userData */
    Widget slng_w;		/* Label for Sub longitude */
    char *slng_pn;		/* persistent location for pick name userData */
    Widget el_w;		/* Label for Elevation */
    char *el_pn;		/* persistent location for pick name userData */
    Widget range_w;		/* Label for Range */
    char *range_pn;		/* persistent location for pick name userData */
    Widget rrate_w;		/* Label for Range Rate */
    char *rrate_pn;		/* persistent location for pick name userData */
    Widget sun_w;		/* Label for whether sunlit */
    char *sun_pn;		/* persistent location for pick name userData */
    Widget age_w;		/* Label for TLE age */
    char *age_pn;		/* persistent location for pick name userData */
    Pixel pix;			/* color to draw */
    int shownbefore;		/* set after first shown */

    short ovis, ox, oy;		/* if orbit visible: screen x and y */
} EObj;


/* projection style */
typedef enum {
    CYLG,			/* cylindrical graphics */
    CYLI,			/* cylindrical image */
    SPHG,			/* spherical graphics */
    SPHI,			/* spherical image */
    WXMAP			/* special weather map */
} Proj;

static void e_create_shell (void);
static void e_create_ctrl (void);
static void e_buildfavs (void);
static void e_point_cb (Widget w, XtPointer client, XtPointer call);
static void e_stat_cb (Widget w, XtPointer client, XtPointer call);
static void e_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void e_close_cb (Widget w, XtPointer client, XtPointer call);
static void e_webupdate_cb (Widget w, XtPointer client, XtPointer call);
static void e_cclose_cb (Widget w, XtPointer client, XtPointer call);
static void e_show_cb (Widget w, XtPointer client, XtPointer call);
static void e_wantlbl_cb (Widget w, XtPointer client, XtPointer call);
static void e_wantfoot_cb (Widget w, XtPointer client, XtPointer call);
static void e_wantorb_cb (Widget w, XtPointer client, XtPointer call);
static void e_anim_cb (Widget w, XtPointer client, XtPointer call);
static void e_mloop_cb (Widget w, XtPointer client, XtPointer call);
static void e_set_buttons (int whether);
static void e_chelp_cb (Widget w, XtPointer client, XtPointer call);
static void e_mhelp_cb (Widget w, XtPointer client, XtPointer call);
static void e_helpon_cb (Widget w, XtPointer client, XtPointer call);
static void e_print_cb (Widget w, XtPointer client, XtPointer call);
static void e_print (void);
static void e_ps_annotate (Now *np);
static void e_ps_ll (char *tag, double lt, double lg, int x, int y);
static void e_exp_cb (Widget w, XtPointer client, XtPointer call);
static void e_motion_eh (Widget w, XtPointer client, XEvent *ev,
    Boolean *continue_to_dispatch);
static void e_copen_cb (Widget w, XtPointer client, XtPointer call);
static void e_tb_cb (Widget w, XtPointer client, XtPointer call);
static void e_ontrack_cb (Widget w, XtPointer client, XtPointer call);
static void e_onpick_cb (Widget w, XtPointer client, XtPointer call);
static void e_trail_cb (Widget w, XtPointer client, XtPointer call);
static void e_showtgl_cb (Widget w, XtPointer client, XtPointer call);
static void e_wantfoottgl_cb (Widget w, XtPointer client, XtPointer call);
static void e_wantlbltgl_cb (Widget w, XtPointer client, XtPointer call);
static void e_wantorbtgl_cb (Widget w, XtPointer client, XtPointer call);
static void e_wxreload_cb (Widget w, XtPointer client, XtPointer call);
static void e_proj_cb (Widget w, XtPointer client, XtPointer call);
static void e_setmain_cb (Widget w, XtPointer client, XtPointer call);
static void e_latlong_cb (Widget w, XtPointer client, XtPointer call);
static void e_lat0_cb (Widget w, XtPointer client, XtPointer call);
static void e_track_latlng (Widget w, int d, int xb, int yb, double lt,
    double lg);
static void e_popup (Widget w, XEvent *ev, unsigned d, unsigned xb,
    unsigned yb, int x, int y);
static void e_create_popup (void);
static void e_init_gcs (void);
static void e_copy_pm (void);
static void e_setelatlng (double lt, double lg);
static void e_subobject (Now *np, Obj *op, double *latp, double *longp);
static void e_show_esat_stats (EObj *eop);
static int e_coord (unsigned d, unsigned wb, unsigned hb, double pl,
    double pL, short *xp, short *yp);
static void e_drawgrid (unsigned d, unsigned wb, unsigned hb);
static void e_drawtrail (EObj *eop, int d, int wb, int hb);
static void e_draworbit (EObj *eop, int d, int wb, int hb);
static void e_drawname (EObj *eop, int d, int wb, int hb);
static void e_draworbname (EObj *eop, int x, int y);
static int e_drawcross (unsigned d, unsigned wb, unsigned hb, double lt,
    double lg, int style);
static int e_drawobject (Now *np, EObj *eop, int d, int wb, int hb);
static int e_drawfootprint (Now *np, EObj *eop, int d, int wb, int hb,
    double slat, double slng, double el);
static int e_drawcircle (Pixel pix, unsigned d, unsigned wb, unsigned hb,
    double slat, double slng, double rad);
static int e_set_dasize (void);
static int e_uncoord (Proj proj, unsigned d, unsigned xb, unsigned yb,
    int x, int y, double *ltp, double *lgp);
static void e_all (int statstoo);
static void e_getcircle (int *wp, int *hp, int *rp, int *xbp, int *ybp);
static void e_map (Now *np);
static void e_mainmenuloc (Now *np, unsigned d, unsigned wb, unsigned hb);
static void e_soleclipse (Now *np, unsigned d, unsigned wb, unsigned hb);
static int e_ano (double *latp, double *longp, int *xp, int *yp, int w2x,
    int arg);
static void e_sunlit (Now *np, unsigned int d, unsigned int wb,
    unsigned int hb);
static void e_msunlit (Now *np, unsigned int d, unsigned int wb,
    unsigned int hb);
static void e_ssunlit (Now *np, unsigned int d, unsigned int wb,
    unsigned int hb);
static void e_drawcontinents (unsigned d, unsigned wb, unsigned hb);
static void e_drawsites (unsigned d, unsigned wb, unsigned hb);
static int add_to_polyline (XPoint xp[], int xpsize, int i, int vis, int nxp,
    int max, int w, int x, int y);
static void e_viewrad (Now *np, double height, double alt, double *radp);
static void e_resettrail (EObj *eop, Now *np, int discard);
static int e_mktrail (TrTS ts[], TrState *statep, XtPointer client);
static Trail *e_growtrail (EObj *eop);
static int e_setupwxpm (int reload, int rebuild);
static int e_issunlit (Now *np, double l, double L);
static void noTrack (void);
static void mollweide_llxy (double l, double L, short *xp, short *yp);
static int mollweide_xyll (int x, int y, double *lp, double *Lp);
static void rd2ll (double ra, double dec, double *lp, double *Lp);
static void ll2rd (double l, double L, double *rap, double *decp);
static int loaddei (void);
static int loadnei (void);
static void drawei (unsigned int ww, unsigned int wh, unsigned int d,
    unsigned int xb, unsigned int yb);
static void e_greatCircle (double lt, double lg, double *dp, double *bp);
static void e_plotGreatCircle (unsigned d, unsigned wb, unsigned hb, double lt, double lg);

static Widget eshell_w;		/* main shell */
static Widget ectrl_w;		/* control shell */
static Widget e_da_w;		/* map DrawingArea widget */
static Widget e_dt_w;		/* main date/time stamp label widget */
static Widget e_cdt_w;		/* ctrl menu date/time stamp label widget */
static Pixmap e_pm;		/* use off screen pixmap for smoother drawing */
static Widget lat_w;		/* latitude scale widget */
static Widget lat0_w;		/* PB to set lat 0 */
static Widget long_w;		/* longitude scale widget */
static Widget zoom_w;		/* zoom scale widget */
static Widget objrc_w;		/* main RC in object control dialog */
static Widget cylg_w;		/* cylindrical graphics view toggle button */
static Widget sphg_w;		/* spherical graphics view toggle button */
static Widget cyli_w;		/* cylindrical image view toggle button */
static Widget sphi_w;		/* spherical image view toggle button */
static Widget wxm_w;		/* weather map view toggle button */
static Widget sites_w;		/* TB whether to show sites */
static Widget altlbl_w;		/* label for Altitude, showing units */
static Widget rangelbl_w;	/* label for Range, showing units */
static Widget rangerlbl_w;	/* label for Range Rate, showing units */
static Widget inertial_w;	/* TB whether to maintain inertial reference */
static Pixel e_bg;		/* bg color */
static GC e_strgc;		/* used for text within the drawing area */
static GC e_olgc;		/* used to draw map overlay details */
static GC e_gc;			/* used for all other GXCopy map drawing uses */
static XFontStruct *e_f;	/* used to compute string extents */
static Proj projection;		/* projection style */

/* describe each column.
 * N.B. meanings must agree with usage in e_buildfavs()
 */
static struct {
    Widget *l_wp;		/* label widget, if changes at runtime */
    char *label;		/* label */
    XtCallbackProc cb;		/* column-wide callback, if used */
    char *tip;			/* tip */
    Widget col_w;		/* the column RC */
} rhd[] = {
    {NULL,        "Object\n ",   NULL, "Name of object"},
    {NULL,        "Show \n ",    e_showtgl_cb,
	"Toggle the buttons below, which control whether to display object at all"},
    {NULL,        "Label\n ",    e_wantlbltgl_cb,
	"Toggle the buttons below, which control whether to show obj name"},
    {NULL,        "Foot\nPrint", e_wantfoottgl_cb,
    	"Toggle the buttons below, which control whether to show footprint"},
    {NULL,        "Orbit\n ",	 e_wantorbtgl_cb,
	"Toggle the buttons below, which control whether to show earth satellite orbit"},
    {NULL,        "Trail\n ",	 NULL,
    				"Define trail and whether to show time ticks"},
    {NULL,        "Track\n ",	 NULL, "Choose one object to stay centered"},
    {NULL,        "Popup\nRef",  NULL,
    				"Choose one object as reference in popup"},
    {NULL,        "Sub\nLat",	 NULL, "Latitude of object ground track, +N"},
    {NULL,        "Sub\nLong",   NULL, "Longitude of object ground track, +W"},
    {&altlbl_w,   "Alt\n ",      NULL, "Height above sea level"},
    {&rangelbl_w, "Range\n ",    NULL, "Distance from Main Site to object"},
    {&rangerlbl_w,"Range\nRate", NULL, "Rate of change of Range"},
    {NULL,        "Sun\nlit",    NULL,
    				"1 if object is in Sun light, 0 in shadow"},
    {NULL,        "Age\ndays",   NULL, "Age of TLE orbital element set, days"},
};

/* this is to save info about the popup: widgets, and current pointing dir.
 */
#define	MAXPUL	8		/* max label widgets we ever need at once */
typedef struct {
    Widget pu_w;		/* main Popup parent widget */
    Widget pu_labels[MAXPUL];	/* the misc labels */
    double pu_lt;		/* popup's latitude */
    double pu_lg;		/* popup's longitude */
    Site *pu_sp;		/* Site info if one nearby, else NULL */
} PopupInfo;
static PopupInfo pu_info;

/* info to get and keep track of the Earth color resources.
 * this does not include the trail colors.
 * N.B. the order of the entries in the enum must match the stuff in ecolors[].
 */
typedef enum {
    BORDERC, GRIDC, SITEC, ECLIPSEC, SUNC, HEREC, GREATC, NCOLORS
} EColors;
typedef struct {
    char *name;
    Pixel p;
} EColor;
static EColor ecolors[NCOLORS] = {
    {"EarthBorderColor"},
    {"EarthGridColor"},
    {"EarthSiteColor"},
    {"EarthEclipseColor"},
    {"EarthSunColor"},
    {"EarthHereColor"},
    {"EarthGreatCircleColor"},
};

/* info to keep track of all the "wants" toggle buttons */
typedef enum {		/* client codes for e_tb_cb and indices into wants[] */
    GRID, SITES, SUNLIGHT, LIVEDRAG, MAINMARKER, NITELITES, INERTIALF,
    NWANTS		/* used to count number of entries -- must be last */
} Wants;
static int wants[NWANTS];	/* fast state for each of the "wants" TB */

enum {SET_MAIN, SET_FROM_MAIN};	/* client codes for e_setmain_cb */
static char earthcategory[] = "Earth";		/* Save category */
static char etrres[] = "EarthTrailState";	/* trail resource */

static double elat, elng;	/* view center lat/long: +N +E rads */
static double selat, celat;	/* sin and cos of map elat */
static double ira, idec;	/* inertial frame ra/dec, rads */
static Site refsite;		/* current reference Site info, if siteok */
static int refsiteok;		/* whether site is valid */
static Obj **favs;              /* current list of loaded favorite objects */
static int nfavs;               /* n favs[] */
static EObj *eobjs;		/* malloced array of column info */
static int neobjs;		/* number of object columns, always >= nfavs */
static int e_selecting;		/* set while our fields are being selected */

/* handy ways to fiddle with TBs w/o invoking callbacks */
#define	TBISON(w)	XmToggleButtonGetState(w)
#define	TBISOFF(w)	(!TBISON(w))
#define	TBOFF(w)	XmToggleButtonSetState(w, False, False)
#define	TBON(w)		XmToggleButtonSetState(w, True, False)

/* constants that define details of various drawing actions */
#define	WIDE_LW		1		/* wide line width */
#define	NARROW_LW	0		/* narrow line width */
#define	LINE_ST		LineSolid	/* line style for lines */
#define	CAP_ST		CapRound	/* cap style for lines */
#define	JOIN_ST		JoinRound	/* join style for lines */
#define	CROSSH		1		/* drawcross code for a cross hair */
#define	PLUSS		2		/* drawcross code for a plus sign */
#define	PLLEN		degrad(3)	/* plussign arm radius, rads */
#define	CHLEN		degrad(1)	/* cross hair leg length, rads */
#define	LNGSTEP		15		/* longitude grid spacing, degrees */
#define	LATSTEP		15		/* latitude grid spacing, degrees */
#define	MAXPDIST	10		/* max screen pick distance, pixels */
#define	CYLASPECT	2.0		/* cylndrical view w/h ratio */
#define	TICKLN		2		/* length of tickmarks on trails */
#define	MOVIE_SS	(5./60)		/* movie step size, hours */
#define	MAXDW		1024		/* max dsp wdth, bewre large vir root */
#define	MAXZOOM		6.0		/* max zoom, earth diams */
#define	MINZOOM		0.05		/* min zoom, earth diams */
#define	ZOOMSCALE	100		/* max zoom scale */
#define	NORBSTEPS	256		/* number of orbit time steps */
#define	SATSZ		5		/* sat marker size, pixels */

/* default trail definition */
static TrState defts = {
    TRLR_2, TRI_5MIN, TRF_TIME, TRR_INTER, TRO_RIGHT, TRS_MEDIUM, 20
};

/* stuff for the weather ("wx") map gif
 * N.B. we assume image is simple cyl proj, N up, 180 long at left, E to right
 */
static char wxhost[] = "www.ssec.wisc.edu";
static char wxfile[] = "/data/comp/latest_cmoll.gif";
#define	WXM_W	640		/* overall gif width */
#define	WXM_H	480		/* overall gif height */
#define	WXM_LX	11		/* offset to left edge of map */
#define	WXM_RX	629		/* offset to right edge of map */
#define	WXM_TY  84		/* offset down to top of map */
#define	WXM_BY	396		/* offset down to bottom of map */
static unsigned char *wxgifpix;	/* malloced exploded gif pixel array */
static XColor wxxcols[256];	/* gif pixels and colors */
static Pixmap e_wxpm;		/* off screen pixmap for wx map */
static Widget wxreload_w;	/* PB to force reloading weather gif */

/* stuff for the surface maps */
static int deiw, deih;		/* size of daytime image */
static unsigned char *deipix;	/* malloced exploded daytime pixel array */
static XColor deixcols[256];	/* daytime colors */
static int neiw, neih;		/* size of nighttime image */
static unsigned char *neipix;	/* malloced exploded nighttime pixel array */
static XColor neixcols[256];	/* nighttime colors */

void
e_manage()
{
	if (!eshell_w) {
	    /* first call: create and init view to main menu's loc.
	     * rely on an expose to do the drawing.
	     */
	    Now *np = mm_get_now();

	    e_create_shell();
	    e_create_ctrl();
	    e_buildfavs();
	    e_setelatlng (lat, lng);
	}

	XtPopup (eshell_w, XtGrabNone);
	set_something (eshell_w, XmNiconic, (XtArgVal)False);

	setXRes (e_viewupres(), "1");
}

/* update the earth details.
 * remove all trail history if any.
 */
void
e_update(np, force)
Now *np;
int force;
{
	EObj *eop;

	/* don't bother if we've never been created */
	if (!eshell_w)
	    return;

	/* put np in trail list as current (first) entry.
	 * discard trail info if not being shown.
	 */
	for (eop = eobjs; eop < &eobjs[nfavs]; eop++)
	    e_resettrail(eop, np, !TBISON(eop->showtr_w));

	/* don't bother with map if we're not up now.
	 * still do stats though if anyone cares about our values.
	 */
	if (!isUp(eshell_w) && !force) {
	    if (any_ison())
		for (eop = eobjs; eop < &eobjs[nfavs]; eop++)
		    e_show_esat_stats (eop);
	    return;
	}

	e_all (1);
}

/* called when basic resources change.
 * rebuild and redraw.
 */
void
e_newres()
{
	if (!eshell_w)
	    return;
	e_init_gcs();
	e_update (mm_get_now(), 1);
}

int
e_ison()
{
	return (isUp(eshell_w));
}

/* called whenever the favorites list changes */
void
e_newfavs()
{
	if (!eshell_w)
	    return;

	/* rebuld rows */
	e_buildfavs();
}

/* called whenever the db beyond the builtin objects has changed.
 * if we've never been up or it was appended we don't have to do anything.
 * if it shrunk, that means some of the fav pointers might be different
 * so we rebuild the favorites list.
 */
void
e_newdb (appended)
int appended;
{
	if (!eshell_w)
	    return;

	/* rebuild rows */
	e_buildfavs();
}

/* called by other menus as they want to hear from our buttons or not.
 * the ons and offs stack.
 */
void
e_selection_mode (whether)
int whether;
{
	if (whether)
	    e_selecting++;
	else if (e_selecting > 0)
	    --e_selecting;

	if (eshell_w)
	    if ((whether && e_selecting == 1)	  /* first one to want on */
		|| (!whether && e_selecting == 0) /* first one to want off */)
		e_set_buttons (whether);
}

void
e_cursor(c)
Cursor c;
{
	Window win;

	if (eshell_w && (win = XtWindow(eshell_w)) != 0) {
	    Display *dsp = XtDisplay(eshell_w);
	    if (c)
		XDefineCursor(dsp, win, c);
	    else
		XUndefineCursor(dsp, win);
	}

	if (ectrl_w && (win = XtWindow(ectrl_w)) != 0) {
	    Display *dsp = XtDisplay(ectrl_w);
	    if (c)
		XDefineCursor(dsp, win, c);
	    else
		XUndefineCursor(dsp, win);
	}
}

/* return the name of the resource containing whether this view is up */
char *
e_viewupres()
{
	return ("EarthViewUp");
}

/* create the main form.
 * the earth stats form is created separately.
 */
static void
e_create_shell()
{
	typedef struct {
	    char *label;	/* what goes on the help label */
	    char *key;		/* string to call hlp_dialog() */
	} HelpOn;
	static HelpOn helpon[] = {
	    {"Intro...",	"Earth"},
	    {"on Mouse...",	"Earth_mouse"},
	    {"on Control...",	"Earth_control"},
	    {"on View...",	"Earth_view"},
	};
	typedef struct {
	    char *name;
	    char *label;
	    void (*cb)();
	    int client;
	    Widget *wp;
	    char *tip;
	} BtnInfo;
	static BtnInfo ctl[] = {
	    {"EPrint",  "Print...",      e_print_cb,   0, NULL,
		"Print the current Earth map"},
	    {"ECtrl", "Objects...",      e_copen_cb,   0, NULL,
		"Open window to define and overlay target Objects"},
	    {"EAnno", "User annotation...",      ano_cb,   0, NULL,
		"Open window to create and manage your own annotation"},
	    {"EMloop", "Add to movie...", e_mloop_cb,   0, NULL,
		"Add scene to movie loop"},
	    {"Sep0"},
	    {"SetMain", "Set Main",      e_setmain_cb, SET_MAIN, NULL,
		"Set location in Main menu to the current center location"},
	    {"SetFrom", "Set from Main", e_setmain_cb, SET_FROM_MAIN, NULL,
		"Set current center location to that of the Main menu"},
	    {"Animation",   "Animation demo",    e_anim_cb,   0, NULL,
		"Start/Stop a fun time-lapse animation"},
	    {"Sep1"},
	    {"Close",   "Close",         e_close_cb,   0, NULL,
		"Close this and all supporting dialogs"},
	};
	static BtnInfo view[] = {
	    {"grid",     "Grid",        e_tb_cb,  GRID, NULL,
	    	"Overlay map with Lat and Long grid spaced every 15 degrees"},
	    {"sites",    "Sites",       e_tb_cb,  SITES, &sites_w,
	    	"Mark each location in the Sites list on the map"},
	    {"sunlight", "Sunlight",    e_tb_cb,  SUNLIGHT, NULL,
	    	"Draw the area currently in sunlight in a different color"},
	    {"mainmarker", "Main marker",    e_tb_cb,  MAINMARKER, NULL,
	    	"Mark the location currently defined in the Main window"},
	    {"nightlights", "Nightside lights",    e_tb_cb,  NITELITES, NULL,
	    	"Show NASA image of lights at night seen from space"},
	    {"livedrag", "Live dragging",e_tb_cb,  LIVEDRAG, NULL,
		"Update scene while dragging scales, not just when release"},
	    {"inertial", "Inertial frame", e_tb_cb,  INERTIALF, &inertial_w,
		"Maintain current inertial reference frame"},
	};
	Widget pd_w, cas_w, mb_w;
	EventMask mask;
	Widget fr_w, eform_w;
	XmString str;
	Widget w;
	Arg args[20];
	int n;
	int i;

	/* create shell and form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNtitle, "xephem Earth view"); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNiconName, "Earth"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	eshell_w = XtCreatePopupShell ("Earth", topLevelShellWidgetClass,
							toplevel_w, args, n);

	setup_icon (eshell_w);
	set_something (eshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (eshell_w, XmNpopdownCallback, e_popdown_cb, 0);
	sr_reg (eshell_w, "XEphem*Earth.width", earthcategory, 0);
	sr_reg (eshell_w, "XEphem*Earth.height", earthcategory, 0);
	sr_reg (eshell_w, "XEphem*Earth.x", earthcategory, 0);
	sr_reg (eshell_w, "XEphem*Earth.y", earthcategory, 0);
	sr_reg (NULL, e_viewupres(), earthcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 2); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 2); n++;
	eform_w = XmCreateForm (eshell_w, "EarthForm", args, n);
	XtAddCallback (eform_w, XmNhelpCallback, e_mhelp_cb, 0);
	XtManageChild (eform_w);

	/* make the menu bar and all its pulldowns */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (eform_w, "MB", args, n);
	XtManageChild (mb_w);

	/* make the Control pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "ControlPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'C'); n++;
	    cas_w = XmCreateCascadeButton (mb_w, "ControlCB", args, n);
	    set_xmstring (cas_w, XmNlabelString, "Control");
	    XtManageChild (cas_w);

	    for (i = 0; i < XtNumber(ctl); i++) {
		BtnInfo *bip = &ctl[i];

		n = 0;
		if (!bip->label)
		    w = XmCreateSeparator (pd_w, bip->name, args, n);
		else {
		    if (bip->cb == e_mloop_cb)
			n += ml_addacc (args, n);
		    w = XmCreatePushButton (pd_w, bip->name, args, n);
		    set_xmstring (w, XmNlabelString, bip->label);
		    XtAddCallback (w, XmNactivateCallback, bip->cb,
						    (XtPointer)(long int)bip->client);
		}
		if (bip->wp)
		    *bip->wp = w;
		if (bip->tip)
		    wtip (w, bip->tip);
		XtManageChild (w);
	    }

	/* make the View pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "ViewPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'V'); n++;
	    cas_w = XmCreateCascadeButton (mb_w, "ViewCB", args, n);
	    set_xmstring (cas_w, XmNlabelString, "View");
	    XtManageChild (cas_w);

	    /* simulate a radio box for the projection selections */

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    cylg_w = XmCreateToggleButton (pd_w, "cylGraphic", args, n);
	    set_xmstring (cylg_w, XmNlabelString, "Cyl graphic");
	    XtAddCallback (cylg_w, XmNvalueChangedCallback, e_proj_cb,
							    (XtPointer)CYLG);
	    wtip (cylg_w, "Display graphical map with cylindrical projection");
	    XtManageChild (cylg_w);

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    cyli_w = XmCreateToggleButton (pd_w, "cylImage", args, n);
	    set_xmstring (cyli_w, XmNlabelString, "Cyl image");
	    XtAddCallback (cyli_w, XmNvalueChangedCallback, e_proj_cb,
							    (XtPointer)CYLI);
	    wtip (cyli_w, "Display image map with cylindrical projection");
	    XtManageChild (cyli_w);

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    sphg_w = XmCreateToggleButton (pd_w, "sphGraphic", args, n);
	    set_xmstring (sphg_w, XmNlabelString, "Sph graphic");
	    XtAddCallback (sphg_w, XmNvalueChangedCallback, e_proj_cb,
							    (XtPointer)SPHG);
	    wtip (sphg_w, "Display graphical with using spherical projection");
	    XtManageChild (sphg_w);

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    sphi_w = XmCreateToggleButton (pd_w, "sphImage", args, n);
	    set_xmstring (sphi_w, XmNlabelString, "Sph image");
	    XtAddCallback (sphi_w, XmNvalueChangedCallback, e_proj_cb,
							    (XtPointer)SPHI);
	    wtip (sphi_w, "Display image map with spherical projection");
	    XtManageChild (sphi_w);

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    wxm_w = XmCreateToggleButton (pd_w, "wxmap", args, n);
	    set_xmstring (wxm_w, XmNlabelString, "Weather map");
	    XtAddCallback (wxm_w, XmNvalueChangedCallback, e_proj_cb,
							    (XtPointer)WXMAP);
	    wtip (wxm_w, "Display graphics over a live weather map");
	    XtManageChild (wxm_w);

	    n = 0;
	    wxreload_w = XmCreatePushButton (pd_w, "wxmap", args, n);
	    set_xmstring (wxreload_w, XmNlabelString, "Reload map");
	    XtAddCallback (wxreload_w, XmNactivateCallback, e_wxreload_cb, 0);
	    wtip (wxreload_w, "Force reloading weather map gif file");
	    /* N.B. defer managing */

	    sr_reg (cyli_w, NULL, earthcategory, 1);
	    sr_reg (cylg_w, NULL, earthcategory, 1);
	    sr_reg (sphi_w, NULL, earthcategory, 1);
	    sr_reg (sphg_w, NULL, earthcategory, 1);
	    sr_reg (wxm_w, NULL, earthcategory, 1);

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    for (i = 0; i < XtNumber(view); i++) {
		BtnInfo *bip = &view[i];

		n = 0;
		if (!bip->label)
		    w = XmCreateSeparator (pd_w, bip->name, args, n);
		else {
		    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		    XtSetArg (args[n], XmNmarginHeight, 0); n++;
		    w = XmCreateToggleButton (pd_w, bip->name, args, n);
		    wants[bip->client] = TBISON(w);
		    set_xmstring (w, XmNlabelString, bip->label);
		    XtAddCallback (w, XmNvalueChangedCallback, bip->cb,
						    (XtPointer)(long int)bip->client);
		    sr_reg (w, NULL, earthcategory, 1);
		}

		if (bip->wp)
		    *bip->wp = w;
		if (bip->tip)
		    wtip (w, bip->tip);

		XtManageChild (w);
	    }

	    /* confirm ok for Sites to be on */
	    if (wants[SITES] && sites_get_list(NULL) < 0) {
		wants[SITES] = 0;
		TBOFF (sites_w);
		xe_msg (0, "No Sites File found.");
	    }

	/* make the help pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "HelpPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w);  n++;
	    XtSetArg (args[n], XmNmnemonic, 'H'); n++;
	    cas_w = XmCreateCascadeButton (mb_w, "HelpCB", args, n);
	    set_xmstring (cas_w, XmNlabelString, "Help");
	    XtManageChild (cas_w);
	    set_something (mb_w, XmNmenuHelpWidget, (XtArgVal)cas_w);

	    for (i = 0; i < XtNumber(helpon); i++) {
		HelpOn *hp = &helpon[i];

		str = XmStringCreate (hp->label, XmSTRING_DEFAULT_CHARSET);
		n = 0;
		XtSetArg (args[n], XmNlabelString, str); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		w = XmCreatePushButton (pd_w, "Help", args, n);
		XtAddCallback (w, XmNactivateCallback, e_helpon_cb,
							(XtPointer)(hp->key));
		XtManageChild (w);
		XmStringFree(str);
	    }

	/* 0 lat PB */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	lat0_w = XmCreatePushButton (eform_w, "Lat0", args, n);
	set_xmstring (lat0_w, XmNlabelString, "0:00");
	XtAddCallback (lat0_w, XmNactivateCallback, e_lat0_cb, NULL);
	wtip (lat0_w,"Set latitude to 0 (only active in Spherical projection)");
	XtManageChild (lat0_w);

	/* make the date/time indicator label */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, lat0_w); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	e_dt_w = XmCreateLabel (eform_w, "DTstamp", args, n);
	wtip (e_dt_w, "Date and Time for which map is computed");
	XtManageChild (e_dt_w);

	/* make the zoom, long and lat scales */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, lat0_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNminimum, -180); n++;
	XtSetArg (args[n], XmNmaximum, 179); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_LEFT); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNshowValue, True); n++;
	long_w = XmCreateScale (eform_w, "LongScale", args, n);
	wtip (long_w, "Longitude control");
	XtAddCallback (long_w, XmNvalueChangedCallback, e_latlong_cb, NULL);
	XtAddCallback (long_w, XmNdragCallback, e_latlong_cb, NULL);
	XtManageChild (long_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, long_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNminimum, 0); n++;
	XtSetArg (args[n], XmNmaximum, ZOOMSCALE); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNshowValue, False); n++;
	zoom_w = XmCreateScale (eform_w, "ZoomScale", args, n);
	wtip (zoom_w, "Zoom control");
	sr_reg (zoom_w, NULL, earthcategory, 0);
	XtAddCallback (zoom_w, XmNvalueChangedCallback, e_latlong_cb, NULL);
	XtAddCallback (zoom_w, XmNdragCallback, e_latlong_cb, NULL);
	XtManageChild (zoom_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, long_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNminimum, -90); n++;
	XtSetArg (args[n], XmNmaximum, 90); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
	XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNshowValue, True); n++;
	lat_w = XmCreateScale (eform_w, "LatScale", args, n);
	wtip (lat_w, "Latitude control (only active in Spherical projection)");
	XtAddCallback (lat_w, XmNvalueChangedCallback, e_latlong_cb, NULL);
	XtAddCallback (lat_w, XmNdragCallback, e_latlong_cb, NULL);
	XtManageChild (lat_w);

	/* make a drawing area on top */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, long_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, zoom_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, lat_w); n++;
	fr_w = XmCreateFrame (eform_w, "MFr", args, n);
	XtManageChild (fr_w);

	    n = 0;
	    e_da_w = XmCreateDrawingArea (fr_w, "EarthMap", args, n);
	    XtAddCallback (e_da_w, XmNexposeCallback, e_exp_cb, NULL);
	    mask = PointerMotionMask | ButtonPressMask | ButtonReleaseMask
				     | EnterWindowMask | LeaveWindowMask;
	    XtAddEventHandler (e_da_w, mask, False, e_motion_eh, 0);
	    XtManageChild (e_da_w);

	/* init da's size */
	(void) e_set_dasize();

	/* register trail resource and init setup */
	sr_reg (NULL, etrres, earthcategory, 0);
	tr_getres (etrres, &defts);

	/* if projection not unique, set default to spherical view */
	i = 0;
	i += TBISON(cylg_w) ? 1 : 0;
	i += TBISON(cyli_w) ? 1 : 0;
	i += TBISON(sphg_w) ? 1 : 0;
	i += TBISON(sphi_w) ? 1 : 0;
	i += TBISON(wxm_w)  ? 1 : 0;
	if (i != 1) {
	    xe_msg (0, "Earth projection conflict -- forcing Sph graphic");
	    TBOFF (cylg_w);
	    TBOFF (cyli_w);
	    TBON (sphg_w);
	    TBOFF (sphi_w);
	    TBOFF (wxm_w);
	}

	/* init projection info */
	if (TBISON(cylg_w)) {
	    projection = CYLG;
	    XtSetSensitive (lat_w, False);
	    XtSetSensitive (lat0_w, False);
	    XtSetSensitive (long_w, True);
	    XtSetSensitive (zoom_w, False);
	    XtUnmanageChild (wxreload_w);
	} else if (TBISON(cyli_w)) {
	    projection = CYLI;
	    XtSetSensitive (lat_w, False);
	    XtSetSensitive (lat0_w, False);
	    XtSetSensitive (long_w, True);
	    XtSetSensitive (zoom_w, False);
	    XtUnmanageChild (wxreload_w);
	} else if (TBISON(sphg_w)) {
	    projection = SPHG;
	    XtSetSensitive (lat_w, True);
	    XtSetSensitive (lat0_w, True);
	    XtSetSensitive (long_w, True);
	    XtSetSensitive (zoom_w, True);
	    XtUnmanageChild (wxreload_w);
	} else if (TBISON(sphi_w)) {
	    projection = SPHI;
	    XtSetSensitive (lat_w, True);
	    XtSetSensitive (lat0_w, True);
	    XtSetSensitive (long_w, True);
	    XtSetSensitive (zoom_w, True);
	    XtUnmanageChild (wxreload_w);
	} else {
	    projection = WXMAP;
	    XtSetSensitive (lat_w, False);
	    XtSetSensitive (lat0_w, False);
	    XtSetSensitive (long_w, False);
	    XtSetSensitive (zoom_w, False);
	    XtManageChild (wxreload_w);
	}
}

/* make the object control dialog -- real work is in e_buildfavs() */
static void
e_create_ctrl()
{
	Arg args[20];
	Widget sep_w;
	Widget eform_w;
	Widget w;
	int n;
	int i;

	/* create shell and form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Earth objects"); n++;
	XtSetArg (args[n], XmNiconName, "EObjs"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	ectrl_w = XtCreatePopupShell ("EarthObjs", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (ectrl_w);
	set_something (ectrl_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (ectrl_w, "XEphem*EarthObjs.x", earthcategory, 0);
	sr_reg (ectrl_w, "XEphem*EarthObjs.y", earthcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 5); n++;
	XtSetArg (args[n], XmNmarginWidth, 5); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	eform_w = XmCreateForm (ectrl_w, "EOForm", args, n);
	XtAddCallback (eform_w, XmNhelpCallback, e_chelp_cb, 0);
	XtManageChild (eform_w);

	/* make one big hor RC table to hold each column */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNadjustMargin, False); n++;
	objrc_w = XmCreateRowColumn (eform_w, "EMH", args, n);
	XtManageChild(objrc_w);

	    /* one RC for each column, first entry is heading label */

	    for (i = 0; i < XtNumber(rhd); i++) {
		n = 0;
		XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
		XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
		XtSetArg (args[n], XmNisAligned, False); n++;
		XtSetArg (args[n], XmNadjustMargin, False); n++;
		w = XmCreateRowColumn (objrc_w, "EVC", args, n);
		XtManageChild (w);
		rhd[i].col_w = w;

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
		w = XmCreatePushButton (rhd[i].col_w, "ECBL", args, n);
		set_xmstring (w, XmNlabelString, rhd[i].label);
		wtip (w, rhd[i].tip);
		if (rhd[i].cb) {
		    XtAddCallback (w, XmNactivateCallback, rhd[i].cb, NULL);
		    buttonAsButton (w, 1);
		} else
		    buttonAsButton (w, 0);
		XtManageChild (w);

		/* save the labels for the headers that change */
		if (rhd[i].l_wp)
		    *rhd[i].l_wp = w;
	    }

	/* add a separator at bottom */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, objrc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (eform_w, "Sep", args, n);
	XtManageChild (sep_w);

	/* add a label for the current date/time stamp */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	e_cdt_w = XmCreateLabel (eform_w, "SDTstamp", args, n);
	wtip (e_cdt_w, "Date and Time for which data are computed");
	XtManageChild (e_cdt_w);

	/* add a close button */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, e_cdt_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 30); n++;
	w = XmCreatePushButton (eform_w, "Close", args, n);
	wtip (w, "Close this window");
	XtAddCallback (w, XmNactivateCallback, e_cclose_cb, 0);
	XtManageChild (w);

	/* add web update button */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, e_cdt_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 40); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 60); n++;
	w = XmCreatePushButton (eform_w, "Close", args, n);
	set_xmstring (w, XmNlabelString, "Web update ...");
	wtip (w, "Update Earth satellites from web");
	XtAddCallback (w, XmNactivateCallback, e_webupdate_cb, 0);
	XtManageChild (w);

	/* add a Help button */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, e_cdt_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 70); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 90); n++;
	w = XmCreatePushButton (eform_w, "Help", args, n);
	wtip (w, "Get more info about this window");
	XtAddCallback (w, XmNactivateCallback, e_chelp_cb, 0);
	XtManageChild (w);
}

/* return a unique number between 0 and 1 for every non-neg value of i.
 * arrange for them to move along ever finer binary steps.
 */
static double
binsplit (int i)
{
	/* 1-based */
	double oneb = i + 1;

	/* divide into base power of two parts */
	double ndiv = pow (2.0, floor(log(oneb)/log(2.0)));

	/* add remainder, divide last evenly on both ends */
	return ((0.5 + oneb - ndiv)/ndiv);
}

/* set up one "row" in objrc for each favorite..
 * this means one row in each column.
 * remember the first row is just the col headers so leave it alone.
 * preserve existing entries based on same name.
 */
static void
e_buildfavs()
{
	Arg args[20];
	Widget w;
	EObj *eop;
	int i, n;
	char *rowname;

	/* get current set of favorites and columns */
	nfavs = fav_get_loaded (&favs);

	/* insure one row for each fav in each col and one EObj to match */
	for (i = 0; i < nfavs; i++) {
	    if (i >= neobjs) {
		/* create another row and its EObj */

		double r, g, b;
		XColor xc;

		eobjs = (EObj*) XtRealloc((char*)eobjs,(neobjs+1)*sizeof(EObj));
		eop = &eobjs[neobjs++];
		memset (eop, 0, sizeof(*eop));

		/* init mem for selecting names */

		eop->slat_pn = XtCalloc (PNL, 1);
		eop->slng_pn = XtCalloc (PNL, 1);
		eop->el_pn = XtCalloc (PNL, 1);
		eop->range_pn = XtCalloc (PNL, 1);
		eop->rrate_pn = XtCalloc (PNL, 1);
		eop->sun_pn = XtCalloc (PNL, 1);
		eop->age_pn = XtCalloc (PNL, 1);

		/* init trail options */

		eop->trstate = defts;

		/* pick a color */

		toRGB (1-binsplit (i), 1.0, 1.0, &r, &g, &b);
		xc.red = (int)(r*65535);
		xc.green = (int)(g*65535);
		xc.blue = (int)(b*65535);
		if (XAllocColor (XtD, xe_cm, &xc))
		    eop->pix = xc.pixel;
		else
		    eop->pix = WhitePixel (XtD, DefaultScreen(XtD));

		/* label for row name (really a PB just to match size) */

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
		eop->name_w = XmCreatePushButton (rhd[0].col_w, "EN", args, n);
		buttonAsButton (eop->name_w, 0);
		XtManageChild (eop->name_w);

		/* whether to display object at all */

		n = 0;
		XtSetArg (args[n], XmNshadowThickness, 0); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		XtSetArg (args[n], XmNselectColor, (XtArgVal)eop->pix); n++;
		XtSetArg (args[n], XmNfillOnSelect, (XtArgVal)True); n++;
		XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
		w = XmCreateToggleButton (rhd[1].col_w, "OTB", args, n);
		XtAddCallback (w, XmNvalueChangedCallback, e_show_cb,
								(XtPointer)(long int)i);
		set_xmstring (w, XmNlabelString, " ");
		XtManageChild (w);
		eop->show_w = w;

		/* whether to show Label */

		n = 0;
		XtSetArg (args[n], XmNshadowThickness, 0); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
		w = XmCreateToggleButton (rhd[2].col_w, "LblTB", args, n);
		XtAddCallback (w, XmNvalueChangedCallback, e_wantlbl_cb,
								(XtPointer)(long int)i);
		set_xmstring (w, XmNlabelString, " ");
		XtManageChild (w);
		eop->wantlbl_w = w;

		/* whether to show footprint */

		n = 0;
		XtSetArg (args[n], XmNshadowThickness, 0); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
		w = XmCreateToggleButton (rhd[3].col_w, "FootTB", args, n);
		XtAddCallback (w, XmNvalueChangedCallback, e_wantfoot_cb,
								(XtPointer)(long int)i);
		set_xmstring (w, XmNlabelString, " ");
		XtManageChild (w);
		eop->wantfoot_w = w;

		/* whether to show orbit ... */

		n = 0;
		XtSetArg (args[n], XmNshadowThickness, 0); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
		w = XmCreateToggleButton (rhd[4].col_w, "OrbitTB", args, n);
		XtAddCallback (w, XmNvalueChangedCallback, e_wantorb_cb,
								(XtPointer)(long int)i);
		set_xmstring (w, XmNlabelString, " ");
		eop->wantorb_w = w;

		/* .. or spacer when not appropriate */

		n = 0;
		w = XmCreatePushButton (rhd[4].col_w, "OrbitLbl", args, n);
		buttonAsButton (w, 0);
		set_xmstring (w, XmNlabelString, " ");
		eop->wantnoorb_w = w;

		/* whether to show Trail */

		n = 0;
		XtSetArg (args[n], XmNshadowThickness, 0); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
		w = XmCreateToggleButton (rhd[5].col_w, "TrailTB", args, n);
		XtAddCallback (w, XmNvalueChangedCallback, e_trail_cb,
								(XtPointer)(long int)i);
		set_xmstring (w, XmNlabelString, " ");
		XtManageChild (w);
		eop->showtr_w = w;

		/* whether to Track */

		n = 0;
		XtSetArg (args[n], XmNshadowThickness, 0); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
		w = XmCreateToggleButton (rhd[6].col_w, "TrackTB", args, n);
		XtAddCallback (w, XmNvalueChangedCallback, e_ontrack_cb,
								(XtPointer)(long int)i);
		set_xmstring (w, XmNlabelString, " ");
		XtManageChild (w);
		eop->track_w = w;

		/* whether to use as popup reference */

		n = 0;
		XtSetArg (args[n], XmNshadowThickness, 0); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
		w = XmCreateToggleButton (rhd[7].col_w, "MTB", args, n);
		XtAddCallback (w, XmNvalueChangedCallback, e_onpick_cb,
								(XtPointer)(long int)i);
		set_xmstring (w, XmNlabelString, " ");
		XtManageChild (w);
		eop->pick_w = w;

		/* label to show SubLat */

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		w = XmCreatePushButton (rhd[8].col_w, "LatTB", args, n);
		set_something (w, XmNuserData, (XtArgVal)eop->slat_pn);
		XtAddCallback (w, XmNactivateCallback, e_stat_cb, (XtPointer)(long int)i);
		XtManageChild (w);
		eop->slat_w = w;

		/* lable to show SubLong */

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		w = XmCreatePushButton (rhd[9].col_w, "LngTB", args, n);
		set_something (w, XmNuserData, (XtArgVal)eop->slng_pn);
		XtAddCallback (w, XmNactivateCallback, e_stat_cb, (XtPointer)(long int)i);
		XtManageChild (w);
		eop->slng_w = w;

		/* label to show Alt */

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		w = XmCreatePushButton (rhd[10].col_w, "AltTB", args, n);
		set_something (w, XmNuserData, (XtArgVal)eop->el_pn);
		XtAddCallback (w, XmNactivateCallback, e_stat_cb, (XtPointer)(long int)i);
		XtManageChild (w);
		eop->el_w = w;

		/* label to show Range */

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		w = XmCreatePushButton (rhd[11].col_w, "RanTB", args, n);
		XtAddCallback (w, XmNactivateCallback, e_stat_cb, (XtPointer)(long int)i);
		set_something(w, XmNuserData, (XtArgVal)eop->range_pn);
		XtManageChild (w);
		eop->range_w = w;

		/* label to show Range Rate */

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		w = XmCreatePushButton (rhd[12].col_w, "RRTB", args, n);
		XtAddCallback (w, XmNactivateCallback, e_stat_cb, (XtPointer)(long int)i);
		set_something(w, XmNuserData, (XtArgVal)eop->rrate_pn);
		XtManageChild (w);
		eop->rrate_w = w;

		/* label to show whether in Sun light */

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		w = XmCreatePushButton (rhd[13].col_w, "SunTB", args, n);
		XtAddCallback (w, XmNactivateCallback, e_stat_cb, (XtPointer)(long int)i);
		set_something (w, XmNuserData, (XtArgVal)eop->sun_pn);
		XtManageChild (w);
		eop->sun_w = w;

		/* label to show elements Age */

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		w = XmCreatePushButton (rhd[14].col_w, "AgeTB", args, n);
		XtAddCallback (w, XmNactivateCallback, e_stat_cb, (XtPointer)(long int)i);
		set_something (w, XmNuserData, (XtArgVal)eop->age_pn);
		XtManageChild (w);
		eop->age_w = w;

		/* probably not look like buttons yet */

		e_set_buttons(e_selecting);
	    }

	    /* build row, preserve controls if same name */

	    eop = &eobjs[i];
	    eop->op = favs[i];

	    get_xmstring (eop->name_w, XmNlabelString, &rowname);
	    if (strcmp (rowname, favs[i]->o_name)) {
		set_xmstring (eop->name_w, XmNlabelString, favs[i]->o_name);
		TBOFF (eop->show_w);
		TBOFF (eop->wantlbl_w);
		TBOFF (eop->wantfoot_w);
		TBOFF (eop->wantorb_w);
		TBOFF (eop->showtr_w);
		TBOFF (eop->track_w);
		TBOFF (eop->pick_w);
	    }
	    XtFree (rowname);

	    XtManageChild (eop->name_w);

	    XtManageChild (eop->show_w);

	    XtManageChild (eop->wantlbl_w);

	    XtManageChild (eop->wantfoot_w);

	    if (is_type (eop->op, EARTHSATM)) {
		XtManageChild (eop->wantorb_w);
		XtUnmanageChild (eop->wantnoorb_w);
	    } else {
		XtUnmanageChild (eop->wantorb_w);
		XtManageChild (eop->wantnoorb_w);
	    }

	    XtManageChild (eop->showtr_w);

	    XtManageChild (eop->track_w);

	    XtManageChild (eop->pick_w);

	    sprintf (eop->slat_pn, "%s.SatSubLat", favs[i]->o_name);
	    XtManageChild (eop->slat_w);

	    sprintf (eop->slng_pn, "%s.SatSubLong", favs[i]->o_name);
	    XtManageChild (eop->slng_w);

	    sprintf (eop->el_pn, "%s.SatAlt", favs[i]->o_name);
	    XtManageChild (eop->el_w);

	    sprintf (eop->range_pn, "%s.SatRange", favs[i]->o_name);
	    XtManageChild (eop->range_w);

	    sprintf (eop->rrate_pn, "%s.SatRangeR", favs[i]->o_name);
	    XtManageChild (eop->rrate_w);

	    sprintf (eop->sun_pn, "%s.SatSunlit", favs[i]->o_name);
	    XtManageChild (eop->sun_w);

	    sprintf (eop->age_pn, "%s.SatAge", favs[i]->o_name);
	    XtManageChild (eop->age_w);
	}

	/* turn off any remaining rows */
	for ( ; i < neobjs; i++) {
	    eop = &eobjs[i];
	    eop->op = NULL;
	    XtUnmanageChild (eop->name_w);
	    XtUnmanageChild (eop->show_w);
	    XtUnmanageChild (eop->wantlbl_w);
	    XtUnmanageChild (eop->wantfoot_w);
	    XtUnmanageChild (eop->wantorb_w);
	    XtUnmanageChild (eop->wantnoorb_w);
	    XtUnmanageChild (eop->showtr_w);
	    XtUnmanageChild (eop->track_w);
	    XtUnmanageChild (eop->pick_w);
	    XtUnmanageChild (eop->slat_w);
	    XtUnmanageChild (eop->slng_w);
	    XtUnmanageChild (eop->el_w);
	    XtUnmanageChild (eop->range_w);
	    XtUnmanageChild (eop->rrate_w);
	    XtUnmanageChild (eop->sun_w);
	    XtUnmanageChild (eop->age_w);
	}

	/* fill in current values */
	e_update (mm_get_now(), 1);
}

/* callback when any of the stat buttons are activated. */
/* ARGSUSED */
static void
e_stat_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (e_selecting) {
	    char *userD;

	    get_something (w, XmNuserData, (XtArgVal)&userD);
	    register_selection (userD);
	}
}

/* callback when the main form in popped down */
/* ARGSUSED */
static void
e_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown(ectrl_w);

	if (e_pm) {
	    XFreePixmap (XtDisplay(w), e_pm);
	    e_pm = (Pixmap) 0;
	}

	/* leave e_wxpm since it can take a while to reload */

	/* stop movie that might be running */
	mm_movie (0.0);

	/* record we are now down */
	setXRes (e_viewupres(), "0");
}

/* callback for when the main Close button is activated */
/* ARGSUSED */
static void
e_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* let popdown to the real work */
	XtPopdown (eshell_w);
}

/* callback for when the control dialog is closed */
/* ARGSUSED */
static void
e_cclose_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (ectrl_w);
}

/* callback to bring up web update window */
/* ARGSUSED */
static void
e_webupdate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	wdb_manage();
}

/* callback when a Track TB is changed.
 * client is index into eobjs[]
 */
/* ARGSUSED */
static void
e_ontrack_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	EObj *eop = &eobjs[(long int)client];

	/* do nothing if turning off */
	if (!TBISON(w))
	    return;

	/* insure shown as a friendly gesture */
	TBON(eop->show_w);
	eop->shownbefore = 1;
	    
	/* turn off others to simulate a radio box */
	for (eop = eobjs; eop < &eobjs[nfavs]; eop++)
	    if (w != eop->track_w && TBISON(eop->track_w))
		TBOFF (eop->track_w);

	/* turn off INERTIAL */
	if (wants[INERTIALF]) {
	    TBOFF (inertial_w);
	    wants[INERTIALF] = 0;
	}

	/* redraw to reorient */
	e_all(1);
}

/* callback when a Pick TB is changed.
 * client is index into eobjs[]
 */
/* ARGSUSED */
static void
e_onpick_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	EObj *eop;

	/* if turning this one on, turn off others to simulate a radio box */
	if (TBISON(w))
	    for (eop = eobjs; eop < &eobjs[nfavs]; eop++)
		if (w != eop->pick_w)
		    TBOFF (eop->pick_w);
}

/* callback when a Show TB is changed.
 * client is index into eobjs[]
 * N.B. logic is duplicated in e_showtgl_cb
 */
/* ARGSUSED */
static void
e_show_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	EObj *eop = &eobjs[(long int)client];

	/* courtesies */
	if (TBISON(w)) {
	    if (!eop->shownbefore) {
		TBON(eop->wantfoot_w);
		TBON(eop->wantlbl_w);
		TBON(eop->wantorb_w);
		eop->shownbefore = 1;
	    }
	} else {
	    TBOFF (eop->track_w);
	    TBOFF (eop->pick_w);
	}

	e_all(0);
}

/* callback when a Label TB is changed.
 * client is index into eobjs[]
 * N.B. logic is duplicated in e_wantlbltgl_cb
 */
/* ARGSUSED */
static void
e_wantlbl_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* if coming on, show as a courtesy */
	if (TBISON(w)) {
	    TBON (eobjs[(long int)client].show_w);
	    eobjs[(long int)client].shownbefore = 1;
	}

	e_all(0);
}

/* callback when a Footprint TB is changed.
 * client is index into eobjs[]
 * N.B. logic is duplicated in e_wantfoottgl_cb
 */
/* ARGSUSED */
static void
e_wantfoot_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* if coming on, show as a courtesy */
	if (TBISON(w)) {
	    TBON (eobjs[(long int)client].show_w);
	    eobjs[(long int)client].shownbefore = 1;
	}

	e_all(0);
}

/* callback when an orbit TB is changed.
 * client is index into eobjs[]
 * N.B. logic is duplicated in e_wantorbtgl_cb
 */
/* ARGSUSED */
static void
e_wantorb_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* if coming on, show as a courtesy */
	if (TBISON(w)) {
	    TBON (eobjs[(long int)client].show_w);
	    eobjs[(long int)client].shownbefore = 1;
	}

	e_all(0);
}

/* callback for when the Movie button is activated. */
/* ARGSUSED */
static void
e_anim_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	mm_movie (MOVIE_SS);
}

/* callback to add scene to movie loop */
/* ARGSUSED */
static void
e_mloop_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	ml_add (e_pm, e_dt_w);
}

/* go through all the buttons and set whether they
 * should appear to look like buttons or just flat labels.
 */
static void
e_set_buttons (whether)
int whether;	/* whether setting up for plotting or for not plotting */
{
	int i;

	for (i = 0; i < neobjs; i++) {
	    EObj *eop = eobjs + i;
	    buttonAsButton (eop->slat_w, whether);
	    buttonAsButton (eop->slng_w, whether);
	    buttonAsButton (eop->el_w, whether);
	    buttonAsButton (eop->range_w, whether);
	    buttonAsButton (eop->rrate_w, whether);
	    buttonAsButton (eop->sun_w, whether);
	    buttonAsButton (eop->age_w, whether);
	}
}

/* callback from the main Help all button
 */
/* ARGSUSED */
static void
e_mhelp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"This is a simple schematic depiction of the Earth surface at the given time.",
"The view shows the sunlit portion and the location of any object.",
};

	hlp_dialog ("Earth", msg, XtNumber(msg));
}

/* callback from the Control Help all button
 */
/* ARGSUSED */
static void
e_chelp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"Controls which objects are displayed and their current stats"
};

	hlp_dialog ("Earth_control_dialog", msg, XtNumber(msg));
}

/* callback from a specific Help button.
 * client is a string to use with hlp_dialog().
 */
/* ARGSUSED */
static void
e_helpon_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hlp_dialog ((char *)client, NULL, 0);
}

/* callback from Print control.
 */
/* ARGSUSED */
static void
e_print_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XPSAsk ("Earth", e_print);
}

/* proceed to generate a postscript file.
 * call XPSClose() when finished.
 */
static void
e_print ()
{
	Now *np = mm_get_now();
	int w, h, d, wb, hb;

	if (!e_ison()) {
	    xe_msg (1, "Earth must be open to save a file.");
	    XPSClose();
	    return;
	}

	watch_cursor(1);

	/* get size of the rendering area */
	e_getcircle (&w, &h, &d, &wb, &hb);

	/* draw in an area 6.5w x 6.5h centered 1in down from top */
	if (w >= h)
	    XPSXBegin (e_pm, 0, 0, w, h, 1*72, 10*72, (int)(6.5*72));
	else {
	    int pw = (int)(72*6.5*w/h+.5);	/* width on paper when 6.5 hi */
	    XPSXBegin (e_pm, 0, 0, w, h, (int)((8.5*72-pw)/2), 10*72, pw);
	}

	/* redraw */
	e_map (np);

	/* no more X captures */
	XPSXEnd();

	/* add some extra info */
	e_ps_annotate (np);

	/* finished */
	XPSClose();

	watch_cursor(0);
}

static void
e_ps_annotate (np)
Now *np;
{
	int unitspref = pref_get (PREF_UNITS);
	double sslat, sslong;
	char buf[32], buf2[32];
	char dir[128];
	EObj *eop;
	int hdr;
	int r, y;

	/* scroll up by number of favs to show */
	r = 11;
	for (eop = eobjs; eop < &eobjs[nfavs]; eop++)
	    if (TBISON(eop->show_w))
		r++;

	/* title */
	y = AROWY(r);
	(void) sprintf (dir, "(XEphem %s Earth View) 306 %d cstr\n",
				    projection == CYLG ? "Cylindrical graphic" :
				    projection == CYLI ? "Cylindrical image" :
				    projection == SPHG ? "Spherical graphic" :
				    projection == SPHI ? "Spherical image" :
					"Weather Map", y);
	XPSDirect (dir);

	/* date, time, jd */
	y = AROWY(r -= 2);
	fs_time (buf, mjd_hr(mjd));
	fs_date (buf2, pref_get(PREF_DATE_FORMAT), mjd_day(mjd));
	(void) sprintf (dir, "(UTC:) 144 %d rstr (%s  %s) 154 %d lstr\n",
							    y, buf, buf2, y);
	XPSDirect (dir);
	(void) sprintf (dir,"(JD:) 410 %d rstr (%13.5f) 420 %d lstr\n", y,
								mjd+MJD0, y);
	XPSDirect (dir);

	/* map center, subsolar loc */
	y = AROWY(--r);
	e_ps_ll ("Map Center", elat, elng, 144, y);
	e_subobject (np, db_basic(SUN), &sslat, &sslong);
	e_ps_ll ("SubSolar", sslat, sslong, 410, y);

	/* marker, if on */
	if (wants[MAINMARKER]) {
	    char *site = mm_getsite();
	    y = AROWY(--r);
	    e_ps_ll ("Reference", lat, lng, 144, y);
	    if (site) {
		sprintf (dir,"(, %s) show\n", XPSCleanStr(site,strlen(site)));
		XPSDirect (dir);
	    }
	}

	/* info about Targets that are shown */
	hdr = 0;
	r--;
	for (eop = eobjs; eop < &eobjs[nfavs]; eop++) {
	    if (TBISON(eop->show_w)) {
		Trail *tp = &eop->trail[0];
		Obj *op = &tp->t_obj;

		y = AROWY(--r);

		if (!hdr) {
		    int yup = AROWY(r+1);

		    sprintf (dir, "(SubLat/Long) 200 %d cstr\n", y);
		    XPSDirect(dir);
		    sprintf (dir, "(Sun) 285 %d cstr\n", y);
		    XPSDirect(dir);
		    sprintf (dir, "(Ref Alt/Az) 360 %d cstr\n", y);
		    XPSDirect(dir);

		    sprintf (dir, "(Alt) 418 %d cstr (%s) 418 %d cstr\n", yup,
				unitspref == PREF_ENGLISH ? "mi" : "km", y);
		    XPSDirect(dir);
		    sprintf (dir, "(Range) 450 %d cstr (%s) 450 %d cstr\n", yup,
				unitspref == PREF_ENGLISH ? "mi" : "km", y);
		    XPSDirect(dir);
		    sprintf (dir, "(Range') 488 %d cstr (%s) 488 %d cstr\n",yup,
				unitspref == PREF_ENGLISH ? "f/s" : "m/s", y);
		    XPSDirect(dir);
		    sprintf (dir, "(Age) 525 %d cstr (days) 525 %d cstr\n",
									yup, y);
		    XPSDirect(dir);
		    hdr = 1;

		    y = AROWY(--r);
		}

		e_ps_ll (op->o_name, tp->t_sublat, tp->t_sublng, 144, y);

		sprintf (dir, "(%s) 280 %d lstr\n", op->o_type == EARTHSAT
			    ? (op->s_eclipsed ? " N" : " Y")
			    : "", y);
		XPSDirect (dir);

		fs_sexa (buf, raddeg(op->s_alt), 3, 3600);
		fs_sexa (buf2, raddeg(op->s_az), 4, 3600);
		sprintf (dir, "(%s %s) 395 %d rstr\n", buf, buf2, y);
		XPSDirect (dir);

		if (op->o_type == EARTHSAT) {
		    double tmp;

		    if (unitspref == PREF_ENGLISH)
			tmp = op->s_elev*FTPM/5280.0;
		    else
			tmp = op->s_elev/1000.0;
		    sprintf (dir, "( %5.0f) 425 %d rstr\n", tmp, y);
		    XPSDirect (dir);

		    if (unitspref == PREF_ENGLISH)
			tmp = op->s_range*FTPM/5280.0;
		    else
			tmp = op->s_range/1000.0;
		    sprintf (dir, "( %5.0f) 462 %d rstr\n", tmp, y);
		    XPSDirect (dir);

		    if (unitspref == PREF_ENGLISH)
			tmp = op->s_rangev*FTPM;
		    else
			tmp = op->s_rangev;
		    sprintf (dir, "( %5.0f) 500 %d rstr\n", tmp, y);
		    XPSDirect (dir);

		    tmp = mjd - op->es_epoch;
		    sprintf (dir, "( %5.2f) 535 %d rstr\n", tmp, y);
		    XPSDirect (dir);
		}
	    }
	}
}

/* label print with center lat/long */
static void
e_ps_ll (tag, lt, lg, x, y)
char *tag;
double lt, lg;
int x, y;
{
	char ltstr[32], lgstr[32];
	char buf[128];

	fs_sexa (ltstr, raddeg(fabs(lt)), 3, 3600);
	(void) strcat (ltstr, lt < 0 ? " S" : " N");

	fs_sexa (lgstr, raddeg(fabs(lg)), 4, 3600);
	(void) strcat (lgstr, lg < 0 ? " W" : " E");

	(void) sprintf (buf, "(%s:) %d %d rstr (%s  %s) %d %d lstr\n",
					    tag, x, y, ltstr, lgstr, x+10, y);
	XPSDirect (buf);
}

/* called whenever the earth drawing area gets an expose.
 */
/* ARGSUSED */
static void
e_exp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static unsigned last_wid, last_hei;
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
	Display *dsp = XtDisplay(w);
	Window win = XtWindow(w);
	Window root;
	int x, y;
	unsigned int bw, d;
	unsigned wid, hei;

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
		swa.backing_store = NotUseful;
		XChangeWindowAttributes (e->display, e->window, mask, &swa);
		before = 1;
	    }

	    /* wait for the last in the series */
	    if (e->count != 0)
		return;
	    break;
	    }
	default:
	    printf ("Unexpected eshell_w event. type=%d\n", c->reason);
	    abort();
	}

	XGetGeometry (dsp, win, &root, &x, &y, &wid, &hei, &bw, &d);


	/* if no pixmap or we're a new size then (re)build everything */
	if (!e_pm || wid != last_wid || hei != last_hei) {
	    if (e_pm)
		XFreePixmap (dsp, e_pm);

	    if (!e_gc)
		e_init_gcs();

	    e_pm = XCreatePixmap (dsp, win, wid, hei, d);
	    XSetForeground (dsp, e_gc, e_bg);
	    XFillRectangle (dsp, e_pm, e_gc, 0, 0, wid, hei);
	    last_wid = wid;
	    last_hei = hei;

	    e_all(1);
	} else {
	    /* nothing new so just copy from the pixmap */
	    e_copy_pm();
	}
}

/* called on receipt of a MotionNotify or ButtonPress or ButtonRelease event.
 * button 1 shows great circle from Main site.
 * button 2 causes rotation, if LIVEDRAG;
 * button 3 causes a popup.
 */
/* ARGSUSED */
static void
e_motion_eh (w, client, ev, continue_to_dispatch)
Widget w;
XtPointer client;
XEvent *ev;
Boolean *continue_to_dispatch;
{
	static int inwin;
	Display *dsp = XtDisplay(w);
	Window win = XtWindow(w);
	int evt = ev->type;
	int mo  = evt == MotionNotify;
	int bp  = evt == ButtonPress;
	int br  = evt == ButtonRelease;
	int en  = evt == EnterNotify;
	int lv  = evt == LeaveNotify;
	int b3p = bp && ev->xbutton.button == Button3;
	int b2p = bp && ev->xbutton.button == Button2;
	int b2r = br && ev->xbutton.button == Button2;
	int b1r = br && ev->xbutton.button == Button1;
	int m2  = mo && ev->xmotion.state   & Button2Mask;
	int m1  = mo && ev->xmotion.state   & Button1Mask;
	int wide, h, d, xb, yb;
	Window root, child;
	double lt, lg;
	int rx, ry, wx, wy;
	int overearth;
	unsigned mask;

	if (en) inwin = 1;
	if (lv) inwin = 0;

	XQueryPointer (dsp, win, &root, &child, &rx, &ry, &wx, &wy, &mask);
	e_getcircle (&wide, &h, &d, &xb, &yb);
	overearth = e_uncoord (projection, d, xb, yb, wx, wy, &lt, &lg);

	if (m1 && overearth)
	    e_plotGreatCircle (d, xb, yb, lt, lg);
	if (b1r)
	    e_map(mm_get_now());

	if (b3p)
	    e_popup (w, ev, d, xb, yb, wx, wy);
	
	if ((b2p || m2) && wants[LIVEDRAG] && projection != WXMAP) {
	    static int lastx, lasty;
	    double lt, lg;

	    /* if first time turn off any tracking and change cursor */
	    if (b2p) {
		noTrack();
		lastx = wx;
		lasty = wy;
		XDefineCursor (dsp, win, XCreateFontCursor (dsp, XC_fleur));
	    }

	    /* update based on motion */
	    lg = elng - PI*(wx - lastx)/d;
	    lt = elat - PI*(lasty - wy)/d;
	    if (lt > PI/2)
		lt = PI/2;
	    else if (lt < -PI/2)
		lt = -PI/2;
	    lastx = wx;
	    lasty = wy;
	    e_setelatlng (lt, lg);

	    /* site code is certainly voided */
	    refsiteok = 0;

	    /* redraw */
	    e_all (0);
	}
	
	if (b2r)
	    XUndefineCursor(dsp, win);

	if (overearth && !m2 && !b1r) 	/* flashes badly while dragging */
	    e_track_latlng (w, d, xb, yb, lt, lg);
	else if (e_pm)	/* under a pulldown we can get called after popdown */
	    e_copy_pm();
}

/* called when any of the option toggle buttons changes state.
 * client is one of the Wants enum.
 */
/* ARGSUSED */
static void
e_tb_cb (wid, client, call)
Widget wid;
XtPointer client;
XtPointer call;
{
	int state = TBISON (wid);
	int want = (long int) client;
	int w, h, d, wb, hb;

	watch_cursor (1);

	/* set option to new value */
	wants[want] = state;

	/* prepare for graphics */
	e_getcircle (&w, &h, &d, &wb, &hb);

	/* generally we draw just what we need if turning on, everything if off.
	 * this is faster than e_all but can lead to a different overlay order.
	 */
	switch (want) {
	case GRID:
	    if (state) {
		e_drawgrid (d, wb, hb);
		e_copy_pm();
	    } else
		e_all (0);
	    break;

	case SITES:
	    if (state) {
		if (sites_get_list(NULL) < 0) {
		    wants[SITES] = 0;
		    TBOFF (sites_w);
		    xe_msg (1, "No Sites file found.");
		} else {
		    e_drawsites (d, wb, hb);
		    e_copy_pm();
		}
	    } else
		e_all (0);
	    break;

	case SUNLIGHT:
	    if (projection != WXMAP || e_setupwxpm(0, 1) == 0)
		e_all (0);

	case NITELITES:
	    if (projection != WXMAP || e_setupwxpm(0, 1) == 0)
		e_all (0);

	case LIVEDRAG:
	    break;	/* enough to just record it */

	case INERTIALF:
	    if (state) {
		/* capture current ra/dec overhead */
		ll2rd (elat, elng, &ira, &idec);
		/* turn off any tracking */
		noTrack();
	    }
	    break;

	case MAINMARKER:
	    if (state) {
		e_mainmenuloc (mm_get_now(), d, wb, hb);
		e_copy_pm();
	    } else
		e_all (0);
	    break;

	default:
	    printf ("e_tb_cb: bad client: %d\n", want);
	    abort();
	}

	watch_cursor (0);
}

/* called to open the object control window */
/* ARGSUSED */
static void
e_copen_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	e_set_buttons(e_selecting);
	XtPopup (ectrl_w, XtGrabNone);
	set_something (ectrl_w, XmNiconic, (XtArgVal)False);

}

/* called when a Trail PB is activated.
 * client is eobjs[] index.
 */
/* ARGSUSED */
static void
e_trail_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	EObj *eop = &eobjs[(long int)client];

	if (TBISOFF(w)) {
	    /* erase, if anything to erase */
	    if (eop->ntrail > 1)
		e_all(0);	
	} else {
	    /* turning on trail.. insure shown too as a friendly gesture */
	    if (TBISOFF(eop->show_w)) {
		TBON(eop->show_w);
		eop->shownbefore = 1;
	    }

	    /* allow to define if none */
	    if (eop->ntrail <= 1)
		tr_setup ("xephem Earth Trail setup",
			eop->trail[0].t_obj.o_name, &eop->trstate, e_mktrail,
			(XtPointer)client);

	    /* show */
	    e_all(0);
	}
}

/* called to toggle all buttons in the Show column.
 */
/* ARGSUSED */
static void
e_showtgl_cb (Widget w, XtPointer client, XtPointer call)
{
	int i;

	for (i = 0; i < neobjs; i++) {
	    EObj *eop = &eobjs[i];

	    if (TBISON(eop->show_w))
		TBOFF (eop->show_w);
	    else
		TBON (eop->show_w);

	    /* duplicate e_show_cb() but without e_all() */
	    if (TBISON(eop->show_w)) {
		if (!eop->shownbefore) {
		    TBON(eop->wantfoot_w);
		    TBON(eop->wantlbl_w);
		    eop->shownbefore = 1;
		}
	    } else {
		TBOFF (eop->track_w);
		TBOFF (eop->pick_w);
	    }
	}

	e_all(0);
}

/* called to toggle all buttons in the Label column.
 */
/* ARGSUSED */
static void
e_wantlbltgl_cb (Widget w, XtPointer client, XtPointer call)
{
	int i;

	for (i = 0; i < neobjs; i++) {
	    EObj *eop = &eobjs[i];

	    if (TBISON(eop->wantlbl_w))
		TBOFF (eop->wantlbl_w);
	    else
		TBON (eop->wantlbl_w);

	    /* duplicate e_wantlbl_cb() but without e_all() */
	    if (TBISON(eop->wantlbl_w)) {
		TBON (eop->show_w);
		eop->shownbefore = 1;
	    }
	}

	e_all(0);
}

/* called to toggle all buttons in the Foot Print column.
 */
/* ARGSUSED */
static void
e_wantfoottgl_cb (Widget w, XtPointer client, XtPointer call)
{
	int i;

	for (i = 0; i < neobjs; i++) {
	    EObj *eop = &eobjs[i];

	    if (TBISON(eop->wantfoot_w))
		TBOFF (eop->wantfoot_w);
	    else
		TBON (eop->wantfoot_w);

	    /* duplicate e_wantfoot_cb() but without e_all() */
	    if (TBISON(eop->wantfoot_w)) {
		TBON (eop->show_w);
		eop->shownbefore = 1;
	    }
	}

	e_all(0);
}

/* called to toggle all buttons in the Orbit column.
 */
/* ARGSUSED */
static void
e_wantorbtgl_cb (Widget w, XtPointer client, XtPointer call)
{
	int i;

	for (i = 0; i < neobjs; i++) {
	    EObj *eop = &eobjs[i];

	    if (TBISON(eop->wantorb_w))
		TBOFF (eop->wantorb_w);
	    else
		TBON (eop->wantorb_w);

	    /* duplicate e_wantorb_cb() but without e_all() */
	    if (TBISON(eop->wantorb_w)) {
		TBON (eop->show_w);
		eop->shownbefore = 1;
	    }
	}

	e_all(0);
}

/* called when the Reload weather map PB is pressed.
 */
/* ARGSUSED */
static void
e_wxreload_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (e_setupwxpm(1, 1) == 0)
	    e_all (0);
}

/* called when any projection button changes state.
 * client will be one of Proj.
 */
/* ARGSUSED */
static void
e_proj_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Proj newproj = (Proj)client;

	switch (newproj) {
	case CYLG:
	    TBON  (cylg_w);
	    TBOFF (cyli_w);
	    TBOFF (sphg_w);
	    TBOFF (sphi_w);
	    TBOFF (wxm_w);
	    XtSetSensitive (lat_w, False);
	    XtSetSensitive (lat0_w, False);
	    XtSetSensitive (long_w, True);
	    XtSetSensitive (zoom_w, False);
	    XtUnmanageChild (wxreload_w);
	    projection = newproj;
	    break;

	case CYLI:
	    TBOFF (cylg_w);
	    TBON  (cyli_w);
	    TBOFF (sphg_w);
	    TBOFF (sphi_w);
	    TBOFF (wxm_w);
	    XtSetSensitive (lat_w, False);
	    XtSetSensitive (lat0_w, False);
	    XtSetSensitive (long_w, True);
	    XtSetSensitive (zoom_w, False);
	    XtUnmanageChild (wxreload_w);
	    projection = newproj;
	    break;

	case SPHG:
	    TBOFF (cylg_w);
	    TBOFF (cyli_w);
	    TBON  (sphg_w);
	    TBOFF (sphi_w);
	    TBOFF (wxm_w);
	    XtSetSensitive (lat_w, True);
	    XtSetSensitive (lat0_w, True);
	    XtSetSensitive (long_w, True);
	    XtSetSensitive (zoom_w, True);
	    XtUnmanageChild (wxreload_w);
	    projection = newproj;
	    break;

	case SPHI:
	    TBOFF (cylg_w);
	    TBOFF (cyli_w);
	    TBOFF (sphg_w);
	    TBON  (sphi_w);
	    TBOFF (wxm_w);
	    XtSetSensitive (lat_w, True);
	    XtSetSensitive (lat0_w, True);
	    XtSetSensitive (long_w, True);
	    XtSetSensitive (zoom_w, True);
	    XtUnmanageChild (wxreload_w);
	    projection = newproj;
	    break;

	case WXMAP:
	    if (e_setupwxpm(0, 0) == 0) {
		TBOFF (cylg_w);
		TBOFF (sphg_w);
		TBON (wxm_w);
		XtSetSensitive (lat_w, False);
		XtSetSensitive (lat0_w, False);
		XtSetSensitive (long_w, False);
		XtSetSensitive (zoom_w, False);
		XtManageChild (wxreload_w);
		projection = newproj;
	    } else
		TBOFF (wxm_w);
	    break;
	}

	/* recompute the new size.
	 * if no change we call e_all() else leave it up to the expose.
	 */
	if (e_set_dasize() == 0)
	    e_all (0);
}

/* adjust the size of e_da_w for the current projection, if necessary.
 * return 1 if the size really does change, else 0.
 */
static int
e_set_dasize()
{
	Dimension nwid = 0, nhei = 0;
	Dimension wid, hei;
	int maxw;
	Position x;
	Arg args[10];
	int n;

	/* get max width, beware huge virtual screen size */
	maxw = DisplayWidth(XtD, DefaultScreen(XtD));
	if (maxw > MAXDW)
	    maxw = MAXDW;
	maxw = 9*maxw/10;

	/* get drawing area size now */
	n = 0;
	XtSetArg (args[n], XmNwidth, (XtArgVal)&wid); n++;
	XtSetArg (args[n], XmNheight, (XtArgVal)&hei); n++;
	XtGetValues (e_da_w, args, n);

	/* get shell position */
	n = 0;
	XtSetArg (args[n], XmNx, (XtArgVal)&x); n++;
	XtGetValues (eshell_w, args, n);

	/* change set nwid/hei to achieve desired aspect ratio, but never
	 * wider than screen.
	 */
	switch (projection) {
	case SPHG:	/* FALLTHRU */
	case SPHI:
	    /* 1:1 */
	    nhei = nwid = hei;
	    break;

	case CYLG:	/* FALLTHRU */
	case CYLI:
	    /* CYLASPECT:1 */
	    nhei = hei;
	    nwid = (Dimension)(hei*CYLASPECT + 0.5);
	    if (nwid > maxw) {
		nwid = maxw;
		nhei = (Dimension)(nwid/CYLASPECT + 0.5);
	    }
	    break;

	case WXMAP:
	    /* force size to fit map */
	    nhei = WXM_H;
	    nwid = WXM_W;
	    break;
	}

	/* react if either changed */
	if (nwid != wid || nhei != hei) {
	    /* change both at once to disuade multiple exposures */
	    n = 0;
	    XtSetArg (args[n], XmNwidth, (XtArgVal)nwid); n++;
	    XtSetArg (args[n], XmNheight, (XtArgVal)nhei); n++;
	    XtSetValues (e_da_w, args, n);

	    /* reposition if necessary to fit on screen */
	    if (x + nwid > maxw) {
		x = maxw - nwid + 10;
		n = 0;
		XtSetArg (args[n], XmNx, x); n++;
		XtSetValues (eshell_w, args, n);
	    }

	    return (1);
	}

	return (0);
}

/* called when Set Main or Set from Main is activated.
 * client is set to SET_MAIN or SET_FROM_MAIN.
 */
/* ARGSUSED */
static void
e_setmain_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Now *np = mm_get_now();
	int which = (long int)client;

	switch (which) {
	case SET_MAIN:
	    if (refsiteok)
		mm_setsite (&refsite, 1);
	    else
		mm_setll (elat, elng, 1);
	    break;
	case SET_FROM_MAIN:
	    e_setelatlng (lat, lng);
	    noTrack();
	    e_all (0);
	    break;
	default:
	    printf ("e_setmain_cb: Bad client: %d\n", which);
	    abort();
	}
}

/* called when either the longitude or latitude scale moves.
 * doesn't matter which -- just update pointing direction and redraw.
 * as a convenience, also insure tracking is off now.
 * also forget any remembered refsites name.
 */
/* ARGSUSED */
static void
e_latlong_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmScaleCallbackStruct *s = (XmScaleCallbackStruct *)call;

	if (wants[LIVEDRAG] || s->reason == XmCR_VALUE_CHANGED) {
	    int lt, lg;

	    XmScaleGetValue (lat_w, &lt);
	    XmScaleGetValue (long_w, &lg);
	    e_setelatlng (degrad(lt), degrad(-lg)); /*want +E but scale is +W */

	    /* manually sliding lat or long certainly voids a valid site code */
	    refsiteok = 0;

	    noTrack();

	    e_all (0);
	}
}

/* called to force latitude = 0.
 */
/* ARGSUSED */
static void
e_lat0_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int lg;

	XmScaleGetValue (long_w, &lg);
	e_setelatlng (0.0, degrad(-lg)); /*want +E but scale is +W */

	refsiteok = 0;

	noTrack();

	e_all (0);
}

/* called when the Point button is selected on the roaming popup.
 * get the details from pu_info.
 * center the point of view to point_lt/point_lg.
 * as a convenience, also turn off tracking.
 * save the site info, if there was one with this pick.
 */
/* ARGSUSED */
static void
e_point_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	e_setelatlng (pu_info.pu_lt, pu_info.pu_lg);
	noTrack();
	if ((refsiteok = !!pu_info.pu_sp))		/* might be NULL */
	    refsite = *pu_info.pu_sp;
	e_all (0);
}

/* make all the GCs, define all the e_*pix pixels and set the XFontStruct e_f
 * we get the color names and save the pixels in the global ecolors[] arrary.
 */
static void
e_init_gcs ()
{
	Display *dsp = XtDisplay (e_da_w);
	Window win = XtWindow (e_da_w);
	unsigned long gcm;
	XGCValues gcv;
	Pixel fg, p;
	int i;

	/* just in case we are not up yet */
	if (!win)
	    return;

	/* create the main gc. 
	 * it's GCForeground is set each time it's used.
	 */
	gcm = GCFunction;
	gcv.function = GXcopy;
	e_gc = XCreateGC (dsp, win, gcm, &gcv);

	/* get the foreground and background colors */
	get_something (e_da_w, XmNforeground, (XtArgVal)&fg);
	get_color_resource (e_da_w, "EarthBackground", &e_bg);

	/* collect all the Earth color resources */
	for (i = 0; i < XtNumber(ecolors); i++) {
	    EColor *ecp = &ecolors[i];
	    (void) get_color_resource (e_da_w, ecp->name, &ecp->p);
	}

	/* make the overlay gc, e_olgc */
	/* got all the colors -- overlay can just be drawn directly */
	gcm = GCFunction;
	gcv.function = GXcopy;
	e_olgc = XCreateGC (dsp, win, gcm, &gcv);

	/* make the lat/long string gc and get's it's font */

	gcm = GCForeground | GCBackground | GCFont;
	get_something (e_dt_w, XmNforeground, (XtArgVal)&p);
	gcv.foreground = p;
	gcv.background = e_bg;
	get_tracking_font (dsp, &e_f);
	gcv.font = e_f->fid;
	e_strgc = XCreateGC (dsp, win, gcm, &gcv);
}

/* copy the pixmap e_pm to the window of e_da_w. */
static void
e_copy_pm()
{
	Display *dsp = XtDisplay(e_da_w);
	Window win = XtWindow(e_da_w);
	int w, h, r, wb, hb;

	e_getcircle (&w, &h, &r, &wb, &hb);
	XCopyArea (dsp, e_pm, win, e_gc, 0, 0, w, h, 0, 0);
}

/* right button has been hit: find the closest thing to x/y and display what we
 * know about it. if nothing is very close, at least show the lat/long.
 * always give the choice to point to the spot.
 */
/* ARGSUSED */
static void
e_popup (wid, ev, d, xb, yb, x, y)
Widget wid;
XEvent *ev;
unsigned d;
unsigned xb, yb;
int x, y;
{
	Now *np = mm_get_now();
	int mind = 10000000;		/* min pick dist so far */
	Site *minsip = NULL;		/* closest site so far */
	Trail *mintp = NULL;		/* closest trail or orbit so far */
	double lmt;			/* local mean time: UT - radhr(lng) */
	double gst, lst;		/* Greenich and local sidereal times */
	double lt, lg;
	Trail *tp;
	EObj *eop;
	char buf[128];
	Widget w;
	int i, nl;

	/* admit we might be busy for a while */
	watch_cursor (1);

	/* make the popup if this is our first time */
	if (!pu_info.pu_w)
	    e_create_popup();

	/* find the closest site in minsip, if sites are enabled */
	if (wants[SITES]) {
	    Site *sipa;
	    int nsa;

	    for (nsa = sites_get_list(&sipa); --nsa >= 0; ) {
		Site *sip = &sipa[nsa];
		short sx, sy;

		if (e_coord (d, xb, yb, sip->si_lat, sip->si_lng, &sx, &sy)) {
		    int md = abs(sx-x) + abs(sy-y);
		    if (md < mind) {
			minsip = sip;
			mind = md;
		    }
		}
	    }
	}

	/* now check object positions, including any trail markers and orbits,
	 *   for anything closer still. if so, put it in mintp.
	 * remember: the object in its current position is also on trail[].
	 */
	for (eop = eobjs; eop < &eobjs[nfavs]; eop++) {
	    if (TBISOFF(eop->show_w))
		continue;
	    for (tp = eop->trail; tp && tp < &eop->trail[eop->ntrail]; tp++) {
		if (tp->lvis) {
		    int md = abs(tp->lx-x) + abs(tp->ly-y);
		    if (md < mind) {
			mintp = tp;
			mind = md;
		    }
		}
	    }
	    if (eop->ovis) {
		int md = abs(eop->ox-x) + abs(eop->oy-y);
		if (md < mind) {
		    mintp = &eop->trail[0];
		    mind = md;
		}
	    }
	}

	/* build the popup */

	/* first unmanage all the labels -- we'll turn on what we want */
	for (i = 0; i < MAXPUL; i++)
	    XtUnmanageChild (pu_info.pu_labels[i]);
	nl = 0;

	/* assume no sites entry for now */
	pu_info.pu_sp = NULL;

	if (mind <= MAXPDIST) {
	    /* found something, question is: a trailed object or a site?
	     * check them in the opposite order than which they were searched.
	     */
	    if (mintp) {
		double tzo = pref_get(PREF_ZONE)==PREF_LOCALTZ ? -tz/24.0 : 0;
		Obj *op = &mintp->t_obj;

		/* a trail item was closest.
		 * put the name, time, lat and lng in the popup.
		 */
		w = pu_info.pu_labels[nl++];
		set_xmstring (w, XmNlabelString, mintp->t_obj.o_name);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		(void) sprintf (buf, "Date: ");
		fs_date (buf+strlen(buf), pref_get(PREF_DATE_FORMAT),
					    mjd_day(mintp->t_now.n_mjd+tzo));
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		(void) sprintf (buf, "Time: ");
		fs_time (buf+strlen(buf), mjd_hr(mintp->t_now.n_mjd+tzo));
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		if (pref_get (PREF_UNITS) == PREF_ENGLISH)
		    sprintf (buf, "Alt: %.0f mi", op->s_elev*FTPM/5280.);
		else
		    sprintf (buf, "Alt: %.0f km", op->s_elev/1000.);
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		if (pref_get (PREF_UNITS) == PREF_ENGLISH)
		    sprintf (buf, "Range: %.0f mi", op->s_range*FTPM/5280.);
		else
		    sprintf (buf, "Range: %.0f km", op->s_range/1000.);
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		(void) sprintf (buf, "Sub Lat: ");
		fs_dms_angle (buf+strlen(buf), mintp->t_sublat);
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		(void) sprintf (buf, "Sub Long: ");
		fs_dms_angle (buf+strlen(buf), -mintp->t_sublng); /* want +W */
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		if (is_type(&mintp->t_obj,EARTHSATM)) {
		    w = pu_info.pu_labels[nl++];
		    (void) sprintf (buf, "Sunlit: %s",
					mintp->t_obj.s_eclipsed ? "No" : "Yes");
		    set_xmstring (w, XmNlabelString, buf);
		    XtManageChild (w);
		}

		/* save lat/long for Point and if want to display alt/az */
		pu_info.pu_lt = mintp->t_sublat;
		pu_info.pu_lg = mintp->t_sublng;
	    } else if (minsip) {
		/* a site entry was closest.
		 * put it's name, lat/long and alt/az in the popup.
		 */
		double dist, bear;

		w = pu_info.pu_labels[nl++];
		set_xmstring (w, XmNlabelString, minsip->si_name);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		(void) sprintf (buf, "LMT: ");
		lmt = mjd_hr(mjd) + radhr(minsip->si_lng);
		range (&lmt, 24.0);
		fs_time (buf+strlen(buf), lmt);
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		(void) sprintf (buf, "LST: ");
		utc_gst (mjd_day(mjd), mjd_hr(mjd), &gst);
		lst = gst + radhr(minsip->si_lng);
		range (&lst, 24.0);
		fs_time (buf+strlen(buf), lst);
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		(void) sprintf (buf, "Lat: ");
		fs_dms_angle (buf+strlen(buf), minsip->si_lat);
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		w = pu_info.pu_labels[nl++];
		(void) sprintf (buf, "Long: ");
		fs_dms_angle (buf+strlen(buf), -minsip->si_lng); /* want +W */
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		e_greatCircle (minsip->si_lat, minsip->si_lng, &dist, &bear);
		w = pu_info.pu_labels[nl++];
		if (pref_get (PREF_UNITS) == PREF_ENGLISH)
		    (void) sprintf (buf, "Dist: %.1f mi", dist*0.62137119e-3);
		else
		    (void) sprintf (buf, "Dist: %.1f km", dist*1e-3);
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);
		w = pu_info.pu_labels[nl++];
		(void) sprintf (buf, "Bearing: ");
		fs_dms_angle (buf+strlen(buf), bear);
		set_xmstring (w, XmNlabelString, buf);
		XtManageChild (w);

		/* save lat/long for Point and if want to display alt/az */
		pu_info.pu_lt = minsip->si_lat;
		pu_info.pu_lg = minsip->si_lng;

		/* now we have site info too */
		pu_info.pu_sp = minsip;
	    } else {
		printf ("e_popup: unknown closest! mind = %d\n", mind);
		abort();
	    }
	} else {
	    /* nothing was close enough.
	     * just put the lat/long and alt/az of the cursor loc in the popup.
	     * if not over the earth, forget it.
	     */
	    double dist, bear;

	    if (!e_uncoord (projection, d, xb, yb, x, y, &lt, &lg)) {
		watch_cursor(0);
		return;
	    }
	    w = pu_info.pu_labels[nl++];
	    (void) sprintf (buf, "LMT: ");
	    lmt = mjd_hr(mjd) + radhr(lg);
	    range (&lmt, 24.0);
	    fs_time (buf+strlen(buf), lmt);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);

	    w = pu_info.pu_labels[nl++];
	    (void) sprintf (buf, "LST: ");
	    utc_gst (mjd_day(mjd), mjd_hr(mjd), &gst);
	    lst = gst + radhr(lg);
	    range (&lst, 24.0);
	    fs_time (buf+strlen(buf), lst);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);

	    w = pu_info.pu_labels[nl++];
	    (void) sprintf (buf, "Lat: ");
	    fs_dms_angle (buf+strlen(buf), lt);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);

	    w = pu_info.pu_labels[nl++];
	    (void) sprintf (buf, "Long: ");
	    fs_dms_angle (buf+strlen(buf), -lg);	/* want +W */
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);

	    e_greatCircle (lt, lg, &dist, &bear);
	    w = pu_info.pu_labels[nl++];
	    if (pref_get (PREF_UNITS) == PREF_ENGLISH)
		(void) sprintf (buf, "Dist: %.1f mi", dist*0.62137119e-3);
	    else
		(void) sprintf (buf, "Dist: %.1f km", dist*1e-3);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);
	    w = pu_info.pu_labels[nl++];
	    (void) sprintf (buf, "Bearing: ");
	    fs_dms_angle (buf+strlen(buf), bear);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);

	    /* save lat/long for Point and if want to display alt/az */
	    pu_info.pu_lt = lt;
	    pu_info.pu_lg = lg;

	    /* signal no trail found for pick_w work */
	    mintp = NULL;
	}

	/* unless we found a trail entry, show alt/az of pick_w object (if any)
	 */
	for (eop = eobjs; eop < &eobjs[nfavs]; eop++)
	    if (TBISON(eop->pick_w))
		break;
	if (!mintp && eop < &eobjs[nfavs]) {
	    Now now_here;			/* temp Now */
	    Obj obj_here;			/* temp Obj */

	    (void) memcpy ((void *)&now_here, (void *)&eop->trail[0].t_now,
	    							sizeof (Now));
	    now_here.n_lat = pu_info.pu_lt;
	    now_here.n_lng = pu_info.pu_lg;
	    (void) memcpy ((void *)&obj_here, (void *)&eop->trail[0].t_obj,
	    							sizeof (Obj));
	    (void) obj_cir (&now_here, &obj_here);

	    w = pu_info.pu_labels[nl++];
	    (void) sprintf (buf, "%s Alt: ", obj_here.o_name);
	    fs_pangle (buf+strlen(buf), obj_here.s_alt);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);

	    w = pu_info.pu_labels[nl++];
	    (void) sprintf (buf, "%s Az: ", obj_here.o_name);
	    fs_pangle (buf+strlen(buf), obj_here.s_az);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);
	}

	if (nl > MAXPUL) {
	    printf ("Too many earth popup labels: %d of %d\n", nl, MAXPUL);
	    abort();
	}

	XmMenuPosition (pu_info.pu_w, (XButtonPressedEvent *)ev);
	XtManageChild (pu_info.pu_w);

	watch_cursor(0);
}

/* create the popup
 * save all its widgets in the pu_info struct.
 */
static void
e_create_popup()
{
	Widget w;
	Arg args[20];
	int n;
	int i;

	/* make the popup shell which includes a handy row/column */
	n = 0;
	XtSetArg (args[n], XmNisAligned, True); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
	pu_info.pu_w = XmCreatePopupMenu (e_da_w, "EPopup", args, n);

	/* add the max labels we'll ever need -- all unmanaged for now. */
	for (i = 0; i < MAXPUL; i++) {
	    n = 0;
	    pu_info.pu_labels[i] = XmCreateLabel (pu_info.pu_w, "EPUL",args,n);
	}

	/* and add the separator and the "Point" button.
	 * we always wants these so manage them now.
	 */
	w = XmCreateSeparator (pu_info.pu_w, "PUSep", args, 0);
	XtManageChild (w);
	w = XmCreatePushButton (pu_info.pu_w, "Point", args, 0);
	XtAddCallback (w, XmNactivateCallback, e_point_cb, 0);
	XtManageChild (w);
}

/* given diameter and x/y border display info at the given lat/long
 */
static void
e_track_latlng (w, d, xb, yb, lt, lg)
Widget w;
int d;
int xb, yb;
double lt, lg;
{
	static int last_lst_w, last_lmt_w, last_ll_w, last_db_w;
	int topy = projection == WXMAP ? WXM_TY : 0;
	int boty = projection == WXMAP ? WXM_BY : d + 2*yb;
	int wide = d + 2*xb;
	Now *np = mm_get_now();
	Window win = XtWindow(w);
	char lst_buf[64], lmt_buf[64], lng_buf[64], lat_buf[64];
	char dist_buf[64], bear_buf[64];
	Pixel fg, bg;
	XCharStruct all;
	double dist, bear;
	double lst, gst, lmt;
	int dir, asc, des;
	int len;

	/* great circle info */
	e_greatCircle (lt, lg, &dist, &bear);

	/* fill in the strings */
	(void) strcpy (bear_buf, "Bearing: ");
	fs_dms_angle (bear_buf+strlen(bear_buf), bear);
	if (pref_get (PREF_UNITS) == PREF_ENGLISH)
	    (void) sprintf (dist_buf, "Distance: %7.1f mi", dist*0.62137119e-3);
	else
	    (void) sprintf (dist_buf, "Distance: %7.1f km", dist*1e-3);
	(void) strcpy (lat_buf, " Lat: ");
	fs_dms_angle (lat_buf+strlen(lat_buf), lt);
	(void) strcpy (lng_buf, "Long: ");
	fs_dms_angle (lng_buf+strlen(lng_buf), -lg);	/* want +W */
	lmt = mjd_hr(mjd) + radhr(lg);
	range (&lmt, 24.0);
	(void) strcpy (lmt_buf, "LMT: ");
	fs_time (lmt_buf+strlen(lmt_buf), lmt);
	utc_gst (mjd_day(mjd), mjd_hr(mjd), &gst);
	lst = gst + radhr(lg);
	range (&lst, 24.0);
	(void) strcpy (lst_buf, "LST: ");
	fs_time (lst_buf+strlen(lst_buf), lst);

	/* set colors */
	fg = ecolors[SUNC].p;
	bg = e_bg;

	/* draw the strings, first erasing the previous */

	/* lat/long in upper left */
	len = strlen (lat_buf);
	XTextExtents (e_f, lat_buf, len, &dir, &asc, &des, &all);
	if (all.width > last_ll_w)
	    last_ll_w = all.width;
	len = strlen (lng_buf);
	XTextExtents (e_f, lng_buf, len, &dir, &asc, &des, &all);
	if (all.width > last_ll_w)
	    last_ll_w = all.width;
	XSetForeground (XtD, e_strgc, bg);
	XFillRectangle (XtD, win, e_strgc, 0, topy, last_ll_w, 2*(asc+des));
	XSetForeground (XtD, e_strgc, fg);
	XDrawString (XtD, win, e_strgc, 0, topy+asc, lat_buf, strlen(lat_buf)); 
	XDrawString (XtD, win, e_strgc, 0, topy+2*asc+des, lng_buf, strlen(lng_buf)); 

	/* dist/bear in upper right */
	len = strlen (dist_buf);
	XTextExtents (e_f, dist_buf, len, &dir, &asc, &des, &all);
	if (all.width > last_db_w)
	    last_db_w = all.width;
	len = strlen (bear_buf);
	XTextExtents (e_f, bear_buf, len, &dir, &asc, &des, &all);
	if (all.width > last_db_w)
	    last_db_w = all.width;
	XSetForeground (XtD, e_strgc, bg);
	XFillRectangle (XtD, win, e_strgc, wide-last_db_w, topy, last_db_w, 2*(asc+des));
	XSetForeground (XtD, e_strgc, fg);
	XDrawString (XtD, win, e_strgc, wide-last_db_w, topy+asc, dist_buf, strlen(dist_buf)); 
	XDrawString (XtD, win, e_strgc, wide-last_db_w, topy+2*asc+des, bear_buf, strlen(bear_buf)); 

	/* lmt in lower left */
	len = strlen (lmt_buf);
	XTextExtents (e_f, lmt_buf, len, &dir, &asc, &des, &all);
	if (all.width > last_lmt_w)
	    last_lmt_w = all.width;
	XSetForeground (XtD, e_strgc, bg);
	XFillRectangle (XtD, win, e_strgc, 0, boty-(asc+des), last_lmt_w,
								asc+des);
	XSetForeground (XtD, e_strgc, fg);
	XDrawString (XtD, win, e_strgc, 0, boty-des, lmt_buf, len);

	/* lst in lower right */
	len = strlen (lst_buf);
	XTextExtents (e_f, lst_buf, len, &dir, &asc, &des, &all);
	if (all.width > last_lst_w)
	    last_lst_w = all.width;
	XSetForeground (XtD, e_strgc, bg);
	XFillRectangle (XtD, win, e_strgc, wide-last_lst_w, boty-(asc+des),
						    last_lst_w, asc+des);
	XSetForeground (XtD, e_strgc, fg);
	XDrawString (XtD, win, e_strgc, wide-last_lst_w, boty-des,
							    lst_buf, len);
}

/* set the elat/selat/celat/elng values, taking care to put them into ranges
 * -PI/2 .. elat .. PI/2 and -PI .. elng .. PI.
 */
static void
e_setelatlng (lt, lg)
double lt, lg;
{
	elat = lt;
	selat = sin(elat);
	celat = cos(elat);
	elng = lg;
	range (&elng, 2*PI);
	if (elng > PI)
	    elng -= 2*PI;

	/* capture new inertial frame if on */
	if (wants[INERTIALF])
	    ll2rd (elat, elng, &ira, &idec);
}

/* return the lat/lng of the subobject point (lat/long of where the object
 * is directly overhead) at np.
 * both are in rads, lat is +N, long is +E.
 */
static void
e_subobject(np, op, latp, longp)
Now *np;
Obj *op;
double *latp, *longp;
{
	if (op->o_type == EARTHSAT) {
	    /* these are all ready to go */
	    *latp = op->s_sublat;
	    *longp = op->s_sublng;
	} else {
	    double gst;
	    double ra, dec;

	    ra = op->s_gaera;
	    dec = op->s_gaedec;

	    *latp = dec;

	    utc_gst (mjd_day(mjd), mjd_hr(mjd), &gst);
	    *longp = ra - hrrad(gst);	/* remember: we want +E */
	    range (longp, 2*PI);
	    if (*longp > PI)
		*longp -= 2*PI;
	}
}

/* find the rate of change of op->s_edist in m/s
 */
static double
e_edist_rate (Now *np, Obj *op)
{
	double h = 0.5/24.0;			/* step size, days */
	double e0, e1;				/* edist @ mjd-h and mjd+h */
	Now myn = *np;				/* copy for modified times */

	myn.n_mjd = mjd - h;
	obj_cir (&myn, op);
	e0 = op->s_edist;

	myn.n_mjd = mjd + h;
	obj_cir (&myn, op);
	e1 = op->s_edist;

	return ((MAU/SPD)*(e1-e0)/(2*h));
}

/* show the stats for the object in the given trow in its current
 * position. if it's of type EARTHSAT then display its s_ fields; otherwise
 * turn off the display of these items.
 */
static void
e_show_esat_stats (eop)
EObj *eop;
{
	Now *np = mm_get_now();
	Obj *op = &eop->trail[0].t_obj;
	int unitspref = pref_get (PREF_UNITS);
	int dok = dateOK (np, op) == 0;

	/* all objects show sublocation unless invalid date */
	if (dok) {
	    f_dms_angle (eop->slat_w, eop->trail[0].t_sublat);
	    f_dms_angle (eop->slng_w, -eop->trail[0].t_sublng);
	} else {
	    f_string (eop->slat_w, " ");
	    f_string (eop->slng_w, " ");
	}

	if (dok && op->o_type == EARTHSAT) {
	    if (unitspref == PREF_ENGLISH) {
		f_showit (altlbl_w, "Alt\nmiles");
		f_double (eop->el_w, "%7.5g", op->s_elev*FTPM/5280.0);
	    } else {
		f_showit (altlbl_w, "Alt\nkm");
		f_double (eop->el_w, "%7.5g", op->s_elev/1000.0);
	    }

	    if (unitspref == PREF_ENGLISH) {
		f_showit (rangelbl_w, "Range\nmiles");
		f_double (eop->range_w, "%7.5g", op->s_range*FTPM/5280.0);
	    } else {
		f_showit (rangelbl_w, "Range\nkm");
		f_double (eop->range_w, "%7.5g", op->s_range/1000.0);
	    }

	    if (unitspref == PREF_ENGLISH) {
		f_showit (rangerlbl_w, "Range'\nft/s");
		f_double (eop->rrate_w, "%7.5g", op->s_rangev*FTPM);
	    } else {
		f_showit (rangerlbl_w, "Range'\nm/s");
		f_double (eop->rrate_w, "%7.5g", op->s_rangev);
	    }

	    f_double (eop->sun_w, "%.0f", op->s_eclipsed ? 0.0 : 1.0);

	    f_double (eop->age_w, "%.2f", mjd - op->es_epoch);

	} else if (dok && is_type(op,ELLIPTICALM|HYPERBOLICM|PARABOLICM|PLANETM)) {
	    f_string (eop->el_w, " ");
	    f_string (eop->range_w, " ");

	    if (unitspref == PREF_ENGLISH) {
		f_showit (rangerlbl_w, "Range'\nft/s");
		f_double (eop->rrate_w, "%7.5g", e_edist_rate (np, op)*FTPM);
	    } else {
		f_showit (rangerlbl_w, "Range'\nm/s");
		f_double (eop->rrate_w, "%7.5g", e_edist_rate (np, op));
	    }

	    f_string (eop->sun_w, " ");
	    f_string (eop->age_w, " ");

	} else {
	    f_string (eop->el_w, " ");
	    f_string (eop->range_w, " ");
	    f_string (eop->rrate_w, " ");
	    f_string (eop->sun_w, " ");
	    f_string (eop->age_w, " ");
	}
}

/* given a lat/long now find ra/dec overhead */
static void
ll2rd (double l, double L, double *rap, double *decp)
{
	Now *np = mm_get_now();
	double gst;

	utc_gst (mjd_day(mjd), mjd_hr(mjd), &gst);
	*rap = hrrad(gst) + L;		/* because overhead HA == 0 */
	range (rap, 2*PI);
	*decp = l;
}

/* given a ra/dec return lat/long where it is overhead */
static void
rd2ll (double ra, double dec, double *lp, double *Lp)
{
	Now *np = mm_get_now();
	double gst;

	utc_gst (mjd_day(mjd), mjd_hr(mjd), &gst);
	*Lp = ra - hrrad(gst);		/* because overhead HA == 0 */
	range (Lp, 2*PI);
	*lp = dec;
}

/* given:
 *   the current viewpoint coords (from globals selat/celat/elng)
 *   diameter and borders of circle in current window
 *   latitude and longitude of a point p: pl, pL;
 *   projection
 * find:
 *   x and y coords of point on screen
 * always return the coordinates, but return 1 if the point is visible or 0 if
 *   it's over the limb.
 */   
static int
e_coord (d, wb, hb, pl, pL, xp, yp)
unsigned d;		/* diameter of drawing surface circle */
unsigned wb, hb;	/* width/height borders between circle and edges */
double pl, pL;		/* point p lat, rads, +N; long, rads, +E */
short *xp, *yp;		/* projected location onto drawing surface */
{
	switch (projection) {
	case CYLI: /* FALLTHRU */
	case CYLG: {
	    unsigned w = d + 2*wb;
	    unsigned h = d + 2*hb;
	    double dL;

	    // *yp = (int)floor(h/2.0 * (1.0 - sin(pl)) + 0.5);
	    *yp = (int)floor(h/2.0 * (1.0 - 2*pl/PI) + 0.5);

	    dL = pL - elng;
	    range(&dL, 2*PI);
	    if (dL > PI)  dL -= 2*PI;
	    *xp = (int)floor(w/2.0 * (1.0 + dL/PI) + 0.5);

	    return (1);
	    }

	case SPHI: /* FALLTHRU */
	case SPHG: {
	    double sR, cR;	/* R is radius from viewpoint to p */
	    double A, sA,cA;	/* A is azimuth of p as seen from viewpoint */
	    unsigned int r = d/2;

	    solve_sphere (pL - elng, PI/2 - pl, selat, celat, &cR, &A);
	    sR = sqrt(1.0 - cR*cR);
	    sA = sin(A);
	    cA = cos(A);

	    *xp = wb + r + (int)floor(r*sR*sA + 0.5);
	    *yp = hb + r - (int)floor(r*sR*cA + 0.5);

	    return (cR > -.0001);
	    }

	case WXMAP: {
	    mollweide_llxy (pl, pL, xp, yp);
	    return (1);
	    }
	}

	return (0);
}

/* draw circles onto e_pm within circle of diameter d and width and height
 * borders wb and hb showing where the satellite is seen as being 0, 30 and 60
 * degrees altitude above horizon as well.
 * also draw a crosshair at the actual location.
 * return 1 if any part was visible, else 0.
 */
static int
e_drawfootprint (np, eop, d, wb, hb, slat, slng, el)
Now *np;		/* circumstances */
EObj *eop;		/* object info */
int d, wb, hb;		/* dia and width/height borders of circle to use */
double slat, slng;	/* satellite's lat and lng */
double el;		/* satellite's elevation above surface, m */
{
	double rad;	/* viewing altitude radius, rads */
	int isvis;

	e_viewrad (np, el, degrad(0.0), &rad);
	isvis = e_drawcircle (eop->pix, d, wb, hb, slat, slng, rad);
	e_viewrad (np, el, degrad(30.0), &rad);
	e_drawcircle (eop->pix, d, wb, hb, slat, slng, rad);
	e_viewrad (np, el, degrad(60.0), &rad);
	e_drawcircle (eop->pix, d, wb, hb, slat, slng, rad);
	e_drawcross (d, wb, hb, slat, slng, CROSSH);

	return (isvis);
}

/* assuming the current vantage point of elat/elng, draw a circle around the
 *   given lat/long point of given angular radius.
 * all angles in rads.
 * we do it by sweeping an arc from the given location and collecting the end
 *   points. beware going over the horizon or wrapping around the edge.
 * return 1 if any part of the circle is visible, else 0.
 */
static int
e_drawcircle (pix, d, wb, hb, slat, slng, rad)
Pixel pix;		/* drawing color */
unsigned d, wb, hb;	/* dia and width/height borders of circle to use */
double slat, slng;	/* lat/long of object */
double rad;		/* angular radius of circle to draw */
{
#define	MAXVCIRSEGS	62
	XPoint xp[MAXVCIRSEGS+1];
	double cosc = cos(PI/2 - slat);	/* cos/sin of N-pole-to-slat angle */
	double sinc = sin(PI/2 - slat);
	double b = rad;			/* sat-to-arc angle */
	double A;			/* subobject azimuth */
	double cosa;			/* cos of pole-to-arc angle */
	double B;			/* subobj-to-view angle from pole */
	int nvcirsegs = MAXVCIRSEGS;
	int nxp = 0;
	int isvis;
	int w = d + 2*wb;
	int vis;
	int i;

	/* use the overlay gc, wide lines, pix color */
	XSetLineAttributes (XtD, e_olgc, NARROW_LW, LINE_ST, CAP_ST, JOIN_ST);
	XSetForeground (XtD, e_olgc, pix);

	isvis = 0;
	for (i = 0; i <= nvcirsegs; i++) {
	    short x, y;
	    A = 2*PI/nvcirsegs * i;
	    solve_sphere (A, b, cosc, sinc, &cosa, &B);
	    vis = e_coord (d, wb, hb, PI/2-acos(cosa), B+slng, &x, &y);
	    if (vis)
		isvis = 1;

	    nxp = add_to_polyline (xp, XtNumber(xp), i,vis,nxp,nvcirsegs,w,x,y);
	}

	return (isvis);
}

/* draw a little crosshair or plussign at the given location with the e_olgc
 * return whether visible
 */
static int
e_drawcross (d, wb, hb, lt, lg, style)
unsigned d, wb, hb;	/* dia and width/height borders of circle to use */
double lt, lg;		/* desired center location */
int style;		/* CROSSH or PLUSS */
{
	double lats[4], lngs[4];	/* lats and longs of endpoints */
	XSegment xs[4];			/* one for each cardinal direction */
	double a, cosa;			/* north-to-cross from view */
	double B;			/* cross-width from north */
	short sx, sy;			/* x and y of the center */
	int w = d + 2*wb;
	int linew = 0;
	int nxs;
	int vis;
	int xwrap;
	int i;

	/* find location of center of cross-hair */
	vis = e_coord (d, wb, hb, lt, lg, &sx, &sy);
	if (!vis)
	    return (0);	/* center is not visible so forget the rest */

	/* find longitude sweep to produce given e/w arc */

	switch (style) {
	case CROSSH:
	    solve_sphere (PI/4, CHLEN, sin(lt), cos(lt), &cosa, &B);
	    a = acos(cosa);
	    lats[0] = PI/2-a;		lngs[0] = lg+B;
	    lats[1] = PI/2-a;		lngs[1] = lg-B;
	    solve_sphere (3*PI/4, CHLEN, sin(lt), cos(lt), &cosa, &B);
	    a = acos(cosa);
	    lats[2] = PI/2-a;		lngs[2] = lg+B;
	    lats[3] = PI/2-a;		lngs[3] = lg-B;
	    linew = NARROW_LW;
	    break;
	case PLUSS:
	    solve_sphere (PI/2, PLLEN, sin(lt), cos(lt), &cosa, &B);
	    a = acos(cosa);
	    lats[0] = PI/2-a;		lngs[0] = lg+B;
	    lats[1] = PI/2-a;		lngs[1] = lg-B;
	    lats[2] = lt-PLLEN;		lngs[2] = lg;
	    if (lats[2] < -PI/2) {
		lats[2] = -PI - lats[2];lngs[2] += PI;	/* went under S pole */
	    }
	    lats[3] = lt+PLLEN;		lngs[3] = lg;
	    if (lats[3] > PI/2) {
		lats[3] = PI - lats[3];	lngs[3] += PI;	/* went over N pole */
	    }
	    linew = WIDE_LW;
	    break;
	default:
	    printf ("e_drawcross: bad style: %d\n", style);
	    abort();
	}

	nxs = 0;
	for (i = 0; i < 4; i++) {
	    short x, y;
	    vis = e_coord (d, wb, hb, lats[i], lngs[i], &x, &y);

	    if (projection != SPHG && projection != SPHI
					    && (sx - x > w/2 || x - sx > w/2))
		xwrap = 1;
	    else
		xwrap = 0;

	    if (vis && !xwrap) {
		xs[nxs].x1 = x;
		xs[nxs].y1 = y;
		xs[nxs].x2 = sx;
		xs[nxs].y2 = sy;
		nxs++;
	    }
	}

	if (nxs > 0) {
	    XSetLineAttributes (XtD, e_olgc, linew, LINE_ST, CAP_ST, JOIN_ST);
	    XPSDrawSegments (XtD, e_pm, e_olgc, xs, nxs);
	}

	return (1);
}

/* draw the lat/long grid lines */
static void
e_drawgrid (d, wb, hb)
unsigned d, wb, hb;	/* dia and width/height borders of circle to use */
{
#define	MAXGRIDSEGS	60		/* max segments per grid line */
	int nsegs = MAXGRIDSEGS;
	XPoint xp[MAXGRIDSEGS+1];
	int mxp = XtNumber(xp);
	int w = d + 2*wb;
	int i, j;

	/* use the overlay GC, narrow lines, GRIDC color */
	XSetLineAttributes (XtD, e_olgc, NARROW_LW, LINE_ST, CAP_ST, JOIN_ST);
	XSetForeground (XtD, e_olgc, ecolors[GRIDC].p);

	/* draw each line of constant longitude */
	for (i = 0; i < 360/LNGSTEP; i++) {
	    double lg = i * degrad(LNGSTEP);
	    int nxp = 0;
	    for (j = 0; j <= nsegs; j++) {
		short x, y;
		double lt = degrad (90 - j*180.0/nsegs);
		int vis = e_coord (d, wb, hb, lt, lg, &x, &y);
		nxp = add_to_polyline (xp, mxp, j, vis, nxp, nsegs, w, x, y);
	    }
	}

	/* draw each line of constant latitude -- beware of x wrap */
	for (i = 1; i < 180/LATSTEP; i++) {
	    double lt = degrad (90 - i*LATSTEP);
	    int nxp = 0;
	    for (j = 0; j <= nsegs; j++) {
		short x, y;
		double lg = degrad(180.0 - j*360.0/nsegs);
		int vis = e_coord (d, wb, hb, lt, lg, &x, &y);
		nxp = add_to_polyline (xp, mxp, j, vis, nxp, nsegs, w, x, y);
	    }
	}
}

/* given projection, diam and size of window and X-coord x/y, return lat/long.
 * see e_getcircle() for meanings in each projection.
 * return 1 if inside the map, else 0.
 */
static int
e_uncoord (proj, d, xb, yb, x, y, ltp, lgp)
Proj proj;		/* which projection to use */
unsigned int d;		/* diameter of earth limb, pixels */
unsigned xb, yb;	/* borders around circle */
int x, y;		/* X-windows coords of loc */
double *ltp, *lgp;	/* resulting lat/long, rads */
{
	switch (proj) {

	case CYLI:	/* FALLTHRU */
	case CYLG: {
	    int maxx = d + 2*xb;
	    int maxy = d + 2*yb;

	    if (x < 0 || x >= maxx || y < 0 || y >= maxy)
		return (0);

	    //*ltp = asin (1.0 - 2.0*y/maxy);
	    *ltp = PI * (1.0 - 2.0*y/maxy) / 2;

	    *lgp = 2*PI*(x - maxx/2)/maxx + elng;
	    if (*lgp >  PI) *lgp -= 2*PI;
	    if (*lgp < -PI) *lgp += 2*PI;

	    return (1);
	    }

	case SPHI:	/* FALLTHRU */
	case SPHG: {
	    double X, Y;/* x/y but signed from center +x right +y up, pixels*/
	    double R;	/* pixels x/y is from center */
	    double b;	/* angle from viewpoint to x/y */
	    double A;	/* angle between N pole and x/y as seen from viewpt */
	    double a, ca;	/* angle from N pole to x/y */
	    double B;	/* angle from viewpoint to x/y as seen from N pole */
	    unsigned int r = d/2;

	    X = x - (int)(r + xb);
	    Y = (int)(r + yb) - y;
	    R = sqrt (X*X + Y*Y);
	    if (R >= r)
		return (0);

	    if (R < 1.0) {
		*ltp = elat;
		*lgp = elng;
	    } else {
		b = asin (R/r);
		A = atan2 (X, Y);
		solve_sphere (A, b, selat, celat, &ca, &B);
		a = acos (ca);
		*ltp = PI/2 - a;
		*lgp = elng + B;
		range (lgp, 2*PI);
		if (*lgp > PI)
		    *lgp -= 2*PI;
	    }
	    
	    return (1);
	    }
	
	case WXMAP: {
	    return (mollweide_xyll (x, y, ltp, lgp));
	    }

	default:
	    printf ("Unknown earth projection: %d\n", proj);
	    abort();
	}
}

/* Turn off track_w in all eobjs */
static void
noTrack()
{
	EObj *eop;

	for (eop = eobjs; eop < &eobjs[neobjs]; eop++)
	    TBOFF (eop->track_w);
}

/* draw everything: fields (always) and the picture (if managed).
 * if statstoo then redraw the info table too. making this explicit allows
 * for faster redraw when we know full well the stats have not changed.
 */
static void
e_all (statstoo)
int statstoo;
{
	Now *np = mm_get_now();
	EObj *eop;

	watch_cursor(1);

	/* point at it if tracking is on */
	for (eop = eobjs; eop < &eobjs[nfavs]; eop++) {
	    if (TBISON (eop->track_w)) {
		e_setelatlng (eop->trail[0].t_sublat, eop->trail[0].t_sublng);
		break;	/* there can only be one On */
	    }
	}

	/* if want inertial frame, use it to set lat/long */
	if (wants[INERTIALF]) {
	    double l, L;
	    rd2ll (ira, idec, &l, &L);
	    e_setelatlng (l, L);
	}

	/* update the scale -- remember we want +W longitude, -180..+179 */
	XmScaleSetValue (lat_w, (int)floor(raddeg(elat)+0.5));
	XmScaleSetValue (long_w, (((int)floor(raddeg(-elng)+.5)+180)%360)-180);

	/* display the picture now if we are up for sure.
	 * N.B. don't just use whether eshell_w is up: if we are looping
	 * and get managed drawing occurs before the first expose makes the pm.
	 */
	if (e_pm) {
	    e_map (np);
	    e_copy_pm();
	}

	/* update stats.
	 * N.B. do after graphics to avoid flashing
	 */
	if (statstoo)
	    for (eop = eobjs; eop < &eobjs[nfavs]; eop++)
		e_show_esat_stats (eop);

	/* put up the current time */
	timestamp (np, e_dt_w);
	timestamp (np, e_cdt_w);

	watch_cursor(0);
}

/* find size and borders for drawing earth.
 * if spherical, center a circle of max size that completely fits.
 * if cylindrical, use entire rectangle.
 * if wxmap, assume correct (fixed) size, r is horizontal half-width.
 */
static void
e_getcircle (wp, hp, dp, xbp, ybp)
int *wp, *hp;			/* overall width and height */
int *dp;			/* circle diameter */
int *xbp, *ybp;			/* x and y border */
{
	Display *dsp = XtDisplay(e_da_w);
	Window win = XtWindow(e_da_w);
	Window root;
	unsigned int bw, d;
	int x, y;
	int z;

	if (projection == WXMAP) {
	    *wp = WXM_W;
	    *hp = WXM_H;
	    *dp = WXM_W - WXM_LX;
	    *xbp = WXM_LX;
	    *ybp = WXM_TY;
	    return;
	}

	XGetGeometry (dsp, win, &root, &x, &y, (unsigned int *)wp,
					    (unsigned int *)hp, &bw, &d);

	*dp = *wp > *hp ? *hp : *wp;	/* diameter is smaller dimension */
	XmScaleGetValue (zoom_w, &z);
	*dp = (int)(*dp * MINZOOM * pow(MAXZOOM/MINZOOM,(double)z/ZOOMSCALE));
	*xbp = (*wp - *dp)/2;		/* x border */
	*ybp = (*hp - *dp)/2;		/* y border */

}

/* draw everything centered at elat/elng onto e_pm.
 */
static void
e_map (np)
Now *np;
{
	Display *dsp = XtDisplay(e_da_w);
	int w, h, d, wb, hb;
	EObj *eop;

	/* get size of the rendering area */
	e_getcircle (&w, &h, &d, &wb, &hb);

	/* setup the background and night on screen, but don't print */
	switch (projection) {
	case CYLI:
	    if ((!deipix && loaddei() < 0) || (!neipix && loadnei() < 0))
		return;	/* bzzt */
	    drawei(w, h, d, wb, hb);
	    XSetForeground (dsp, e_gc, e_bg);
	    XPSPixmap (e_pm, w, h, xe_cm, e_gc, 0);
	    break;

	case CYLG:
	    XSetForeground (dsp, e_gc, e_bg);
	    XFillRectangle (dsp, e_pm, e_gc, 0, 0, w, h);
	    break;

	case SPHI:
	    if ((!deipix && loaddei() < 0) || (!neipix && loadnei() < 0))
		return;	/* bzzt */
	    drawei(w, h, d, wb, hb);
	    XSetForeground (dsp, e_gc, e_bg);
	    XPSPixmap (e_pm, w, h, xe_cm, e_gc, 0);
	    break;

	case SPHG:
	    XSetForeground (dsp, e_gc, e_bg);
	    XFillRectangle (dsp, e_pm, e_gc, 0, 0, w, h);
	    XFillArc (dsp, e_pm, e_gc, wb, hb, d, d, 0, 360*64);
	    break;

	case WXMAP:
	    if (e_setupwxpm(0, 0) < 0)
		return;
	    XCopyArea (dsp, e_wxpm, e_pm, e_gc, 0, 0, w, h, 0, 0);
	    XPSPixmap (e_pm, w, h, xe_cm, 0, 0);
	    break;
	}

	/* draw sunlit portion of earth */
	if (wants[SUNLIGHT])
	    e_sunlit (np, d, wb, hb);

	/* draw coord grid */
	if (wants[GRID])
	    e_drawgrid (d, wb, hb);

	/* draw each continent border -- just on night side sometimes */
	if (projection == CYLG || projection == SPHG || wants[SUNLIGHT])
	    e_drawcontinents (d, wb, hb);

	/* draw each site */
	if (wants[SITES])
	    e_drawsites (d, wb, hb);

	/* draw each shown object subject to date range and its trails if any */
	for (eop = eobjs; eop < &eobjs[nfavs]; eop++) {
	    if (dateOK(np, eop->op) == 0 && TBISON(eop->show_w)) {
		int dreworb = 0;
		if (is_type(eop->op, EARTHSATM) && TBISON(eop->wantorb_w)) {
		    e_draworbit (eop, d, wb, hb);  /*always draws name if want*/
		    dreworb = 1;
		} else
		    eop->ovis = 0;
		if (e_drawobject (np, eop, d, wb, hb) && !dreworb &&
							TBISON(eop->wantlbl_w))
		    e_drawname (eop, d, wb, hb);
		if (TBISON(eop->showtr_w))
		    e_drawtrail (eop, d, wb, hb);
	    }
	}

	/* mark the mainmenu location */
	if (wants[MAINMARKER])
	    e_mainmenuloc (np, d, wb, hb);

	/* mark any solar eclipse location */
	e_soleclipse (np, d, wb, hb);

	/* draw any annotation */
	ano_draw (e_da_w, e_pm, e_ano, 0);
}

/* convert world to/from X coords depending on w2x.
 * return whether visible.
 */
static int
e_ano (double *latp, double *longp, int *xp, int *yp, int w2x, int arg)
{
	int w, h, d, wb, hb;
	int v;

	/* get size of the rendering area */
	e_getcircle (&w, &h, &d, &wb, &hb);

	if (w2x) {
	    short sx, sy;
	    v = e_coord (d, wb, hb, *latp, *longp, &sx, &sy);
	    *xp = sx;
	    *yp = sy;
	} else {
	    v = e_uncoord (projection, d, wb, hb, *xp, *yp, latp, longp);
	}

	return (v);
}

/* draw the portion of the earth lit by the sun */
static void
e_sunlit (np, d, wb, hb)
Now *np;		/* circumstances */
unsigned int d, wb, hb;	/* circle dia, width and height borders in pixmap */
{
	switch (projection) {
	case CYLI:
	    /* already done in background */
	    break;
	case CYLG:
	    e_msunlit (np, d, wb, hb);
	    break;
	case SPHI:
	    /* already done in background */
	    break;
	case SPHG:
	    e_ssunlit (np, d, wb, hb);
	    break;
	case WXMAP:
	    /* already part of e_wxpm */
	    break;
	}
}

/* draw the sunlit portion of the Earth in cylindrical projection.
 */
static void
e_msunlit (np, d, wb, hb)
Now *np;		/* circumstances */
unsigned int d, wb, hb;	/* circle dia, width and height borders in pixmap */
{
#define MAXNMPTS	256	/* n segments -- larger is finer but slower */
	Display *dsp = XtDisplay(e_da_w);
	XPoint xp[MAXNMPTS+4];	/* extra room for top or bottom box */
	double sslat, sslong;	/* subsolar lat/long, rads, +N +E */
	double az0, daz;	/* initial and delta az so we move +x */
	int nmpts = MAXNMPTS;
	double ssl, csl;
	int w = d + 2*wb;
	int h = d + 2*hb;
	short x, y;
	int yi;
	int wrapn, i;

	e_subobject (np, db_basic(SUN), &sslat, &sslong);
	ssl = sin(sslat);
	csl = cos(sslat);

	if (sslat < 0) {
	    az0 = 0;
	    daz = 2*PI/nmpts;
	} else {
	    az0 = PI;
	    daz = -2*PI/nmpts;
	}

	/* fill in the circle of nmpts points that lie PI/2 from the
	 *   subsolar point and projecting from the viewpoint.
	 * when wrap, shuffle points up so final polygon doesn't wrap.
	 */
	XSetForeground (dsp, e_gc, ecolors[SUNC].p);
	wrapn = 0;
	for (i = 0; i < nmpts; i++) {
	    double A = az0 + i*daz;		/* azimuth of p from subsol */
	    double sA = sin(A), cA = cos(A);
	    double plat = asin (csl*cA);	/* latitude of p */
	    double plong = atan2(sA, -ssl*cA) + sslong;	/* long of p */
	    XPoint *xpp = &xp[i-wrapn];

	    (void) e_coord (d, wb, hb, plat, plong, &x, &y);

	    xpp->x = x;
	    xpp->y = y;

	    if (wrapn == 0 && i > 0 && x < xpp[-1].x) {
		/* when wrap copy points so far to end and this one is first.
		 * N.B. go backwards in case we are over half way.
		 */
		for ( ; wrapn < i; wrapn++)
		    xp[nmpts-1-wrapn] = xp[i-1-wrapn];
		xp[0].x = x;
		xp[0].y = y;
	    }
	}

	/* y-intercept at the edges is the average of the y's at each end */
	yi = (xp[0].y + xp[nmpts-1].y)/2;

	/* complete polygon across bottom if sun is below equator, or vv */
	if (sslat < 0) {
	    xp[i].x = w;    xp[i].y = yi;    i++;    /* right */
	    xp[i].x = w;    xp[i].y = h;     i++;    /* down */
	    xp[i].x = 0;    xp[i].y = h;     i++;    /* left */
	    xp[i].x = 0;    xp[i].y = yi;    i++;    /* up */
	} else {
	    xp[i].x = w;    xp[i].y = yi;    i++;    /* right */
	    xp[i].x = w;    xp[i].y = 0;     i++;    /* up */
	    xp[i].x = 0;    xp[i].y = 0;     i++;    /* left */
	    xp[i].x = 0;    xp[i].y = yi;    i++;    /* down */
	}

	XPSFillPolygon (dsp, e_pm, e_gc, xp, i, Complex, CoordModeOrigin);
}

/* draw the solid gibbous or crescent sun-lit portion of the Earth in spherical
 * projection.
 */
static void
e_ssunlit (np, d, wb, hb)
Now *np;		/* circumstances */
unsigned int d, wb, hb;	/* circle dia, width and height borders in pixmap */
{
#define MAXNSPTS	52	/* max number of polyline points */
#define	FULLANGLE	degrad(5)/* consider full if within this angle */
	Display *dsp = XtDisplay(e_da_w);
	double sslat, sslong;	/* subsolar lat/long, rads, +N +E */
	double ssl, csl;	/* sin/cos of sslat */
	int dx, dy;		/* slope from center to subsolar point */
	double ca;		/* angular sep from viewpoint to subsolar */
	double B;		/* cw angle up from center to subsolar point */
	int maxpts = MAXNSPTS;
	XPoint xp[MAXNSPTS];
	int npts;
	int i;

	/* find where the sun is */
	e_subobject (np, db_basic(SUN), &sslat, &sslong);

	/* set up color */
	XSetForeground (dsp, e_gc, ecolors[SUNC].p);

	/* find direction and length of line from center to subsolar point */
	solve_sphere (sslong - elng, PI/2-sslat, selat, celat, &ca, &B);

	/* check for special cases of eclipsed and directly over subsolar pt */
	if (acos(ca) < FULLANGLE) {
	    short x, y;
	    int vis = e_coord (d, wb, hb, sslat, sslong, &x, &y);
	    if (vis)
		XPSFillArc (dsp, e_pm, e_gc, wb, hb, d, d, 0, 360*64);
	    return;
	}

	/* find direction of line parallel to line from center to subsolar
	 * location and long enough to be well outside perimeter.
	 */
	dx = (int)floor(1.5*d*sin(B) + 0.5);	/* +x to right */
	dy = (int)floor(1.5*d*cos(B) + 0.5);	/* +y up */

	/* follow the circle of points which lie PI/2 from the subsolar point.
	 * those which are visible are used directly; those which are not are
	 * projected to limb on a line parallel to the center/subsolar point.
	 */
	ssl = sin(sslat);
	csl = cos(sslat);
	for (npts = i = 0; i < maxpts; i++) {
	    double T = degrad(i*360.0/maxpts);	/* pole to p as seen frm subso*/
	    double plat, plong;	/* lat/long of point on said circle */
	    short px, py;

	    solve_sphere (T, PI/2, ssl, csl, &ca, &B);
	    plat = PI/2 - acos(ca);
	    plong = sslong - B;
	    if (!e_coord (d, wb, hb, plat, plong, &px, &py)) {
		int tx, ty, lx, ly;	/* tmp and limb point */

		if (lc (wb, hb, d, (int)px, (int)py, px+dx, py-dy,
						    &tx, &ty, &lx, &ly) == 0) {
		    px = lx;
		    py = ly;
		}
	    } 
	    xp[npts].x = px;
	    xp[npts].y = py;
	    npts++;
	}

	XPSFillPolygon (dsp, e_pm, e_gc, xp, npts, Complex, CoordModeOrigin);
}

/* draw each continent border */
static void
e_drawcontinents (d, wb, hb)
unsigned d, wb, hb;
{
#define	MINSEP		1000	/* min sep to draw in low prec, 100ths deg */
#define	PTCACHE		64	/* number of XPoints to cache */
	Display *dsp = XtDisplay(e_da_w);
	Now *np = mm_get_now();
	XPoint xp[PTCACHE];
	MRegion *rp;
	int w = d + 2*wb;

	/* use the overlay GC, wide lines and the BORDERC color */
	XSetLineAttributes (dsp, e_olgc, WIDE_LW, LINE_ST, CAP_ST, JOIN_ST);
	XSetForeground (dsp, e_olgc, ecolors[BORDERC].p);

	for (rp = ereg; rp < ereg + nereg; rp++) {
	    short lastlt = rp->mcp[0].lt, lastlg = rp->mcp[0].lg;
	    int n = rp->nmcp;
	    int nxp = 0;
	    int i;

	    /* draw the region -- including closure at the end */
	    for (i = 0; i <= n; i++) {
		MCoord *cp = rp->mcp + (i%n);
		double l, L;
		short x, y;
		int vis;

		/* don't draw things that are real close if in low prec mode */
		lastlt = cp->lt;
		lastlg = cp->lg;
		l = degrad(cp->lt/100.0);
		L = degrad(cp->lg)/100.0;

		vis = e_coord (d, wb, hb, l, L, &x, &y) &&
				    (projection == CYLG || projection == SPHG
						    || !e_issunlit (np, l, L));
		nxp = add_to_polyline (xp, XtNumber(xp), i, vis, nxp, n, w,x,y);
	    }
	}
}

/* draw each site */
static void
e_drawsites (d, wb, hb)
unsigned d, wb, hb;
{
#define	NPCACHE		64		/* number of XPoints to cache */
	Display *dsp = XtDisplay(e_da_w);
	XPoint xps[NPCACHE];
	Site *sipa;
	int nsa;
	int nxp;

	/* use the overlay GC and the SITEC color */
	XSetForeground (dsp, e_olgc, ecolors[SITEC].p);

	nxp = 0;
	for (nsa = sites_get_list(&sipa); --nsa >= 0; ) {
	    Site *sip = &sipa[nsa];
	    short x, y;
	    int vis;

	    vis = e_coord (d, wb, hb, sip->si_lat, sip->si_lng, &x, &y);
	    if (vis) {
		/* show each site as a little square */
		xps[nxp].x = x;		xps[nxp].y = y;		nxp++;
		xps[nxp].x = x+1;	xps[nxp].y = y;		nxp++;
		xps[nxp].x = x;		xps[nxp].y = y+1;	nxp++;
		xps[nxp].x = x+1;	xps[nxp].y = y+1;	nxp++;
	    }
	    if (nxp > XtNumber(xps)-4 || nsa == 0) {
		if (nxp > 0)
		    XPSDrawPoints (dsp, e_pm, e_olgc, xps, nxp,CoordModeOrigin);
		nxp = 0;
	    }
	}
}

/* draw all but the first object in the trails list onto e_pm relative to
 * elat/elng connected lines, with tickmarks and possible time stamps.
 */
static void
e_drawtrail (eop, d, wb, hb)
EObj *eop;		/* info about object */
int d;			/* Earth circle diameter, pixels */
int wb, hb;		/* width and height borders, pixels */
{
	int w = d + 2*wb;
	short lx = 0, ly = 0, lv;
	int i;

	XSetLineAttributes (XtD, e_olgc, NARROW_LW, LINE_ST, CAP_ST, JOIN_ST);
	XSetForeground (XtD, e_olgc, eop->pix);
	XSetForeground (XtD, e_gc, eop->pix);

	lv = 0;
	for (i = 1; i < eop->ntrail; i++) {
	    Trail *tp = &eop->trail[i];
	    Obj *top = &tp->t_obj;
	    short x, y;
	    int v;

	    v = e_coord (d, wb, hb, tp->t_sublat, tp->t_sublng, &x, &y);
	    tp->lvis= v;
	    tp->lx = x;
	    tp->ly = y;

	    if (v && lv) {
		TrTS ts, lts, *ltsp;
		int e = (is_type(top,EARTHSATM) && top->s_eclipsed);

		/* establish a TrTS for tr_draw() */
		ts.t = tp->t_now.n_mjd;
		ts.lbl = tp->t_lbl;

		/* and go back to pick up the first if this is the second tick*/
		if (i == 2) {
		    lts.t = eop->trail[1].t_now.n_mjd;
		    lts.lbl = eop->trail[1].t_lbl;
		    ltsp = &lts;
		} else
		    ltsp = NULL;

		switch (projection) {
		case CYLI:	/* FALLTHRU */
		case CYLG:
		    if (abs(x-lx) > w/2) {
			/* break in two when wraps */
			if (x > lx) {
			    /* wrapped over the left edge */
			    tr_draw (XtD, e_pm, e_olgc, e, TICKLN, &ts, ltsp,
						&eop->trstate, lx, ly, x-w, y);
			    tr_draw (XtD, e_pm, e_olgc, e, TICKLN, &ts, ltsp,
						&eop->trstate, lx+w, ly, x, y);
			} else {
			    /* wrapped over the right edge */
			    tr_draw (XtD, e_pm, e_olgc, e, TICKLN, &ts, ltsp,
						&eop->trstate, lx, ly, x+w, y);
			    tr_draw (XtD, e_pm, e_olgc, e, TICKLN, &ts, ltsp,
						&eop->trstate, lx-w, ly, x, y);
			}
		    } else
			tr_draw (XtD, e_pm, e_olgc, e, TICKLN, &ts, ltsp,
						&eop->trstate, lx, ly, x, y);
		    break;

		case SPHI:	/* FALLTHRU */
		case SPHG:
		    tr_draw (XtD, e_pm, e_olgc, e, TICKLN, &ts, ltsp,
						&eop->trstate, lx, ly, x, y);
		    break;

		case WXMAP:
		    if (abs(x-lx) < w/10) 
			tr_draw (XtD, e_pm, e_olgc, e, TICKLN, &ts, ltsp,
						&eop->trstate, lx, ly, x, y);
		    break;
		}
	    }

	    lv = v;
	    lx = x;
	    ly = y;
	}
}

/* draw the first object in the trails list onto e_pm relative to elat/elng
 * with a full bullseye.
 * return 1 if any part was visible, else 0.
 */
static int
e_drawobject (np, eop, d, wb, hb)
Now *np;		/* circumstances */
EObj *eop;		/* which object */
int d;			/* Earth circle diameter, pixels */
int wb, hb;		/* width and height borders, pixels */
{
	Trail *tp = eop->trail;
	int isvis;

	XSetForeground (XtD, e_olgc, eop->pix);
	XSetForeground (XtD, e_gc, eop->pix);

	/* draw the current location (first object on trails) */
	if (TBISON(eop->wantfoot_w))
	    isvis = e_drawfootprint (np, eop, d, wb, hb, tp->t_sublat, tp->t_sublng,
			tp->t_obj.o_type == EARTHSAT ? tp->t_obj.s_elev : 1e9);
	else
	    isvis = e_drawcross (d, wb, hb, tp->t_sublat, tp->t_sublng, CROSSH);

	/* store location -- TODO: already computed this by now */
	tp->lvis = e_coord (d, wb, hb, tp->t_sublat, tp->t_sublng, &tp->lx,
								    &tp->ly);

	return (isvis);
}

/* draw one orbit of the given Earth satellite object.
 * orbit begins over current ground location then proceeds in a sidereal
 * reference frame, ie, ignores rotation of the earth.
 * draw name if want.
 */
static void
e_draworbit (eop, d, wb, hb)
EObj *eop;		/* which object */
int d;			/* Earth circle diameter, pixels */
int wb, hb;		/* width and height borders, pixels */
{
	Now now, *np;		/* advance time through one rev */
	Obj obj, *op;		/* advance object through one rev */
	double t0;		/* start time */
	XPoint xpts[NORBSTEPS+1];/* orbit steps */
	XPoint xpt0;		/* first point, reuse for closure */
	int wasvis;		/* whether visible last step */
	int w = d + 2*wb;	/* screen width */
	int re = d/2;		/* earth radiu, pixels */
	int drewname = 0;	/* whether name has been drawn */
	int sproj = (projection == SPHI || projection == SPHG);
	int i;
	int nxpts;

	/* make local copies of time and object */
	np = mm_get_now();
	now = *np;
	np = &now;
	obj = *eop->op;
	op = &obj;
	t0 = mjd;

	/* set color */
	XSetForeground (XtD, e_gc, eop->pix);

	/* step along for one rev */
	nxpts = 0;
	wasvis = 0;
	xpt0.x = xpt0.y = 0;		/* lint */
	for (i = 0; i < NORBSTEPS; i++) {
	    double dt, lg;		/* time step and sidereal longitude */
	    int xs, ys;			/* coords of orbit from upper left */
	    int isvis;

	    /* compute circumstances at next time step */
	    dt = i/op->es_n/NORBSTEPS;
	    mjd = t0 + dt;
	    obj_cir (np, op);
	    lg = op->s_sublng;

	    /* project onto screen, note visibility */
	    if (sproj) {
		double sR, cR;		/* R= radius from viewpoint to p */
		double A, sA,cA;	/* A= az of p as seen from viewpoint */
		int ro;			/* orbit radius, pixels */
		int xo, yo;		/* coords of orbit from center */

		/* remove rotation effect */
		lg += hrrad(24/SIDRATE*dt);

		solve_sphere (lg - elng,PI/2 - op->s_sublat,selat,celat,&cR,&A);
		sR = sqrt(1.0 - cR*cR);
		sA = sin(A);
		cA = cos(A);
		ro = re + re*op->s_elev/ERAD;
		xs = wb + re + (xo = (int)floor(ro*sR*sA + 0.5));
		ys = hb + re - (yo = (int)floor(ro*sR*cA + 0.5));
		isvis = xo*xo + yo*yo > re*re || cR > 0;
	    } else {
		short sx, sy;
		isvis = e_coord (d, wb, hb, op->s_sublat, lg, &sx, &sy);
		xs = sx;
		ys = sy;
	    }

	    if (i == 0) {
		/* first time = position now */
		wasvis = isvis;
		xpt0.x = xs;
		xpt0.y = ys;

		/* mark if vis */
		eop->ovis = isvis;
		if (isvis) {
		    XPSFillArc (XtD, e_pm, e_gc, xs-SATSZ/2, ys-SATSZ/2,
						SATSZ+1, SATSZ+1, 0, 360*64);
		    eop->ox = xs;
		    eop->oy = ys;
		}
	    }

	    /* draw name first time orbit is visible */
	    if (isvis && !drewname && TBISON(eop->wantlbl_w)) {
		e_draworbname (eop, xs, ys);
		drewname = 1;
	    }

	    /* process depending on what happens at limb/edge */
	    if (isvis && wasvis) {
		/* still visible - add to list -- beware of wrap in cyl mode */
		if (!sproj && nxpts > 0 && abs(xpts[nxpts-1].x-xs) > w/2) {
		    /* cyl wrap */
		    if (xs > w/2) {
			xpts[nxpts].x = 0;
			xpts[nxpts].y = xpts[nxpts-1].y;
			nxpts++;
			XPSDrawLines (XtD,e_pm,e_gc,xpts,nxpts,CoordModeOrigin);
			xpts[0].x = w;
			xpts[0].y = ys;
			xpts[1].x = xs;
			xpts[1].y = ys;
			nxpts = 2;
		    } else {
			xpts[nxpts].x = w;
			xpts[nxpts].y = xpts[nxpts-1].y;
			nxpts++;
			XPSDrawLines (XtD,e_pm,e_gc,xpts,nxpts,CoordModeOrigin);
			xpts[0].x = 0;
			xpts[0].y = ys;
			xpts[1].x = xs;
			xpts[1].y = ys;
			nxpts = 2;
		    }
		} else {
		    xpts[nxpts].x = xs;
		    xpts[nxpts].y = ys;
		    nxpts++;
		}
	    } else if (!isvis && !wasvis) {
		/* still hidden - just need one history */
		xpts[0].x = xs;
		xpts[0].y = ys;
		nxpts = 1;
	    } else if (isvis && !wasvis) {
		/* just became visible around limb - use one history */
		int sx1, sy1, sx2, sy2;
		/* expand circle slightly to avoid 1-pixel grazings */
		lc (wb-1, hb-1, d+2, xs, ys, xpts[0].x, xpts[0].y,
							&sx1, &sy1, &sx2, &sy2);
		xpts[0].x = sx1;
		xpts[0].y = sy1;
		xpts[1].x = xs;
		xpts[1].y = ys;
		nxpts = 2;
	    } else if (!isvis && wasvis) {
		/* just went behind limb - draw and start one history */
		int sx1, sy1, sx2, sy2;
		/* expand circle slightly to avoid 1-pixel grazings */
		lc (wb-1, hb-1, d+2, xpts[nxpts-1].x, xpts[nxpts-1].y, xs, ys,
							&sx1, &sy1, &sx2, &sy2);
		xpts[nxpts].x = sx1;
		xpts[nxpts].y = sy1;
		nxpts++;
		XPSDrawLines (XtD, e_pm, e_gc, xpts, nxpts, CoordModeOrigin); 
		xpts[0].x = xs;
		xpts[0].y = ys;
		nxpts = 1;
	    }

	    wasvis = isvis;
	}

	/* finish remaining segment */
	if (wasvis && nxpts > 0) {
	    if (sproj)
		xpts[nxpts++] = xpt0;	/* close */
	    XPSDrawLines (XtD, e_pm, e_gc, xpts, nxpts, CoordModeOrigin);
	}
}

/* draw obj's name */
static void
e_drawname (eop, d, wb, hb)
EObj *eop;		/* which object */
int d;			/* Earth circle diameter, pixels */
int wb, hb;		/* width and height borders, pixels */
{
	Trail *tp = &eop->trail[0];
	char *name = tp->t_obj.o_name;
	int nl = strlen (name);
	XCharStruct all;
	int dir, asc, des;
	short x, y;

	XTextExtents (e_f, name, nl, &dir, &asc, &des, &all);
	XSetForeground (XtD, e_strgc, eop->pix);
	e_coord (d, wb, hb, tp->t_sublat, tp->t_sublng, &x, &y);
	XPSDrawString (XtD, e_pm, e_strgc, x-all.width/2, y-10-des, name, nl);
}

/* draw orbit's name */
static void
e_draworbname (EObj *eop, int x, int y)
{
	Trail *tp = &eop->trail[0];
	char *name = tp->t_obj.o_name;
	int nl = strlen (name);
	XCharStruct all;
	int dir, asc, des;

	XTextExtents (e_f, name, nl, &dir, &asc, &des, &all);
	XSetForeground (XtD, e_strgc, eop->pix);
	XPSDrawString (XtD, e_pm, e_strgc, x-all.width/2, y-10-des, name, nl);
}

/* this function just captures the code we were inventing over and over to draw
 * polylines.
 */
static int
add_to_polyline (xp, xpsize, i, vis, nxp, max, w, x, y)
XPoint xp[];	/* working array */
int xpsize;	/* entries in xp[] */
int i;		/* item we are on: 0..max */
int vis;	/* is this point visible */
int nxp;	/* number of items in xp[] in use -- next goes in xp[npx] */
int max;	/* largest item number we will draw */
int w;		/* window width -- used to check top wraps */
int x, y;	/* the point to add to polyline */
{
	int lx = 0, ly = 0;
	int xwrap = 0;

	if (vis) {
	    xp[nxp].x = x;
	    xp[nxp].y = y;
	    nxp++;
	}

	if (nxp > 1) {
	    int dx;

	    lx = xp[nxp-2].x;
	    dx = lx - xp[nxp-1].x;

	    switch (projection) {
	    case CYLI:	/* FALLTHRU */
	    case CYLG:
		if (dx >= w/2) {
		    /* wrapped around right edge */
		    xp[nxp-1].x += w;
		    ly = xp[nxp-2].y;
		    xwrap = 1;
		} else if (-dx >= w/2) {
		    /* wrapped around left edge */
		    xp[nxp-1].x -= w;
		    ly = xp[nxp-2].y;
		    xwrap = -1;
		} else
		    xwrap = 0;
		break;
	    
	    case SPHI:	/* FALLTHRU */
	    case SPHG:
		/* never wraps due to not being visible over edge */
		xwrap = 0;
		break;

	    case WXMAP:
		xwrap = abs(dx) > w/10;
		if (xwrap)
		    nxp--;
		break;
	    }

	} else
	    xwrap = 0;

	/* draw the line if it just turned invisible, done or wrapped */
	if (!vis || i == max || nxp == xpsize || xwrap != 0) {
	    if (nxp > 1)
		XPSDrawLines (XtD, e_pm, e_olgc, xp, nxp, CoordModeOrigin);
	    if (nxp == xpsize) {
		xp[0] = xp[nxp-1];
		nxp = 1;
	    } else
		nxp = 0;

	    switch (projection) {
	    case CYLI:	/* FALLTHRU */
	    case CYLG:
		/* if we wrapped then pick up fragment on opposite end */
		if (xwrap > 0) {
		    /* pick up fragment on left end */
		    xp[nxp].x = lx-w; xp[nxp].y = ly; nxp++;
		    xp[nxp].x = x;    xp[nxp].y = y;  nxp++;
		} else if (xwrap < 0) {
		    /* pick up fragment on right end */
		    xp[nxp].x = lx+w; xp[nxp].y = ly; nxp++;
		    xp[nxp].x = x;    xp[nxp].y = y;  nxp++;
		}
		break;

	    case SPHI:	/* FALLTHRU */
	    case SPHG:
		break;

	    case WXMAP:
		/* put new at front of line */
		xp[nxp].x = x;    xp[nxp].y = y;  nxp++;
		break;
	    }

	    /* leave at front of line but draw if this is all */
	    if (i == max && nxp > 0)
		XPSDrawLines (XtD, e_pm, e_olgc, xp, nxp, CoordModeOrigin);
	}

	return (nxp);
}

/* mark the mainmenu location */
static void
e_mainmenuloc (np, d, wb, hb)
Now *np;
unsigned d, wb, hb;
{
	XSetForeground (XtD, e_olgc, ecolors[HEREC].p);
	e_drawcross (d, wb, hb, lat, lng, PLUSS);
}

/* mark the location of a solar ecipse, if any.
 *
 * N.B. this code is geometrically correct for all objects, but in practice
 * only the sun and moon are computed accurately enough by xephem to make this
 * worth while for now. in particular, I have tried tests against several S&T
 * tables of minor planet occultations and while the asteroids are computed
 * well enough for visual identification remember that at even 1 AU the earth
 * only subtends 18 arc seconds and the asteroids are not computed *that*
 * accurately (especially since we do not yet include perturbations).
 *
 * I will try to describe the algorithm: the exective summary is that we
 * are striving for the spherical arc subtended by the intersection of a line
 * from one object and the earth's center and the line from the other object
 * to the earth's center.
 *
 * N.B. I tried just computing the intersection of a line connecting the two
 * objects and a unit sphere but it suffered from terrible numerical instabilty.
 *
 * start in a plane defined by the center of the earth, the north pole and
 * object obj0. label the center of the earth as O, the location of object 0
 * as P0, and place object 1 someplace off the line O-P0 and label it P1.
 * what you have actually placed is the location of P1 as it projected onto
 * this place; ie, we are only working with dec here.  define decA as the
 * angle P1-O-P0; it is so named because it is the amount of declication
 * subtended from P0 to P1. Project the line P0-P1 back to a line
 * perpendicular to the line O-P0 at O. decD is the distance from O to the
 * point where P0-P1 intersects the line. if it is less than the earth radius
 * we have an occultation! now do all this again only this time place
 * yourself in a plane defined by the real locations of O, P0 and P1. and
 * repeat everything except this time use the real angled subtended in the
 * sky between P0 and P1 (not just the dec difference). this angle we define
 * as skyA (and its projection back onto a plane perpendicular to P0-P1
 * through O we call skyD). what we want next is the spherical angle
 * subtended between the point at which O-P0 intersects the earth's surface
 * (which is just the geocentric coords of P0) and the point where a line
 * from the tip of skyD to P1 intersects the earth's surface. we call this
 * skyT (I used tau in my original sketch) and I will let you work out the
 * trig (it's just planar trig since you are working in the O-P0-P1 plane).
 * this gives us the spherical angle between the two lines and the earth
 * surface; now all we need is the angle.image yourself at P0 now looking
 * right at O. we see decD as a vertical line and SkyD as a line going off
 * from O at an angle somewhere. the angle between these lines we define as
 * theta. knowing decD and skyD and knowing that there is a right angle at
 * the tip of decD between O and the tip of skyD we can compute the angle
 * between them. theta.  now just use a little spherical trig to find where
 * our arc ends up, compute the new RA, compute longitude by subtracting
 * gst, set latitude to dec, project and draw!
 */
static void
e_soleclipse (np, d, wb, hb)
Now *np;
unsigned d, wb, hb;
{
	Obj *op0 = db_basic (SUN);	/* op0 must be the further one */
	Obj *op1 = db_basic (MOON);
	Obj obj0, obj1;			/* use copies */
	double r0, r1;			/* dist to objects, in earth radii */
	double theta;			/* angle between projections */
	double decD, decA;		/* dec-only proj dist and angle */
	double skyD, skyA, skyP, skyT;	/* full sky projection */
	Now now;			/* local copy to compute EOD info */
	double lst, gst;		/* local and UTC time */
	double lt, lg;			/* lat/long */
	double sD, dRA;

	now = *np;
	obj0 = *op0;
	obj1 = *op1;

	now.n_epoch = EOD;
	(void) obj_cir (&now, &obj0);
	if (is_ssobj(&obj0))
	    r0 = obj0.s_edist*(MAU/ERAD);	/* au to earth radii */
	else
	    r0 = 1e7;				/* way past pluto */

	(void) obj_cir (&now, &obj1);
	if (is_ssobj(&obj1))
	    r1 = obj1.s_edist*(MAU/ERAD);	/* au to earth radii */
	else
	    r1 = 1e7;				/* way past pluto */

	decA = obj1.s_gaedec - obj0.s_gaedec;
	decD = r0*r1*sin(decA)/(r0 - r1);	/* similar triangles */
	if (fabs(decD) >= 1.0)
	    return;

	skyA = acos (sin(obj0.s_gaedec)*sin(obj1.s_gaedec) +
				    cos(obj0.s_gaedec)*cos(obj1.s_gaedec) *
					cos(obj0.s_gaera-obj1.s_gaera));
	skyD = r0*r1*sin(skyA)/(r0 - r1);	/* similar triangles */
	if (fabs(skyD) >= 1.0)
	    return;

	/* skyP is angle subtended by skyD as seen from obj0 (I called it psi).
	 * skyT is angle subtended by line from earth center to obj0 to a
	 *   point where the line from obj0 to the tip of skyD intersects the
	 *   earth surface (I called it tau).
	 */
	skyP = atan(skyD/r0);
	skyT = asin(skyD*r0/sqrt(r0*r0+skyD*skyD)) - skyP;

	theta = acos(decD/skyD);
	solve_sphere (theta, skyT, sin(obj0.s_gaedec), cos(obj0.s_gaedec),
								    &sD, &dRA);

	lt = asin(sD);

	if (obj1.s_gaera > obj0.s_gaera)
	    dRA = -dRA;	/* eastward */

	lst = obj0.s_gaera - dRA;
	utc_gst (mjd_day(mjd), mjd_hr(mjd), &gst);
	lg = lst - hrrad(gst);
	while (lg < -PI) lg += 2*PI;
	while (lg >  PI) lg -= 2*PI;

	XSetForeground (XtD, e_olgc, ecolors[ECLIPSEC].p);
	e_drawcross (d, wb, hb, lt, lg, CROSSH);
}

/* given a height above the earth, in meters, and an altitude above the
 * horizon, in rads, return the great-circle angular distance from the subpoint
 * to the point at which the given height appears at the given altitude, in
 * rads.
 * N.B. beware of negative heights (deorbited ;-))
 */
static void
e_viewrad (np, height, alt, radp)
Now *np;	/* refraction circumstances */
double height;	/* satellite elevation, m above mean earth */
double alt;	/* viewing altitude, rads above horizon */
double *radp;	/* great-circle distance from subpoint to viewing circle, rads*/
{
	/* remove effect of refraction from geometry */
	unrefract (pressure, temp, alt, &alt);

	if (height > 0)
	    *radp = acos(ERAD/(ERAD+height)*cos(alt)) - alt;
	else
	    *radp = 0;
}

/* make the first entry in the trail list, ie the current position of eop->op,
 * correct as of *np. then, if desired, discard any existing trail history.
 * return 0 if ok else -1 if no memory.
 */
static void
e_resettrail(eop, np, discard)
EObj *eop;
Now *np;
int discard;
{
	Obj *op = eop->op;
	Trail *tp = eop->trail;

	db_update (op);
	if (discard) {
	    if (eop->trail) {
		free ((char *)eop->trail);
		eop->trail = NULL;
	    }
	    eop->ntrail = 0;
	    tp = e_growtrail(eop);
	}

	/* first entry is for the current time, regardless of whether there
	 * are additional entries.
	 */
	tp = &eop->trail[0];

	tp->t_lbl = 0;
	tp->t_now = *np;
	tp->t_obj = *op;
	e_subobject (&tp->t_now, &tp->t_obj, &tp->t_sublat, &tp->t_sublng);
}

/* called from the trails facilty to build up a trail.
 * do it and refresh the view.
 * client is index into eobjs[].
 * return 0 if ok, else -1.
 */
/* ARGSUSED */
static int
e_mktrail (ts, statep, client)
TrTS ts[];
TrState *statep;
XtPointer client;
{
	EObj *eop = &eobjs[(long int)client];
	Obj *op = eop->op;
	Now *np = mm_get_now();
	int wastrail = eop->ntrail > 1;
	int i;

	watch_cursor (1);

	/* erase all but the first entry (ie, leave current alone) */
	db_update (op);
	e_resettrail(eop, np, 1);

	for (i = 0; i < statep->nticks; i++, ts++) {
	    Trail *tp;
	    Now now;

	    /* check date range */
	    now = *np;
	    now.n_mjd = ts->t;
	    if (dateOK (&now, op) < 0)
		continue;

	    /* ok, add to trail */
	    tp = e_growtrail(eop);
	    if (!tp)
		return (-1);
	    tp->t_lbl = ts->lbl;
	    tp->t_now = now;
	    tp->t_obj = *op;
	    (void) obj_cir (&tp->t_now, &tp->t_obj);
	    e_subobject (&tp->t_now, &tp->t_obj, &tp->t_sublat, &tp->t_sublng);
	}

	/* save trail setup as next default */
	eop->trstate = *statep;

	/* update trail state resource */
	tr_setres (etrres, statep);

	/* draw the trail display if desired but beware getting called while
	 * not up.
	 */
	if (TBISON(eop->showtr_w) && TBISON(eop->show_w) && isUp(eshell_w)) {
	    if (wastrail)
		e_all(0);	/* erase old and draw new */
	    else if (TBISON(eop->show_w)) {
		/* just draw new */
		int w, h, d, wb, hb;

		e_getcircle (&w, &h, &d, &wb, &hb);
		e_drawtrail (eop, d, wb, hb);
		e_copy_pm();
	    }
	}

	watch_cursor (0);

	return (0);
}

/* grow the trails list by one and return the address of the new entry.
 */
static Trail *
e_growtrail (eop)
EObj *eop;
{
	eop->trail = (Trail *) XtRealloc ((char *)eop->trail,
					    (eop->ntrail+1)*sizeof(Trail));
	return (&eop->trail[eop->ntrail++]);
}
/* fetch the weather map, and fill wxgifpix and wxxcols[]
 * return 0 if ok, else -1
 */
static int
e_getwxgif()
{
	Display *dsp = XtDisplay(e_da_w);
	unsigned char rawgif[200000];
	int nrawgif;
	char buf[1024];
	int w, h;
	int fd;

	/* open test case, else real network */
	fd = openh ("/tmp/latest_cmoll.gif", O_RDONLY);
	if (fd >= 0) {
	    nrawgif = read (fd, rawgif, sizeof(rawgif));
	    close (fd);
	} else {
	    int isgif;
	    int length;
	    int nr;

	    /* announce net activity and give user a way to stop */
	    stopd_up();

	    /* make connection to server for the file */
	    xe_msg (0, "Getting\nhttp://%s%s", wxhost, wxfile);
	    (void) sprintf (buf, "GET http://%s%s HTTP/1.0\r\nUser-Agent: xephem/%s\r\n\r\n",
						wxhost, wxfile, PATCHLEVEL);
	    fd = httpGET (wxhost, buf, buf);
	    if (fd < 0) {
		xe_msg (1, "http get:\n%s", buf);
		stopd_down();
		return (-1);
	    }

	    /* read header, looking for some header info */
	    isgif = 0;
	    length = 0;
	    while (recvline (fd, buf, sizeof(buf)) > 1) {
		xe_msg (0, "Rcv: %s", buf);
		if (strstr (buf, "image/gif"))
		    isgif = 1;
		if (strstr (buf, "Content-Length"))
		    length = atoi (buf+15);
	    }
	    if (!isgif) {
		while (recvline (fd, buf, sizeof(buf)) > 1)
		    xe_msg (0, "Rcv: %s", buf);
		close (fd);
		stopd_down();
		return (-1);
	    }
	    if (length == 0)
		length = 100000;	/* ?? */

	    /* read gif into rawgif[] */
	    pm_up();
	    for (nrawgif = 0; nrawgif < sizeof(rawgif); nrawgif += nr) {
		pm_set (100*nrawgif/length);
		nr = readbytes (fd, rawgif+nrawgif, 4096);
		if (nr < 0) {
		    xe_msg (1, "%s:\n%s", wxhost, syserrstr());
		    stopd_down();
		    pm_down();
		    close (fd);
		    return (-1);
		}
		if (nr == 0)
		    break;
	    }
	    stopd_down();
	    pm_down();
	    close (fd);
	    if (nr > 0) {
		xe_msg (1, "File too large");
		return (-1);
	    }
	}

	/* uncompress the gif into X terms */
	if (wxgifpix) {
	    free (wxgifpix);
	    wxgifpix = NULL;
	    freeXColors (dsp, xe_cm, wxxcols, 256);
	}
	if (gif2X (dsp,xe_cm,rawgif,nrawgif,&w,&h,&wxgifpix,wxxcols,buf)<0) {
	    xe_msg (1, "Gif: %s", buf);
	    return (-1);
	}
	if (w != WXM_W || h != WXM_H) {
	    xe_msg (1, "Expecting %dx%d but found %dx%d", WXM_W, WXM_H, w,h);
	    free (wxgifpix);
	    wxgifpix = NULL;
	    freeXColors (dsp, xe_cm, wxxcols, 256);
	    return (-1);
	}

	/* ok */
	return (0);
}

/* create an XImage of size wXh.
 * return XImage * if ok else NULL and xe_msg().
 */
static XImage *
e_create_xim (w, h)
int w, h;
{
	Display *dsp = XtDisplay(e_da_w);
	XImage *xip;
	int mdepth;
	int mbpp;
	int nbytes;
	char *data;

	/* establish depth and bits per pixel */
	get_something (e_da_w, XmNdepth, (XtArgVal)&mdepth);
	if (mdepth < 8) {
	    xe_msg (1, "Require at least 8 bit pixel depth");
	    return (NULL);
	}
	mbpp = mdepth>=17 ? 32 : (mdepth >= 9 ? 16 : 8);
	nbytes = w*h*mbpp/8;

	/* get memory for image pixels.  */
	data = malloc (nbytes);
	if (!data) {
	    xe_msg (1,"Can not get %d bytes for image pixels", nbytes);
	    return (NULL);
	}

	/* create the XImage */
	xip = XCreateImage (dsp, DefaultVisual (dsp, DefaultScreen(dsp)),
	    /* depth */         mdepth,
	    /* format */        ZPixmap,
	    /* offset */        0,
	    /* data */          data,
	    /* width */         w,
	    /* height */        h,
	    /* pad */           mbpp,
	    /* bpl */           0);
	if (!xip) {
	    xe_msg (1, "Can not create shadow XImage");
	    free ((void *)data);
	    return (NULL);
	}

        xip->bitmap_bit_order = LSBFirst;
	xip->byte_order = LSBFirst;

	/* ok */
	return (xip);
}

/* given a time and location, return 1 if location is in sunlight, else 0 */
static int
e_issunlit (np, l, L)
Now *np;
double l, L;
{
	static double last_mjd;
	static double csslat, ssslat;
	static double sslong;
	double ca;

	if (mjd != last_mjd) {
	    double sslat;
	    e_subobject (np, db_basic(SUN), &sslat, &sslong);
	    csslat = cos (sslat);
	    ssslat = sin (sslat);
	    last_mjd = mjd;
	}
	solve_sphere (L - sslong, PI/2-l, ssslat, csslat, &ca, NULL);
	return (ca > 0.);
}

/* return 1 if the given wxgifpix is gray, else 0 */
static int
e_wxisgray (p)
int p;
{
	int r = (int)wxxcols[p].red >> 8;
	int g = (int)wxxcols[p].green >> 8;
	int b = (int)wxxcols[p].blue >> 8;
	int mean = (r+g+b)/3;
	int gray = abs(r-mean)<20 && abs(g-mean)<20 && abs(b-mean)<20;

	return (gray);
}

/* see that e_wxpm exists and is filled properly.
 * return 0 if ok, else -1
 */
static int
e_newwxpm()
{
	Display *dsp = XtDisplay(e_da_w);
	Window win = RootWindow(dsp, DefaultScreen(dsp));
	unsigned long black = BlackPixel (dsp, DefaultScreen(dsp));
	Now *np = mm_get_now();
	XImage *xip;
	int x, y;

	/* required */
	if (!wxgifpix) {
	    printf ("newwxpm but no wxgifpix!\n");
	    abort();
	}

	/* create XImage */
	xip = e_create_xim (WXM_W, WXM_H);
	if (!xip)
	    return (-1);

	/* fill XImage with weather map */
	for (y = 0; y < WXM_H; y++) {
	    int yrow = y*WXM_W;
	    for (x = 0; x < WXM_W; x++) {
		int gp = (int)wxgifpix[x + yrow];
		unsigned long p;
		double l, L;

		p = !e_uncoord (WXMAP,0,0,0,x,y,&l,&L) || !wants[SUNLIGHT] ||
					e_issunlit (np,l,L) || e_wxisgray (gp)
					? wxxcols[gp].pixel : black;
		XPutPixel (xip, x, y, p);
	    }
	}

	/* create pixmap if necessary and fill with weather map */
	if (!e_wxpm)
	    e_wxpm = XCreatePixmap (dsp, win, WXM_W, WXM_H, xip->depth);
	XPutImage (dsp, e_wxpm, e_gc, xip, 0, 0, 0, 0, WXM_W, WXM_H);

	/* ok */
	free ((void *)xip->data);
	xip->data = NULL;
	XDestroyImage (xip);
	return (0);
}

/* see that e_wxpm is ready.
 * return 0 if ok, else -1.
 */
static int
e_setupwxpm (reload, rebuild)
int reload;
int rebuild;
{
	static double last_mjd;
	Now *np = mm_get_now();
	int newpm;

	watch_cursor(1);

	/* insure we have the weather gif */
	newpm = reload || (e_wxpm == (Pixmap)0);
	if ((!wxgifpix || reload) && e_getwxgif() < 0) {
	    watch_cursor(0);
	    return (-1);
	}

	/* build the pixmap, as necessary */
	if (rebuild || newpm || (wants[SUNLIGHT] && mjd != last_mjd)) {
	    if (e_newwxpm() < 0) {
		watch_cursor(0);
		return (-1);
	    }
	    last_mjd = mjd;
	}

	/* e_wxpm is ready */
	watch_cursor(0);
	return (0);
}

/* convert lat/long to x/y on www.ssec.wisc.edu/data/comp/latest_cmoll.gif */
static void
mollweide_llxy (l, L, xp, yp)
double l, L;		/* lat, long, rads, +N, +E */
short *xp, *yp;		/* gif location */
{
	double tmp;

	tmp = 1.116*sin(.7071*l);
	*yp = (short)((WXM_TY+WXM_BY)/2 - tmp*(WXM_BY-WXM_TY)/2);
	range (&L, 2*PI);
	if (L > PI)
	    L -= 2*PI;
	tmp = L/PI*sqrt(1. - tmp*tmp);
	*xp = (short)((WXM_LX+WXM_RX)/2 + tmp*(WXM_RX-WXM_LX)/2);
}

/* convert x/y to lat/long on www.ssec.wisc.edu/data/comp/latest_cmoll.gif.
 * return 1 if actually on the earth map, else 0.
 */
static int
mollweide_xyll (x, y, lp, Lp)
int x, y;		/* gif location */
double *lp, *Lp;	/* lat, long, rads, +N, +E */
{
	double fx, fy;

	fy = 2.*((WXM_BY + WXM_TY)/2 - y)/(WXM_BY - WXM_TY);
	if (fabs(fy) > 1.)
	    return (0);
	*lp = 1.414*asin(fy/1.116);
	fx = 2.*(x - (WXM_RX + WXM_LX)/2)/(WXM_RX - WXM_LX);
	*Lp = PI*fx/sqrt(1. - fy*fy);
	return (fabs(*Lp) <= PI);
}

/* load the daytime earth image file into deipix and colors in deixcols.
 * return 0 if ok, else write xe_msg and return -1.
 */
static int
loaddei()
{
	struct stat s;
	char *rawgif;
	int nrawgif;
	char buf[1024];
	char fn[256];
	int n, fd;

	/* open map file */
	(void) sprintf (fn, "%s/auxil/dearth.gif",  getShareDir());
	fd = openh (fn, 0);
	if (fd < 0) {
	    xe_msg (1, "%s: %s", fn, syserrstr());
	    return (-1);
	}

	/* read in */
	fstat (fd, &s);
	n = (int)s.st_size;
	rawgif = malloc (n);
	if (!rawgif) {
	    xe_msg (1, "%s: can not get %d to read", fn, n);
	    close (fd);
	    return (-1);
	}
	nrawgif = read (fd, rawgif, n);
	if (nrawgif < 0) {
	    xe_msg (1, "%s: %s", fn, syserrstr());
	    free (rawgif);
	    close (fd);
	    return (-1);
	}
	if (nrawgif == 0) {
	    xe_msg (1, "%s: contains 0 bytes", fn);
	    free (rawgif);
	    close (fd);
	    return (-1);
	}
	close (fd);

	/* crack open */
	if (gif2X (XtD, xe_cm, (unsigned char *)rawgif, nrawgif, &deiw, &deih,
						&deipix, deixcols, buf) < 0) {
	    xe_msg (1, "%s: %s", fn, buf);
	    free (rawgif);
	    return (-1);
	}

	/* ok */
	free (rawgif);
	return (0);
}

/* load the nighttime earth image file into neipix and colors in deixcols.
 * return 0 if ok, else write xe_msg and return -1.
 */
static int
loadnei()
{
	struct stat s;
	char *rawgif;
	int nrawgif;
	char buf[1024];
	char fn[256];
	int n, fd;

	/* open map file */
	(void) sprintf (fn, "%s/auxil/nearth.gif",  getShareDir());
	fd = openh (fn, 0);
	if (fd < 0) {
	    xe_msg (1, "%s: %s", fn, syserrstr());
	    return (-1);
	}

	/* read in */
	fstat (fd, &s);
	n = (int)s.st_size;
	rawgif = malloc (n);
	if (!rawgif) {
	    xe_msg (1, "%s: can not get %d to read", fn, n);
	    close (fd);
	    return (-1);
	}
	nrawgif = read (fd, rawgif, n);
	if (nrawgif < 0) {
	    xe_msg (1, "%s: %s", fn, syserrstr());
	    free (rawgif);
	    close (fd);
	    return (-1);
	}
	if (nrawgif == 0) {
	    xe_msg (1, "%s: contains 0 bytes", fn);
	    free (rawgif);
	    close (fd);
	    return (-1);
	}
	close (fd);

	/* crack open */
	if (gif2X (XtD, xe_cm, (unsigned char *)rawgif, nrawgif, &neiw, &neih,
						&neipix, neixcols, buf) < 0) {
	    xe_msg (1, "%s: %s", fn, buf);
	    free (rawgif);
	    return (-1);
	}

	/* ok */
	free (rawgif);
	return (0);
}

/* render deipix and neipix into e_pm */
static void
drawei (ww, wh, d, xb, yb)
unsigned int ww, wh, d, xb, yb;
{
	Now *np = mm_get_now();
	Display *dsp = XtD;
	unsigned long p;
	XImage *xip;
	double lt, lg;
	int wx, wy;

	/* required */
	if (!deipix) {
	    printf ("no deipix!\n");
	    abort();
	}
	if (!neipix) {
	    printf ("no neipix!\n");
	    abort();
	}

	/* create XImage */
	xip = e_create_xim (ww, wh);
	if (!xip)
	    return;

	/* fill XImage with earth image */
	for (wy = 0; wy < wh; wy++) {
	    for (wx = 0; wx < ww; wx++) {
		if (!e_uncoord (projection, d, xb, yb, wx, wy, &lt, &lg))
		    p = e_bg;
		else if (!wants[SUNLIGHT] || e_issunlit (np,lt,lg)) {
		    int eix = (int)(deiw*(lg+PI)/(2*PI));
		    int eiy = deiw*(int)(deih*(PI/2-lt)/PI);
		    int pi = (int)deipix[eix + eiy];
		    p = deixcols[pi].pixel;
		} else if (wants[NITELITES]) {
		    int eix = (int)(neiw*(lg+PI)/(2*PI));
		    int eiy = neiw*(int)(neih*(PI/2-lt)/PI);
		    int pi = (int)neipix[eix + eiy];
		    p = neixcols[pi].pixel;
		} else
		    p = e_bg;
		XPutPixel (xip, wx, wy, p);
	    }
	}

	/* fill e_pm with image */
	XPutImage (dsp, e_pm, e_gc, xip, 0, 0, 0, 0, ww, wh);

	/* ok */
	free ((void *)xip->data);
	xip->data = NULL;
	XDestroyImage (xip);
}

/* given lat long find Great cirle distance and bearing from Main site.
 * distance will be in m.
 */
static void
e_greatCircle (double lt, double lg, double *dp, double *bp)
{
	Now *np = mm_get_now();
	double ca;

	solve_sphere (lg-lng, PI/2-lt, sin(lat), cos(lat), &ca, bp);
	*dp = acos(ca)*ERAD;		/* arc to distance */

}

/* given window info and a lat/lng, draw the great circle through it
 * and the Main site.
 */
static void
e_plotGreatCircle (unsigned d, unsigned wb, unsigned hb, double lt, double lg)
{
	Now *np = mm_get_now();
	double dist, bear;
	double ca, a, B;

	/* draw fresh map */
	e_map (np);

	/* find bearing from Main to here */
	e_greatCircle (lt, lg, &dist, &bear);

	/* find point normal to plane define by center, here and Main loc */
	solve_sphere (bear+PI/2, PI/2, sin(lat), cos(lat), &ca, &B);
	a = acos(ca);

	/* draw circle with radius 90 degrees about said point */
	e_drawcircle (ecolors[GREATC].p, d, wb, hb, PI/2-a, lng+B, PI/2);

	e_copy_pm();
}


/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: earthmenu.c,v $ $Date: 2012/11/23 04:15:23 $ $Revision: 1.116 $ $Name:  $"};

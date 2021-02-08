/* code code to manage the stuff on the sky view display.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>

#include "xephem.h"

/* we keep a linked-list of TrailObj's we want trails for. these in turn
 *   contain an array of TSky's for each location in the trail.
 * objects are added and controlled from the popup; we only ever actually
 *   discard an object if the whole db changes or it is off when we get an
 *   update.
 */
typedef struct {
    unsigned char flags;/* OBJF_* flags (shared with Obj->o_flags) */
    TrTS trts;		/* mjd of o and whether to draw timestamp */
    Obj o;		/* copy of the object at ts_mjd */
} TSky;
typedef struct _trailObj {
    struct _trailObj *ntop;	/* pointer to next, or NULL */
    int on;		/* turn trail on/off (discarded if off during update) */
    TrState trs;	/* general details of the trail setup */
    Obj *op;		/* pointer to actual db object being trailed */
    int nsky;		/* number of items in use within sky[] */
    TSky sky[1];	/* array of Objs (gotta love C :-) */
} TrailObj;

static double x_angle (double ra, double dec, double pa);
static int sv_ano (double *ap, double *bp, int *xp, int *yp, int w2x, int arg);
static int objdiam (Obj *op);
static void sv_copy_sky (void);
static void sv_create_svshell (void);
static void sv_create_favmenu (Widget parent);
static void sv_create_options (void);
static void sv_create_op_lbl (Widget rc_w);
static void sv_create_grid_option (Widget f_w);
static void sv_closeopdialog_cb (Widget w, XtPointer client,XtPointer call);
static void sv_draw_pl (Display *dsp, Drawable win, GC gc, Obj *op, int x,
    int y, int diam);
static void sv_opdialog_cb (Widget w, XtPointer client, XtPointer call);
static void sv_close_cb (Widget w, XtPointer client, XtPointer call);
static void sv_popdown_cb (Widget w, XtPointer client, XtPointer call);
static void sv_help_cb (Widget w, XtPointer client, XtPointer call);
static void sv_helpon_cb (Widget w, XtPointer client, XtPointer call);
static void sv_aa_cb (Widget w, XtPointer client, XtPointer call);
static void sv_cyl_cb (Widget w, XtPointer client, XtPointer call);
static void sv_filter_cb (Widget w, XtPointer client, XtPointer call);
static void sv_fits_cb (Widget w, XtPointer client, XtPointer call);
static void sv_image_cb (Widget w, XtPointer client, XtPointer call);
static void sv_registration_cb (Widget w, XtPointer client, XtPointer call);
static void sv_wcs_cb (Widget w, XtPointer client, XtPointer call);
static void sv_addframe_cb (Widget w, XtPointer client, XtPointer call);
static void sv_gt_cb (Widget w, XtPointer client, XtPointer call);
static void sv_tcp_cb (Widget w, XtPointer client, XtPointer call);
static void sv_indi_cb (Widget w, XtPointer client, XtPointer call);
static void sv_grid_cb (Widget w, XtPointer client, XtPointer call);
static void sv_brs_cb (Widget w, XtPointer client, XtPointer call);
static void sv_gridtf_cb (Widget w, XtPointer client, XtPointer call);
static void sv_list_cb (Widget w, XtPointer client, XtPointer call);
static void sv_mancoord_cb (Widget w, XtPointer client, XtPointer call);
static void sv_print_cb (Widget w, XtPointer client, XtPointer call);
static void sv_print (void);
static void sv_ps_annotate (Now *np);
static void sv_track_cb (Widget w, XtPointer client, XtPointer call);
static void sv_set_fov (double fov);
static void sv_da_exp_cb (Widget w, XtPointer client, XtPointer call);
static void sv_resize (int neww, int newh);
static void sv_pick (XEvent *ev, int wx, int wy);
static void sv_da_ptr_eh (Widget w, XtPointer client, XEvent *ev,
    Boolean *continue_to_dispatch);
static void sv_scale_cb (Widget w, XtPointer client, XtPointer call);
static void sv_lbln_cb (Widget w, XtPointer client, XtPointer call);
static void sv_lblm_cb (Widget w, XtPointer client, XtPointer call);
static void sv_option_cb (Widget w, XtPointer client, XtPointer call);
static void sv_eyep_cb (Widget w, XtPointer client, XtPointer call);
static void sv_altdecsc_cb (Widget w, XtPointer client, XtPointer call);
static void sv_fovsc_cb (Widget w, XtPointer client, XtPointer call);
static void sv_faving_cb (Widget wid, XtPointer client, XtPointer call);
static void sv_fav_cb (Widget w, XtPointer client, XtPointer call);
static void sv_bsmap_cb (Widget w, XtPointer client, XtPointer call);
static void sv_loadcnsfigs_cb (Widget w, XtPointer client, XtPointer call);
static void sv_popup (XEvent *ev, Obj *op, TSky *tsp);
static void sv_punames (Obj *op);
static void sv_riset (Now *np, Obj *op);
static void sv_create_popup (void);
static void sv_magscale (Display *dsp, Window win);
static void sv_create_zoomcascade (Widget pd_w);
static void sv_create_labellr (Widget pd_w);
static void sv_pu_activate_cb (Widget w, XtPointer client, XtPointer call);
static void sv_pu_zoom_cb (Widget w, XtPointer client, XtPointer call);
static void sv_pu_trail_cb (Widget wid, XtPointer client, XtPointer call);
static void sv_pu_label_cb (Widget wid, XtPointer client, XtPointer call);
static void sv_pu_track_cb (Widget wid, XtPointer client, XtPointer call);
static void sv_telpd_cb (Widget wid, XtPointer client, XtPointer call);
static void sv_dtelpd_cb (Widget wid, XtPointer client, XtPointer call);
static void sv_etelpd_cb (Widget wid, XtPointer client, XtPointer call);
static void sv_addtelpd (Obj *op);
static void sv_erasetelpd (void);
static void sv_drawBin (Obj *op, XArc *xfap[], XSegment *xsp[], int diam,
    int x, int y);
static void sv_read_scale (int which);
static void sv_set_scale (int which, int cutzoom);
static void sv_erase_report_coords (void);
static void sv_draw_report_coords (int new, int sep, int winx, int winy,
    double altdec, double azra);
static void sv_drawtm (Display *dsp, Window win, int x, int y);
static void sv_drawm (Display *dsp, Window win, int x, int y);
static int sv_mark (Obj *op, int in_center, int out_center, int mark,
    int msg, int use_window, double newfov);
static void sv_set_dashed_cnsgc (Display *dsp, int dashed);
static void sv_win2im (int wx, int wy, double *ix, double *yp);
static void svtb_sync (void);
static void tobj_rmoff (void);
static void tobj_rmobj (Obj *op);
static TrailObj *tobj_addobj (Obj *op, int nsky);
static void tobj_newdb (void);
static TrailObj *tobj_find (Obj *op);
static void tobj_display_all (void);
static void aatwarn_msg (void);
static int sv_mktrail (TrTS ts[], TrState *statep, XtPointer client);
static int sv_dbobjloc (Obj *op, int *xp, int *yp, int *dp);
static int sv_drawComet (XArc xfa[], int diam, int x, int y);
static int sv_trailobjloc (TSky *tsp, int *xp, int *yp);
static int sv_precheck (Obj *op);
static int sv_onscrn (Now *np, Obj *op);
static int sv_loc (double altdec, double azra, int *xp, int *yp);
static int sv_unloc (int x, int y, double *altdecp, double *azrap);
static void sv_fullwhere (Now *np, double altdec, double azra, int aa,
    double *altp, double *azp, double *rap, double *decp);
static void draw_hznmap (Now *np, Display *dsp, Window win, GC gc);
static void draw_hznac (Display *dsp, Window win, GC gc);
static void draw_hznother (Display *dsp, Window win, GC gc);
static void draw_hznProfile (Display *dsp, Window win, GC gc);
static void draw_compass (Display *dsp, Drawable win);
static void draw_ecliptic (Now *np, Display *dsp, Window win, GC gc);
static void draw_umbra (Now *np, Display *dsp, Window win, GC gc);
static void draw_equator (Now *np, Display *dsp, Window win, GC gc);
static void draw_galactic (Now *np, Display *dsp, Window win, GC gc);
static int split_line (Display *dsp, Window win, GC gc, int x1, int y1,
    int x2, int y2);
static int split_lines (Display *dsp, Window win, GC gc,XPoint xp[],int np);
static int split_segs (Display *dsp, Window win,GC gc,XSegment sp[],int ns);
static void split_wrap (XSegment *sp, int *xwp, int *ywp);
static void draw_milkyway (Now *np, Display *dsp, Window win, GC gc);
static void draw_cnsbounds (Now *np, Display *dsp, Window win);
static void draw_cns (Now *np, Display *dsp, Window win);
static void draw_cnsname ( Display *dsp, Window win, int conid,
    int minx, int miny, int maxx, int maxy);
static void draw_grid_label (Display *dsp, Window win, GC gc, double ad,
    double ar, int x, int y, int samesys, int arlabel, double dv, double dh);
static void draw_grid (Display *dsp, Window win, GC gc);
static void draw_allobjs (Display *dsp, Drawable win);
static void draw_label (Window win, GC gc, Obj *op, int flags, int x, int y,
    int d);
static int chk_greeklabel (char name[], int *glp, char *gcodep);
static void load_deffig (Widget om);
static void gridStepLabel (void);
static void draw_eyep (Display *dsp, Window win, GC gc);
static void setImFOVScales (FImage *fip);
static void objGC(Obj *op, int d, GC *gcp);
static void sv_mk_gcs (void);
static int segisvis (int x1, int y1, int x2, int y2,
    int *cx1, int *cy1, int *cx2, int *cy2);
static void chkFS_to (XtPointer client, XtIntervalId *id);

#define	MAXGRID		20	/* max grid lines each dimension */
#define	GMARG		20	/* min margin of grid label from edge */
#define	NGSEGS		3	/* piecewise segments per grid arc */
#define	NHZNSEG		360	/* number of horizon map segments */
#define	EQ_NON		3	/* dots on in equator line pattern */
#define	EQ_NOFF		6	/* dots off in equator line pattern */
#define	ECL_NON		2	/* dots on in ecliptic line pattern */
#define	ECL_NOFF	2	/* dots off in ecliptic line pattern */
#define	GAL_NON		1	/* dots on in galaxy plane line pattern */
#define	GAL_NOFF	4	/* dots off in galaxy plane line pattern */
#define	GAL_W		5	/* half-width of galactic pole X */
#define	MARKR		20	/* pointer marker half-width, pixels */
#define	TICKLEN		2	/* length of trail tickmarks, pixels */
#define	MINEPR		1	/* minimum eyepiece radius we'll draw, pixels */
#define	MIND		5	/* min diam for non-star non-planet symbols*/
#define MAXBRLBLS       100	/* max number of brightest objects we label */
#define	MAGBOXN		6	/* number of mag steps to show in box */
#define	FOV_STEP	5	/* FOV min and step size, arc mins */
#define	MAXPMOK		30	/* min days for fs reload (Barnard's ~ 1"/mo) */
#define	FSTO		1000	/* field star load timer, ms */
#define	MAXFSFOV	30.0	/* max fov we ever load field stars, degs */
#define	ASR		4	/* anti-solar marker radius, pixels */
#define	TBORD		4	/* tracking border from edge, pixels */
#define	UDLRF		8	/* up/dn/lf/rt toolbar motion fraction */
#define	LOSTBSZ		10	/* Label option TB and Scale size */
#define	MAXXW		16000	/* max X Win pos (SHRT_MAX broke XFree 4.2.0) */
#define	PUTW		10	/* popup line title width */

static char telAnon[] = "Anonymous";	/* name used for anon (pure loc) objs */
static char cns_suffix[] = ".csf";	/* constellation figure file suffix */
static char cns_ffres[] = "CnsFigFile";	/* resource with default cons figures */

Widget svshell_w;		/* main sky view shell */
static Widget svda_w;		/* sky view drawing area */
static Widget svops_w;		/* main options dialog */
static Widget fov_w, altdec_w, azra_w;	/* scale widgets ... */
static Widget fovl_w, altdecl_w, azral_w; /*   ... and their labels */
static Widget hgrid_w, vgrid_w;	/* h and v grid spacing TF */
static Widget hgridl_w, vgridl_w;	/* h and v grid spacing labels */
static Widget grid_w;		/* want grid TB */
static Widget autogrid_w;	/* want auto grid spacing TB */
static Widget gridlbl_w;	/* want grid labels TB */
static Widget aagrid_w;		/* want AA grid TB */
static Widget naagrid_w;	/* notwant AA grid TB */
static Widget aa_w, rad_w;	/* altaz/radec toggle buttons */
static Widget sph_w, cyl_w;	/* spherical/cylindrical toggle buttons */
static Widget fliplr_w, fliptb_w;	/* orientation TBs */
static Widget conn_w;		/* constellation name TB */
static Widget conf_w;		/* constellation fig TB */
static Widget conb_w, cona_w;	/* constellation boundry and abbr TBs */
static Widget justd_w;		/* justdots TB */
static Widget eclip_w, galac_w, eq_w;	/* eclip, galactic, equator TBs */
static Widget dt_w;		/* the date/time stamp label widget */
static Widget fovimg_w;		/* image aspect PB */
static Widget keeptelvis_w;	/* TB whether to keep tel marker visible */
static Pixmap sv_pm;		/* off-screen pixmap we *really* draw */
static XFontStruct *sv_tf;	/* font for the tracking coord display*/
static XFontStruct *sv_pf;	/* font for all object labels */
static XFontStruct *sv_gf;	/* greek font */
static XFontStruct *sv_cf;	/* constellation names font */
static XFontStruct *sv_rf;	/* grid font */
static GC sv_ggc;		/* greek gc */
static Pixmap trk_pm;		/* used to draw track info smoothly */
static unsigned int trk_w, trk_h;	/* size of trk_pm, iff trk_pm */

/* pixels and GCs
 */
static Pixel annot_p;		/* annotation color */
static Pixel bg_p;		/* background color */
static Pixel cnsfig_p;		/* constellation figures color */
static Pixel cnsbnd_p;		/* constellation boundaries color */
static Pixel cnsnam_p;		/* constellation name color */
static Pixel sky_p;		/* sky background color */
static Pixel hzn_p;		/* horizon profile color */
static Pixel grid_p;		/* grid color */
static Pixel eq_p;		/* equator color */
static Pixel mw_p;		/* milky way color */
static GC sv_gc;		/* the default GC */
static GC sv_tmgc;		/* gc for transient window marker */
static GC sv_cnsgc;		/* the GC for constellations */
static GC sv_strgc;		/* gc for use in drawing text */
static GC zm_xorgc;		/* gc for zoom box */
static GC dimgc;		/* gray for small tiny stars */

static int aa_mode = -1;	/* 1 for alt/az or 0 for ra/dec */
static int cyl_proj = -1;	/* 1 for cylindrical proj or 0 for spherical */
static double sv_altdec;	/* view center alt or dec, rads */
static double sv_azra;		/* view center az or ra, rads */
static double sv_vfov;		/* vertical field of view, rads */
static double sv_hfov;		/* horizontal field of view, rads */
static double sv_dfov;		/* diagonal field of view, rads */
static int sv_w, sv_h;		/* size of svda_w, pixels */
static int justdots;		/* set when only want to use dots on the map */
static int want_livedrag;	/* set when want live dragging */
static int flip_lr;		/* set when want to flip left/right */
static int flip_tb;		/* set when want to flip top/bottom */
static int want_ec;		/* set when want to see the ecliptic */
static int want_eq;		/* set when want to see the equator */
static int want_ga;		/* set when want to see galactic poles and eq*/
static int want_hznmap;		/* set when want to see horizon map */
static int want_clipping;	/* set when not want to see objects below horizon */
static int want_conb;		/* set when want to see the constel boundaries*/
static int want_conf;		/* set when want to see the constel figures */
static int want_conn;		/* set when want to see the constel names */
static int want_cona;		/* set when want to see the constel abbrevs */
static int want_eyep;		/* set when want to see eyepieces */
static int want_grid;		/* set when we want to draw the coord grid */
static int want_aagrid;		/* set when we want an AA grid */
static int want_gridlbl;	/* set when we want grid labeling */
static int want_autogrid;	/* set when we want auto coord spacing */
static int want_fs;		/* set when we want to see field stars */
static int want_magscale;	/* set when we want to see the mag scale */
static int want_compass;	/* set when we want to see the compass rose */
static int want_automag;	/* set when we want to automatically set mags */
static int user_automagoff;	/* set when user manually turns off automag */
static int anytrails;		/* set to add postscript trail time comment */
static int want_report;         /* set when we want live report */

static ObjF *fldstars;		/* malloced list of field stars, or NULL */
static int nfldstars;		/* number of entries in fldstars[] */

static TrailObj *trailobj;	/* head of a malloced linked-list -- 0 when
				 * empty
				 */
static TrState trstate = {	/* trail setup state */
    TRLR_FL, TRI_DAY, TRF_DATE, TRR_NONE, TRO_PATHL, TRS_MEDIUM, 10
};
static Obj *track_op;		/* object to track, when not NULL */
static Widget tracktb_w;	/* toggle button indicating tracking is on */
static Widget wanteyep_w;	/* TB for whether to show eyepieces */
static Widget wantfs_w;		/* TB for whether to show field stars */
static Widget wantmag_w;	/* TB for whether to show mag scale */
static Widget wantcompass_w;	/* TB for whether to show compass rose */
static Widget wantamag_w;	/* TB for whether to automatically set mag */
static Widget hznmap_w;		/* TB for whether to show horizon map */
static Widget ttbar_w;		/* top toolbar RC */
static Widget ltbar_w;		/* left toolbar RC */
static Widget rtbar_w;		/* right toolbar RC */
static Widget telpd_w;		/* Telescope pulldown, for adding histories */
static Widget wantreport_w;     /* TB for whether to show live report */

static int lbl_lst;		/* catalog star name/mag label options */
static int lbl_lfs;		/* field star name/mag label options */
static int lbl_lss;		/* sol sys name/mag label options */
static int lbl_lds;		/* deep sky name/mag label options */
static Widget lbl_nst_w;	/* TB for star name label */
static Widget lbl_mst_w;	/* TB for star mag label */
static Widget lbl_nfs_w;	/* TB for field star name label */
static Widget lbl_mfs_w;	/* TB for field star mag label */
static Widget lbl_nss_w;	/* TB for sol sys name label */
static Widget lbl_mss_w;	/* TB for sol sys mag label */
static Widget lbl_nds_w;	/* TB for deep sky name label */
static Widget lbl_mds_w;	/* TB for deep sky mag label */
static Widget lbl_bst_w;	/* Scale for brightest stars */
static Widget lbl_bfs_w;	/* Scale for brightest stars */
static Widget lbl_bss_w;	/* Scale for brightest sol sys */
static Widget lbl_bds_w;	/* Scale for brightest deep sky */

static Pixel eyep_p;		/* eyepiece color */
static EyePiece *largeyepr;	/* largest eyepiece printed, for print label */
static int neyepr;		/* number of eyepieces printed, " */

static int sv_tmx;		/* last transient marker x coord */
static int sv_tmy;		/* last transient marker y coord */
static int sv_tmon;		/* whether transient marker is on screen now */

/* info for the popup widget.
 * we make one once and keep reusing it -- seems to be a bit faster that way.
 * but see sv_popup() for all the work to get it customized for each object.
 */
typedef struct {
    int wx, wy;			/* screen coords of cursor at moment of click */
    Widget pu_w;		/* the overall PopupMenu */
    Widget name_w;		/* label for object name */
    Widget altnm_w;		/* cascade button for alternate names */
    Widget desc_w;		/* label for object description */
    Widget infocb_w;		/* CB for misc object info */
    Widget spect_w;		/* label for object spectral class */
    Widget size_w;		/* label for object size */
    Widget pa_w;		/* label for object position angle */
    Widget eclipsed_w;		/* label for whether esat is eclipsed */
    Widget ud_w;		/* label for object date */
    Widget ut_w;		/* label for object time */
    Widget rise_w;      	/* label for object rise time */
    Widget trans_w;     	/* label for object transit time */
    Widget transalt_w;     	/* label for object transit altitude */
    Widget transaz_w;     	/* label for object transit azimuth */
    Widget set_w;       	/* label for object set time */
    Widget ra_w;		/* label for object RA */
    Widget dec_w;		/* label for object Dec */
    Widget alt_w;		/* label for object Alt */
    Widget az_w;		/* label for object Az */
    Widget nmsep_w;		/* separator under obj name */
    Widget mag_w;		/* label for object mag */
    Widget refmag_w;		/* PB to set mag as photom ref */
    Widget deleyep_w;		/* PB to delete an eyepiece */
    Widget goto_w;		/* PB to send coords to telescope */
    Widget gal_w;		/* PB to show gallery image */
    Widget av_w;		/* PB to load nearest AAVSO */
    Widget label_w;		/* CB to select left/label */
    Widget llabel_w;		/* TB to select left label be drawn */
    Widget rlabel_w;		/* TB to select right label be drawn */
    Widget fav_w;		/* PB to add this obj to favs list */
    Widget obslog_w;		/* PB to add this obj to observer's log */
    Widget track_w;		/* TB to track this object */
    Widget trail_w;		/* TB to let existing trail be turned on/off */
    Widget newtrail_w;		/* PB to create new trail */
    Widget bsmap_w;		/* PB to bring up binary star orbit */
    Obj *op;			/* real database pointer, or possibly svobj */
    TSky *tsp;			/* used iff we are displaying a trailed object*/
} Popup;
static Popup pu;
static Obj svobj;		/* used to point when far from any real object*/

/* popup button activate codes */
enum PUB {
    AIM, SETPHOTOMREF, PEYEPIECE, DEYEPIECE, TGOTO, OBSLOG,
    GALLERY, LOADAV, ADD2FAV, NEWTRAIL
};

enum SCALES {			/* scale changed codes */
    FOV_S, ALTDEC_S, AZRA_S
};

/* zoom support */
static ZM_Undo *zm_undo;	/* malloced list of zoom undo info */
static int zm_nundo;		/* n valid entries in zm_undo */
static int zm_cundo;		/* current depth on zm_undo */
static ZM_Undo wzm;		/* working zoom while drawing */
static void zm_noundo (void);
static int zm_addundo (void);
static void zm_flip (int lr);
static void zm_installundo (void);
static void zm_cutundo (void);
static void zm_draw (void);

/* connect handler for some fun kb shortcuts */
static void sv_shortcuts (Widget w, XEvent *e,String *args,Cardinal *nargs);
static XtActionsRec scuts[] = {
    {"SVScut", sv_shortcuts}
};

static char skyocategory[] = "Sky View -- Options";	/* Save category */
char skycategory[] = "Sky View";			/* Save category */
static char svtrres[] = "SkyViewTrailState";		/* trail resource */

/* used for creating the Options menu */
typedef struct {
    char *label;	/* toggle button label */
    char oneofmany;	/* 1 for ONE_OF_MANY, 0 for N_OF_MANY */
    char *name;		/* instance name */
    int *flagp;		/* address of flag it controls */
    Widget *wp;		/* controlling widget, or NULL */
    char *tip;		/* tip text */
} ViewOpt;

/* cursor used while mag glass is on */
static Cursor magglass_cursor;

static XtIntervalId fs_to;	/* set while checking field stars */

/* bring up Sky view shell, creating if first time */
void
sv_manage ()
{
	if (!svshell_w) {
	    sv_create_svshell();
	    indi_createShell();
	}

	sfifo_openin();
        XtPopup (svshell_w, XtGrabNone);
	set_something (svshell_w, XmNiconic, (XtArgVal)False);
	if (fs_to != 0) {
	    XtRemoveTimeOut (fs_to);
	    fs_to = 0;
	}
	fs_to = XtAppAddTimeOut (xe_app, FSTO, chkFS_to, 0);
	/* rely on expose to cause a fresh update */

	setXRes (sv_viewupres(), "1");
}

/* called when basic resources change.
 * rebuild and redraw.
 */
void
sv_newres()
{
	if (!svshell_w)
	    return;
	sv_mk_gcs();
	svtb_newpm(ttbar_w);
	sv_update (mm_get_now(), 1);
}

/* return true if sky view is up now and ok to draw to, else 0 */
int
sv_ison()
{
	return(isUp(svshell_w) && sv_pm);
}

/* called when we are to update our view.
 * don't bother if we are not managed or if fields are off.
 * discard any trails that have been turned off.
 * reaim if we are tracking an object.
 */
/* ARGSUSED */
void
sv_update (np, how_much)
Now *np;
int how_much;
{
        if (!svshell_w || !sv_pm)
	    return;
	if (!sv_ison() && !how_much)
	    return;

	/* remove trails not on now */
	tobj_rmoff();

	if (track_op) {
	    db_update (track_op);
	    if (sv_mark (track_op, 1, 1, 0, 1, 0, 0.0) < 0)
		sv_all(np);	/* still need it even if mark failed */
	} else 
	    sv_all(np);
}

/* mark a new scope location.
 * if inview, move sky view if necessary to keep it visible.
 */
void
sv_scopeMark (Obj *op)
{
	if (sv_ison())
	    sv_mark (op, 0, XmToggleButtonGetState(keeptelvis_w), 1, 0, 1, 0.0);
}

/* called when the db beyond the builtin objects has changed.
 * if it was appended to we can just redraw else it was reduced and we need to
 * discard any trails we are keeping for objects that no longer exist and
 * make sure no pending trail creations can come in and do something.
 */
void
sv_newdb(appended)
int appended;
{
	if (!appended) {
	    tobj_newdb();
	    pu.op = NULL;	/* effectively disables sv_mktrail() */
	}

	sv_update (mm_get_now(), 1);
}

/* display a new FITS image */
void
sv_newFITS ()
{
	double ra, dec;
	FImage *fip;

	watch_cursor(1);

	/* fetch the FITS descriptor */
	fip = si_getFImage();
	if (!fip) {
	    printf ("newfits Bug! FITS file disappeared in newfits()!\n");
	    abort();
	}

	/* set loc and size .. if have WCS fields */
	if (xy2RADec (fip, fip->sw/2.0, fip->sh/2.0, &ra, &dec) == 0) {

	    /* !aa_mode and sph_proj.
	     * N.B. do this before calling sv_set_scale().
	     */
	    if (aa_mode) {
		aa_mode = 0;
		XmToggleButtonSetState (aa_w, aa_mode, False);
		XmToggleButtonSetState (rad_w, !aa_mode, False);
	    }
	    if (cyl_proj) {
		cyl_proj = 0;
		XmToggleButtonSetState (cyl_w, cyl_proj, False);
		XmToggleButtonSetState (sph_w, !cyl_proj, False);
		svtb_updateCyl (cyl_proj);
	    }

	    /* set aim scales */
	    sv_altdec = dec;
	    sv_set_scale(ALTDEC_S, 1);
	    sv_azra = ra;
	    sv_set_scale(AZRA_S, 1);

	    /* FOV */
	    setImFOVScales (fip);
	}

	/* discard zooms */
	zm_noundo();

	/* let's have a look! */
	sv_dspFITS();
	watch_cursor(0);
}

/* called to display a FITS image whose aim and scale have already been set. */
void
sv_dspFITS ()
{
	/* build a pixmap to match our current size and orientation */
	si_newPixmap (sv_w, sv_h, flip_lr, flip_tb, zm_undo, zm_cundo);

	/* display */
	sv_all(NULL);
}

/* called to turn on the horizon. */
void
sv_hznOn()
{
	if (!want_hznmap)
	    XmToggleButtonSetState (hznmap_w, True, True);
}

/* whether to show TB that sets whether to keep telescope marker visible */
void
sv_showkeeptelvis (int on)
{
	if (on)
	    XtManageChild (keeptelvis_w);
	else
	    XtUnmanageChild (keeptelvis_w);
}

/* return the name of the resource containing whether this view is up */
char *
sv_viewupres()
{
	return ("SkyViewUp");
}

/* set FOV scales to current /image/ display.
 * mimic si_newPixmap's cropping behavior.
 */
static void
setImFOVScales (fip)
FImage *fip;
{
	double ra1, dec1, ra2, dec2, cvfov, vfov;

	/* use image height -- not critical to be exact */
	if (fip && xy2RADec (fip, fip->sw/2.0, 0.0, &ra1, &dec1) == 0) {
	    (void) xy2RADec (fip, fip->sw/2.0, fip->sh-1, &ra2, &dec2);
	    solve_sphere (ra2-ra1, PI/2-dec1, sin(dec2), cos(dec2), &cvfov, 0);
	    vfov = acos (cvfov);
	    if (sv_h*fip->sw < sv_w*fip->sh)
		vfov *= (double)sv_h/sv_w;
	    sv_set_fov (vfov);
	    sv_set_scale(FOV_S, 1);
	}
}

/* draw a circle of radius rad on screen over image coords ix/iy.
 * color has no meaning, but different values may result in different colors.
 * this is not for heavy use, just a simple means for files that otherwise have
 *   no need for X stuff to draw a little on the sky view window.
 */
void
sv_drawimdot (ix, iy, rad, color)
double ix, iy;
int rad, color;
{
	static Pixel *colrs[] = {&eq_p, &cnsfig_p, &grid_p};
	Display *dsp = XtDisplay (svda_w);
	Window win = XtWindow (svda_w);
	int wx, wy;

	if (!si_ison() || !win)
	    return;
	si_im2win (ix, iy, sv_w, sv_h, &wx, &wy);
	if (flip_lr)
	    wx = sv_w - 1 - wx;
	if (flip_tb)
	    wy = sv_h - 1 - wy;
	XSetForeground (dsp, sv_gc, *colrs[color%XtNumber(colrs)]);
	if (rad < 1) rad = 1;	/* friendly clamp */
	XDrawArc (dsp, win, sv_gc, wx-rad, wy-rad, 2*rad, 2*rad, 0, 360*64);
	XSync (dsp, False);
}

/* called to reinstate a SvHistory */
void
svh_goto (SvHistory *hp)
{
	/* don't even try to get back a FITS image */
	si_off();

	/* restore aa_mode.
	 * N.B. do this before calling sv_set_scale().
	 */
	if (hp->aa_mode != aa_mode) {
	    aa_mode = hp->aa_mode;
	    XmToggleButtonSetState (aa_w, aa_mode, False);
	    XmToggleButtonSetState (rad_w, !aa_mode, False);
	}

	/* restore aim scales */
	sv_altdec = hp->altdec;
	sv_set_scale (ALTDEC_S, 1);
	sv_azra = hp->azra;
	sv_set_scale (AZRA_S, 1);

	/* restore fov scale */
	sv_set_fov (hp->fov);
	sv_set_scale(FOV_S, 1);

	/* restore flip orientation */
	XmToggleButtonSetState (fliplr_w, flip_lr = hp->flip_lr, False);
	XmToggleButtonSetState (fliptb_w, flip_tb = hp->flip_tb, False);

	/* restore mag limits and filter settings */
	svf_setmaglimits (hp->stmag, hp->ssmag, hp->dsmag, hp->magstp);
	svf_settables (hp->type_table, hp->fclass_table);

	/* restore the grid options */
	XmToggleButtonSetState (grid_w, want_grid = hp->grid, False);
	XmToggleButtonSetState (autogrid_w, want_autogrid = hp->autogrid,False);
	XmToggleButtonSetState (aagrid_w, want_aagrid = hp->aagrid, False);
	XmToggleButtonSetState (naagrid_w, !want_aagrid, False);
	XmToggleButtonSetState (gridlbl_w, want_gridlbl = hp->gridlbl, False);
	XmTextFieldSetString (vgrid_w, hp->vgrid);
	XmTextFieldSetString (hgrid_w, hp->hgrid);
	gridStepLabel();

	/* restore labeling options */
	lbl_lst = hp->lbl_lst;
	XmToggleButtonSetState(lbl_nst_w, !!(lbl_lst & OBJF_NLABEL), False);
	XmToggleButtonSetState(lbl_mst_w, !!(lbl_lst & OBJF_MLABEL), False);
	lbl_lfs = hp->lbl_lfs;
	XmToggleButtonSetState(lbl_nfs_w, !!(lbl_lfs & OBJF_NLABEL), False);
	XmToggleButtonSetState(lbl_mfs_w, !!(lbl_lfs & OBJF_MLABEL), False);
	lbl_lss = hp->lbl_lss;
	XmToggleButtonSetState(lbl_nss_w, !!(lbl_lss & OBJF_NLABEL), False);
	XmToggleButtonSetState(lbl_mss_w, !!(lbl_lss & OBJF_MLABEL), False);
	lbl_lds = hp->lbl_lds;
	XmToggleButtonSetState(lbl_nds_w, !!(lbl_lds & OBJF_NLABEL), False);
	XmToggleButtonSetState(lbl_mds_w, !!(lbl_lds & OBJF_MLABEL), False);
	XmScaleSetValue (lbl_bst_w, hp->lbl_bst);
	XmScaleSetValue (lbl_bfs_w, hp->lbl_bfs);
	XmScaleSetValue (lbl_bss_w, hp->lbl_bss);
	XmScaleSetValue (lbl_bds_w, hp->lbl_bds);

	/* restore other misc options */
	XmToggleButtonSetState (justd_w, justdots = hp->justd, False);
	XmToggleButtonSetState (eclip_w, want_ec = hp->eclip, False);
	XmToggleButtonSetState (eq_w, want_eq = hp->eq, False);
	XmToggleButtonSetState (galac_w, want_ga = hp->galac, False);
	XmToggleButtonSetState (hznmap_w, want_hznmap = hp->hznmap, False);
	XmToggleButtonSetState (conb_w, want_conb = hp->conb, False);
	XmToggleButtonSetState (conf_w, want_conf = hp->conf, False);
	XmToggleButtonSetState (conn_w, want_conn = hp->conn, False);
	XmToggleButtonSetState (cona_w, want_cona = hp->cona, False);
	XmToggleButtonSetState (wanteyep_w, want_eyep = hp->eyep, False);
	XmToggleButtonSetState (wantmag_w, want_magscale = hp->magscale, False);
	XmToggleButtonSetState (wantamag_w, want_automag = hp->automag, False);
	XmToggleButtonSetState (cyl_w, cyl_proj = hp->cyl_proj, False);
	XmToggleButtonSetState (sph_w, !hp->cyl_proj, False);

	/* set to prevent automag from coming on and wiping out restored mags */
	if (!want_automag)
	    user_automagoff = 1;

	/* sync the toolbar to match */
	svtb_sync();

	/* resize will do it all */
	sv_resize (hp->winw, hp->winh);
}

/* called by the history mechanism when it needs to know the current settings.
 */
void
svh_get (SvHistory *hp)
{
	Dimension w, h;
	char *str;
	Arg args[10];
	int n;

	hp->fov = sv_vfov;
	hp->altdec = sv_altdec;
	hp->azra = sv_azra;
	hp->aa_mode = aa_mode;
	hp->flip_lr = flip_lr;
	hp->flip_tb = flip_tb;
	svf_getmaglimits (&hp->stmag, &hp->ssmag, &hp->dsmag, &hp->magstp);
	hp->justd = justdots;
	hp->eclip = want_ec;
	hp->eq = want_eq;
	hp->galac = want_ga;
	hp->hznmap = want_hznmap;
	hp->conb = want_conb;
	hp->conf = want_conf;
	hp->conn = want_conn;
	hp->cona = want_cona;
	hp->eyep = want_eyep;
	hp->magscale = want_magscale;
	hp->automag = want_automag;
	hp->cyl_proj = cyl_proj;

	hp->grid = want_grid;
	hp->autogrid = want_autogrid;
	hp->aagrid = want_aagrid;
	hp->gridlbl = want_gridlbl;
	str = XmTextFieldGetString (vgrid_w);
	(void) strcpy (hp->vgrid, str);
	XtFree (str);
	str = XmTextFieldGetString (hgrid_w);
	(void) strcpy (hp->hgrid, str);
	XtFree (str);

	hp->lbl_lst = lbl_lst;
	hp->lbl_lfs = lbl_lfs;
	hp->lbl_lss = lbl_lss;
	hp->lbl_lds = lbl_lds;
	XmScaleGetValue (lbl_bst_w, &hp->lbl_bst);
	XmScaleGetValue (lbl_bfs_w, &hp->lbl_bfs);
	XmScaleGetValue (lbl_bss_w, &hp->lbl_bss);
	XmScaleGetValue (lbl_bds_w, &hp->lbl_bds);
	svf_gettables (hp->type_table, hp->fclass_table);

	n = 0;
	XtSetArg (args[n], XmNwidth, (XtArgVal)&w); n++;
	XtSetArg (args[n], XmNheight, (XtArgVal)&h); n++;
	XtGetValues (svda_w, args, n);
	hp->winw = w;
	hp->winh = h;
}

/* turn off auto mag for an internal logic reason.
 * N.B. do not use this to implement user turning off manually, that would
 *   interfere with user_automagoff.
 */
void
sv_amagoff ()
{
	want_automag = 0;
	XmToggleButtonSetState (wantamag_w, False, False);
	svtb_updateAutoMag (0);
}

/* called when the toolbar "brighter" PB is hit */
void
svtb_brighter_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int stmag, ssmag, dsmag, magstp;

	svf_getmaglimits (&stmag, &ssmag, &dsmag, &magstp);
	stmag += 1;
	ssmag += 1;
	dsmag += 1;
	sv_amagoff ();
	svf_setmaglimits (stmag, ssmag, dsmag, magstp);
	sv_all(mm_get_now());
}

/* called when either toolbar "flip" PB is hit.
 * client is 0 for l/r, else 1 for t/b
 */
void
svtb_flip_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int tb = (long int)client;

	if (tb)
	    XmToggleButtonSetState (fliptb_w, flip_tb ^= 1, True);
	else
	    XmToggleButtonSetState (fliplr_w, flip_lr ^= 1, True);
}

/* called when the toolbar "constel" TB changes */
void
svtb_constel_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static int want_conb_atoff = -1;	/* flag as uninitialized */
	static int want_conf_atoff;
	static int want_conn_atoff;
	static int want_cona_atoff;

	int set = XmToggleButtonGetState(w);

	if (set) {
	    if (want_conb_atoff < 0) {
		want_conb_atoff = want_conb;
		want_conf_atoff = want_conf;
		want_conn_atoff = want_conn;
		want_cona_atoff = want_cona;
	    }

	    if (!want_conb_atoff && !want_conf_atoff && !want_conn_atoff
							&& !want_cona_atoff)
		want_conf_atoff = 1;

	    /* restore settings when last turned off */
	    XmToggleButtonSetState (conf_w, want_conf=want_conf_atoff, False);
	    XmToggleButtonSetState (cona_w, want_cona=want_cona_atoff, False);
	    XmToggleButtonSetState (conn_w, want_conn=want_conn_atoff, False);
	    XmToggleButtonSetState (conb_w, want_conb=want_conb_atoff, False);
	} else {
	    /* save current settings then turn all off */
	    want_conb_atoff = want_conb;
	    want_conf_atoff = want_conf;
	    want_conn_atoff = want_conn;
	    want_cona_atoff = want_cona;
	    XmToggleButtonSetState (conf_w, want_conf=False, False);
	    XmToggleButtonSetState (cona_w, want_cona=False, False);
	    XmToggleButtonSetState (conn_w, want_conn=False, False);
	    XmToggleButtonSetState (conb_w, want_conb=False, False);
	}

	sv_all(mm_get_now());
}

/* called when the toolbar "dimmer" PB is hit */
void
svtb_dimmer_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int stmag, ssmag, dsmag, magstp;

	svf_getmaglimits (&stmag, &ssmag, &dsmag, &magstp);
	stmag -= 1;
	ssmag -= 1;
	dsmag -= 1;
	sv_amagoff ();
	svf_setmaglimits (stmag, ssmag, dsmag, magstp);
	sv_all(mm_get_now());
}

/* called when the toolbar "grid" TB changes */
void
svtb_grid_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int onnow = XmToggleButtonGetState(grid_w);

	/* rely on redraw in callback */
	if (onnow) {
	    XmToggleButtonSetState (grid_w, False, True);
	} else {
	    /* match grid coord to display mode */
	    want_aagrid = aa_mode;
	    XmToggleButtonSetState (aagrid_w, want_aagrid, False);
	    XmToggleButtonSetState (naagrid_w, !want_aagrid, False);
	    XmToggleButtonSetState (grid_w, True, True);
	}
}

/* called when the toolbar "automag" TB changes */
void
svtb_automag_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonSetState (wantamag_w,
				    !XmToggleButtonGetState(wantamag_w), True);
}

/* called when the toolbar "horizon" TB changes */
void
svtb_hzn_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonSetState (hznmap_w, !XmToggleButtonGetState(hznmap_w), 1);
}

/* called when the toolbar "live report" TB changes */
void
svtb_report_cb(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	want_report = XmToggleButtonGetState(w);
	XmToggleButtonSetState (wantreport_w, want_report, 1);
}


/* called when the toolbar "field stars" TB is hit */
void
svtb_fstars_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonSetState (wantfs_w, !XmToggleButtonGetState(wantfs_w), 1);
}

/* called when the toolbar "names" TB changes */
void
svtb_names_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static int lbl_lst_atoff = -1;		/* flag as uninitialized */
	static int lbl_lfs_atoff;
	static int lbl_lss_atoff;
	static int lbl_lds_atoff;

	int set = XmToggleButtonGetState (w);

	if (set) {
	    if (lbl_lst_atoff < 0) {
		lbl_lst_atoff = lbl_lst;
		lbl_lfs_atoff = lbl_lfs;
		lbl_lss_atoff = lbl_lss;
		lbl_lds_atoff = lbl_lds;
	    }

	    if (!lbl_lst_atoff && !lbl_lss_atoff & !lbl_lds_atoff)
		lbl_lst_atoff = lbl_lss_atoff = lbl_lds_atoff = OBJF_NLABEL;

	    lbl_lst = lbl_lst_atoff;
	    XmToggleButtonSetState(lbl_nst_w, !!(lbl_lst & OBJF_NLABEL), False);
	    XmToggleButtonSetState(lbl_mst_w, !!(lbl_lst & OBJF_MLABEL), False);
	    lbl_lfs = lbl_lfs_atoff;
	    XmToggleButtonSetState(lbl_nfs_w, !!(lbl_lfs & OBJF_NLABEL), False);
	    XmToggleButtonSetState(lbl_mfs_w, !!(lbl_lfs & OBJF_MLABEL), False);
	    lbl_lss = lbl_lss_atoff;
	    XmToggleButtonSetState(lbl_nss_w, !!(lbl_lss & OBJF_NLABEL), False);
	    XmToggleButtonSetState(lbl_mss_w, !!(lbl_lss & OBJF_MLABEL), False);
	    lbl_lds = lbl_lds_atoff;
	    XmToggleButtonSetState(lbl_nds_w, !!(lbl_lds & OBJF_NLABEL), False);
	    XmToggleButtonSetState(lbl_mds_w, !!(lbl_lds & OBJF_MLABEL), False);
	} else {
	    XmToggleButtonSetState (lbl_nst_w, False, False);
	    XmToggleButtonSetState (lbl_mst_w, False, False);
	    lbl_lst_atoff = lbl_lst;
	    lbl_lst = 0;
	    XmToggleButtonSetState (lbl_nfs_w, False, False);
	    XmToggleButtonSetState (lbl_mfs_w, False, False);
	    lbl_lfs_atoff = lbl_lfs;
	    lbl_lfs = 0;
	    XmToggleButtonSetState (lbl_nss_w, False, False);
	    XmToggleButtonSetState (lbl_mss_w, False, False);
	    lbl_lss_atoff = lbl_lss;
	    lbl_lss = 0;
	    XmToggleButtonSetState (lbl_nds_w, False, False);
	    XmToggleButtonSetState (lbl_mds_w, False, False);
	    lbl_lds_atoff = lbl_lds;
	    lbl_lds = 0;
	}

	sv_all(mm_get_now());
}

/* whether roaming report */
int
svtb_reportIsOn()
{
	return (want_report);
}


/* called when the toolbar "planes" TB changes */
void
svtb_planes_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static int want_eq_atoff = -1;		/* flag as uninitialized */
	static int want_ga_atoff;
	static int want_ec_atoff;

	int set = XmToggleButtonGetState (w);

	if (set) {
	    if (want_eq_atoff < 0) {
		want_eq_atoff = want_eq;
		want_ga_atoff = want_ga;
		want_ec_atoff = want_ec;
	    }

	    if (!want_eq_atoff && !want_ga_atoff && !want_ec_atoff)
		want_eq_atoff = want_ga_atoff = want_ec_atoff = 1;

	    XmToggleButtonSetState (eclip_w, want_ec = want_ec_atoff, False);
	    XmToggleButtonSetState (galac_w, want_ga = want_ga_atoff, False);
	    XmToggleButtonSetState (eq_w,    want_eq = want_eq_atoff, False);
	} else {
	    XmToggleButtonSetState (eclip_w, False, False);
	    want_ec_atoff = want_ec;
	    want_ec = 0;
	    XmToggleButtonSetState (galac_w, False, False);
	    want_ga_atoff = want_ga;
	    want_ga = 0;
	    XmToggleButtonSetState (eq_w, False, False);
	    want_eq_atoff = want_eq;
	    want_eq = 0;
	}

	sv_all(mm_get_now());
}

/* called when the toolbar "magscale" TB changes */
void
svtb_magscale_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonSetState (wantmag_w, XmToggleButtonGetState(w), True);
}

/* called when the toolbar "orient" TB changes */
void
svtb_orient_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (aa_mode)
	    XmToggleButtonSetState (rad_w, True, True);
	else
	    XmToggleButtonSetState (aa_w, True, True);
}

/* called when the toolbar "cylindrical projection" TB is hit */
void
svtb_proj_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonSetState (sph_w, !XmToggleButtonGetState(w), True);
}

/* called when the toolbar "print" PB is hit */
void
svtb_print_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XPSAsk ("Sky View", sv_print);
}

/* called when the toolbar "zoom in" PB is hit */
void
svtb_zoomin_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* asserts */
	if (zm_cundo < 0) {
	    printf ("zoom Bug! zm_cundo=%d\n", zm_cundo);
	    abort();
	}

	/* use undo list if in the middle somewhere else create new */
	if (zm_nundo - zm_cundo >= 2) {
	    /* install next from undo stack */
	    zm_cundo++;
	    zm_installundo ();
	    svtb_zoomok(1);
	    svtb_unzoomok(1);
	    if (si_ison())
		sv_dspFITS();
	    else
		sv_all(NULL);
	} else {
	    /* save new */
	    if (zm_addundo() == 0) {
		svtb_zoomok(0);
		if (si_ison())
		    sv_dspFITS();
		else
		    sv_all(NULL);
		svtb_unzoomok(1);
	    }
	    /* else error already reported */
	}
}

/* called when the toolbar "unzoom" PB is hit */
void
svtb_unzoom_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* asserts */
	if (!zm_undo || zm_cundo <= 0) {
	    printf ("unzoom Bug! zm_undo=%d zm_cundo=%d\n", !!zm_undo,zm_cundo);
	    abort();
	}

	/* show previous undo; disable PB if hit the top */
	--zm_cundo;
	zm_installundo();
	svtb_zoomok(1);
	if (si_ison())
	    sv_dspFITS();
	else
	    sv_all(NULL);
	svtb_unzoomok (zm_cundo != 0);	/* no more unzooms? */
}

/* sync the toolbar to match the options */
static void
svtb_sync()
{
	char tt[NOBJTYPES];
	char ct[NCLASSES];

	svtb_updateCyl (cyl_proj);
	svtb_updateGrid (want_grid);
	svtb_updateLRFlip (flip_lr);
	svtb_updateTBFlip (flip_tb);
	svtb_updateAutoMag (want_automag);
	svtb_updateNames (lbl_lst || lbl_lfs || lbl_lss || lbl_lds);
	svtb_updateHorizon (want_hznmap);
	svtb_updateFStars (want_fs);
	svtb_updateCns (want_conb||want_conf||want_conn||want_cona);
	svtb_updatePlanes (want_eq || want_ec || want_ga);
	svtb_updateMagScale (want_magscale);
	svtb_updateReport (want_report);

	svf_gettables (tt, ct);
	svtb_updateCTTT (ct, tt);
}

/* mark the given object, centering if outside current fov.
 * N.B. we do *not* update the s_ fields of op.
 */
void
sv_point (op)
Obj *op;
{
	if (!sv_ison() || !op || op->o_type == UNDEFOBJ)
	    return;

	/* turn off following remote inputs when told explicitly to point */
	XmToggleButtonSetState (keeptelvis_w, False, 1);

	(void) sv_mark (op, 0, 1, 1, 1, 0, 0.0);
}

/* show a marker at the location of the given object *if* it's in fov now.
 * N.B. we do *not* update the s_ fields of op.
 */
void
sv_id (op)
Obj *op;
{
	if (!sv_ison() || !op || op->o_type == UNDEFOBJ)
	    return;

	(void) sv_mark (op, 0, 0, 1, 1, 0, 0.0);
}

/* called to put up or remove the watch cursor.  */
void
sv_cursor (c)
Cursor c;
{
	Window win;

	if (svshell_w && (win = XtWindow(svshell_w)) != 0) {
	    Display *dsp = XtDisplay(svshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}

	if (svops_w && (win = XtWindow(svops_w)) != 0) {
	    Display *dsp = XtDisplay(svops_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* draw everything subject to any filtering.
 */
void
sv_all(np)
Now *np;
{
	Display *dsp = XtDisplay(svda_w);
	/* N.B. don't cache sv_pm, it can change if resized vis XmUpdate */

	/* busy */
	watch_cursor(1);

	/* print "sky" as paper */
	XPSPaperColor (sky_p);

	/* use current time if none provided */
	if (!np)
	    np = mm_get_now();

	/* put up the timestamp */
	timestamp (np, dt_w);

	/* rebuild a clean sky */
	if (si_ison()) {
	    Pixmap fpm = si_getPixmap();
	    FImage *fip = si_getFImage();
	    double ra, dec;
	    if (!fpm) {
		printf ("sv_all Bug! FITS pixmap disappeared!\n");
		abort();
	    }
	    if (!fip) {
		printf ("sv_all Bug! FITS FImage disappeared!\n");
		abort();
	    }
	    XCopyArea (dsp, fpm, sv_pm, sv_gc, 0, 0, sv_w, sv_h, 0, 0);
	    if (XPSDrawing())
		si_ps ();
	    svtb_imMode (1);
	    XtManageChild (fovimg_w);
	    if (xy2RADec (fip, 0.0, 0.0, &ra, &dec) < 0) {
		/* no WCS coords, so can't draw anything else */
		sv_copy_sky();
		watch_cursor(0);
		return;
	    }
	} else if (!cyl_proj && sv_dfov > PI) {
	    /* full horizon will show */
	    int w = (int)(PI*sv_h/sv_vfov);
	    int x = (sv_w - w)/2;
	    int y = (sv_h - w)/2;

	    XSetForeground (dsp, sv_gc, sky_p);
	    XPSFillRectangle (dsp, sv_pm, sv_gc, 0, 0, sv_w, sv_h);
	    XSetForeground (dsp, sv_gc, bg_p);
	    XPSDrawArc (dsp, sv_pm, sv_gc, x, y, w, w, 0, 360*64);

	    /* N.B. we assume no FITS file could ever be displayed this large */
	    svtb_imMode (0);
	    XtUnmanageChild (fovimg_w);
	} else {
	    /* solid sky bkq, just border on paper */
	    XSetForeground (dsp, sv_gc, sky_p);
	    XPSFillRectangle (dsp, sv_pm, sv_gc, 0, 0, sv_w, sv_h);
	    XSetForeground (dsp, sv_gc, bg_p);
	    XPSDrawRectangle (dsp, sv_pm, sv_gc, 0, 0, sv_w-1, sv_h-1);
	    svtb_imMode (0);
	    XtUnmanageChild (fovimg_w);
	}

	/* draw eyepieces next so everything is on top of them */
	if (want_eyep)
	    draw_eyep (dsp, sv_pm, sv_gc);

	/* draw the coord grid if desired */
	if (want_grid) {
	    XSetForeground (dsp, sv_gc, grid_p);
	    draw_grid(dsp, sv_pm, sv_gc);
	}

	/* draw the equator if desired */
	if (want_eq) {
	    XSetForeground (dsp, sv_gc, eq_p);
	    draw_equator (np, dsp, sv_pm, sv_gc);
	}

	/* draw the galactic isophote, poles and eq if desired */
	if (want_ga) {
	    XSetForeground (dsp, sv_gc, eq_p);
	    draw_galactic (np, dsp, sv_pm, sv_gc);
	    XSetForeground (dsp, sv_gc, mw_p);
	    draw_milkyway (np, dsp, sv_pm, sv_gc);
	}

	/* draw the ecliptic */
	if (want_ec) {
	    XSetForeground (dsp, sv_gc, eq_p);
	    draw_ecliptic (np, dsp, sv_pm, sv_gc);
	}

	/* draw constellation boundaries if desired */
	if (want_conb)
	    draw_cnsbounds (np, dsp, sv_pm);

	/* draw constellation figures and/or names if desired */
	if (want_conf || want_conn || want_cona)
	    draw_cns (np, dsp, sv_pm);

	/* go through the trailobj list and display that stuff too. */
	tobj_display_all();

	/* draw umbra (late, so shows over moon) */
	if (want_ec) {
	    XSetForeground (dsp, sv_gc, eq_p);
	    draw_umbra (np, dsp, sv_pm, sv_gc);
	}

	/* draw horizon map if desired, late so skyline can do clipping */
	if (want_hznmap)
	    draw_hznmap (np, dsp, sv_pm, sv_gc);

	/* draw all the objects -- they do their own clipping so
	 * this way their names are not clipped
	 */
	draw_allobjs (dsp, sv_pm);

	/* draw the magnitude scale last so it always covers everything else */
	if (want_magscale)
	    sv_magscale (dsp, sv_pm);

	/* user annotation */
	ano_draw (svda_w, sv_pm, sv_ano, aa_mode);

	/* compass rose */
	if (want_compass)
	    draw_compass (dsp, sv_pm);

	/* and we're done */
	sv_copy_sky();

	watch_cursor(0);

}

/* convert world to/from X coords depending on w2x.
 * if arg then a/b are alt/az else dec/ra
 * return whether visible.
 */
static int
sv_ano (double *ap, double *bp, int *xp, int *yp, int w2x, int arg)
{
	double altdec, azra;
	int v;

	if (w2x) {
	    if (arg != aa_mode)
		sv_other (*ap, *bp, arg, &altdec, &azra);
	    else {
		altdec = *ap;
		azra = *bp;
	    }
	    v = sv_loc (altdec, azra, xp, yp);
	} else {
	    v = sv_unloc (*xp, *yp, &altdec, &azra);
	    if (arg != aa_mode)
		sv_other (altdec, azra, arg, ap, bp);
	    else {
		*ap = altdec;
		*bp = azra;
	    }
	}

	return (v);
}

/* compute the angle ccw from 3 o'clock at which we should draw the galaxy
 * ellipse
 */
static double
x_angle (double ra, double dec, double pa)
{
	Now *np = mm_get_now();
	double na;		/* angle from 3 o'clock to N */
	double a;		/* ellipse axis: angle CCW from 3 oclk*/
	int x0, y0;		/* location of object */
	int xn, yn;		/* location a little N of object */
	int xe, ye;		/* location a little E of object */
	double step;		/* "a little" */
	Obj o;			/* test object */
	double altdec, azra;	/* test location */
	int eccw;		/* 1 if E shows CCW of N, else 0 */

	/* move by about 50 pixels each way (or 1 degree if new) */
	step = sv_h>0 ? sv_vfov/sv_h*50 : degrad(1);

	/* location of object */
	memset (&o, 0, sizeof(o));
	o.o_type = FIXED;
	o.f_RA = ra;
	o.f_dec = dec;
	o.f_epoch = (float)epoch;
	(void) obj_cir (np, &o);
	altdec = aa_mode ? o.s_alt : o.s_dec;
	azra   = aa_mode ? o.s_az  : o.s_ra;
	(void) sv_loc (altdec, azra, &x0, &y0);

	/* location due north */
	o.f_RA = ra;
	o.f_dec = (float)(dec + step);
	if (o.f_dec > PI/2) {
	    o.f_RA += (float)PI;
	    o.f_dec = (float)PI - o.f_dec;
	}
	(void) obj_cir (np, &o);
	altdec = aa_mode ? o.s_alt : o.s_dec;
	azra   = aa_mode ? o.s_az  : o.s_ra;
	(void) sv_loc (altdec, azra, &xn, &yn);

	/* location due east */
	o.f_RA = (float)(ra + step/cos(dec));
	o.f_dec = dec;
	(void) obj_cir (np, &o);
	altdec = aa_mode ? o.s_alt : o.s_dec;
	azra   = aa_mode ? o.s_az  : o.s_ra;
	(void) sv_loc (altdec, azra, &xe, &ye);

	/* see which way is east from north -- use cross product */
	eccw = (xn-x0)*(y0-ye)-(y0-yn)*(xe-x0) > 0 ? 1 : 0;

	/* find angle ccw from 3-oclock */
	if (xn == x0)
	    na = yn > y0 ? -PI/2 : PI/2;
	else
	    na = atan2 ((double)(y0-yn), (double)(xn-x0));
	a = eccw ? na+pa : na-pa;
	range (&a, 2*PI);

	return (a);
}

/* draw the given object so it has a nominal diameter of diam pixels.
 * we maintain a static cache of common X drawing objects for efficiency.
 * (mallocing seemed to keep the memory arena too fragmented).
 * to force a flush of this cache, call with op == NULL.
 * N.B. X's DrawRect function is actually w+1 wide and h+1 high.
 */
void
sv_draw_obj (Display *dsp, Drawable win, GC gc, Obj *op, int x, int y,
int diam, int dotsonly)
{
#define	MINSYMDIA	4	/* never use fancy symbol at diam this small */
#define	CACHE_SZ	75	/* size of points/arcs/etc cache */
#define	CACHE_PAD	10	/* most we ever need in one call */
#define	CACHE_HWM	(CACHE_SZ - CACHE_PAD)	/* hi water mark */
	static XPoint     xpoints[CACHE_SZ],    *xp  = xpoints;
	static XArc       xdrawarcs[CACHE_SZ],  *xda = xdrawarcs;
	static XArc       xfillarcs[CACHE_SZ],  *xfa = xfillarcs;
	static XArc       xstars[CACHE_SZ],     *xst = xstars;
	static XSegment   xsegments[CACHE_SZ],  *xs  = xsegments;
	static XRectangle xdrawrects[CACHE_SZ], *xdr = xdrawrects;
	static XRectangle xfillrects[CACHE_SZ], *xfr = xfillrects;
	static GC cache_gc;
	int force;
	int n;

	/* might be called to draw on other screens before us */
	if (!svshell_w)
	    sv_create_svshell();

	/* for sure if no op or different gc */
	force = !op || gc != cache_gc;

	if ((n = xp - xpoints) >= CACHE_HWM || (force && n > 0)) {
	    XPSDrawPoints (dsp, win, cache_gc, xpoints, n, CoordModeOrigin);
	    xp = xpoints;
	}
	if ((n = xda - xdrawarcs) >= CACHE_HWM || (force && n > 0)) {
	    XPSDrawArcs (dsp, win, cache_gc, xdrawarcs, n);
	    xda = xdrawarcs;
	}
	if ((n = xfa - xfillarcs) >= CACHE_HWM || (force && n > 0)) {
	    XPSFillArcs (dsp, win, cache_gc, xfillarcs, n);
	    xfa = xfillarcs;
	}
	if ((n = xs - xsegments) >= CACHE_HWM || (force && n > 0)) {
	    XPSDrawSegments (dsp, win, cache_gc, xsegments, n);
	    xs = xsegments;
	}
	if ((n = xdr - xdrawrects) >= CACHE_HWM || (force && n > 0)) {
	    XPSDrawRectangles (dsp, win, cache_gc, xdrawrects, n);
	    xdr = xdrawrects;
	}
	if ((n = xfr - xfillrects) >= CACHE_HWM || (force && n > 0)) {
	    XPSFillRectangles (dsp, win, cache_gc, xfillrects, n);
	    xfr = xfillrects;
	}
	if ((n = xst - xstars) >= CACHE_HWM || (force && n > 0)) {
	    int i;
	    for (i = 0; i < n; i++)
		XPSDrawStar (dsp, win, cache_gc, xstars[i].x, xstars[i].y,
							    xstars[i].width);
	    xst = xstars;
	}

	cache_gc = gc;

	if (!op)
	    return;	/* just flushing, thanks */

	/* if object is smallish then we don't use the fancy
	 * symbols, just use dots or filled circles.
	 */
	if (diam <= MINSYMDIA) {
	    if (diam <= 1) {
		xp->x = x;
		xp->y = y;
		xp++;
	    } else {
		xst->x = x - diam/2;
		xst->y = y - diam/2;
		xst->width = diam;
		xst++;
	    }
	    return;
	}

	switch (op->o_type) {
	case PLANET:
	    if (op->s_phase > 98) {
		/* just use a filled circle */
		xst->x = x - diam/2;
		xst->y = y - diam/2;
		xst->width = diam;
		xst++;
	    } else
		sv_draw_pl (dsp, win, gc, op, x, y, diam);
	    break;

	case FIXED:
	    switch (op->f_class) {
	    case 'G': /* galaxies */
	    case 'H':
		{
		    int xa;		/* X angle: + CCW from 3 oclock */
		    int hh;		/* half-height */
		    double a;		/* ellipse axis: angle CCW from 3 oclk*/

		    a = x_angle (op->s_ra, op->s_dec, get_pa(op));

		    /* convert to X Windows units */
		    xa = (int)floor(raddeg(a)*64 + 0.5);

		    /* draw ellipse */
		    n = diam/2;
		    hh = (int)floor(n*get_ratio(op) + 0.5);
		    XPSDrawEllipse (dsp, win, gc, x - n, y - hh,
						    xa, 2*n, 2*hh, 0, 360*64);
		}
		break;

	    case 'A': /* galaxy cluster */
		/* ellipse */
		n = diam/2;
		xda->x = x - n;
		xda->y = y - n/2;
		xda->width = diam;
		xda->height = n;
		xda->angle1 = 0;
		xda->angle2 = 360*64;
		xda++;
		break;

	    case 'C': /* globular clusters */
		/* plus in a circle, like Tirion */
		n = diam/2;
		xda->x = x - n;
		xda->y = y - n;
		xda->width = 2*n;
		xda->height = 2*n;
		xda->angle1 = 0;
		xda->angle2 = 360*64;
		xda++;
		xs->x1 = x-n; xs->y1 = y; xs->x2 = x+n; xs->y2 = y; xs++;
		xs->x1 = x; xs->y1 = y-n; xs->x2 = x; xs->y2 = y+n; xs++;
		break;

	    case 'U': /* glubular cluster within nebulosity */
		/* plus in a dotted circle */
		n = diam/2;
		xs->x1 = x-n; xs->y1 = y; xs->x2 = x+n; xs->y2 = y; xs++;
		xs->x1 = x; xs->y1 = y-n; xs->x2 = x; xs->y2 = y+n; xs++;

		/* FALLTHRU */

	    case 'O': /* open cluster */
		/* dotted circle */
		{
		    int nd = (int)(PI*diam/12 + 1); /* every 3rd dot */
		    double da = PI/2/nd;
		    int r  = diam/2;
		    int i;

		    /* reflect for quadrants 2-4 */
		    for (i = 0; i < nd; i++) {
			int dx = (int)floor(r*cos(i*da) + 0.5);
			int dy = (int)floor(r*sin(i*da) + 0.5);

			/* be mindful of filling the xpoints cache */
			if ((n = xp - xpoints) > CACHE_HWM-4) {
			    XPSDrawPoints (dsp, win, cache_gc, xpoints, n,
							    CoordModeOrigin);
			    xp = xpoints;
			}

			xp->x = x + dx; xp->y = y + dy; xp++;
			xp->x = x - dy; xp->y = y + dx; xp++;
			xp->x = x - dx; xp->y = y - dy; xp++;
			xp->x = x + dy; xp->y = y - dx; xp++;
		    }
		}
		break;

	    case 'P': /* planetary nebula */
		/* open square */
		n = diam/2;
		xdr->x = x-n;
		xdr->y = y-n;
		xdr->width = diam;
		xdr->height = diam;
		xdr++;
		break;

	    case 'R': /* supernova remnant */
		/* two concentric circles */
		n = diam/2;
		xda->x = x - n;
		xda->y = y - n;
		xda->width = 2*n+1;
		xda->height = 2*n+1;
		xda->angle1 = 0;
		xda->angle2 = 360*64;
		xda++;

		if (diam > 3) {
		    n /= 2;			/* well inside */
		    xda->x = x - n;
		    xda->y = y - n;
		    xda->width = 2*n+1;
		    xda->height = 2*n+1;
		    xda->angle1 = 0;
		    xda->angle2 = 360*64;
		    xda++;
		} else {
		    xp->x = x;
		    xp->y = y;
		    xp++;
		}

		break;

	    case 'Y': /* supernova */
		/* filled circle in diamond */
		n = diam/2;

		xs->x1 = x-n; xs->y1 = y; xs->x2 = x; xs->y2 = y-n; xs++;
		xs->x1 = x; xs->y1 = y-n; xs->x2 = x+n; xs->y2 = y; xs++;
		xs->x1 = x+n; xs->y1 = y; xs->x2 = x; xs->y2 = y+n; xs++;
		xs->x1 = x; xs->y1 = y+n; xs->x2 = x-n; xs->y2 = y; xs++;

		if (diam > 3) {
		    n /= 2;			/* well inside */
		    xfa->x = x - n;
		    xfa->y = y - n;
		    xfa->width = 2*n+1;
		    xfa->height = 2*n+1;
		    xfa->angle1 = 0;
		    xfa->angle2 = 360*64;
		    xfa++;
		} else {
		    xp->x = x;
		    xp->y = y;
		    xp++;
		}

		break;

	    case 'S': /* stars */
		/* filled circle */
		n = diam/2;
		xst->x = x - n;
		xst->y = y - n;
		xst->width = diam;
		xst++;
		break;

	    case 'D': /* double stars */
		/* filled circle with one horizontal line through it */
		n = diam/2;
		xst->x = x - n;
		xst->y = y - n;
		xst->width = diam;
		xst++;

		if (!dotsonly) {
		    xs->x1 = x - n - diam/4;
		    xs->y1 = y;
		    xs->x2 = x - n + diam + diam/4;
		    xs->y2 = y;
		    xs++;
		}

		break;

	    case 'M': /* multiple stars */
		/* open circle with two horizontal lines through it */
		n = diam/2;
		xda->x = x - n;
		xda->y = y - n;
		xda->width = diam;
		xda->height = diam;
		xda->angle1 = 0;
		xda->angle2 = 360*64;
		xda++;

		if (!dotsonly) {
		    xs->x1 = x - n - diam/4;
		    xs->x2 = x - n + diam + diam/4;
		    xs->y2 = xs->y1 = y - n + n/2 + 1;
		    xs++;
		    xs->x1 = x - n - diam/4;
		    xs->x2 = x - n + diam + diam/4;
		    xs->y2 = xs->y1 = y - n + diam - 1 - n/2;
		    xs++;
		}

		break;
	    case 'V': /* variable star */
		/* central dot with concentric circle */
		n = diam/2;
		if (dotsonly) {
		    xst->x = x - n;
		    xst->y = y - n;
		    xst->width = diam;
		    xst++;
		} else {
		    xda->x = x - n;
		    xda->y = y - n;
		    xda->width = 2*n+1;
		    xda->height = 2*n+1;
		    xda->angle1 = 0;
		    xda->angle2 = 360*64;
		    xda++;

		    if (diam > 3) {
			n /= 2;
			xfa->x = x - n+1;
			xfa->y = y - n+1;
			xfa->width = 2*n;
			xfa->height = 2*n;
			xfa->angle1 = 0;
			xfa->angle2 = 360*64;
			xfa++;
		    }
		}

		break;

	    case 'F': /* diffuse nebula */
		/* open diamond with top and bottom bar */
		n = diam/2;
		xs->x1 = x-n; xs->y1 = y; xs->x2 = x; xs->y2 = y-n; xs++;
		xs->x1 = x; xs->y1 = y-n; xs->x2 = x+n; xs->y2 = y; xs++;
		xs->x1 = x+n; xs->y1 = y; xs->x2 = x; xs->y2 = y+n; xs++;
		xs->x1 = x; xs->y1 = y+n; xs->x2 = x-n; xs->y2 = y; xs++;
		xs->x1 = x-n; xs->y1 = y-n; xs->x2 = x+n; xs->y2 = y-n; xs++;
		xs->x1 = x-n; xs->y1 = y+n; xs->x2 = x+n; xs->y2 = y+n; xs++;
		break;

	    case 'K': /* dark nebulae */
		/* open diamond */
		n = diam/2;
		xs->x1 = x-n; xs->y1 = y; xs->x2 = x; xs->y2 = y-n; xs++;
		xs->x1 = x; xs->y1 = y-n; xs->x2 = x+n; xs->y2 = y; xs++;
		xs->x1 = x+n; xs->y1 = y; xs->x2 = x; xs->y2 = y+n; xs++;
		xs->x1 = x; xs->y1 = y+n; xs->x2 = x-n; xs->y2 = y; xs++;
		break;

	    case 'N': /* bright nebulae */
		/* open hexagon */
		n = diam/2;
		xs->x1 = x-n; xs->y1 = y; xs->x2 = x-n/2; xs->y2 = y-n; xs++;
		xs->x1 = x-n/2; xs->y1 = xs->y2 = y-n; xs->x2 = x+n/2;  xs++;
		xs->x1 = x+n/2; xs->y1 = y-n; xs->x2 = x+n; xs->y2 = y; xs++;
		xs->x1 = x+n; xs->y1 = y; xs->x2 = x+n/2; xs->y2 = y+n; xs++;
		xs->x1 = x+n/2; xs->y1 = xs->y2 = y+n; xs->x2 = x-n/2;  xs++;
		xs->x1 = x-n/2; xs->y1 = y+n; xs->x2 = x-n; xs->y2 = y; xs++;
		break;

	    case 'Q': /* Quasar */
		/* plus sign */
		n = diam/2;
		xs->x1 = x-n; xs->y1 = y; xs->x2 = x+n; xs->y2 = y; xs++;
		xs->x1 = x; xs->y1 = y-n; xs->x2 = x; xs->y2 = y+n; xs++;
		break;

	    case 'L':	/* Pulsar */
		/* opposing jets */
		n = diam/2;
		xfa->x = x - n;
		xfa->y = y - n;
		xfa->width = diam;
		xfa->height = diam;
		xfa->angle1 = 160*64;
		xfa->angle2 = 40*64;
		xfa++;
		xfa->x = x - n;
		xfa->y = y - n;
		xfa->width = diam;
		xfa->height = diam;
		xfa->angle1 = 340*64;
		xfa->angle2 = 40*64;
		xfa++;
		break;

	    case 'J':	/* Radio source */
		/* half-circle pointing up and to the left */
		n = diam/2;
		xda->x = x - n;
		xda->y = y - n;
		xda->width = diam;
		xda->height = diam;
		xda->angle1 = 225*64;
		xda->angle2 = 180*64;
		xda++;
		break;

	    case 'T':	/* stellar object */
		if (dotsonly) {
		    /* filled circle */
		    n = diam/2;
		    xst->x = x - n;
		    xst->y = y - n;
		    xst->width = diam;
		    xst++;
		    break;
		}
		/* FALLTHRU */

	    default:	/* unknown type */
		/* an X */
		n = diam/3;
		xs->x1= x-n; xs->y1= y-n; xs->x2= x+n; xs->y2= y+n; xs++;
		xs->x1= x-n; xs->y1= y+n; xs->x2= x+n; xs->y2= y-n; xs++;
		break;
	    }
	    break;

	case HYPERBOLIC:
	case PARABOLIC:
	    /* hopefully has a tail -- draw a filled circle with one */
	    xfa += sv_drawComet (xfa, diam, x, y);
	    break;

	case ELLIPTICAL: /* asteroids --- and periodic comets */
	    if (op->o_name[0] == 'C' && op->o_name[1] == '/') {
		/* name looks like a comet - draw tail */
		xfa += sv_drawComet (xfa, diam, x, y);
	    } else {
		/* asteroid is filled square */
		n = diam/2;
		xfr->x = x - n;
		xfr->y = y - n;
		xfr->width = diam;
		xfr->height = diam;
		xfr++;
	    }
	    break;
	case EARTHSAT:
	    /* bird */
	    n = diam/2;
	    xda->x = x - diam/4 - n;
	    xda->y = y - diam/15;
	    xda->width = diam;
	    xda->height = diam;
	    xda->angle1 = 60*64;
	    xda->angle2 = 60*64+1;	/* insure completion */
	    xda++;
	    xda->x = x - diam/4;
	    xda->y = y - diam/15;
	    xda->width = diam;
	    xda->height = diam;
	    xda->angle1 = 60*64;
	    xda->angle2 = 60*64;
	    xda++;

	    break;

	case BINARYSTAR:
	    /* filled circle, or two angled properly if not dotsonly */
	    if (dotsonly) {
		n = diam/2;
		xst->x = x - n;
		xst->y = y - n;
		xst->width = diam;
		xst++;
	    } else
		sv_drawBin (op, &xfa, &xs, diam, x, y);
	    break;

	default:
	    printf ("Bad type to sv_draw_obj: %d\n", op->o_type);
	    abort();
	}
}

/* wrapper for calling sv_draw_obj from outside skyviewmenu.c*/
void
sv_draw_obj_x (Display *dsp, Drawable win, GC gc, Obj *op, int x, int y,
int diam, int dotsonly, int flip_tb_x, int flip_lr_x, int aa_mode_x, int cyl_proj_x,
double altdec_x, double azra_x, double vfov_x, double hfov_x, int w_x, int h_x)
{
	/* might be called to draw on other screens before us */
	if (!svshell_w)
	    sv_create_svshell();

	int flip_tb_i   = flip_tb;
	int flip_lr_i   = flip_lr;
	int aa_mode_i   = aa_mode;
	int cyl_proj_i  = cyl_proj;

	double altdec_i = sv_altdec;
	double azra_i   = sv_azra;
	double vfov_i   = sv_vfov;
	double hfov_i   = sv_hfov;
	int w_i         = sv_w;
	int h_i         = sv_h;

	flip_tb   = flip_tb_x;
	flip_lr   = flip_lr_x;
	aa_mode   = aa_mode_x;
	cyl_proj  = cyl_proj_x;

	sv_altdec = altdec_x;
	sv_azra   = azra_x;
	sv_vfov   = vfov_x;
	sv_hfov   = hfov_x;
	sv_w      = w_x;
	sv_h      = h_x;
	
	sv_draw_obj (dsp, win, gc, op, x, y, diam, dotsonly);

	flip_tb   = flip_tb_i;
	flip_lr   = flip_lr_i;
	aa_mode   = aa_mode_i;
	cyl_proj  = cyl_proj_i;

	sv_altdec = altdec_i;
	sv_azra   = azra_i;
	sv_vfov   = vfov_i;
	sv_hfov   = hfov_i;
	sv_w      = w_i;
	sv_h      = h_i;
}

static void
sv_drawBin (Obj *op, XArc *xfap[], XSegment *xsp[], int diam, int x, int y)
{
	XArc *xfa = *xfap;
	XSegment *xs = *xsp;
	int n = diam/2;
	double xa;			/* X angle: rads CCW from 3 oclock */
	int dx, dy;

	/* find X Windows angle from primary to secondary */
	if (op->b_nbp == 0) {
	    Now *np = mm_get_now();
	    op->b_2compute = 1;
	    obj_cir (np, op);
	    xa = x_angle (op->s_ra, op->s_dec, op->b_bo.bo_pa);
	} else {
	    xa = 0;
	}

	/* offset in direction of sec */
	dx = n * cos(xa);
	dy = n * sin(xa);

	/* primary full size opposite direction */
	xfa->x = x - dx - n;
	xfa->y = y + dy - n;
	xfa->width = diam;
	xfa->height = diam;
	xfa->angle1 = 0;
	xfa->angle2 = 360*64;
	xfa++;

	/* secondary half size correct direction */
	xfa->x = x + dx - n/2;
	xfa->y = y - dy - n/2;
	xfa->width = n;
	xfa->height = n;
	xfa->angle1 = 0;
	xfa->angle2 = 360*64;
	xfa++;

	/* connector */
	xs->x1 = x - 3*dx/2;
	xs->y1 = y + 3*dy/2;
	xs->x2 = x + 3*dx/2;
	xs->y2 = y - 3*dy/2;
	xs++;

	*xfap = xfa;
	*xsp = xs;
}

static int
sv_drawComet (XArc xfa[], int diam, int x, int y)
{
	int n = diam/2;

	xfa->x = x;
	xfa->y = y;
	xfa->width = n;
	xfa->height = n;
	xfa->angle1 = 0;
	xfa->angle2 = 360*64;
	xfa++;

	xfa->x = x - n;
	xfa->y = y - n;
	xfa->width = diam;
	xfa->height = diam;
	xfa->angle1 = 120*64;
	xfa->angle2 = 30*64;
	xfa++;

	return (2);
}

/* draw the planet at [x,y] with diameter d showing proper phase */
static void
sv_draw_pl (dsp, win, gc, op, x, y, d)
Display *dsp;
Drawable win;
GC gc;
Obj *op;
int x, y;
int d;
{
	double f = op->s_phase/100.0;	/* fraction lit */
	int x0 = x-d/2, y0 = y-d/2;	/* corner of moon bounding box */
	int sx, sy;			/* screen x,y of sun */
	int r;				/* rotation to face sun, X units */
	int df;				/* diam to terminator */
	Obj *sop;			/* Sun info */

	/* find direction towards sun from input x,y */
	sop = db_basic (SUN);
	db_update (sop);
	if (aa_mode)
	    sv_loc (sop->s_alt, sop->s_az, &sx, &sy);
	else
	    sv_loc (sop->s_dec, sop->s_ra, &sx, &sy);
	r = (int)floor(64*raddeg(atan2((double)(y-sy),(double)(sx-x))) + .5);

	/* all sky, then half towards sun lit, then ellipse for phase */
	XSetForeground (dsp, sv_gc, sky_p);
	XPSFillArc (dsp, win, sv_gc, x0, y0, d, d, 0, 360*64);
	if (f > .5) {
	    /* gibbous: more than half is lit */
	    df = (int)floor(d*(2*f - 1) + .5);
	    XPSFillEllipse (dsp, win, gc, x0, y0, r, d, d, -90*64, 180*64);
	    XPSFillEllipse (dsp, win, gc, x-df/2, y0, r, df, d, 0, 64*360);
	} else if (f*d > 3) {
	    /* crescent: more than half is sky */
	    df = (int)floor(d*(1 - 2*f) + .5);
	    XPSFillEllipse (dsp, win, gc, x0, y0, r, d, d, -90*64, 180*64);
	    XPSFillEllipse (dsp, win, sv_gc, x-df/2, y0, r, df, d, 0, 64*360);
	} else {
            /* almost new, idealized crescent with variable angle, to a point */
            df = (int)((1.0-f*d/6)*180*64);
	    if (df > 120*64)
		df = 181*64;
            XPSDrawArc (dsp, win, gc, x0, y0, d, d, r-df, 2*df);
	}
}

/* called by outsiders to get the current pointing info */
void
sv_getcenter (int *aamodep, double *fovp, double *altp, double *azp,
double *rap, double *decp)
{
	sv_fullwhere (mm_get_now(), sv_altdec, sv_azra, aa_mode, altp, azp,
								rap, decp);

	*aamodep = aa_mode;
	*fovp = sv_vfov;
}

/* return to the caller our current field star list.
 * if !want_fs we return NULL even if there is a pending list.
 */
void
sv_getfldstars (ObjF **fspp, int *nfsp)
{
	if (want_fs) {
	    *fspp = fldstars;
	    *nfsp = nfldstars;
	} else {
	    *fspp = NULL;
	    *nfsp = 0;
	}
}

/* called by trails.c to create a new trail for pu.op.
 * client is the Obj* to which a trail will be added.
 * return 0 if ok else -1.
 */
static int
sv_mktrail (ts, statep, client)
TrTS ts[];
TrState *statep;
XtPointer client;
{
	Obj *trop = (Obj *)client;
	TrailObj *top;
	Now *np, now;
	int i;

	/* set trop to object getting new trail.
	 * started out as popup object but if that has changed we search the
	 * db to make sure it is still valid. if not, well it's gone.
	 */
	if (trop != pu.op) {
	    DBScan dbs;
	    Obj *op;

	    for (db_scaninit(&dbs,ALLM, want_fs ? fldstars : NULL, nfldstars);
					    (op = db_scan (&dbs)) != NULL; )
		if (trop == op && !strcmp (trop->o_name, op->o_name))
		    break;
	    if (!op) {
		xe_msg (1, "Object has disappeared -- no trail created");
		return (-1);
	    }
	}

	/* turn on watch cursor for really slow systems */
	watch_cursor (1);

	/* discard any preexisting trails for this object. */
	tobj_rmobj (trop);	/* silently does nothing if op not on list */

	/* allocate a new trailobj for this trail */
	top = tobj_addobj (trop, statep->nticks);

	/* work with a copy of Now because we modify n_mjd */
	np = mm_get_now();
	now = *np;

	/* make each entry */
	for (i = 0; i < statep->nticks; i++) {
	    TSky *tsp = &top->sky[i];
	    TrTS *tp = &ts[i];

	    /* save the timestamp info time */
	    tsp->trts = *tp;

	    /* compute circumstances */
	    tsp->o = *trop;
	    now.n_mjd = tp->t;
	    (void) obj_cir (&now, &tsp->o);
	}

	/* save the other setup details for this trail */
	top->trs = *statep;

	/* retain setup state as default for next time */
	trstate = *statep;

	/* update trail state resource */
	tr_setres (svtrres, &trstate);

	/* redraw everything with the new trail, if we are up */
	if (isUp(svshell_w))
	    sv_all (mm_get_now());

	watch_cursor (0);

	return (0);
}

/* helper function to fill in the Help menu*/
static void
sv_fillin_help (mb_w)
Widget mb_w;
{
	typedef struct {
	    char *label;	/* what goes on the help label */
	    char *key;		/* string to call hlp_dialog() */
	} HelpOn;
	static HelpOn helpon[] = {
	    {"Intro...",	"SkyView"},
	    {"on Control...",	"SkyView_control"},
	    {"on Images...",	"SkyView_images"},
	    {"on Favorites...",	"SkyView_favorites"},
	    {"on Telescope...",	"SkyView_telescope"},
	    {"on History...",	"SkyView_history"},
	    {"on Toolbars...",	"SkyView_toolbars"},
	    {"on Mouse...",	"SkyView_mouse"},
	    {"on Trails...",	"SkyView_trails"},
	    {"on Scales...",	"SkyView_scales"},
	};
	Widget pd_w, cb_w;
	Widget w;
	XmString str;
	Arg args[20];
	int n;
	int i;

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "HPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    XtSetArg (args[n], XmNmnemonic, 'H'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Help", args, n);
	    XtManageChild (cb_w);
	    set_something (mb_w, XmNmenuHelpWidget, (XtArgVal)cb_w);

	    for (i = 0; i < XtNumber(helpon); i++) {
		HelpOn *hp = &helpon[i];

		str = XmStringCreate (hp->label, XmSTRING_DEFAULT_CHARSET);
		n = 0;
		XtSetArg (args[n], XmNlabelString, str); n++;
		XtSetArg (args[n], XmNmarginHeight, 0); n++;
		w = XmCreatePushButton (pd_w, "Help", args, n);
		XtAddCallback (w, XmNactivateCallback, sv_helpon_cb,
							(XtPointer)(hp->key));
		XtManageChild (w);
		XmStringFree(str);
	    }
}

/* helper function to fill in the menubar */
static void
sv_fillin_mb (mb_w)
Widget mb_w;
{
	Widget cb_w, pd_w;
	Widget w;
	Arg args[20];
	int n;

	/* control pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "CPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    XtSetArg (args[n], XmNmnemonic, 'C'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Control", args, n);
	    wtip (cb_w, "Access to many Sky View supporting features");
	    XtManageChild (cb_w);

	    /* make the PB which can bring up the Options dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "FPB", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_opdialog_cb, 0);
	    set_xmstring (w, XmNlabelString, "Options...");
	    wtip (w, "List of several viewing options");
	    XtManageChild (w);

	    /* make the pb that controls the filter menu */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "Filter", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_filter_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Filter...");
	    wtip (w, "Dialog to select type and brightness of objects");
	    XtManageChild (w);

	    /* create the print control */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "SVPrint", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_print_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Print...");
	    wtip (w, "Print the current Sky View map");
	    XtManageChild (w);

	    /* create the list control */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "List", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_list_cb, NULL);
	    set_xmstring (w, XmNlabelString, "List...");
	    wtip (w, "Create a file listing all objects in the current view");
	    XtManageChild (w);

	    /* make the PB which can bring up the horizon setup dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "Hzn", args, n);
	    XtAddCallback (w, XmNactivateCallback,(XtCallbackProc)hzn_manage,0);
	    set_xmstring (w, XmNlabelString, "Horizon...");
	    wtip (w, "Define local horizon shape");
	    XtManageChild (w);

	    /* make the PB which can bring up the field star setup dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "FS", args, n);
	    XtAddCallback (w, XmNactivateCallback, (XtCallbackProc)fs_manage,0);
	    set_xmstring (w, XmNlabelString, "Field Stars...");
	    wtip (w, "Setup for GSC and other big catalogs");
	    XtManageChild (w);

	    /* make the PB which can bring up the Favorites setup dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "Fav", args, n);
	    XtAddCallback (w, XmNactivateCallback,(XtCallbackProc)fav_manage,0);
	    set_xmstring (w, XmNlabelString, "Favorites...");
	    wtip (w, "Manage the list of Favorites");
	    XtManageChild (w);

	    /* make the PB which can bring up the Eyepiece dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "SVEPSz", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_eyep_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Eyepieces...");
	    wtip (w, "Setup Eyepiece parameters");
	    XtManageChild (w);

	    /* make the PB which can bring up the manual Coordinates dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "SVMC", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_mancoord_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Coordinates...");
	    wtip (w, "Handy coordinates converter");
	    XtManageChild (w);

	    /* make the PB which can bring up the user annotation dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "SVUA", args, n);
	    XtAddCallback (w, XmNactivateCallback, ano_cb, NULL);
	    set_xmstring (w, XmNlabelString, "User annotation...");
	    wtip (w, "Open window to create and manage your own annotation");
	    XtManageChild (w);

	    /* make a pb to add the current scene to a movie loop */
	    n = 0;
	    n += ml_addacc (args, n);
	    w = XmCreatePushButton (pd_w, "Mloop", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_addframe_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Add to movie...");
	    wtip (w, "Add the current Sky View frame to the movie loop");
	    XtManageChild (w);

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    /* create the tracking tb */

	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNset, False); n++;	/* insure not overrode*/
	    tracktb_w = XmCreateToggleButton (pd_w, "Tracking", args, n);
	    XtAddCallback (tracktb_w, XmNvalueChangedCallback, sv_track_cb, 0);
	    XtSetSensitive (tracktb_w, False);
	    wtip (tracktb_w,"When on, Sky View stays locked onto an object");
	    XtManageChild (tracktb_w);

	    n = 0;
	    w = XmCreateSeparator (pd_w, "Sep", args, n);
	    XtManageChild (w);

	    n = 0;
	    w = XmCreatePushButton (pd_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_close_cb, 0);
	    wtip (w, "Close this and all supporting dialogs");
	    XtManageChild (w);

	/* Images pulldown */

	n = 0;
	pd_w = XmCreatePulldownMenu (mb_w, "IPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    XtSetArg (args[n], XmNmnemonic, 'I'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Images", args, n);
	    wtip (cb_w, "Access to features for viewing and analysing images");
	    XtManageChild (cb_w);

	    /* make the pb which can bring up the FITS file dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "Fits", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_fits_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Load and save...");
	    wtip (w, "Open and Save FITS files and download DSS images");
	    XtManageChild (w);

	    /* make the pb which can bring up the image analysis dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "Image", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_image_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Analysis tools...");
	    wtip (w, "Image analysis functions");
	    XtManageChild (w);

	    /* make the pb which can bring up the image registration dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "Registration", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_registration_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Registration...");
	    wtip (w, "Align one or more images to a reference image.");
	    XtManageChild (w);

	    /* make the pb which can bring up the WCS solver dialog */
	    n = 0;
	    w = XmCreatePushButton (pd_w, "WCS", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_wcs_cb, NULL);
	    set_xmstring (w, XmNlabelString, "WCS solver...");
	    wtip (w, "Pattern match with field stars to find WCS solution");
	    XtManageChild (w);

	/* make the Favorites cascade button and pulldown */

	sv_create_favmenu (mb_w);

	/* make the telescope control pulldown */

	n = 0;
	XtSetArg (args[n], XmNisAligned, False); n++;
	telpd_w = XmCreatePulldownMenu (mb_w, "FPD", args, n);

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, telpd_w); n++;
	    XtSetArg (args[n], XmNmnemonic, 'T'); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Telescope", args, n);
	    wtip (cb_w, "Control connections to telescope control system");
	    XtManageChild (cb_w);

	    /* make the PB to bring up the config dialog */

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    w = XmCreatePushButton (telpd_w, "TCFG", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_tcp_cb, NULL);
	    set_xmstring (w, XmNlabelString, "Configure...");
	    wtip (w, "Open the telescope configuration window");
	    XtManageChild (w);

	    /* make the PB to bring up the INDI panel */

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    w = XmCreatePushButton (telpd_w, "TIN", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_indi_cb, NULL);
	    set_xmstring (w, XmNlabelString, "INDI panel...");
	    wtip (w, "Open the INDI control panel");
	    XtManageChild (w);

	    /* TB whether to keep tel visible */

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNvisibleWhenOff, (XtArgVal)True); n++;
	    keeptelvis_w = XmCreateToggleButton (telpd_w, "KTV", args, n);
	    set_xmstring (keeptelvis_w, XmNlabelString, "Keep visible");
	    wtip (w, "Whether to move view to keep telescope marker visible");
	    /* N.B. managed elsewhere! */

	    /* history */

	    n = 0;
	    w = XmCreateSeparator (telpd_w, "TGS", args, n);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	    w = XmCreateLabel (telpd_w, "TH", args, n);
	    set_xmstring (w, XmNlabelString, "GoTo History");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	    w = XmCreatePushButton (telpd_w, "Erase", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_etelpd_cb, NULL);
	    wtip (w, "Erase all GoTo History entries");
	    XtManageChild (w);

	/* history */
	svh_create (mb_w);

	/* help */
	sv_fillin_help (mb_w);
}

/* create the main skyview shell */
static void
sv_create_svshell()
{
	Widget altdecsc_w, fovsc_w;
	Widget mb_w;
	Widget fr_w;
	Widget svform_w;
	XmString xms;
	Arg args[20];
	EventMask mask;
	int n;

	/* connect shortcuts */
	XtAppAddActions (xe_app, scuts, XtNumber(scuts));

	/* create shell and form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Sky View"); n++;
	XtSetArg (args[n], XmNiconName, "Sky View"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	svshell_w = XtCreatePopupShell ("SkyView", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (svshell_w);
	set_something (svshell_w, XmNcolormap, (XtArgVal)xe_cm);
        XtAddCallback (svshell_w, XmNpopdownCallback, sv_popdown_cb, 0);
	sr_reg (svshell_w, "XEphem*SkyView.width", skycategory, 0);
	sr_reg (svshell_w, "XEphem*SkyView.height", skycategory, 0);
	sr_reg (svshell_w, "XEphem*SkyView.x", skycategory, 0);
	sr_reg (svshell_w, "XEphem*SkyView.y", skycategory, 0);
	sr_reg (NULL, sv_viewupres(), skycategory, 0);

	n = 0;
	svform_w = XmCreateForm (svshell_w, "SVForm", args, n);
	XtAddCallback (svform_w, XmNhelpCallback, sv_help_cb, 0);
	XtManageChild (svform_w);

	/* make a menu bar across the top -- fill in later */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (svform_w, "MB", args, n);
	XtManageChild (mb_w);

	/* top toolbar below */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 0); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 0); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	ttbar_w = XmCreateRowColumn (svform_w, "TToolbar", args, n);
	XtManageChild (ttbar_w);

	/* make a timestamp label across the bottom */

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 25); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 75); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	dt_w = XmCreateLabel (svform_w, "TimeStamp", args, n);
	wtip (dt_w, "Date and Time for which map is computed");
	XtManageChild(dt_w);

	/* altdec short cuts in lower right corner.
	 * create with XmString because of !XmNrecomputeSize
	 */

	xms = XmStringCreateSimple ("45:00");
	n = 0;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 1); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 1); n++;
	XtSetArg (args[n], XmNmarginWidth, 1); n++;
	XtSetArg (args[n], XmNmarginHeight, 1); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNlabelString, xms); n++;
	altdecsc_w = XmCreatePushButton (svform_w, "ADSC", args, n);
	XmStringFree (xms);
	XtAddCallback (altdecsc_w, XmNactivateCallback, sv_altdecsc_cb, 
							    (XtPointer)45);
	wtip (altdecsc_w, "Press to slide Alt/Dec scale to 45:00");
	XtManageChild (altdecsc_w);

	xms = XmStringCreateSimple ("0:00");
	n = 0;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, altdecsc_w); n++;
	XtSetArg (args[n], XmNrightOffset, 1); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 1); n++;
	XtSetArg (args[n], XmNmarginWidth, 1); n++;
	XtSetArg (args[n], XmNmarginHeight, 1); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNlabelString, xms); n++;
	altdecsc_w = XmCreatePushButton (svform_w, "ADSC", args, n);
	XmStringFree (xms);
	XtAddCallback (altdecsc_w, XmNactivateCallback, sv_altdecsc_cb, 
							    (XtPointer)0);
	wtip (altdecsc_w, "Press to slide Alt/Dec scale to 0:00");
	XtManageChild (altdecsc_w);

	xms = XmStringCreateSimple ("-45:00");
	n = 0;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, altdecsc_w); n++;
	XtSetArg (args[n], XmNrightOffset, 1); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 1); n++;
	XtSetArg (args[n], XmNmarginWidth, 1); n++;
	XtSetArg (args[n], XmNmarginHeight, 1); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNlabelString, xms); n++;
	altdecsc_w = XmCreatePushButton (svform_w, "ADSC", args, n);
	XmStringFree (xms);
	XtAddCallback (altdecsc_w, XmNactivateCallback, sv_altdecsc_cb, 
							    (XtPointer)-45);
	wtip (altdecsc_w, "Press to slide Alt/Dec scale to 45:00");
	XtManageChild (altdecsc_w);

	/* FOV short cuts in lower left corner.
	 * create with XmString because of !XmNrecomputeSize
	 */

	xms = XmStringCreateSimple ("90H");
	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 1); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 1); n++;
	XtSetArg (args[n], XmNmarginWidth, 1); n++;
	XtSetArg (args[n], XmNmarginHeight, 1); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNlabelString, xms); n++;
	fovsc_w = XmCreatePushButton (svform_w, "FOVSC", args, n);
	XtAddCallback (fovsc_w, XmNactivateCallback, sv_fovsc_cb,(XtPointer)0);
	XmStringFree (xms);
	wtip (fovsc_w, "Set FOV scale to 90:00H");
	XtManageChild (fovsc_w);

	xms = XmStringCreateSimple ("1:1");
	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, fovsc_w); n++;
	XtSetArg (args[n], XmNleftOffset, 5); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 1); n++;
	XtSetArg (args[n], XmNmarginWidth, 1); n++;
	XtSetArg (args[n], XmNmarginHeight, 1); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNlabelString, xms); n++;
	fovsc_w = XmCreatePushButton (svform_w, "FOVSC", args, n);
	XtAddCallback (fovsc_w, XmNactivateCallback, sv_fovsc_cb,(XtPointer)1);
	XmStringFree (xms);
	wtip (fovsc_w, "Resize width to create 1:1 window size");
	XtManageChild (fovsc_w);

	xms = XmStringCreateSimple ("2:1");
	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, fovsc_w); n++;
	XtSetArg (args[n], XmNleftOffset, 1); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 1); n++;
	XtSetArg (args[n], XmNmarginWidth, 1); n++;
	XtSetArg (args[n], XmNmarginHeight, 1); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNlabelString, xms); n++;
	fovsc_w = XmCreatePushButton (svform_w, "FOVSC", args, n);
	XtAddCallback (fovsc_w, XmNactivateCallback, sv_fovsc_cb,(XtPointer)2);
	XmStringFree (xms);
	wtip (fovsc_w, "Resize width to create 2:1 window size");
	XtManageChild (fovsc_w);

	xms = XmStringCreateSimple ("Image");
	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, fovsc_w); n++;
	XtSetArg (args[n], XmNleftOffset, 1); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomOffset, 1); n++;
	XtSetArg (args[n], XmNmarginWidth, 1); n++;
	XtSetArg (args[n], XmNmarginHeight, 1); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNlabelString, xms); n++;
	fovimg_w = XmCreatePushButton (svform_w, "FOVSC", args, n);
	XtAddCallback (fovimg_w, XmNactivateCallback, sv_fovsc_cb,(XtPointer)3);
	XmStringFree (xms);
	wtip (fovimg_w, "Resize to match image aspect ratio");
	/* manage when loading image XtManageChild (fovimg_w); */

	/* make the three scale value displays above the shortcuts */

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 33); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, altdecsc_w); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	fovl_w = XmCreateLabel (svform_w, "FOVL", args, n);
	wtip (fovl_w, "Field of View, Deg:Min");
	XtManageChild (fovl_w);

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 34); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 66); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, altdecsc_w); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	azral_w = XmCreateLabel (svform_w, "AzRAL", args, n);
	wtip (azral_w, "Azimuth in Deg:Min or RA in Hours:Min");
	XtManageChild (azral_w);

	n = 0;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 67); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, altdecsc_w); n++;
	XtSetArg (args[n], XmNrecomputeSize, False); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
	altdecl_w = XmCreateLabel (svform_w, "AltDecL", args, n);
	wtip (altdecl_w, "Altitude or Declination, Deg:Min");
	XtManageChild (altdecl_w);

	/* make the bottom scale above the row of scale values */

	n = 0;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, azral_w); n++; /* typical */
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNminimum, 0); n++;
	XtSetArg (args[n], XmNmaximum, 4320-1); n++;	/* 5' steps */
	XtSetArg (args[n], XmNshowValue, False); n++;
	azra_w = XmCreateScale (svform_w, "AzRAScale", args, n);
	wtip (azra_w, "Azimuth or RA scale");
	XtAddCallback (azra_w, XmNdragCallback, sv_scale_cb, (XtPointer)AZRA_S);
	XtAddCallback (azra_w, XmNvalueChangedCallback, sv_scale_cb,
							    (XtPointer)AZRA_S);
	XtManageChild (azra_w);
	sr_reg (azra_w, NULL, skycategory, 0);

	/* make the left toolbar */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ttbar_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, azra_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 0); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	ltbar_w = XmCreateRowColumn (svform_w, "LToolbar", args, n);
	XtManageChild (ltbar_w);

	/* make the right toolbar */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ttbar_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, azra_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightOffset, 0); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	rtbar_w = XmCreateRowColumn (svform_w, "RToolbar", args, n);
	XtManageChild (rtbar_w);

	/* make the left (fov) scale */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ttbar_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, ltbar_w); n++;
	XtSetArg (args[n], XmNleftOffset, 0); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, azra_w); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNminimum, 1); n++;
	XtSetArg (args[n], XmNmaximum, 180*(60/FOV_STEP)); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_BOTTOM); n++;
	XtSetArg (args[n], XmNshowValue, False); n++;
	fov_w = XmCreateScale (svform_w, "FOVScale", args, n);
	wtip (fov_w, "Field of view scale");
	XtAddCallback (fov_w, XmNdragCallback, sv_scale_cb, (XtPointer)FOV_S);
	XtAddCallback (fov_w, XmNvalueChangedCallback, sv_scale_cb, 
							    (XtPointer)FOV_S);
	XtManageChild (fov_w);
	sr_reg (fov_w, NULL, skycategory, 0);

	/* make the right scale */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ttbar_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, rtbar_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, azra_w); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
	XtSetArg (args[n], XmNminimum, -1080); n++;	/* changed dynamicly */
	XtSetArg (args[n], XmNmaximum, 1080); n++;
	XtSetArg (args[n], XmNshowValue, False); n++;
	altdec_w = XmCreateScale (svform_w, "AltDecScale", args, n);
	wtip (altdec_w, "Altitude or Declination scale");
	XtAddCallback (altdec_w, XmNdragCallback, sv_scale_cb,
							(XtPointer)ALTDEC_S);
	XtAddCallback (altdec_w, XmNvalueChangedCallback, sv_scale_cb,
							(XtPointer)ALTDEC_S);
	XtManageChild (altdec_w);
	sr_reg (altdec_w, NULL, skycategory, 0);

	/* make the sky drawing area in a frame inside the sliders */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ttbar_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, azra_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, fov_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, altdec_w); n++;
	fr_w = XmCreateFrame (svform_w, "MapF", args, n);
	XtManageChild (fr_w);

	    n = 0;
	    svda_w = XmCreateDrawingArea (fr_w, "SkyMap", args, n);
	    XtAddCallback (svda_w, XmNexposeCallback, sv_da_exp_cb, 0);
	    mask = Button1MotionMask | Button2MotionMask | ButtonPressMask
				     | ButtonReleaseMask | PointerMotionMask
				     | EnterWindowMask   | LeaveWindowMask
				     | PointerMotionHintMask;
	    XtAddEventHandler (svda_w, mask, False, sv_da_ptr_eh, 0);
	    XtManageChild (svda_w);

	/* fill in toolbars */
	svtb_create (ttbar_w, ltbar_w, rtbar_w);

	/* create the filter dialog.
	 * it's not managed yet, but its state info is used right off.
	 */
	svf_create(svshell_w);

	/* make the options menu */
	sv_create_options ();

	/* set up user's defaults.
	 * N.B. need Options by now since sv_set_scale(FOV_S) uses wantamag_w
	 * si_create() is to sync ROI tb state with resource
	 */
	sv_read_scale(AZRA_S);
	sv_set_scale(AZRA_S, 1);
	sv_read_scale(ALTDEC_S);
	sv_set_scale(ALTDEC_S, 1);
	sv_read_scale(FOV_S);
	sv_set_scale(FOV_S, 1);
	si_create();

	/* now fill in the menu bar. this late because "History" pd must follow
	 * creating other stuff since it adds the first item.
	 */
	sv_fillin_mb (mb_w);

	/* init toolbar to match options */
	svtb_sync();

#if XmVersion == 1001
	/* try to connect arrow and page up/down keys to FOV initially */
	XmProcessTraversal (fov_w, XmTRAVERSE_CURRENT);
	XmProcessTraversal (fov_w, XmTRAVERSE_CURRENT); /* yes, twice!! */
#endif

#ifdef XmNinitialFocus
	/* try to connect arrow and page up/down keys to FOV initially */
	/* this approach showed up in Motif 1.2 */
	set_something (svform_w, XmNinitialFocus, (XtArgVal)fov_w);
#endif

	/* make the gcs and set up pixels */
	sv_mk_gcs();

	/* register trail resource and init setup */
	sr_reg (NULL, svtrres, skycategory, 0);
	tr_getres (svtrres, &trstate);

	/* create the crosshair cursur used under the mag glass */
	magglass_cursor = XCreateFontCursor (XtD, XC_tcross);
}

static void
gridStepLabel()
{
	/* f_showit avoids blinking when updating scope position */
	f_showit (vgridl_w, want_aagrid ? "Alt: " : "Dec: ");
	f_showit (hgridl_w, want_aagrid ? "Az: "  : "RA: ");
}

/* create the Options dialog.
 */
static void
sv_create_options ()
{
	static ViewOpt vopts[] = {
	    {"Just dots",  0, "JustDots",      &justdots,        &justd_w,
	    	"Draw all star types using simple dots, not typed symbols"},
	    {"Flip Left/Right", 0, "FlipLR",   &flip_lr,         &fliplr_w,
	    	"Flip display left-to-right"},
	    {"Flip Top/Bottom",  0, "FlipTB",  &flip_tb,         &fliptb_w,
	    	"Flip display top-to-bottom"},
	    {"Equatorial plane", 0, "Equator",  &want_eq,   	 &eq_w,
	    	"Draw the Celestial Equator as a 1:2 dotted line"},
	    {"Ecliptic plane", 0, "Ecliptic",  &want_ec,         &eclip_w,
	    	"Draw the Ecliptic as a 1:3 dotted line, and mark anti-solar location"},
	    {"Galactic plane", 0, "Galactic",  &want_ga,         &galac_w,
	    	"Draw the galactic equator as a 1:4 dotted line, and mark the pole"},
	    {"Eyepieces",  0, "Eyepieces",     &want_eyep,       &wanteyep_w,
	    	"Display eyepieces in current view, if any"},
	    {"Magnitude key",  0, "MagScale",  &want_magscale,   &wantmag_w,
	    	"Display table of dot sizes vs. star magnitudes in lower corner"},
	    {"Compass rose",   0, "Compass",   &want_compass,    &wantcompass_w,
	    	"Display compass rose showing directions at center of chart"},
	    {"Auto magnitude", 0, "AutoMag",   &want_automag,    &wantamag_w,
                "Maintain a reasonable faint magnitude based on Field of View"},
	    {"Field stars",0, "FieldStars",    &want_fs,         &wantfs_w,
	    	"Auto load and show Field Stars when position changes)"},
	    {"Live dragging",  0, "LiveDrag",  &want_livedrag,   NULL,
		"Update scene while dragging, not just when release"},
	    {"Horizon map",   0, "HznMap",     &want_hznmap,     &hznmap_w,
	    	"Draw horizon map from Az/Alt profile in .hzn file"},
	    {"Clipping",   0, "ClipObjects",   &want_clipping,   NULL,
	    	"Don't draw objects which are below horizon, if a horizon is drawn"},
	    {"Live report", 0, "LiveReport",  &want_report,      &wantreport_w,
	        "Live report in corners" },
	};
	static ViewOpt cnsopts[] = {
	    {"Figures",    0, "CnsFigures",    &want_conf,       &conf_w,
	    	"Draw constellation figures from above file"},
	    {"Boundaries", 0, "CnsBoundaries", &want_conb,       &conb_w,
	    	"Draw constellation boundaries"},
	    {"Full names", 0, "CnsNames",      &want_conn,       &conn_w,
	    	"Label constellations with full name centered in figure"},
	    {"Abbreviations", 0, "CnsAbbr",    &want_cona,       &cona_w,
	       "Label constellations with abbreviation centered in figure"},
	};
	Widget w, f_w, rc_w;
	Widget hcrc_w, cfom_w;
	Arg args[20];
	XmString str;
	char *s[1];
	int i;
	int n;

	/* create form */

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	svops_w = XmCreateFormDialog (svshell_w, "SkyOps", args, n);
	set_something (svops_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (XtParent(svops_w), "XEphem*SkyOps.x", skyocategory, 0);
	sr_reg (XtParent(svops_w), "XEphem*SkyOps.y", skyocategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "Sky options"); n++;
	XtSetValues (XtParent(svops_w), args, n);

	/* put everything in a RC */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	rc_w = XmCreateRowColumn (svops_w, "RC", args, n);
	XtManageChild (rc_w);

	n = 0;
	w = XmCreateLabel (rc_w, "SOL", args, n);
	set_xmstring (w, XmNlabelString, "Display mode:");
	XtManageChild (w);

	/* simulate a radio boxes for alt-az/ra-dec and spherical/cylindrical */

	n = 0;
	f_w = XmCreateForm (rc_w, "AARDF", args, n);
	XtManageChild (f_w);

	    str = XmStringCreate ("Alt-Az", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    aa_w = XmCreateToggleButton (f_w, "AltAzMode", args, n);
	    XtAddCallback (aa_w, XmNvalueChangedCallback, sv_aa_cb, 0);
	    wtip (aa_w, "Orient display with Az Left-Right, Alt Up-Down");
	    XtManageChild (aa_w);
	    XmStringFree (str);
	    sr_reg (aa_w, NULL, skyocategory, 0);

	    str = XmStringCreate ("RA-Dec", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, aa_w); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    rad_w = XmCreateToggleButton(f_w, "RADecMode", args, n);
	    XtAddCallback (rad_w, XmNvalueChangedCallback, sv_aa_cb, 0);
	    wtip (rad_w, "Orient display with RA Left-Right, Dec Up-Down");
	    XtManageChild (rad_w);
	    XmStringFree (str);

	    aa_mode = XmToggleButtonGetState (aa_w);
	    XmToggleButtonSetState (rad_w, !aa_mode, False); 

	    str = XmStringCreate ("Sphere", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 50); n++;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    sph_w = XmCreateToggleButton (f_w, "SphProj", args, n);
	    XtAddCallback (sph_w, XmNvalueChangedCallback, sv_cyl_cb, 0);
	    wtip (sph_w, "Draw using Spherical projection");
	    XtManageChild (sph_w);
	    XmStringFree (str);

	    str = XmStringCreate ("Cylinder", XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sph_w); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 50); n++;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    cyl_w = XmCreateToggleButton(f_w, "CylProj", args, n);
	    XtAddCallback (cyl_w, XmNvalueChangedCallback, sv_cyl_cb, 0);
	    wtip (cyl_w, "Draw using Cylindrical projection");
	    XtManageChild (cyl_w);
	    XmStringFree (str);
	    sr_reg (cyl_w, NULL, skyocategory, 0);

	    cyl_proj = XmToggleButtonGetState (cyl_w);
	    XmToggleButtonSetState (sph_w, !cyl_proj, False); 

	/* make all the "grid" controls in their own form */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	w = XmCreateLabel (rc_w, "SOL", args, n);
	set_xmstring (w, XmNlabelString, "Grid Control:");
	XtManageChild (w);

	n = 0;
	f_w = XmCreateForm (rc_w, "Grid", args, n);
	XtManageChild (f_w);

	sv_create_grid_option (f_w);

	/* make the collection of options */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	w = XmCreateLabel (rc_w, "SOL", args, n);
	set_xmstring (w, XmNlabelString, "View Options:");
	XtManageChild (w);

	for (i = 0; i < XtNumber(vopts); i++) {
	    ViewOpt *vp = &vopts[i];

	    str = XmStringCreate (vp->label, XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateToggleButton (rc_w, vp->name, args, n);
	    XtAddCallback (w, XmNvalueChangedCallback, sv_option_cb,
						    (XtPointer)(vp->flagp));

	    XtManageChild (w);
	    XmStringFree(str);

	    if (vp->flagp)
		*vp->flagp = XmToggleButtonGetState (w);
	    if (vp->wp)
		*vp->wp = w;
	    if (vp->tip)
		wtip (w, vp->tip);
	    sr_reg (w, NULL, skyocategory, 0);
	}

	/* if automag off initially consider as having been turned off */
	if (!want_automag)
	    user_automagoff = 1;

	/* add the Constellations options */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	w = XmCreateLabel (rc_w, "SOL", args, n);
	set_xmstring (w, XmNlabelString, "Constellation:");
	XtManageChild (w);

	s[0] = cns_suffix;
	cfom_w = createFSM (rc_w, s, 1, "auxil", sv_loadcnsfigs_cb);
	set_xmstring (cfom_w, XmNlabelString, "");
	wtip (cfom_w, "Select a constellation figure definition file");
	load_deffig (cfom_w);

	for (i = 0; i < XtNumber(cnsopts); i++) {
	    ViewOpt *vp = &cnsopts[i];

	    str = XmStringCreate (vp->label, XmSTRING_DEFAULT_CHARSET);
	    n = 0;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    XtSetArg (args[n], XmNlabelString, str); n++;
	    w = XmCreateToggleButton (rc_w, vp->name, args, n);
	    XtAddCallback (w, XmNvalueChangedCallback, sv_option_cb,
						    (XtPointer)(vp->flagp));
	    XtManageChild (w);
	    *(vp->flagp) = XmToggleButtonGetState (w);
	    XmStringFree(str);

	    if (vp->wp)
		*vp->wp = w;
	    if (vp->tip)
		wtip (w, vp->tip);
	    sr_reg (w, NULL, skyocategory, 0);
	}

	/* add the Labeling controls */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	w = XmCreateLabel (rc_w, "SOL", args, n);
	set_xmstring (w, XmNlabelString, "Labeling:");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNseparatorType, XmNO_LINE); n++;
	XtSetArg (args[n], XmNheight, 2); n++;
	w = XmCreateSeparator (rc_w, "LS", args, n);
	XtManageChild (w);

	sv_create_op_lbl (rc_w);

	/* the help and close buttons in its own rc so it can be centered */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNisAligned, False); n++;
	hcrc_w = XmCreateRowColumn (rc_w, "HCRC", args, n);
	XtManageChild (hcrc_w);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	    w = XmCreatePushButton (hcrc_w, "Help", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_helpon_cb, "SkyView_ops");
	    wtip (w, "Additional details");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	    w = XmCreatePushButton (hcrc_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_closeopdialog_cb, 0);
	    wtip (w, "Close this dialog");
	    XtManageChild (w);
}

/* load the default cns_ffres constellation figures file and set the
 * given option menu to display it
 * abort if trouble, we really need this now.
 */
static void
load_deffig (Widget om)
{
	WidgetList children;
	Cardinal numChildren;
	char msg[1024];
	Widget pane;
	char *fn;
	FILE *fp;

	/* open */
	fn = getXRes (cns_ffres, NULL);
	fp = fopendq (fn, "auxil", "r");
	if (!fp) {
	    printf ("can not find %s\n", fn);
	    abort();
	}

	/* load */
	if (cns_loadfigs (fp, msg) < 0) {
	    printf ("%s: %s\n", fn, msg);
	    abort();
	}
	fclose (fp);

	/* register file name resource */
	sr_reg (NULL, cns_ffres, skyocategory, 0);

	/* set name in om's pulldown */
	get_something (om, XmNsubMenuId, (XtArgVal)&pane);
	get_something (pane, XmNchildren, (XtArgVal)&children);
	get_something (pane, XmNnumChildren, (XtArgVal)&numChildren);
	if (numChildren == 1) 
	    set_xmstring (children[0], XmNlabelString, fn);
}

/* helper func to make all the "grid" controls in the given form */
static void
sv_create_grid_option (f_w)
Widget f_w;
{
	Arg args[20];
	int n;

	/* main grid on/off toggle */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	grid_w = XmCreateToggleButton (f_w, "Grid", args, n);
	XtAddCallback (grid_w, XmNvalueChangedCallback, sv_grid_cb,
						    (XtPointer)&want_grid);
	wtip (grid_w, "Overlay map with a coordinate grid");
	XtManageChild (grid_w);
	sr_reg (grid_w, NULL, skyocategory, 0);

	want_grid = XmToggleButtonGetState (grid_w);

	/* grid auto spacing on/off toggle */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 50); n++;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	autogrid_w = XmCreateToggleButton (f_w, "Auto", args, n);
	XtAddCallback (autogrid_w, XmNvalueChangedCallback, sv_grid_cb,
						(XtPointer)&want_autogrid);
	wtip (autogrid_w, "Whether to automatically set grid spacing");
	XtManageChild (autogrid_w);
	sr_reg (autogrid_w, NULL, skyocategory, 0);

	want_autogrid = XmToggleButtonGetState (autogrid_w);

	/* V grid label and text field */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, grid_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNcolumns, GRIDLEN); n++;
	XtSetArg (args[n], XmNmaxLength, GRIDLEN); n++;
	vgrid_w = XmCreateTextField (f_w, "VGrid", args, n);
	XtAddCallback (vgrid_w, XmNactivateCallback, sv_gridtf_cb, 0);
	wtip (vgrid_w, "Vertical grid step size");
	sr_reg (vgrid_w, NULL, skyocategory, 0);
	XtManageChild (vgrid_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, grid_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	vgridl_w = XmCreateLabel (f_w, "SVGridVL", args, n);
	XtManageChild (vgridl_w);

	/* H grid label and text field */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, vgrid_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNcolumns, GRIDLEN); n++;
	XtSetArg (args[n], XmNmaxLength, GRIDLEN); n++;
	hgrid_w = XmCreateTextField (f_w, "HGrid", args, n);
	XtAddCallback (hgrid_w, XmNactivateCallback, sv_gridtf_cb, 0);
	wtip (hgrid_w, "Horizontal grid step size");
	sr_reg (hgrid_w, NULL, skyocategory, 0);
	XtManageChild (hgrid_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, vgrid_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	hgridl_w = XmCreateLabel (f_w, "SVGridHL", args, n);
	XtManageChild (hgridl_w);

	/* fake a left radio box for grid system */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, hgrid_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	aagrid_w = XmCreateToggleButton (f_w, "AltAz", args, n);
	XtAddCallback (aagrid_w, XmNvalueChangedCallback, sv_grid_cb, 
						(XtPointer)&want_aagrid);
	wtip (aagrid_w, "Draw Alt-Az grid");
	set_xmstring (aagrid_w, XmNlabelString, "Alt-Az");
	XtManageChild (aagrid_w);
	sr_reg (aagrid_w, NULL, skyocategory, 0);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, aagrid_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	naagrid_w = XmCreateToggleButton (f_w, "RADec", args, n);
	wtip (naagrid_w, "Draw RA-Dec grid");
	XtAddCallback (naagrid_w, XmNvalueChangedCallback, sv_gt_cb, 
							(XtPointer)aagrid_w);
	set_xmstring (naagrid_w, XmNlabelString, "RA-Dec");
	XtManageChild (naagrid_w);

	want_aagrid = XmToggleButtonGetState (aagrid_w);
	XmToggleButtonSetState (naagrid_w, !want_aagrid, False);
	gridStepLabel();

	/* TB whether want labels */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, hgrid_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 50); n++;
	XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
	XtSetArg (args[n], XmNmarginHeight, 2); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	gridlbl_w = XmCreateToggleButton (f_w, "Labels", args, n);
	XtAddCallback (gridlbl_w, XmNvalueChangedCallback, sv_grid_cb, 
						    (XtPointer)&want_gridlbl);
	want_gridlbl = XmToggleButtonGetState (gridlbl_w);
	wtip (gridlbl_w, "Add labels to grid lines");
	XtManageChild (gridlbl_w);
	sr_reg (gridlbl_w, NULL, skyocategory, 0);
}

/* helper function to create the labeling controls in the Options dialog */
static void
sv_create_op_lbl (rc_w)
Widget rc_w;
{
	static ViewOpt lblnbr[] = {
	    /* these are TB/TB/Scale triples */
	    {"N",           0, "LblStName",     &lbl_lst,    &lbl_nst_w,
	    	"Whether Name is in label for stars"},
	    {"M",           0, "LblStMag",      &lbl_lst,    &lbl_mst_w,
	    	"Whether Magnitude is in label for stars"},
	    {"Stars",       0,"LblNStars",      0,           &lbl_bst_w,
	    	"Number of the brightest catalog stars to label"},

	    {"N",           0, "LblFSName",     &lbl_lfs,    &lbl_nfs_w,
	    	"Whether Name is in label for field stars"},
	    {"M",           0, "LblFSMag",      &lbl_lfs,    &lbl_mfs_w,
	    	"Whether Magnitude is in label for stars"},
	    {"Field stars", 0, "LblNFStars",    0,           &lbl_bfs_w,
	    	"Number of the brightest field stars to label"},

	    {"N",           0, "LblSSName",     &lbl_lss,    &lbl_nss_w,
	    	"Whether Name is in label for Solar System objects"},
	    {"M",           0, "LblSSMag",      &lbl_lss,    &lbl_mss_w,
	    	"Whether Magnitude is in label for Solar System objects"},
	    {"Sol Sys",     0,"LblNSolSys",     0,           &lbl_bss_w,
	    	"Number of the brightest solar system objects to label"},

	    {"N",           0, "LblDSName",     &lbl_lds,    &lbl_nds_w,
	    	"Whether Name is in label for Deep Sky objects"},
	    {"M",           0, "LblDSMag",      &lbl_lds,    &lbl_mds_w,
	    	"Whether Magnitude is in label for Deep Sky objects"},
	    {"Deep Sky",    0, "LblNDeepSky",   0,           &lbl_bds_w,
	    	"Number of the brightest deep sky objects to lablel"},
	};
	Widget n_w, m_w, s_w;
	XmFontList sfl;
	Widget nl_w, ml_w;
	Widget f_w;
	Arg args[20];
	int i;
	int n;

	/* put everything in a form */

	n = 0;
	f_w = XmCreateForm (rc_w, "VOLF", args, n);
	XtManageChild (f_w);

	/* the three stacked controls */

	for (i = 0; i < XtNumber(lblnbr); /* incremented in body */ ) {
	    ViewOpt *vp;

	    vp = &lblnbr[i++];
	    n = 0;
	    if (vp == lblnbr) {
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    } else {
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
		XtSetArg (args[n], XmNtopWidget, *vp[-1].wp); n++;
	    }
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
	    XtSetArg (args[n], XmNindicatorOn, True); n++;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNindicatorSize, LOSTBSZ); n++;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    n_w = XmCreateToggleButton (f_w, vp->name, args, n);
	    XtAddCallback (n_w, XmNvalueChangedCallback, sv_lbln_cb,
							(XtPointer)vp->flagp);
	    set_xmstring (n_w, XmNlabelString, "");
	    XtManageChild (n_w);
	    *vp->wp = n_w;
	    wtip (n_w, vp->tip);
	    if (XmToggleButtonGetState (n_w))
		*(vp->flagp) |= OBJF_NLABEL;
	    sr_reg (n_w, NULL, skyocategory, 0);

	    vp = &lblnbr[i++];
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, n_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, n_w); n++;
	    XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
	    XtSetArg (args[n], XmNindicatorOn, True); n++;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    XtSetArg (args[n], XmNindicatorSize, LOSTBSZ); n++;
	    XtSetArg (args[n], XmNmarginWidth, 0); n++;
	    m_w = XmCreateToggleButton (f_w, vp->name, args, n);
	    XtAddCallback (m_w, XmNvalueChangedCallback, sv_lblm_cb,
							(XtPointer)vp->flagp);
	    set_xmstring (m_w, XmNlabelString, "");
	    XtManageChild (m_w);
	    *vp->wp = m_w;
	    wtip (m_w, vp->tip);
	    if (XmToggleButtonGetState (m_w))
		*(vp->flagp) |= OBJF_MLABEL;
	    sr_reg (m_w, NULL, skyocategory, 0);

	    vp = &lblnbr[i++];
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, n_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, m_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    XtSetArg (args[n], XmNminimum, 0); n++;
	    XtSetArg (args[n], XmNmaximum, MAXBRLBLS); n++;
	    XtSetArg (args[n], XmNshowValue, False); n++;
	    XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	    XtSetArg (args[n], XmNscaleHeight, LOSTBSZ); n++;
	    s_w = XmCreateScale (f_w, vp->name, args, n);
	    XtAddCallback (s_w, XmNvalueChangedCallback, sv_brs_cb, 0);
	    set_xmstring (s_w, XmNtitleString, vp->label);
	    XtManageChild (s_w);
	    *vp->wp = s_w;
	    wtip (s_w, vp->tip);
	    sr_reg (s_w, NULL, skyocategory, 0);
	}

	/* last scale is hooked to bottom */

	set_something (s_w, XmNbottomAttachment, XmATTACH_FORM);

	/* add two more labels in lower left.. match Scale font */

	get_something (s_w, XmNfontList, (XtArgVal)&sfl);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, n_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, m_w); n++;
	XtSetArg (args[n], XmNfontList, sfl); n++;
	XtSetArg (args[n], XmNmarginWidth, 0); n++;
	ml_w = XmCreateLabel (f_w, "M", args, n);
	XtManageChild (ml_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, n_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, n_w); n++;
	XtSetArg (args[n], XmNfontList, sfl); n++;
	XtSetArg (args[n], XmNmarginWidth, 0); n++;
	nl_w = XmCreateLabel (f_w, "N", args, n);
	XtManageChild (nl_w);
}

/* callback from selecting the Close PB in the Options control. */
/* ARGSUSED */
static void
sv_closeopdialog_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (svops_w);
}

/* callback from selecting the Options control. */
/* ARGSUSED */
static void
sv_opdialog_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (!XtIsManaged(svops_w))
	    XtManageChild (svops_w);
}

/* make the "Favorites" cascade button and menu.
 * empty for now, filled when cascades
 */
static void
sv_create_favmenu (parent)
Widget parent;
{
	Arg args[20];
	Widget pd_w, cb_w;
	int n;

	n = 0;
	pd_w = XmCreatePulldownMenu (parent, "FavPD", args, n);
	XtAddCallback (pd_w, XmNmapCallback, sv_faving_cb, NULL);

	n = 0;
	XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNmnemonic, 'F'); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	cb_w = XmCreateCascadeButton (parent, "FavCB", args, n);
	set_xmstring (cb_w, XmNlabelString, "Favorites");
	wtip (cb_w, "Center on a Favorite");
	XtManageChild (cb_w);
}

/* callback from the main Close button */
/* ARGSUSED */
static void
sv_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* let popdown do all the work */
	XtPopdown (svshell_w);
}

/* callback from closing the main skyview shell.
 * N.B. do not try to reclaim memory so that we can run FITS Watch while down.
 */
/* ARGSUSED */
static void
sv_popdown_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (svops_w);
	sl_unmanage ();
	svf_unmanage();
	sf_unmanage ();
	si_unmanage ();
	sc_unmanage ();
	hzn_unmanage ();
	siwcs_unmanage ();
	se_unmanage ();
	svh_unmanage();
	sfifo_closein();

	if (fs_to != 0) {
	    XtRemoveTimeOut (fs_to);
	    fs_to = 0;
	}

	setXRes (sv_viewupres(), "0");
}

/* callback from the overall Help button.
 */
/* ARGSUSED */
static void
sv_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"This displays all database objects currently in memory onto the sky. The view",
"may be Alt/Az or RA/Dec. The three sliders adjust the field of fov, the",
"azimuth (or RA), and the altitude (or Dec). Objects may be filtered out by",
"type and magnitude."
};

	hlp_dialog ("SkyView", msg, sizeof(msg)/sizeof(msg[0]));
}

/* callback from a specific Help button.
 * client is a string to use with hlp_dialog().
 */
/* ARGSUSED */
static void
sv_helpon_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hlp_dialog ((char *)client, NULL, 0);
}

/* callback when either the Cylindrical or Spherical projection TB changes.
 * distinguish by comparing w to sph_w or cyl_w.
 */
/* ARGSUSED */
static void
sv_cyl_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *sp = (XmToggleButtonCallbackStruct *)call;
	int cyl = (w == cyl_w);
	Now *np = mm_get_now();

	/* set new state */
	cyl_proj = (cyl == sp->set);
	svtb_updateCyl (cyl_proj);

	/* implement radio box */
	if (cyl)
	    XmToggleButtonSetState (sph_w, !cyl_proj, False);
	else
	    XmToggleButtonSetState (cyl_w, cyl_proj, False);

	/* deactivate any FITS image */
	si_off();

	/* discard zoom history */
	zm_noundo();

	/* redraw */
	sv_all(np);
}

/* callback when either the alt/az or ra/dec button changes state.
 * distinguish by comparing w to aa_w or rad_w.
 */
/* ARGSUSED */
static void
sv_aa_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
#define	POLEERR		degrad(90 - 1.0/3600.0)	/* polar ambiguous area */
	XmToggleButtonCallbackStruct *sp = (XmToggleButtonCallbackStruct *)call;
	static double last_az_polar_angle = 0.0;
	static double last_ra_polar_angle = 0.0;
	int aaw = (w == aa_w);
	int wantaa = (aaw == sp->set);
	Now *np = mm_get_now();
	double alt, az, ra, dec;

	/* save the ambiguous polar angle if at the pole for later restore */
	if (fabs(sv_altdec) > POLEERR) {
	    if (aa_mode)
		last_az_polar_angle = sv_azra;
	    else
		last_ra_polar_angle = sv_azra;
	}

	/* prepare for current coords in opposite coord system */
	sv_fullwhere (np, sv_altdec, sv_azra, aa_mode, &alt, &az, &ra, &dec);

	/* engage */
	if (wantaa) {
	    /* change from ra/dec to alt/az mode */
	    sv_altdec = alt;
	    sv_azra = az;
	    aa_mode = 1;
	} else {
	    /* change from alt/az to ra/dec mode */
	    sv_azra = ra;
	    sv_altdec = dec;
	    aa_mode = 0;
	}

	/* restore the ambiguous polar angle if came back */
	if (fabs(sv_altdec) > POLEERR) {
	    if (aa_mode)
		sv_azra = last_az_polar_angle;
	    else
		sv_azra = last_ra_polar_angle;
	}

	/* simulate radiobox pair.
	 * N.B. only change the _other_ TB
	 */
	if (aaw)
	    XmToggleButtonSetState (rad_w, !aa_mode, False);
	else
	    XmToggleButtonSetState (aa_w, aa_mode, False);

	si_off();
	zm_noundo();
	sv_set_scale(ALTDEC_S, 0);
	sv_set_scale(AZRA_S, 0);

	/* redraw */
	if (track_op) {
	    if (sv_mark (track_op, 1, 1, 1, 1, 0, 0.0) < 0)
		sv_all(np);	/* still need it even if mark failed */
	} else 
	    sv_all(np);
}

/* callback from the filter button.
 * just bring up the filter dialog.
 */
/* ARGSUSED */
static void
sv_filter_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	svf_manage();
}

/* callback from the FITS dialog button.
 * just bring up the FITS dialog.
 */
/* ARGSUSED */
static void
sv_fits_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	sf_manage();
}

/* callback from the image dialog button.
 * just bring up the image dialog.
 */
/* ARGSUSED */
static void
sv_image_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	si_manage();
}

/* bring up the image reg dialog.
 */
/* ARGSUSED */
static void
sv_registration_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	ir_manage();
}

/* callback from the wcs dialog button.
 * just bring up the wcs dialog.
 */
/* ARGSUSED */
static void
sv_wcs_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	siwcs_manage();
}

/* callback to add the current scene to a movie loop.
 */
/* ARGSUSED */
static void
sv_addframe_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	ml_add (sv_pm, dt_w);
}

/* callback from the List control.
 * just bring up the list dialog.
 */
/* ARGSUSED */
static void
sv_list_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	sl_manage();
}

/* callback from the two fake radio buttons in the grid control.
 * client is the paired widget to toggle.
 */
/* ARGSUSED */
static void
sv_gt_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Widget pw = (Widget)client;

	XmToggleButtonSetState (pw, !XmToggleButtonGetState(w), True);
}

/* callback from any of the grid control buttons.
 * client is pointer to want_* flag effected.
 * redrawing all keeps the overlays in good order.
 */
/* ARGSUSED */
static void
sv_grid_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int state = XmToggleButtonGetState(w);
	int *wantp = (int *)client;

	/* update state flag and inform toolbar */
	*wantp = state;
	svtb_updateGrid(want_grid);

	/* implement the fake radio box */
	if (w == aagrid_w)
	    XmToggleButtonSetState (naagrid_w, !state, False);

	/* no need to actually redraw if already off or just turning off auto */
	if (wantp != &want_grid && !want_grid)
	    return;
	if (wantp == &want_autogrid && !want_autogrid)
	    return;

	sv_all(mm_get_now());
}

/* callback from any of the N Brightest scale.
 */
/* ARGSUSED */
static void
sv_brs_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	sv_all(mm_get_now());
}

/* callback from RETURN in either grid size TF */
/* ARGSUSED */
static void
sv_gridtf_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* turns off Auto as a side effect */
	XmToggleButtonSetState (autogrid_w, want_autogrid = False, True);

	if (want_grid)
	    sv_all(mm_get_now());
}

/* callback from the manual coordinates PB
 */
/* ARGSUSED */
static void
sv_mancoord_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	cc_manage();
}

/* callback from the Print control.
 */
/* ARGSUSED */
static void
sv_print_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XPSAsk ("Sky View", sv_print);
}

/* proceed to generate a postscript file.
 * call XPSClose() when finished.
 */
static void
sv_print ()
{
	Now *np = mm_get_now();

	if (!sv_ison()) {
	    xe_msg (1, "Sky View must be open to print.");
	    XPSClose();
	    return;
	}

	watch_cursor(1);

	/* fit view in square across the top and prepare to capture X calls*/
	if (sv_w > sv_h)
	    XPSXBegin (sv_pm, 0, 0, sv_w, sv_h, 1*72, 10*72, (int)(6.5*72));
	else {
	    int pw = (int)(72*6.5*sv_w/sv_h+.5);  /* width on paper as 6.5 hi */
	    XPSXBegin (sv_pm, 0, 0, sv_w, sv_h, (int)((8.5*72-pw)/2), 10*72,pw);
	}

	/* redraw */
	sv_all (np);

	/* no more X captures */
	XPSXEnd();

	/* add some extra info */
	sv_ps_annotate (np);

	/* finished */
	XPSClose();

	watch_cursor(0);
}

static void
sv_ps_annotate (np)
Now *np;
{
	double alt, az, ra, dec;
	double fov;
	char dir[256];
	char buf[256];
	char *bp;
	char *site;
	double tmp;
	int aamode;
	int ctr = 306;	/* = 8.5*72/2 */
	int y;

 	/* separator */
 	y = (int)AROWY(14.5);
 	sprintf (buf, "newpath 108 %d moveto 504 %d lineto stroke\n", y, y);
 	XPSDirect (buf);
 
	/* if displaying an image, start with filename and object name */
	if (si_ison()) {
	    char fn[1024];

	    sf_getName (fn, buf);
	    if (fn[0] || buf[0]) {
		y = AROWY(15);
		(void) sprintf (dir, "(%s %s) %d %d cstr", fn, buf, ctr, y);
		XPSDirect (dir);
	    }
	}

	sv_getcenter (&aamode, &fov, &alt, &az, &ra, &dec);

	/* caption */
	XPSDirect ("\n% Circumstances\n");
	y = AROWY(13);
	if (aa_mode) {
	    (void) strcpy (dir, "(XEphem Alt/Az Sky View");
	} else {
	    (void) sprintf (dir, "(XEphem %s %s RA/Dec Sky View",
		    pref_get (PREF_EQUATORIAL) == PREF_TOPO ? "Topocentric"
		    					    : "Geocentric",
					epoch == EOD ? "Apparent" : "Mean");
	}
	if (anytrails) {
	    char *wtz= pref_get(PREF_ZONE)==PREF_LOCALTZ ? "Local time" : "UTC";
	    (void) sprintf (dir+strlen(dir), " -- Trail Times are in %s", wtz);
	}
	(void) sprintf (dir+strlen(dir), ") %d %d cstr", ctr, y);
	XPSDirect (dir);

	site = mm_getsite();
	if (site) {
	    y = AROWY(12);
	    (void) sprintf (dir, "(%s) %d %d cstr\n",
	    				XPSCleanStr(site,strlen(site)), ctr, y);
	    XPSDirect (dir);
	}

	/* left column */

	y = AROWY(11);
	fs_sexa (buf, radhr(ra), 2, 36000);
	for (bp = buf; *bp == ' '; bp++) continue;
	(void) sprintf (dir, "(Center RA:) 124 %d rstr (%s) 134 %d lstr\n",
								    y, bp, y);
	XPSDirect (dir);

	y = AROWY(10);
	fs_sexa (buf, raddeg(dec), 3, 3600);
	for (bp = buf; *bp == ' '; bp++) continue;
	(void) sprintf (dir, "(Declination:) 124 %d rstr (%s) 134 %d lstr\n",
								y, bp, y);
	XPSDirect (dir);

	y = AROWY(9);
	if (epoch == EOD)
	    (void) strcpy (buf, "EOD");
	else {
	    mjd_year (epoch, &tmp);
	    (void) sprintf (buf, "%.2f", tmp);
	}
	(void) sprintf (dir, "(Epoch:) 124 %d rstr (%s) 134 %d lstr\n",y,buf,y);
	XPSDirect (dir);

	y = AROWY(8);
	fs_sexa (buf, raddeg(alt), 3, 3600);
	for (bp = buf; *bp == ' '; bp++) continue;
	(void) sprintf (dir, "(Altitude:) 124 %d rstr (%s) 134 %d lstr\n",
								    y, bp, y);
	XPSDirect (dir);

	y = AROWY(7);
	fs_sexa (buf, raddeg(az), 4, 3600);
	for (bp = buf; *bp == ' '; bp++) continue;
	(void) sprintf (dir, "(Azimuth:) 124 %d rstr (%s) 134 %d lstr\n",
								    y, bp, y);
	XPSDirect (dir);

	y = AROWY(6);
	fs_sexa (buf, raddeg(fov), 3, 60);
	for (bp = buf; *bp == ' '; bp++) continue;
	(void) sprintf (dir,"(Field Width:) 124 %d rstr (%s) 134 %d lstr\n",
								    y, bp, y);
	XPSDirect (dir);

	/* right column */

	y = AROWY(11);
	(void) sprintf (dir,"(Julian Date:) 460 %d rstr (%13.5f) 470 %d lstr\n",
							    y, mjd+MJD0, y);
	XPSDirect (dir);

	y = AROWY(10);
	now_lst (np, &tmp);
	fs_time (buf, tmp);
	for (bp = buf; *bp == ' '; bp++) continue;
	(void)sprintf(dir,"(Sidereal Time:) 460 %d rstr (%s) 470 %d lstr\n",
								    y, bp, y);
	XPSDirect (dir);

	y = AROWY(9);
	fs_date (buf, pref_get(PREF_DATE_FORMAT), mjd_day(mjd));
	for (bp = buf; *bp == ' '; bp++) continue;
	(void) sprintf (dir, "(UTC Date:) 460 %d rstr (%s) 470 %d lstr\n",
								    y, bp,y);
	XPSDirect (dir);

	y = AROWY(8);
	fs_time (buf, mjd_hr(mjd));
	for (bp = buf; *bp == ' '; bp++) continue;
	(void) sprintf (dir, "(UTC Time:) 460 %d rstr (%s) 470 %d lstr\n",
								    y, bp,y);
	XPSDirect (dir);

	y = AROWY(7);
	fs_sexa (buf, raddeg(fabs(lat)), 3, 3600);
	for (bp = buf; *bp == ' '; bp++) continue;
	(void) sprintf (dir, "(Latitude:) 460 %d rstr (%s %c) 470 %d lstr\n",
						y, bp, lat < 0 ? 'S' : 'N', y);
	XPSDirect (dir);

	y = AROWY(6);
	fs_sexa (buf, raddeg(fabs(lng)), 4, 3600);
	for (bp = buf; *bp == ' '; bp++) continue;
	(void) sprintf (dir,"(Longitude:) 460 %d rstr (%s %c) 470 %d lstr\n",
						y, bp, lng < 0 ? 'W' : 'E', y);
	XPSDirect (dir);

	/* center column */

	/* show basic directions unless compass on or pole is visible */
	if (!want_compass && sv_altdec - sv_vfov/2 > -PI/2
					    && sv_altdec + sv_vfov/2 < PI/2) {
	    y = AROWY(10);
	    if (aa_mode)
		(void) sprintf (dir, "(Zenith is %s - CW is %s) %d %d cstr\n",
						flip_tb ? "down" : "up",
						flip_lr ? "left" : "right",
									ctr, y);
	    else
		(void) sprintf (dir, "(North is %s - West is %s) %d %d cstr\n",
						flip_tb ? "down" : "up",
						flip_lr ? "left" : "right",
									ctr, y);
	    XPSDirect (dir);
	}

	if (want_grid) {
	    char *hval, *vval;
	    char *nam1, *val1;
	    char *nam2, *val2;

	    hval = XmTextFieldGetString (hgrid_w);
	    vval = XmTextFieldGetString (vgrid_w);

	    if (want_aagrid) {
		nam1 = "Alt";
		nam2 = "Az";
		val1 = vval;
		val2 = hval;
	    } else {
		nam1 = "RA";
		nam2 = "Dec";
		val1 = hval;
		val2 = vval;
	    }

	    y = AROWY(9);
	    (void) sprintf (dir, "(Grid Steps:) %d %d cstr\n", ctr, y);
	    XPSDirect (dir);

	    y = AROWY(8);
	    (void) sprintf (dir,"(%s: %s) %d %d cstr\n", nam1, val1, ctr, y);
	    XPSDirect (dir);

	    y = AROWY(7);
	    (void) sprintf (dir,"(%s: %s) %d %d cstr\n", nam2, val2, ctr, y);
	    XPSDirect (dir);

	    XtFree (hval);
	    XtFree (vval);
	}

	if (want_eyep && largeyepr) {
	    char wbuf[32], *wbp, hbuf[32], *hbp;
	    char *sz;

	    fs_sexa (wbuf, raddeg(largeyepr->eyepw), 3, 60);
	    for (wbp = wbuf; *wbp == ' '; wbp++) continue;
	    fs_sexa (hbuf, raddeg(largeyepr->eyeph), 3, 60);
	    for (hbp = hbuf; *hbp == ' '; hbp++) continue;

	    y = AROWY(6);
	    sz = neyepr > 1 ? "Largest " : "";
	    if (largeyepr->eyepw == largeyepr->eyeph)
		(void) sprintf (dir, "(%sEyepiece: %s) %d %d cstr\n", sz,
								wbp, ctr, y);
	    else
		(void) sprintf (dir, "(%sEyepiece: %s x %s) %d %d cstr\n",
							sz, wbp, hbp, ctr, y);
	    XPSDirect (dir);
	}
}

/* callback when the Control->tracking tb is toggled.
 * N.B. also triggered to implement turning tracking off from the popup too.
 */
/* ARGSUSED */
static void
sv_track_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (!XmToggleButtonGetState (w)) {
	    track_op = (Obj *)0;
	    XtSetSensitive (w, False);
	    sv_all (mm_get_now());	/* erase the mark */
	}
}

/* set one of the scales and its labels from the current real values.
 * also possibly reset any unzoom records.
 */
static void
sv_set_scale(which, cutzoom)
int which;
int cutzoom;
{
	Arg args[20];
	char buf[64];
	int a, n;

	switch (which) {
	case ALTDEC_S:
	    a = (int)floor(raddeg(sv_altdec)*(60./FOV_STEP) + 0.5);
	    XmScaleSetValue (altdec_w, a);

	    (void) strcpy (buf, aa_mode ? "Alt: " : "Dec: ");
	    fs_dm_angle (buf+5, sv_altdec);
	    set_xmstring (altdecl_w, XmNlabelString, buf);
	    break;

	case AZRA_S:
	    a = (int)floor(raddeg(sv_azra)*(60./FOV_STEP) + 0.5);
	    if (a >= 4320)	/* beware of round-up */
		a -= 4320;

	    n = 0;
	    XtSetArg (args[n], XmNvalue, a); n++;
	    XtSetArg (args[n], XmNprocessingDirection,
				aa_mode ? XmMAX_ON_RIGHT : XmMAX_ON_LEFT); n++;
	    XtSetValues (azra_w, args, n);

	    if (aa_mode) {
		(void) strcpy (buf, "Az: ");
		fs_sexa (buf+4, raddeg(sv_azra), 3, 60);
	    } else {
		(void) strcpy (buf, "RA: ");
		fs_sexa (buf+4, radhr(sv_azra), 2, 3600);
	    }
	    set_xmstring (azral_w, XmNlabelString, buf);
	    break;

	case FOV_S: {
	    char vbuf[32], hbuf[32];

	    a = (int)floor(raddeg(sv_vfov)*(60./FOV_STEP)+0.5);
	    if (a < 1)
		a = 1;
	    if (a > 180*(60/FOV_STEP))
		a = 180*(60/FOV_STEP);
	    XmScaleSetValue (fov_w, a);

	    fs_dm_angle (vbuf, sv_vfov);
	    fs_dm_angle (hbuf, sv_hfov);
	    if (strcmp (vbuf, hbuf))
		sprintf (buf, "FOV: %sW x %sH", hbuf, vbuf);
	    else
		sprintf (buf, "FOV: %s", hbuf);
	    set_xmstring (fovl_w, XmNlabelString, buf);

	    /* turn on automag when changing FOV unless user turned it off */
	    if (!user_automagoff) {
		XmToggleButtonSetState (wantamag_w, want_automag = 1, False);
		svtb_updateAutoMag (1);
		svf_automag(sv_vfov);
	    }
	    }

	    break;

	default:
	    printf ("sv_set_scale Bug! bogus which: %d\n", which);
	    abort();
	}

	/* check for desired resets */
	if (cutzoom) {
	    zm_cutundo();
	    svtb_zoomok(0);
	}
}

/* given a new vertical fov set sv_vfov, sv_hfov and sv_dfov.
 * N.B. might be called before first expose when we don't yet know sv_h
 */
static void
sv_set_fov(vfov)
double vfov;
{
	if (vfov > degrad(180.))
	    vfov = degrad(180.);
	sv_vfov = vfov;
	sv_hfov = sv_h ? sv_vfov*sv_w/sv_h : 0;
	sv_dfov = sqrt((double)(sv_hfov*sv_hfov + sv_vfov*sv_vfov));
}

/* expose event of sky view drawing area.
 * if same size just copy from pixmap, else recompute all (it's resized).
 * N.B. we set bit_gravity to ForgetGravity so we can just use Expose events.
 * N.B. we turn off backing store since we effectively do it in the pixmap.
 */
/* ARGSUSED */
static void
sv_da_exp_cb (w, client,
call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmDrawingAreaCallbackStruct *c = (XmDrawingAreaCallbackStruct *)call;
	Display *dsp = XtDisplay(svda_w);
	Window win = XtWindow(svda_w);
	Window root;
	int x, y;
	unsigned int bw, d;
	unsigned int wid, hei;

	switch (c->reason) {
	case XmCR_EXPOSE: {
	    static int before;
	    XExposeEvent *e = &c->event->xexpose;

	    if (!before) {
		XSetWindowAttributes swa;
		unsigned long mask = CWBitGravity | CWBackingStore;

		swa.bit_gravity = ForgetGravity;
		swa.backing_store = NotUseful; /* we use a pixmap */
		XChangeWindowAttributes (dsp, win, mask, &swa);

		before = 1;

		if (svh_nhist() == 0)
		    svh_add_current();
	    }

	    /* wait for the last in the series */
	    if (e->count != 0)
		return;
	    break;
	    }
	default:
	    printf ("Unexpected svda_w event. type=%d\n", c->reason);
	    abort();
	}

	XGetGeometry (dsp, win, &root, &x, &y, &wid, &hei, &bw, &d);

	if (!sv_pm || (int)wid != sv_w || (int)hei != sv_h) {
	    if (sv_pm) {
		/* user resized, else from within */
		XFreePixmap (dsp, sv_pm);
		sv_pm = (Pixmap) NULL;
	    }

	    sv_pm = XCreatePixmap (dsp, win, wid, hei, d);
	    XSetForeground (dsp, sv_gc, bg_p);
	    XFillRectangle (dsp, sv_pm, sv_gc, 0, 0, wid, hei);
	    sv_w = wid;
	    sv_h = hei;
	    zm_noundo();		/* coords no longer valid if unzoom */

	    /* update FOV */
	    if (si_ison()) {
		si_newPixmap (sv_w, sv_h, flip_lr, flip_tb, NULL, 0);
		setImFOVScales (si_getFImage());
	    } else {
		sv_set_fov(sv_vfov);	/* update angular dimensions */
		sv_set_scale(FOV_S, 1);
	    }

	    sv_all(mm_get_now());
	} else {
	    /* same size; just copy from the pixmap */
	    sv_copy_sky();
	}
}

/* resize to [neww, newh].
 * try to keep all on screen too.
 */
static void
sv_resize (neww, newh)
int neww, newh;
{
	Display *dsp = XtDisplay(svda_w);
	int dspw = DisplayWidth(dsp, DefaultScreen(dsp)) - 100;	 /* fudge */
	int dsph = DisplayHeight(dsp, DefaultScreen(dsp)) - 100; /* fudge */
	Position shx, shy;
	Arg args[10];
	int n;

	/* removing the shadow pixmap tells exp_cb() this resize is internal */
	if (sv_pm) {
	    XFreePixmap (dsp, sv_pm);
	    sv_pm = (Pixmap) NULL;
	}

	/* setting new size will cause expose IF size really changes.
	 * we tell by looking for a new sv_pm.
	 */
	n = 0;
	XtSetArg (args[n], XmNwidth, neww); n++;
	XtSetArg (args[n], XmNheight, newh); n++;
	XtSetValues (svda_w, args, n);
	XmUpdateDisplay (svda_w);

	if (!sv_pm) {
	    /* no expose so force */
	    Window win = XtWindow(svda_w);
	    XExposeEvent e;

	    e.type = Expose;
	    e.display = dsp;
	    e.window = win;
	    e.x = e.y = 0;
	    e.width = neww;
	    e.height = newh;
	    e.count = 0;

	    XSendEvent (dsp, win, False, ExposureMask, (XEvent *)&e);
	    XmUpdateDisplay (svda_w);
	}

	/* toolbars impose min sizes, so grow again if still not right */
	if (sv_h > newh && sv_h > sv_w*newh/neww) {
	    /* height was forced larger by toolbar, so make wider too */
	    sv_resize (sv_h*neww/newh, sv_h);
	} else if (sv_w > neww && sv_w > sv_h*neww/newh) {
	    /* width was forced larger by toolbar, so make taller too */
	    sv_resize (sv_w, sv_w*newh/neww);
	}

	/* see if should move left or up to keep on screen */
	n = 0;
	XtSetArg (args[n], XmNx, &shx); n++;
	XtSetArg (args[n], XmNy, &shy); n++;
	XtGetValues (svshell_w, args, n);
	n = 0;
	if (shx + sv_w > dspw) {
	    XtSetArg (args[n], XmNx, 0); n++;
	}
	if (shy + sv_h > dsph) {
	    XtSetArg (args[n], XmNy, 0); n++;
	}
	XtSetValues (svshell_w, args, n);
}

/* draw the magnitude scale, based on the near-sky setting */
static void
sv_magscale (dsp, win)
Display *dsp;
Window win;
{
	int cw, ch;		/* w and h of typical digit */
	int x0, y0;		/* ul corner of box */
	int boxw, boxh;		/* overall box size */
	int stmag, ssmag, dsmag, magstp;
	int faintest;
	char buf[16];
	int dir, asc, dsc;
	XCharStruct all;
	GC gc;
	Obj o;
	int i;

	/* get size of a typical digit -- none have descenders */
	buf[0] = '3';	/* a typical wide digit :-) */
	XTextExtents (sv_pf, buf, 1, &dir, &asc, &dsc, &all);
	cw = all.width;
	ch = asc;	/* just numbers so no descenders */

	/* establish box height.
	 * use 3/2 vertical char spacing per symbol + 1 gap after each plus
	 * another half at the bottom for balance.
	 */
	boxh = (ch*3/2+1)*MAGBOXN + ch/2;

	/* set box width based on longest string to be displayed then add
	 * 1 cw for the dot, 1 for a gap, + 1/2 on each end 
	 */
	svf_getmaglimits (&stmag, &ssmag, &dsmag, &magstp);
	faintest = stmag;
	boxw = 0;
	for (i = 0; i < MAGBOXN; i++) {
	    int sw;

	    (void) sprintf (buf, "%d", faintest - i*magstp);
	    sw = strlen(buf)*cw;
	    if (sw > boxw)
		boxw = sw;
	}
	boxw += 3*cw;

	/* position in lower left corner with a small gap for fun */
	x0 = 5;
	y0 = sv_h - boxh - 5;

	/* draw a solid sky box with a bg border.
	 * TODO: fix printing kludge
	 */
	if (si_ison())
	    XPSPaperColor (bg_p);
	XSetForeground (dsp, sv_gc, sky_p);
	XPSFillRectangle (dsp, win, sv_gc, x0, y0, boxw, boxh);
	if (si_ison())
	    XPSPaperColor (sky_p);
	XSetForeground (dsp, sv_gc, bg_p);
	XPSDrawRectangle (dsp, win, sv_gc, x0, y0, boxw-1, boxh-1);

	/* ok, draw the scale */
	memset (&o, 0, sizeof(o));
	o.o_type = FIXED;
	o.f_class = 'S';
	o.f_spect[0] = 'G';
	o.s_size = 0;
	for (i = 0; i < MAGBOXN; i++) {
	    int d, y;

	    set_smag (&o, faintest - i*magstp);
	    d = objdiam(&o);
	    objGC (&o, d, &gc);
	    y = y0 + (ch*3/2 + 1)*(i+1);
	    sv_draw_obj (dsp, win, gc, &o, x0 + cw, y - ch/2, d, 1);
	    (void) sprintf (buf, "%g", get_mag(&o));
	    XPSDrawString (dsp, win, gc, x0 + 5*cw/2, y, buf, strlen(buf));
	}
	sv_draw_obj (dsp, win, (GC)0, NULL, 0, 0, 0, 0);	/* flush */
}

/* copy the pixmap to the drawing area and add zoom box if on.
 */
static void
sv_copy_sky()
{
	Display *dsp = XtDisplay (svda_w);
	Window win = XtWindow (svda_w);

	if (sv_pm) {
	    XCopyArea (dsp, sv_pm, win, sv_gc, 0, 0, sv_w, sv_h, 0, 0);
	    if (svtb_iszoomok())
		zm_draw();
	    sv_tmon = 0;
	}
}

/* called when there is mouse activity over the drawing area */
/* ARGSUSED */
static void
sv_da_ptr_eh (w, client, ev, continue_to_dispatch)
Widget w;
XtPointer client;
XEvent *ev;
Boolean *continue_to_dispatch;
{
	Now *np = mm_get_now();
	Display *dsp = XtDisplay(w);
	Window win = XtWindow(w);
	double altdec, azra;
	Window root, child;
	int rx, ry, wx, wy;
	unsigned mask;
	int evt = ev->type;
	int mo, bp, br, en, lv;
	int m1, b1p, b1r;
	int m2, b2p, b2r;
	int     b3p, b3r;
	int inwin, inside;

	/* what happened? */
	en  = evt == EnterNotify;
	lv  = evt == LeaveNotify;
	mo  = evt == MotionNotify;
	bp  = evt == ButtonPress;
	br  = evt == ButtonRelease;
	m1  = mo && ev->xmotion.state & Button1Mask;
	m2  = mo && ev->xmotion.state & Button2Mask;
	b1p = bp && ev->xbutton.button == Button1;
	b1r = br && ev->xbutton.button == Button1;
	b2p = bp && ev->xbutton.button == Button2;
	b2r = br && ev->xbutton.button == Button2;
	b3p = bp && ev->xbutton.button == Button3;
	b3r = br && ev->xbutton.button == Button3;

	/* where are we?
	 * N.B. can't depend on en/lv for inwin if moving fast 
	 */
	XQueryPointer (dsp, win, &root, &child, &rx, &ry, &wx, &wy, &mask);
	inwin = wx>=0 && wx<sv_w && wy>=0 && wy<sv_h;
	inside = inwin && sv_unloc (wx, wy, &altdec, &azra);

	/* what happened? */
	if (b3p) {
	    sv_pick (ev, wx, wy);
	} else if (b3r) {
	    return;
	} else if ((b2p || m2 || b2r) && !si_ison()) {
	    /* handle cursor-based dragging */

	    if (inside) {
		static int lastx, lasty;
		static Cursor hc;

		if (!hc)
		    hc = XCreateFontCursor (dsp, XC_fleur);

		if (b2p) {
		    lastx = wx;
		    lasty = wy;
		    XDefineCursor (dsp, win, hc);
		}

		/* don't draw on b2r if live dragging -- tends to lurch */
		if ((!want_livedrag && b2r) || (want_livedrag && m2)) {
		    /* compute new center */
		    double dar;

		    sv_altdec -= sv_vfov*(lasty - wy)/sv_h*(flip_tb?-1:1);
		    if (sv_altdec > PI/2)
			sv_altdec = PI/2;
		    if (sv_altdec < -PI/2)
			sv_altdec = -PI/2;

		    dar = sv_vfov*(wx - lastx)/sv_h*(flip_lr?-1:1);
		    if (fabs(altdec) < PI/2)
			dar /= cos(altdec);
		    sv_azra -= aa_mode ? dar : -dar;
		    range (&sv_azra, 2*PI);

		    lastx = wx;
		    lasty = wy;

		    /* update scales, redraw */
		    sv_set_scale (ALTDEC_S, 1);
		    sv_set_scale (AZRA_S, 1);
		    sv_all(np);

		    /* turn off following remote inputs when dragging */
		    XmToggleButtonSetState (keeptelvis_w, False, 1);
		}
	    }

	    if (b2r)
		XUndefineCursor (dsp, win);
	} else {
	    int sx, sy;

	    if (b1p && inwin && ir_setting()) {
		double ix, iy;
		sv_win2im (wx, wy, &ix, &iy);
		ir_setstar (ix, iy);
		return;
	    }

	    if (hznDrawing() && inside && (b1p || m1 || b1r)) {
		/* drawing new horizon -- temp suppress other mouse actions */
		double alt = altdec, az = azra;
		if (!aa_mode)
		    sv_other (altdec, azra, 0, &alt, &az);
		if (b1p) {
		    XSetFunction (dsp, sv_gc, GXxor);
		    hznRawProfile(1);
		}
		draw_hznProfile (dsp, win, sv_gc);
		hznAdd (b1p, alt, az);
		sv_draw_report_coords (b1p, 0, wx, wy, altdec, azra);
		draw_hznProfile (dsp, win, sv_gc);
		if (b1r) {
		    XSetFunction (dsp, sv_gc, GXcopy);
		    hznRawProfile(0);
		    sv_all(NULL);
		}
		return;
	    }

	    /* modify cursor pos to closest object if snap is on */
	    if (inwin && svtb_snapIsOn())
		si_findSnap (sv_w, sv_h, wx, wy, flip_lr, flip_tb, &sx,&sy);
	    else {
		sx = wx;
		sy = wy;
	    }

	    /* region of interest.
	     * erase before glass, if any
	     */
	    if (inwin && svtb_ROIIsOn()) {
		if (b1p) {
		    /* erase current, if any */
		    if (svtb_iszoomok())
			zm_draw();

		    /* discard subsequent undos, init new location, show hope */
		    zm_cutundo();
		    wzm.x0 = wzm.x1 = sx;
		    wzm.y0 = wzm.y1 = sy;
		    svtb_zoomok(1);
		} else if (m1) {
		    /* erase old */
		    zm_draw();
		}
	    }

	    /* show coords if on or dragging */
	    if (inside && (svtb_reportIsOn() || (b1p||m1)))
		sv_draw_report_coords (b1p, (b1p||m1) && svtb_ROIIsOn(),
							wx, wy, altdec, azra);

	    /* maybe glass, slice and photom */
	    if (inwin && si_ison()) {
		    
		/* area-based glass between erase/draw of line-oriented ops */
		if (svtb_sliceIsOn()) {
		    if (en && (mask & Button1Mask))
			si_doSlice (dsp, win, 2, sv_w, sv_h, sx, sy,
							    flip_lr, flip_tb);
		    else if (m1)
			si_doSlice (dsp, win, 0, sv_w, sv_h, sx, sy,
							    flip_lr, flip_tb);
		}
		if (svtb_glassIsOn() && (b1p || m1)) {
		    si_doGlass (dsp, win, b1p, m1, sv_w, sv_h, wx, wy, sx, sy,
							    flip_lr, flip_tb);
		    XDefineCursor (XtD, XtWindow(svda_w), magglass_cursor);
		}
		if (svtb_sliceIsOn() && (b1p || m1))
		    si_doSlice (dsp, win, b1p?-1:1, sv_w, sv_h, sx, sy,
							    flip_lr, flip_tb);

		if (svtb_gaussIsOn() && (b1p || m1))
		    si_doGauss (dsp, sv_w, sv_h, sx, sy, flip_lr,flip_tb);
	    }

	    /* clean stuff off when finished or leave */
	    if (b1r || lv) {
		if (svtb_glassIsOn() || svtb_sliceIsOn()) {
		    sv_copy_sky();
		    XUndefineCursor(XtD, XtWindow(svda_w));
		} else if (lv || !svtb_reportIsOn())
		    sv_erase_report_coords ();
	    } else if (!inwin)
		sv_erase_report_coords ();

	    /* draw ROI after glass, if any */
	    if (svtb_ROIIsOn()) {
		if (inwin && m1) {
		    /* draw new */
		    wzm.x1 = sx;
		    wzm.y1 = sy;
		    zm_draw();
		    if (si_ison())
			si_doROI (dsp, sv_w, sv_h, flip_lr, flip_tb, &wzm);
		}

		if (b1r) {
		    /* no area no box */
		    svtb_zoomok (wzm.x0 != wzm.x1 && wzm.y0 != wzm.y1);
		}
	    }
	}
}

/* erase the tracking coords by copying back from sv_pm */
static void
sv_erase_report_coords ()
{
	Display *dsp = XtDisplay(svda_w);
	Window win = XtWindow(svda_w);
	int x, y;

	/* beware being called after sv_pm is freed during closing */
	if (!sv_pm)
	    return;

	x = TBORD;
	y = TBORD;
	XCopyArea (dsp,sv_pm,win,sv_gc,x,y,trk_w,trk_h,x,y);

	x = sv_w-TBORD-trk_w;
	y = TBORD;
	XCopyArea (dsp,sv_pm,win,sv_gc,x,y,trk_w,trk_h,x,y);
}

/* a dot has been picked:
 * find what it is and report stuff about it in a popup.
 */
/* ARGSUSED */
static void
sv_pick (XEvent *ev, int wx, int wy)
{
#define	PICKRANGE	10	/* dist allowed from pointer */
	double csep, maxcsep;	/* cos(separation) and max so far */
	double altdec, azra;	/* cursor location */
	double saltdec, caltdec;/* sin/cos altdec */
	double ad, ar;		/* candidate location */
	TSky *mintsp = NULL;	/* set to a TSky if find one close */
	Obj *minop = NULL;	/* set to an Obj if find one closer */
	DBScan dbs;
	TrailObj *top;
	int x, y;
	Obj *op;
	int i;

	/* find world coord of cursor -- don't proceed if outside the svda_w */
	if (!sv_unloc (wx, wy, &altdec, &azra))
	    return;

	/* ok, own up to some work in progress */
	watch_cursor(1);

	/* find object with largest cos from current pos */
	caltdec = cos(altdec);
	saltdec = sin(altdec);
	maxcsep = 0.0;

	/* search the trailed stuff first */
	for (top = trailobj; top; top = top->ntop) {
	    if (!top->on)
		continue;
	    for (i = 0; i < top->nsky; i++) {
		op = &top->sky[i].o;
		ad = aa_mode ? op->s_alt : op->s_dec;
		ar = aa_mode ? op->s_az  : op->s_ra;
		csep = saltdec*sin(ad) + caltdec*cos(ad)*cos(azra-ar);
		if (csep > maxcsep) {
		    mintsp = &top->sky[i];
		    maxcsep = csep;
		}
	    }
	}

	/* search the database too -- might be something closer */
	for (db_scaninit(&dbs, ALLM, want_fs ? fldstars : NULL, nfldstars);
					    (op = db_scan (&dbs)) != NULL; ) {
	    if (!(op->o_flags & OBJF_ONSCREEN))
		continue;
	    ad = aa_mode ? op->s_alt : op->s_dec;
	    ar = aa_mode ? op->s_az  : op->s_ra;
	    csep = saltdec*sin(ad) + caltdec*cos(ad)*cos(azra-ar);
	    if (csep > maxcsep) {
		minop = op;
		maxcsep = csep;
	    }
	}

	/* show info about closest object, if any near, else generic */
	if (minop || mintsp) {
	    op = minop ? minop : &mintsp->o;
	    ad = aa_mode ? op->s_alt : op->s_dec;
	    ar = aa_mode ? op->s_az : op->s_ra;
	    if (sv_loc (ad, ar, &x, &y) && abs(x-wx) <= PICKRANGE
						    && abs(y-wy) <= PICKRANGE)
		sv_popup (ev, minop, minop ? NULL : mintsp);
	    else
		sv_popup (ev, NULL, NULL);
	} else
	    sv_popup (ev, NULL, NULL);

	watch_cursor(0);
}

/* callback when any of the scales change value.
 * client is a SCALES enum to tell us which.
 */
/* ARGSUSED */
static void
sv_scale_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmScaleCallbackStruct *s = (XmScaleCallbackStruct *)call;
	int which = (long int) client;

	/* finish up then avoid runaway auto repeat */
	XmUpdateDisplay (svda_w);
	XSync (XtD, True);

	/* disables any image display */
	si_off();

	/* read the widget to set the new value */
	sv_read_scale (which);
	sv_set_scale (which, 1);

	/* only do the map work if this is the final value or want live drag */
	if (want_livedrag || s->reason == XmCR_VALUE_CHANGED) {
	    /* update the map */
	    sv_all(mm_get_now());

	    /* turn off following remote inputs when told explicitly to point */
	    XmToggleButtonSetState (keeptelvis_w, False, 1);
	}
}

/* callback to load a new constellation figure file.
 * file basename is in label of this widget
 */
static void
sv_loadcnsfigs_cb (Widget w, XtPointer client, XtPointer call)
{
	char msg[1024];
	char *fn;
	FILE *fp;

	get_xmstring (w, XmNlabelString, &fn);
	fp = fopend (fn, "auxil", "r");		/* calls xe_msg() if fails */
	if (fp) {
	    if (cns_loadfigs (fp, msg) < 0)
		xe_msg (1, "%s:\n%s", fn, msg);
	    else {
		setXRes (cns_ffres, fn);
		if (want_conf)
		    sv_all(mm_get_now());
	    }
	    fclose (fp);
	}
	XtFree (fn);
}

/* callback when any of the Name label TB's change state.
 * client points to a state variable that should track the new state.
 */
/* ARGSUSED */
static void
sv_lbln_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int *lp = (int *)client;

	/* update flag */
	if (XmToggleButtonGetState(w))
	    *lp |= OBJF_NLABEL;
	else
	    *lp &= ~OBJF_NLABEL;

	/* update tollbar */
	svtb_updateNames (lbl_lst || lbl_lfs || lbl_lss || lbl_lds);

	/* redraw everything to pick up the new option state */
	sv_all(mm_get_now());
}

/* callback when any of the Magnitude label TB's change state.
 * client points to a state variable that should track the new state.
 */
/* ARGSUSED */
static void
sv_lblm_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int *lp = (int *)client;

	if (XmToggleButtonGetState(w))
	    *lp |= OBJF_MLABEL;
	else
	    *lp &= ~OBJF_MLABEL;

	/* update tollbar */
	svtb_updateNames (lbl_lst || lbl_lfs || lbl_lss || lbl_lds);

	/* redraw everything to pick up the new option state */
	sv_all(mm_get_now());
}

/* callback when any of the options toggle buttons change state that just
 *   causes a redraw.
 * client points to a state variable that should track the new state.
 */
/* ARGSUSED */
static void
sv_option_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int *p = (int *)client;

	/* update the flag for sure */
	*p = XmToggleButtonGetState (w);

	/* load fstars immediately, rather than wait for timer */
	if (p == &want_fs && want_fs)
	    sv_loadfs(0);

	/* update any corresponding toolbar */
	svtb_sync();

	/* insure horizon editing off if hzn being turn off */
	if (p == &want_hznmap && !want_hznmap)
	    hznEditingOff();

	/* capture manual user desire about automag.
	 * also, no need to redraw if just turning it off.
	 */
	if (p == &want_automag) {
	    user_automagoff = !want_automag;
	    if (want_automag)
		svf_automag(sv_vfov);
	    else
		return;
	}

	/* no need to do anything if changing live-dragging option */
	if (p == &want_livedrag)
	    return;

	/* special consideration for turning on eyepieces when there aren't
	 * any now
	 */
	if (p == &want_eyep && want_eyep) {
	    if (se_getlist (NULL) == 0)
		return;
	}

	/* special consideration for making constellation buttons work like
	 * a radio pair but ok if both off
	 */
	if (p == &want_cona && *p && want_conn) {
	    XmToggleButtonSetState (conn_w, False, True);	/* draws */
	    return;
	}
	if (p == &want_conn && *p && want_cona) {
	    XmToggleButtonSetState (cona_w, False, True);	/* draws */
	    return;
	}

	/* if flipping, update undoes and tell FITS to make a new pixmap */
	if (p == &flip_lr || p == &flip_tb) {
	    zm_flip (p == &flip_lr);
	    if (si_ison())
		si_newPixmap (sv_w, sv_h, flip_lr, flip_tb, zm_undo, zm_cundo);
	}

	/* redraw everything to pick up the new option state */
	sv_all(mm_get_now());
}

/* called to bring up the eyepiece control dialog */
/* ARGSUSED */
static void
sv_eyep_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	se_manage();
}

/* called to bring up the binary star orbit map */
/* ARGSUSED */
static void
sv_bsmap_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	svbs_manage(pu.op);
}

/* callback just before the "Favorite" menu comes up.
 * fill with the current loaded Favorites
 * w is the pulldown menu.
 */
/* ARGSUSED */
static void
sv_faving_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Obj **favs;
	int nfavs = fav_get_loaded (&favs);
	Cardinal npb;
	WidgetList pbl;
	int i;

	/* get current set of push buttons */
	get_something (w, XmNnumChildren, (XtArgVal)&npb);
	get_something (w, XmNchildren, (XtArgVal)&pbl);

	/* make sure there are at least nfavs buttons */
	for (i = 0; i < nfavs; i++) {
	    Widget pbw;
	    if (i < npb)
		pbw = pbl[i];
	    else {
		pbw = XmCreatePushButton (w, "FPB", NULL, 0);
		XtAddCallback (pbw, XmNactivateCallback,sv_fav_cb,(XtPointer)(long int)i);
	    }
	    set_xmstring (pbw, XmNlabelString, favs[i]->o_name);
	    XtManageChild (pbw);
	}

	/* unmanage any extra */
	for (; i < npb; i++)
	    XtUnmanageChild (pbl[i]);
}

/* callback when one of the favorites cascade buttons changes state.
 * client is index into favs list.
 */
/* ARGSUSED */
static void
sv_fav_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XEvent *ev = ((XmAnyCallbackStruct*)call)->event;
	Obj **favs, *op;
	int nfavs = fav_get_loaded (&favs);
	int i = (long int)client;
	int dax, day;
	double altdec, azra;
	Window cw;

	if (i < 0 || i >= nfavs) {
	    printf ("SV bogus favorite %d of %d\n", i, nfavs);
	    abort();
	}

	/* turn off following remote inputs when told explicitly to point */
	XmToggleButtonSetState (keeptelvis_w, False, 1);

	/* put up the same popup as if user right-clicked on the favorite */
	op = favs[i];
	db_update (op);
	sv_mark (op, 0, 1, 1, 0, 0, 0.0);
	altdec = aa_mode ? op->s_alt : op->s_dec;
	azra   = aa_mode ? op->s_az  : op->s_ra;
	sv_loc (altdec, azra, &dax, &day);
	XTranslateCoordinates (XtD,
	    XtWindow(svda_w), 
	    RootWindow (XtD, DefaultScreen(XtD)),
	    dax, day, 
	    &ev->xbutton.x_root, &ev->xbutton.y_root,
	    &cw);
	sv_popup ((XEvent *)ev, op, NULL);
}

/* given one of the SCALES enums, read it and compute the corresponding 
 * real value.
 */
static void
sv_read_scale (which)
int which;
{
	int i;

	switch (which) {
	case FOV_S:
	    XmScaleGetValue (fov_w, &i);
	    sv_set_fov(degrad(i/(60./FOV_STEP)));
	    break;
	case ALTDEC_S:
	    XmScaleGetValue (altdec_w, &i);
	    sv_altdec = degrad(i/(60./FOV_STEP));
	    break;
	case AZRA_S:
	    XmScaleGetValue (azra_w, &i);
	    sv_azra = degrad(i/(60./FOV_STEP));
	    break;
	default:
	    printf ("sv_read_scale Bug! bad which: %d\n", which);
	    abort();
	}
}

/* load fresh set of field stars if new circumstances and yet stable. 
 * base mag limit on value of near-sky mag limit.
 * we are called every FSTO to monitor for change and steady state.
 * set force to disregard tests for stable conditions.
 */
void
sv_loadfs(force)
int force;
{
	static double lcfov, lcazra, lcaltdec;		/* last call */
	static double lsmjd, lsfov, lsra, lsdec;	/* last shown */
	static int lsmag;
	Now *np = mm_get_now();
	double alt, az, ra, dec;
	int chging;
	int stmag, ssmag, dsmag, magstp;
	double epsfov;

	/* check some basic conditions */
	if (!sv_ison())
	    return;

	/* wait until things are not changing and meet basic requirements */
	chging = lcazra!=sv_azra || lcaltdec!=sv_altdec || lcfov!=sv_dfov;
	lcazra = sv_azra;
	lcaltdec = sv_altdec;
	lcfov = sv_dfov;
	if ((!force && chging) || !want_fs)
	    return;

	/* get current display values */
	sv_fullwhere (np, sv_altdec, sv_azra, aa_mode, &alt, &az, &ra, &dec);
	svf_getmaglimits (&stmag, &ssmag, &dsmag, &magstp);

	/* skip unless forced if nearly the same as last time displayed */
	epsfov = sv_dfov/10.;				/* "small" fov change */
	if (!force  && fabs(lsdec-dec) < epsfov		/* small dec move */
		    && delra(lsra-ra)*cos(dec) < epsfov /* small ra move */
		    && fabs(lsfov-sv_dfov) < epsfov	/* small zoom */
		    && lsmag >= stmag			/* brighter limit */
		    && (!fs_pmon() || fabs(lsmjd-mjd)<MAXPMOK)) /*small propmo*/
	    return;

	/* remove any existing stars */
	if (fldstars) {
	    free ((void *)fldstars);
	    fldstars = NULL;
	    nfldstars = 0;
	    tobj_newdb();	/* in case a FS was tracked or trailed */
	}

	/* no reload if fov now too large */
	if (sv_dfov > degrad(MAXFSFOV))
	    return;

	/* ok, now we can go */

	watch_cursor(1);
	nfldstars = fs_fetch (np, ra, dec, sv_dfov, (double)stmag, &fldstars);
	lsra = ra;
	lsdec = dec;
	lsfov = sv_dfov;
	lsmag = stmag;
	lsmjd = mjd;

	/* update the state and its PB */
	if (nfldstars < 0) {
	    /* fs_fetch() has already put up the error message */
	    XmToggleButtonSetState (wantfs_w, False, False);
	    want_fs = 0;
	    svtb_updateFStars (want_fs);
	    nfldstars = 0;
	} else if (nfldstars > 0) {
	    char rastr[32], decstr[32];
	    fs_sexa (rastr, radhr(ra), 2, 36000);
	    fs_sexa (decstr, raddeg(dec), 3, 3600);
	    xe_msg (0, "Loaded %5d field stars to mag %2d @ %s %s diam %g deg",
			    nfldstars, stmag, rastr, decstr, raddeg(sv_dfov));
	    XmToggleButtonSetState (wantfs_w, True, False);
	    want_fs = 1;
	    svtb_updateFStars (want_fs);
	}

	/* show */
	sv_all (np);
	watch_cursor(0);
}

/* make sure trk_pm is at least minw x minh */
static void
sv_track_pm (dsp, win, minw, minh)
Display *dsp;
Window win;
int minw, minh;
{
	Window root;
	unsigned int bw, d;
	unsigned int wid, hei;
	int x, y;

	if (trk_pm) {
	    if ((int)trk_w >= minw && (int)trk_h >= minh)
		return;
	    XFreePixmap (dsp, trk_pm);
	}

	trk_w = (unsigned int) minw;
	trk_h = (unsigned int) minh;
	XGetGeometry (dsp, win, &root, &x, &y, &wid, &hei, &bw, &d);
	trk_pm = XCreatePixmap (dsp, win, trk_w, trk_h, d);
}

/* draw all the stuff in the coord tracking areas in the corners of the drawing
 *   area.
 * if new, reset the angular reference.
 * if sep, show separations from angular reference, else absolute coords.
 * N.B. draw coords directly to svda_w, *NOT* sv_pm, so it can be erased by
 *   just copying sv_pm to svda_w again.
 */
static void
sv_draw_report_coords (new, sep, winx, winy, altdec, azra)
int new, sep;
int winx, winy;
double altdec, azra;
{
#define	NTLAB		12			/* /even/ number of labels */
	static double start_altdec, start_azra;	/* angular reference */
	static double start_gl, start_gL;	/*   " galactic */
	static double start_pa;			/* parallactic angle */
	static int maxrw;			/* right width, never shrinks */
	Now *np = mm_get_now();
	Window win = XtWindow(svda_w);
	Display *dsp = XtDisplay(svda_w);
	double alt, az, asra, ra, ha, asdec, dec, e, gl, gL, pa;
	char alt_buf[64], zen_buf[64], az_buf[64];
	char ra_buf[64], ha_buf[64], dec_buf[64];
	char gl_buf[64], gL_buf[64];
	char sep_buf[64];
	char fits_buf[64];
	char pa_buf[64];
	char *cns;
	char *strp[NTLAB];
	int maxlw, maxh;
	double ca;
	int asc;
	int i;

	/* alt/az and ra/dec regardless of current display mode */
	sv_fullwhere (np, altdec, azra, aa_mode, &alt, &az, &ra, &dec);

	/* gal coords */
	asra = ra;
	asdec = dec;
	e = epoch == EOD ? mjd : epoch;
	if (epoch == EOD)
	    ap_as (np, mjd, &asra, &asdec);
	eq_gal (e, asra, asdec, &gl, &gL);

	/* HA and Parallactic angle */
	radec2ha (np, ra, dec, &ha);
	pa = parallacticLHD (lat, ha, dec);

	/* save if new reference */
	if (new) {
	    start_altdec = altdec;
	    start_azra = azra;
	    start_gl = gl;
	    start_gL = gL;
	    start_pa = pa;
	}

	/* fill in each string.
	 * N.B. later we assume the left set is always wider than the right
	 */
	if (sep) {
	    double salt, saz, sra, sdec, dx;
	    sv_fullwhere (np, start_altdec, start_azra, aa_mode,
						    &salt, &saz, &sra, &sdec);
	    (void) strcpy (alt_buf, "dAlt:");
	    dx = raddeg(alt-salt);
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (alt_buf+5, dx, 5, 60);
	    else
		fs_sexa (alt_buf+5, dx, 5, 3600);

	    (void) strcpy (zen_buf, "dZD: ");
	    dx = raddeg(salt-alt);
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (zen_buf+5, dx, 5, 60);
	    else
		fs_sexa (zen_buf+5, dx, 5, 3600);

	    (void) strcpy (az_buf,  "dAz: ");
	    dx = raddeg(az-saz)+180.;
	    range (&dx, 360.0);
	    dx -= 180.;
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (az_buf+5, dx, 5, 60);
	    else
		fs_sexa (az_buf+5, dx, 5, 3600);

	    (void) strcpy (gl_buf,  "dGLat:");
	    dx = raddeg(gl-start_gl)+180;
	    range (&dx, 360.0);
	    dx -= 180.;
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (gl_buf+6, dx, 4, 60);
	    else
		fs_sexa (gl_buf+6, dx, 4, 3600);

	    (void) strcpy (gL_buf, "dGLng:");
	    dx = raddeg(gL-start_gL);
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (gL_buf+6, dx, 4, 60);
	    else
		fs_sexa (gL_buf+6, dx, 4, 3600);

	    (void) strcpy (ra_buf,  "dRA:");
	    dx = radhr(ra-sra)+12.;
	    range (&dx, 24.0);
	    dx -= 12.;
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (ra_buf+4, dx, 6, 600);
	    else
		fs_sexa (ra_buf+4, dx, 6, 360000);

	    (void) strcpy (ha_buf,  "dHA:");
	    dx = radhr(sra-ra)+12.;
	    range (&dx, 24.0);
	    dx -= 12.;
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (ha_buf+4, dx, 6, 600);
	    else
		fs_sexa (ha_buf+4, dx, 6, 360000);

	    (void) strcpy (dec_buf, "dDec:");
	    dx = raddeg(dec-sdec);
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (dec_buf+5, dx, 5, 600);
	    else
		fs_sexa (dec_buf+5, dx, 5, 36000);

	    (void) strcpy (pa_buf,  "dPA:  ");
	    dx = raddeg(pa - start_pa);
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (pa_buf+4, dx, 6, 60);
	    else
		fs_sexa (pa_buf+4, dx, 6, 3600);

	} else {
	    (void) strcpy (alt_buf, "Alt:   ");
	    fs_pangle (alt_buf+7, alt);
	    (void) strcpy (zen_buf, "ZD:    ");
	    fs_pangle (zen_buf+7, PI/2 - alt);
	    (void) strcpy (az_buf,  "Az:    ");
	    fs_pangle (az_buf+7, az);
	    (void) strcpy (ra_buf,  "RA:     ");
	    fs_ra (ra_buf+7, ra);
	    (void) strcpy (ha_buf,  "HA:    ");
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (ha_buf+6, radhr(ha), 3, 600);
	    else
		fs_sexa (ha_buf+6, radhr(ha), 3, 360000);
	    (void) strcpy (dec_buf, "Dec:   ");
	    fs_prdec (dec_buf+6, dec);

	    gl = raddeg(gl);
	    gL = raddeg(gL);
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC) {
		fs_sexa(gl_buf+strlen(strcpy(gl_buf, "GLat:  ")),gl,3,60);
		fs_sexa(gL_buf+strlen(strcpy(gL_buf, "GLong: ")),gL,3,60);
	    } else {
		fs_sexa(gl_buf+strlen(strcpy(gl_buf, "GLat:  ")),gl,3,3600);
		fs_sexa(gL_buf+strlen(strcpy(gL_buf, "GLong: ")),gL,3,3600);
	    }

	    (void) strcpy (pa_buf,  "PA:    ");
	    if (pref_get(PREF_DPYPREC) == PREF_LOPREC)
		fs_sexa (pa_buf+6, raddeg(pa), 4, 60);
	    else
		fs_sexa (pa_buf+6, raddeg(pa), 4, 3600);
	}
	cns = cns_name(cns_pick (ra, dec, epoch==EOD ? mjd : epoch));
	cns += 5;	/* skip the abbreviation */
	solve_sphere (azra - start_azra, PI/2-start_altdec, sin(altdec),
						    cos(altdec), &ca, &pa);
	(void) strcpy (sep_buf, "Sep:  ");
	fs_pangle (sep_buf+6, acos(ca));
	sprintf (sep_buf+strlen(sep_buf), "@%.2f",
			fmod(540-raddeg(pa),360.0));

	fits_buf[0] = '\0';
	if (si_ison()) {
	    FImage *fip = si_getFImage();
	    double ix, iy;
	    int p;
	    sv_win2im (winx, winy, &ix, &iy);
	    p = ((CamPix *)fip->image)[(int)(fip->sw*(int)(iy + .5) + ix + .5)];
	    sprintf (fits_buf, "%5d @ %6.1f %6.1f", p, ix, iy);
	}

	/* use the tracking font */
	XSetFont (dsp, sv_strgc, sv_tf->fid);

	/* assign strings, down left side then down right */
	strp[0] = ra_buf;
	strp[1] = ha_buf;
	strp[2] = dec_buf;
	strp[3] = sep_buf;
	strp[4] = cns;
	strp[5] = fits_buf;

	strp[6] = alt_buf;
	strp[7] = zen_buf;
	strp[8] = az_buf;
	strp[9] = gl_buf;
	strp[10] = gL_buf;
	strp[11] = pa_buf;

	/* find largest string in each side */
	maxlw = maxh = 0;
	for (i = 0; i < NTLAB; i++) {
	    char *sp = strp[i];
	    int l = strlen (sp);
	    XCharStruct all;
	    int dir, des;

	    XTextExtents (sv_tf, sp, l, &dir, &asc, &des, &all);
	    if (i < NTLAB/2) {
		if (all.width > maxlw)
		    maxlw = all.width;
	    } else {
		if (all.width > maxrw)
		    maxrw = all.width;
	    }
	    if (asc + des > maxh)
		maxh = asc + des;
	}

	/* insure trk_pm is large enough, (re)creating if necessary.
	 * N.B. we assume the left half is always widest, right highest
	 */
	sv_track_pm (dsp, win, maxlw, ((NTLAB+1)/2)*maxh);

	/* erase and draw left side */
	XSetForeground (dsp, sv_strgc, sky_p);
	XFillRectangle (dsp, trk_pm, sv_strgc, 0, 0, trk_w, trk_h);
	XSetForeground (dsp, sv_strgc, annot_p);
	for (i = 0; i < NTLAB/2; i++) {
	    char *sp = strp[i];
	    int l = strlen (sp);
	    int y = asc + i*maxh;
	    XDrawString (dsp, trk_pm, sv_strgc, 0, y, sp, l); 
	}
	XCopyArea (dsp,trk_pm,win,sv_strgc,0,0,trk_w,trk_h,TBORD,TBORD);

	/* erase and draw right side */
	XSetForeground (dsp, sv_strgc, sky_p);
	XFillRectangle (dsp, trk_pm, sv_strgc, 0, 0, trk_w, trk_h);
	XSetForeground (dsp, sv_strgc, annot_p);
	for (i = NTLAB/2; i < NTLAB; i++) {
	    char *sp = strp[i];
	    int l = strlen (sp);
	    int y = asc + (i-NTLAB/2)*maxh;
	    XDrawString (dsp, trk_pm, sv_strgc, 0, y, sp, l); 
	}
	XCopyArea (dsp, trk_pm, win, sv_strgc, 0, 0, maxrw, trk_h,
						    sv_w-TBORD-maxrw, TBORD);

#undef NTLAB
}

/* draw the transient window marker at [x,y] using sv_tmgc */
static void
sv_drawtm (dsp, win, x, y)
Display *dsp;
Window win;
int x, y;
{
	int diag = 707*MARKR/1000;

	/* draw window marker */
	XDrawLine (dsp, win, sv_tmgc, x-diag, y-diag, x-diag/2, y-diag/2);
	XDrawLine (dsp, win, sv_tmgc, x+diag, y-diag, x+diag/2, y-diag/2);
	XDrawLine (dsp, win, sv_tmgc, x+diag, y+diag, x+diag/2, y+diag/2);
	XDrawLine (dsp, win, sv_tmgc, x-diag, y+diag, x-diag/2, y+diag/2);
	XDrawArc (dsp, win, sv_tmgc, x-MARKR, y-MARKR, 2*MARKR, 2*MARKR,
								    0, 64*360);
	XDrawArc (dsp, win, sv_tmgc, x-MARKR/2, y-MARKR/2, MARKR, MARKR,
								    0, 64*360);
}

/* draw the stickier pixmap marker at [x,y] using sv_gc */
static void
sv_drawm (dsp, pm, x, y)
Display *dsp;
Pixmap pm;
int x, y;
{
	int diag = 707*MARKR/1000;

	/* draw window marker */
	XSetForeground (dsp, sv_gc, annot_p);
	XDrawLine (dsp, pm, sv_gc, x-diag, y-diag, x+diag, y+diag);
	XDrawLine (dsp, pm, sv_gc, x+diag, y-diag, x-diag, y+diag);
	XDrawArc (dsp, pm, sv_gc, x-MARKR, y-MARKR, 2*MARKR, 2*MARKR, 0,64*360);
}

/* mark the given object, possibly reaiming depending on the center flags.
 * then if newfov is not 0, change to new fov, silently enforcing limits.
 * return 0 if did all that was asked else generate a message using xe_msg(*,1)
 *   and return -1.
 * N.B. we do *not* update the s_ fields of op.
 */
static int
sv_mark (op, in_center, out_center, mark, xemsg, use_window, newfov)
Obj *op;
int in_center;	/* whether to center an object already within the fov */
int out_center;	/* whether to center an object not already within the fov */
int mark;	/* whether to mark the object if it ends up visible */
int xemsg;	/* whether to issue an xe_msg if object is out of view */
int use_window;	/* write directly to the window, not the pixmap (for speed) */
double newfov;	/* new fov if other conditions met. leave unchanged if == 0 */
{
	Display *dsp = XtDisplay (svda_w);
	Window win = XtWindow (svda_w);
	double altdec, azra;
	int onscrn;
	int x, y;

	/* see if it's within the current fov.
	 * maybe complain if it's not and we have no permission to recenter.
	 */
	altdec = aa_mode ? op->s_alt : op->s_dec;
	azra   = aa_mode ? op->s_az  : op->s_ra;
	onscrn = sv_loc (altdec, azra, &x, &y);
	if (!onscrn && !out_center) {
	    if (xemsg) {
		xe_msg (1, "`%.*s' is outside the field of view.\n",
							    MAXNM, op->o_name);
	    }
	    if (sv_tmon) {
		sv_drawtm (dsp, win, sv_tmx, sv_tmy);	/* erase */
		sv_tmon = 0;
	    }
	    return (-1);
	}

	/* recenter if we can/may */
	if ((onscrn && in_center) || (!onscrn && out_center)) {
	    si_off();
	    if (newfov != 0.0) {
		sv_set_fov (newfov);
		sv_set_scale(FOV_S, 1);
	    }
	    sv_altdec = altdec;
	    sv_set_scale (ALTDEC_S, 1);
	    sv_azra = azra;
	    sv_set_scale (AZRA_S, 1);
	    sv_all(mm_get_now());

	    /* get window loc at new view */
	    if (sv_loc (altdec, azra, &x, &y) < 0) {
		printf ("sv_mark bug! object disappeared from view?!\n");
		abort();
	    } 
	}

	/* and mark if asked to.
	 * by now we are certainly visible.
	 */
	if (mark) {
	    if (use_window) {
		/* transient marker drawn/erased; sv_copy_sky looks bad */
		if (sv_tmon)
		    sv_drawtm (dsp, win, sv_tmx, sv_tmy);	/* erase */
		sv_drawtm (dsp, win, x, y);
		sv_tmx = x;
		sv_tmy = y;
		sv_tmon = 1;
	    } else {
		/* transient marker drawn on pixmap to be stickier */
		sv_drawm (dsp, sv_pm, x, y);
		/* update */
		sv_copy_sky();
	    }

	    /* if want it, no buttons and off-screen then show marker coords */
	    if (svtb_reportIsOn()) {
		Window root, child;
		int rx, ry, wx, wy;
		unsigned mask;

		XQueryPointer (dsp,win,&root,&child,&rx,&ry,&wx,&wy,&mask);
		if (!mask) {
		    double caltdec, cazra;
		    if (!sv_unloc (wx, wy, &caltdec, &cazra))
			sv_draw_report_coords(0, 0, wx, wy, altdec, azra);
		}
	    }
	}

	return (0);
}

/* we are called when the right mouse button is pressed because the user wants
 * to identify and possibly control an object in some way. we fill in the popup
 * with goodies and manage it.
 * three cases:
 *   1) we are called with op set and tsp == 0. this means we were called with
 *      a real db object so just use *op;
 *   2) we are called with op == 0 and tsp set. this means we were called with
 *      a trailed object and so use *tsp->op;
 *   3) we are called with neither op nor tsp set. this means we were called
 *      far from any object; compute the location and display it. also set
 *      pu.op to a static FIXED object with basic info set for possible
 *      pointing.
 * position the popup as indicated by ev and display it.
 * it goes down by itself.
 */
static void
sv_popup (ev, op, tsp)
XEvent *ev;
Obj *op;
TSky *tsp;
{
	Now *np = mm_get_now();
	char buf[32], buf2[64];
	int noobj;
	int newfav = op && fav_already(op->o_name) < 0;
	int llabel = 0, rlabel = 0;
	int hastrail = 0, trailon = 0;
	int track = 0;
	double jd;

	/* create popup first time */
	if (!pu.pu_w)
	    sv_create_popup();

	/* save cursor coords at moment of popup */
	pu.wx = ev->xbutton.x;
	pu.wy = ev->xbutton.y;

	/* set op */
	if (tsp) {
	    /* we were given a trailed Obj.
	     * pu.op is the top object, pu.tsp is tsp.
	     */
	    TrailObj *top;

	    op = &tsp->o;
	    top = tobj_find (op);
	    jd = tsp->trts.t;
	    llabel = !!(tsp->flags & OBJF_LLABEL);
	    rlabel = !!(tsp->flags & OBJF_RLABEL);
	    track = top->op == track_op;
	    hastrail = 1;
	    trailon = 1;
	    pu.op = top->op;
	    pu.tsp = tsp;
	    noobj = 0;
	} else if (op) {
	    /* not called with tsp trail -- just op.
	     * pu.op is op, pu.tsp is NULL.
	     */
	    TrailObj *top = tobj_find(op);

	    jd = mjd;
	    llabel = !!(op->o_flags & OBJF_LLABEL);
	    rlabel = !!(op->o_flags & OBJF_RLABEL);
	    noobj = 0;
	    track = op == track_op;
	    hastrail = top ? 1 : 0;
	    trailon = hastrail && top->on;
	    pu.op = op;
	    pu.tsp = NULL;
	} else {
	    /* nothing -- compute from ev and use svobj
	     * pu.op and op are &svobj, pu.tsp is NULL.
	     */
	    double altdec, azra, alt, az, ra, dec;

	    if (!sv_unloc (pu.wx, pu.wy, &altdec, &azra))
		return;	/* outside the window */
	    sv_fullwhere (np, altdec, azra, aa_mode, &alt, &az, &ra, &dec);
	    jd = mjd;
	    /* some will need current state, some will need definition */
	    memset (&svobj, 0, sizeof(svobj));
	    svobj.o_type = FIXED;
	    (void) strcpy (svobj.o_name, telAnon);
	    svobj.f_RA = (float)ra;
	    svobj.f_dec = (float)dec;
	    svobj.f_epoch = (float)(epoch == EOD ? mjd : epoch);
	    svobj.s_ra = (float)ra;
	    svobj.s_dec = (float)dec;
	    svobj.s_az = (float)az;
	    svobj.s_alt = (float)alt;
	    pu.op = op = &svobj;
	    pu.tsp = NULL;
	    noobj = 1;
	}

	/* set up popup */

	/* prep info for BINARY */
	if (!noobj && is_type (op,BINARYSTARM)) {
	    op->b_2compute = 1;
	    obj_cir (np, op);
	}

	if (noobj) {
	    XtUnmanageChild (pu.name_w);
	    XtUnmanageChild (pu.altnm_w);
	    XtUnmanageChild (pu.desc_w);
	    XtUnmanageChild (pu.nmsep_w);
	    XtUnmanageChild (pu.infocb_w);
	} else {
	    /* display name(s) */
	    sv_punames(op);

	    /* display description */
	    set_xmstring (pu.desc_w, XmNlabelString, obj_description(op));
	    XtManageChild (pu.desc_w);
	    XtManageChild (pu.nmsep_w);
	    XtManageChild (pu.infocb_w);
	}

	if (!noobj && is_type(op, FIXEDM|BINARYSTARM) && op->f_spect[0]) {
	    char *label = "Spect";
	    int l;

	    if (is_type (op, FIXEDM)) {
		switch (op->f_class) {
		case 'T': case 'B': case 'D': case 'M': case 'S': case 'V':
		    break;
		default:
		    label = "Type";
		    break;
		}
	    }

	    l = sprintf (buf2, "%*s: %.*s", 6, label,
					(int)sizeof(op->f_spect), op->f_spect);
	    if (is_type(op, BINARYSTARM) && op->b_2spect[0])
		sprintf (buf2+l, ", %.*s", (int)sizeof(op->b_2spect),
								op->b_2spect);
	    set_xmstring (pu.spect_w, XmNlabelString, buf2);
	    XtManageChild (pu.spect_w);
	} else
	    XtUnmanageChild (pu.spect_w);

	if (!noobj && ((is_type(op,BINARYSTARM) && op->b_nbp == 0) ||
		((is_type(op,FIXEDM)||is_type(op,PLANETM)) && op->s_size>0))) {
	    char *szstr= is_type(op,BINARYSTARM) ? "Sep" : "Size";
	    double szv = is_type(op,BINARYSTARM) ? op->b_bo.bo_sep : op->s_size;
	    if (szv < 60)
		(void) sprintf (buf2, "%*s: %.1f\"", 6, szstr, szv);
	    else if (szv < 3600)
		(void) sprintf (buf2, "%*s: %.1f'", 6, szstr, szv/60.0);
	    else
		(void) sprintf (buf2, "%*s: %.2f°", 6, szstr, szv/3600.0);
	    set_xmstring (pu.size_w, XmNlabelString, buf2);
	    XtManageChild (pu.size_w);
	} else
	    XtUnmanageChild (pu.size_w);

	if (!noobj && is_type(op,EARTHSATM)) {
	    sprintf (buf, "Sunlit: %*s", 6, op->s_eclipsed ? "No" : "Yes");
	    set_xmstring (pu.eclipsed_w, XmNlabelString, buf);
	    XtManageChild (pu.eclipsed_w);
	} else
	    XtUnmanageChild (pu.eclipsed_w);

	if (!noobj && ((is_type(op,BINARYSTARM) && op->b_nbp == 0) ||
				    (is_type(op,FIXEDM) && op->f_pa>0) )) {
	    double pa = is_type(op,BINARYSTARM) ? op->b_bo.bo_pa : get_pa(op);
	    sprintf (buf, "%*s: %.0f° EofN", 6, "PA", raddeg(pa));
	    set_xmstring (pu.pa_w, XmNlabelString, buf);
	    XtManageChild (pu.pa_w);
	} else
	    XtUnmanageChild (pu.pa_w);

	if (ol_isUp() && !noobj && !tsp)
	    XtManageChild (pu.obslog_w);
	else
	    XtUnmanageChild (pu.obslog_w);

	if (noobj) {
	    XtUnmanageChild (pu.ud_w);
	    XtUnmanageChild (pu.ut_w);
	} else {
	    /* show in preferred zone to match rise/set */
	    double tzjd = pref_get(PREF_ZONE)==PREF_UTCTZ ? jd : jd-tz/24;

	    fs_date (buf, pref_get(PREF_DATE_FORMAT), mjd_day(tzjd));
	    (void) sprintf (buf2, "%*s:  %s", 5, "Date", buf);
	    set_xmstring (pu.ud_w, XmNlabelString, buf2);
	    XtManageChild (pu.ud_w);

	    fs_time (buf, mjd_hr(tzjd));
	    (void) sprintf (buf2, "%*s:  %s", 5, "Time", buf);
	    set_xmstring (pu.ut_w, XmNlabelString, buf2);
	    XtManageChild (pu.ut_w);
	}

	fs_ra (buf, op->s_ra);
	(void) sprintf (buf2, "%*s:  %s", 5, "RA", buf);
	set_xmstring (pu.ra_w, XmNlabelString, buf2);

	fs_prdec (buf, op->s_dec);
	(void) sprintf (buf2, "%*s: %s", 5, "Dec", buf);
	set_xmstring (pu.dec_w, XmNlabelString, buf2);

	fs_pangle (buf, op->s_alt);
	(void) sprintf (buf2, "%*s: %s", 6, "Alt", buf);
	set_xmstring (pu.alt_w, XmNlabelString, buf2);

	fs_pangle (buf, op->s_az);
	(void) sprintf (buf2, "%*s: %s", 6, "Az", buf);
	set_xmstring (pu.az_w, XmNlabelString, buf2);

	if (noobj) {
	    XtUnmanageChild (pu.mag_w);
	    XtUnmanageChild (pu.refmag_w);
	} else {
	    int l = sprintf (buf2, "%*s: %6.2f", 5, "Mag", get_mag(op));
	    if (is_type(op,BINARYSTARM))
		sprintf (buf2+l, ",%.2f", op->b_2mag/MAGSCALE);
	    set_xmstring (pu.mag_w, XmNlabelString, buf2);
	    XtManageChild (pu.mag_w);
	    if (si_ison() && si_isup() && svtb_gaussIsOn())
		XtManageChild (pu.refmag_w);
	    else
		XtUnmanageChild (pu.refmag_w);
	}

	if (tsp) {
	    Now n = *np;
	    n.n_mjd = jd;
	    sv_riset (&n, op);
	} else {
	    sv_riset (np, op);
	}

	if (telIsOn())
	    XtManageChild (pu.goto_w);
	else
	    XtUnmanageChild (pu.goto_w);

	if (noobj) {
	    XtUnmanageChild (pu.label_w);
	    XtUnmanageChild (pu.fav_w);
	    XtUnmanageChild (pu.track_w);
	    XtUnmanageChild (pu.trail_w);
	    XtUnmanageChild (pu.newtrail_w);
	} else {
	    XtManageChild (pu.label_w);
	    XmToggleButtonSetState (pu.llabel_w, llabel, False);
	    XmToggleButtonSetState (pu.rlabel_w, rlabel, False);
	    if (newfav)
		XtManageChild (pu.fav_w);
	    else
		XtUnmanageChild (pu.fav_w);  /* fs and trails not in real db */
	    XtManageChild (pu.track_w);
	    XmToggleButtonSetState (pu.track_w, track, False);
	    if (hastrail) {
		XtManageChild (pu.trail_w);
		XmToggleButtonSetState (pu.trail_w, trailon, False);
		set_xmstring (pu.newtrail_w, XmNlabelString, "Change Trail...");
	    } else {
		XtUnmanageChild (pu.trail_w);
		set_xmstring (pu.newtrail_w, XmNlabelString, "Create Trail...");
	    }
	    XtManageChild (pu.newtrail_w);
	}

	if ((aa_mode && se_isOneHere (1, op->s_az, op->s_alt))
			|| (!aa_mode && se_isOneHere (0, op->s_ra, op->s_dec)))
	    XtManageChild (pu.deleyep_w);
	else
	    XtUnmanageChild (pu.deleyep_w);

	if (noobj || gal_opfind (op) < 0)
	    XtUnmanageChild (pu.gal_w);
	else
	    XtManageChild (pu.gal_w);
	    
	if (!noobj && is_type (pu.op, BINARYSTARM))
	    XtManageChild (pu.bsmap_w);
	else
	    XtUnmanageChild (pu.bsmap_w);

	XmMenuPosition (pu.pu_w, (XButtonPressedEvent *)ev);
	XtManageChild (pu.pu_w);
}

/* if there are no dupnames for op, put it in the pu.name_w label, else
 * put the original name in the pu.altnm_w cascade button and put the rest in a
 * pullright menu.
 */
static void
sv_punames (Obj *op)
{
	DupName *dnp;
	int i, nn, ndn = db_dups (&dnp);

	/* count number of names */
	for (i = nn = 0; i < ndn; i++)
	    if (dnp[i].op == op)
		nn++;

	if (nn > 1) {
	    /* put primary in CB, build pullright with alternates */
	    Widget pd;
	    Arg args[20];
	    int n;

	    /* first name in CB itself */
	    set_xmstring (pu.altnm_w, XmNlabelString, op->o_name);
	    XtManageChild (pu.altnm_w);
	    XtUnmanageChild (pu.name_w);

	    /* remaining in new pullright */
	    get_something (pu.altnm_w, XmNsubMenuId, (XtArgVal)&pd);
	    if (pd)
		XtDestroyWidget(pd);
	    n = 0;
	    pd = XmCreatePulldownMenu (pu.pu_w, "ALTPD", args, n);
	    set_something (pu.altnm_w, XmNsubMenuId, (XtArgVal)pd);

	    for (i = 0; i < ndn; i++) {
		if (dnp[i].op == op && strcmp (dnp[i].nm, op->o_name)) {
		    Widget lbl = XmCreateLabel (pd, "AL", args, 0);
		    set_xmstring (lbl, XmNlabelString, dnp[i].nm);
		    XtManageChild (lbl);
		}
	    }
	} else {
	    /* put only name in label */
	    XtUnmanageChild (pu.altnm_w);
	    set_xmstring (pu.name_w, XmNlabelString, op->o_name);
	    XtManageChild (pu.name_w);
	}
}

/* fill in the pu.rise/trans/set_w widgets from op at np.
 * we are never called for trailed objects so the time is always just Now.
 */
static void
sv_riset (Now *np, Obj *op)
{
	char buf1[64], buf2[64];
	RiseSet rs;

	dm_riset (np, op, &rs);

	dm_colFormat (np, op, &rs, RSTIME_ID, buf1);
	(void) sprintf (buf2, "%*s:  %s", PUTW, "Rise at", buf1);
	set_xmstring (pu.rise_w, XmNlabelString, buf2);

	dm_colFormat (np, op, &rs, TRTIME_ID, buf1);
	(void) sprintf (buf2, "%*s:  %s", PUTW, "Trans at", buf1);
	set_xmstring (pu.trans_w, XmNlabelString, buf2);

	dm_colFormat (np, op, &rs, TRALT_ID, buf1);
	(void) sprintf (buf2, "%*s: %s", PUTW, "Trans alt", buf1);
	set_xmstring (pu.transalt_w, XmNlabelString, buf2);

	dm_colFormat (np, op, &rs, TRAZ_ID, buf1);
	(void) sprintf (buf2, "%*s: %s", PUTW, "Trans az", buf1);
	set_xmstring (pu.transaz_w, XmNlabelString, buf2);

	dm_colFormat (np, op, &rs, SETTIME_ID, buf1);
	(void) sprintf (buf2, "%*s:  %s", PUTW, "Set at", buf1);
	set_xmstring (pu.set_w, XmNlabelString, buf2);
}

/* create the id popup */
static void
sv_create_popup()
{
	Widget pd_w, cb_w;
	Arg args[20];
	XmString str;
	Widget w;
	int n;

	/* create the outer window */

	n = 0;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNspacing, 0); n++;		/* getting crowded */
	pu.pu_w = XmCreatePopupMenu (svda_w, "SVPopup", args, n);

	/* passive name and cascade for alternates */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	pu.name_w = XmCreateLabel (pu.pu_w, "SVPopNM", args, n);
	wtip (pu.name_w, "Object name");

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	pu.altnm_w = XmCreateCascadeButton (pu.pu_w, "SVPopCB", args, n);
	wtip (pu.altnm_w, "Object name and alternates");

	/* description on name */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	pu.desc_w = XmCreateLabel (pu.pu_w, "SVPopNM", args, n);
	wtip (pu.desc_w, "Object classification");
	XtManageChild (pu.desc_w);

	/* add a separator -- managed only if there is a name */

	n = 0;
	XtSetArg (args[n], XmNmargin, 15); n++;
	XtSetArg (args[n], XmNseparatorType, XmSINGLE_LINE); n++;
	pu.nmsep_w = XmCreateSeparator (pu.pu_w, "SVSep", args, n);

	/* make the command buttons */

	str = XmStringCreateLtoR ("Center", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	w = XmCreatePushButton (pu.pu_w, "SVPopPoint", args, n);
	XtAddCallback (w, XmNactivateCallback, sv_pu_activate_cb,
							    (XtPointer)AIM);
	XtManageChild (w);
	wtip (w, "Center the Sky View at this location");
	XmStringFree (str);

	/* make the zoom cascade */

	sv_create_zoomcascade (pu.pu_w);

	/* make the label l/r casade menu */

	sv_create_labellr (pu.pu_w);

	str = XmStringCreateLtoR ("Place eyepiece", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	w = XmCreatePushButton (pu.pu_w, "SVPopEyeP", args, n);
	XtAddCallback (w, XmNactivateCallback, sv_pu_activate_cb,
							(XtPointer)PEYEPIECE);
	XtManageChild (w);
	wtip (w, "Drop an eyepiece centered at this location");
	XmStringFree(str);

	str = XmStringCreateLtoR ("Delete eyepiece", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	pu.deleyep_w = XmCreatePushButton (pu.pu_w, "SVPopDelEyeP", args, n);
	XtAddCallback (pu.deleyep_w, XmNactivateCallback, sv_pu_activate_cb,
							(XtPointer)DEYEPIECE);
	wtip (pu.deleyep_w, "Delete the eyepiece covering this location");
	XmStringFree(str);

	str = XmStringCreateLtoR("New Photom ref", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	pu.refmag_w = XmCreatePushButton (pu.pu_w, "SVPopRefMag", args, n);
	XtAddCallback (pu.refmag_w, XmNactivateCallback, sv_pu_activate_cb,
						    (XtPointer)SETPHOTOMREF);
	wtip (pu.refmag_w, "Define this object as the photometric reference");
	XmStringFree(str);

	str = XmStringCreateLtoR ("Telescope GoTo", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	pu.goto_w = XmCreatePushButton (pu.pu_w, "SVPopGoto", args, n);
	XtAddCallback (pu.goto_w, XmNactivateCallback, sv_pu_activate_cb,
							    (XtPointer)TGOTO);
	wtip (pu.goto_w, "Send coordinates of this location to external app");
	XmStringFree (str);

	str = XmStringCreateLtoR ("Add to logbook", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	pu.obslog_w = XmCreatePushButton (pu.pu_w, "SVPopLog", args, n);
	XtAddCallback (pu.obslog_w, XmNactivateCallback, sv_pu_activate_cb,
							    (XtPointer)OBSLOG);
	wtip (pu.obslog_w, "Set logbook to info about this object");
	XmStringFree (str);

#ifdef AAVSO
	str=XmStringCreateLtoR("AAVSO", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	pu.av_w = XmCreatePushButton (pu.pu_w, "SVPopAV", args, n);
	wtip (pu.av_w,"Load closest AAVSO star into Tools->AAVSO");
	XtAddCallback (pu.av_w, XmNactivateCallback, sv_pu_activate_cb,
							    (XtPointer)LOADAV);
	XmStringFree (str);
	XtManageChild (pu.av_w);
#endif

	str = XmStringCreateLtoR("Add to Favorites",XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	pu.fav_w = XmCreatePushButton (pu.pu_w, "SVPopF", args, n);
	XtAddCallback (pu.fav_w, XmNactivateCallback, sv_pu_activate_cb,
							    (XtPointer)ADD2FAV);
	wtip (pu.fav_w, "Add this object to list of Favorites");
	XmStringFree (str);

	str = XmStringCreateLtoR ("Show in gallery", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	pu.gal_w = XmCreatePushButton (pu.pu_w, "SVGal", args, n);
	XtAddCallback (pu.gal_w, XmNactivateCallback, sv_pu_activate_cb,
							    (XtPointer)GALLERY);
	wtip (pu.gal_w, "Display the gallery image of this object");
	XmStringFree (str);

	/* BINARYSTAR button */

	n = 0;
	pu.bsmap_w = XmCreatePushButton (pu.pu_w, "BPB", args, n);
	XtAddCallback (pu.bsmap_w, XmNactivateCallback, sv_bsmap_cb, 0);
	set_xmstring (pu.bsmap_w, XmNlabelString, "Show orbit...");
	wtip (pu.bsmap_w,"Open window to display orbit of this binary system"); 

	/* the Change Trail PB: must un/manage and set labelString each use */
	n = 0;
	pu.newtrail_w = XmCreatePushButton (pu.pu_w, "SVPopNewTrail", args, n);
	XtAddCallback (pu.newtrail_w, XmNactivateCallback, sv_pu_activate_cb,
							(XtPointer)NEWTRAIL);
	wtip (pu.newtrail_w,
		    "Create (or change) a time-sequence trail for this object");

	/* the Trail TB: must un/manage and set state on each use */
	str = XmStringCreateLtoR ("Trail", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	pu.trail_w = XmCreateToggleButton (pu.pu_w, "SVPopTrail", args, n);
	XtAddCallback(pu.trail_w, XmNvalueChangedCallback, sv_pu_trail_cb,NULL);
	wtip(pu.trail_w,"Whether to display an existing trail for this object");
	XmStringFree(str);

	str = XmStringCreateLtoR ("Track", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg (args[n], XmNlabelString, str); n++;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	pu.track_w = XmCreateToggleButton (pu.pu_w, "SVPopTrack", args, n);
	XtAddCallback (pu.track_w, XmNvalueChangedCallback, sv_pu_track_cb, 0);
	wtip (pu.track_w, "Keep this object centered after each Main Update");
	XtManageChild (pu.track_w);
	XmStringFree (str);

	/* add a nice separator */
	n = 0;
	w = XmCreateSeparator (pu.pu_w, "SVSep", args, n);
	XtManageChild(w);

	/* date/time */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	pu.ud_w = XmCreateLabel (pu.pu_w, "SVPopValueL", args, n);
	XtManageChild (pu.ud_w);
	wtip (pu.ud_w, "Date of this information, as per time zone Pref");

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	pu.ut_w = XmCreateLabel (pu.pu_w, "SVPopValueL", args, n);
	XtManageChild (pu.ut_w);
	wtip (pu.ut_w, "Time of this information, as per time zone Pref");

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	pu.ra_w = XmCreateLabel (pu.pu_w, "SVPopValueL", args, n);
	XtManageChild (pu.ra_w);
	wtip (pu.ra_w, "RA of this location or object");

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	pu.dec_w = XmCreateLabel (pu.pu_w, "SVPopValueL", args, n);
	XtManageChild (pu.dec_w);
	wtip (pu.dec_w, "Declination of this location or object");

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	pu.mag_w = XmCreateLabel (pu.pu_w, "SVPopValueL", args, n);
	XtManageChild (pu.mag_w);
	wtip (pu.mag_w, "Nominal brightness");

	/* rise/set pull right */

	n = 0;
	pd_w = XmCreatePulldownMenu (pu.pu_w, "CPD", args, n);

	n = 0;
	XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	cb_w = XmCreateCascadeButton (pu.pu_w, "CCB", args, n);
	set_xmstring (cb_w, XmNlabelString, "Rise-Set");
	XtManageChild (cb_w);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.rise_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.rise_w);
            wtip (pu.rise_w, "Rise time today, as per time zone Preference");

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.trans_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.trans_w);
	    wtip (pu.trans_w,"Transit time today, as per time zone Preference");

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.transalt_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.transalt_w);
	    wtip (pu.transalt_w, "Transit altitude today");

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.transaz_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.transaz_w);
	    wtip (pu.transaz_w, "Transit azimuth today");

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.set_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.set_w);
	    wtip (pu.set_w,  "Set time today, as per time zone Preference");

	/* info pull right */

	n = 0;
	pd_w = XmCreatePulldownMenu (pu.pu_w, "CPD", args, n);

	n = 0;
	XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	pu.infocb_w = XmCreateCascadeButton (pu.pu_w, "CCB", args, n);
	set_xmstring (pu.infocb_w, XmNlabelString, "More info");
	XtManageChild (pu.infocb_w);

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.alt_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.alt_w);
            wtip (pu.alt_w, "Local altitude: angle above horizon");

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.az_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.az_w);
            wtip (pu.az_w, "Local azimuth: angle E of N");

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.spect_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.spect_w);
            wtip (pu.spect_w, "Spectral classification");

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.size_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.size_w);
            wtip (pu.size_w, "Angular diameter or separation");

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.pa_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.pa_w);
            wtip (pu.pa_w, "Position angle, degs E of N");

	    n = 0;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    pu.eclipsed_w = XmCreateLabel (pd_w, "SVPopValueL", args, n);
	    XtManageChild (pu.eclipsed_w);
            wtip (pu.eclipsed_w, "Whether satellite is eclipsed or sunlit");
}

/* create the zoom cascade menu off pulldown menu pd_w */
static void
sv_create_zoomcascade (pd_w)
Widget pd_w;
{
	Widget cb_w, zpd_w, w;
	Arg args[20];
	int n;
	int i;

	n = 0;
	zpd_w = XmCreatePulldownMenu (pd_w, "ZPD", args, n);

	n = 0;
	XtSetArg (args[n], XmNsubMenuId, zpd_w); n++;
	cb_w = XmCreateCascadeButton (pd_w, "ZCB", args, n);
	set_xmstring (cb_w, XmNlabelString, "Center + Zoom");
	wtip (cb_w, "Center at this location and zoom in or out");
	XtManageChild (cb_w);

	for (i = 0; i < 6; i++) {
	    int z = 0;
	    char buf[64];

	    /* z is the numerator, 10 is the denominator of zoom-in ratio */
	    switch (i) {
	    case 0: z =  20; break;
	    case 1: z =  50; break;
	    case 2: z = 100; break;
	    case 3: z =   5; break;
	    case 4: z =   2; break;
	    case 5: z =   1; break;
	    default:
		printf ("create_zoom Bug! bad setup %d\n", i);
		abort();
	    }

	    if (z >= 10)
		(void) sprintf (buf, "Zoom in %dX", z/10);
	    else
		(void) sprintf (buf, "Zoom out %dX", 10/z);

	    n = 0;
	    w = XmCreatePushButton (zpd_w, "Zoom", args, n);
	    XtAddCallback (w, XmNactivateCallback, sv_pu_zoom_cb, (XtPointer)(long int)z);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);
	}
}

/* create the label l/r cascade menu off pulldown menu pd_w */
static void
sv_create_labellr (pd_w)
Widget pd_w;
{
	Widget lpd_w;
	Arg args[20];
	int n;

	n = 0;
	lpd_w = XmCreatePulldownMenu (pd_w, "LPD", args, n);

	n = 0;
	XtSetArg (args[n], XmNsubMenuId, lpd_w); n++;
	pu.label_w = XmCreateCascadeButton (pd_w, "Persistent Label",
								    args, n);
	wtip (pu.label_w, "Control how/if this object is labeled");

	n = 0;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	pu.llabel_w = XmCreateToggleButton (lpd_w, "on Left", args, n);
	XtAddCallback(pu.llabel_w, XmNvalueChangedCallback, sv_pu_label_cb,
							(XtPointer)OBJF_LLABEL);
	wtip (pu.llabel_w, "Label with name to the left of the symbol");
	XtManageChild (pu.llabel_w);

	n = 0;
	XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	pu.rlabel_w = XmCreateToggleButton (lpd_w, "on Right", args, n);
	XtAddCallback(pu.rlabel_w, XmNvalueChangedCallback, sv_pu_label_cb,
							(XtPointer)OBJF_RLABEL);
	wtip (pu.rlabel_w, "Label with name to the right of the symbol");
	XtManageChild (pu.rlabel_w);
}

/* called when any of the popup's pushbuttons are activated.
 * (the zoom cascade is not done here though).
 * client is a code to indicate which.
 * obtain current state from pu. The current object is at pu.op. If it is on
 *   a trail that specific entry is at pu.tsp->o.
 */
/* ARGSUSED */
static void
sv_pu_activate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int code = (long int)client;
	Obj *op = pu.tsp ? &pu.tsp->o : pu.op;

	switch (code) {
	case AIM:
	    /* turn off following remote inputs when told explicitly to point */
	    XmToggleButtonSetState (keeptelvis_w, False, 1);
	    (void) sv_mark (op, 1, 1, 0, 1, 0, 0.0);
	    break;

	case SETPHOTOMREF: {
	    double ix, iy;
	    sv_win2im (pu.wx, pu.wy, &ix, &iy);
	    si_setPhotomRef (ix, iy, get_mag(op));
	    }
	    break;

	case PEYEPIECE:
	    if (aa_mode)
		se_add (1, op->s_az, op->s_alt);
	    else
		se_add (0, op->s_ra, op->s_dec);
	    XmToggleButtonSetState (wanteyep_w, True, False);
	    want_eyep = 1;
	    sv_all (mm_get_now());
	    break;

	case DEYEPIECE:
	    if (aa_mode)
		se_del (1, op->s_az, op->s_alt);
	    else
		se_del (0, op->s_ra, op->s_dec);
	    sv_all (mm_get_now());
	    break;

	case TGOTO:
	    /* send the current object to the telecope. */
	    (void) telGoto (op);

	    /* add to Telescope pulldown */
	    sv_addtelpd (op);
	    break;

	case OBSLOG:
	    ol_setObj (op);
	    break;

	case LOADAV:
	    av_load (op);
	    break;

	case ADD2FAV:
	    /* N.B. we assume we won't get here unless op is in real db */
	    fav_add (op);
	    break;

	case GALLERY:
	    gal_opscroll (op);
	    break;

	case NEWTRAIL: {
	    TrailObj *top;
	    TrState *sp;

	    sp= pu.tsp && (top = tobj_find (&pu.tsp->o)) != 0
							? &top->trs : &trstate;
	    tr_setup ("xephem Sky Trail setup", pu.op->o_name, sp, sv_mktrail,
							    (XtPointer)pu.op);

	    }
	    break;

	default:
	    printf ("sv_pu_activate_db Bug! code=%d\n", code);
	    abort();
	    break;
	}
}

/* called when any of the zoom cascade buttons is activated.
 * client is the zoom-in factor numerator -- the denominator is 10.
 */
/* ARGSUSED */
static void
sv_pu_zoom_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int factor = (long int)client;
	double newfov = sv_vfov*10.0/factor;
	Obj *op = pu.tsp ? &pu.tsp->o : pu.op;

	/* turn off following remote inputs when told explicitly to point */
	XmToggleButtonSetState (keeptelvis_w, False, 1);

	(void) sv_mark (op, 1, 1, 0, 1, 0, newfov);
}

/* called when the Trail popup toggle button changes.
 * when called, pu.op will point to the base object (which is known to have a
 *   trail, but may not be on)
 */
/* ARGSUSED */
static void
sv_pu_trail_cb (wid, client, call)
Widget wid;
XtPointer client;
XtPointer call;
{
	TrailObj *top = tobj_find (pu.op);

	if (!top) {
	    printf ("sv_pu_trail_cb() Bug! no trail!\n");
	    abort();
	}

	if (XmToggleButtonGetState(wid)) {
	    /* trail is being turned on. just display it */
	    top->on = 1;
	    tobj_display_all();
	    sv_copy_sky();
	} else {
	    /* trailing is being turned off. mark it as being off.
	     * it will get discarded at the next update if it's still off.
	     * redraw sky so it disappears.
	     */
	    top->on = 0;
	    sv_all (mm_get_now());
	}
}

/* called when the Label popup toggle button changes.
 * client is one of OBJF_{L,R}LABEL.
 * we get all other context from the pu structure.
 */
/* ARGSUSED */
static void
sv_pu_label_cb (wid, client, call)
Widget wid;
XtPointer client;
XtPointer call;
{
	int set = ((XmToggleButtonCallbackStruct *)call)->set;
	int side = (long int)client;
	int oside = (side & OBJF_LLABEL) ? OBJF_RLABEL : OBJF_LLABEL;
	unsigned char *flagsp;

	/* if this is a trailed item then its TSky will be in pu.tsp
	 * otherwise it is a plain db object so use pu.op.
	 */
	flagsp = pu.tsp ? &pu.tsp->flags : &pu.op->o_flags;
	if (set) {
	    *flagsp |= (side|OBJF_PERSLB);
	    *flagsp &= ~oside;
	} else
	    *flagsp &= ~(side|OBJF_PERSLB);

	sv_all (mm_get_now());
}

/* called when the Track popup toggle button changes.
 * we get all context from the pu structure.
 */
/* ARGSUSED */
static void
sv_pu_track_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XmToggleButtonGetState (w)) {
	    track_op = pu.op;
	    XtSetSensitive (tracktb_w, True);
	    XmToggleButtonSetState (tracktb_w, True, True);
	    (void) sv_mark (track_op, 0, 0, 1, 1, 0, 0.0);
	} else
	    XmToggleButtonSetState (tracktb_w, False, True);
}

/* action routine to implement some keyboard shortcuts.
 * SVScut(a,x) where a selects what and x is a factor.
 */
static void
sv_shortcuts (w, e, p, n)
Widget w;
XEvent *e;
String *p;
Cardinal *n;
{
	int what;
	double factor;

	if (!(n && *n == 2)) {
	    printf ("Bad sv_shortcuts: %p %d %p\n", n, n?*n:0, p);
	    abort();
	}

	/* disable any image display */
	si_off();

	/* perform the action */
	what = atoi(p[0]);
	factor = atof(p[1]);
	switch (what) {
	case 0:			/* pan horizontal */
	    /* account for flipping and somewhat faster near poles */
	    sv_azra += factor*sv_hfov*((aa_mode ^ flip_lr) ? 1 : -1)
					    /(1+.9*(cos(sv_altdec)-1));
	    range (&sv_azra, 2*PI);
	    sv_set_scale(AZRA_S, 0);
	    break;
	case 1:			/* pan vertical */
	    /* account for flipping */
	    sv_altdec += factor*sv_vfov*(flip_tb ? -1 : 1);
	    if (sv_altdec < -PI/2) sv_altdec = -PI/2;
	    if (sv_altdec >  PI/2) sv_altdec =  PI/2;
	    sv_set_scale(ALTDEC_S, 0);
	    break;
	case 2:			/* zoom */
	    sv_vfov *= factor;
	    sv_set_fov (sv_vfov);
	    sv_set_scale(FOV_S, 0);
	}

	/* draw then abandon auto repeats */
	sv_all(NULL);
	XmUpdateDisplay (svda_w);
	XSync (XtD, True);
}


/* remove trails which are no longer turned on. */
static void
tobj_rmoff()
{
	TrailObj **topp;	/* address to be changed if we decide to 
				 * remove *topp
				 */
	TrailObj *top;		/* handy *topp */

	for (topp = &trailobj; (top = *topp) != NULL; ) {
	    if (top->on) {
		topp = &top->ntop;
	    } else {
		*topp = top->ntop;
		XtFree ((char *)top);
	    }
	}
}

/* remove the trailobj list that contains the given pointer.
 * we have to search each trail list to find the one with this pointer.
 * it might be the one on TrailObj itself or one of the older ones on
 *    the TSky list.
 * it's no big deal if op isn't really on any trail list.
 */
static void
tobj_rmobj (op)
Obj *op;
{
	TrailObj **topp;	/* address to be changed if we decide to 
				 * remove *topp
				 */
	TrailObj *top;		/* handy *topp */

	for (topp = &trailobj; (top = *topp) != NULL; ) {
	    int i;
	    if (top->op == op)
		goto out;
	    for (i = 0; i < top->nsky; i++)
		if (&top->sky[i].o == op)
		    goto out;
	    topp = &top->ntop;
	}

    out:

	if (!top)
	    return;	/* oh well */

	*topp = top->ntop;
	XtFree ((char *)top);
}

/* add a new TrailObj entry to the trailobj list for db object op.
 * make enough room in the sky array for nsky entries.
 * return a pointer to the new TrailObj.
 * (we never return if there's no memory)
 */
static TrailObj *
tobj_addobj (op, nsky)
Obj *op;
int nsky;
{
	TrailObj *top;
	int nbytes;

	/* don't forget there is inherently room for one TSky in a TrailObj */
	nbytes = sizeof(TrailObj) + (nsky-1)*sizeof(TSky);
	top = (TrailObj *) XtMalloc (nbytes);
	zero_mem ((void *)top, nbytes);
	top->nsky = nsky;	  /* though none are in use now */
	top->op = op;
	top->on = 1;

	/* link directly off trailobj -- order is unimportant */
	top->ntop = trailobj;
	trailobj = top;

	return (top);
}

/* remove each trail that refers to a db object, including field stars, no
 * longer around. also reset the tracked object if it is gone.
 * this is done after the db has been reduced.
 */
static void
tobj_newdb()
{
	TrailObj *top;
	DBScan dbs;
	Obj *op;

	for (top = trailobj; top; ) {
	    for (db_scaninit (&dbs, ALLM, fldstars, nfldstars);
	    					(op = db_scan (&dbs)) != NULL; )
		if (top->op == op)
		    break;
	    if (op == NULL) {
		/* trailed object is gone -- remove trail and restart */
		tobj_rmobj (top->op);
		top = trailobj;
	    } else
		top = top->ntop;
	}


	if (track_op) {
	    for (db_scaninit (&dbs, ALLM, fldstars, nfldstars);
	    					(op = db_scan (&dbs)) != NULL; )
		if (track_op == op)
		    break;
	    if (op == NULL) {
		/* tracked object is gone */
		XmToggleButtonSetState (tracktb_w, False, False);
		XtSetSensitive (tracktb_w, False);
		track_op = NULL;
	    }
	}
}

/* find the TrailObj that contains op.
 * return NULL if don't find it.
 */
static TrailObj *
tobj_find (op)
Obj *op;
{
	TrailObj *top;

	for (top = trailobj; top; top = top->ntop) {
	    int i;
	    if (top->op == op)
		return (top);
	    for (i = 0; i < top->nsky; i++)
		if (&top->sky[i].o == op)
		    return (top);
	}

	return (NULL);
}

/* display everything in the trailobj list that is marked on onto sv_pm
 * clipped to the current window.
 */
static void
tobj_display_all()
{
	static int aatrailwarn;
	Display *dsp = XtDisplay(svda_w);
	TrailObj *top;

	/* reset trail counter -- increment if actually print any trails.
	 * this is just used when printing to know whether to include a message.
	 */
	anytrails = 0;

	for (top = trailobj; top; top = top->ntop) {
	    int x1 = 0, y1 = 0, x2, y2;
	    int i, d;
	    GC gc;

	    if (!top->on)
		continue;

	    d = objdiam (top->op);
	    objGC (top->op, d, &gc);

	    /* warn about aa trails and topo ra/dec first time only */
	    if (!aatrailwarn && aa_mode && top->nsky > 1) {
		aatwarn_msg();
		aatrailwarn = 1;
	    }

	    for (i = 0; i < top->nsky; i++) {
		TSky *sp = &top->sky[i];
		Obj *op = &sp->o;

		if (sv_trailobjloc (sp, &x2, &y2)) {
		    if (sp->flags & OBJF_PERSLB) {
			int lflags;
			if (is_deepsky (op))
			    lflags = lbl_lds;
			else if (is_ssobj (op))
			    lflags = lbl_lss;
			else if (op->o_flags & FLDSTAR)
			    lflags = lbl_lfs;
			else
			    lflags = lbl_lst;
			draw_label (sv_pm, gc, op, lflags|sp->flags, x2, y2, d);
		    }
		}
		if (i > 0) {
		    int sx1, sy1, sx2, sy2;

		    if (segisvis (x1, y1, x2, y2, &sx1, &sy1, &sx2, &sy2)) {
			TrTS *tp = (x2==sx2 && y2==sy2) ? &sp->trts : NULL;
			TrTS *ltp = (i==1 && x1==sx1 && y1==sy1)
						? &top->sky[0].trts : NULL;
			int e = (is_type(op,EARTHSATM) && op->s_eclipsed);
			int xwrap, ywrap;
			XSegment xs;

			xs.x1 = sx1;
			xs.y1 = sy1;
			xs.x2 = sx2;
			xs.y2 = sy2;
			split_wrap (&xs, &xwrap, &ywrap);

			if (xwrap || ywrap) {
			    tr_draw (dsp, sv_pm, gc, e, TICKLEN, tp, ltp,
				    &top->trs, sx1, sy1, sx2+xwrap, sy2+ywrap);
			    tr_draw (dsp, sv_pm, gc, e, TICKLEN, tp, ltp,
				    &top->trs, sx1-xwrap, sy1-ywrap, sx2, sy2);
			} else
			    tr_draw (dsp, sv_pm, gc, e, TICKLEN, tp, ltp,
					    &top->trs, sx1, sy1, sx2, sy2);
			anytrails++;
		    }
		}
		x1 = x2;
		y1 = y2;
	    }

	}

	sv_draw_obj (dsp, sv_pm, (GC)0, NULL, 0, 0, 0, 0);	/* flush */
}

static void
aatwarn_msg ()
{
	xe_msg (1, 
"\n\
Please be aware that due to diurnal motion Alt/Az trails can be\n\
very misleading when viewed against the fixed background of stars.\n\
\n\
              This message will only appear one time");
}

/* determine if the given object is visible and within sv_w/sv_h.
 * if so, return 1 and compute the size and location, else return 0.
 * N.B. only call this for bona fide db objects -- *not* for objects in the
 *   TrailObj lists -- it will destroy their history.
 */
static int
sv_dbobjloc (Obj *op, int *xp, int *yp, int *dp)
{
	double altdec, azra;

	if (!sv_precheck(op))
	    return (0);

	/* remaining things need accurate s_* fields */
	db_update(op);
	if (op->o_flags & NOCIRCUM)
	    return (0);

	if (!sv_hznOpOk(op))
	    return(0);

	/* persistent labels are immune to faint cutoff */
	if (!(op->o_flags & OBJF_PERSLB)) {
	    int stmag, ssmag, dsmag, magstp;
	    double m = get_mag(op);

	    svf_getmaglimits (&stmag, &ssmag, &dsmag, &magstp);
	    if (m > (is_deepsky(op) ? dsmag : (is_ssobj(op) ? ssmag : stmag)))
		return(0);	/* it's not within mag range after all */
	}

	/* check if any part of object is on screen */
	altdec = aa_mode ? op->s_alt : op->s_dec;
	azra   = aa_mode ? op->s_az  : op->s_ra;
	*dp = objdiam (op);
	if (sv_loc (altdec, azra, xp, yp))
	    return (1);
	if (abs(*xp)!=MAXXW && abs(*yp)!=MAXXW && sv_dfov<PI) {
	    /* might still have a portion visible */
	    int r = *dp/2;
	    return (*xp+r>=0 && *xp-r<sv_w && *yp+r>=0 && *yp-r<sv_h);
	}
	return (0);
}

/* given a TSky find its location on a window of size sv_w/sv_h.
 * return 1 if the resulting x/y is in fact within the window, else 0 if it is
 *   outside or otherwise should not be shown now (but return the x/y anyway).
 * N.B. we take care to not change the tsp->o in any way.
 */
static int
sv_trailobjloc (tsp, xp, yp)
TSky *tsp;
int *xp, *yp;
{
	Obj *op = &tsp->o;
	double altdec, azra;

	altdec = aa_mode ? op->s_alt : op->s_dec;
	azra   = aa_mode ? op->s_az  : op->s_ra;
	return (sv_loc (altdec, azra, xp, yp) && sv_hznOpOk(op));
}

/* do as much as possible to pre-check whether op should be on screen now
 * WITHOUT computing it's actual coordinates. put another way, we are not to
 * use any s_* fields in these tests.
 * return 0 if we know it's definitely not on screen, or 1 if it might be.
 */
static int
sv_precheck (op)
Obj *op;
{
	Now *np = mm_get_now();

	if (op->o_type == UNDEFOBJ)
	    return(0);

	/* check orbital elements for proper date range */
	if (dateOK (np, op) < 0)
	    return (0);

	/* persistent labels are immune to faint cutoff */
	if (!(op->o_flags & OBJF_PERSLB) && !svf_filter_ok(op))
	    return(0);
	if (!sv_onscrn(np, op))
	    return(0);
	
	return (1);
}

/* return 1 if the object can potentially be on screen, else 0.
 * N.B. this is meant to be cheap - we only do fixed objects and we don't
 *      precess. most specifically, !! we don't use any s_* fields. !!
 */
static int
sv_onscrn (Now *np, Obj *op)
{
#define	DELEP	3650		/* maximum epoch difference we dare go without
				 * precessing, days
				 */
#define	MARGIN	degrad(1.0)	/* border around fov still considered "in"
				 * in spite of having not precessed.
				 */
	double ra0, dec0;	/* ra/dec of our center of view */
	double r;
	int polecap;

	if (!is_type (op, FIXEDM))
	    return (1);
	if (fabs (mjd - op->f_epoch) > DELEP)
	    return (1);

	if (aa_mode) {
	    /* compute ra/dec of view center; worth caching too */
	    static double last_lat = 9876, last_alt, last_az;
	    static double last_ha, last_dec0;
	    double ha, lst;

	    if(lat == last_lat && sv_altdec == last_alt && sv_azra == last_az) {
		ha = last_ha;
		dec0 = last_dec0;
	    } else {
		aa_hadec (lat, sv_altdec, sv_azra, &ha, &dec0);
		last_lat = lat;
		last_alt = sv_altdec;
		last_az = sv_azra;
		last_ha = ha;
		last_dec0 = dec0;
	    }

	    /* now_lst() already knows how to cache; others are very cheap */
	    now_lst (np, &lst);
	    ra0 = hrrad(lst) - ha;
	    range (&ra0, 2*PI);
	} else {
	    ra0 = sv_azra;
	    dec0 = sv_altdec;
	}

	r = sv_dfov/2 + MARGIN;
	polecap = fabs(op->f_dec) > PI/2 - (r + fabs(dec0));

	/* just check a surrounding "rectangular" region. */
	return (fabs(op->f_dec - dec0) < r
		&& (polecap || delra(op->f_RA-ra0)*cos(op->f_dec) < r));
}

/* compute x/y loc of a point at azra/altdec as viewed from sv_azra/sv_altdec.
 * if we are displaying a FITS image, use it directly.
 * always set *x/yp, but return value is whether it is really on screen now.
 */
static int
sv_loc (altdec, azra, xp, yp)
double altdec;	/* angle up from spherical equator, such as alt or dec; rads */
double azra;	/* angle around spherical pole, such as az or ra; rads */
int *xp, *yp;	/* return X coords within sv_w/h window */
{
	if (si_ison()) {
	    /* use FITS image */
	    double x, y;
	    FImage *fip;

	    fip = si_getFImage();
	    if (!fip) {
		printf ("sv_loc Bug! FITS disappeared\n");
		abort();
	    }
	    if (RADec2xy (fip, azra, altdec, &x, &y) < 0) {
		*xp = sv_w/2;
		*yp = sv_h/2;
		return (0);
	    }

	    si_im2win (x, y, sv_w, sv_h, xp, yp);
	    if (flip_lr)
		*xp = sv_w - 1 - *xp;
	    if (flip_tb)
		*yp = sv_h - 1 - *yp;
	} else if (cyl_proj) {
	    /* cylindrical:
	     * lines of constant altdec are horizontal,
	     * lines of constant azra are vertical.
	     * nothing shows beyond the poles.
	     */
	    double scale = sv_h/sv_vfov;
	    int fullwide = (int)(2*PI*scale+.5);
	    int halfwide = (int)(1*PI*scale+.5);

	    /* looks nice to space evenly with sv_azra in center */
	    *xp = (int)floor((sv_azra - azra)*scale + sv_w/2 + .5);
	    if (*xp < sv_w/2-halfwide)
		*xp += fullwide;
	    if (*xp >= sv_w/2+halfwide)
		*xp -= fullwide;
	    *yp = (int)floor((sv_altdec  - altdec)*scale + sv_h/2 + .5);

	    /* flipping */
	    if (aa_mode != flip_lr)	/* a/a opposite of r/d */
		*xp = sv_w - *xp;
	    if (flip_tb)
		*yp = sv_h - *yp;

	} else {
	    /* spherical */
#define	LOCEPS	(1e-7)	/* an angle too small to see on screen, rads */
	    static double last_sv_altdec = 123.0, last_sa, last_ca;
	    double a,sa,ca;	/* angle from viewpoint to pole */
	    double b,sb,cb;	/* angle from object to pole */
	    double c,sc,cc;	/* difference in polar angles of obj and vwpt */
	    double d,sd,cd;	/* angular separation of object and viewpoint */
	    double r;		/* proportion of d to desired field of view */
	    double se, ce;	/* angle between (vwpt,pole) and (vwpt,obj) */


	    a = PI/2 - sv_altdec;
	    if (sv_altdec == last_sv_altdec) {
		sa = last_sa;
		ca = last_ca;
	    } else {
		last_sv_altdec = sv_altdec;
		last_sa = sa = sin(a);
		last_ca = ca = cos(a);
	    }

	    b = PI/2 - altdec;
	    sb = sin(b);
	    cb = cos(b);

	    c = aa_mode ? azra - sv_azra : sv_azra - azra;
	    cc = cos(c);

	    cd = ca*cb + sa*sb*cc;
	    if (cd >  1.0) cd =  1.0;
	    if (cd < -1.0) cd = -1.0;
	    d = acos(cd);

	    if (d < LOCEPS) {
		*xp = sv_w/2;
		*yp = sv_h/2;
		return (1);
	    }

	    r = d/(sv_vfov/2.0);

	    sc = sin(c);
	    sd = sin(d);
	    se = sc*sb/sd;
	    *xp = (int)floor ((sv_w + sv_h*r*se)/2 + 0.5);
	    if (flip_lr)
		*xp = sv_w - *xp;
	    if (a < LOCEPS) {
		/* as viewpoint approaches N pole, e approaches PI - c */
		ce = -cc;
	    } else if (a > PI - LOCEPS) {
		/* as viewpoint approaches S pole, e approaches c */
		ce = cc;
	    } else {
		/* ok (we've already checked for small d) */
		ce = (cb - cd*ca)/(sd*sa);
	    }
	    *yp = (int)floor ((sv_h - sv_h*r*ce)/2 + 0.5);
	    if (flip_tb)
		*yp = sv_h - *yp;

	    if (d > PI/2)
		return (0);
#undef LOCEPS
	}

	/* maintain angles but reign in huge values */
	if (abs(*xp) > MAXXW || abs(*yp) > MAXXW) {
	    if (abs(*xp) > abs(*yp)) {
		/* set x to MAXXW and scale y to maintain ratio */
		*yp = (int)floor((MAXXW/(double)abs(*xp))*(*yp) + .5);
		*xp = *xp >= 0 ? MAXXW : -MAXXW;
	    } else {
		/* set y to MAXXW and scale x to maintain ratio */
		*xp = (int)floor((MAXXW/(double)abs(*yp))*(*xp) + .5);
		*yp = *yp >= 0 ? MAXXW : -MAXXW;
	    }
	}

	/* in any coord system, return whether on screen */
	return(*xp >= 0 && *xp < sv_w && *yp >= 0 && *yp < sv_h);
}

/* compute azra/altdec loc of a point at x/y as viewed from sv_azra/sv_altdec.
 * if displaying a FITS image, use it directly.
 * return true if x/y is valid, else 0.
 */
static int
sv_unloc (x, y, altdecp, azrap)
int x, y;	/* X coords within window */
double *altdecp;/* angle up from spherical equator, such as alt or dec; rad */
double *azrap;	/* angle around spherical pole, such as az or ra; rad */
{
	/* basic bounds check */
	if (x < 0 || x >= sv_w || y < 0 || y >= sv_h)
	    return (0);

	if (si_ison()) {
	    FImage *fip = si_getFImage();
	    double ix, iy;
	    sv_win2im (x, y, &ix, &iy);
	    if (xy2RADec (fip, ix, iy, azrap, altdecp) < 0)
		return (0);
	} else if (cyl_proj) {
	    double scale = sv_vfov/sv_h;

	    if (aa_mode != flip_lr)	/* a/a opposite of r/d */
		x = sv_w - x;
	    if (flip_tb)
		y = sv_h - y;

	    *altdecp = sv_altdec  - scale*(y - sv_h/2);
	    *azrap = sv_azra - scale*(x - sv_w/2);
	    if (*altdecp > PI/2) {
		*altdecp = PI - *altdecp;
		*azrap += PI;
	    }
	    if (*altdecp < -PI/2) {
		*altdecp = -PI - *altdecp;
		*azrap += PI;
	    }
	    range (azrap, 2*PI);
	} else {
#define	UNLOCEPS (1e-7)	/* sufficiently close to pole to not know az/ra; rads */
	    double a,sa,ca;	/* angle from viewpoint to pole */
	    double r;		/* distance from center to object, pixels */
	    double d,sd,cd;	/* distance from center to object, rads */
	    double   se,ce;	/* angle between (vwpt,pole) and (vwpt,obj) */
	    double b,sb,cb;	/* angle from object to pole */
	    double c,   cc;	/* difference in polar angles of obj and vwpt */
	    int x0 = sv_w/2, y0 = sv_h/2;	/* screen center */

	    /* undo flipping first in case either sv_[wh] is odd ... */
	    if (flip_lr)
		x = sv_w - x;
	    if (flip_tb)
		y = sv_h - y;

	    /* then check for center -- avoids cases where r == 0 */
	    if (x == x0 && y == y0) {
		*altdecp = sv_altdec;
		*azrap = sv_azra;
		return (1);
	    }

	    a = PI/2 - sv_altdec;
	    sa = sin(a);
	    ca = cos(a);

	    r = sqrt ((double)((x-x0)*(x-x0) + (y-y0)*(y-y0)));
	    d = r*sv_vfov/sv_h;
	    if (fabs(d) >= PI/2)
		return (0);	/* outside sphere */
	    sd = sin(d);
	    cd = cos(d);
	    ce = (y0 - y)/r;
	    se = (x - x0)/r;
	    cb = ca*cd + sa*sd*ce;
	    b = acos(cb);
	    *altdecp = PI/2 - b;

	    /* find c, the polar angle between viewpoint and object */
	    if (a < UNLOCEPS) {
		/* as viewpoint approaches N pole, c approaches PI - e */
		c = acos(-ce);
	    } else if (a > PI - UNLOCEPS) {
		/* as viewpoint approaches S pole, c approaches e */
		c = acos(ce);
	    } else if (b < UNLOCEPS || b > PI - UNLOCEPS) {
		/* as object approaches either pole, c becomes arbitary */
		c = 0.0;
	    } else {
		sb = sin(b);
		cc = (cd - ca*cb)/(sa*sb);
		if (cc < -1.0) cc = -1.0;	/* heh man, it happens */
		if (cc >  1.0) cc =  1.0;	/* yah man, it happens */
		c = acos (cc);		/* 0 .. PI; next step checks if c
					     * should be > PI
					     */
	    }
	    if (se < 0.0) 		/* if e > PI */
		c = PI + (PI - c);	/*     so is c */

	    if (aa_mode)
		*azrap = sv_azra + c;
	    else
		*azrap = sv_azra - c;
	    range (azrap, 2*PI);

#undef UNLOCEPS
	}

	return (1);
}

/* if aa
 *    altdec/azra are alt/az and return dec/ra
 * else
 *    altdec/azra are dec/ra and return alt/az
 */
void
sv_other (double altdec, double azra, int aa, double *altdecp, double *azrap)
{
	Now *np = mm_get_now();
	double tmp;

	if (aa)
	    sv_fullwhere (np, altdec, azra, aa, &tmp, &tmp, azrap, altdecp);
	else
	    sv_fullwhere (np, altdec, azra, aa, altdecp, azrap, &tmp, &tmp);
}

/* given an altdec/azra pair (decided by aa), find all coords for our current
 * location. all values will be topocentric if we are currently in Alt/Az
 * display mode, else all values will be geocentric.
 */
static void
sv_fullwhere (np, altdec, azra, aa, altp, azp, rap, decp)
Now *np;
double altdec, azra;
int aa;
double *altp, *azp;
double *rap, *decp;
{
	double ha;
	double lst;

	now_lst (np, &lst);
	lst = hrrad(lst);

	if (aa) {
	    /* need to make the ra/dec entries */
	    *altp = altdec;
	    *azp = azra;
	    unrefract (pressure, temp, altdec, &altdec);
	    aa_hadec (lat, altdec, azra, &ha, decp);
	    *rap = lst - ha;
	    range (rap, 2*PI);
	    if (epoch != EOD)
		ap_as (np, epoch, rap, decp);
	} else {
	    /* need to make the alt/az entries */
	    double ra, dec;

	    ra = *rap = azra;
	    dec = *decp = altdec;
	    if (epoch != EOD)
		as_ap (np, epoch, &ra, &dec);
	    ha = lst - ra;
	    hadec_aa (lat, ha, dec, altp, azp);
	    refract (pressure, temp, *altp, altp);
	}
}

/* aux info for labeling */

/* one per label category, order sets position priority */
typedef enum 
{
    DrawLblFirst = 0,
    LBLFS = DrawLblFirst, 
    LBLDS, 
    LBLST, 
    LBLSS,			/* last so gets drawn last */
    DrawLblNb			/* Number of label categories */
} DrawLbl;

/* one per object drawn on screen */
typedef struct {
    Obj *op;			/* Obj to draw */
    int x, y, d;		/* x,y, diam */
    GC gc;			/* GC */
    int lbl;			/* DrawLbl category */
} OneObj;

/* one per category */
typedef struct
{
    OneObj *objlist;		/* list of objects on screen */
    int objlistsize;		/* total room */
    int objlistnb;		/* n in use */
} OneObjList;

/* given two LBLSS OneObj* compare by decreasing edist, qsort-style */
static int
draw_edist_cmpf (const void *v1, const void *v2)
{
	OneObj *oop1 = (OneObj*)v1;
	OneObj *oop2 = (OneObj*)v2;
	return (oop1->op->s_edist > oop2->op->s_edist ? -1 : 1);
}

/* given two OneObj* compare by increasing numeric mag, qsort-style */
static int
draw_mag_cmpf (const void *v1, const void *v2)
{
	OneObj *oop1 = (OneObj*)v1;
	OneObj *oop2 = (OneObj*)v2;
	return (get_mag(oop1->op) < get_mag(oop2->op) ? -1 : 1);
}


/* decide which side to draw label */
static int
labelSide(OneObj *oop, Region lr, int cw, int ch, XRectangle *xrp, int lblflags)
{
	int d, lx, rx, y, w, h;
	int prefl;

	/* find label size. TODO: use exact same as draw_label() */
	d = oop->d/3;			/* ~d/(2*sqrt2)) */
	w = 0;
	if (lblflags & OBJF_MLABEL)
	    w += cw * 3;
	if (lblflags & OBJF_NLABEL)
	    w += cw * strlen (oop->op->o_name);
	h = ch;
	lx = oop->x - w - d;
	rx = oop->x + d;
	y = oop->y - d - ch;

	/* use preferred side unless it overlaps and opposite side does not */
	prefl = (oop->op->o_flags & OBJF_LLABEL);
	if (XRectInRegion (lr, prefl?lx:rx, y, w, h) != RectangleOut &&
		    XRectInRegion (lr, prefl?rx:lx,  y, w, h) == RectangleOut)
	    prefl = !prefl;

	/* assign desired region */
	xrp->y = y;
	xrp->width = w;
	xrp->height = h;
	if (prefl) {
	    xrp->x = lx;
	    lblflags &= ~OBJF_RLABEL;
	    lblflags |=  OBJF_LLABEL;
	} else {
	    xrp->x = rx;
	    lblflags &= ~OBJF_LLABEL;
	    lblflags |=  OBJF_RLABEL;
	}

	return (lblflags);
}

/* draw all visible objects */
static void
draw_allobjs (dsp, win)
Display *dsp;
Drawable win;
{
#define NOPMEM		4096
	OneObjList oplist[DrawLblNb];	/* objects on screen, by category */
        OneObjList *catlist;		/* Object list for one category */
	int nwant=0, nlbl=0;		/* n wanted labeled, n so far */
	DBScan dbs;			/* db scan context */
	int lblflags=0;			/* how to label current category */
	int cw, ch;                     /* char w and h */
	int dir, asc, des;		/* char dir, ascent and descent */
	XCharStruct xcs;		/* char info */
	Region lr;			/* all labels in one region */
        DrawLbl lbl;
	OneObj *oop;
	Obj *op;
	int i;
	int d;

        /* Initialize each list */
        for (lbl = DrawLblFirst; lbl < DrawLblNb; lbl++ ) {
	    oplist[lbl].objlist = (OneObj*) malloc (1); /* seed for realloc */
	    oplist[lbl].objlistsize = 0;
	    oplist[lbl].objlistnb = 0;
        }

	/* go through the database and find what we want to display */
	for (db_scaninit(&dbs, ALLM, want_fs ? fldstars : NULL, nfldstars);
					    (op = db_scan(&dbs)) != NULL; ) {
	    int x, y;

	    if (!sv_dbobjloc(op, &x, &y, &d))
		op->o_flags &= ~OBJF_ONSCREEN;
	    else {
		op->o_flags |= OBJF_ONSCREEN;

		/* Determine object type */
		if (op->o_flags & FLDSTAR)
		    lbl = LBLFS;
		else if (is_ssobj (op))
		    lbl = LBLSS;
		else if (is_deepsky (op))
		    lbl = LBLDS;
		else
		    lbl = LBLST;

		/* Fill list according to category */
		catlist = &oplist[lbl];

		/* expand list if need more room */
		if (catlist->objlistnb >= catlist->objlistsize) {
		    catlist->objlistsize += NOPMEM;
		    catlist->objlist = (OneObj*) realloc (catlist->objlist,
					   catlist->objlistsize*sizeof(OneObj));
		}

		/* find info for this obj once */
		oop = &catlist->objlist[catlist->objlistnb++];
		memset (oop, 0, sizeof(*oop));
		oop->op = op;
		oop->x = x;
		oop->y = y;
		oop->d = d;
		objGC (op, d, &oop->gc);
		oop->lbl = lbl;
	    }
	}

        /* Sort only solar system objects by distance */
	qsort (oplist[LBLSS].objlist, oplist[LBLSS].objlistnb, sizeof(OneObj), 
							   draw_edist_cmpf);

	/* draw objects by decreasing distance from earth */
        for (lbl = DrawLblFirst; lbl < DrawLblNb; lbl++) {
           catlist = &oplist[lbl];

           for (i = 0; i < catlist->objlistnb; i++) {
		oop = &catlist->objlist[i];
		sv_draw_obj (dsp, win, oop->gc, oop->op, oop->x, oop->y, oop->d,
								    justdots);
	    }
	}
	sv_draw_obj (dsp, win, (GC)0, NULL, 0, 0, 0, 0);	/* flush */

	/* get rough char size */
	XTextExtents (sv_pf, "A", 1, &dir, &asc, &des, &xcs);
	cw = xcs.width;
	ch = xcs.ascent;

	/* init label regions */
	lr = XCreateRegion();

  	/* label all persistent and n brightest per category */
	for (lbl = DrawLblFirst; lbl < DrawLblNb; lbl++ ) {
	    switch (lbl) {
	    case LBLST:
		XmScaleGetValue (lbl_bst_w, &nwant);
		lblflags = lbl_lst;
		break;
	    case LBLDS:
		XmScaleGetValue (lbl_bds_w, &nwant);
		lblflags = lbl_lds;
		break;
	    case LBLFS:
		XmScaleGetValue (lbl_bfs_w, &nwant);
		lblflags = lbl_lfs;
		break;
	    case LBLSS:
		XmScaleGetValue (lbl_bss_w, &nwant);
		lblflags = lbl_lss;
		break;
	    case DrawLblNb:
		break;			/* for lint */
	    }
	    nlbl = 0;
  
	    catlist = &oplist[lbl];
  
	    qsort (catlist->objlist, catlist->objlistnb, sizeof(OneObj), 
								draw_mag_cmpf);
  
	    for (i = 0; i < catlist->objlistnb; i++) {
		oop = &catlist->objlist[i];
		op = oop->op;

		if ((op->o_flags & OBJF_PERSLB) ||
					((lblflags&(OBJF_NLABEL|OBJF_MLABEL))
							&& nlbl++ < nwant)) {
		    XRectangle xr;
		    Region tmpr;

		    /* draw on best side */
		    lblflags = labelSide (oop, lr, cw, ch, &xr, lblflags);
		    draw_label (win, oop->gc, op, lblflags|op->o_flags, oop->x,
							    oop->y, oop->d);

		    /* add label to region */
		    tmpr = XCreateRegion();
		    XUnionRectWithRegion (&xr, lr, tmpr);
		    XDestroyRegion (lr);
		    lr = tmpr;
		}
	    }

	    /* This list is not needed any more */
	    free (catlist->objlist) ;
        }

	XDestroyRegion (lr);

#undef	NOPMEM
}

/* choose a nice step size for about MAXGRID steps for angular range a.
 * *dp will be step size, *np will be number of steps to include full range.
 * all angles in rads.
 */
static void
niceStep (a, dp, np)
double a;
double *dp;
int *np;
{
	static int nicesecs[] = {
	       1,    2,     5,    10,    20,     30,
	      60,  120,   300,   600,  1200,   1800,
	    3600, 7200, 18000, 36000, 72000, 108000,
	    1296000 /* safety net */
	};
	double as = raddeg(a)*3600.0;
	double d;
	int i;

	for (i = 0; i < XtNumber(nicesecs); i++)
	    if ((int)floor(as/nicesecs[i]) < MAXGRID)
		break;

	d = degrad(nicesecs[i]/3600.0);
	*np = (int)ceil(a/d);
	*dp = d;
}

/* draw label on grid near [x,y] */
static void
draw_grid_label (dsp, win, gc, ad, ar, x0, y0, samesys, arlabel, dv, dh)
Display *dsp;
Window win;
GC gc;
double ad, ar;		/* location known to be at intersection*/
int x0, y0;		/* location known to be at intersection*/
int samesys;		/* whether grid is in same coord system as display */
int arlabel;		/* whether want azra or altdec label */
double dv, dh;		/* altdec and azra whole grid step sizes, rads */
{
	char buf[32];		/* coord string */
	double a;		/* text rotation angle */
	int x1, y1;		/* other end */
	int xc, yc;		/* center pos */

	/* comes in as display mode, we need grid mode coordinates */
	if (!samesys)
	    sv_other (ad, ar, !want_aagrid, &ad, &ar);

	/* want perfect grid crossing and no wrap */
	ad = dv*floor((ad+dv/2)/dv);
	ar = dh*floor((ar+dh/2)/dh);
	range (&ar, 2*PI);

	/* build string and move to other end one grid step away */
	if (arlabel) {
	    fs_sexa (buf, want_aagrid ? raddeg(ar) : radhr(ar), 3, 3600);
	    ad += (ad > 0) ? -dv : dv;		/* back off from poles */
	} else {
	    fs_sexa (buf, raddeg(ad), 3, 3600);
	    ar -= dh;				/* not down the center */
	}

	/* back to display mode to get screen coords of other end */
	if (!samesys)
	    sv_other (ad, ar, want_aagrid, &ad, &ar);
	if (!sv_loc (ad, ar, &x1, &y1))
	    return;

	/* rotate so text is never upside down */
	a = raddeg(atan2((double)y0-y1, (double)(x1==x0?1:x1-x0)));
	if (a > 90)
	    a -= 180;
	if (a < -90)
	    a += 180;

	/* center label between parallels */
	xc = (x0+x1)/2;
	yc = (y0+y1)/2;
	XPSRotDrawAlignedString (dsp, sv_rf, a, 1.0, win, gc,xc,yc,buf,BCENTRE);
}

/* draw a nice grid, with labels */
static void
draw_grid(dsp, win, gc)
Display *dsp;
Window win;
GC gc;
{
	XSegment xsegs[50], *xs;/* segments cache */
	int samesys;		/* whether grid is in same coord sys as dsp */
	double altdec, azra;	/* center in grid coords */
	double dv, dh;		/* v and h step size in grid coords */
	double pangle;		/* grid coord polar angle */
	int seepole;		/* whether can see pole in grid coord sys */
	double polegap;		/* don't crowd closer than this to pole */
	int nvt, nht;		/* num v and h steps */
	char msg[128];
	int i, j;

	/* decide whether grid is in different coord system than display mode */
	samesys = (!!want_aagrid == !!aa_mode);

	/* find center and whether pole is visible in grid system */
	if (samesys) {
	    altdec = sv_altdec;
	    azra = sv_azra;
	    seepole = sv_loc (altdec >= 0.0 ? PI/2 : -PI/2, 0.0, &i, &j);
	} else {
	    double h, v;
	    sv_other (sv_altdec, sv_azra, aa_mode, &altdec, &azra);
	    sv_other (PI/2, 0.0, want_aagrid, &v, &h);
	    seepole = sv_loc (v, h, &i, &j);
	    if (!seepole) {
		sv_other (-PI/2, 0.0, want_aagrid, &v, &h);
		seepole = sv_loc (v, h, &i, &j);
	    }
	}

	/* grid's polar angle: 2*PI if pole visible, else scales by 1/cos */
	if (seepole || fabs(altdec) >= PI/2)
	    pangle = 2*PI;
	else
	    pangle = sv_dfov/cos(altdec) * 1.2;	/* fudge */
	if (pangle > 2*PI)
	    pangle = 2*PI;

	/* pick size and number of steps, either from user or automatically */
	if (want_autogrid) {
	    if (want_aagrid)
		niceStep (pangle, &dh, &nht);
	    else {
		/* do RA in hours */
		niceStep (pangle/15.0, &dh, &nht);
		dh *= 15.0;
	    }
	    niceStep (sv_dfov, &dv, &nvt);
	} else {
	    char *str;

	    str = XmTextFieldGetString (hgrid_w);
	    f_scansexa (str, &dh);
	    XtFree (str);
	    dh = want_aagrid ? degrad(dh) : hrrad(dh);
	    if (dh > pangle/2) {
		xe_msg (1, "Horizontal grid spacing must be < FOV/2");
		return;
	    }
	    nht = (int)floor(pangle/dh + 0.5) + 1; /* inclusive */

	    str = XmTextFieldGetString (vgrid_w);
	    f_scansexa (str, &dv);
	    XtFree (str);
	    dv = degrad(dv);
	    if (dv > sv_dfov/2) {
		xe_msg (1, "Vertical grid spacing must be < FOV/2");
		return;
	    }
	    nvt = (int)floor(sv_dfov/dv + 0.5) + 1; /* inclusive */
	}

	/* round center to nearest whole multiple of step size */
	altdec -= fmod (altdec, dv);
	azra -= fmod (azra, dh);

	/* report */
	fs_sexa (msg, raddeg(dv), 3, 3600);
	XmTextFieldSetString (vgrid_w, msg);
	fs_sexa(msg, want_aagrid ? raddeg(dh) : radhr(dh), 3, 3600);
	XmTextFieldSetString (hgrid_w, msg);
	gridStepLabel();

	/* set up max eq dist, dv down then seg size up towards PI/2 */
	polegap = dv*floor(PI/2/dv - dv/NGSEGS/2) + dv/NGSEGS/2;

	/* do the vertical lines (constant ra or az):
	 * for each horizontal tick mark
	 *   for each vertical tick mark, by NGSEGS
	 *     compute coord on screen
	 *     if we've at least 2 pts now
	 *       connect the points with what is visible within the circle.
	 */
	nht*=2;
	nvt*=2;
	for (i = -nht/2; i <= nht/2; i++) {
	    double h0 = azra + i*dh;
	    int before = 0;
	    int vis1 = 0, vis2;
	    int x1 = 0, y1 = 0, x2, y2;
	    xs = xsegs;
	    for (j = -NGSEGS*nvt/2; j <= NGSEGS*nvt/2; j++) {
		double h = h0, v = altdec + j*dv/NGSEGS;
		if (fabs(v) > polegap)
		    continue;
		if (!samesys)
		    sv_other (v, h0, want_aagrid, &v, &h);
		vis2 = sv_loc(v,h,&x2,&y2);	/* hzn done with clipping */
		if (before++ && (vis1 || vis2)) {
		    int sx1, sy1, sx2, sy2;

		    /* move label away from pole and a little off center */
		    if (want_gridlbl && j == (altdec<0?1:-1)*NGSEGS)
			draw_grid_label(dsp,win,gc,v,h,x2,y2,samesys,1,dv,dh);

		    /* draw segment if visible */
		    if (segisvis(x1, y1, x2, y2, &sx1, &sy1, &sx2, &sy2)) {
			xs->x1 = sx1; xs->y1 = sy1;
			xs->x2 = sx2; xs->y2 = sy2;
			if (++xs == &xsegs[XtNumber(xsegs)]) {
			    split_segs (dsp, win, gc, xsegs, xs - xsegs);
			    xs = xsegs;
			}
		    }
		}
		x1 = x2;
		y1 = y2;
		vis1 = vis2;
	    }
	    if (xs > xsegs)
		split_segs (dsp, win, gc, xsegs, xs - xsegs);
	}

	/* do the horizontal lines (constant dec or alt):
	 * for each vertical tick mark
	 *   for each horizontal tick mark, by NGSEGS
	 *     compute coord on screen
	 *     if we've at least 2 pts now
	 *       connect the points with what is visible within the circle.
	 */
	for (j = -nvt/2; j <= nvt/2; j++) {
	    double v0 = altdec + j*dv;
	    int before = 0;
	    int vis1 = 0, vis2;
	    int x1 = 0, y1 = 0, x2, y2;
	    xs = xsegs;
	    for (i = -NGSEGS*nht/2; i <= NGSEGS*nht/2; i++) {
		double v = v0, h = azra + i*dh/NGSEGS;
		if (fabs(v) > polegap)
		    continue;
		if (!samesys)
		    sv_other (v, h, want_aagrid, &v, &h);
		vis2 = sv_loc(v,h,&x2,&y2);	/* hzn down with clipping */
		if (before++ && (vis1 || vis2)) {
		    int sx1, sy1, sx2, sy2;

		    /* draw label once a little off center */
		    if (want_gridlbl && i == -NGSEGS)
			draw_grid_label(dsp,win,gc,v,h,x2,y2,samesys,0,dv,dh);

		    /* draw segment if visible */
		    if (segisvis(x1, y1, x2, y2, &sx1, &sy1, &sx2, &sy2)) {
			xs->x1 = sx1; xs->y1 = sy1;
			xs->x2 = sx2; xs->y2 = sy2;
			if (++xs == &xsegs[XtNumber(xsegs)]) {
			    split_segs (dsp, win, gc, xsegs, xs - xsegs);
			    xs = xsegs;
			}
		    }
		}
		x1 = x2;
		y1 = y2;
		vis1 = vis2;
	    }
	    if (xs > xsegs)
		split_segs (dsp, win, gc, xsegs, xs - xsegs);
	}
}

/* draw the horizon map.
 */
static void
draw_hznmap (np, dsp, win, gc)
Now *np;
Display *dsp;
Window win;
GC gc;
{
	if (cyl_proj && aa_mode)
	    draw_hznac (dsp, win, gc);
	else
	    draw_hznother (dsp, win, gc);
}

/* draw the filled horizon in aa_mode && cyl_proj.
 * this one is easy because the path to infinity is always parallel to the
 * vertical dimension of the screen.
 */
static void
draw_hznac (dsp, win, gc)
Display *dsp;
Window win;
GC gc;
{
#define	UNIVLEN	2	/* points for wrap around the universe */
	XPoint *xpts;
	short top;
	int x;

	/* get points array, contour plus fill pattern */
	xpts = (XPoint *) XtMalloc ((sv_w+UNIVLEN) * sizeof(XPoint));

	/* walk across the screen */
	for (x = 0; x < sv_w; x++) {
	    double alt, az, halt;
	    int hx, hy;

	    sv_unloc (x, sv_h/2, &alt, &az);
	    halt = hznAlt (az);
	    if (!sv_loc (halt, az, &hx, &hy))
		hy = hy < 0 ? 0 : sv_h;
	    xpts[x].x = x;
	    xpts[x].y = hy;
	}

	/* add path around top (or bottom if flipped) */
	top = (short)(flip_tb ? 0 : sv_h);
	xpts[x].x = sv_w;  xpts[x].y = top;  x++;
	xpts[x].x =    0;  xpts[x].y = top;  x++;

	/* black fill */
	if (want_clipping) {
	    if (XPSDrawing()) {
		XSetForeground (dsp, gc, WhitePixel(dsp, DefaultScreen(dsp)));
		XPSFillPolygon (dsp, win, gc, xpts, sv_w+UNIVLEN, Complex,
								CoordModeOrigin);
	    }
	    XSetForeground (dsp, gc, BlackPixel(dsp, DefaultScreen(dsp)));
	    XFillPolygon (dsp, win, gc, xpts, sv_w+UNIVLEN, Complex,
							    CoordModeOrigin);
	}

	/* crisp boundary */
	draw_hznProfile (dsp, win, gc);

	/* finished */
	XtFree ((char *)xpts);
#undef UNIVLEN
}

/* draw the filled horizon in !aa_mode && cyl_proj.
 * if !cyl_proj then only the horizon border is drawn
 */ 
static void
draw_hznother (dsp, win, gc)
Display *dsp;
Window win;
GC gc;
{
#define	UNIVLEN	2	/* points for wrap around the universe */
	int wx, lx;
	int sv_w_eff = sv_w;
	XPoint *xpts;
	short top;
	int nprofile;
	int npts;
	int i;

	/* get n entries in profile, skip if none */
	nprofile = hznNProfile();
	if (!nprofile)
	    return;

	if (!cyl_proj || !want_clipping) {
	    /* crisp boundary */
	    draw_hznProfile (dsp, win, gc);
	    return;
	}

	/* calculate effective screen width */
	if (sv_hfov > 2 * PI)
	    sv_w_eff = (int)((2 * PI / sv_hfov) * sv_w);

	/* get array for contour */
	xpts = (XPoint *) XtMalloc (((5*nprofile)+UNIVLEN) * sizeof(XPoint));

	/* top/left position for path completion */

	/* find points around entire horizon */
	wx = -1;
	lx = 0;
	npts = 0;
	for (i = 0; i < nprofile; i++) {
	    double altdec, azra;
	    int x, y;

	    /* find x,y */
	    hznProfile (i, &altdec, &azra);
	    if (!aa_mode)
		sv_other (altdec, azra, 1, &altdec, &azra);
	    sv_loc (altdec, azra, &x, &y);

	    /* find the maximum right pixel */
	    if (x > wx)
		wx = x;

	    /* collect and mind the wrap */
	    xpts[npts].x = x;
	    xpts[npts].y = y;
	    npts++;

	    if (npts > 1 && abs(lx-x) > sv_w_eff/2) {
		/* wrapped */
		if (!flip_lr) {
		    x = x + wx - (sv_w - wx);
		} else
		{
		    x = x - wx + (sv_w - wx);
		}
		xpts[npts-1].x = x;
	    }

	    lx = x;
	}

	/* make the horizon wider */
	memcpy(&xpts[npts], xpts, nprofile * sizeof(XPoint));
	memcpy(&xpts[2*npts], xpts, nprofile * sizeof(XPoint));
	memcpy(&xpts[3*npts], xpts, nprofile * sizeof(XPoint));
	memcpy(&xpts[4*npts], xpts, nprofile * sizeof(XPoint));

	int xshift = wx - (sv_w - wx);
	if (flip_lr) {
	    for (i = 0; i < nprofile; i++) {
		xpts[i].x = xpts[i].x + 2 * xshift;
		xpts[i+npts].x = xpts[i+npts].x + xshift;
		xpts[i+3*npts].x = xpts[i+3*npts].x - xshift;
		xpts[i+4*npts].x = xpts[i+4*npts].x - 2 * xshift;
	    }
	} else
	{
	    for (i = 0; i < nprofile; i++) {
		xpts[i].x = xpts[i].x - 2 * xshift;
		xpts[i+npts].x = xpts[i+npts].x - xshift;
		xpts[i+3*npts].x = xpts[i+3*npts].x + xshift;
		xpts[i+4*npts].x = xpts[i+4*npts].x + 2 * xshift;
	    }
	}
	npts = 5 * npts;
	
	/* add path around top (or bottom if flipped) */
	top = (short)(flip_tb ? 0 : sv_h);
	xpts[npts].x = xpts[npts-1].x;  xpts[npts].y = top;  npts++;
	xpts[npts].x =      xpts[0].x;  xpts[npts].y = top;  npts++;
	
	/* black fill */
	if (XPSDrawing()) {
	    XSetForeground (dsp, gc, WhitePixel(dsp, DefaultScreen(dsp)));
	    XPSFillPolygon (dsp, win, gc, xpts, npts, Complex, CoordModeOrigin);
	}
	XSetForeground (dsp, gc, BlackPixel(dsp, DefaultScreen(dsp)));
	XFillPolygon (dsp, win, gc, xpts, npts, Complex, CoordModeOrigin);

	/* finished */
	XtFree ((char *)xpts);

	/* crisp boundary */
	draw_hznProfile (dsp, win, gc);
#undef UNIVLEN
}

/* draw the horizon profile in any mode */
static void
draw_hznProfile (dsp, win, gc)
Display *dsp;
Window win;
GC gc;
{
	int lx, ly, lv;
	int sv_w_eff = sv_w;
	XPoint *xpts;
	int nprofile;
	int npts;
	int i;

	/* get n entries in profile, skip if none */
	nprofile = hznNProfile();
	if (!nprofile)
	    return;

	/* calculate effective screen width */
	if (sv_hfov > 2 * PI)
	    sv_w_eff = (int)((2 * PI / sv_hfov) * sv_w);

	/* get array for contour */
	xpts = (XPoint *) XtMalloc (nprofile * sizeof(XPoint));

	/* find points around entire horizon, draw at exit/entry */
	XSetForeground (dsp, gc, hzn_p);
	lx = ly = lv = 0;
	npts = 0;
	for (i = 0; i < nprofile; i++) {
	    double altdec, azra;
	    int x, y, v;

	    /* find x,y and whether visible */
	    hznProfile (i, &altdec, &azra);
	    if (!aa_mode)
		sv_other (altdec, azra, 1, &altdec, &azra);
	    v = sv_loc (altdec, azra, &x, &y);

	    /* collect and break cleanly where it exits/enters or wraps */
	    xpts[npts].x = x;
	    xpts[npts].y = y;
	    npts++;
	    if (v || lv) {
		if (v != lv && npts > 1) {
		    /* just became visible */
		    int x1, y1, x2, y2;
		    if (segisvis (lx, ly, x, y, &x1, &y1, &x2, &y2)) {
			xpts[npts-2].x = (short)x1;
			xpts[npts-2].y = (short)y1;
			xpts[npts-1].x = (short)x2;
			xpts[npts-1].y = (short)y2;
		    } else
			npts -= 2;
		}
		if (!v && lv) {
		    /* just went non-visible so draw and restart */
		    XPSDrawLines (dsp, win, gc, xpts, npts,CoordModeOrigin);
		    npts = 0;
		}
		if (npts > 1 && abs(lx-x) > sv_w_eff/2) {
		    /* wrapped */
		    XPSDrawLines (dsp, win, gc, xpts, npts-1, CoordModeOrigin);
		    xpts[0] = xpts[npts-1];
		    npts = 1;
		}
	    } else
		npts = 1;

	    lx = x;
	    ly = y;
	    lv = v;
	}

	/* remaining boundary */
	XPSDrawLines (dsp, win, gc, xpts, npts, CoordModeOrigin);

	/* finished */
	XtFree ((char *)xpts);
}

/* return whether op is visible with respect to horizon choices */
int
sv_hznOpOk (Obj *op)
{
	return (!want_clipping || !want_hznmap || op->s_alt >= hznAlt(op->s_az));
}

/* draw compass */
static void
draw_compass (dsp, win)
Display *dsp;
Drawable win;
{
#define	COMPASSHH	25	/* compass half-height, pixels */
	XCharStruct xcs;	/* char metrics */
	int dir, asc, des;	/* more char metrics */
	int chh, chw;		/* char half-height and half-width */
	double minpole;		/* min pole distance, rads */
	int dxu=0, dyu=0;	/* step up */
	int dxr=0, dyr=0;	/* step right */
	int dxn=0, dyn=0;	/* step north */
	int dxe=0, dye=0;	/* step east */
	int xaa0=0, yaa0=0;	/* alt/az cross center */
	int xrd0=0, yrd0=0;	/* ra/dec cross center */
	int x0, y0;		/* image center */
	double step;		/* COMPASSHH in rads */
	double altdec, azra;	/* center in opposite coords */
	int aaok, rdok;		/* whether ok, ie, away from pole */
	double r;		/* normalization radius */

	/* basic size based upon font */
	XSetFont (dsp, sv_strgc, sv_pf->fid);
	XSetForeground (dsp, sv_strgc, annot_p);
	XTextExtents (sv_pf, "E", 1, &dir, &asc, &des, &xcs);
	chh = xcs.ascent/2;
	chw = xcs.width/2;

	/* find step north, east, up and right directions */
	sv_other (sv_altdec, sv_azra, aa_mode, &altdec, &azra);
	step = sv_dfov/8;
	minpole = PI/2-1.1*step;	/* don't step over pole */
	x0 = sv_w/2;
	y0 = sv_h/2;
	if (aa_mode) {
	    aaok = (fabs(sv_altdec) < minpole);
	    if (aaok) {
		/* U-D always vertical */
		dxu = 0;
		dyu = flip_tb ? COMPASSHH : -COMPASSHH;

		/* L-R always horizontal */
		dxr = flip_lr ? -COMPASSHH/2 : COMPASSHH/2;
		dyr = 0;

		xaa0 = sv_w - 3*COMPASSHH/2;
		yaa0 = sv_h - 3*COMPASSHH/2;
	    }

	    rdok = (fabs(altdec) < minpole);
	    if (rdok) {
		double a, b;
		int x, y;

		sv_other (altdec+step, azra, !aa_mode, &a, &b);
		sv_loc (a, b, &x, &y);
		dxn = x - x0;
		dyn = y - y0;
		r = sqrt((double)dxn*dxn + (double)dyn*dyn);
		dxn = (int)floor(COMPASSHH * dxn / r + 0.5);
		dyn = (int)floor(COMPASSHH * dyn / r + 0.5);

		/* E-W always right angles to N-S */
		dxe = flip_tb ^ flip_lr ? -dyn/2 : dyn/2;
		dye = flip_tb ^ flip_lr ? dxn/2 : -dxn/2;

		xrd0 = sv_w - 9*COMPASSHH/2;
		yrd0 = sv_h - 3*COMPASSHH/2;
	    }
	} else {
	    int x, y;

	    rdok = (fabs(sv_altdec) < minpole);
	    if (rdok) {
		if (si_ison()) {
		    sv_loc (sv_altdec+step, sv_azra, &x, &y);
		    dxn = x - x0;
		    dyn = y - y0;
		    r = sqrt((double)dxn*dxn + (double)dyn*dyn);
		    dxn = (int)floor(COMPASSHH * dxn / r + 0.5);
		    dyn = (int)floor(COMPASSHH * dyn / r + 0.5);

		    sv_loc (sv_altdec, sv_azra+step/cos(sv_altdec), &x, &y);
		    dxe = (x > x0) ? abs(dyn)/2 : -abs(dyn)/2;
		    dye = (y > y0) ? abs(dxn)/2 : -abs(dxn)/2;
		} else {
		    /* N-S always vertical */
		    dxn = 0;
		    dyn = flip_tb ? COMPASSHH : -COMPASSHH;

		    /* E-W always horizontal */
		    dxe = flip_lr ? COMPASSHH/2 : -COMPASSHH/2;
		    dye = 0;
		}

		xrd0 = sv_w - 3*COMPASSHH/2;
		yrd0 = sv_h - 3*COMPASSHH/2;
	    }

	    aaok = (fabs(altdec) < minpole);
	    if (aaok) {
		double r, d;

		sv_other (altdec+step, azra, !aa_mode, &r, &d);
		sv_loc (r, d, &x, &y);
		dxu = x - x0;
		dyu = y - y0;
		r = sqrt((double)dxu*dxu + (double)dyu*dyu);
		dxu = (int)floor(COMPASSHH * dxu / r + 0.5);
		dyu = (int)floor(COMPASSHH * dyu / r + 0.5);

		if (si_ison()) {
		    sv_other (altdec, azra+step/cos(altdec), !aa_mode,&r,&d);
		    sv_loc (r, d, &x, &y);
		    dxr = x - x0;
		    dyr = y - y0;
		    r = sqrt((double)dxr*dxr + (double)dyr*dyr);
		    dxr = (int)floor(COMPASSHH/2 * dxr / r + 0.5);
		    dyr = (int)floor(COMPASSHH/2 * dyr / r + 0.5);
		} else {
		    /* R-L always at right angles to U-D */
		    dxr = flip_tb ^ flip_lr ? dyu/2 : -dyu/2;
		    dyr = flip_tb ^ flip_lr ? -dxu/2 : dxu/2;
		}

		xaa0 = sv_w - 9*COMPASSHH/2;
		yaa0 = sv_h - 3*COMPASSHH/2;
	    }
	}

	/* draw each rose, if known */
	if (aaok) {
	    XPSDrawLine (dsp,win,sv_strgc,xaa0-dxu,yaa0-dyu,xaa0+dxu,yaa0+dyu);
	    r = sqrt ((double)dxu*dxu + (double)dyu*dyu);
	    r = (r + 3*chh)/r;
	    dxu = (int)floor(r*dxu + 0.5);
	    dyu = (int)floor(r*dyu + 0.5);
	    XPSDrawString (dsp, win, sv_strgc, xaa0+dxu-chw,yaa0+dyu+chh,"Z",1);

	    XPSDrawLine (dsp,win,sv_strgc,xaa0-dxr,yaa0-dyr,xaa0+dxr,yaa0+dyr);
	    r = sqrt ((double)dxr*dxr + (double)dyr*dyr);
	    r = (r + 3*chh)/r;
	    dxr = (int)floor(r*dxr + 0.5);
	    dyr = (int)floor(r*dyr + 0.5);
	    XPSDrawString (dsp, win, sv_strgc, xaa0+dxr-chw,yaa0+dyr+chh,"R",1);
	}

	if (rdok) {
	    XPSDrawLine (dsp,win,sv_strgc,xrd0-dxn,yrd0-dyn,xrd0+dxn,yrd0+dyn);
	    r = sqrt ((double)dxn*dxn + (double)dyn*dyn);
	    r = (r + 3*chh)/r;
	    dxn = (int)floor(r*dxn + 0.5);
	    dyn = (int)floor(r*dyn + 0.5);
	    XPSDrawString (dsp, win, sv_strgc, xrd0+dxn-chw,yrd0+dyn+chh,"N",1);

	    XPSDrawLine (dsp,win,sv_strgc,xrd0-dxe,yrd0-dye,xrd0+dxe,yrd0+dye);
	    r = sqrt ((double)dxe*dxe + (double)dye*dye);
	    r = (r + 3*chh)/r;
	    dxe = (int)floor(r*dxe + 0.5);
	    dye = (int)floor(r*dye + 0.5);
	    XPSDrawString (dsp, win, sv_strgc, xrd0+dxe-chw,yrd0+dye+chh,"E",1);
	}
}


/* draw the ecliptic */
static void
draw_ecliptic(np, dsp, win, gc)
Now *np;
Display *dsp;
Window win;
GC gc;
{
	XPoint ptcache[100];
        double elat0, elng0;    /* ecliptic lat and long at center of fov */
	double elngmin, elngmax;/* ecliptic long limits */
	double ra, dec;
	double altdec, azra;
	double elng;
	double lst;
	int ncache;
	int x, y;
	int on, n;

	now_lst (np, &lst);
	XSetFont (dsp, sv_strgc, sv_pf->fid);
	XSetForeground (dsp, sv_strgc, eq_p);

	/* poles */

	ecl_eq (mjd, PI/2, 0.0, &azra, &altdec);
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "NEP");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}
	ecl_eq (mjd, -PI/2, 0.0, &azra, &altdec);
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "SEP");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}

	/* some points of interest on the ecliptic */

	ecl_eq (mjd, 0.0, 0.0, &azra, &altdec);
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "VEq");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}
	ecl_eq (mjd, 0.0, PI, &azra, &altdec);
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "AEq");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}
	ecl_eq (mjd, 0.0, PI/2., &azra, &altdec);
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "SS");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}
	ecl_eq (mjd, 0.0, -PI/2., &azra, &altdec);
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "WS");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}

	/* find equatorial coords of center of view */

	if (aa_mode) {
	    double ha0;		/* local hour angle */
	    aa_hadec (lat, sv_altdec, sv_azra, &ha0, &dec);
	    ra = hrrad(lst) - ha0;
	} else {
	    ra = sv_azra;
	    dec = sv_altdec;
	}
	eq_ecl (mjd, ra, dec, &elat0, &elng0);

	/* no ecliptic visible if ecliptic latitude at center of view 
	 * is more than the window diagonal.
	 */
	if (fabs(elat0) >= sv_dfov)
	    return;

	/* worst-case elong limits is center elong += half size unless cyl */
	if (cyl_proj) {
	    elngmin = elng0 - PI;
	    elngmax = elng0 + PI;
	} else {
	    elngmin = elng0 - sv_dfov/2.0;
	    elngmax = elng0 + sv_dfov/2.0;
	}

	/* draw dashed line */
	ncache = 0;
	on = 1;
	n = 0;
	for (elng = elngmin; elng <= elngmax; elng += sv_vfov/sv_h) {

	    /* dashed pattern */
	    if (on && n == ECL_NON)
		on = 0, n = 0;
	    else if (!on && n == ECL_NOFF)
		on = 1, n = 0;
	    n++;
	    if (!on)
		continue;

	    /* convert longitude along the ecliptic to ra/dec */
	    ecl_eq (mjd, 0.0, elng, &azra, &altdec);

	    /* if in aa mode, we need it in alt/az */
	    if (aa_mode)
		sv_other (altdec, azra, 0, &altdec, &azra);

	    /* if visible, display point */
	    if (sv_loc (altdec, azra, &x, &y)) {
		XPoint *xp = &ptcache[ncache++];
                xp->x = x;
		xp->y = y;

		if (ncache == XtNumber(ptcache)) {
		    XPSDrawPoints(dsp,win,gc,ptcache,ncache,CoordModeOrigin);
		    ncache = 0;
		}
	    }
	}

	if (ncache > 0)
	    XPSDrawPoints (dsp, win, gc, ptcache, ncache, CoordModeOrigin);
}

/* draw the [pe]numbra, and throw in the anti-solar point too */
static void
draw_umbra(np, dsp, win, gc)
Now *np;
Display *dsp;
Window win;
GC gc;
{
	Obj *sop, *mop;
	double altdec, azra;
	double asra, asdec;
	int x, y;

	/* mark the far-field anti-solar point */
	sop = db_basic (SUN);
	mop = db_basic (MOON);
	asra = sop->s_ra + PI;
	asdec = -sop->s_dec;
	azra = asra;
	altdec = asdec;
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
            XPSDrawLine (dsp, win, gc, x-ASR, y, x+ASR, y);
	    XPSDrawLine (dsp, win, gc, x, y-ASR, x, y+ASR);
	    XPSDrawArc (dsp, win, gc, x-ASR, y-ASR, 2*ASR, 2*ASR, 0, 64*360);
	}

	/* mark the umbra/penumbra.
	 * account for parallax if showing topocentric
	 */
	if (pref_get(PREF_EQUATORIAL) == PREF_TOPO) {
	    double ha_in, ha_out;
	    double lst, rho_topo;
	    now_lst (np, &lst);
	    ha_in = hrrad(lst) - asra;
	    rho_topo = mop->s_edist * MAU/ERAD;    /* convert to earth radii */
	    ta_par (ha_in, asdec, lat, elev, &rho_topo, &ha_out, &asdec);
	    asra -= ha_out - ha_in;
	    azra = asra;
	    altdec = asdec;
	    if (aa_mode)
		sv_other (altdec, azra, 0, &altdec, &azra);
	}

	if (sv_loc (altdec, azra, &x, &y)) {
	    double rsn = sop->s_edist;
	    double rmn = mop->s_edist;
	    double shadow0 = ERAD + rmn / rsn * (ERAD - SRAD); /* umbra */
	    double shadow1 = ERAD + rmn / rsn * (ERAD + SRAD); /* penumbra */
	    double urad = asin(shadow0/MAU/rmn);
	    double prad = asin(shadow1/MAU/rmn);
	    int upix = (int)floor(urad/(sv_vfov/sv_h) + 0.5);
	    int ppix = (int)floor(prad/(sv_vfov/sv_h) + 0.5);

	    XPSDrawArc (dsp, win, gc, x-upix,y-upix,2*upix+1,2*upix+1,0,64*360);
	    XSetLineAttributes (XtD, gc, 0, LineOnOffDash, CapButt, JoinMiter);
	    XPSDrawArc (dsp, win, gc, x-ppix,y-ppix,2*ppix+1,2*ppix+1,0,64*360);
	    XSetLineAttributes (XtD, gc, 0, LineSolid, CapButt, JoinMiter);
	}
}

/* draw the equator */
static void
draw_equator(np, dsp, win, gc)
Now *np;
Display *dsp;
Window win;
GC gc;
{
	XPoint ptcache[100];
	double alt0, az0, ra0, dec0;
	double ramin, ramax;	/* display limits */
	double ra;
	double altdec, azra;
	double lst;
	int ncache;
	int on, n;
	int x, y;

	now_lst (np, &lst);
	XSetFont (dsp, sv_strgc, sv_pf->fid);
	XSetForeground (dsp, sv_strgc, eq_p);

	/* first the poles as little crosses with labels */
	azra = 0.;
	altdec = PI/2.;
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "NCP");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}
	azra = 0.;
	altdec = -PI/2.;
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "SCP");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}

	/* no equator visible if dec at center of view 
	 * is not less than half the window diagonal.
	 */
	sv_fullwhere (np, sv_altdec, sv_azra, aa_mode, &alt0, &az0, &ra0,&dec0);
	if (fabs(dec0) >= sv_dfov/2.0)
	    return;

	/* worst-case limits is center += half diag size unless cyl */
	if (cyl_proj) {
	    ramin = ra0 - PI;
	    ramax = ra0 + PI;
	} else {
	    ramin = ra0 - sv_dfov/2.0;
	    ramax = ra0 + sv_dfov/2.0;
	}

	/* draw dashed line */
	ncache = 0;
	on = 1;
	n = 0;
	for (ra = ramin; ra <= ramax; ra += sv_vfov/sv_h) {
	    double altdec = 0.0, azra = ra;
	    int x, y;

	    /* dashed pattern */
	    if (on && n == EQ_NON)
		on = 0, n = 0;
	    else if (!on && n == EQ_NOFF)
		on = 1, n = 0;
	    n++;
	    if (!on)
		continue;

	    /* if in aa mode, we need it in alt/az */
	    if (aa_mode)
		sv_other (altdec, azra, 0, &altdec, &azra);

	    /* if visible, display point */
	    if (sv_loc (altdec, azra, &x, &y)) {
		XPoint *xp = &ptcache[ncache++];
                xp->x = x;
		xp->y = y;

		if (ncache == XtNumber(ptcache)) {
		    XPSDrawPoints(dsp,win,gc,ptcache,ncache,CoordModeOrigin);
		    ncache = 0;
		}
	    }
	}

	if (ncache > 0)
	    XPSDrawPoints (dsp, win, gc, ptcache, ncache, CoordModeOrigin);
}

/* draw the galactic plane and poles */
static void
draw_galactic(np, dsp, win, gc)
Now *np;
Display *dsp;
Window win;
GC gc;
{
	XPoint ptcache[100];
	double e = epoch == EOD ? mjd : epoch;
        double glat0, glng0;    /* galactic lat and long at center of fov */
	double glngmin, glngmax;/* galactic long limits */
	double altdec, azra;
	double ra, dec;
	double glng;
	double lst;
	int x, y;
	int ncache;
	int on, n;

	now_lst (np, &lst);
	XSetFont (dsp, sv_strgc, sv_pf->fid);
	XSetForeground (dsp, sv_strgc, eq_p);

	/* first the poles and center as little crosses with labels */
	gal_eq (e, PI/2, 0.0, &azra, &altdec);
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "NGP");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}
	gal_eq (e, -PI/2, 0.0, &azra, &altdec);
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "SGP");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}
	gal_eq (e, 0.0, 0.0, &azra, &altdec);
	if (aa_mode)
	    sv_other (altdec, azra, 0, &altdec, &azra);
	if (sv_loc (altdec, azra, &x, &y)) {
	    int dir, asc, dsc;
	    XCharStruct all;
	    char buf[32];
	    int l;

	    XPSDrawLine(dsp, win, gc, x-GAL_W, y, x+GAL_W, y);
	    XPSDrawLine(dsp, win, gc, x, y-GAL_W, x, y+GAL_W);

	    l = sprintf (buf, "%s", "GC");
	    XQueryTextExtents (dsp, XGContextFromGC(sv_strgc), buf, l,
							&dir, &asc, &dsc, &all);
	    XPSDrawString (dsp, win, sv_strgc, x-all.width/2, y-GAL_W-2, buf,l);
	}

	/* now the equator */

	/* find ecliptic coords of center of view */
	if (aa_mode) {
	    double ha0;		/* local hour angle */
	    aa_hadec (lat, sv_altdec, sv_azra, &ha0, &dec);
	    ra = hrrad(lst) - ha0;
	} else {
	    ra = sv_azra;
	    dec = sv_altdec;
	}
	eq_gal (e, ra, dec, &glat0, &glng0);

	/* no galactic eq visible if galactic latitude at center of view 
	 * is not less than diagonal window radius.
	 */
	if (fabs(glat0) >= sv_dfov/2.0)
	    return;

	/* worst-case glng limits is center glng += half win size unless cyl */
	if (cyl_proj) {
	    glngmin = glng0 - PI;
	    glngmax = glng0 + PI;
	} else {
	    glngmin = glng0 - sv_dfov/2;
	    glngmax = glng0 + sv_dfov/2;
	}

	/* draw dashed line */
	ncache = 0;
	on = 1;
	n = 0;
	for (glng = glngmin; glng <= glngmax; glng += sv_vfov/sv_h) {

	    /* dashed pattern */
	    if (on && n == GAL_NON)
		on = 0, n = 0;
	    else if (!on && n == GAL_NOFF)
		on = 1, n = 0;
	    n++;
	    if (!on)
		continue;

	    /* convert longitude along the galactic eq to ra/dec */
	    gal_eq (e, 0.0, glng, &azra, &altdec);

	    /* if in aa mode, we need it in alt/az */
	    if (aa_mode)
		sv_other (altdec, azra, 0, &altdec, &azra);

	    /* if visible, display point */
	    if (sv_loc (altdec, azra, &x, &y)) {
		XPoint *xp = &ptcache[ncache++];
                xp->x = x;
		xp->y = y;

		if (ncache == XtNumber(ptcache)) {
		    XPSDrawPoints(dsp,win,gc,ptcache,ncache,CoordModeOrigin);
		    ncache = 0;
		}
	    }
	}

	if (ncache > 0)
	    XPSDrawPoints (dsp, win, gc, ptcache, ncache, CoordModeOrigin);
}

/* draw the Milky Way */
/* Pertti Paakkonen, Nov. 24, 2003 */
static void
draw_milkyway(np, dsp, win, gc)
Now *np;
Display *dsp;
Window win;
GC gc;
{
	/* rough presentation of the Milky Way edges */
	static float mw1[257][2] = {
	    {180.0,-1.1}, {178.5,-1.6}, {177.1,-2.0}, {174.1,-1.7}, 
	    {172.7,-1.1}, {171.0,-1.6}, {170.3,-1.2}, {170.2,-0.3}, 
	    {169.5,-0.3}, {169.0,0.3}, {167.9,-0.0}, {165.9,-0.3}, 
	    {163.4,-1.2}, {162.0,-2.8}, {161.1,-5.1},  {160.8,-6.4}, 
	    {159.5,-6.6}, {158.0,-6.2}, {155.7,-5.7}, {153.7,-6.1}, 
	    {153.6,-6.8}, {151.8,-6.7}, {148.9,-7.5}, {146.3,-8.4}, 
	    {144.7,-9.8}, {142.8,-10.0}, {141.9,-9.2}, {140.9,-7.7}, 
	    {139.5,-7.4}, {138.6,-8.2},  {137.6,-7.8}, {136.3,-7.3}, 
	    {135.0,-6.5}, {134.1,-6.6}, {134.0,-7.8}, {133.1,-7.8}, 
	    {132.0,-8.1}, {131.7,-7.6}, {130.1,-6.8}, {129.5,-5.9}, 
	    {128.3,-5.3}, {126.0,-5.6}, {124.3,-5.6}, {123.0,-5.9}, 
	    {121.1,-7.2},  {119.3,-7.7}, {118.8,-8.7}, {118.0,-8.7}, 
	    {117.7,-7.9}, {116.8,-7.8}, {115.7,-8.8}, {114.3,-10.2}, 
	    {112.5,-12.0}, {111.2,-12.1}, {109.7,-11.5}, {108.1,-10.7}, 
	    {106.4,-9.5}, {104.2,-8.7}, {102.1,-8.7}, {100.9,-8.4},  
	    {100.2,-7.3}, {98.8,-6.8}, {97.1,-7.0}, {95.1,-7.8}, {93.7,-7.9}, 
	    {93.2,-7.5}, {92.3,-7.0}, {90.3,-8.1}, {90.4,-9.3}, {90.0,-10.3}, 
	    {88.5,-11.1}, {86.8,-10.7}, {84.3,-10.7}, {83.5,-11.7}, 
	    {82.4,-11.8},  {80.3,-11.2}, {78.1,-10.9}, {76.8,-11.1}, 
	    {74.8,-11.1}, {73.9,-10.6}, {72.3,-11.0}, {69.1,-10.7}, 
	    {65.3,-11.0}, {63.0,-11.0}, {61.3,-9.3}, {60.0,-9.2}, 
	    {58.3,-9.8}, {56.9,-9.2}, {55.5,-9.7}, {53.6,-10.4},  
	    {52.4,-10.4}, {51.1,-9.6}, {50.2,-9.9}, {48.9,-10.0}, 
	    {45.7,-9.3}, {43.1,-8.9}, {41.3,-8.6}, {39.7,-7.9}, {38.5,-7.0}, 
	    {37.9,-7.3}, {37.6,-8.8}, {36.7,-9.3}, {34.8,-8.6}, {34.6,-6.9}, 
	    {33.9,-6.1},  {33.1,-6.3}, {33.2,-8.3}, {32.6,-10.2}, 
	    {30.8,-11.0}, {27.8,-11.2}, {25.7,-11.5}, {24.5,-12.4}, 
	    {24.1,-14.0}, {22.2,-14.4}, {19.2,-14.2}, {16.9,-13.8}, 
	    {14.9,-14.0}, {13.3,-13.1}, {12.0,-11.5}, {11.0,-10.3},  
	    {10.2,-10.4}, {10.2,-12.3}, {10.4,-14.5}, {9.2,-17.1}, 
	    {6.9,-18.3}, {3.7,-18.0}, {1.6,-17.1}, {-0.3,-16.8}, 
	    {-2.2,-16.2}, {-5.0,-15.0}, {-8.0,-13.2}, {-10.4,-11.3}, 
	    {-13.1,-10.3}, {-15.8,-10.5}, {-18.5,-11.7},  {-21.1,-12.3}, 
	    {-24.0,-11.8}, {-26.2,-11.4}, {-27.5,-10.4}, {-29.4,-9.8}, 
	    {-31.5,-10.1}, {-33.5,-11.4}, {-36.4,-12.2}, {-39.6,-12.1}, 
	    {-41.2,-11.5}, {-42.6,-9.8}, {-44.9,-7.9}, {-47.1,-5.8}, 
	    {-49.6,-5.4}, {-51.9,-7.2},  {-53.0,-9.3}, {-53.9,-10.9}, 
	    {-55.3,-11.3}, {-56.6,-10.1}, {-58.2,-9.3}, {-59.8,-8.9}, 
	    {-59.9,-8.4}, {-58.9,-8.1}, {-59.3,-7.6}, {-62.2,-7.8}, 
	    {-63.9,-9.0}, {-65.7,-10.4}, {-67.8,-10.3}, {-70.0,-9.6}, 
	    {-72.3,-8.9},  {-74.2,-8.9}, {-75.3,-8.6}, {-74.9,-7.6}, 
	    {-74.3,-6.5}, {-74.8,-6.3}, {-76.2,-7.2}, {-77.7,-7.3}, 
	    {-80.4,-7.0}, {-82.7,-6.4}, {-84.4,-6.1}, {-86.3,-5.5}, 
	    {-88.2,-5.0}, {-89.7,-4.4}, {-90.6,-3.3}, {-91.5,-3.0},  
	    {-93.2,-3.3}, {-95.3,-3.0}, {-96.5,-2.2}, {-96.5,-2.8}, 
	    {-96.0,-3.9}, {-94.8,-5.1}, {-95.5,-6.9}, {-97.7,-8.8}, 
	    {-99.0,-8.9}, {-100.1,-7.8}, {-100.0,-7.2}, {-98.7,-7.4}, 
	    {-98.6,-6.9}, {-100.8,-6.2}, {-102.9,-4.7},  {-104.7,-4.5}, 
	    {-106.7,-4.0}, {-106.4,-3.0}, {-104.2,-2.2}, {-104.3,-1.2}, 
	    {-106.6,-0.3}, {-107.6,0.6}, {-108.2,1.9}, {-109.9,2.2}, 
	    {-108.9,1.4}, {-109.0,0.9}, {-112.3,1.2}, {-113.6,1.9}, 
	    {-114.9,2.0}, {-115.5,1.1},  {-117.6,0.9}, {-118.6,0.3}, 
	    {-121.9,-0.4}, {-124.9,-0.8}, {-126.4,-0.5}, {-127.7,-1.1}, 
	    {-127.9,-3.1}, {-129.4,-3.6}, {-130.6,-3.3}, {-131.0,-2.1}, 
	    {-132.3,-2.1}, {-133.1,-3.1}, {-134.2,-3.7}, {-135.7,-2.4}, 
	    {-137.3,-1.8},  {-138.2,-0.3}, {-138.6,1.3}, {-141.1,1.9}, 
	    {-143.5,1.7}, {-145.1,1.2}, {-146.3,-0.2}, {-147.3,-0.7}, 
	    {-147.6,-2.0}, {-147.1,-3.0}, {-148.1,-3.4}, {-150.3,-3.0}, 
	    {-152.6,-2.8}, {-155.8,-3.1}, {-158.7,-3.0}, {-161.3,-3.6},  
	    {-163.0,-4.4}, {-163.9,-5.3}, {-165.8,-5.9}, {-167.3,-6.0}, 
	    {-168.8,-5.4}, {-168.4,-6.3}, {-169.9,-7.0}, {-171.4,-7.9}, 
	    {-172.8,-8.0}, {-173.6,-7.0}, {-173.9,-4.0}, {-174.6,-2.9}, 
	    {-174.4,-1.4}, {-174.8,-0.3}, {-176.6,-0.3},  {-178.2,-1.1}, 
	    {-180.0,-1.1}
	};

	static float mw2[537][2] = {
	    {-180.0,9.2}, {-178.2,8.6}, {-177.4,7.1}, {-176.0,5.9}, 
	    {-174.2,5.4}, {-173.7,5.7}, {-173.8,6.3}, {-175.2,7.1}, 
	    {-175.7,8.4}, {-175.4,9.5}, {-174.4,10.3}, {-174.0,11.5}, 
	    {-173.7,12.7}, {-172.9,13.1}, {-171.9,12.5},  {-171.9,11.3}, 
	    {-171.2,10.9}, {-170.1,10.6}, {-169.2,10.2}, {-168.4,9.5}, 
	    {-167.6,9.3}, {-166.4,9.9}, {-166.2,10.4}, {-164.9,10.7}, 
	    {-164.2,11.1}, {-163.6,10.9}, {-163.5,10.3}, {-163.9,9.6}, 
	    {-163.2,9.0}, {-161.8,8.4},  {-161.1,8.6}, {-160.7,9.6}, 
	    {-160.0,10.2}, {-158.8,10.2}, {-158.0,9.5}, {-157.6,8.3}, 
	    {-157.5,6.9}, {-157.1,6.7}, {-156.6,7.0}, {-156.0,8.4}, 
	    {-155.5,8.7}, {-154.2,8.7}, {-153.0,8.5}, {-152.1,8.9}, 
	    {-151.5,8.9},  {-151.1,7.9}, {-150.2,7.7}, {-149.5,8.2}, 
	    {-149.7,9.2}, {-150.2,10.3}, {-150.7,11.7}, {-150.2,12.2}, 
	    {-148.6,11.8}, {-147.3,11.6}, {-146.7,11.0}, {-146.1,10.0}, 
	    {-145.1,9.9}, {-144.5,10.4}, {-144.6,11.7}, {-143.9,12.1},  
	    {-142.8,11.8}, {-141.1,11.2}, {-139.0,10.9}, {-136.8,10.0}, 
	    {-134.3,8.7}, {-133.0,7.4}, {-132.5,6.5}, {-131.4,5.8}, 
	    {-130.3,5.9}, {-129.1,6.6}, {-127.8,7.8}, {-126.1,8.9}, 
	    {-124.4,9.9}, {-122.7,10.7}, {-120.3,11.5},  {-117.7,11.5}, 
	    {-115.4,11.4}, {-114.1,10.7}, {-113.3,10.0}, {-112.3,10.0}, 
	    {-111.0,10.9}, {-109.4,11.6}, {-108.0,12.0}, {-106.6,11.8}, 
	    {-105.1,10.7}, {-103.6,9.0}, {-101.8,7.8}, {-99.2,6.5}, 
	    {-96.9,5.3}, {-95.5,4.7},  {-94.8,3.1}, {-93.6,2.0}, {-92.3,1.7}, 
	    {-90.9,2.2}, {-90.2,1.9}, {-90.5,0.4}, {-90.5,-0.3}, 
	    {-89.8,-0.9}, {-88.2,-0.9}, {-86.9,0.0}, {-87.0,0.8}, 
	    {-87.8,1.7}, {-87.9,2.9}, {-87.3,3.7}, {-87.8,4.7},  {-87.5,6.0}, 
	    {-86.9,6.9}, {-86.9,7.5}, {-87.3,8.6}, {-86.4,10.1}, 
	    {-84.6,11.1}, {-83.5,11.0}, {-82.0,10.4}, {-79.3,10.4}, 
	    {-77.8,9.6}, {-77.5,8.9}, {-77.2,7.9}, {-76.4,7.6}, {-74.8,8.7}, 
	    {-73.7,10.5},  {-72.3,11.8}, {-70.8,12.5}, {-69.9,12.3}, 
	    {-70.4,11.2}, {-70.4,10.7}, {-70.8,10.5}, {-70.7,10.0}, 
	    {-69.7,10.0}, {-68.2,10.7}, {-66.3,11.7}, {-64.4,12.5}, 
	    {-62.4,12.9}, {-62.0,12.5}, {-63.1,11.5}, {-63.0,10.6},  
	    {-62.3,10.3}, {-60.3,11.2}, {-57.9,11.8}, {-57.2,11.5}, 
	    {-57.5,10.8}, {-59.8,9.6}, {-61.1,8.8}, {-61.8,7.8}, {-61.4,7.5}, 
	    {-59.2,7.9}, {-56.0,8.7}, {-52.7,9.7}, {-49.7,10.0}, {-46.6,9.9}, 
	    {-44.7,9.2},  {-44.6,7.8}, {-43.7,7.0}, {-42.9,7.3}, {-42.2,8.3}, 
	    {-41.6,9.2}, {-40.1,8.9}, {-38.8,9.2}, {-38.3,10.1}, 
	    {-39.0,11.5}, {-38.8,13.2}, {-38.0,13.8}, {-37.1,12.8}, 
	    {-36.6,10.5}, {-36.4,8.9}, {-34.7,8.4},  {-31.4,8.6}, 
	    {-28.6,9.8}, {-28.7,10.6}, {-29.8,11.6}, {-30.0,12.9}, 
	    {-28.4,14.4}, {-27.0,15.5}, {-26.3,16.9}, {-25.2,16.8}, 
	    {-24.8,13.7}, {-24.4,12.0}, {-23.5,11.7}, {-21.9,12.0}, 
	    {-19.7,12.5}, {-18.3,11.8},  {-18.2,10.7}, {-20.4,9.0}, 
	    {-22.0,7.8}, {-24.1,7.1}, {-25.5,5.8}, {-26.7,5.3}, {-27.6,3.6}, 
	    {-28.8,3.1}, {-29.4,1.9}, {-29.4,1.0}, {-30.0,0.6}, {-31.1,0.9}, 
	    {-32.5,0.8}, {-34.5,0.6}, {-37.2,1.3},  {-39.0,1.9}, {-41.0,3.4}, 
	    {-42.5,3.6}, {-43.2,2.6}, {-43.0,0.6}, {-42.3,0.0}, {-40.2,0.1}, 
	    {-38.2,-0.2}, {-36.5,-1.0}, {-35.3,-1.8}, {-33.7,-1.6}, 
	    {-32.7,-1.2}, {-30.5,-1.2}, {-28.2,-0.3}, {-27.2,1.4},  
	    {-24.4,1.8}, {-23.0,2.4}, {-21.9,2.9}, {-19.9,2.8}, {-18.2,2.2}, 
	    {-17.4,3.0}, {-17.4,3.8}, {-16.6,5.2}, {-15.8,7.2}, {-14.6,8.4}, 
	    {-12.9,8.9}, {-11.8,8.6}, {-11.2,7.9}, {-10.2,7.8}, {-10.1,8.4},  
	    {-10.1,9.0}, {-8.8,10.1}, {-7.0,11.7}, {-5.9,12.3}, {-4.2,12.3}, 
	    {-2.7,13.3}, {-1.4,13.5}, {-0.5,13.4}, {0.1,13.9}, {0.9,14.3}, 
	    {2.5,14.3}, {3.7,13.9}, {4.9,13.1}, {7.1,12.1}, {7.8,11.9},  
	    {9.3,10.6}, {10.1,10.3}, {10.3,11.1}, {10.8,11.4}, {11.2,10.8}, 
	    {11.5,9.2}, {12.3,7.5}, {13.6,6.2}, {13.7,4.4}, {12.8,3.0}, 
	    {11.2,2.6}, {10.2,2.0}, {10.4,1.3}, {12.2,0.8}, {13.6,1.6},  
	    {15.1,2.5}, {16.3,2.5}, {17.1,2.1}, {17.8,0.9}, {19.1,0.4}, 
	    {21.3,-0.1}, {22.3,-0.2}, {22.3,-0.5}, {21.7,-1.2}, {22.0,-1.8}, 
	    {23.4,-1.4}, {24.5,-0.5}, {24.2,0.0}, {24.5,0.5}, {26.1,0.8},  
	    {27.0,1.2}, {28.1,2.0}, {30.2,2.2}, {32.2,1.7}, {34.0,1.6}, 
	    {34.7,0.8}, {36.3,-0.1}, {38.1,0.1}, {39.7,0.2}, {41.2,-0.2}, 
	    {43.4,-0.6}, {44.7,-0.2}, {45.6,-0.2}, {46.1,-0.9}, {47.4,-1.2},  
	    {49.1,-1.3}, {50.2,-1.6}, {52.3,-1.5}, {52.7,-1.0}, {53.1,-0.5}, 
	    {54.4,-1.0}, {55.9,-1.6}, {57.7,-1.6}, {58.9,-1.9}, {60.2,-1.8}, 
	    {61.5,-2.8}, {62.6,-3.8}, {65.4,-4.0}, {66.1,-3.4}, {67.2,-2.5},  
	    {68.9,-2.5}, {69.8,-1.7}, {70.3,-0.9}, {71.1,-0.9}, {72.1,-1.2}, 
	    {74.1,-0.9}, {76.0,-0.6}, {77.2,-1.0}, {78.8,-1.9}, {80.3,-2.2}, 
	    {81.6,-2.5}, {83.0,-1.8}, {83.5,-1.2}, {84.0,-0.3}, {84.1,1.2},  
	    {84.8,2.2}, {84.7,3.4}, {83.5,4.9}, {82.6,5.0}, {81.7,4.6}, 
	    {81.5,3.4}, {80.5,2.6}, {79.8,1.9}, {78.2,1.2}, {76.6,0.5}, 
	    {72.6,0.4}, {70.3,0.2}, {68.6,-0.7}, {66.8,-0.4}, {65.1,-0.9},  
	    {63.8,-1.5}, {62.3,-1.2}, {61.8,-0.1}, {62.3,1.3}, {61.9,2.2}, 
	    {60.8,2.5}, {59.8,2.2}, {58.6,2.8}, {58.3,3.9}, {58.6,4.8}, 
	    {57.4,5.9}, {56.2,6.4}, {55.2,6.4}, {54.9,5.6}, {54.6,5.3},  
	    {54.1,5.8}, {54.1,6.7}, {53.4,7.2}, {53.0,6.8}, {52.5,6.1}, 
	    {51.3,5.0}, {50.3,3.7}, {50.3,2.6}, {49.9,1.8}, {48.4,1.7}, 
	    {47.7,2.6}, {47.4,3.3}, {46.7,3.3}, {45.9,2.4}, {44.7,2.1},  
	    {43.4,2.8}, {41.6,2.8}, {40.0,3.6}, {38.6,4.0}, {37.2,3.9}, 
	    {36.2,3.3}, {35.4,2.9}, {34.2,3.4}, {33.3,5.2}, {32.5,6.5}, 
	    {30.5, 8.6}, {28.8,10.3}, {27.4,11.5}, {27.3,12.3}, {28.3,14.1},  
	    {29.4,14.5}, {30.8,13.9}, {31.9,12.7}, {33.1,11.5}, {35.2,10.7}, 
	    {37.0,10.0}, {38.4,10.1}, {40.5,10.3}, {42.0,9.5}, {43.4,8.2}, 
	    {45.5,7.6}, {47.1,7.6}, {49.2,8.6}, {51.5,9.6}, {53.6,9.9},  
	    {55.9,9.9}, {56.6,10.4}, {57.4,10.9}, {59.0,10.7}, {61.1,11.0}, 
	    {62.0,11.9}, {62.8,11.6}, {63.0,10.1}, {63.3,9.1}, {64.9,8.8}, 
	    {67.3,9.2}, {69.2,10.7}, {69.1,11.0}, {68.8,11.6}, {69.8,11.7},  
	    {71.6,10.9}, {72.6,10.5}, {75.4,10.6}, {76.9,11.1}, {78.8,11.8}, 
	    {80.8,11.6}, {81.5,12.0}, {81.7,12.6}, {82.4,11.8}, {82.8,11.2}, 
	    {83.6,11.1}, {84.5,11.8}, {85.6,11.7}, {87.2,11.0}, {88.6,11.0},  
	    {90.5,11.7}, {92.0,10.8}, {92.3,9.9}, {91.1,7.6}, {90.1,5.9}, 
	    {90.5,4.0}, {89.5,1.9}, {89.5,1.1}, {90.3,0.4}, {90.0,-0.2}, 
	    {90.6,-0.5}, {91.3,0.4}, {92.6,1.2}, {94.2,2.1}, {95.5,2.1},  
	    {96.0,2.9}, {95.4,3.7}, {95.5,4.7}, {96.2,5.1}, {96.7,4.9}, 
	    {97.6,3.9}, {98.4,4.2}, {98.8,5.1}, {98.8,6.7}, {100.5,8.2}, 
	    {102.1,8.4}, {104.0,7.9}, {105.6,8.1}, {107.1,9.6}, 
	    {108.2,10.6},  {109.1,10.4}, {108.7,8.8}, {108.8,8.2}, 
	    {110.1,7.6}, {110.3,6.3}, {109.5,5.5}, {108.1,6.0}, {106.3,5.5}, 
	    {105.1,4.5}, {104.5,3.4}, {104.0,2.3}, {104.1,1.1}, {105.8,0.7}, 
	    {109.1,1.4}, {111.4,1.6},  {114.4,1.9}, {117.4,2.5}, {120.5,2.5}, 
	    {122.5,2.5}, {122.8,2.0}, {121.8,0.8}, {121.8,0.2}, {122.3,0.0}, 
	    {123.6,0.6}, {125.3,1.4}, {125.0,1.9}, {124.1,2.2}, {124.2,2.8}, 
	    {126.7,3.4}, {128.7,3.8},  {130.2,4.4}, {133.4,4.5}, {137.6,5.6}, 
	    {140.9,5.3}, {142.6,5.5}, {143.5,4.1}, {142.6,3.0}, {140.8,2.6}, 
	    {138.8,2.0}, {136.2,2.0}, {133.4,2.0}, {131.4,1.6}, {131.3,0.9}, 
	    {132.6,0.5}, {135.0,0.6},  {137.5,1.0}, {138.9,1.0}, {139.4,0.4}, 
	    {139.0,-1.7}, {138.2,-3.1}, {138.5,-4.0}, {142.2,-5.9}, 
	    {145.7,-6.8}, {147.5,-6.8}, {147.6,-5.9}, {146.7,-4.9}, 
	    {147.2,-3.7}, {149.0,-3.6}, {151.2,-4.0}, {153.1,-4.0},  
	    {154.9,-3.8}, {156.8,-3.7}, {156.9,-2.8}, {155.6,-2.0}, 
	    {154.1,-0.9}, {153.8,-0.2}, {152.3,1.2}, {152.9,2.6}, 
	    {154.6,2.5}, {155.9,1.7}, {156.1,0.8}, {157.6,-0.3}, 
	    {159.7,-0.8}, {161.1,-0.4}, {161.9,1.6},  {161.9,2.5}, 
	    {161.9,3.8}, {162.3,5.5}, {164.3,7.3}, {165.4,7.8}, {169.4,7.8}, 
	    {171.4,8.4}, {172.7,9.8}, {175.0,10.3}, {177.1,10.0}, 
	    {179.3,9.3}, {180.0,9.2}
	};

	XSegment xsegs[100], *xs;	/* segments cache */
	double e = epoch == EOD ? mjd : epoch;
	double altdec, azra;
	double glon, glat;
	int n, i;

	for (i=0; i<2; i++) {
	    int nmax = i==0 ? XtNumber(mw1) : XtNumber(mw2);
	    int x1 = 0, y1 = 0, x2, y2;
	    int before = 0;
	    int vis1 = 0, vis2;
	    xs = xsegs;

	    for (n=0; n<nmax; n++) {

		if (i==0) {
                    /* draw the first run */
                    glon=degrad(mw1[n][0]);
                    glat=degrad(mw1[n][1]);
		} else {
                    /* draw the second run */
                    glon=degrad(mw2[n][0]);
                    glat=degrad(mw2[n][1]);
		}
		/* convert longitude along the galactic eq to ra/dec */
		gal_eq (e, glat, glon, &azra, &altdec);

		/* if in aa mode, we need it in alt/az */
		if (aa_mode)
		    sv_other (altdec, azra, 0, &altdec, &azra);

		vis2 = sv_loc(altdec,azra,&x2,&y2);
		if (before++ && (vis1 || vis2)) {
		    int sx1, sy1, sx2, sy2;

		    /* draw segment if visible */
		    if (segisvis(x1, y1, x2, y2, &sx1, &sy1, &sx2, &sy2)) {
			xs->x1 = sx1; xs->y1 = sy1;
			xs->x2 = sx2; xs->y2 = sy2;
			if (++xs == &xsegs[XtNumber(xsegs)]) {
			    split_segs (dsp, win, gc, xsegs, xs - xsegs);
			    xs = xsegs;
			}
		    }
		}
		x1 = x2;
		y1 = y2;
		vis1 = vis2;
	    }

	    if (xs > xsegs)
		split_segs (dsp, win, gc, xsegs, xs - xsegs);
	}
}

/* draw the constellation lines */
static void
draw_cnsbounds(np, dsp, win)
Now *np;
Display *dsp;
Window win;
{
#define	NCONSEGS    23	/* draw with this fraction of r (primes look best) */
	double alt, az, ra, dec;
	double e = epoch == EOD ? mjd : epoch;
	double segsize = sv_dfov/NCONSEGS;
	double cdec, sdec;
	double *era0, *edec0, *era1, *edec1;
	double lst;
	int nedges;

	/* get all the edges, precessed to e.
	 */
	nedges = cns_edges (e, &era0, &edec0, &era1, &edec1);
	if (nedges <= 0) {
	    xe_msg (1, "Can't find constellation edges");
	    return;
	}

	/* prepare for drawing */
	XSetForeground (dsp, sv_cnsgc, cnsbnd_p);
	sv_fullwhere (np, sv_altdec, sv_azra, aa_mode, &alt, &az, &ra, &dec);
	now_lst (np, &lst);
	cdec = cos(dec);
	sdec = sin(dec);

	/* for each edge.. break into smaller segments
	 * and draw any that are even partially visible.
	 */
	while (--nedges >= 0) {
	    double ra0  = era0[nedges];
	    double dec0 = edec0[nedges];
	    double ra1  = era1[nedges];
	    double dec1 = edec1[nedges];
	    XPoint xpts[NCONSEGS+10];
	    int lastvis, lastx, lasty;
	    double dra, ddec;
	    double sep, csep;
	    int see0, see1;
	    int nsegs;
	    int npts;
	    int j;

	    /* cull segments that aren't even close */
	    solve_sphere (ra-ra0, PI/2-dec0, sdec, cdec, &csep, NULL);
	    see0 = (acos(csep) < sv_dfov);
	    solve_sphere (ra-ra1, PI/2-dec1, sdec, cdec, &csep, NULL);
	    see1 = (acos(csep) < sv_dfov);
	    if (!see0 && !see1)
		continue;

	    /* find number of segments with which to draw this edge */
	    solve_sphere (ra1-ra0, PI/2-dec1, sin(dec0), cos(dec0), &csep,NULL);
	    sep = acos(csep);
	    nsegs = (int)(sep/segsize) + 1;

	    /* find step sizes.
	     * N.B. watch for RA going the long way through 0
	     */
	    dra = ra1 - ra0;
	    if (dra < -PI)
		dra += 2*PI;
	    else if (dra > PI)
		dra -= 2*PI;
	    dra /= nsegs;
	    ddec = (dec1 - dec0)/nsegs;

	    /* step along the segment ends */
	    lastvis = -1; /* illegal return value from sv_loc() */
	    lastx = lasty = 0;
	    npts = 0;
	    for (j = 0; j <= nsegs; j++) {
		int vis, x, y;
		double ad, ar;

		ad = dec0 + j*ddec;
		ar = ra0 + j*dra;

		/* need alt/az when are in aa mode */
		if (aa_mode)
		    sv_other (ad, ar, 0, &ad, &ar);

		vis = sv_loc (ad, ar, &x, &y);	/* hzn down with clipping */

		if (lastvis >= 0 && vis != lastvis) {
		    int x1, y1, x2, y2;

		    /* at edge of circle -- find crossing point */
		    if (segisvis (lastx, lasty, x, y, &x1, &y1, &x2, &y2)) {
			if (vis) {
			    xpts[npts].x = x1;
			    xpts[npts].y = y1;
			    npts++;
			} else {
			    xpts[npts].x = x2;
			    xpts[npts].y = y2;
			    npts++;
			    break;	/* end of visible portion */
			}
		    }
		}
		if (vis) {
		    xpts[npts].x = x;
		    xpts[npts].y = y;
		    npts++;
		}

		lastvis = vis;
		lastx = x;
		lasty = y;
	    }

	    if (npts > XtNumber(xpts)) {
		/* stack has already overflowed but try to show how much */
		printf ("cnsbounds Bug! npts=%d N=%d\n", npts, XtNumber(xpts));
		abort();
	    }

	    if (npts > 1)
		split_lines(dsp, win, sv_cnsgc, xpts, npts);
	}
#undef	NCONSEGS
}

/* like XPSDrawLine but for use when cyl_proj to split around left or right
 *   edge if longer than half a screen width.
 * return 1 if had to split, else 0.
 */
static int
split_line (dsp, win, gc, x1, y1, x2, y2)
Display *dsp;
Window win;
GC gc;
int x1, y1;
int x2, y2;
{
	if (cyl_proj) {
	    XSegment xs;

	    xs.x1 = x1;
	    xs.y1 = y1;
	    xs.x2 = x2;
	    xs.y2 = y2;
	    return (split_segs (dsp, win, gc, &xs, 1));
	} else {
	    XPSDrawLine (dsp, win, gc, x1, y1, x2, y2);
	    return (0);
	}
}

/* like XPSDrawLines but for use when in cyl_proj to split around left or
 *   right edge if any leg longer than half a screen width.
 * return the number of legs that were split.
 * N.B. we assume np >= 2.
 */
static int
split_lines (dsp, win, gc, xp, np)
Display *dsp;
Window win;
GC gc;
XPoint xp[];
int np;
{
	if (cyl_proj) {
	    int ns = np-1;
	    XSegment *sp0 = (XSegment *) XtMalloc (ns * sizeof(XSegment));
	    XSegment *sp = sp0;
	    XPoint *lxp;
	    int nsplit;

	    for (lxp = xp+ns; xp<lxp; sp++, xp++) {
		sp->x1 = xp->x;
		sp->y1 = xp->y;
		sp->x2 = xp[1].x;
		sp->y2 = xp[1].y;
	    }
	    nsplit = split_segs (dsp, win, gc, sp0, ns);
	    XtFree ((void*)sp0);
	    return (nsplit);
	} else {
	    XPSDrawLines (dsp, win, gc, xp, np, CoordModeOrigin);
	    return (0);
	}
}

/* like XPSDrawSegments but for use when in cyl_proj to split around edge.
 * we know that both ends of each segment are visible.
 * return the number of segments that were split.
 * N.B. unlike XPSDrawSegments we may modify sp[] IN PLACE
 */
static int
split_segs (dsp, win, gc, xsp, ns)
Display *dsp;
Window win;
GC gc;
XSegment xsp[];
int ns;
{
	int nsplit = 0;

	/* check for probably wraps */
	if (cyl_proj) {
	    XSegment *sp = xsp, *lsp;
	    int xwrap, ywrap;

	    for (lsp = sp+ns; sp<lsp; sp++) {
		split_wrap (sp, &xwrap, &ywrap);
		if (xwrap || ywrap) {
		    /* draw one end here, modify other end for array draw */
		    XPSDrawLine (dsp, win, gc, sp->x1, sp->y1,
						    sp->x2+xwrap, sp->y2+ywrap);
		    sp->x1 -= xwrap;
		    sp->y1 -= ywrap;
		    nsplit++;
		}
	    }
	}

	XPSDrawSegments (dsp, win, gc, xsp, ns);

	return (nsplit);
}

/* given a segment, determine if wraps around in the cyl projection.
 * if so, return how far to wrap in each direction, or 0 if none.
 */
static void
split_wrap (sp, xwp, ywp)
XSegment *sp;
int *xwp, *ywp;
{
	double scale = sv_h/sv_vfov;
	int diff;

	if (abs(diff = sp->x2 - sp->x1) > PI*scale)
	    *xwp = (int)floor((2*PI)*scale * (diff > 1 ? -1 : 1) + .5);
	else
	    *xwp = 0;

	if (abs(diff = sp->y2 - sp->y1) > 2*scale)
	    *ywp = (int)floor(PI*scale * (diff > 1 ? -1 : 1) + .5);
	else
	    *ywp = 0;
}

/* draw the constellation figures and/or names */
static void
draw_cns(np, dsp, win)
Now *np;
Display *dsp;
Window win;
{
#define	BBOX(x1,y1,x2,y2)   {				\
	    if (want_conn || want_cona) {		\
		if (!begun || x1 < minx) minx = x1;	\
		if (!begun || x1 > maxx) maxx = x1;	\
		if (!begun || y1 < miny) miny = y1;	\
		if (!begun || y1 > maxy) maxy = y1;	\
		if (x2 < minx) minx = x2;		\
		if (x2 > maxx) maxx = x2;		\
		if (y2 < miny) miny = y2;		\
		if (y2 > maxy) maxy = y2;		\
		begun = 1;				\
	    }						\
	}
	double alt, az, ra, dec;
	double altdec, azra;
	double e = epoch == EOD ? mjd : epoch;
	double fra[40], fdec[40];
	double lst;
	int dcodes[40];
	int conids[89];
	int ncns, ndc;
	int x, lastx = 0, y, lasty = 0;
	int vis, lastvis = 0;
	int sx1, sy1, sx2, sy2;
	int minx = 0, maxx = 0, miny = 0, maxy = 0;
	int begun;
	int split;
	int i;

	XSetForeground (dsp, sv_cnsgc, cnsfig_p);
	sv_fullwhere (np, sv_altdec, sv_azra, aa_mode, &alt, &az, &ra, &dec);
	ncns = cns_list (ra, dec, e, sv_dfov/2, conids);
	now_lst (np, &lst);

	while (--ncns >= 0) {
	    ndc = cns_figure (conids[ncns], e, fra, fdec, dcodes);
	    if (ndc <= 0) {
		printf ("Can't get cns figures! id=%d\n", conids[ncns]);
		abort();
	    }
	    split = begun = 0;
	    for (i = 0; i < ndc; i++) {
		/* need alt/az if in alt/az mode */
		if (aa_mode) {
		    sv_other (fdec[i], fra[i], 0, &altdec, &azra);
		} else {
		    altdec = fdec[i];
		    azra = fra[i];
		}
		vis =  sv_loc (altdec, azra, &x, &y); /* hzn uses clipping */

		if (dcodes[i]) {
		    if (want_conf)
			sv_set_dashed_cnsgc (dsp, dcodes[i] == 2);
		    if (vis != lastvis) {
			if (segisvis(lastx,lasty,x,y,&sx1,&sy1,&sx2,&sy2)) {
			    if (want_conf)
				split += split_line (dsp, win, sv_cnsgc, sx1,
								sy1, sx2, sy2);
			    BBOX(sx1, sy1, sx2, sy2);
			}
		    } else if (vis) {
			if (want_conf)
			    split += split_line (dsp, win, sv_cnsgc, lastx,
								lasty, x, y);
			BBOX(lastx, lasty, x, y);
		    }
		}
		lastx = x;
		lasty = y;
		lastvis = vis;
	    }

	    if ((want_conn || want_cona) && begun && !split)
		draw_cnsname (dsp, win, conids[ncns], minx, miny, maxx, maxy);
	}

	sv_set_dashed_cnsgc (dsp, 0);
#undef	BBOX
}

/* draw the name of the given constellation centered in the bounding box */
static void
draw_cnsname (dsp, win, conid, minx, miny, maxx, maxy)
Display *dsp;
Window win;
int conid;
int minx, miny, maxx, maxy;
{
	char *name = cns_name (conid);
	XCharStruct all;
	int len;
	int dir, asc, des;
	int x, y;

	if (want_conn) {
	    name += 5;				/* skip "XXX: " */
	    len = strlen (name);
	} else
	    len = 3;				/* just XXX: */
	XTextExtents (sv_cf, name, len, &dir, &asc, &des, &all);

	x = minx + (maxx - minx - all.rbearing)/2;
	y = miny + (maxy - miny - (all.ascent + all.descent))/2 + all.ascent;

	XSetFont (dsp, sv_strgc, sv_cf->fid);
	XSetForeground (dsp, sv_strgc, cnsnam_p);
	XPSDrawString (dsp, win, sv_strgc, x, y, name, len); 
}

/* draw a label for an object that is located at [x,y] with symbol diam d.
 * the label consists OBJF_* in flags.
 * label may contain greek names with superscripts.
 */
static void
draw_label (win, gc, op, flags, x, y, d)
Window win;
GC gc;
Obj *op;
int flags;	/* mask of OBJF_{L,R,N,M,PERSLB}LABEL set */
int x, y;	/* center of object we are labeling, pixels */
int d;		/* diam of object we are labeling, pixels */
{
	Display *dsp = XtDisplay (svda_w);
	char *name = op->o_name;
	int gw = 0;	/* greek width, pixels */
	int sw = 0;	/* superscript width */
	int sa = 0;	/* superscript ascent */
	int nw = 0;	/* regular name width (might include mag too) */
	int gl = 0;	/* n chars in greek name, if any */
	char g = '\0';	/* char code to drawn greek character, if any */
	XCharStruct xcs;
	char buf[128];
	int dir, asc, des;
	int sx, sy;
	int tw;

	/* check the trivial case of not drawing anything :-) */
	if (!name[0] || !(flags & (OBJF_NLABEL|OBJF_MLABEL|OBJF_PERSLB)))
	    return;

	/* default to name if persistent and neither name or mag are set */
	if (!(flags & (OBJF_NLABEL|OBJF_MLABEL))) {
	    flags |= OBJF_NLABEL;
	    if (!(flags & (OBJF_LLABEL|OBJF_RLABEL)))
		flags |= OBJF_RLABEL;
	}

	/* deal with name portion first */
	if (flags & OBJF_NLABEL) {
	    if (sv_ggc && chk_greeklabel (name, &gl, &g)) {
		XTextExtents (sv_gf, &g, 1, &dir, &asc, &des, &xcs);
		gw = xcs.width;
		if (isdigit(name[4+gl])) {
		    /* don't crowd the superscript */
		    XTextExtents (sv_pf, name+gl+4, 1, &dir, &asc, &des, &xcs);
		    gw += 1;
		    sw = xcs.width + 1;
		    sa = xcs.ascent;
		    (void) strcpy (buf, name+gl+5);
		} else
		    (void) strcpy (buf, name+gl+4);
	    } else
		(void) strcpy (buf, name);
	}

	/* set (or append) mag in buf (too) if enabled */
	if (flags & OBJF_MLABEL) {
	    int m = (int)floor(get_mag(op)*10.0 + 0.5);
	    if (flags & OBJF_NLABEL)
		(void) sprintf (buf+strlen(buf), "(%d)", m);
	    else
		(void) sprintf (buf, "%d", m);
	}

	/* eclipsed earth sat in parens */
	if (is_type(op,EARTHSATM) && op->s_eclipsed) {
	    int l = strlen(buf);
	    memmove (buf+1, buf, l);
	    buf[0] = '[';
	    buf[++l] = ']';
	    buf[++l] = '\0';
	}

	XTextExtents (sv_pf, buf, strlen(buf), &dir, &asc, &des, &xcs);
	nw = xcs.width;

	/* find total offset from x to be drawn then starting x and y */
	tw = gw + sw + nw;
	sx = (flags & OBJF_LLABEL) ? x - tw - 2*d/5 - 2 : x + 2*d/5 + 2;
	sy = y - 2*d/5;

	/* draw everything, starting at sx,sy */
	if (gw) {
	    unsigned long gcm;
	    XGCValues gcv;

	    /* use same color in ggc as in gc */
	    gcm = GCForeground;
	    (void) XGetGCValues (dsp, gc, gcm, &gcv);
	    XSetForeground (dsp, sv_ggc, gcv.foreground);

	    XPSDrawString (dsp, win, sv_ggc, sx, sy, &g, 1);
	    sx += gw;
	    if (sw) {
		XPSDrawString (dsp, win, gc, sx, sy-sa/2, name+gl+4, 1);
		sx += sw;
	    }
	}
	XPSDrawString (dsp, win, gc, sx, sy, buf, strlen(buf));
}

/* see if the given name is of the form "Cns BayerN-Flams". If so, find the
 * number of chars in the Bayer (greek) part and the greek font code and ret 1.
 * else return 0.
 */
static int
chk_greeklabel (name, glp, gcodep)
char name[];	/* name */
int *glp;	/* number of chars in the greek name part */
char *gcodep;	/* code to use for drawing the greek character */
{
	static char *greeks[] = {
	    "Alpha", "Beta",    "Gamma",   "Delta", "Epsilon", "Zeta",
	    "Eta",   "Theta",   "Iota",    "Kappa", "Lambda",  "Mu", 
	    "Nu",    "Xi",      "Omicron", "Pi",    "Rho",     "Sigma",
	    "Tau",   "Upsilon", "Phi",     "Chi",   "Psi",     "Omega",
	};
	static char grfontidx[] = "abgdezhqiklmnxoprstufcjw";
	static int glen[XtNumber(greeks)];
	int strl;
	int gl;
	int i;

	/* forget it if no greek gc available */
	if (!sv_ggc)
	    return (0);

	/* init glen array the first time */
	if (glen[0] == 0)
	    for (i = 0; i < XtNumber(greeks); i++)
		glen[i] = strlen(greeks[i]);

	/* fast preliminary checks */
	strl = strlen (name);
	if (strl < 6)	/* shortest greek entry is "Cns Pi" */
	    return (0);
	if (name[3] != ' ')
	    return (0);

	/* find length of potentionally greek portion */
	for (gl = 0; ; gl++)
	    if (!isalpha(name[4+gl]))
		break;
	if (gl < 2)	/* shortest greek name is 2 chars */
	    return (0);

	/* scan for greek name -- it may be truncated */
	for (i = 0; i < XtNumber(greeks); i++) {
	    if (gl <= glen[i] && strncmp (name+4, greeks[i], gl) == 0) {
		*gcodep = grfontidx[i];
		*glp = gl;
		return (1);
	    }
	}

	return (0);
}

/* draw all visible eyepieces */
static void
draw_eyep (dsp, win, gc)
Display *dsp;
Window win;
GC gc;
{
	int cir = 360*64;/* 360 degrees in X */
	EyePiece *eyep;
	int neyep;
	int i;

	/* gather the list */
	neyep = se_getlist (&eyep);
	if (neyep == 0)
	    return;

	/* use ep color unless it matches the sky then use fg color */
	XSetForeground (dsp, gc, eyep_p == sky_p ? annot_p : eyep_p);

	/* set this to the largest eyep we actually draw -- just used for the
	 * print label.
	 */
	largeyepr = NULL;
	neyepr = 0;

	/* draw each eyepiece */
	for (i = 0; i < neyep; i++) {
	    EyePiece *ep = &eyep[i];
	    double altdec, azra;
	    int epw, eph;	/* eyepiece half width, height, pixels */
	    double xra, xdec, xpa;
	    int x, y;

	    /* forget it if too small */
	    epw = (int)floor(ep->eyepw*sv_h/sv_vfov/2 + 0.5);
	    eph = (int)floor(ep->eyeph*sv_h/sv_vfov/2 + 0.5);
	    if (epw < MINEPR && eph < MINEPR)
		continue;

	    /* get ep coords in this display mode */
	    if (aa_mode == ep->aamode) {
		altdec = ep->altdec;
		azra = ep->azra;
	    } else {
		sv_other (ep->altdec, ep->azra, ep->aamode, &altdec, &azra);
	    }

	    /* get ep ra/dec/pa */
	    if (ep->aamode) {
		Now *np = mm_get_now();
		double lst;
		sv_other (ep->altdec, ep->azra, 1, &xdec, &xra);
		now_lst (np, &lst);
		xpa = parallacticLHD (lat, hrrad(lst) - xra, xdec) - ep->eyepa;
	    } else {
		xra  = ep->azra;
		xdec = ep->altdec;
		xpa  = ep->eyepa;
	    }

	    /* only draw if center is on screen */
	    if (sv_loc (altdec, azra, &x, &y)) {
		double a = x_angle (xra, xdec, xpa) + PI/2;
		if (ep->round) {
		    int xa = (int)floor(raddeg(a)*64 + 0.5);
		    if (ep->solid)
			XPSFillEllipse(dsp, win, gc, x-epw, y-eph, xa, 2*epw,
								2*eph, 0, cir);
		    else
			XPSDrawEllipse(dsp, win, gc, x-epw, y-eph, xa, 2*epw,
								2*eph, 0, cir);
		} else {
		    XPoint r[5];
		    double ca = cos(a);
		    double sa = sin(a);
		    int xr =  epw*ca + eph*sa;
		    int yr = -epw*sa + eph*ca;
		    int xl = -epw*ca + eph*sa;
		    int yl =  epw*sa + eph*ca;
		    r[0].x = x + xr; r[0].y = y + yr;
		    r[1].x = x + xl; r[1].y = y + yl;
		    r[2].x = x - xr; r[2].y = y - yr;
		    r[3].x = x - xl; r[3].y = y - yl;
		    r[4] = r[0];
		    if (ep->solid)
			XPSFillPolygon (dsp, win, gc, r, 4, Nonconvex,
							    CoordModeOrigin);
		    else
			XPSDrawLines (dsp, win, gc, r, 5, CoordModeOrigin);
		}
		if (!largeyepr || largeyepr->eyepw < ep->eyepw
						|| largeyepr->eyeph < ep->eyeph)
		    largeyepr = ep;
		neyepr++;
	    }
	}
}

/* given an object return its desired diameter, in pixels.
 * the size is the larger of the actual size at the current window scale or
 *   a size designed to be proportional to the objects visual magnitude.
 *   but we also force deep sky objects to be at least MIND.
 * N.B. we assume we already know op is at least as bright as dsmag or fsmag
 *   (depending on is_deepsky(), respectively).
 */
static int
objdiam(Obj *op)
{
	int stmag, ssmag, dsmag, magstp;
	int isdeep = is_deepsky(op);
	int faint, d;

	svf_getmaglimits (&stmag, &ssmag, &dsmag, &magstp);
	faint = isdeep ? dsmag : (is_ssobj(op) ? ssmag : stmag);
	d = magdiam (faint, magstp, sv_vfov/sv_h, get_mag(op),
					    degrad(op->s_size/3600.0));

	if (isdeep) {
	    if (d < MIND)
		d = MIND;
	}

	return (d);
}

/* given an object and its diameter return its desired gc.
 */
static void
objGC(Obj *op, int d, GC *gcp)
{
	if (d <= 1)
	    *gcp = dimgc;
	else
	    obj_pickgc (op, svda_w, gcp);
}

/* make the GCs, load the fonts and gather the colors.
 * TODO: clean up previous stuff if called more than once.
 */
static void
sv_mk_gcs()
{
	Display *dsp = XtDisplay(toplevel_w);
	Window win = XtWindow(toplevel_w);
	unsigned long gcm;
	XGCValues gcv;
	Pixel p;

	get_something (svda_w, XmNbackground, (XtArgVal)&bg_p);

	(void) get_color_resource (svda_w, "SkyAnnotColor", &annot_p);
	(void) get_color_resource (svda_w, "SkyColor", &sky_p);
	(void) get_color_resource (svda_w, "HorizonColor", &hzn_p);
	(void) get_color_resource (svda_w, "SkyCnsFigColor", &cnsfig_p);
	(void) get_color_resource (svda_w, "SkyCnsBndColor", &cnsbnd_p);
	(void) get_color_resource (svda_w, "SkyCnsNamColor", &cnsnam_p);
	(void) get_color_resource (svda_w, "SkyGridColor", &grid_p);
	(void) get_color_resource (svda_w, "SkyEqColor", &eq_p);
	(void) get_color_resource (svda_w, "SkyMWColor", &mw_p);
	(void) get_color_resource (svda_w, "SkyEyePColor", &eyep_p);

	sv_rf = getXResFont ("SkyGridFont");
	gcm = GCFont;
	gcv.font = sv_rf->fid;
	sv_gc = XCreateGC (dsp, win, gcm, &gcv);

	gcm = 0L;
	sv_cnsgc = XCreateGC (dsp, win, gcm, &gcv);

	gcm = 0L;
	sv_strgc = XCreateGC (dsp, win, gcm, &gcv);

	/* load the tracking and constellation fonts */
	get_tracking_font (dsp, &sv_tf);
	sv_cf = getXResFont ("CnsFont");

	/* load the greek font */
	loadGreek (dsp, win, &sv_ggc, &sv_gf);

	/* get font used by all gcs from obj_pickgc() */
	get_views_font (dsp, &sv_pf);

	/* build gc for dim stars */
	(void) get_color_resource (svda_w, "DimStarColor", &p);
	gcm = GCForeground | GCFont;
	gcv.foreground = p;
	gcv.font = sv_pf->fid;
	dimgc = XCreateGC (dsp, win, gcm, &gcv);

	/* zoom box gc */
	gcm = GCForeground | GCFunction;
	gcv.foreground = sky_p ^ WhitePixel (dsp, DefaultScreen (dsp));
	gcv.function = GXxor;
	zm_xorgc = XCreateGC (dsp, win, gcm, &gcv);

	/* transient window marker gc */
	gcm = GCForeground | GCFunction;
	gcv.foreground = sky_p ^ annot_p;
	gcv.function = GXxor;
	sv_tmgc = XCreateGC (dsp, win, gcm, &gcv);
}

/* set line_style of sv_cnsgc to LineSolid if dashed==0 else to LineOnOffDash.
 * keep a cache so we don't mess with it any more than necessary.
 */
static void
sv_set_dashed_cnsgc (dsp, dashed)
Display *dsp;
int dashed;
{
	static int last_dashed = -1243;	/* anything bogus */
	XGCValues xgcv;
	unsigned long mask;

	if (last_dashed == dashed)
	    return;
	last_dashed = dashed;

	mask = GCLineStyle;
	xgcv.line_style = dashed ? LineOnOffDash : LineSolid;
	XChangeGC (dsp, sv_cnsgc, mask, &xgcv);
}

/* convert the given X windows coords to image coords.
 * N.B. we assume an image is indeed being displayed
 */
static void
sv_win2im (wx, wy, ix, iy)
int wx, wy;
double *ix, *iy;
{
	if (flip_lr)
	    wx = sv_w - 1 - wx;
	if (flip_tb)
	    wy = sv_h - 1 - wy;
	si_win2im (wx, wy, sv_w, sv_h, ix, iy);
}

/* if a circular edge is showing clip, else just copy.
 * return 1 if any part MAY be visible, else 0.
 */ 
static int
segisvis (x1, y1, x2, y2, cx1, cy1, cx2, cy2)
int x1, y1, x2, y2;		/* original segment */
int *cx1, *cy1, *cx2, *cy2;	/* clipped segment */
{
	int vis;

	if (cyl_proj || sv_dfov < PI) {
	    /* no round edge so just check for rectangle vis.
	     */
	    *cx1 = x1;
	    *cy1 = y1;
	    *cx2 = x2;
	    *cy2 = y2;

	    vis = (x1>=0 || x2>=0) && (x1<sv_w || x2<sv_w)
			&& (y1>=0 || y2>=0) && (y1<sv_h || y2<=sv_h);
	} else {
	    int w = (int)(PI*sv_h/sv_vfov);
	    int x = (sv_w - w)/2;
	    int y = (sv_h - w)/2;

	    vis = lc (x, y, w, x1, y1, x2, y2, cx1, cy1, cx2, cy2) < 0 ? 0 : 1;
	}

	return (vis);
}

/* flip the given region, left/right else top/bottom */
static void
zm_flip0 (lr, zp)
int lr;
ZM_Undo *zp;
{
	if (lr) {
	    zp->x0 = sv_w - zp->x0;
	    zp->x1 = sv_w - zp->x1;
	} else {
	    zp->y0 = sv_h - zp->y0;
	    zp->y1 = sv_h - zp->y1;
	}
}

/* user has flipped image .. update zoom regions to match */
static void
zm_flip (lr)
int lr;
{
	int i;

	/* flip working zoom and all on stack */
	zm_flip0 (lr, &wzm);
	for (i = 0; i < zm_nundo; i++)
	    zm_flip0 (lr, &zm_undo[i]);

}

/* add the current settings in a new zoom undo entry and install.
 * if successful all deeper entries are discarded.
 * return -1 with no change if zoom box center is not in map, else 0.
 */
static int
zm_addundo()
{
	ZM_Undo *zp;
	double ad, ar;
	double dx, dy;
	double ratio;
	double newfov;

	/* assert */
	if (zm_cundo < 0) {
	    printf ("addundo Bug! zm_cundo=%d zm_nundo=%d\n",zm_cundo,zm_nundo);
	    abort();
	}

	/* always add to end of list */
	zm_cutundo();

	/* compute new center.
	 * beware off image
	 * just use x/y if image has no WCS
	 */
	if (!sv_unloc ((wzm.x0+wzm.x1)/2, (wzm.y0+wzm.y1)/2, &ad, &ar)) {
	    if (si_ison()) {
		ad = sv_altdec;
		ar = sv_azra;
	    } else {
		xe_msg (1, "Center of zoom box must be inside window");
		return (-1);
	    }
	}

	/* make room for 1 new undo entry */
	zm_undo = (ZM_Undo *) XtRealloc ((void *)zm_undo,
					    (++zm_nundo)*sizeof(ZM_Undo));
	zp = &zm_undo[zm_cundo++];

	/* save current settings in new entry */
	zp->ad = sv_altdec;
	zp->ar = sv_azra;
	zp->fov = sv_vfov;
	zp->x0 = wzm.x0;
	zp->y0 = wzm.y0;
	zp->x1 = wzm.x1;
	zp->y1 = wzm.y1;

	/* compute new fov */
	dx = abs(wzm.x0 - wzm.x1);
	dy = abs(wzm.y0 - wzm.y1);
	ratio = (dx>dy?dx:dy)/sv_h;
	newfov = ratio*sv_vfov;

	/* install */
	sv_altdec = ad;
	sv_set_scale(ALTDEC_S, 0);
	sv_azra = ar;
	sv_set_scale(AZRA_S, 0);
	sv_set_fov (newfov);
	sv_set_scale(FOV_S, 0);

	return (0);
}

/* install the zm_cundo entry of the zoom undo stack */
static void
zm_installundo()
{
	ZM_Undo *zp = &zm_undo[zm_cundo];

	/* set new center and fov */
	sv_altdec = zp->ad;
	sv_set_scale(ALTDEC_S, 0);
	sv_azra = zp->ar;
	sv_set_scale(AZRA_S, 0);
	sv_set_fov (zp->fov);
	sv_set_scale(FOV_S, 0);

	/* set to new aoi box */
	wzm.x0 = zp->x0;
	wzm.y0 = zp->y0;
	wzm.x1 = zp->x1;
	wzm.y1 = zp->y1;
}

/* discard zoom undo stack deeper than current setting */
static void
zm_cutundo()
{
	zm_nundo = zm_cundo;
}

/* disable and forget all unzooms */
static void
zm_noundo()
{
	/* disable for later sv_all's */
	svtb_zoomok(0);

	if (zm_undo) {
	    XtFree ((void *)zm_undo);
	    zm_undo = NULL;
	    zm_nundo = zm_cundo = 0;
	    svtb_unzoomok(0);
	}
}

/* draw rectangle defined by zm_[xy][01] using XOR.
 * this is used both to draw and erase.
 */
static void
zm_draw()
{
	Display *dsp = XtDisplay(svda_w);
	Window win = XtWindow(svda_w);
	XPoint xp[5];

	/* TODO: leaves bit turd if have 0 thickness */
	xp[0].x = wzm.x0; xp[0].y = wzm.y0;
	xp[1].x = wzm.x1; xp[1].y = wzm.y0;
	xp[2].x = wzm.x1; xp[2].y = wzm.y1;
	xp[3].x = wzm.x0; xp[3].y = wzm.y1;
	xp[4].x = wzm.x0; xp[4].y = wzm.y0;
	XPSDrawLines (dsp, win, zm_xorgc, xp, 5, CoordModeOrigin);
}

/* respond to fov shortcut PB.
 * client is secret code.. see various XtAdds
 */
static void
sv_fovsc_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int code = (long int)client;

	switch (code) {
	case 0:		/* 90:00 FOV */
	    si_off();
	    sv_set_fov (degrad(90));
	    sv_set_scale (FOV_S, 1);
	    sv_all (mm_get_now());
	    break;

	case 1:		/* 1:1 */
	    sv_resize (sv_h, sv_h);
	    break;

	case 2:		/* 2:1 */
	    sv_resize (sv_h & ~1, sv_h/2);	/* insure exact */
	    break;

	case 3:
	    if (si_ison()) {
		FImage *fip = si_getFImage();
		int neww = sv_h * fip->sw / fip->sh;
		int newh = sv_w * fip->sh / fip->sw;
		if (neww > sv_w)
		    sv_resize (neww, sv_h);
		else if (newh > sv_h)
		    sv_resize (sv_w, newh);
	    }
	    break;

	default:
	    printf ("Bug! Bogus fovsc code: %d\n", code);
	    abort();
	}
}

/* respond to an altdec shortcut PB.
 * client is desired position, in degrees.
 */
static void
sv_altdecsc_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	si_off();
	sv_altdec = degrad((long int)client);
	sv_set_scale (ALTDEC_S, 1);
	sv_all (mm_get_now());
}

/* bring up telescope configure window */
/* ARGSUSED */
static void
sv_tcp_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	sc_manage();
}

/* bring up INDI operations panel window */
/* ARGSUSED */
static void
sv_indi_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	indi_manage();
}

/* callback when a Telescope history item is destroyed */
/* ARGSUSED */
static void
sv_dtelpd_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPointer ud;

	get_something (w, XmNuserData, (XtArgVal)&ud);
	if (ud)
	    XtFree ((char *)ud);
}

/* callback to erase all telescope history */
/* ARGSUSED */
static void
sv_etelpd_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	sv_erasetelpd();
}

/* callback when a Telescope history item is activated.
 * userData is a pointer to a malloced copy of the Obj to which we are to point.
 */
/* ARGSUSED */
static void
sv_telpd_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Obj *op;

	get_something (w, XmNuserData, (XtArgVal)&op);
	telGoto (op);
}

/* add op to the Telescope pulldown as a history entry.
 * we add a new PB and store a copy of op in its userData.
 */
static void
sv_addtelpd (op)
Obj *op;
{
	char label[64];
	XtPointer ud;
	Widget w;
	Arg args[20];
	int n;
	WidgetList ch;
	Cardinal nch;
	int i;

	/* create the label for the button */
	if (strcmp (op->o_name, telAnon))
	    strcpy (label, op->o_name);
	else {
	    char rastr[32], decstr[32];
	    fs_ra (rastr, op->f_RA);
	    fs_prdec (decstr, op->f_dec);
	    sprintf (label, "%s %s", rastr, decstr);
	}

	/* check for already in list */
	get_something (telpd_w, XmNchildren, (XtArgVal)&ch);
	get_something (telpd_w, XmNnumChildren, (XtArgVal)&nch);
	for (i = 0; i < (int)nch; i++) {
	    char *chlabel;
	    int found;

	    if (!XmIsPushButton(ch[i]))
		continue;
	    get_xmstring (ch[i], XmNlabelString, &chlabel);
	    found = !strcmp (chlabel, label);
	    XtFree (chlabel);
	    if (found)
		return;
	}

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreatePushButton (telpd_w, "TELH", args, n);
	XtAddCallback (w, XmNactivateCallback, sv_telpd_cb, NULL);
	XtAddCallback (w, XmNdestroyCallback, sv_dtelpd_cb, NULL);
	set_xmstring (w, XmNlabelString, label);
	ud = (XtPointer) XtMalloc (sizeof(Obj));
	memcpy ((void *)ud, (void *)op, sizeof(Obj));
	set_something (w, XmNuserData, (XtArgVal)ud);
	XtManageChild (w);
}

/* erase the entire telescope goto history.
 * N.B. relies on fact they are the only children of telpd_w using userData.
 */
/* ARGSUSED */
static void
sv_erasetelpd ()
{
	WidgetList ch;
	Cardinal nch;
	int i;

	get_something (telpd_w, XmNchildren, (XtArgVal)&ch);
	get_something (telpd_w, XmNnumChildren, (XtArgVal)&nch);
	for (i = 0; i < (int)nch; i++) {
	    XtPointer ud;
	    get_something (ch[i], XmNuserData, (XtArgVal)&ud);
	    if (ud)
		XtDestroyWidget (ch[i]);
	}
}

/* called periodically to decide whether to reload field stars.
 */
static void
chkFS_to (client, id)
XtPointer client;
XtIntervalId *id;
{
	sv_loadfs(0);
	fs_to = XtAppAddTimeOut (xe_app, FSTO, chkFS_to, 0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: skyviewmenu.c,v $ $Date: 2015/04/09 00:23:02 $ $Revision: 1.351 $ $Name:  $"};

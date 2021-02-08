#ifndef _XEPHEM_H
#define	_XEPHEM_H

/* master include file for xephem.
 * first portion to be used by all, second assumes X already included.
 */

/* CHAPG  Alex Chupahin */
#ifdef VMS
#define fork vfork
#endif

#include <stdarg.h>		/* be kind to those who don't use xe_msg() */

#include "astro.h"
#include "ip.h"

/* local glue files */
#include "map.h"
#include "net.h"
#include "patchlevel.h"
#include "preferences.h"
#include "db.h"
#include "dm.h"
#include "sites.h"
#include "indiapi.h"

extern FILE *fopenh (char *name, char *how);
extern FILE *fopend (char *name, char *sdir, char *how);
extern FILE *fopendq (char *fn, char *sdir, char *how);
extern INumberVectorProperty *indi_getNProperty (char *dev, char *prop);
extern Now *mm_get_now (void);
extern Obj *db_basic (int id);
extern Obj *db_scan (DBScan *sp);  
extern char *dm_viewupres(void);
extern char *sun_viewupres(void);
extern char *e_viewupres(void);
extern char *m_viewupres(void);
extern char *mars_viewupres(void);
extern char *ng_viewupres (void);
extern char *jm_viewupres(void);
extern char *sm_viewupres(void);
extern char *um_viewupres(void);
extern char *sv_viewupres(void);
extern char *ss_viewupres(void);
extern char *mm_autortres(void);
extern Obj *fav_scan (int *np, int typemask);
extern char *cns_name (int id);
extern char *expand_home (char *path);
extern char *getPrivateDir (void);
extern char *getShareDir (void);
extern char *getXRes (char *name, char *def);
extern char *tempfilename (char *buf, char *name, char *suffix);
extern char *obj_description (Obj *op);
extern char *strtolower (char *str);
extern char *syserrstr (void);
extern char *userResFile (void);
extern double atod (char *buf);
extern double delra (double dra);
extern double hznAlt (double az);
extern double dm_sep (Obj *op);
extern void hznProfile (int i, double *altp, double *azp);
extern int hznDrawing (void);
extern int hznNProfile (void);
extern int GSCFetch (double ra0, double dec0, double fov, double fmag, ObjF **spp, int nspp, char msg[]);
extern int GSCSetup (char *cdp, char *chp, char msg[]);
extern int UCACSetup (char *path, char msg[]);
extern int UCACFetch (double r0, double d0, double fov, double fmag, ObjF **opp, int nopp, char msg[]);
extern int USNOFetch (double r0, double d0, double fov, double fmag, ObjF **opp, int nopp, char msg[]);
extern int USNOSetup (char *cdp, int wantgsc, char msg[]);
extern int any_ison (void);
extern int cns_pick (double ra, double dec, double e);
extern int compile_expr (char *ex, char *errbuf);
extern int confirm (void);
extern int dateOK (Now *np, Obj *op);
extern int db_load1 (void);
extern int db_n (void);
extern int db_set_field (char bp[], int id, PrefDateFormat pref, Obj *op);
extern int execute_expr (double *vp, char *errbuf);
extern int existsh (char *path);
extern int existd (char *path, char *sdir);
extern int f_ison (void);
extern int fav_already (char *name);
extern int fav_get_loaded (Obj ***oppp);
extern int fs_fetch (Now *np, double ra, double dec, double fov, double mag, ObjF **opp);
extern int fs_pmon (void);
extern int fs_setnodups (int new);
extern int gal_opfind (Obj *op);
extern int gsc23fetch (char *url, Now *np, double ra, double dec, double fov, double mag, ObjF **opp, int nop, char msg[]);
extern int indi_connected (void);
extern int indi_setNProperty (char *dev, char *prop, char *n[], double v[], int nv, char whynot[]);
extern int indi_setTProperty (char *dev, char *prop, char *n[], char *v[], int nv, char whynot[]);
extern int is_deepsky (Obj *op);
extern int listing_ison (void);
extern int ol_isUp (void);
extern int openh (char *name, int flags, ...);
extern int plot_ison (void);
extern int prog_isgood (void);
extern int sc_isGotoOn (void);
extern int sr_autosaveon (void);
extern int sr_isUp (void);
extern int sr_refresh (void);
extern int sr_save (int talk);
extern int srch_eval (double Mjd, double *tmincp);
extern int srch_ison (void);
extern int stopd_check (void);
extern int strnncmp (char *s1, char *s2);
extern int sv_ison (void);
extern int sv_hznOpOk (Obj *op);
extern int svf_filter_ok (Obj *op);
extern int svf_ismanaged (void);
extern int telIsOn (void);
extern int tickmarks (double min, double max, int numdiv, double ticks[]);
extern int tz_fromsys (Now *np);
extern int xe2chkfile (char *file, char *msg);
extern int xe2fetch (char *file, Now *np, double ra, double dec, double fov, double mag, ObjF **opp, char *msg);
extern int xe3chkdir (char *dir, char *msg);
extern int xe3fetch (char *dir, double ra, double dec, double fov, double mag, ObjF **opp, int nop, char *msg);
extern void all_newdb (int appended);
extern void all_newfavs (void);
extern void all_selection_mode (int whether);
extern void all_update (Now *np, int how_much);
extern void ano_newres (void);
extern void av_load (Obj *op);
extern void av_manage (void);
extern void c_manage (void);
extern void c_update (Now *np, int force);
extern void calm_newres (void);
extern void calm_set (Now *np);
extern void cc_manage (void);
extern void cc_update (Now *np, int force);
extern void compiler_log (char *name, double value);
extern void db_clr_cp (void);
extern void db_connect_fifo (void);
extern void db_invalidate (void);
extern void db_loadinitial (void);
extern void db_manage (void);
extern void db_newdb (int appended);
extern void db_read (char *fn);
extern void db_scaninit (DBScan *sp, int tmask, ObjF *op, int nop);
extern void db_update (Obj *op);
extern void dm_create_shell (void);
extern void dm_manage (void);
extern void dm_newfavs (void);
extern void dm_riset (Now *np, Obj *op, RiseSet *rsp);
extern void dm_selection_mode (int whether);
extern void dm_separation (Obj *p, Obj *q, double *sep);
extern void dm_update (Now *np, int how_much);
extern void e_manage (void);
extern void e_newfavs (void);
extern void e_newdb (int appended);
extern void e_newres (void);
extern void e_selection_mode (int whether);
extern void e_update (Now *np, int force);
extern void f_off (void);
extern void f_on (void);
extern void fav_add (Obj *op);
extern void fav_manage (void);
extern void fav_newdb (void);
extern void fs_create (void);
extern void fs_dm_angle (char out[], double a);
extern void fs_dms_angle (char out[], double a);
extern void fs_manage (void);
extern void fs_mtime (char out[], double t);
extern void fs_pangle (char out[], double a);
extern void fs_prdec (char out[], double jd);
extern void fs_ra (char out[], double ra);
extern void fs_time (char out[], double t);
extern void fs_timestamp (Now *np, char stamp[]);
extern void fs_tz (char *timezonename, int tzpref, Now *np);
extern void gal_manage (void);
extern void gal_opscroll (Obj *op);
extern void gk_mag (double g, double k, double rp, double rho, double *mp);
extern void hlp_config (void);
extern void hlp_dialog (char *tag, char *deflt[], int ndeflt);
extern void hznAdd (int init, double alt, double az);
extern void hznEditingOff (void);
extern void hznRawProfile (int on);
extern void hzn_manage (void);
extern void inc_mjd (Now *np, double inc, int rev, int rtcflag);
extern void indi_createShell (void);
extern void indi_manage (void);
extern void indi_newres (void);
extern void ir_manage (void);
extern void ir_setstar (double ix, double iy);
extern int ir_setting (void);
extern void jm_manage (void);
extern void jm_newdb (int appended);
extern void jm_newres (void);
extern void jm_selection_mode (int whether);
extern void jm_update (Now *np, int how_much);
extern void listing (void);
extern void llibration (double JD, double *llatp, double *llonp);
extern void lst_log (char *name, char *str);
extern void lst_manage (void);
extern void lst_selection (char *name);
extern void m_manage (void);
extern void m_newdb (int appended);
extern void m_newres (void);
extern void m_selection_mode (int whether);
extern void m_update (Now *np, int how_much);
extern void make_objgcs (void);
extern void mars_manage (void);
extern void mars_newres (void);
extern void mars_selection_mode (int whether);
extern void mars_update (Now *np, int force);
extern void marsm_manage (void);
extern void marsm_newdb (int appended);
extern void marsm_newres (void);
extern void marsm_selection_mode (int whether);
extern void marsm_update (Now *np, int how_much);
extern void mm_connActions (void);
extern void mm_external (void);
extern void mm_movie (double stepsz);
extern void mm_newcaldate (double newmjd);
extern void mm_newres (void);
extern void mm_selection_mode (int whether);
extern void mm_setll (double slat, double slng, int update);
extern void mm_startrt(void);
extern void moonnf (double Mjd, double *Mjdn, double *Mjdf);
extern void msg_manage (void);
extern void net_create (void);
extern void net_manage (void);
extern void ng_manage (void);
extern void ng_newfavs (void);
extern void ng_newres (void);
extern void ng_update (Now *np, int force);
extern void obj_manage (void);
extern void obj_newdb (int appended);
extern void obj_newres (void);
extern void ol_manage (void);
extern void ol_setObj (Obj *op);
extern void plot (void);
extern void plot_manage (void);
extern void plt_log (char *name, double value);
extern void plt_selection (char *name);
extern void pm_down (void);
extern void pm_manage (void);
extern void pm_set (int percentage);
extern void pm_up (void);
extern void redraw_screen (int how_much);
extern void register_selection (char *name);
extern void riset_cir (Now *np, Obj *op, double dis, RiseSet *rp);
extern void sc_gethost (char **host, char **port);
extern void sc_goto (Obj *op);
extern void sc_manage (void);
extern void sc_unmanage (void);
extern void setButtonInfo (void);
extern void setXRes (char *name, char *val);
extern void setXRes (char *name, char *value);
extern void set_t0 (Now *np);
extern void sm_manage (void);
extern void sfifo_openin(void);
extern void sfifo_closein(void);
extern void sm_newdb (int appended);
extern void sm_newres (void);
extern void sm_selection_mode (int whether);
extern void sm_update (Now *np, int how_much);
extern void sr_addFallbacks (void);
extern void sr_chknightv (void);
extern void sr_manage (void);
extern void sr_xmanage (void);
extern void src_manage (void);
extern void srch_log (char *name, double value);
extern void srch_manage (void);
extern void srch_selection (char *name);
extern void srch_selection_mode (int whether);
extern void srf_manage (void);
extern void ss_manage (void);
extern void ss_newdb (int appended);
extern void ss_newres (void);
extern void ss_update (Now *np, int how_much);
extern void sun_update (Now *np, int how_much);
extern void stopd_down (void);
extern void stopd_up (void);
extern void sun_newres (void);
extern void sv_all (Now *np);
extern void sv_hznOn(void);
extern void sv_amagoff (void);
extern void sv_drawimdot (double ix, double iy, int rad, int color);
extern void sv_dspFITS (void);
extern void sv_getcenter (int *aamodep, double *fovp, double *altp, double *azp, double *rap, double *decp);
extern void sv_getfldstars (ObjF **fspp, int *nfsp);
extern void sv_id (Obj *op);
extern void sv_loadfs (int force);
extern void sv_manage (void);
extern void sv_newFITS (void);
extern void sv_newdb (int appended);
extern void sv_newres (void);
extern void sv_point (Obj *op);
extern void sv_showkeeptelvis (int on);
extern void sv_scopeMark (Obj *);
extern void sv_update (Now *np, int how_much);
extern void svbs_manage (Obj *op);
extern void svbs_newres (void);
extern void svf_automag (double fov);
extern void svf_getmaglimits (int *stmagp, int *ssmagp, int *dsmagp, int *magstpp);
extern void svf_gettables (char tt[NOBJTYPES], char ct[NCLASSES]);
extern void svf_manage (void);
extern void svf_setmaglimits (int stmag, int ssmag, int dsmag, int magstp);
extern void svf_settables (char tt[NOBJTYPES], char ct[NCLASSES]);
extern void svf_unmanage (void);
extern void telGoto (Obj *op);
extern void time_fromsys (Now *np);
extern void um_manage (void);
extern void um_newdb (int appended);
extern void um_newres (void);
extern void um_selection_mode (int whether);
extern void um_update (Now *np, int how_much);
extern void version (void);
extern void watch_cursor (int want);
extern void wdb_manage (void);
extern void wtip_alldown (void);
extern void wtip_init (void);
extern void xe_msg (int modal, char *fmt, ...);
extern void zero_mem (void *loc, unsigned len);

#if defined (X_PROTOCOL)
/* these require X Windows */

#include "plot.h"
#include "ps.h"
#include "rotated.h"
#include "skyeyep.h"
#include "skyhist.h"
#include "skyip.h"
#include "skylist.h"
#include "skytoolbar.h"
#include "trails.h"

#define XtD     XtDisplay(toplevel_w)
extern Colormap xe_cm;
extern String fallbacks[];
extern Widget svshell_w;
extern Widget toplevel_w;
extern XtAppContext xe_app;
extern char myclass[];

extern Colormap checkCM (Colormap cm, int nwant);
extern Widget calm_create (Widget parent);
extern Widget createFSM (Widget p, char **suffix, int nsuffix, char *sdir,
    XtCallbackProc cb);
extern XFontStruct * getXResFont (char *rn);
extern XImage *create_xim (int w, int h);
extern int alloc_ramp (Display *dsp, XColor *basep, Colormap cm, Pixel pix[], int maxn);
extern int get_color_resource (Widget w, char *cname, Pixel *p);
extern int gif2X (Display *dsp, Colormap cm, unsigned char gif[], int ngif, int *wp, int *hp, unsigned char **gifpix, XColor xcols[256], char err[]);
extern int gif2pm (Display *dsp, Colormap cm, unsigned char gif[], int ngif, int *wp, int *hp, Pixmap *pmp, char why[]);
extern int gray_ramp (Display *dsp, Colormap cm, Pixel **pix);
extern int isUp (Widget shell_w);
extern int jpeg2pm (Display *dsp, Colormap cm, FILE *jpegfp, int *wp, int *hp, Pixmap *pmp, XColor xcols[256], char why[]);
extern unsigned char *jpegRead(FILE *infile, int *width, int *height, unsigned char r[256], unsigned char g[256], unsigned char b[256], char why[]);
extern int plot_cartesian (DrawInfo *di, Widget widget, Dimension nx, Dimension ny);
extern void plot_coords (Widget da_w, DrawInfo *di, int window_x, int window_y);
extern void XCheck (XtAppContext app);
extern void XPS_cursor (Cursor c);
extern void ano_cb (Widget w, XtPointer client, XtPointer call);
extern void ano_cursor (Cursor c);
extern void ano_draw (Widget w, Drawable dr, int convwx(double *ap, double *bp,
    int *xp, int *yp, int w2x, int arg), int arg);
extern void av_cursor (Cursor c);
extern void buttonAsButton (Widget w, int whether);
extern void c_cursor (Cursor c);
extern void cc_cursor (Cursor c);
extern void centerScrollBars (Widget sw_w);
extern void db_cursor (Cursor c);
extern void defaultTextFN (Widget w, int setcols, char *x, char *y);
extern void dm_cursor (Cursor c);
extern void dm_setup_cb (Widget w, XtPointer client, XtPointer call);
extern void e_cursor (Cursor c);
extern void f_date (Widget w, double jd);
extern void f_dm_angle (Widget w, double a);
extern void f_dms_angle (Widget w, double a);
extern void f_double (Widget w, char *fmt, double f);
extern void f_mtime (Widget w, double t);
extern void f_pangle (Widget w, double a);
extern void f_prdec (Widget w, double a);
extern void f_ra (Widget w, double ra);
extern void f_sexa (Widget wid, double a, int w, int fracbase);
extern void f_showit (Widget w, char *s);
extern void f_string (Widget w, char *s);
extern void f_time (Widget w, double t);
extern void fav_cursor (Cursor c);
extern void field_log (Widget w, double value, int logv, char *str);
extern void fixTextCursor (Widget w);
extern void freeXColors (Display *dsp, Colormap cm, XColor xcols[], int nxcols);
extern void fs_cursor (Cursor c);
extern void gal_cursor (Cursor c);
extern void get_something (Widget w, char *resource, XtArgVal value);
extern void get_tracking_font (Display *dsp, XFontStruct **fspp);
extern void get_views_font (Display *dsp, XFontStruct **fspp);
extern void get_xmlabel_font (Widget w, XFontStruct **f);
extern void get_xmstring (Widget w, char *resource, char **txtp);
extern void hzn_cursor (Cursor c);
extern void hzn_unmanage (void);
extern void jm_cursor (Cursor c);
extern void loadGreek (Display *dsp, Drawable win, GC *greekgcp, XFontStruct **greekfspp);
extern void lst_cursor (Cursor c);
extern void m_cursor (Cursor c);
extern void main_cursor (Cursor c);
extern void make_logo (Widget rc);
extern void mars_cursor (Cursor c);
extern void marsm_cursor (Cursor c);
extern void ml_add (Drawable pm, Widget timestamp);
extern void ml_cursor (Cursor c);
extern void ml_manage (void);
extern int ml_addacc (Arg args[], int n);
extern void mm_create (Widget mainrc);
extern void mm_go_cb (Widget w, XtPointer client, XtPointer call);
extern void msg_cursor (Cursor c);
extern void newEnv (int *argcp, char *argv[]);
extern void ng_cursor (Cursor c);
extern void obj_cursor (Cursor c);
extern void obj_pickgc (Obj *op, Widget w, GC *gcp);
extern void ol_cursor (Cursor c);
extern void pixCache (XColor *xcp);
extern void plt_cursor (Cursor c);
extern void pm_cursor (Cursor c);
extern void pref_create_pulldown (Widget mb_w);
extern void prompt_map_cb (Widget w, XtPointer client, XtPointer call);
extern void query (Widget tw, char *msg, char *label0, char *label1, char *label2, void (*func0)(), void (*func1)(), void (*func2)());
extern void sc_cursor (Cursor c);
extern void set_something (Widget w, char *resource, XtArgVal value);
extern void set_tracking_font (Display *dsp, XFontStruct *fsp);
extern void set_views_font (Display *dsp, XFontStruct *fsp);
extern void set_xmstring (Widget w, char *resource, char *text);
extern void setup_icon(Widget w);
extern void sl_cursor (Cursor c);
extern void sm_cursor (Cursor c);
extern void splashOpen (int *argc, char *argv[], XrmOptionDescRec options[], int nops);
extern void splashClose (void);
extern void splashMsg (char *fmt, ...);
extern void sr_cursor (Cursor c);
extern void sr_getDirPM (Pixmap *pmopen, Pixmap *pmclose);
extern void sr_reg (Widget w, char *res, char *cat, int autosav);
extern void sr_unreg (Widget w);
extern void srch_cursor (Cursor c);
extern void ss_cursor (Cursor c);
extern void sun_cursor (Cursor c);
extern void sun_manage (void);
extern void sv_cursor (Cursor c);
extern void sv_draw_obj (Display *dsp, Drawable win, GC gc, Obj *op, int x, int y, int diam, int dotsonly);
extern void sv_draw_obj_x (Display *dsp, Drawable win, GC gc, Obj *op, int x, int y, int diam, int dotsonly, int flip_tb_x, int flip_lr_x, int aa_mode_x, int cyl_proj_x, double altdec_x, double azra_x, double vfov_x, double hfov_x, int w_x, int h_x);
extern void sv_other (double altdec, double azra, int aa, double *altdecp, double *azrap);
extern void svf_create (Widget shell_w);
extern void svf_cursor (Cursor c);
extern void svh_cursor (Cursor c);
extern void timestamp (Now *np, Widget w);
extern void um_cursor (Cursor c);
extern void v_cursor (Cursor c);
extern void wdb_cursor (Cursor c);
extern void wtip (Widget w, char *tip);


#endif

#endif /* _XEPHEM_H */

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: xephem.h,v $ $Date: 2012/12/30 17:01:02 $ $Revision: 1.63 $ $Name:  $
 */

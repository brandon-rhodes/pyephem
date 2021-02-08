/* this file glues XEphem to INDI.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>


#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Scale.h>
#include <Xm/SelectioB.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>

#include "xephem.h"


static void sc_create_w (void);
static void sc_help_cb (Widget w, XtPointer client, XtPointer call);
static void oneConfigRow (Widget rc_w, Widget *tb_wp, XtCallbackProc cb,
    char *savereg, char *prompt, char *name, Widget *v_wp, Widget *m_wp,
    Widget *b_wp);
static int getINDIprop (Widget tf_w, char **dev, char **prop, char **ele);
static int getINDIele (Widget w1, Widget w2, char **dev, char **prop,
    char *ele[]);
static INumber *findNumber (INumberVectorProperty *nvp, char *name);
static double toINDIValue (double localv, Widget m_w, Widget b_w);
static double fromINDIValue (double indiv, Widget m_w, Widget b_w);

static void jd_cb (Widget w, XtPointer client, XtPointer call);
static void posixtm_cb (Widget w, XtPointer client, XtPointer call);
static void ll_cb (Widget w, XtPointer client, XtPointer call);
static void wx_cb (Widget w, XtPointer client, XtPointer call);
static void marker_cb (Widget w, XtPointer client, XtPointer call);
static void marker_tcb (XtPointer client, XtIntervalId *id);
static void close_cb (Widget w, XtPointer client, XtPointer call);

static Widget scopeshell_w;	/* overall shell */
static Widget host_w;		/* INDI tcp/ip host TF */
static Widget port_w;		/* INDI tcp/ip port TF */
static Widget jdv_w;		/* INDI JD value */
static Widget jdm_w;		/* INDI JD slope */
static Widget jdb_w;		/* INDI JD offset */
static Widget posixtmv_w;	/* INDI POSIX time value */
static Widget latv_w;		/* INDI latitude value */
static Widget latm_w;		/* INDI latitude slope */
static Widget latb_w;		/* INDI latitude offset */
static Widget lngv_w;		/* INDI longitude value */
static Widget lngm_w;		/* INDI longitude slope */
static Widget lngb_w;		/* INDI longitude offset */
static Widget tempv_w;		/* INDI temperature value */
static Widget tempm_w;		/* INDI temperature slope */
static Widget tempb_w;		/* INDI temperature offset */
static Widget presv_w;		/* INDI pressure value */
static Widget presm_w;		/* INDI pressure slope */
static Widget presb_w;		/* INDI pressure offset */
static Widget rav_w;		/* INDI ra value */
static Widget ram_w;		/* INDI ra slope */
static Widget rab_w;		/* INDI ra offset */
static Widget decv_w;		/* INDI dec value */
static Widget decm_w;		/* INDI dec slope */
static Widget decb_w;		/* INDI dec offset */
static Widget edbv_w;		/* INDI edb value */
static Widget gotorav_w;	/* INDI goto ra value */
static Widget gotoram_w;	/* INDI goto ra slope */
static Widget gotorab_w;	/* INDI goto ra offset */
static Widget gotodecv_w;	/* INDI goto dec value */
static Widget gotodecm_w;	/* INDI goto dec slope */
static Widget gotodecb_w;	/* INDI goto dec offset */

static Widget edbon_w;		/* TB whether edb GOTO is on */
static Widget rdon_w;		/* TB whether ra/dec GOTO is on */
static Widget markeron_w;	/* TB whether skyview marker is on */

static XtIntervalId marker_tid;	/* timer to display sky marker */
#define	MARK_INT	333	/* marker update interval, ms */

static char scopecategory[] = "INDI Configuration";

void 
sc_manage()
{
	if (!scopeshell_w)
	    sc_create_w();

	XtPopup (scopeshell_w, XtGrabNone);
	set_something (scopeshell_w, XmNiconic, (XtArgVal)False);
}

void 
sc_unmanage()
{
	if (scopeshell_w)
	    XtPopdown (scopeshell_w);
}

/* return malloced name of host and port to connect to INDI services.
 * N.B. caller must XtFree() each
 */
void
sc_gethost (char **host, char **port)
{
	if (!host_w)
	    sc_create_w();
	*host = XmTextFieldGetString (host_w);
	*port = XmTextFieldGetString (port_w);
}

/* return whether we are sending goto commands */
int
sc_isGotoOn()
{
	if (!scopeshell_w)
	    sc_create_w();
	return (XmToggleButtonGetState(edbon_w) ||
						XmToggleButtonGetState(rdon_w));
}

/* send the given object to INDI if On and alt>0.
 */
void
sc_goto (op)
Obj *op;
{
	char buf[1024];

	/* check whether enabled */
	if (!sc_isGotoOn())
	    return;

	/* assert */
	if (!op) {
	    printf ("sc_goto called with no op\n");
	    abort();
	}

	/* send as edb if on */
	if (XmToggleButtonGetState (edbon_w)) {
	    char *dev, *prop, *ele;
	    db_write_line (op, buf);
	    if (!getINDIprop (edbv_w, &dev, &prop, &ele)) {
		char *bp = buf;
		if (indi_setTProperty (dev, prop, &ele, &bp, 1, buf) < 0) {
		    xe_msg (1, "%s", buf);
		    XmToggleButtonSetState (edbon_w, False, True);
		}
	    }
	}

	/* send as astrometric J2000 ra/dec if on */
	if (XmToggleButtonGetState (rdon_w)) {
	    char *dev, *prop, *ele[2];
	    double v[2];
	    Now now, *np;
	    Obj obj;

	    /* update ephemeris for op */
	    np = mm_get_now();
	    if (epoch != J2000) {
		now = *np;
		np = &now;
		epoch = J2000;
		obj = *op;
		op = &obj;
	    }
	    obj_cir (np, op);

	    /* scale to INDI coords */
	    v[0] = toINDIValue (op->s_ra, gotoram_w, gotorab_w);
	    v[1] = toINDIValue (op->s_dec, gotodecm_w, gotodecb_w);

	    /* send INDI ra/dec elements */
	    if (getINDIele (gotorav_w, gotodecv_w, &dev, &prop, ele) < 0) {
		XmToggleButtonSetState (rdon_w, False, True);
		return;
	    }
	    if (indi_setNProperty (dev, prop, ele, v, 2, buf) < 0) {
		xe_msg (1, "%s", buf);
		XmToggleButtonSetState (edbon_w, False, True);
	    }

	    /* clean up */
	    XtFree (dev);
	    XtFree (prop);
	    XtFree (ele[0]);
	    XtFree (ele[1]);
	}
}

/* called to put up or remove the watch cursor.  */
void
sc_cursor (c)
Cursor c;
{
	Window win;

	if (scopeshell_w && (win = XtWindow(scopeshell_w)) != 0) {
	    Display *dsp = XtDisplay(scopeshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create the main scope dialog.
 * init Running TB according to whether process is already running.
 */
static void
sc_create_w()
{
	Widget rc_w, f_w, sep_w;
	Widget scopef_w;
	Widget w;
	Arg args[20];
	int n;

	/* create shell and form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem INDI configuration"); n++;
	XtSetArg (args[n], XmNiconName, "INDI conf"); n++;
	scopeshell_w = XtCreatePopupShell ("INDIConfig",
				topLevelShellWidgetClass, toplevel_w, args, n);
	setup_icon (scopeshell_w);
	set_something (scopeshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (scopeshell_w, "XEphem*INDIConfig.x", scopecategory, 0);
	sr_reg (scopeshell_w, "XEphem*INDIConfig.y", scopecategory, 0);

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	scopef_w = XmCreateForm (scopeshell_w, "ScF", args, n);
	XtAddCallback (scopef_w, XmNhelpCallback, sc_help_cb, 0);
	XtManageChild (scopef_w);

	/* big vertical RC */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	rc_w = XmCreateRowColumn (scopef_w, "SRC", args, n);
	XtManageChild (rc_w);

	/* host/port form */

	n = 0;
	f_w = XmCreateForm (rc_w, "SHPF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (f_w, "SHL", args, n);
	    set_xmstring (w, XmNlabelString, "Host:");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 70); n++;
	    host_w = XmCreateTextField (f_w, "Host", args, n);
	    wtip (host_w, "Network host for connecting to INDI server");
	    fixTextCursor (host_w);
	    sr_reg (host_w, NULL, scopecategory, 1);
	    XtManageChild (host_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, host_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 5); n++;
	    w = XmCreateLabel (f_w, "SPL", args, n);
	    set_xmstring (w, XmNlabelString, "Port:");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    port_w = XmCreateTextField (f_w, "Port", args, n);
	    wtip (host_w, "Network port number for connecting to INDI server");
	    fixTextCursor (port_w);
	    sr_reg (port_w, NULL, scopecategory, 1);
	    XtManageChild (port_w);

	n = 0;
	w = XmCreateLabel (rc_w, "SGap", args, n);
	set_xmstring (w, XmNlabelString, " ");
	XtManageChild (w);

	/* config entries */

	oneConfigRow (rc_w, NULL, jd_cb, NULL,
				    "Send computer time as JD once to",
				    "JD", &jdv_w, &jdm_w, &jdb_w);
	oneConfigRow (rc_w, NULL, posixtm_cb, NULL,
				    "Send computer time as POSIX once to",
				    "POSIXTM", &posixtmv_w, NULL, NULL);
	oneConfigRow (rc_w, NULL, ll_cb, NULL,
				    "Send lat and long once to",
				    "Latitude", &latv_w, &latm_w, &latb_w);
	oneConfigRow (rc_w, NULL, NULL, NULL, NULL,
				    "Longitude", &lngv_w, &lngm_w, &lngb_w);
	oneConfigRow (rc_w, NULL, wx_cb, NULL,
				    "Get temp and pressure once from",
				    "Temperature", &tempv_w, &tempm_w,&tempb_w);
	oneConfigRow (rc_w, NULL, NULL, NULL, NULL,
				    "Pressure", &presv_w,&presm_w,&presb_w);
	oneConfigRow (rc_w, &edbon_w, NULL, "EnableEDB",
				    "Enable sending edb target to",
				    "EDB", &edbv_w, NULL, NULL);
	oneConfigRow (rc_w, &rdon_w, NULL, "EnableRADec",
				    "Enable sending RA/Dec goto to",
				"GotoRA", &gotorav_w, &gotoram_w, &gotorab_w);
	oneConfigRow (rc_w, NULL, NULL, NULL, NULL,
			    "GotoDec", &gotodecv_w, &gotodecm_w, &gotodecb_w);
	oneConfigRow (rc_w, &markeron_w, marker_cb, "SkyMarker",
				    "Enable Sky marker from",
				    "SkyRA", &rav_w, &ram_w, &rab_w);
	oneConfigRow (rc_w, NULL, NULL, NULL, NULL,
				    "SkyDec", &decv_w, &decm_w, &decb_w);

	/* start listening right away if default for sky marker is On.
	 * also manage keep-vis option in skyview
	 */
	if (XmToggleButtonGetState (markeron_w)) {
	    marker_tid = XtAppAddTimeOut (xe_app, MARK_INT, marker_tcb, 0);
	    sv_showkeeptelvis (1);
	}


	/* bottom controls */

	n = 0;
	f_w = XmCreateForm (rc_w, "SHPF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNtopOffset, 5); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    sep_w = XmCreateSeparator (f_w, "Sep", args, n);
	    XtManageChild (sep_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 10); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomOffset, 5); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 25); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 35); n++;
	    w = XmCreatePushButton (f_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, close_cb, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 10); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomOffset, 5); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 65); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 75); n++;
	    w = XmCreatePushButton (f_w, "Help", args, n);
	    XtAddCallback (w, XmNactivateCallback, sc_help_cb, NULL);
	    XtManageChild (w);
}

/* add a row to the configuration table.
 */
static void
oneConfigRow (
Widget rc_w,		/* table RC */
Widget *tb_wp,		/* TB widget, or NULL */
XtCallbackProc cb,	/* tb callback, or NULL */
char *savereg,		/* name of persisent TB, or NULL */
char *prompt,		/* tb label, or NULL */
char *name,		/* base of widget names */
Widget *v_wp,		/* indi property value TF widget */
Widget *m_wp,		/* slope TF widget, or NULL */
Widget *b_wp)		/* intercept TF widget, or NULL */
{
	Widget w, f_w;
	Arg args[20];
	char tfname[64];
	int n;

	/* main form */
	n = 0;
	f_w = XmCreateForm (rc_w, "SRCF", args, n);
	XtManageChild (f_w);

	/* toggle button on left, if we have prompt */
	if (prompt) {
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    w = XmCreateToggleButton (f_w, savereg?savereg:"SFTB", args, n);
	    if (cb)
		XtAddCallback (w, XmNvalueChangedCallback, cb, NULL);
	    set_xmstring (w, XmNlabelString, prompt);
	    XtManageChild (w);
	    if (tb_wp)
		*tb_wp = w;
	    if (savereg)
		sr_reg (w, NULL, scopecategory, 1);
	}

	/* value and optional slope and intercept TFs */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 40); n++;
	XtSetArg (args[n], XmNcolumns, 40); n++;
	sprintf (tfname, "%sValue", name);
	*v_wp = XmCreateTextField (f_w, tfname, args, n);
	fixTextCursor (*v_wp);
	sr_reg (*v_wp, NULL, scopecategory, 1);
	wtip (*v_wp, "INDI Device.Property.Element for this quantity");
	XtManageChild (*v_wp);

	if (m_wp && b_wp) {
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, *v_wp); n++;
	    w = XmCreateLabel (f_w, "SM", args, n);
	    set_xmstring (w, XmNlabelString, " x ");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNcolumns, 8); n++;
	    sprintf (tfname, "%sSlope", name);
	    *m_wp = XmCreateTextField (f_w, tfname, args, n);
	    sr_reg (*m_wp, NULL, scopecategory, 1);
	    fixTextCursor (*m_wp);
	    wtip (*m_wp, "Scale multiplier applied to INDI value");
	    XtManageChild (*m_wp);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, *m_wp); n++;
	    w = XmCreateLabel (f_w, "SL", args, n);
	    set_xmstring (w, XmNlabelString, " + ");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 8); n++;
	    sprintf (tfname, "%sOffset", name);
	    *b_wp = XmCreateTextField (f_w, tfname, args, n);
	    sr_reg (*b_wp, NULL, scopecategory, 1);
	    fixTextCursor (*b_wp);
	    wtip (*b_wp, "Offset added to INDI value");
	    XtManageChild (*b_wp);
	}
}

/* called when the help button is hit */
/* ARGSUSED */
static void
sc_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
	    "Configure how INDI services connect with XEphem."
	};

	hlp_dialog ("SkyView_telescope", msg, sizeof(msg)/sizeof(msg[0]));

}

/* callback to send JD */
static void
jd_cb (Widget w, XtPointer client, XtPointer call)
{
	char *dev, *prop, *ele;
	char buf[1024];
	Now now;
	double v;

	/* act like a pushbutton */
	if (!XmToggleButtonGetState(w))
	    return;
	XmToggleButtonSetState (w, False, True);

	/* gather property name */
	if (getINDIprop (jdv_w, &dev, &prop, &ele) < 0)
	    return;

	/* convert computer mjd to indi jd and send */
	time_fromsys (&now);
	v = toINDIValue (now.n_mjd, jdm_w, jdb_w);
	if (indi_setNProperty (dev, prop, &ele, &v, 1, buf) < 0)
	    xe_msg (1, "%s", buf);

	/* clean up */
	XtFree (dev);
	XtFree (prop);
	XtFree (ele);
}

/* callback to send time in POSIX format */
static void
posixtm_cb (Widget w, XtPointer client, XtPointer call)
{
	char *dev, *prop, *ele;
	char buf[1024], *bp = buf;
	int mm, yy, d, h, m, s;
	double dd, dh, dm, ds;
	Now now;

	/* act like a pushbutton */
	if (!XmToggleButtonGetState(w))
	    return;
	XmToggleButtonSetState (w, False, True);

	/* gather property name */
	if (getINDIprop (posixtmv_w, &dev, &prop, &ele) < 0)
	    return;

	/* convert computer mjd to POSIX and send */
	time_fromsys (&now);
	mjd_cal (now.n_mjd, &mm, &dd, &yy);
	d = (int)dd;
	dh = (dd - d)*24.;
	h = (int)dh;
	dm = (dh - h)*60.;
	m = (int)dm;
	ds = (dm - m)*60.;
	if (ds >= 59.5) {
	    s = 0;
	    if (++m == 60) {
		m = 0;
		h += 1; /* TODO: roll date if hits 24 */
	    }
	} else
	    s = (int)ds;

	sprintf (buf, "%4d-%02d-%02dT%02d:%02d:%02d", yy, mm, d, h, m, s);

	if (indi_setTProperty (dev, prop, &ele, &bp, 1, buf) < 0)
	    xe_msg (1, "%s", buf);

	/* clean up */
	XtFree (dev);
	XtFree (prop);
	XtFree (ele);
}

/* callback to send lat/long */
static void
ll_cb (Widget w, XtPointer client, XtPointer call)
{
	Now *np = mm_get_now();
	char *dev, *prop, *ele[2];
	double v[2];
	char buf[1024];

	/* act like a pushbutton */
	if (!XmToggleButtonGetState(w))
	    return;
	XmToggleButtonSetState (w, False, True);

	/* get INDI elements with lat/long */
	if (getINDIele (latv_w, lngv_w, &dev, &prop, ele) < 0)
	    return;

	v[0] = toINDIValue (lat, latm_w, latb_w);
	v[1] = toINDIValue (lng, lngm_w, lngb_w);
	if (indi_setNProperty (dev, prop, ele, v, 2, buf) < 0)
	    xe_msg (1, "%s", buf);

	/* clean up */
	XtFree (dev);
	XtFree (prop);
	XtFree (ele[0]);
	XtFree (ele[1]);
}

/* set our temp/pres from INDI */
static void
wx_cb (Widget w, XtPointer client, XtPointer call)
{
	INumberVectorProperty *wxp;
	char *dev, *prop, *ele[2];

	/* act like a pushbutton */
	if (!XmToggleButtonGetState(w))
	    return;
	XmToggleButtonSetState (w, False, True);

	/* get INDI elements with current temp/pres */
	if (getINDIele (tempv_w, presv_w, &dev, &prop, ele) < 0)
	    return;

	wxp = indi_getNProperty (dev, prop);
	if (wxp) {
	    INumber *tnp = findNumber (wxp, ele[0]);
	    if (tnp) {
		INumber *pnp = findNumber (wxp, ele[1]);
		if (pnp) {
		    Now *np = mm_get_now();
		    temp = fromINDIValue (tnp->value, tempm_w, tempb_w);
		    pressure = fromINDIValue (pnp->value, presm_w, presb_w);
		    redraw_screen (1);
		} else {
		    xe_msg (1, "INDI: %s.%s.%s not found", dev,prop,ele[1]);
		}
	    } else {
		xe_msg (1, "INDI: %s.%s.%s not found", dev,prop,ele[0]);
	    }
	}

	/* clean up */
	XtFree (dev);
	XtFree (prop);
	XtFree (ele[0]);
	XtFree (ele[1]);
}

static void
close_cb (Widget w, XtPointer client, XtPointer call)
{
	XtPopdown (scopeshell_w);
}

/* toggle whether to display the sky marker */
static void
marker_cb (Widget w, XtPointer client, XtPointer call)
{
	int on = XmToggleButtonGetState (w);

	/* always remove any old timer */
	if (marker_tid) {
	    XtRemoveTimeOut (marker_tid);
	    marker_tid = (XtIntervalId)0;
	}

	/* start fresh if turning on */
	if (on)
	    marker_tid = XtAppAddTimeOut (xe_app, MARK_INT, marker_tcb, 0);

	/* only show keep-vis in skyview is marker is on */
	sv_showkeeptelvis(on);
}

/* timer callback to put up the sky marker.
 * ok if properties not available now
 */
static void
marker_tcb (XtPointer client, XtIntervalId *id)
{
	char *dev, *prop, *ele[2];
	int ok = 1;

	/* if connected, get names of ra/dec properties */
	if (indi_connected() == 0) {
	    if (getINDIele (rav_w,decv_w,&dev,&prop,ele) < 0)
		ok = 0;	/* already issued xe_msg() */
	    else {
		/* get the live vector, ok if not available yet */
		INumberVectorProperty *rap = indi_getNProperty (dev, prop);
		if (rap) {
		    /* extract the ra/dec values */
		    INumber *rnp = findNumber (rap, ele[0]);
		    if (rnp) {
			INumber *dnp = findNumber (rap, ele[1]);
			if (dnp) {
			    Obj o;
			    Now *np;

			    /* build object */
			    memset (&o, 0, sizeof(o));
			    o.f_RA = fromINDIValue (rnp->value, ram_w, rab_w);
			    o.f_dec = fromINDIValue (dnp->value, decm_w,decb_w);
			    o.o_type = FIXED;
			    o.f_epoch = J2000;

			    /* mark, on screen */
			    np = mm_get_now();
			    obj_cir (np, &o);
			    sv_scopeMark (&o);
			} else {
			    xe_msg(1,"INDI:%s.%s.%s not found",dev,prop,ele[1]);
			    ok = 0;
			}
		    } else {
			xe_msg (1, "INDI: %s.%s.%s not found", dev,prop,ele[0]);
			ok = 0;
		    }
		}

		/* clean up */
		XtFree (dev);
		XtFree (prop);
		XtFree (ele[0]);
		XtFree (ele[1]);
	    }
	}

	/* repeat if ok */
	if (ok)
	    marker_tid = XtAppAddTimeOut (xe_app, MARK_INT, marker_tcb, 0);
	else
	    XmToggleButtonSetState (markeron_w, False, True);
}

/* get the name of the INDI d.p.e(s) described by TF w1 and possibly w2.
 * return 0 if ok else issue xe_msg() and return -1.
 * N.B. if return 0 caller must XtFree() each pointer
 */
static int
getINDIele (Widget w1, Widget w2, char **dev, char **prop, char *ele[])
{
	char *dev1, *prop1;
	char *dev2, *prop2;
	char *myele[2];

	/* gather property names */
	if (getINDIprop (w1, &dev1, &prop1, &myele[0]) < 0)
	    return (-1);
	if (w2 && getINDIprop (w2, &dev2, &prop2, &myele[1]) < 0) {
	    XtFree (dev1);
	    XtFree (prop1);
	    XtFree (myele[0]);
	    return (-1);
	}

	/* if two then must be the same */
	if (w2 && (strcmp (dev1,dev2) || strcmp (prop1,prop2))) {
	    xe_msg (1, "Properties %s.%s and %s.%s must match to be atomic",
						    dev1, prop1, dev2, prop2);
	    XtFree (dev1);
	    XtFree (prop1);
	    XtFree (myele[0]);
	    XtFree (dev2);
	    XtFree (prop2);
	    XtFree (myele[1]);
	    return (-1);
	}

	/* ok */
	*dev = dev1;
	*prop = prop1;
	ele[0] = myele[0];
	if (w2)
	    ele[1] = myele[1];
	return (0);
}

/* extract the device.property.element name from the given text field.
 * return 0 if ok else issue xe_msg() and return -1.
 * N.B. if return 0 caller must XtFree() each pointer
 */
static int
getINDIprop (Widget tf_w, char **dev, char **prop, char **ele)
{
	char *string = XmTextFieldGetString (tf_w);
	char devstr[64], propstr[64], elestr[64];
	int ok;

	ok = sscanf (string, "%[^.].%[^.].%s", devstr, propstr, elestr) == 3;
	if (!ok) {
	    xe_msg (1, "%s:\nINDI format must be dev.prop.element", string);
	}
	XtFree (string);
	if (ok) {
	    *dev = XtNewString (devstr);
	    *prop = XtNewString (propstr);
	    *ele = XtNewString (elestr);
	    return (0);
	}
	return (-1);
}

/* find a particular INumber in a INumberVector */
static INumber *
findNumber (INumberVectorProperty *nvp, char *name)
{
	INumber *np;

	for (np = nvp->np; np < &nvp->np[nvp->nnp]; np++)
	    if (strcmp (np->name, name) == 0)
		return (np);
	return (NULL);
}

/* convert the given local number to its INDI value using the slope and offset 
 * values in the given TF widgets
 */
static double
toINDIValue (double localv, Widget m_w, Widget b_w)
{
	char *mstr = XmTextFieldGetString (m_w);
	char *bstr = XmTextFieldGetString (b_w);
	double indiv = (localv - atod(bstr))/atod(mstr);
	XtFree (mstr);
	XtFree (bstr);
	return (indiv);
}

/* convert the given INDI number to its local value using the slope and offset 
 * values in the given TF widgets
 */
static double
fromINDIValue (double indiv, Widget m_w, Widget b_w)
{
	char *mstr = XmTextFieldGetString (m_w);
	char *bstr = XmTextFieldGetString (b_w);
	double localv = indiv*atod(mstr) + atod(bstr);
	XtFree (mstr);
	XtFree (bstr);
	return (localv);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: scope.c,v $ $Date: 2011/09/23 01:52:39 $ $Revision: 1.28 $ $Name:  $"};

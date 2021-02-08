/* display and operate the GUI for INDI Server or Device.
 *
 * Overall GUI is one big RowColumn, devrc_w
 *   First entry is a ScrolledText for the common message area, msg_w
 *   Then follows for each Device:
 *     One Form, gd->f_w
 *       Light, TB to reveal/hide gd->grc_w, Name, label
 *     One RowColumn for all Groups, gd->grc_w, containing for each group:
 *       One Form (unless no group name, then group is not hideable)
 *         TB to reveal/hide gg->prc_w, name
 *       One RowColumn for each Group, gg->prc_w, containing for each property:
 *         Form
 *           Light, Name, label
 *         One H RowColumn for each vector member, each of which contains
 *           Form
 *             Name label, value Label for read and/or TextField for write
 *   
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/ArrowB.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledW.h>
#include <Xm/PanedW.h>
#include <Xm/Scale.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#include "xephem.h"
#include "lilxml.h"
#include "base64.h"
#include "zlib.h"

#define	MSGR		10		/* number of rows in the message TF */
#define	MSGC		60		/* number of columns in the message TF*/
#define LIGHTW           9     		/* overall light width, pixels */
#define LIGHTH           9     		/* overall light height, pixels */
#define	WFLPOS		35		/* left writable field position */
#define	WFRPOS		75		/* right writable field position */
#define	INDENT		12		/* indent */
#define	INDITWIDTH	500		/* overall width of INDI table, pixels*/

/* one RC per group to implement spacial grouping of its properties */
typedef struct {
    char group[MAXINDIGROUP];		/* name of this group, NULL if anon */
    Widget prc_w;			/* RC for properties in this group */
} IGUIGroup;

/* GUI for one Device */
typedef struct {
    char device[MAXINDIDEVICE];		/* handy (same as in each property) */
    Widget f_w;				/* form for controls row */
    Widget grc_w;			/* RC for all groups for show/hide */
    Widget lda_w;			/* status light drawing area */
    IGUIGroup **grp;			/* malloced group ptr list */
    int ngrp;				/* number in grp[] */
    INumberVectorProperty **nvpp;	/* malloced number vector ptr list */
    int nnvpp;				/* number in nvpp[] */
    ITextVectorProperty **tvpp;		/* malloced text vector ptr list */
    int ntvpp;				/* number in tvpp[] */
    ISwitchVectorProperty **svpp;	/* malloced switch vector ptr list */
    int nsvpp;				/* number in svpp[] */
    ILightVectorProperty **lvpp;	/* malloced light vector ptr list */
    int nlvpp;				/* number in lvpp[] */
    IBLOBVectorProperty **bvpp;		/* malloced BLOB vector ptr list */
    int nbvpp;				/* number in bvpp[] */
} IGUIDevice;

static Widget createLt (Widget p_w, char *name, Arg args[], int n);
static void closeCB (Widget w, XtPointer client, XtPointer call);
static void connectCB (Widget w, XtPointer client, XtPointer call);
static void eraseCB (Widget w, XtPointer client, XtPointer call);
static void devLtCB (Widget w, XtPointer client, XtPointer call);
static void getLtColors (void);
static void drawLt (Widget ltw, IPState s);
static void drawDevLt (IGUIDevice *);
static void devTogCB (Widget w, XtPointer client, XtPointer call);
static void groupTogCB (Widget w, XtPointer client, XtPointer call);
static void displayBLOBCB (Widget w, XtPointer client, XtPointer call);
static void saveBLOBCB (Widget w, XtPointer client, XtPointer call);
static void indi_disconnect(void);
static void newBLOB_cb (Widget w, XtPointer client, XtPointer call);
static void newnum_cb (Widget w, XtPointer client, XtPointer call);
static void newnumdw_cb (Widget w, XtPointer client, XtPointer call);
static void newnumup_cb (Widget w, XtPointer client, XtPointer call);
static void newtext_cb (Widget w, XtPointer client, XtPointer call);
static void newswitch_cb (Widget w, XtPointer client, XtPointer call);
static void sendNumberVectorProperty (INumberVectorProperty *nvp);
static void sendTextVectorProperty (ITextVectorProperty *tvp);
static void sendSwitchVectorProperty (ISwitchVectorProperty *svp);
static void sendEnableBLOBs (IBLOBVectorProperty *bvp, int on);
static void traceon_cb (Widget w, XtPointer client, XtPointer call);
static int indi_connect(void);
static int launchCmd (XMLEle *root, char errmsg[]);
static int defTextCmd (XMLEle *root, char errmsg[]);
static int defNumberCmd (XMLEle *root, char errmsg[]);
static int defSwitchCmd (XMLEle *root, char errmsg[]);
static int defLightCmd (XMLEle *root, char errmsg[]);
static int defBLOBCmd (XMLEle *root, char errmsg[]);
static int setNumberCmd (XMLEle *root, char errmsg[]);
static int setTextCmd (XMLEle *root, char errmsg[]);
static int setSwitchCmd (XMLEle *root, char errmsg[]);
static int setLightCmd (XMLEle *root, char errmsg[]);
static int setBLOBCmd (XMLEle *root, char errmsg[]);
static int doMessage (XMLEle *root, char errmsg[]);
static void newGUIDevice (IGUIDevice *gd);
static void newGUIGroup (IGUIDevice *gd, IGUIGroup *gg);
static void newGUIWrNumber (INumber *np, Widget p_w, int lpos);
static void freeTextVectorProperty (ITextVectorProperty *tvp);
static void freeNumberVectorProperty (INumberVectorProperty *nvp);
static void freeSwitchVectorProperty (ISwitchVectorProperty *svp);
static void freeLightVectorProperty (ILightVectorProperty *lvp);
static void freeBLOBVectorProperty (IBLOBVectorProperty *lvp);
static int newGUINumberVector(INumberVectorProperty *nvp, IGUIDevice *gd,
    IGUIGroup *gg, char *errmsg);
static int setGUINumberVector (XMLEle *root, INumberVectorProperty *nvp,
    IGUIDevice *gd, char errmsg[]);
static int newGUITextVector(ITextVectorProperty *nvp, IGUIDevice *gd,
    IGUIGroup *gg, char *errmsg);
static int setGUITextVector (XMLEle *root, ITextVectorProperty *nvp,
    IGUIDevice *gd, char errmsg[]);
static int newGUISwitchVector(ISwitchVectorProperty *nvp, IGUIDevice *gd,
    IGUIGroup *gg, char *errmsg);
static int setGUISwitchVector (XMLEle *root, ISwitchVectorProperty *svp,
    IGUIDevice *gd, char errmsg[]);
static int newGUILightVector(ILightVectorProperty *nvp, IGUIDevice *gd,
    IGUIGroup *gg, char *errmsg);
static int setGUILightVector (XMLEle *root, ILightVectorProperty *lvp,
    IGUIDevice *gd, char errmsg[]);
static int newGUIBLOBVector(IBLOBVectorProperty *nbp, IGUIDevice *gd,
    IGUIGroup *gg, char *errmsg);
static int setGUIBLOBVector (XMLEle *root, IBLOBVectorProperty *bvp,
    IGUIDevice *gd, char errmsg[]);
static int handleOneBLOB (XMLEle *root, IBLOB *bp, char errmsg[]);
static void showMessage (XMLEle *root);
static char *setTStampNow (char tstamp[MAXINDITSTAMP]);
static void traceMessage (char *buf);
static void logMessage (char *timestr, char *device, char *str);
static IGUIDevice *findDev (char *device, int create, char errmsg[]);
static IGUIGroup *findGroup (IGUIDevice *gd, char *group, int create,
    char errmsg[]);
static INumber *findNumber (INumberVectorProperty *nvp, char *name);
static INumberVectorProperty *findNumberProperty (IGUIDevice *gd, char *name,
    char whynot[]);
static ITextVectorProperty *findTextProperty (IGUIDevice *gd, char *name, 
    char whynot[]);
static ISwitchVectorProperty *findSwitchProperty (IGUIDevice *gd, char *name,
    char whynot[]);
static ILightVectorProperty *findLightProperty (IGUIDevice *gd, char *name, 
    char whynot[]);
static IBLOBVectorProperty *findBLOBProperty (IGUIDevice *gd, char *name, 
    char whynot[]);
static IText *findText (ITextVectorProperty *tvp, char *name);
static ISwitch *findSwitch (ISwitchVectorProperty *svp, char *name);
static ILight *findLight (ILightVectorProperty *lvp, char *name);
static IBLOB *findBLOB (IBLOBVectorProperty *bvp, char *name);
static int crackCharAtt (XMLEle *root, char *vp, int maxl, char *name,
    char errmsg[]);
static int crackPerm (XMLEle *root, IPerm *p, char errmsg[]);
static int crackPState (XMLEle *root, IPState *s, char errmsg[]);
static int crackPString (char *str, IPState *s, char errmsg[]);
static int crackSState (XMLEle *root, ISState *s, char *errmsg);
static int crackRule (XMLEle *root, ISRule *r, char *errmsg);
static INumberVectorProperty *crackdefNumberVector (XMLEle *root,char errmsg[]);
static ITextVectorProperty *crackdefTextVector (XMLEle *root, char errmsg[]);
static ISwitchVectorProperty *crackdefSwitchVector (XMLEle *root,char errmsg[]);
static ILightVectorProperty *crackdefLightVector (XMLEle *root, char *errmsg);
static IBLOBVectorProperty *crackdefBLOBVector (XMLEle *root, char *errmsg);
static int crackDoubleAtt (XMLEle *root, double *dop, char *name, char *msg);
static int numberFormat (char *buf, const char *format, double value);
static void pushDisplay(void);
static void xmlv1(void);

static IGUIDevice *devices;		/* list of devices */
static int ndevices;			/* number devices[] */
static Pixmap opendev_pm;		/* icon to open a device section */
static Pixmap closedev_pm;		/* icon to close a device section */
static GC lt_gc;			/* GC for drawing lights */
static Pixel tshad_p, bshad_p;  	/* top bottom shadow colors */
static char *state_r[] = {		/* resource names of color states */
    "INDIIdleColor",
    "INDIOkColor",
    "INDIBusyColor",
    "INDIAlertColor"
};
#define	NSTATES	XtNumber(state_r)	/* number of color states */
static Pixel state_p[NSTATES];		/* pixels to draw each state color */

static Widget indi_w;			/* main shell */
static Widget connect_w;		/* TB for connect/disconnect */
static Widget devrc_w;			/* main RC for msg window and all dev */
static Widget msg_w;			/* main message ST */
static Widget mttb_w;			/* message trace TB */
static Widget mttf_w;			/* message trace file name TF */

static int svrsocket = -1;      /* socket to INDI device or server */
static FILE *swfp;              /* unbuffered server write FILE for fprintf */
static XtInputId scbid;         /* socket callback id */
static LilXML *lillp;           /* XML parser context */

static char indicategory[] = "INDI Panel";	/* Save category */

/* bring up an INDI control panel */
void
indi_manage()
{
	FILE *fp;

	/* create shell if first time */
	if (!indi_w)
	    indi_createShell ();

	/* just for testing, build GUI from test file if found, else run real */
	fp = fopen ("x.xml", "r");
	if (fp) {
	    LilXML *lp = newLilXML();
	    int c;

	    if (!swfp)
		swfp = stdout;
	    while ((c = fgetc(fp)) != EOF) {
		char msg[1024];
		XMLEle *root = readXMLEle (lp, c, msg);
		if ((!root && msg[0]) || (root && launchCmd(root, msg) < 0)) {
		    if (root) {
			prXMLEle (stdout, root, 0);
			xe_msg (0, "%s", msg);
		    }
		    xe_msg (0, "%s", msg);
		}
		if (root)
		    delXMLEle (root);
	    }

	    delLilXML (lp);
	    fclose (fp);
	}

	/* up we go */
	XtPopup (indi_w, XtGrabNone);
	set_something (indi_w, XmNiconic, (XtArgVal)False);
}

/* return 0 if currently connected to an INDI server, else -1
 */
int
indi_connected()
{
	return (svrsocket < 0 ? -1 : 0);
}

/* send a new set of values for the given Number device/property.
 * return 0 if ok else -1 if trouble with reason on why[]
 */
int
indi_setNProperty (char *dev, char *prop, char *n[], double v[], int nv,
char whynot[])
{
	INumberVectorProperty *nvp;
	IGUIDevice *gd;
	char buf[256];
	int i;

	gd = findDev (dev, 0, whynot);
	if (!gd)
	    return (-1);
	nvp = findNumberProperty (gd, prop, whynot);
	if (!nvp)
	    return (-1);
	if (nvp->p != IP_WO && nvp->p != IP_RW) {
	    sprintf (whynot, "Attempt to write to RO property %s.%s",
							nvp->device, nvp->name);
	    return (-1);
	}
	for (i = 0; i < nv; i++) {
	    INumber *np = findNumber (nvp, n[i]);
	    if (np) {
		np->value = v[i];
		numberFormat (buf, np->format, np->value);
		XmTextFieldSetString (np->aux1, buf);	/* echo */
	    } else {
		sprintf (whynot, "INDI %s.%s.%s: not found", dev, prop, n[i]);
		return (-1);
	    }
	}

	sendNumberVectorProperty (nvp);
	return (0);
}

/* send a new set of values for the given Text device/property.
 * return 0 if ok else -1 if trouble with reason on why[]
 */
int
indi_setTProperty (char *dev, char *prop, char *n[], char *v[], int nv,
char whynot[])
{
	ITextVectorProperty *tvp;
	IGUIDevice *gd;
	int i;

	gd = findDev (dev, 0, whynot);
	if (!gd)
	    return (-1);
	tvp = findTextProperty (gd, prop, whynot);
	if (!tvp)
	    return (-1);
	if (tvp->p != IP_WO && tvp->p != IP_RW) {
	    sprintf (whynot, "Attempt to write to RO property %s.%s",
							tvp->device, tvp->name);
	    return (-1);
	}
	for (i = 0; i < nv; i++) {
	    IText *tp = findText (tvp, n[i]);
	    if (tp) {
		tp->text = strcpy (XtRealloc (tp->text, strlen(v[i])+1), v[i]);
		XmTextFieldSetString (tp->aux1, tp->text);	/* echo */
	    } else {
		sprintf (whynot, "INDI %s.%s.%s: not found", dev, prop, n[i]);
		return (-1);
	    }
	}

	sendTextVectorProperty (tvp);
	return (0);
}

/* return the current Number vector for the given device/property, else NULL.
 * N.B. caller should not modify returned storage.
 */
INumberVectorProperty *
indi_getNProperty (char *dev, char *prop)
{
	IGUIDevice *gd = findDev (dev, 0, NULL);
	if (!gd)
	    return (NULL);
	return (findNumberProperty (gd, prop, NULL));
}

/* called when saveres is changing colors.
 * only thing we need to do is change the pixmaps in the dev TBs.
 */
void
indi_newres()
{
	/* plagerize from saveres's + and - */ 
	sr_getDirPM (&opendev_pm, &closedev_pm);

	/* install in each device TB */
}


/* create the main INDI shell, indi_w and main rc, devrc_w */
void
indi_createShell ()
{
	Widget w, f_w, mf_w, sw_w, sep_w, pw_w;
	Arg args[20];
	int n;

	/* create shell */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem INDI Control Panel"); n++;
	XtSetArg (args[n], XmNiconName, "INDI"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	indi_w = XtCreatePopupShell ("INDI", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (indi_w);
	set_something (indi_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (indi_w, "XEphem*INDI.x", indicategory, 0);
	sr_reg (indi_w, "XEphem*INDI.y", indicategory, 0);
	sr_reg (indi_w, "XEphem*INDI.width", indicategory, 0);
	sr_reg (indi_w, "XEphem*INDI.height", indicategory, 0);

	/* everything in a form */

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	f_w = XmCreateForm (indi_w, "INDIF", args, n);
	XtManageChild (f_w);

	/* controls across the bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	mf_w = XmCreateForm (f_w, "IMF", args, n);
	XtManageChild (mf_w);

	    n = 0;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 5); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 20); n++;
	    w = XmCreatePushButton (mf_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, closeCB, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 30); n++;
	    connect_w = XmCreateToggleButton (mf_w, "Connect", args, n);
	    wtip (connect_w,
		    "Connect and disconnect from the configured INDI server");
	    XtAddCallback (connect_w, XmNvalueChangedCallback, connectCB, NULL);
	    XtManageChild (connect_w);
	    sr_reg (connect_w, NULL, indicategory, 1);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, mf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (f_w, "S", args, n);
	XtManageChild (sep_w);

	/* top messages and bottom device area in a paned window */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, sep_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNsashHeight, 10); n++;
	XtSetArg (args[n], XmNspacing, 14); n++;
	pw_w = XmCreatePanedWindow (f_w, "IPW", args, n);
	XtManageChild (pw_w);

	/* message area in a form in top of pw */

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	mf_w = XmCreateForm (pw_w, "IMF", args, n);
	XtManageChild (mf_w);

	    /* erase */

	    n = 0;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 5); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 20); n++;
	    w = XmCreatePushButton (mf_w, "Erase", args, n);
	    wtip (w, "Clear message area");
	    XtAddCallback (w, XmNactivateCallback, eraseCB, NULL);
	    XtManageChild (w);

	    /* save TB and TF */

	    n = 0;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 30); n++;
	    mttb_w = XmCreateToggleButton (mf_w, "traceOn", args, n);
	    set_xmstring (mttb_w, XmNlabelString, "Trace to ");
	    XtAddCallback (mttb_w, XmNvalueChangedCallback, traceon_cb, NULL);
	    sr_reg (mttb_w, NULL, indicategory, 0);
	    wtip (mttb_w, "Whether to save messages to the file at right");
	    XtManageChild (mttb_w);

	    n = 0;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, mttb_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 95); n++;
	    mttf_w = XmCreateTextField (mf_w, "traceFile", args, n);
	    defaultTextFN (mttf_w, 0, "inditrace.txt", NULL);
	    sr_reg (mttf_w, NULL, indicategory, 0);
	    wtip (mttf_w, "File in which to append messages, if on");
	    XtManageChild (mttf_w);

	    /* message window */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNbottomWidget, mttf_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrows, MSGR); n++;
	    XtSetArg (args[n], XmNcolumns, MSGC); n++;
	    XtSetArg (args[n], XmNeditable, False); n++;
	    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	    XtSetArg (args[n], XmNblinkRate, 0); n++;
	    msg_w = XmCreateScrolledText (mf_w, "INDIMsg", args, n);
	    wtip (msg_w, "Messages from all INDI Devices");
	    XtManageChild (msg_w);

	/* main RC for all devices in a scrolled window */

	n = 0;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	sw_w = XmCreateScrolledWindow (pw_w, "ISW", args, n);
	XtManageChild (sw_w);

	    n = 0;
	    XtSetArg (args[n], XmNwidth, INDITWIDTH); n++;
	    XtSetArg (args[n], XmNresizeWidth, False); n++;
	    XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	    XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	    XtSetArg (args[n], XmNisAligned, False); n++;
	    XtSetArg (args[n], XmNadjustMargin, False); n++;
	    devrc_w = XmCreateRowColumn (sw_w, "IMRC", args, n);
	    XtManageChild (devrc_w);

	/* set up colors for lights and pixmaps for dev TB's */

	getLtColors ();
	indi_newres();

	/* start now if default Connect is On */
	if (XmToggleButtonGetState(connect_w))
	    if (indi_connect() < 0)
		XmToggleButtonSetState (connect_w, False, True);
}

/* using indi_w as a reference, set up various colors needed for our
 * home-made light.
 */
static void
getLtColors ()
{
	Pixel bg_p, fg_p, sel_p;
	Arg args[20];
	int n;

	if (lt_gc)
	    return;

	/* get motif shadow colors */
	n = 0;
	XtSetArg (args[n], XmNbackground, &bg_p); n++;
	XtGetValues (indi_w, args, n);
	XmGetColors (DefaultScreenOfDisplay(XtD), xe_cm, bg_p, &fg_p,
						    &tshad_p, &bshad_p, &sel_p);

	/* get INDI state colors */
	for (n = 0; n < NSTATES; n++)
	    get_color_resource (indi_w, state_r[n], &state_p[n]);

	/* make a GC to use for everything */
	lt_gc = XCreateGC (XtD, RootWindow (XtD, DefaultScreen(XtD)), 0L, NULL);
}

/* called when user wants to close the INDI window */
static void
closeCB (Widget w, XtPointer client, XtPointer call)
{
	XtPopdown (indi_w);
}

static void
connectCB (Widget w, XtPointer client, XtPointer call)
{
	if (XmToggleButtonGetState(w)) {
	    if (indi_connect() < 0)
		XmToggleButtonSetState (w, False, True);
	} else
	    indi_disconnect();
}

/* called to change whether to save the given BLOB given in call.
 * the actual saving is done whenever w is On by handleOneBLOB().
 */
static void
saveBLOBCB (Widget w, XtPointer client, XtPointer call)
{	
	IBLOBVectorProperty *bvp = (IBLOBVectorProperty *)client; 

	sendEnableBLOBs (bvp, XmToggleButtonGetState(w));
}

/* called to change whether to display the given BLOB given in call.
 * the actual displaying is done whenever w is On by handleOneBLOB().
 */
static void
displayBLOBCB (Widget w, XtPointer client, XtPointer call)
{	
	IBLOBVectorProperty *bvp = (IBLOBVectorProperty *)client; 

	sendEnableBLOBs (bvp, XmToggleButtonGetState(w));
}

static void
eraseCB (Widget w, XtPointer client, XtPointer call)
{
	XmTextSetString (msg_w, "");
}

/* draw the given light (aka drawing area) corresponding to state s */
static void
drawLt (Widget ltw, IPState s)
{
	Display *dsp = XtDisplay(ltw);
	Window win = XtWindow(ltw);
	Pixel p = state_p[s];
	Arg args[20];
	Dimension w, h;
	int x, y;
	int n;

	if (!win)
	    return;

	n = 0;
	XtSetArg(args[n], XmNwidth, (XtArgVal)&w); n++;
	XtSetArg(args[n], XmNheight, (XtArgVal)&h); n++;
	XtGetValues (ltw, args, n);

	x = (w - LIGHTW)/2;
	y = (h - LIGHTH)/2;

	XSetForeground (dsp, lt_gc, p);
	XFillRectangle (dsp, win, lt_gc, x+1, y+1, LIGHTW-2, LIGHTH-2);

	XSetForeground (dsp, lt_gc, BlackPixel(dsp, DefaultScreen(dsp)));
	XDrawRectangle (dsp, win, lt_gc, x, y, LIGHTW-1, LIGHTH-1);
}

/* process incoming INDI commands, disconnect if comm trouble */
static void
inputCB (XtPointer client, int *fdp, XtInputId *idp)
{
	static int trace = -1;
	char ibuf[32];	/* not so much user input lags */
	char msg[1024];
	int i, nr;

	/* set trace from env once */
	if (trace < 0) {
	    char *ie = getenv ("INDITRACE");
	    trace = (ie && atoi(ie) > 0);
	}

	/* read INDI command */
	nr = read (*fdp, ibuf, sizeof(ibuf));
	if (nr <= 0) {
	    if (nr < 0)
		xe_msg (0, "INDI: input error: %s", syserrstr());
	    else
		xe_msg (0, "INDI: agent closed connection");
	    indi_disconnect();
	    return;
	}

	/* process each char */
	for (i = 0; i < nr; i++) {
	    XMLEle *root = readXMLEle (lillp, (int)ibuf[i], msg);
	    if (root) {
		if (launchCmd (root, msg) < 0) {
		    prXMLEle (stdout, root, 0);
		    xe_msg (0, "%s", msg);
		} else if (trace > 0)
		    prXMLEle (stdout, root, 0);
		delXMLEle (root);
	    } else if (msg[0]) {
		xe_msg (0, "%s", msg);
	    }
	}
}

/* try to contact an INDI server
 */
static int
indi_connect()
{
	char *host, *port;
	char msg[1024];

	/* connect */
	sc_gethost (&host, &port);
	svrsocket = mkconnection (host, atoi(port), msg);
	if (svrsocket < 0) {
	    xe_msg (1, "INDI: Connection error to %s %s:\n%s", host, port, msg);
	    swfp = stdout;	/* tracing at least */
	    XtFree (host);
	    XtFree (port);
	    return (-1);
	}
	XtFree (host);
	XtFree (port);

	/* build a fresh parser context */
	if (lillp)
	    delLilXML (lillp);
	lillp = newLilXML();

	/* ready for server */
	scbid = XtAppAddInput (xe_app, svrsocket, (XtPointer)XtInputReadMask,
								inputCB, NULL);

	/* handy FILE for fprintf output messages */
	swfp = fdopen (svrsocket, "w");
	setbuf (swfp, NULL);

	/* get info */
	xmlv1();
	fprintf (swfp, "<getProperties version='%g'/>\n", INDIV);
	return (0);
}

/* disconnect from INDI server, fine if not already */
static void
indi_disconnect()
{
	int i, j;

	/* remove input */
	if (scbid) {
	    XtRemoveInput (scbid);
	    scbid = 0;
	}
	if (svrsocket >= 0) {
	    fclose (swfp);	/* closes svrsocket too */
	    swfp = NULL;
	    svrsocket = -1;
	}

	/* clean up lilxml */
	if (lillp) {
	    delLilXML (lillp);
	    lillp = NULL;
	}

	/* destroy gui and device properties info */
	if (devices) {
	    for (i = 0; i < ndevices; i++) {
		IGUIDevice *gd = &devices[i];
		XtDestroyWidget (gd->f_w);
		XtDestroyWidget (gd->grc_w);
		if (gd->ntvpp > 0) {
		    for (j = 0; j < gd->ntvpp; j++)
			freeTextVectorProperty (gd->tvpp[j]);
		    XtFree ((char *)gd->tvpp);
		}
		if (gd->nnvpp > 0) {
		    for (j = 0; j < gd->nnvpp; j++)
			freeNumberVectorProperty (gd->nvpp[j]);
		    XtFree ((char *)gd->nvpp);
		}
		if (gd->nsvpp > 0) {
		    for (j = 0; j < gd->nsvpp; j++)
			freeSwitchVectorProperty (gd->svpp[j]);
		    XtFree ((char *)gd->svpp);
		}
		if (gd->nlvpp > 0) {
		    for (j = 0; j < gd->nlvpp; j++)
			freeLightVectorProperty (gd->lvpp[j]);
		    XtFree ((char *)gd->lvpp);
		}
		if (gd->ngrp > 0) {
		    for (j = 0; j < gd->ngrp; j++)
			XtFree ((char *)gd->grp[j]);
		    XtFree ((char *)gd->grp);
		}
	    }

	    XtFree ((char *)devices);
	    devices = NULL;
	    ndevices = 0;
	}

	if (XmToggleButtonGetState(connect_w))
	    XmToggleButtonSetState (connect_w, False, False);
}

/* called to read the GUI and send the new values of an INumberVector.
 * client is pointer to INumberVector.
 * N.B. do not use w or call; we are used for TF and PB
 */
static void
newnum_cb (Widget w, XtPointer client, XtPointer call)
{
	INumberVectorProperty *nvp = (INumberVectorProperty *)client;
	int i;

	for (i = 0; i < nvp->nnp; i++) {
	    INumber *np = &nvp->np[i];
	    char *txt = XmTextFieldGetString ((Widget)np->aux1);
	    (void) f_scansexa (txt, &np->value);
	    XtFree (txt);
	}

	sendNumberVectorProperty (nvp);
}

/* called by the up arrow button for a writeable INumber.
 * client is pointer to INumber.
 */
static void
newnumup_cb (Widget w, XtPointer client, XtPointer call)
{
	INumber *np = (INumber *) client;
	char *txt = XmTextFieldGetString ((Widget)np->aux1);
	double v = strtod (txt, NULL), stepv = v + np->step;
	char buf[128];

	XtFree (txt);

	if (stepv < np->min || stepv > np->max) {
	    xe_msg (1, "%s: value must be %g .. %g", np->name, np->min,np->max);
	} else {
	    np->value = stepv;
	    numberFormat (buf, np->format, np->value);
	    XmTextFieldSetString (np->aux1, buf);
	}
}

/* called by the down arrow button for a writeable INumber.
 * client is pointer to INumber.
 */
static void
newnumdw_cb (Widget w, XtPointer client, XtPointer call)
{
	INumber *np = (INumber *) client;
	char *txt = XmTextFieldGetString ((Widget)np->aux1);
	double v = strtod (txt, NULL), stepv = v - np->step;
	char buf[128];

	XtFree (txt);

	if (stepv < np->min || stepv > np->max) {
	    xe_msg (1, "%s: value must be %g .. %g", np->name, np->min,np->max);
	} else {
	    np->value = stepv;
	    numberFormat (buf, np->format, np->value);
	    XmTextFieldSetString (np->aux1, buf);
	}
}

/* called to read the GUI and send the new values of an ITextVector.
 * client is pointer to ITextVectorProperty.
 * N.B. do not use w or call, we are called from PB and TF
 */
static void
newtext_cb (Widget w, XtPointer client, XtPointer call)
{
	ITextVectorProperty *tvp = (ITextVectorProperty *)client;
	int i;

	for (i = 0; i < tvp->ntp; i++) {
	    IText *tp = &tvp->tp[i];
	    char *txt = XmTextFieldGetString ((Widget)tp->aux1);
	    tp->text = strcpy (XtRealloc (tp->text, strlen(txt)+1), txt);
	    XtFree (txt);
	}

	sendTextVectorProperty (tvp);
}

/* called when the GUI TB for an ISwitch changes.
 * client is pointer to ISwitch.
 * send the new collection of its ISwitchVector.
 */
static void
newswitch_cb (Widget w, XtPointer client, XtPointer call)
{
	ISwitch *sp = (ISwitch *) client;
	int set = XmToggleButtonGetState(w);

	sp->s = set ? ISS_ON : ISS_OFF;

	/* don't send 1OFMANY going Off because some other will soon be On */
	if (!set && sp->svp->r == ISR_1OFMANY)
	    return;

	sendSwitchVectorProperty (sp->svp);
}

/* called when the GUI Set PB or filename TF for an IBLOBVectorProperty changes.
 * send the file named in the TF.
 * client is pointer to IBLOBVectoryProperty.
 * N.B. don't use client, this is used by PB and TF widgets
 */
static void
newBLOB_cb (Widget w, XtPointer client, XtPointer call)
{
	IBLOBVectorProperty *bvp = (IBLOBVectorProperty *) client;
	char tstamp[MAXINDITSTAMP];
	int started = 0;
	int i;

	watch_cursor(1);

	for (i = 0; i < bvp->nbp; i++) {
	    /* open the file */
	    IBLOB *bp = &bvp->bp[i];
	    char *fn = XmTextFieldGetString ((Widget)bp->aux2);
	    FILE *fp = fopend (fn, NULL, "r");
	    unsigned char *file, *file64;
	    int filen, file64n;
	    char *format;
	    char msg[1024];
	    int i;

	    /* skip if didn't open ok */
	    if (!fp) {
		/* already told user */
		XtFree (fn);
		break;
	    }

	    /* read file into memory */
	    if (fseek (fp, 0L, SEEK_END) < 0) {
		fclose (fp);
		XtFree (fn);
		xe_msg (1, "%s: can not seek", fn);
		break;
	    }
	    filen = ftell (fp);
	    file = malloc (filen);
	    if (!file) {
		fclose (fp);
		XtFree (fn);
		xe_msg (1, "Can not get %d bytes to read %s", filen, fn);
		break;
	    }
	    rewind (fp);
	    if (fread (file, 1, filen, fp) != filen) {
		fclose (fp);
		XtFree (fn);
		free (file);
		xe_msg (1, "%s: can not read", filen);
		break;
	    }
	    fclose (fp);

	    /* convery to base64 */
	    file64n = 4*filen/3+4;
	    file64 = malloc (file64n);
	    if (!file64) {
		XtFree (fn);
		free (file);
		xe_msg (1, "Can not get %d bytes for %s as base64", file64n,
								    fn);
		break;
	    }
	    file64n = to64frombits (file64, file, filen);
	    free (file);

	    /* format is last suffix */
	    format = strrchr (fn, '.');
	    if (!format)
		format = "";

	    /* send */
	    sprintf (msg, "Sending %s", fn);
	    logMessage (NULL, bvp->device, msg);
	    if (!started) {
		xmlv1();
		fprintf (swfp,
		    "<newBLOBVector device='%s' name='%s' timestamp='%s'>\n",
				bvp->device, bvp->name, setTStampNow (tstamp));
		started = 1;
	    }
	    fprintf (swfp, "  <oneBLOB name='%s' size='%d' format='%s'>\n",
						bp->name, filen, format);
	    for (i = 0; i < file64n; i += 72)
		fprintf (swfp, "    %.72s\n", file64+i);
	    fprintf (swfp, "  </oneBLOB>\n");

	    free (file64);
	    XtFree (fn);
	}

	if (started) {
	    fprintf (swfp, "</newBLOBVector>\n");
	    bvp->s = i < bvp->nbp ? IPS_ALERT : IPS_BUSY;
	    drawDevLt (findDev (bvp->device, 0, NULL));
	}

	watch_cursor(0);
}

/* send the given NumberVector to the connected INDI device */
static void
sendNumberVectorProperty (INumberVectorProperty *nvp)
{
	char tstamp[MAXINDITSTAMP];
	int i;

	xmlv1();
	fprintf(swfp,"<newNumberVector device='%s' name='%s' timestamp='%s'>\n",
				nvp->device, nvp->name, setTStampNow (tstamp));
	for (i = 0; i < nvp->nnp; i++) {
	    INumber *np = &nvp->np[i];
	    fprintf (swfp, "  <oneNumber name='%s'>%.20g</oneNumber>\n",
							np->name, np->value);
	}

	fprintf (swfp, "</newNumberVector>\n");

	nvp->s = IPS_BUSY;
	drawDevLt (findDev (nvp->device, 0, NULL));
}

/* send the given TextVector to the connected INDI device */
static void
sendTextVectorProperty (ITextVectorProperty *tvp)
{
	char tstamp[MAXINDITSTAMP];
	int i;

	xmlv1();
	fprintf (swfp, "<newTextVector device='%s' name='%s' timestamp='%s'>\n",
				tvp->device, tvp->name, setTStampNow (tstamp));
	for (i = 0; i < tvp->ntp; i++) {
	    fprintf (swfp, "  <oneText name='%s'>%s</oneText>\n",
					    tvp->tp[i].name, tvp->tp[i].text);
	}

	fprintf (swfp, "</newTextVector>\n");

	tvp->s = IPS_BUSY;
	drawDevLt (findDev (tvp->device, 0, NULL));
}

/* send the given ISwitchVectorProperty to the INDI device and set our state to BUSY */
static void
sendSwitchVectorProperty (ISwitchVectorProperty *svp)
{
	char tstamp[MAXINDITSTAMP];
	int i;

	xmlv1();
	fprintf(swfp,"<newSwitchVector device='%s' name='%s' timestamp='%s'>\n",
				svp->device, svp->name, setTStampNow (tstamp));

	for (i = 0; i < svp->nsp; i++) {
	    fprintf (swfp, "  <oneSwitch name='%s'>%s</oneSwitch>\n", svp->sp[i].name,
						svp->sp[i].s == ISS_ON ? "On" : "Off");
	}

	fprintf (swfp, "</newSwitchVector>\n");

	svp->s = IPS_BUSY;
	drawDevLt (findDev (svp->device, 0, NULL));
}

/* send a message to the server to enable or disable intermixed BLOB messages */
static void
sendEnableBLOBs (IBLOBVectorProperty *bvp, int on)
{
	xmlv1();
	fprintf(swfp, "<enableBLOB device='%s' name='%s'>%s</enableBLOB>\n",
		bvp->device, bvp->name, on ? "Also" : "Never");
}

/* implement the given INDI command received from the INDI server.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
launchCmd (XMLEle *root, char *errmsg)
{
	static struct {
	    char *cmdname;
	    int (*fp)(XMLEle *root, char errmsg[]);
	} commands[] = {
	    /* N.B. must be sorted by cmdname */
	    {"defBLOBVector",	defBLOBCmd},
	    {"defLightVector",	defLightCmd},
	    {"defNumberVector",	defNumberCmd},
	    {"defSwitchVector",	defSwitchCmd},
	    {"defTextVector",	defTextCmd},
	    {"message",		doMessage},
	    {"newNumberVector",	setNumberCmd},
	    {"newSwitchVector",	setSwitchCmd},
	    {"newTextVector",	setTextCmd},
	    {"setBLOBVector",	setBLOBCmd},
	    {"setLightVector",	setLightCmd},
	    {"setNumberVector",	setNumberCmd},
	    {"setSwitchVector",	setSwitchCmd},
	    {"setTextVector",	setTextCmd},
	};

	char *tag = tagXMLEle(root);
	int u = XtNumber(commands)-1;
	int l = 0;
	int diff = -1;
	int m = -1;

	/* binary search */
	while (l <= u) {
	    m = (l+u)/2;
	    diff = strcmp (tag, commands[m].cmdname);
	    if (diff == 0)
		break;
	    if (diff < 0)
		u = m-1;
	    else
		l = m+1;
	}

	if (diff == 0)
	    return ((*commands[m].fp) (root, errmsg));

	sprintf (errmsg, "INDI: unrecognized tag %s", tag);
	return (-1);
}

/* set a new value for a Number property */
static int
setNumberCmd (XMLEle *root, char errmsg[])
{
	INumberVectorProperty *nvp;
	IGUIDevice *gd;
	char *name;

	gd = findDev (findXMLAttValu (root, "device"), 0, errmsg);
	if (!gd)
	    return (-1);
	name = findXMLAttValu (root, "name");
	nvp = findNumberProperty (gd, name, errmsg);
	if (!nvp)
	    return (-1);
	if(crackCharAtt(root,nvp->timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (nvp->timestamp);
	return (setGUINumberVector(root, nvp, gd, errmsg));
}

/* set a new value for a Text property */
static int
setTextCmd (XMLEle *root, char errmsg[])
{
	ITextVectorProperty *tvp;
	IGUIDevice *gd;
	char *name;

	gd = findDev (findXMLAttValu (root, "device"), 0, errmsg);
	if (!gd)
	    return (-1);
	name = findXMLAttValu (root, "name");
	tvp = findTextProperty (gd, name, errmsg);
	if (!tvp)
	    return (-1);
	if(crackCharAtt(root,tvp->timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (tvp->timestamp);
	return (setGUITextVector(root, tvp, gd, errmsg));
}

/* set a new value for a Switch property */
static int
setSwitchCmd (XMLEle *root, char errmsg[])
{
	ISwitchVectorProperty *svp;
	IGUIDevice *gd;
	char *name;

	gd = findDev (findXMLAttValu (root, "device"), 0, errmsg);
	if (!gd)
	    return (-1);
	name = findXMLAttValu (root, "name");
	svp = findSwitchProperty (gd, name, errmsg);
	if (!svp)
	    return (-1);
	if(crackCharAtt(root,svp->timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (svp->timestamp);
	return (setGUISwitchVector(root, svp, gd, errmsg));
}

/* set a new value for a Light property */
static int
setLightCmd (XMLEle *root, char errmsg[])
{
	ILightVectorProperty *lvp;
	IGUIDevice *gd;
	char *name;

	gd = findDev (findXMLAttValu (root, "device"), 0, errmsg);
	if (!gd)
	    return (-1);
	name = findXMLAttValu (root, "name");
	lvp = findLightProperty (gd, name, errmsg);
	if (!lvp)
	    return (-1);
	if(crackCharAtt(root,lvp->timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (lvp->timestamp);
	return (setGUILightVector(root, lvp, gd, errmsg));
}

/* set a new value for a BLOB property */
static int
setBLOBCmd (XMLEle *root, char errmsg[])
{
	IBLOBVectorProperty *bvp;
	IGUIDevice *gd;
	char *name;

	gd = findDev (findXMLAttValu (root, "device"), 0, errmsg);
	if (!gd)
	    return (-1);
	name = findXMLAttValu (root, "name");
	bvp = findBLOBProperty (gd, name, errmsg);
	if (!bvp)
	    return (-1);
	if(crackCharAtt(root,bvp->timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (bvp->timestamp);
	return (setGUIBLOBVector(root, bvp, gd, errmsg));
}

/* display a message from the server */
static int
doMessage (XMLEle *root, char errmsg[])
{
	showMessage (root);
	return (0);
}

/* define a new Text property, ignore if already known */
static int
defTextCmd (XMLEle *root, char errmsg[])
{
	ITextVectorProperty *tvp;
	IGUIDevice *gd;
	IGUIGroup *gg;
	char buf[256], *bp = buf;
	int i;

	/* crack new text vector */
	if ((tvp = crackdefTextVector (root, errmsg)) == NULL)
	    return (-1);

	/* ignore if already known */
	gd = findDev (tvp->device, 0, errmsg);
	if (gd && findTextProperty (gd, tvp->name, errmsg)) {
	    freeTextVectorProperty (tvp);
	    return (0);
	}

	/* record */
	bp += sprintf (bp, "New text vector %s[", tvp->name);
	for (i = 0; i < tvp->ntp; i++) {
	    bp += sprintf (bp, "%s", tvp->tp[i].name);
	    if (i < tvp->ntp-1)
		bp += sprintf (bp, ",");
	    if (bp > &buf[sizeof(buf)] - MAXINDINAME) {
		bp += sprintf (bp, "...");
		break;
	    }
	}
	bp += sprintf (bp, "]");
	logMessage (tvp->timestamp, tvp->device, buf);

	/* add to existing or create new device, group and property */
	if (!gd)
	    gd = findDev (tvp->device, 1, errmsg);  /* can't fail w/create */
	gg = findGroup (gd, tvp->group, 1, errmsg); /* can't fail w/create */
	return (newGUITextVector(tvp, gd, gg, errmsg));
}

/* define a new Number property, ignore if already known */
static int
defNumberCmd (XMLEle *root, char errmsg[])
{
	INumberVectorProperty *nvp;
	IGUIDevice *gd;
	IGUIGroup *gg;
	char buf[256], *bp = buf;
	int i;

	/* crack new number vector */
	if ((nvp = crackdefNumberVector (root, errmsg)) == NULL)
	    return (-1);

	/* ignore if already known */
	gd = findDev (nvp->device, 0, errmsg);
	if (gd && findNumberProperty (gd, nvp->name, errmsg)) {
	    freeNumberVectorProperty (nvp);
	    return (0);
	}

	/* record */
	bp += sprintf (bp, "New number vector %s[", nvp->name);
	for (i = 0; i < nvp->nnp; i++) {
	    bp += sprintf (bp, "%s", nvp->np[i].name);
	    if (i < nvp->nnp-1)
		bp += sprintf (bp, ",");
	    if (bp > &buf[sizeof(buf)] - MAXINDINAME) {
		bp += sprintf (bp, "...");
		break;
	    }
	}
	bp += sprintf (bp, "]");
	logMessage (nvp->timestamp, nvp->device, buf);

	/* add to existing or create new device, group and property */
	if (!gd)
	    gd = findDev (nvp->device, 1, errmsg);  /* can't fail w/create */
	gg = findGroup (gd, nvp->group, 1, errmsg); /* can't fail w/create */
	return (newGUINumberVector(nvp, gd, gg, errmsg));
}

/* define a new Switch property, ignore if already known */
static int
defSwitchCmd (XMLEle *root, char errmsg[])
{
	ISwitchVectorProperty *svp;
	IGUIDevice *gd;
	IGUIGroup *gg;
	char buf[256], *bp = buf;
	int i;

	/* crack new Switch vector */
	if ((svp = crackdefSwitchVector (root, errmsg)) == NULL)
	    return (-1);

	/* ignore if already known */
	gd = findDev (svp->device, 0, errmsg);
	if (gd && findSwitchProperty (gd, svp->name, errmsg)) {
	    freeSwitchVectorProperty (svp);
	    return (0);
	}

	/* record */
	bp += sprintf (bp, "New switch vector %s[", svp->name);
	for (i = 0; i < svp->nsp; i++) {
	    bp += sprintf (bp, "%s", svp->sp[i].name);
	    if (i < svp->nsp-1)
		bp += sprintf (bp, ",");
	    if (bp > &buf[sizeof(buf)] - MAXINDINAME) {
		bp += sprintf (bp, "...");
		break;
	    }
	}
	bp += sprintf (bp, "]");
	logMessage (svp->timestamp, svp->device, buf);

	/* add to existing or create new device, group and property */
	if (!gd)
	    gd = findDev (svp->device, 1, errmsg);  /* can't fail w/create */
	gg = findGroup (gd, svp->group, 1, errmsg); /* can't fail w/create */
	return (newGUISwitchVector(svp, gd, gg, errmsg));
}

/* define a new Lights property, ignore if already known */
static int
defLightCmd (XMLEle *root, char errmsg[])
{
	ILightVectorProperty *lvp;
	IGUIDevice *gd;
	IGUIGroup *gg;
	char buf[256], *bp = buf;
	int i;

	/* crack new Light vector */
	if ((lvp = crackdefLightVector (root, errmsg)) == NULL)
	    return (-1);

	/* ignore if already known */
	gd = findDev (lvp->device, 0, errmsg);
	if (gd && findLightProperty (gd, lvp->name, errmsg)) {
	    freeLightVectorProperty (lvp);
	    return (0);
	}

	/* record */
	bp += sprintf (bp, "New light vector %s[", lvp->name);
	for (i = 0; i < lvp->nlp; i++) {
	    bp += sprintf (bp, "%s", lvp->lp[i].name);
	    if (i < lvp->nlp-1)
		bp += sprintf (bp, ",");
	    if (bp > &buf[sizeof(buf)] - MAXINDINAME) {
		bp += sprintf (bp, "...");
		break;
	    }
	}
	bp += sprintf (bp, "]");
	logMessage (lvp->timestamp, lvp->device, buf);

	/* add to existing or create new device, group and property */
	if (!gd)
	    gd = findDev (lvp->device, 1, errmsg);  /* can't fail w/create */
	gg = findGroup (gd, lvp->group, 1, errmsg); /* can't fail w/create */
	return (newGUILightVector(lvp, gd, gg, errmsg));
}

/* define a new BLOB property, ignore if already known */
static int
defBLOBCmd (XMLEle *root, char errmsg[])
{
	IBLOBVectorProperty *bvp;
	IGUIDevice *gd;
	IGUIGroup *gg;
	char buf[256], *bp = buf;
	int i;

	/* crack new BLOB vector */
	if ((bvp = crackdefBLOBVector (root, errmsg)) == NULL)
	    return (-1);

	/* ignore if already known */
	gd = findDev (bvp->device, 0, errmsg);
	if (gd && findBLOBProperty (gd, bvp->name, errmsg)) {
	    freeBLOBVectorProperty (bvp);
	    return (0);
	}

	/* record */
	bp += sprintf (bp, "New BLOB vector %s[", bvp->name);
	for (i = 0; i < bvp->nbp; i++) {
	    bp += sprintf (bp, "%s", bvp->bp[i].name);
	    if (i < bvp->nbp-1)
		bp += sprintf (bp, ",");
	    if (bp > &buf[sizeof(buf)] - MAXINDINAME) {
		bp += sprintf (bp, "...");
		break;
	    }
	}
	bp += sprintf (bp, "]");
	logMessage (bvp->timestamp, bvp->device, buf);

	/* add to existing or create new device, group and property */
	if (!gd)
	    gd = findDev (bvp->device, 1, errmsg);  /* can't fail w/create */
	gg = findGroup (gd, bvp->group, 1, errmsg); /* can't fail w/create */
	return (newGUIBLOBVector(bvp, gd, gg, errmsg));
}

/* free the given TVP and any supporting memory */
static void
freeTextVectorProperty (ITextVectorProperty *tvp)
{
	if (tvp->tp) {
	    int i;
	    for (i = 0; i < tvp->ntp; i++)
		if (tvp->tp[i].text)
		    XtFree (tvp->tp[i].text);
	    XtFree ((char *) tvp->tp);
	}
	XtFree ((char *) tvp);
}

/* free the given NVP and any supporting memory */
static void
freeNumberVectorProperty (INumberVectorProperty *ivp)
{
	if (ivp->np)
	    XtFree ((char *) ivp->np);
	XtFree ((char *) ivp);
}

/* free the given SVP and any supporting memory */
static void
freeSwitchVectorProperty (ISwitchVectorProperty *svp)
{
	if (svp->sp)
	    XtFree ((char *) svp->sp);
	XtFree ((char *) svp);
}

/* free the given LVP and any supporting memory */
static void
freeLightVectorProperty (ILightVectorProperty *lvp)
{
	if (lvp->lp)
	    XtFree ((char *) lvp->lp);
	XtFree ((char *) lvp);
}

/* free the given BVP and any supporting memory */
static void
freeBLOBVectorProperty (IBLOBVectorProperty *bvp)
{
	if (bvp->bp) {
	    int i;
	    for (i = 0; i < bvp->nbp; i++)
		if (bvp->bp[i].blob)
		    XtFree (bvp->bp[i].blob);
	    XtFree ((char *) bvp->bp);
	}
	XtFree ((char *) bvp);
}

/* make a da for use as state indicator.
 * when exposed, it draws an indicator using its foreground color
 */
static Widget
createLt (Widget p_w, char *name, Arg args[], int n)
{
	Widget da_w;

	XtSetArg (args[n], XmNwidth, LIGHTW); n++;
	XtSetArg (args[n], XmNheight, LIGHTH); n++;
	XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
	da_w = XmCreateDrawingArea (p_w, name, args, n);
	return (da_w);
}

/* called to expose or hide all the properties for a device.
 * client is index into devices[]
 */
static void
devTogCB (Widget w, XtPointer client, XtPointer call)
{
	IGUIDevice *gd = &devices[(long int)client];

	if (XmToggleButtonGetState(w)) {
	    XtManageChild (gd->grc_w);
	    drawDevLt (gd);
	} else
	    XtUnmanageChild (gd->grc_w);
}

/* called to expose or hide the properties in a group.
 * client is IGUIGroup to un/manage.
 */
static void
groupTogCB (Widget w, XtPointer client, XtPointer call)
{
	IGUIGroup *gg = (IGUIGroup *) client;

	if (XmToggleButtonGetState(w))
	    XtManageChild (gg->prc_w);
	else
	    XtUnmanageChild (gg->prc_w);
}

/* called when exposing a device status light. 
 * client is index into devices[]
 */
static void
devLtCB (Widget w, XtPointer client, XtPointer call)
{
	IGUIDevice *gd = &devices[(long int)client];

	drawDevLt (gd);
}

/* create a new GUIDevice */
static void
newGUIDevice (IGUIDevice *gd)
{
	Widget w;
	Arg args[20];
	int n;

	n = 0;
	gd->f_w = XmCreateForm (devrc_w, "Device", args, n);
	XtManageChild (gd->f_w);

	/* worst-property light */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	gd->lda_w = createLt (gd->f_w, "DL", args, n);
	XtAddCallback (gd->lda_w, XmNexposeCallback, devLtCB,
						(XtPointer)(gd-devices));
	XtManageChild (gd->lda_w);

	/* open/close TB */

	n = 0; 
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, INDENT); n++;
	XtSetArg (args[n], XmNlabelType, XmPIXMAP); n++;
	XtSetArg (args[n], XmNlabelPixmap, opendev_pm); n++;
	XtSetArg (args[n], XmNselectPixmap, closedev_pm); n++;
	XtSetArg (args[n], XmNindicatorOn, False); n++;
	XtSetArg (args[n], XmNshadowThickness, 0); n++;
	w = XmCreateToggleButton (gd->f_w, "DTB", args, n);
	XtAddCallback (w, XmNvalueChangedCallback, devTogCB,
						    (XtPointer)(gd-devices));
	XtManageChild (w);

	/* device name */

	n = 0; 
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, w); n++;
	w = XmCreateLabel (gd->f_w, "DL", args, n);
	set_xmstring (w, XmNlabelString, gd->device);
	XtManageChild (w);

	/* RC for groups -- leave unmanaged initially */

	n = 0;
	gd->grc_w = XmCreateRowColumn (devrc_w, "DGRC", args, n);
}

/* create a new GUIGroup within the GUIDevice.
 * if the group has a name, give it a control to show/hide and set it initially
 * hidden, otherwise it is always visible.
 */
static void
newGUIGroup (IGUIDevice *gd, IGUIGroup *gg)
{
	int hideable = gg->group[0] != '\0';
	Widget f_w = (Widget)0;		/* avoid compiler warning */
	Widget w;
	Arg args[20];
	int n;

	if (hideable) {
	    n = 0;
	    f_w = XmCreateForm (gd->grc_w, "Group", args, n);
	    XtManageChild (f_w);
	}

	n = 0;
	gg->prc_w = XmCreateRowColumn (gd->grc_w, "PRC", args, n);
	if (!hideable)
	    XtManageChild (gg->prc_w);

	if (hideable) {
	    /* open/close TB */

	    n = 0; 
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, INDENT); n++;
	    XtSetArg (args[n], XmNlabelType, XmPIXMAP); n++;
	    XtSetArg (args[n], XmNlabelPixmap, opendev_pm); n++;
	    XtSetArg (args[n], XmNselectPixmap, closedev_pm); n++;
	    XtSetArg (args[n], XmNindicatorOn, False); n++;
	    XtSetArg (args[n], XmNshadowThickness, 0); n++;
	    w = XmCreateToggleButton (f_w, "GTB", args, n);
	    XtAddCallback (w, XmNvalueChangedCallback, groupTogCB, gg);
	    XtManageChild (w);

	    /* device name */

	    n = 0; 
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    w = XmCreateLabel (f_w, "GL", args, n);
	    set_xmstring (w,XmNlabelString, gg->group);
	    XtManageChild (w);
	}
}

/* add the given number onto the given device list in the given group and
 * build its GUI.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
newGUINumberVector(INumberVectorProperty *nvp, IGUIDevice *gd, IGUIGroup *gg,
char *errmsg)
{
	Widget nrc_w, set_w;
	Widget f_w, hrc_w, l_w;
	Arg args[20];
	int i;
	int n;

	/* add nvp to device list */

	gd->nvpp = (INumberVectorProperty **) XtRealloc ((char *)(gd->nvpp),
				(gd->nnvpp+1) * sizeof(INumberVectorProperty*));
	gd->nvpp[gd->nnvpp++] = nvp;

	/* master rc, works faster to delay managing until finished */

	n = 0;
	XtSetArg (args[n], XmNspacing, 0); n++;
	nrc_w = XmCreateRowColumn (gg->prc_w, "NRC", args, n);

	/* property rc */

	n = 0;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNmarginWidth, INDENT); n++;
	XtSetArg (args[n], XmNspacing, 3); n++;
	hrc_w = XmCreateRowColumn (nrc_w, "NPF", args, n);
	XtManageChild (hrc_w);

	    /* property's status light */

	    n = 0;
	    nvp->aux = (void *) createLt (hrc_w, "NLT", args, n);
	    XtAddCallback ((Widget)nvp->aux, XmNexposeCallback, devLtCB,
						    (XtPointer)(gd-devices));
	    XtManageChild ((Widget)nvp->aux);

	    /* set PB , unless RO */

	    if (nvp->p != IP_RO) {
		n = 0;
		set_w = XmCreatePushButton (hrc_w, "Set", args, n);
		XtAddCallback (set_w, XmNactivateCallback, newnum_cb, nvp);
		XtManageChild (set_w);
	    }

	    /* label */

	    n = 0;
	    l_w = XmCreateLabel (hrc_w, "NL", args, n);
	    set_xmstring (l_w, XmNlabelString, (char *) nvp->label);
	    XtManageChild (l_w);

	/* one row for each vector element */

	for (i = 0; i < nvp->nnp; i++) {
	    INumber *np = &nvp->np[i];
	    char buf[128];

	    numberFormat (buf, np->format, np->value);

	    n = 0;
	    f_w = XmCreateForm (nrc_w, "NF", args, n);
	    XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 2*INDENT); n++;
	    l_w = XmCreateLabel (f_w, "NL", args, n);
	    set_xmstring (l_w, XmNlabelString, (char *) np->label);
	    XtManageChild (l_w);

	    switch (nvp->p) {
	    case IP_RO:

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNrightPosition, 100); n++;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		np->aux0 = (void *) XmCreateLabel (f_w, "NRO", args, n);
		set_xmstring ((Widget) np->aux0, XmNlabelString, buf);
		XtManageChild ((Widget) np->aux0);
		break;

	    case IP_RW:

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNleftPosition, WFLPOS); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNrightPosition, WFRPOS-2); n++;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		XtSetArg (args[n], XmNrecomputeSize, False); n++;
		np->aux0 = (void *) XmCreateLabel (f_w, "NRO", args, n);
		set_xmstring ((Widget) np->aux0, XmNlabelString, buf);
		XtManageChild ((Widget) np->aux0);

		/* FALLTHRU */

	    case IP_WO:

		newGUIWrNumber (np, f_w, WFRPOS);
		XmTextFieldSetString ((Widget)np->aux1, buf);
		break;
	    }
	}

	drawDevLt (gd);

	XtManageChild (nrc_w);
	pushDisplay();

	return (0);
}

/* build a gui for the writable portion of an INumber */
static void
newGUIWrNumber (INumber *np, Widget p_w, int lpos)
{
	Arg args[20];
	int n;

	if (np->step) {
	    Widget up_w, dw_w;

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 100); n++;
	    XtSetArg (args[n], XmNarrowDirection, XmARROW_UP); n++;
	    up_w = XmCreateArrowButton (p_w, "IDW", args, n);
	    XtAddCallback (up_w, XmNactivateCallback, newnumup_cb, np);
	    XtManageChild (up_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, up_w); n++;
	    XtSetArg (args[n], XmNarrowDirection, XmARROW_DOWN); n++;
	    dw_w = XmCreateArrowButton (p_w, "IUP", args, n);
	    XtAddCallback (dw_w, XmNactivateCallback, newnumdw_cb, np);
	    XtManageChild (dw_w);

	    n = 0;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, dw_w); n++;

	} else {

	    n = 0;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 100); n++;
	}

	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, lpos); n++;
	XtSetArg (args[n], XmNcolumns, 10); n++;
	np->aux1 = (void *) XmCreateTextField (p_w, "NWO", args, n);
	XtAddCallback ((Widget) np->aux1,XmNactivateCallback,newnum_cb,np->nvp);
	XtManageChild ((Widget) np->aux1);
}

/* capture and display state, msg and each value(s) for a set/newNumberVector */
static int
setGUINumberVector (XMLEle *root, INumberVectorProperty *nvp, IGUIDevice *gd,
char errmsg[])
{
	int new = !strcmp (tagXMLEle(root), "newNumberVector");
	XMLEle *ep;
	char buf[64];

	for (ep = nextXMLEle(root,1); ep; ep = nextXMLEle(root,0))  {
	    if (strcmp(tagXMLEle(ep), "oneNumber") == 0) {
		char *name = findXMLAttValu (ep, "name");
		INumber *np = findNumber (nvp, name);
		if (np) {
		    np->value = strtod (pcdataXMLEle(ep), NULL);
		    numberFormat (buf, np->format, np->value);
		    if (np->nvp->p == IP_WO || (np->nvp->p == IP_RW && new))
			XmTextFieldSetString ((Widget)np->aux1, buf);
		    else
			set_xmstring ((Widget)np->aux0, XmNlabelString, buf);
		} else {
		    sprintf (errmsg, "INDI: set %s.%s.%s not found", gd->device,
							    nvp->name, name);
		    return (-1);
		}
	    }
	}

	showMessage (root);
	if (crackPState (root, &nvp->s, errmsg) == 0)
	    drawDevLt (gd);
	return (0);
}

/* add the given text onto the given device list in the given group and
 * build its GUI.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
newGUITextVector(ITextVectorProperty *tvp, IGUIDevice *gd, IGUIGroup *gg,
char *errmsg)
{
	Widget set_w, trc_w;
	Widget hrc_w, f_w, l_w;
	Arg args[20];
	int i;
	int n;

	/* add tvp to device list */

	gd->tvpp = (ITextVectorProperty **) XtRealloc ((char *)(gd->tvpp),
				(gd->ntvpp+1) * sizeof(ITextVectorProperty*));
	gd->tvpp[gd->ntvpp++] = tvp;

	/* master rc, works faster to delay managing until finished */

	n = 0;
	XtSetArg (args[n], XmNspacing, 0); n++;
	trc_w = XmCreateRowColumn (gg->prc_w, "TRC", args, n);

	/* property rc */

	n = 0;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNmarginWidth, INDENT); n++;
	XtSetArg (args[n], XmNspacing, 3); n++;
	hrc_w = XmCreateRowColumn (trc_w, "TPF", args, n);
	XtManageChild (hrc_w);

	    /* property's status light */

	    n = 0;
	    tvp->aux = (void *) createLt (hrc_w, "TLT", args, n);
	    XtAddCallback ((Widget)tvp->aux, XmNexposeCallback, devLtCB,
						    (XtPointer)(gd-devices));
	    XtManageChild ((Widget)tvp->aux);

	    /* set PB, unless RO */

	    if (tvp->p != IP_RO) {
		n = 0;
		set_w = XmCreatePushButton (hrc_w, "Set", args, n);
		XtAddCallback (set_w, XmNactivateCallback, newtext_cb, tvp);
		XtManageChild (set_w);
	    }

	    /* label */

	    n = 0;
	    l_w = XmCreateLabel (hrc_w, "TL", args, n);
	    set_xmstring (l_w, XmNlabelString, (char *) tvp->label);
	    XtManageChild (l_w);

	/* one row for each vector element */

	for (i = 0; i < tvp->ntp; i++) {
	    IText *tp = &tvp->tp[i];

	    n = 0;
	    f_w = XmCreateForm (trc_w, "TF", args, n);
	    XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 2*INDENT); n++;
	    l_w = XmCreateLabel (f_w, "TL", args, n);
	    set_xmstring (l_w, XmNlabelString, (char *) tp->label);
	    XtManageChild (l_w);

	    switch (tvp->p) {

	    case IP_RO:

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNrightPosition, 100); n++;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		tp->aux0 = (void *) XmCreateLabel (f_w, "TRO", args, n);
		set_xmstring ((Widget) tp->aux0, XmNlabelString, tp->text);
		XtManageChild ((Widget) tp->aux0);
		break;

	    case IP_RW:

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNleftPosition, WFLPOS); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNrightPosition, WFRPOS-2); n++;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
		XtSetArg (args[n], XmNrecomputeSize, False); n++;
		tp->aux0 = (void *) XmCreateLabel (f_w, "TRO", args, n);
		set_xmstring ((Widget) tp->aux0, XmNlabelString, tp->text);
		XtManageChild ((Widget) tp->aux0);

		/* FALLTHRU */

	    case IP_WO:

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNleftPosition, WFRPOS); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNrightPosition, 100); n++;
		XtSetArg (args[n], XmNcolumns, 10); n++;
		XtSetArg (args[n], XmNvalue, tp->text); n++;
		tp->aux1 = (void *) XmCreateTextField (f_w, "NWO", args, n);
		XtAddCallback ((Widget) tp->aux1, XmNactivateCallback,
							    newtext_cb, tvp);
		XtManageChild ((Widget) tp->aux1);
		break;
	    }
	}

	drawDevLt (gd);

	XtManageChild (trc_w);
	pushDisplay();

	return (0);
}

/* capture and display state, msg and each value(s) for a set/newTextVector */
static int
setGUITextVector (XMLEle *root, ITextVectorProperty *tvp, IGUIDevice *gd,
char errmsg[])
{
	int new = !strcmp (tagXMLEle(root), "newTextVector");
	XMLEle *ep;

	for (ep = nextXMLEle(root,1); ep; ep = nextXMLEle(root,0))  {
	    if (strcmp(tagXMLEle(ep), "oneText") == 0) {
		char *name = findXMLAttValu (ep, "name");
		IText *tp = findText (tvp, name);
		if (tp) {
		    char *t = pcdataXMLEle (ep);
		    tp->text = strcpy (XtRealloc(tp->text,strlen(t)+1), t);
		    if (tp->tvp->p == IP_WO || (tp->tvp->p == IP_RW && new))
			XmTextFieldSetString ((Widget)tp->aux1, t);
		    else
			set_xmstring ((Widget)tp->aux0, XmNlabelString, t);
		} else {
		    sprintf (errmsg, "INDI: set %s.%s.%s not found", gd->device,
							    tvp->name, name);
		    return (-1);
		}
	    }
	}

	showMessage (root);
	if (crackPState (root, &tvp->s, errmsg) == 0)
	    drawDevLt (gd);
	return (0);
}

/* add the given switch onto the given device list in the given group and
 * build its GUI.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
newGUISwitchVector(ISwitchVectorProperty *svp, IGUIDevice *gd, IGUIGroup *gg,
char *errmsg)
{
	Widget w, l_w, hrc_w;
	Widget src_w;
	Arg args[20];
	int i;
	int n;

	/* add svp to device list */

	gd->svpp = (ISwitchVectorProperty **) XtRealloc ((char *)(gd->svpp),
				(gd->nsvpp+1) * sizeof(ISwitchVectorProperty*));
	gd->svpp[gd->nsvpp++] = svp;

	/* master rc, work faster delay managing until finished */

	n = 0;
	XtSetArg (args[n], XmNspacing, 0); n++;
	src_w = XmCreateRowColumn (gg->prc_w, "SRC", args, n);

	/* property rc */

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, INDENT); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	hrc_w = XmCreateRowColumn (src_w, "SPF", args, n);
	XtManageChild (hrc_w);

	    /* property's status light */

	    n = 0;
	    svp->aux = (void *) createLt (hrc_w, "NLS", args, n);
	    XtAddCallback ((Widget)svp->aux, XmNexposeCallback, devLtCB,
						    (XtPointer)(gd-devices));
	    XtManageChild ((Widget)svp->aux);

	    /* label */

	    n = 0;
	    l_w = XmCreateLabel (hrc_w, "SL", args, n);
	    set_xmstring (l_w, XmNlabelString, (char *) svp->label);
	    XtManageChild (l_w);

	/* each element in a RC */

	n = 0;
	XtSetArg (args[n], XmNspacing, 4); n++;
	XtSetArg (args[n], XmNmarginWidth, 2*INDENT); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNnumColumns, svp->nsp/5+1); n++;
	switch (svp->r) {
	    case ISR_1OFMANY:
		XtSetArg (args[n], XmNradioBehavior, 1); n++;
		XtSetArg (args[n], XmNradioAlwaysOne, 1); n++;
		break;
	    case ISR_ATMOST1:
		XtSetArg (args[n], XmNradioBehavior, 1); n++;
		XtSetArg (args[n], XmNradioAlwaysOne, 0); n++;
		break;
	    case ISR_NOFMANY:
		XtSetArg (args[n], XmNradioBehavior, 0); n++;
		XtSetArg (args[n], XmNradioAlwaysOne, 0); n++;
		break;
	}
	hrc_w = XmCreateRowColumn (src_w, "SRC", args, n);
	XtManageChild (hrc_w);

	for (i = 0; i < svp->nsp; i++) {
	    ISwitch *sp = &svp->sp[i];

	    n = 0;
	    XtSetArg (args[n], XmNindicatorType, svp->r == ISR_NOFMANY ? 
					    XmN_OF_MANY : XmONE_OF_MANY); n++;
	    XtSetArg (args[n], XmNset, sp->s == ISS_ON); n++;
	    w = XmCreateToggleButton (hrc_w, "STB", args, n);
	    XtAddCallback (w, XmNvalueChangedCallback, newswitch_cb, sp);
	    set_xmstring (w, XmNlabelString, sp->label);
	    XtManageChild (w);
	    sp->aux = (void *) w;
	}

	drawDevLt (gd);

	XtManageChild (src_w);
	pushDisplay();

	return (0);
}

/* capture and display state, msg and each value(s) for a set/newSwitchVector */
static int
setGUISwitchVector (XMLEle *root, ISwitchVectorProperty *svp, IGUIDevice *gd,
char errmsg[])
{
	XMLEle *ep;

	for (ep = nextXMLEle(root,1); ep; ep = nextXMLEle(root,0))  {
	    if (strcmp(tagXMLEle(ep), "oneSwitch") == 0) {
		char *name = findXMLAttValu (ep, "name");
		ISwitch *sp = findSwitch (svp, name);
		if (sp) {
		    if (crackSState (ep, &sp->s, errmsg) < 0)
			return (-1);
		    XmToggleButtonSetState ((Widget)sp->aux, sp->s==ISS_ON, 0);
		} else {
		    sprintf (errmsg, "INDI: set %s.%s.%s not found", gd->device,
							    svp->name, name);
		    return (-1);
		}
	    }
	}

	showMessage (root);
	if (crackPState (root, &svp->s, errmsg) == 0)
	    drawDevLt (gd);
	return (0);
}

/* add the given light onto the given device list in the given group and
 * build its GUI.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
newGUILightVector(ILightVectorProperty *lvp, IGUIDevice *gd, IGUIGroup *gg,
char *errmsg)
{
	Widget lrc_w;
	Widget w, l_w, hrc_w;
	Arg args[20];
	int i;
	int n;

	/* add lvp to device list */

	gd->lvpp = (ILightVectorProperty **) XtRealloc ((char *)(gd->lvpp),
				(gd->nlvpp+1) * sizeof(ILightVectorProperty*));
	gd->lvpp[gd->nlvpp++] = lvp;

	/* master rc, works faster to delay managing until finished */

	n = 0;
	XtSetArg (args[n], XmNspacing, 0); n++;
	lrc_w = XmCreateRowColumn (gg->prc_w, "LRC", args, n);

	/* property rc */

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, INDENT); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	hrc_w = XmCreateRowColumn (lrc_w, "LPF", args, n);
	XtManageChild (hrc_w);

	    /* property's status light */

	    n = 0;
	    lvp->aux = (void *) createLt (hrc_w, "NLL", args, n);
	    XtAddCallback ((Widget)lvp->aux, XmNexposeCallback, devLtCB,
						    (XtPointer)(gd-devices));
	    XtManageChild ((Widget)lvp->aux);

	    /* label */

	    n = 0;
	    l_w = XmCreateLabel (hrc_w, "LL", args, n);
	    set_xmstring (l_w, XmNlabelString, (char *) lvp->label);
	    XtManageChild (l_w);

	/* each element in a RC */

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, 2*INDENT); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNnumColumns, lvp->nlp); n++;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	hrc_w = XmCreateRowColumn (lrc_w, "SRC", args, n);
	XtManageChild (hrc_w);

	for (i = 0; i < lvp->nlp; i++) {
	    ILight *lp = &lvp->lp[i];
	    char buf[128];

	    n = 0;
	    w = createLt (hrc_w, "LLt", args, n);
	    XtManageChild (w);
	    lp->aux = (void *) w;

	    n = 0;
	    w = XmCreateLabel (hrc_w, "LL", args, n);
	    sprintf (buf, "%s ", lp->label);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);
	}

	drawDevLt (gd);

	XtManageChild (lrc_w);
	pushDisplay();

	return (0);
}

/* capture and display state, msg and each new value(s) for a Light vector */
static int
setGUILightVector (XMLEle *root, ILightVectorProperty *lvp, IGUIDevice *gd,
char errmsg[])
{
	XMLEle *ep;

	for (ep = nextXMLEle(root,1); ep; ep = nextXMLEle(root,0))  {
	    if (strcmp(tagXMLEle(ep), "oneLight") == 0) {
		char *name = findXMLAttValu (ep, "name");
		ILight *lp = findLight (lvp, name);
		if (lp) {
		    if (crackPString (pcdataXMLEle(ep), &lp->s, errmsg) < 0)
			return (-1);
		    drawLt ((Widget)lp->aux, lp->s);
		} else {
		    sprintf (errmsg, "INDI: set %s.%s.%s not found", gd->device,
							    lvp->name, name);
		    return (-1);
		}
	    }
	}

	showMessage (root);
	if (crackPState (root, &lvp->s, errmsg) == 0)
	    drawDevLt (gd);
	return (0);
}

/* add the given BLOB onto the given device list in the given group and
 * build its GUI. set up display and saving callbacks to enable BLOBs on the
 * service channel.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
newGUIBLOBVector(IBLOBVectorProperty *bvp, IGUIDevice *gd, IGUIGroup *gg,
char *errmsg)
{
	Widget set_w, brc_w;
	Widget w, hrc_w, f_w, l_w;
	Arg args[20];
	int i;
	int n;

	/* add bvp to device list */

	gd->bvpp = (IBLOBVectorProperty **) XtRealloc ((char *)(gd->bvpp),
				(gd->nbvpp+1) * sizeof(IBLOBVectorProperty*));
	gd->bvpp[gd->nbvpp++] = bvp;

	/* master rc, works faster to delay managing until finished */

	n = 0;
	XtSetArg (args[n], XmNspacing, 0); n++;
	brc_w = XmCreateRowColumn (gg->prc_w, "BRC", args, n);

	/* property rc */

	n = 0;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNmarginWidth, INDENT); n++;
	XtSetArg (args[n], XmNspacing, 3); n++;
	hrc_w = XmCreateRowColumn (brc_w, "BPF", args, n);
	XtManageChild (hrc_w);

	    /* property's status light */

	    n = 0;
	    bvp->aux = (void *) createLt (hrc_w, "BLT", args, n);
	    XtAddCallback ((Widget)bvp->aux, XmNexposeCallback, devLtCB,
						    (XtPointer)(gd-devices));
	    XtManageChild ((Widget)bvp->aux);

	    /* set PB, unless RO */

	    if (bvp->p != IP_RO) {
		n = 0;
		set_w = XmCreatePushButton (hrc_w, "Set", args, n);
		XtAddCallback (set_w, XmNactivateCallback, newBLOB_cb, bvp);
		wtip (set_w, "Send the named file");
		XtManageChild (set_w);
	    }

	    /* label */

	    n = 0;
	    l_w = XmCreateLabel (hrc_w, "BVL", args, n);
	    set_xmstring (l_w, XmNlabelString, (char *) bvp->label);
	    XtManageChild (l_w);

	/* one row for each vector element */

	for (i = 0; i < bvp->nbp; i++) {
	    IBLOB *bp = &bvp->bp[i];

	    /* TODO: this is really only correct for RO perm only */

	    n = 0;
	    f_w = XmCreateForm (brc_w, "BF", args, n);
	    XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 2*INDENT); n++;
	    l_w = XmCreateLabel (f_w, "BL", args, n);
	    set_xmstring (l_w, XmNlabelString, (char *) bp->label);
	    XtManageChild (l_w);

	    /* aux2 is filename T */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 100); n++;
	    XtSetArg (args[n], XmNcolumns, 23); n++;	/* ISO date + .fts */
	    w = XmCreateTextField (f_w, "BFN", args, n);
	    switch (bvp->p) {
	    case IP_WO:
		wtip (w, "Name of file to send");
		XtAddCallback (w, XmNactivateCallback, newBLOB_cb, bvp);
		break;
	    case IP_RO:
		wtip (w, "Name of file to save");
		break;
	    case IP_RW:
		wtip (w, "Name of file to send or save");
		break;
	    }
	    XtManageChild (w);
	    bp->aux2 = w;

	    if (bvp->p != IP_WO) {

		/* aux1 is Save TB */

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
		XtSetArg (args[n], XmNrightWidget, w); n++;
		XtSetArg (args[n], XmNrightOffset, 10); n++;
		w = XmCreateToggleButton (f_w, "Save", args, n);
		wtip (w, "Automatically save each incoming image");
		set_xmstring (w, XmNlabelString, "Auto save to:");
		XtAddCallback (w, XmNvalueChangedCallback, saveBLOBCB, bvp);
		XtManageChild (w);
		bp->aux1 = w;

		/* aux0 is Display TB */

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
		XtSetArg (args[n], XmNrightWidget, w); n++;
		XtSetArg (args[n], XmNrightOffset, 10); n++;
		w = XmCreateToggleButton (f_w, "Display", args, n);
		wtip (w, "Automatically display each incoming FITS image");
		XtAddCallback (w, XmNvalueChangedCallback, displayBLOBCB, bvp);
		set_xmstring (w, XmNlabelString, "Auto display");
		XtManageChild (w);
		bp->aux0 = w;
	    }
	}

	drawDevLt (gd);

	XtManageChild (brc_w);
	pushDisplay();

	return (0);
}

/* capture and display state, msg and each new value(s) for a BLOB vector */
static int
setGUIBLOBVector (XMLEle *root, IBLOBVectorProperty *bvp, IGUIDevice *gd,
char errmsg[])
{
	XMLEle *ep;

	for (ep = nextXMLEle(root,1); ep; ep = nextXMLEle(root,0))  {
	    if (strcmp(tagXMLEle(ep), "oneBLOB") == 0) {
		char *name = findXMLAttValu (ep, "name");
		IBLOB *bp = findBLOB (bvp, name);
		if (bp) {
		    if (handleOneBLOB (ep, bp, errmsg) < 0)
			return (-1);
		} else {
		    sprintf (errmsg, "INDI: set %s.%s.%s not found", gd->device,
							    bvp->name, name);
		    return (-1);
		}
	    }
	}

	showMessage (root);
	if (crackPState (root, &bvp->s, errmsg) == 0)
	    drawDevLt (gd);
	return (0);
}

/* process the given XML just arrived for the given oneBLOB.
 * N.B. beware persistence: the XML memory is freed shortly after we return
 */
static int
handleOneBLOB (XMLEle *root, IBLOB *bp, char errmsg[])
{
	IBLOBVectorProperty *bvp = bp->bvp;
	char fn[64];
	int isz;
	int i;
	char *tsp;

	/* get uncompressed size */
	bp->size = atoi(findXMLAttValu (root, "size"));
	if (bp->size < 0) {
	    sprintf (errmsg, "INDI: %s.%s.%s no size", bvp->device, bvp->name,
								    bp->name);
	    return (-1);
	}
	if (bp->size == 0)
	    return (0);		/* well, ok, that was easy */

	/* get format */
	if (crackCharAtt (root, bp->format, MAXINDIBLOBFMT, "format", errmsg)<0)
	    return (-1);
	isz = !strcmp (bp->format,".fts.z") || !strcmp (bp->format,".fits.z");
	if (!isz && strcmp(bp->format,".fts") && strcmp(bp->format,".fits")) {
	    sprintf (errmsg, "INDI: %s.%s.%s not FITS", bvp->device, bvp->name,
								    bp->name);
	    return (-1);
	}

	/* decode blob */
	if (bp->blob)
	    free (bp->blob);
	bp->blob = malloc (3*pcdatalenXMLEle(root)/4);
	bp->bloblen = from64tobits (bp->blob, pcdataXMLEle(root));
	if (bp->bloblen < 0) {
	    free (bp->blob);
	    sprintf (errmsg, "INDI: %s.%s.%s bad base64", bvp->device,
							bvp->name, bp->name);
	    return (-1);
	}

	/* uncompress effectively in place if z */
	if (isz) {
	    uLong nuncomp = bp->size;
	    unsigned char *uncomp = malloc (nuncomp);
	    int ok = uncompress (uncomp, &nuncomp, bp->blob, bp->bloblen);
	    if (ok != Z_OK) {
		sprintf (errmsg, "INDI: %s.%s.%s uncompress error %d",
					bvp->device, bvp->name, bp->name, ok);
		free (uncomp);
		return (-1);
	    }
	    free (bp->blob);
	    bp->blob = uncomp;
	    bp->bloblen = nuncomp;
	}

	/* rig up a file name from the timestamp and format */
	for (i = 0, tsp = bvp->timestamp; *tsp != '\0'; tsp++)
	    if (isdigit(*tsp))
		fn[i++] = *tsp;
	fn[i] = '\0';
	strcat (fn, bp->format);
	if (isz)
	    *(strstr (fn,".z")) = '\0'; 	/* chop off .z */
	XmTextFieldSetString ((Widget)bp->aux2, fn);

	/* display, if enabled */
	if (XmToggleButtonGetState ((Widget)bp->aux0)) {
	    FImage fim, *fip = &fim;
	    if (readFITSmem (bp->blob, bp->bloblen, fip, errmsg) < 0)
		return (-1);
	    sf_newFITS (fip, fn, 1);
	}

	/* save, if enabled */
	if (XmToggleButtonGetState ((Widget)bp->aux1)) {
	    FILE *fp = fopend (fn, NULL, "w");
	    if (fp) {
		fwrite (bp->blob, bp->bloblen, 1, fp);
		fclose (fp);
		strcat (fn, ": saved");
		logMessage (bvp->timestamp, bvp->device, fn);
	    } else {
		/* fopend() already notified user */
		XmToggleButtonSetState ((Widget)bp->aux0, False, True);
	    }
	}

	return (0);
}

/* display device, timestamp and message attributes, if message */
static void
showMessage (XMLEle *root)
{
	char *m, *t, *d;

	m = findXMLAttValu (root, "message");
	if (!m[0])
	    return;
	t = findXMLAttValu (root, "timestamp");
	d = findXMLAttValu (root, "device");

	logMessage (t, d, m);
}

/* fill the given string with the current time as an INDI timestamp */
static char *
setTStampNow (char tstamp[MAXINDITSTAMP])
{
	time_t t = time(NULL);
	struct tm *tp = gmtime (&t);
	strftime (tstamp, MAXINDITSTAMP, "%Y-%m-%dT%H:%M:%S", tp);
	return (tstamp);
}

/* log a message prefixed with time and device */
static void
logMessage (char *tstamp, char *device, char *msg)
{
	char buf[1024], *bp = buf;
	char *msgnow;
	int msgnowl;

	/* time and device first */
	if (tstamp && tstamp[0])
	    bp += sprintf (bp, "%s: ", tstamp);
	else {
	    char ts[MAXINDITSTAMP];
	    bp += sprintf (bp, "%s: ", setTStampNow(ts));
	}
	if (device && device[0])
	    bp += sprintf (bp, "%s: ", device);

	/* add the message */
	bp += sprintf (bp, "%s", msg);
	if (bp[-1] != '\n') {
	    *bp++ = '\n';
	    *bp = '\0';
	}

	/* append to scrolled text */
	msgnow = XmTextGetString (msg_w);
	msgnowl = strlen(msgnow);
	XtFree (msgnow);
	XmTextReplace (msg_w, msgnowl, msgnowl, buf);

	/* append to trace file, if on */
	if (XmToggleButtonGetState (mttb_w))
	    traceMessage (buf);
}

/* append buf in the trace file */
static void
traceMessage (char *buf)
{
	char *fn = XmTextFieldGetString (mttf_w);
	FILE *fp = fopend (fn, NULL, "a");
	if (fp) {
	    fputs (buf, fp);
	    fclose (fp);
	} else {
	    XmToggleButtonSetState (mttb_w, False, True);
	    xe_msg (1, "%s:\n%s", fn, syserrstr());
	}
	XtFree (fn);
}

/* called when tracing is turned on or off */
static void
traceon_cb (Widget w, XtPointer client, XtPointer call)
{
	if (XmToggleButtonGetState(w)) {
	    char *msgnow = XmTextGetString (msg_w);
	    traceMessage (msgnow);
	    XtFree (msgnow);
	}
}

/* draw the status light for the given property.
 * visit each property and draw its light, then draw the worst state for
 * device as a whole
 */
static void
drawDevLt (IGUIDevice *gd)
{
	IPState s, maxs = IPS_IDLE;
	int i, j;

	for (i = 0; i < gd->ntvpp; i++) {
	    drawLt ((Widget)gd->tvpp[i]->aux, s = gd->tvpp[i]->s);
	    if (s > maxs)
		maxs = s;
	}
	for (i = 0; i < gd->nnvpp; i++) {
	    drawLt ((Widget)gd->nvpp[i]->aux, s = gd->nvpp[i]->s);
	    if (s > maxs)
		maxs = s;
	}
	for (i = 0; i < gd->nsvpp; i++) {
	    drawLt ((Widget)gd->svpp[i]->aux, s = gd->svpp[i]->s);
	    if (s > maxs)
		maxs = s;
	}
	for (i = 0; i < gd->nlvpp; i++) {
	    drawLt ((Widget)gd->lvpp[i]->aux, s = gd->lvpp[i]->s);
	    if (s > maxs)
		maxs = s;
	    for (j = 0; j < gd->lvpp[i]->nlp; j++)
		drawLt ((Widget)gd->lvpp[i]->lp[j].aux, gd->lvpp[i]->lp[j].s);

	}
	for (i = 0; i < gd->nbvpp; i++) {
	    drawLt ((Widget)gd->bvpp[i]->aux, s = gd->bvpp[i]->s);
	    if (s > maxs)
		maxs = s;
	}

	drawLt (gd->lda_w, maxs);
}

/* find existing or optionally create new IGUIDevice for the given device.
 * return pointer if ok (always true if create) else NULL with why in errmsg[]
 */
static IGUIDevice *
findDev (char *device, int create, char errmsg[])
{
	IGUIDevice *gd;
	int i;

	for (i = 0; i < ndevices; i++)
	    if (strcmp (devices[i].device, device) == 0)
		break;
	if (i < ndevices)
	    gd = &devices[i];
	else if (create) {
	    devices = (IGUIDevice *) XtRealloc ((void *) devices,
					    (ndevices+1)*sizeof(IGUIDevice));
	    gd = &devices[ndevices++];
	    memset (gd, 0, sizeof(*gd));
	    strcpy (gd->device, device);
	    newGUIDevice (gd);
	} else {
	    if (errmsg)
		sprintf (errmsg, "Device %s not found", device);
	    return (NULL);
	}

	return (gd);
}

/* find existing or optionally create new IGUIGroup for the given device.
 * return pointer if ok (always true if create) else NULL with why in errmsg[]
 */
static IGUIGroup *
findGroup (IGUIDevice *gd, char *group, int create, char errmsg[])
{
	int i;

	for (i = 0; i < gd->ngrp; i++)
	    if(!strcmp (gd->grp[i]->group, group))
		return (gd->grp[i]);

	if (create) {
	    IGUIGroup *gg;

	    gd->grp = (IGUIGroup **) XtRealloc ((void *) gd->grp,
					    (gd->ngrp+1)*sizeof(IGUIGroup*));
	    gg = gd->grp[gd->ngrp++] = (IGUIGroup *)XtMalloc(sizeof(IGUIGroup));
	    memset (gg, 0, sizeof(*gg));
	    strcpy (gg->group, group);
	    newGUIGroup (gd, gg);
	    return (gg);
	} else {
	    sprintf (errmsg, "Group %s not found", group);
	    return (NULL);
	}
}

/* find the named Number Vector in the given device, else return NULL with
 * reason in optional whynot[]
 */
static INumberVectorProperty *
findNumberProperty (IGUIDevice *gd, char *name, char whynot[])
{
	int i;

	for (i = 0; i < gd->nnvpp; i++)
	    if (strcmp (gd->nvpp[i]->name, name) == 0)
		return (gd->nvpp[i]);
	if (whynot)
	    sprintf (whynot, "INDI %s.%s: number not found", gd->device, name);
	return (NULL);
}

/* find the named Text Vector in the given device, else return NULL
 * reason in optional whynot[]
 */
static ITextVectorProperty *
findTextProperty (IGUIDevice *gd, char *name, char whynot[])
{
	int i;

	for (i = 0; i < gd->ntvpp; i++)
	    if (strcmp (gd->tvpp[i]->name, name) == 0)
		return (gd->tvpp[i]);
	if (whynot)
	    sprintf (whynot, "INDI %s.%s: text not found", gd->device, name);
	return (NULL);
}

/* find the named Switch Vector in the given device, else return NULL
 * reason in optional whynot[]
 */
static ISwitchVectorProperty *
findSwitchProperty (IGUIDevice *gd, char *name, char whynot[])
{
	int i;

	for (i = 0; i < gd->nsvpp; i++)
	    if (strcmp (gd->svpp[i]->name, name) == 0)
		return (gd->svpp[i]);
	if (whynot)
	    sprintf (whynot, "INDI %s.%s: switch not found", gd->device, name);
	return (NULL);
}

/* find the named Light Vector in the given device, else return NULL
 * reason in optional whynot[]
 */
static ILightVectorProperty *
findLightProperty (IGUIDevice *gd, char *name, char whynot[])
{
	int i;

	for (i = 0; i < gd->nlvpp; i++)
	    if (strcmp (gd->lvpp[i]->name, name) == 0)
		return (gd->lvpp[i]);
	if (whynot)
	    sprintf (whynot, "INDI %s.%s: light not found", gd->device, name);
	return (NULL);
}

/* find the named BLOB Vector in the given device, else return NULL
 * reason in optional whynot[]
 */
static IBLOBVectorProperty *
findBLOBProperty (IGUIDevice *gd, char *name, char whynot[])
{
	int i;

	for (i = 0; i < gd->nbvpp; i++)
	    if (strcmp (gd->bvpp[i]->name, name) == 0)
		return (gd->bvpp[i]);
	if (whynot)
	    sprintf (whynot, "INDI %s.%s: BLOB not found", gd->device, name);
	return (NULL);
}

/* find a particular Number in a NumberVector */
static INumber *
findNumber (INumberVectorProperty *nvp, char *name)
{
	INumber *np;

	for (np = nvp->np; np < &nvp->np[nvp->nnp]; np++)
	    if (strcmp (np->name, name) == 0)
		return (np);
	return (NULL);
}

/* find a particular Text in a TextVector */
static IText *
findText (ITextVectorProperty *tvp, char *name)
{
	IText *tp;

	for (tp = tvp->tp; tp < &tvp->tp[tvp->ntp]; tp++)
	    if (strcmp (tp->name, name) == 0)
		return (tp);
	return (NULL);
}

/* find a particular Switch in a SwitchVector */
static ISwitch *
findSwitch (ISwitchVectorProperty *svp, char *name)
{
	ISwitch *sp;

	for (sp = svp->sp; sp < &svp->sp[svp->nsp]; sp++)
	    if (strcmp (sp->name, name) == 0)
		return (sp);
	return (NULL);
}

/* find a particular Light in a LightVector */
static ILight *
findLight (ILightVectorProperty *lvp, char *name)
{
	ILight *lp;

	for (lp = lvp->lp; lp < &lvp->lp[lvp->nlp]; lp++)
	    if (strcmp (lp->name, name) == 0)
		return (lp);
	return (NULL);
}

/* find a particular BLOB in a BLOBVector */
static IBLOB *
findBLOB (IBLOBVectorProperty *bvp, char *name)
{
	IBLOB *bp;

	for (bp = bvp->bp; bp < &bvp->bp[bvp->nbp]; bp++)
	    if (strcmp (bp->name, name) == 0)
		return (bp);
	return (NULL);
}

/* crack any char[] attribute of the given XML element.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
crackCharAtt (XMLEle *root, char *vp, int maxl, char *name, char *errmsg)
{
	char *p = findXMLAttValu (root, name);
	if (!p[0]) {
	    sprintf (errmsg, "%s has no %s attribute", tagXMLEle(root), name);
	    return (-1);
	}
	strncpy (vp, p, maxl);
	vp[maxl-1] = '\0';
	return (0);
}

/* crack a double attribute of the given XML element.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
crackDoubleAtt (XMLEle *root, double *dop, char *name, char *errmsg)
{
	char *t = findXMLAttValu (root, name);
	if (!t[0]) {
	    sprintf (errmsg, "%s has no %s attribute", tagXMLEle(root), name);
	    return (-1);
	}
	*dop = strtod (t, NULL);
	return (0);
}

/* crack the perm attribute of the given XML element.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
crackPerm (XMLEle *root, IPerm *p, char *errmsg)
{
	char *pstr = findXMLAttValu (root, "perm");
	if (!pstr[0]) {
	    sprintf (errmsg, "%s has no perm attribute", tagXMLEle(root));
	    return (-1);
	}

	if (strcmp (pstr, "ro") == 0)
	    *p = IP_RO;
	else if (strcmp (pstr, "wo") == 0)
	    *p = IP_WO;
	else if (strcmp (pstr, "rw") == 0)
	    *p = IP_RW;
	else {
	    sprintf (errmsg, "%s has bogus perm %s", tagXMLEle(root), pstr);
	    return (-1);
	}

	return (0);
}

/* crack the rule attribute of the given XML element.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
crackRule (XMLEle *root, ISRule *r, char *errmsg)
{
	char *rstr = findXMLAttValu (root, "rule");
	if (!rstr[0]) {
	    sprintf (errmsg, "%s has no rule attribute", tagXMLEle(root));
	    return (-1);
	}

	if (strcmp (rstr, "OneOfMany") == 0)
	    *r = ISR_1OFMANY;
	else if (strcmp (rstr, "AtMostOne") == 0)
	    *r = ISR_ATMOST1;
	else if (strcmp (rstr, "AnyOfMany") == 0)
	    *r = ISR_NOFMANY;
	else {
	    sprintf (errmsg, "%s has bogus rule %s", tagXMLEle(root), rstr);
	    return (-1);
	}

	return (0);
}

/* crack the switch state pcdata value of the given XML element.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
crackSState (XMLEle *ep, ISState *s, char *errmsg)
{
	char *sstr = pcdataXMLEle (ep);

	if (strcmp (sstr, "On") == 0)
	    *s = ISS_ON;
	else if (strcmp (sstr, "Off") == 0)
	    *s = ISS_OFF;
	else {
	    sprintf(errmsg, "%s.%s: bogus switch state %s",
			    tagXMLEle(parentXMLEle(ep)), tagXMLEle(ep), sstr);
	    return (-1);
	}

	return (0);
}

/* crack the property state attribute of the given XML element.
 * return 0 if ok else -1 with reason in errmsg[]
 */
static int
crackPState (XMLEle *root, IPState *sp, char *errmsg)
{
	char *sstr = findXMLAttValu (root, "state");
	if (!sstr[0]) {
	    sprintf (errmsg, "%s has no state attribute", tagXMLEle(root));
	    return (-1);
	}
	return (crackPString (sstr, sp, errmsg));
}

static int
crackPString (char *str, IPState *sp, char errmsg[])
{
	if (strstr (str, "Idle"))
	    *sp = IPS_IDLE;
	else if (strstr (str, "Ok"))
	    *sp = IPS_OK;
	else if (strstr (str, "Busy"))
	    *sp = IPS_BUSY;
	else if (strstr (str, "Alert"))
	    *sp = IPS_ALERT;
	else {
	    sprintf (errmsg, "IPState %s not found", str);
	    return (-1);
	}
	return (0);
}

/* crack the given defNumberVector XML element.
 * return a malloced INumberVectorProperty if ok else NULL and reason in errmsg
 */
static INumberVectorProperty *
crackdefNumberVector (XMLEle *root, char *errmsg)
{
	INumberVectorProperty nv, *nvp;		/* use stack until sure */
	INumber *np;
	XMLEle *e;

	/* init */
	memset (&nv, 0, sizeof(nv));

	/* crack the attributes */
	if (crackCharAtt (root, nv.device, MAXINDIDEVICE,"device",errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, nv.name, MAXINDINAME, "name", errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, nv.label, MAXINDILABEL, "label", errmsg) < 0)
	    strcpy (nv.label, nv.name);		/* default label = name */
	if (crackCharAtt (root, nv.group, MAXINDIGROUP, "group", errmsg) < 0)
	    nv.group[0] = '\0';			/* default group is "" */
	if (crackPerm (root, &nv.p, errmsg) < 0)
	    return (NULL);
	if (crackDoubleAtt (root, &nv.timeout, "timeout", errmsg) < 0)
	    nv.timeout = 0;
	if (crackPState (root, &nv.s, errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root,nv.timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (nv.timestamp);

	/* crack each number element */
	for (e = nextXMLEle(root,1); e != NULL; e = nextXMLEle(root,0)) {
	    if (strcmp (tagXMLEle(e), "defNumber") == 0) {
		/* grow list, but don't count until sure */
		nv.np = (INumber *) XtRealloc ((char *)nv.np, 
						(nv.nnp+1)*sizeof(INumber));
		np = &nv.np[nv.nnp];
		memset (np, 0, sizeof(*np));

		/* add each field */
		if (crackCharAtt (e, np->name, MAXINDINAME, "name",errmsg) < 0)
		    return (NULL);
		if (crackCharAtt (e, np->label, MAXINDILABEL,"label",errmsg)<0)
		    strcpy (np->label, np->name);	/* default */
		if (crackCharAtt(e, np->format,MAXINDIFORMAT,"format",errmsg)<0)
		    return (NULL);
		if (crackDoubleAtt (e, &np->min, "min", errmsg) < 0)
		    return (NULL);
		if (crackDoubleAtt (e, &np->max, "max", errmsg) < 0)
		    return (NULL);
		if (crackDoubleAtt (e, &np->step,"step", errmsg) < 0)
		    return (NULL);
		np->value = strtod (pcdataXMLEle(e), NULL);

		/* confident now in this Number */
		nv.nnp++;
	    }
	}

	/* make the persistent copy, go back and set parent pointers */
	nvp = (INumberVectorProperty *) memcpy (XtMalloc(sizeof(nv)), &nv,
								sizeof(nv));
	for (np = &nvp->np[0]; np < &nvp->np[nvp->nnp]; np++)
	    np->nvp = nvp;

	/* return malloced copy */
	return (nvp);
}

/* crack the given defTextVector XML element.
 * return a malloced ITextVectorProperty if ok else NULL and reason in errmsg
 */
static ITextVectorProperty *
crackdefTextVector (XMLEle *root, char *errmsg)
{
	ITextVectorProperty tv, *tvp;		/* use stack until sure */
	IText *tp;
	XMLEle *e;

	/* init */
	memset (&tv, 0, sizeof(tv));

	/* crack the attributes */
	if (crackCharAtt (root, tv.device, MAXINDIDEVICE,"device", errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, tv.name, MAXINDINAME, "name", errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, tv.label, MAXINDILABEL, "label", errmsg) < 0)
	    strcpy (tv.label, tv.name);		/* default label = name */
	if (crackCharAtt (root, tv.group, MAXINDIGROUP, "group", errmsg) < 0)
	    tv.group[0] = '\0';			/* default group is "" */
	if (crackPerm (root, &tv.p, errmsg) < 0)
	    return (NULL);
	if (crackDoubleAtt (root, &tv.timeout, "timeout", errmsg) < 0)
	    tv.timeout = 0;
	if (crackPState (root, &tv.s, errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root,tv.timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (tv.timestamp);

	/* crack each text element */
	for (e = nextXMLEle(root,1); e != NULL; e = nextXMLEle(root,0)) {
	    if (strcmp (tagXMLEle(e), "defText") == 0) {
		/* grow list, but don't count until sure */
		tv.tp = (IText *) XtRealloc ((char *)tv.tp, 
						    (tv.ntp+1)*sizeof(IText));
		tp = &tv.tp[tv.ntp];
		memset (tp, 0, sizeof(*tp));

		/* add each field */
		if (crackCharAtt (e, tp->name, MAXINDINAME, "name",errmsg) < 0)
		    return (NULL);
		if (crackCharAtt (e, tp->label, MAXINDILABEL,"label",errmsg)<0)
		    strcpy (tp->label, tp->name);	/* default */
		tp->text = XtNewString(pcdataXMLEle(e));

		/* confident now in this Text */
		tv.ntp++;
	    }
	}

	/* make the persistent copy, go back and set parent pointers */
	tvp = (ITextVectorProperty *) memcpy (XtMalloc(sizeof(tv)), &tv,
								sizeof(tv));
	for (tp = &tvp->tp[0]; tp < &tvp->tp[tvp->ntp]; tp++)
	    tp->tvp = tvp;

	/* return malloced copy */
	return (tvp);
}

/* crack the given defSwitchVector XML element.
 * return a malloced ISwitchVectorProperty if ok else NULL and reason in errmsg
 */
static ISwitchVectorProperty *
crackdefSwitchVector (XMLEle *root, char *errmsg)
{
	ISwitchVectorProperty sv, *svp;		/* use stack until sure */
	ISwitch *sp;
	XMLEle *e;

	/* init */
	memset (&sv, 0, sizeof(sv));

	/* crack the attributes */
	if (crackCharAtt (root, sv.device, MAXINDIDEVICE,"device", errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, sv.name, MAXINDINAME, "name", errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, sv.label, MAXINDILABEL, "label", errmsg) < 0)
	    strcpy (sv.label, sv.name);		/* default label = name */
	if (crackCharAtt (root, sv.group, MAXINDIGROUP, "group", errmsg) < 0)
	    sv.group[0] = '\0';			/* default group is "" */
	if (crackPerm (root, &sv.p, errmsg) < 0)
	    return (NULL);
	if (crackDoubleAtt (root, &sv.timeout, "timeout", errmsg) < 0)
	    sv.timeout = 0;
	if (crackPState (root, &sv.s, errmsg) < 0)
	    return (NULL);
	if (crackRule (root, &sv.r, errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root,sv.timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (sv.timestamp);

	/* crack each switch element */
	for (e = nextXMLEle(root,1); e != NULL; e = nextXMLEle(root,0)) {
	    if (strcmp (tagXMLEle(e), "defSwitch") == 0) {
		/* grow list, but don't count until sure */
		sv.sp = (ISwitch *) XtRealloc ((char *)sv.sp, 
						    (sv.nsp+1)*sizeof(ISwitch));
		sp = &sv.sp[sv.nsp];
		memset (sp, 0, sizeof(*sp));

		/* add each field */
		if (crackCharAtt (e, sp->name, MAXINDINAME, "name",errmsg) < 0)
		    return (NULL);
		if (crackCharAtt (e, sp->label, MAXINDILABEL,"label",errmsg)<0)
		    strcpy (sp->label, sp->name);	/* default */
		if (crackSState (e, &sp->s, errmsg) < 0)
		    return (NULL);

		/* confident now in this Switch */
		sv.nsp++;
	    }
	}

	/* make the persistent copy, go back and set parent pointers */
	svp = (ISwitchVectorProperty *) memcpy (XtMalloc(sizeof(sv)), &sv,
								sizeof(sv));
	for (sp = &svp->sp[0]; sp < &svp->sp[svp->nsp]; sp++)
	    sp->svp = svp;

	/* return malloced copy */
	return (svp);
}

/* crack the given defLightVectory XML element.
 * return a malloced ILightVectorProperty if ok else NULL and reason in errmsg
 */
static ILightVectorProperty *
crackdefLightVector (XMLEle *root, char *errmsg)
{
	ILightVectorProperty lv, *lvp;		/* use stack until sure */
	ILight *lp;
	XMLEle *e;

	/* init */
	memset (&lv, 0, sizeof(lv));

	/* crack the attributes */
	if (crackCharAtt (root, lv.device, MAXINDIDEVICE,"device", errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, lv.name, MAXINDINAME, "name", errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, lv.label, MAXINDILABEL, "label", errmsg) < 0)
	    strcpy (lv.label, lv.name);		/* default label = name */
	if (crackCharAtt (root, lv.group, MAXINDIGROUP, "group", errmsg) < 0)
	    lv.group[0] = '\0';			/* default group is "" */
	if (crackPState (root, &lv.s, errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root,lv.timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (lv.timestamp);

	/* crack each light element */
	for (e = nextXMLEle(root,1); e != NULL; e = nextXMLEle(root,0)) {
	    if (strcmp (tagXMLEle(e), "defLight") == 0) {
		/* grow list, but don't count until sure */
		lv.lp = (ILight *) XtRealloc ((char *)lv.lp, 
						    (lv.nlp+1)*sizeof(ILight));
		lp = &lv.lp[lv.nlp];
		memset (lp, 0, sizeof(*lp));

		/* add each field */
		if (crackCharAtt (e, lp->name, MAXINDINAME, "name",errmsg) < 0)
		    return (NULL);
		if (crackCharAtt (e, lp->label, MAXINDILABEL,"label",errmsg)<0)
		    strcpy (lp->label, lp->name);	/* default */
		if (crackPString (pcdataXMLEle(e), &lp->s, errmsg) < 0)
		    return (NULL);

		/* confident now in this Light */
		lv.nlp++;
	    }
	}

	/* make the persistent copy, go back and set parent pointers */
	lvp = (ILightVectorProperty *) memcpy (XtMalloc(sizeof(lv)), &lv,
								sizeof(lv));
	for (lp = &lvp->lp[0]; lp < &lvp->lp[lvp->nlp]; lp++)
	    lp->lvp = lvp;

	/* return malloced copy */
	return (lvp);
}

/* crack the given defBLOBVector XML element.
 * return a malloced IBLOBVectorProperty if ok else NULL and reason in errmsg
 */
static IBLOBVectorProperty *
crackdefBLOBVector (XMLEle *root, char *errmsg)
{
	IBLOBVectorProperty bv, *bvp;		/* use stack until sure */
	IBLOB *bp;
	XMLEle *e;

	/* init */
	memset (&bv, 0, sizeof(bv));

	/* crack the attributes */
	if (crackCharAtt (root, bv.device, MAXINDIDEVICE,"device", errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, bv.name, MAXINDINAME, "name", errmsg) < 0)
	    return (NULL);
	if (crackCharAtt (root, bv.label, MAXINDILABEL, "label", errmsg) < 0)
	    strcpy (bv.label, bv.name);		/* default label = name */
	if (crackCharAtt (root, bv.group, MAXINDIGROUP, "group", errmsg) < 0)
	    bv.group[0] = '\0';			/* default group is "" */
	if (crackPState (root, &bv.s, errmsg) < 0)
	    return (NULL);
	if (crackPerm (root, &bv.p, errmsg) < 0)
	    return (NULL);
	if (crackDoubleAtt (root, &bv.timeout, "timeout", errmsg) < 0)
	    bv.timeout = 0;
	if (crackCharAtt (root,bv.timestamp,MAXINDITSTAMP,"timestamp",errmsg)<0)
	    setTStampNow (bv.timestamp);

	/* crack each BLOB element */
	for (e = nextXMLEle(root,1); e != NULL; e = nextXMLEle(root,0)) {
	    if (strcmp (tagXMLEle(e), "defBLOB") == 0) {
		/* grow list, but don't count until sure */
		bv.bp = (IBLOB *) XtRealloc ((char *)bv.bp, 
						    (bv.nbp+1)*sizeof(IBLOB));
		bp = &bv.bp[bv.nbp];
		memset (bp, 0, sizeof(*bp));

		/* add each field */
		if (crackCharAtt (e, bp->name, MAXINDINAME, "name",errmsg) < 0)
		    return (NULL);
		if (crackCharAtt (e, bp->label, MAXINDILABEL,"label",errmsg)<0)
		    strcpy (bp->label, bp->name);	/* default */

		/* confident now in this BLOB */
		bv.nbp++;
	    }
	}

	/* make the persistent copy, go back and set parent pointers */
	bvp = (IBLOBVectorProperty *) memcpy (XtMalloc(sizeof(bv)), &bv,
								sizeof(bv));
	for (bp = &bvp->bp[0]; bp < &bvp->bp[bvp->nbp]; bp++)
	    bp->bvp = bvp;

	/* return malloced copy */
	return (bvp);
}

/* fill buf with properly formatted INumber string. return length */
static int
numberFormat (char *buf, const char *format, double value)
{
        int w, f, s;
	char m;

        if (sscanf (format, "%%%d.%d%c", &w, &f, &m) == 3 && m == 'm') {
            /* INDI sexi format */
            switch (f) {
            case 9:  s = 360000; break;
            case 8:  s = 36000;  break;
            case 6:  s = 3600;   break;
            case 5:  s = 600;    break;
            default: s = 60;     break;
            }
            return (fs_sexa (buf, value, w-f, s));
        } else {
            /* normal printf format */
            return (sprintf (buf, format, value));
        }
}

/* sometimes the indi window does not seem to finish its geo management, no
 * clue why. this hack just tries to goose things into action again. it helps
 * some, but still is not perfect.
 */
static void
pushDisplay()
{
	Dimension w;

	get_something (indi_w, XmNwidth, (XtArgVal)&w);
	w += 1;
	set_something (indi_w, XmNwidth, (XtArgVal)w);
	XmUpdateDisplay (indi_w);
}


/* print the boilerplate comment introducing xml */
static void
xmlv1()
{
        fprintf (swfp, "<?xml version='1.0'?>\n");
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: indimenu.c,v $ $Date: 2014/02/03 01:41:23 $ $Revision: 1.83 $ $Name:  $"};

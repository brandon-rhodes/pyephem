/* this file contains the code to put up misc messages: xe_msg().
 * we also create and use system messages dialog.
 * the latter can be toggled on/off from the main menu.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>

#include "xephem.h"


extern char maincategory[];

static void msg_create_w (void);
static void msg_erase_cb (Widget w, XtPointer client, XtPointer call);
static void msg_close_cb (Widget w, XtPointer client, XtPointer call);
static void msg_help_cb (Widget w, XtPointer client, XtPointer call);
static void msg_add (char *msg);
static void msg_scroll_down (void);
static void alert_ok_cb (Widget w, XtPointer client, XtPointer call);
static void xe_msg_alert (char *p);


#define	MINBELLSECS	3	/* min time between bell rings, seconds */

static Widget msg_w;		/* system error log widget */
static Widget alert_w;		/* system error message widget */
static Widget txt_w;		/* scrolled text widget */
static int txtl;		/* current length of text in txt_w */

/* called to force the scrolling message window to be up.
 */
void
msg_manage()
{
	if (!msg_w)
	    msg_create_w();

	XtPopup (msg_w, XtGrabNone);
	set_something (msg_w, XmNiconic, (XtArgVal)False);
	if (XtWindow(msg_w))
	    XMapRaised (XtDisplay(msg_w), XtWindow(msg_w));
}

/* ring the bellm but avoid overdoing a lot of them */
void
msg_bell()
{
	static long lastbellt;
	long t;

	t = time (NULL);

	if (t - lastbellt >= MINBELLSECS) {
	    XBell (XtDisplay(toplevel_w), 0);
	    lastbellt = t;
	}
}

/* add the printf-style fmt[] to the message list.
 * if app_alert also show in an obnoxious app alert box.
 */
void
xe_msg (int app_alert, char *fmt, ...)
{
	char msg[2048];
	va_list ap;

	/* explode */
	va_start (ap, fmt);
	vsprintf (msg, fmt, ap);
	va_end (ap);


	/* if this is the first message, create the message box */
	if (!msg_w)
	    msg_create_w();

	/* add to message list */
	msg_add (msg);

	/* if an alert, display directly too */
	if (app_alert && isUp(toplevel_w))
	    xe_msg_alert (msg);

	/* and ring the bell if turned on */
	if (pref_get (PREF_MSG_BELL) == PREF_MSGBELL)
	    msg_bell();
}

/* called to put up or remove the watch cursor.  */
void
msg_cursor (c)
Cursor c;
{
	Window win;

	if (msg_w && (win = XtWindow(msg_w)) != 0) {
	    Display *dsp = XtDisplay(msg_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}

	if (alert_w && (win = XtWindow(alert_w)) != 0) {
	    Display *dsp = XtDisplay(alert_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create the message dialog */
static void
msg_create_w()
{
	static struct {
	    char *name;
	    void (*cb)();
	    char *tip;
	} cb[] = {
	    {"Erase", msg_erase_cb, "Erase messages"},
	    {"Close", msg_close_cb, "Close this dialog"},
	    {"Help",  msg_help_cb,  "Get more information"},
	};
	Widget w, f_w;
	Arg args[20];
	int i, n;

	/* make the help shell and form */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem System log"); n++;
	XtSetArg (args[n], XmNiconName, "Sys log"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	msg_w = XtCreatePopupShell ("SysLog", topLevelShellWidgetClass,
							toplevel_w, args, n);
	set_something (msg_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (msg_w, "XEphem*SysLog.x", maincategory, 0);
	sr_reg (msg_w, "XEphem*SysLog.y", maincategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNfractionBase, 12); n++;
	f_w = XmCreateForm (msg_w, "SLF", args, n);
	XtAddCallback (f_w, XmNhelpCallback, msg_help_cb, 0);
	XtManageChild (f_w);

	/* make the control buttons */

	for (i = 0; i < XtNumber(cb); i++) {
	    n = 0;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomOffset, 10); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 1 + 4*i); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 3 + 4*i); n++;
	    w = XmCreatePushButton (f_w, cb[i].name, args, n);
	    XtAddCallback (w, XmNactivateCallback, cb[i].cb, NULL);
	    wtip (w, cb[i].tip);
	    XtManageChild (w);
	}

	/* make the scrolled text area to hold the messages */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomOffset, 10); n++;
	XtSetArg (args[n], XmNbottomWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg (args[n], XmNeditable, False); n++;
	XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	txt_w = XmCreateScrolledText (f_w, "Log", args, n);
	sr_reg (txt_w, "XEphem*SysLog*Log.columns", maincategory, 0);
	sr_reg (txt_w, "XEphem*SysLog*Log.rows", maincategory, 0);
	XtManageChild (txt_w);
}

/* callback from the erase pushbutton */
/* ARGSUSED */
static void
msg_erase_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmTextReplace (txt_w, 0, txtl, "");
	txtl = 0;
	XFlush (XtDisplay(toplevel_w));
}

/* callback from the close pushbutton */
/* ARGSUSED */
static void
msg_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (msg_w);
}

/* callback from the help pushbutton */
/* ARGSUSED */
static void
msg_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
        static char *msg[] = {
	    "System messages"
	};

	hlp_dialog ("SysLog", msg, sizeof(msg)/sizeof(msg[0]));
}

/* add msg to the txt_w widget.
 * Always set the vertical scroll bar to the extreme bottom.
 */
static void
msg_add (msg)
char *msg;
{
	int l;

	l = strlen(msg);
	if (l == 0)
	    return;

	XmTextReplace (txt_w, txtl, txtl, msg);
	txtl += l;

	if (msg[l-1] != '\n')
	    msg_add ("\n");
	else
	    msg_scroll_down();
}

/* make sure the text is scrolled to the bottom */
static void
msg_scroll_down()
{
	XmTextSetInsertionPosition (txt_w, txtl);
}

/* print a message, p, in shell message box */
static void
xe_msg_alert (p)
char *p;
{
	static Widget apmsg_w;
	Arg args[20];
	int n;

	if (!apmsg_w) {
	    Widget w;

	    /* Create shell. */
	    n = 0;
	    XtSetArg (args[n], XmNallowShellResize, True);  n++;
	    XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	    XtSetArg (args[n], XmNtitle, "xephem Alert");  n++;
	    XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	    alert_w = XtCreatePopupShell ("Alert", topLevelShellWidgetClass,
							toplevel_w, args, n);
	    setup_icon (alert_w);
	    XtAddCallback (alert_w, XmNpopupCallback, prompt_map_cb, NULL);
	    set_something (alert_w, XmNcolormap, (XtArgVal)xe_cm);

	    /* Create MessageBox . */
	    n = 0;
	    XtSetArg (args[n], XmNresizePolicy, XmRESIZE_ANY); n++;
	    XtSetArg (args[n], XmNdialogType, XmDIALOG_WARNING); n++;
	    apmsg_w = XmCreateMessageBox (alert_w, "WB", args, n);
	    XtAddCallback (apmsg_w, XmNokCallback, alert_ok_cb, NULL);
	    XtManageChild (apmsg_w);

	    w = XmMessageBoxGetChild (apmsg_w, XmDIALOG_CANCEL_BUTTON);
	    XtUnmanageChild (w);
	    w = XmMessageBoxGetChild (apmsg_w, XmDIALOG_HELP_BUTTON);
	    XtUnmanageChild (w);
	}

	set_xmstring (apmsg_w, XmNmessageString, p);

	/* show it */
	XtPopdown (alert_w);	/* so Popup causes position and raise */
	XtPopup (alert_w, XtGrabNone);
	set_something (alert_w, XmNiconic, (XtArgVal)False);

	/* make sure it's up because we are occasionally used when the main
	 * loop might not run for a while
	 */
	XmUpdateDisplay (toplevel_w);
	XSync (XtDisplay(toplevel_w), False);
}

/* callback from the alert message box ok pushbutton */
/* ARGSUSED */
static void
alert_ok_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (alert_w);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: msgmenu.c,v $ $Date: 2012/11/23 05:36:46 $ $Revision: 1.15 $ $Name:  $"};

/* this file contains the code to tigger a browser to display help information.
 * to run IE XEHELPURL to full FAT path of xephem.html, such as :
 * export XEHELPURL='d:\xephem\help\xephem.html'
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>

#include "xephem.h"

char helpcategory[] = "Help";		/* Save category */

static void hlp_create (void);
static void hlp_close_cb (Widget w, XtPointer client, XtPointer call);
static void hlp_radio_cb (Widget w, XtPointer client, XtPointer call);
static void runCmd (char *cmd);

#define	NBROWSER	7		/* browser choices we support */
typedef struct {
    Widget tb_w;			/* selection TB */
    Widget nam_w;			/* common name, defs in resources */
    Widget cmd_w;			/* command, defs in resources */
} Browser;
static Browser browser[NBROWSER];	/* entries for browser specs */
static Browser *curb;			/* pointer to current choice */

static Widget hlp_w;			/* main help shell */

static char help_instructions[] = 
    "XEphem uses a web browser to display help. Use this window to define a\n"
    "shell command that sends a URL to your preferred browser. The full URL\n"
    "will be substituted where each %s occurs.  Select from the examples or\n"
    "create a line for your browser;  they are live so you can test without\n"
    "first closing. Please email us your changes for the user-contrib page.\n";

/* trigger help for the given tag.
 * if fail so and use the deflt provided, if any.
 */
void
hlp_dialog (tag, deflt, ndeflt)
char *tag;	/* tag to look for in help file */
char *deflt[];	/* help text to use if tag not found */
int ndeflt;	/* number of strings in deflt[] */
{
	static char *hurl;
	static int triedhurl;
	char cmd[1024];
	char url[1024];
	char *fmt;
	int i, l, urll;

	if (!triedhurl) {
	    hurl = getenv ("XEHELPURL");
	    triedhurl = 1;
	}

	if (!hlp_w)
	    hlp_create ();

	if (!curb) {
	    xe_msg (1, "Please choose a browser for help");
	    return;
	}

	/* build url with tag and get length */
	if (hurl)
	    urll = sprintf (url, "%s#%s", hurl, tag);
	else
	    urll = sprintf (url, "file://%s/help/xephem.html#%s",
							    getShareDir(), tag);

	/* scan fmt and sub url for each %s */
	fmt = XmTextFieldGetString (curb->cmd_w);
	for (l = i = 0; fmt[i]; i++) {
	    if (fmt[i] == '%' && fmt[i+1] == 's') {
		strcpy (cmd+l, url);
		l += urll;
		i++;
	    } else
		cmd[l++] = fmt[i];
	}
	XtFree (fmt);

	/* run as background */
	strcpy (cmd+l, " &");
	runCmd (cmd);
}

void
hlp_config()
{
	if (!hlp_w)
	    hlp_create ();

	XtPopup (hlp_w, XtGrabNone);
	set_something (hlp_w, XmNiconic, (XtArgVal)False);
}

/* create the help configuration window (hlp_w).
 */
static void
hlp_create ()
{
	Widget w, f_w, l_w;
	Arg args[20];
	int i;
	int n;

	/* make the help shell and rc */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNiconName, "Help"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	XtSetArg (args[n], XmNtitle, "xephem Help configure"); n++;
	hlp_w = XtCreatePopupShell ("HelpConfig", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (hlp_w);
	set_something (hlp_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (hlp_w, "XEphem*HelpConfig.x", helpcategory, 0);
	sr_reg (hlp_w, "XEphem*HelpConfig.y", helpcategory, 0);
	sr_reg (hlp_w, "XEphem*HelpConfig.width", helpcategory, 0);

	/* make the main form */

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	f_w = XmCreateForm (hlp_w, "HelpF", args, n);
	XtManageChild (f_w);

	/* make the title label */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	l_w = XmCreateLabel (f_w, "HL", args, n);
	set_xmstring (l_w, XmNlabelString, help_instructions);
	XtManageChild (l_w);

	/* make each browser control */

	for (i = 0; i < NBROWSER; i++)  {
	    Browser *bp = &browser[i];
	    Widget topw = i == 0 ? l_w : bp[-1].cmd_w;
	    char name[32];

	    /* control TB on left */

	    sprintf (name, "BrowserChoice%d", i);
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, topw); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    bp->tb_w = XmCreateToggleButton (f_w, name, args, n);
	    XtAddCallback (bp->tb_w, XmNvalueChangedCallback, hlp_radio_cb, 0);
	    set_xmstring (bp->tb_w, XmNlabelString, " ");
	    wtip (bp->tb_w, "Try using this browser command");
	    sr_reg (bp->tb_w, NULL, helpcategory, 1);
	    XtManageChild (bp->tb_w);
	    if (XmToggleButtonGetState(bp->tb_w))
		curb = bp;

	    /* name */

	    sprintf (name, "BrowserName%d", i);
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, topw); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, bp->tb_w); n++;
	    XtSetArg (args[n], XmNcolumns, 10); n++;
	    bp->nam_w = XmCreateTextField (f_w, name, args, n);
	    wtip (bp->nam_w, "Browser name");
	    sr_reg (bp->nam_w, NULL, helpcategory, 1);
	    XtManageChild (bp->nam_w);

	    /* remainder is command */

	    sprintf (name, "BrowserCmd%d", i);
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, topw); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, bp->nam_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    bp->cmd_w = XmCreateTextField (f_w, name, args, n);
	    wtip (bp->cmd_w, "Browser command");
	    sr_reg (bp->cmd_w, NULL, helpcategory, 1);
	    XtManageChild (bp->cmd_w);
	}

	/* make the Close button */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, browser[NBROWSER-1].cmd_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 30); n++;
	w = XmCreatePushButton (f_w, "Close", args, n);
	wtip (w, "Close this Help window");
	XtAddCallback (w, XmNactivateCallback, hlp_close_cb, 0);
	XtManageChild (w);
}

/* called on Close */
/* ARGSUSED */
static void
hlp_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (hlp_w);
}

/* called when any TB changes */
/* ARGSUSED */
static void
hlp_radio_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int set = XmToggleButtonGetState(w);
	int i;

	if (!set)
	    return;

	/* we are coming on, save in curb and turn all others off */
	for (i = 0; i < NBROWSER; i++) {
	    if (browser[i].tb_w == w)
		curb = &browser[i];
	    else
		XmToggleButtonSetState (browser[i].tb_w, False, False);
	}
}

/* run the given shell command, return immediately.
 */
static void
runCmd (char *cmd)
{
	int i, pid;

	/* log cmd */
	xe_msg (0, "Running '%s'", cmd);

	/* launch in new shell, parent resumes */
	pid = fork();
	if (pid < 0) {
	    xe_msg (1, "fork: %s", syserrstr());
	    return;
	}
	if (pid == 0) {
	    /* child execs sh to run cmd */
	    for (i = 3; i < 100; i++)
		close (i);
	    execl ("/bin/sh", "sh", "-c", cmd, NULL);
	    _exit(1);
	}

	/* N.B. we depend on SIGCHLD/reapchildren() in main to avoid zombies */
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: helpmenu.c,v $ $Date: 2004/03/16 18:44:38 $ $Revision: 1.25 $ $Name:  $"};

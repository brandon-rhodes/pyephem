/* code to manage downloading database files off the web */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>

#include "xephem.h"

static void wdb_create (void);
static void getURL (char *url);
static void close_cb (Widget w, XtPointer client, XtPointer call);
static void help_cb (Widget w, XtPointer client, XtPointer call);
static void getast_cb (Widget w, XtPointer client, XtPointer call);
static void geturl_cb (Widget w, XtPointer client, XtPointer call);
static void ast_tcb (XtPointer client, XtIntervalId *ip);
static void ast_icb (XtPointer client, int *fdp, XtInputId *idp);

static Widget wdbshell_w;	/* the main shell */

#define	WDBNFILES	8	/* number of web files in table */
#define	WDBINDENT	20	/* table indent, pixels */
#define	POLLINT		2000	/* ast download status polling interval, ms */

/* asteroid download info */
typedef struct {
    char *name;			/* organization name */
    char *prefix;		/* prefix of files to create */
    char *script;		/* name of perl script */
    char *cmpfn;		/* name of compressed file being downloaded */
    char *expfn;		/* name of expanded after uncompressed */
    int allsz;			/* approx number of MB in all final files */
    FILE *pipefp;		/* fp from popen(), NULL when not in use */
} AstInfo;
static AstInfo astinfo[] = {
    {"Minor Planet Center", "AstMPC",    "mpcorb2edb.pl", "MPCORB.ZIP",
							"MPCORB.DAT", 160},
    {"Lowell Observatory",  "AstLowell", "astorb2edb.pl", "astorb.dat.gz",
							"astorb.dat", 220},
};
#define	NAST	XtNumber(astinfo)

static char wdbcategory[] = "Web Databases";	/* Save category */

void
wdb_manage()
{
	if (!wdbshell_w)
	    wdb_create();

        XtPopup (wdbshell_w, XtGrabNone);
	set_something (wdbshell_w, XmNiconic, (XtArgVal)False);
}

/* called to put up or remove the watch cursor.  */
void
wdb_cursor (c)
Cursor c;
{
	Window win;

	if (wdbshell_w && (win = XtWindow(wdbshell_w)) != 0) {
	    Display *dsp = XtDisplay(wdbshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

static void
wdb_create()
{
	Widget mf_w, f_w, rc_w;
	Widget pb_w, tf_w;
	Widget w;
	int i;
	char buf[1024];
	Arg args[20];
	int n;

	/* create shell and main form */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle,"xephem Web update");n++;
	XtSetArg (args[n], XmNiconName, "WebDB"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	wdbshell_w = XtCreatePopupShell ("WebDB",topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (wdbshell_w);
	set_something (wdbshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (wdbshell_w, "XEphem*WebDB.x", wdbcategory, 0);
	sr_reg (wdbshell_w, "XEphem*WebDB.y", wdbcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	mf_w = XmCreateForm (wdbshell_w, "WDBForm", args, n);
        XtAddCallback (mf_w, XmNhelpCallback, help_cb, NULL);
	XtManageChild (mf_w);

	/* everything in a RC */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNspacing, 4); n++;
	rc_w = XmCreateRowColumn (mf_w, "WRC", args, n);
	XtManageChild (rc_w);

	/* .edb section */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (rc_w, "WTL", args, n);
	set_xmstring (w, XmNlabelString, 
			    "Download a file containing .edb or TLE formats:");
	XtManageChild (w);

	for (i = 0; i < WDBNFILES; i++) {
	    n = 0;
	    XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	    f_w = XmCreateForm (rc_w, "WDTF", args, n);
	    XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, WDBINDENT); n++;
	    pb_w = XmCreatePushButton (f_w, "Get", args, n);
	    wtip (pb_w, "Download this file");
	    XtManageChild (pb_w);

	    sprintf (buf, "URL%d", i);
	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, pb_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    tf_w = XmCreateTextField (f_w, buf, args, n);
	    defaultTextFN (tf_w, 1, "", NULL);
	    wtip (tf_w, "URL of file to download (must use HTTP)");
	    XtManageChild (tf_w);
	    sprintf (buf, "XEphem*WebDB*URL%d.value", i);
	    sr_reg (tf_w, buf, wdbcategory, 1);

	    /* pass TF to PB callback */
	    XtAddCallback (pb_w, XmNactivateCallback, geturl_cb,
							    (XtPointer)tf_w);
	}

	/* gap */

	n = 0;
	w = XmCreateLabel (rc_w, " ", args, n);
	XtManageChild (w);

	/* asteroid section */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (rc_w, "WTA", args, n);
	set_xmstring (w, XmNlabelString, "Download large daily epoch asteroid data set:");
	XtManageChild (w);

	for (i = 0; i < NAST; i++) {
	    AstInfo *ap = &astinfo[i];

	    n = 0;
	    XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	    f_w = XmCreateForm (rc_w, "WAF", args, n);
	    XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, WDBINDENT); n++;
	    w = XmCreatePushButton (f_w, "Get", args, n);
	    wtip (w, "Download and reformat very large asteroid data set");
	    XtAddCallback (w, XmNactivateCallback, getast_cb, (XtPointer)ap);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    w = XmCreateLabel (f_w, "WDBML", args, n);
	    sprintf (buf, "from %s, saved in %s.edb and %s_dim.edb", ap->name,
						    ap->prefix, ap->prefix);
	    set_xmstring (w, XmNlabelString, buf);
	    XtManageChild (w);
	}

	/* controls at the bottom */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	f_w = XmCreateForm (rc_w, "WBF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 35); n++;
	    w = XmCreatePushButton (f_w, "Close", args, n);
	    XtAddCallback (w, XmNactivateCallback, close_cb, NULL);
	    wtip (w, "Close this window");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 55); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 75); n++;
	    w = XmCreatePushButton (f_w, "Help", args, n);
	    XtAddCallback (w, XmNactivateCallback, help_cb, NULL);
	    wtip (w, "Get more information about this window");
	    XtManageChild (w);
}

/* called from Close */
/* ARGSUSED */
static void
close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* outta here */
	XtPopdown (wdbshell_w);
}

/* called from any of the top Get PBs.
 * client is TF widget containing URL
 */
/* ARGSUSED */
static void
geturl_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Widget tf_w = (Widget)client;
	char *url = XmTextFieldGetString(tf_w);

	getURL (url);
	XtFree (url);
}

/* called from any Asteroid "Get".
 * client is pointer to AstInfo.
 * basic idea is run a perl script to do the work, put anything in it
 * says in the message log and monitor progress by watching file sizes.
 */
/* ARGSUSED */
static void
getast_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	AstInfo *ap = (AstInfo *)client;
	char cmd[1024];

	/* check if already trying */
	if (ap->pipefp) {
	    xe_msg (1, "%s:\nDownload is already in progress", ap->name);
	    return;
	}

	/* open a channel */
	sprintf (cmd, "cd '%s'; %s/auxil/%s -f %s", getPrivateDir(),
					getShareDir(), ap->script, ap->prefix);
	ap->pipefp = popen (cmd, "r");
	if (!ap->pipefp) {
	    xe_msg (1, "Can not start process for %s:\n%s", ap->name,
								syserrstr());
	    return;
	}

	/* log anything that comes back, done when EOF */
	XtAppAddInput(xe_app, fileno(ap->pipefp), (XtPointer)XtInputReadMask,
						    ast_icb, (XtPointer)ap);

	/* use disk space as crude progress indicator */
	pm_set (0);
	pm_up();
	XtAppAddTimeOut (xe_app, POLLINT, ast_tcb, (XtPointer)ap);
}

/* timer callback for asteroid download monitoring.
 * report progress by judging disk space change.
 * client is pointer to AstInfo.
 * repeat so long as ap->pipefp.
 */
/* ARGSUSED */
static void
ast_tcb (client, ip)
XtPointer client;
XtIntervalId *ip;
{
	AstInfo *ap = (AstInfo *)client;
	struct stat st;
	char buf[1024];
	int sz = 0;

	/* all done */
	if (!ap->pipefp)
	    return;

	sprintf (buf, "%s/%s", getPrivateDir(), ap->cmpfn);
	if (stat (buf, &st) == 0)
	    sz += st.st_size;
	sprintf (buf, "%s/%s", getPrivateDir(), ap->expfn);
	if (stat (buf, &st) == 0)
	    sz += st.st_size;
	sprintf (buf, "%s/%s.edb", getPrivateDir(), ap->prefix);
	if (stat (buf, &st) == 0)
	    sz += st.st_size;
	sprintf (buf, "%s/%s_dim.edb", getPrivateDir(), ap->prefix);
	if (stat (buf, &st) == 0)
	    sz += st.st_size;

	/* show progress and repeat */
	pm_set (sz/10000/ap->allsz);	/* MB to % */
	XtAppAddTimeOut (xe_app, POLLINT, ast_tcb, (XtPointer)ap);
}

/* Input callback whenever there is input from an asteroid perl script.
 * client is AstInfo *.
 */
static void
ast_icb (client, fdp, idp)
XtPointer client;
int *fdp;
XtInputId *idp;
{
	AstInfo *ap = (AstInfo *)client;
	int fd = *fdp;
	char buf[1024];
	int n;

	n = read (fd, buf, sizeof(buf));
	if (n < 0) {
	    /* trouble */
	    xe_msg (1, "Error from %s:\n%s", ap->name, syserrstr());
	    XtRemoveInput (*idp);
	    (void) pclose (ap->pipefp);
	    ap->pipefp = NULL;
	    pm_down();
	} else if (n == 0) {
	    /* EOF -- assume script already sent final message */
	    XtRemoveInput (*idp);
	    pclose (ap->pipefp);	/* return useless, always ECHILD */
	    ap->pipefp = NULL;
	    pm_down();
	} else {
	    /* log and contune */
	    buf[n] = '\0';
	    xe_msg (1, "%s", buf);	/* show progress, and done */
	}
}

/* called from Ok */
/* ARGSUSED */
static void
help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
        static char *msg[] = {"Download database files from the Web."};

	hlp_dialog ("WebDB", msg, XtNumber(msg));

}

/* download the given URL.
 * it may contain .edb or TLE.
 * collect what we find and save in basename(url).edb
 */
static void
getURL (url)
char *url;
{
	static char http[] = "http://";
	char buf[512], msg[1024];
	char l0[512], l1[512], l2[512];
	char *l0p = l0, *l1p = l1, *l2p = l2;
	char host[128];
	char *slash, *dot;
	char filename[256];
	FILE *fp;
	int sockfd;
	int nfound;

	/* start */
	watch_cursor(1);
	l0[0] = l1[0] = l2[0] = '\0';

	/* find transport and host */
	if (strncmp (url, http, 7)) {
	    xe_msg (1, "URL must begin with %s", http);
	    watch_cursor (0);
	    return;
	}

	slash = strchr (url+7, '/');
	dot = strrchr (url, '.');
	if (!slash || !dot) {
	    xe_msg (1, "Badly formed URL");
	    watch_cursor (0);
	    return;
	}

	/* connect to check url */
	sprintf (host, "%.*s", (int)(slash-url-7), url+7);
	sprintf (buf, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: xephem/%s\r\n\r\n",
						url, host, PATCHLEVEL);
	stopd_up();
	sockfd = httpGET (host, buf, msg);
	if (sockfd < 0) {
	    xe_msg (1, "http GET to %s failed: %s%s\n", host, buf, msg);
	    stopd_down();
	    watch_cursor (0);
	    return;
	}

	/* create local file */
	slash = strrchr (url+7, '/');
	sprintf (filename, "%s/%.*sedb", getPrivateDir(), (int)(dot-slash), slash+1);
	fp = fopen (filename, "w");
	if (!fp) {
	    xe_msg (1, "%s:\n%s", filename, syserrstr());
	    watch_cursor (0);
	    close (sockfd);
	    return;
	}

	/* copy to file, insuring only .edb lines.
	 */
	nfound = 0;
	while (recvlineb (sockfd, l2p, sizeof(l2)) > 0) {
	    char *lrot;
	    Obj o;

	    /* look lively while working */
	    XCheck (xe_app);

	    if (db_crack_line (l2p, &o, NULL, 0, msg) > 0) {
		fprintf (fp, "%s", l2p); /* retain full name */
		nfound++;
	    } else if (db_tle (l0p, l1p, l2p, &o) == 0) {
		db_write_line (&o, buf);
		fprintf (fp, "%s\n", buf);
		nfound++;
	    }

	    lrot = l0p;
	    l0p = l1p;
	    l1p = l2p;
	    l2p = lrot;
	}

	/* tidy up and done */
	fclose (fp);
	close (sockfd);
	if (!nfound) {
	    xe_msg (1, "No objects in file");
	    remove (filename);
	} else {
	    db_read (filename);
	    all_newdb(1);
	}
	stopd_down();
	watch_cursor(0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: webdbmenu.c,v $ $Date: 2012/11/23 05:22:03 $ $Revision: 1.26 $ $Name:  $"};

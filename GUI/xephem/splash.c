/* build a shell in a separate process to display info before main XEphem
 * window appears. reads messages on stdin, dies when sees eof.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/DrawingA.h>
#include <Xm/Text.h>

#include "xephem.h"

static int wantSplash (void);
static void forkSplash (int argc, char *argv[], XrmOptionDescRec options[],
    int nops);
static void createSplash (int argc, char *argv[], XrmOptionDescRec options[],
    int nops);
static void noteSplash (int on);
static void inputCB (XtPointer client, int *fdp, XtInputId *idp);
static void life_to (XtPointer client, XtIntervalId *id);
static void life_mark (Display *dsp, Window win, GC gc, int x);
static void life_exp_cb (Widget w, XtPointer client, XtPointer call);
static void onoff_cb (Widget w, XtPointer client, XtPointer call);

#define	MWW	400		/* main window width */
#define	LTO	100		/* life meter pause, ms */
#define	LMH	7		/* life meter height */
#define	LMT	3		/* life meter line thickness */
#define	NMM	5		/* number of moving markers */

static Widget stf_w;		/* message text field */
static FILE *splashpipe;	/* write here to send message */
static Widget life_w;		/* scrolling life meter */
static GC lgc;			/* life meter gc */
static Pixel lfg, lbg;		/* life meter fg and bg colors */
static Dimension llw, llh;	/* life meter size */
static char nsfn[] = "nosplash";/* nosplash flag file name */

/* perhaps start splash, depending on args and nosplash file.
 * scan argv for following, remove if found
 *   -nosplash: do not run splash and remove nsfn[]
 *   -splash: run splash and create nsfn[] 
 *   [neither]: run splash depending on whether nsfs[] exists
 */
void
splashOpen (int *argc, char *argv[], XrmOptionDescRec options[], int nops)
{
	int go = -1;
	int i;

	/* scan for -splash and -nosplash and remove if found */
	for (i = 0; i < *argc; i++) {
	    if (strcmp (argv[i], "-nosplash") == 0) {
		go = 0;
		*argc -= 1;
		memmove (&argv[i], &argv[i+1], (*argc+1) * sizeof(char *));
		noteSplash (0);
		break;
	    }
	    if (strcmp (argv[i], "-splash") == 0) {
		go = 1;
		*argc -= 1;
		memmove (&argv[i], &argv[i+1], (*argc+1) * sizeof(char *));
		noteSplash (1);
		break;
	    }
	}

/* CHAPG Alex Chupahin patched
it seems, OpenVMS 7.3-1 Motif version has a bug(?) or specific fiches (?), so splash screen is not work for a while.
I hope, this is not very importent, because the program is working nicely :), but I will work to fix it :)
*/
#ifdef VMS
	go=0;
	noteSplash (0);
#endif


	/* if neither honor flag file */
	if (go < 0)
	    go = wantSplash();

	if (go) {
	    /* copy argv so our XtAppInit can eat it sep from xephem's */
	    int myargc = *argc;
	    char **myargv = (char **) malloc ((myargc+1)*sizeof(char *));
	    memcpy (myargv, argv, (myargc+1)*sizeof(char *));
	    forkSplash(myargc, myargv, options, nops);
	    free ((char *)myargv);
	}
}

/* called by the real xephem to display a line of text on the splash window,
 * of stderr if not enabled
 */
void
splashMsg (char *fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	vfprintf (splashpipe ? splashpipe : stderr, fmt, ap);
	va_end (ap);
}

/* called by the real xephem to close the splash window */
void
splashClose ()
{
	if (!splashpipe)
	    return;

	fclose (splashpipe);
	splashpipe = NULL;
}

/* create the splash GUI.
 * we already a separate process
 */
static void
createSplash (int argc, char *argv[], XrmOptionDescRec options[], int nops)
{
	XrmDatabase dspdb;
	Widget l_w, rc_w, nsrc_w, ns_w;
	char t[1024];
	Arg args[20];
	int sw;
	int n;

	/* create splash shell */

	newEnv(&argc, argv);

	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Splash"); n++;
	XtSetArg (args[n], XmNy, 200); n++;
	XtSetArg (args[n], XmNwidth, MWW); n++;
	XtSetArg (args[n], XmNoverrideRedirect, True); n++;
	XtSetArg (args[n], XmNborderWidth, 2); n++;
	toplevel_w = XtAppInitialize (&xe_app, myclass, options, nops,
					    &argc, argv, fallbacks, args, n);

	/* get XEphem resources */

	dspdb = XtDatabase (XtDisplay(toplevel_w));
	XrmCombineFileDatabase("/etc/XEphem", &dspdb, True);
	XrmCombineFileDatabase(userResFile(), &dspdb, True);

	/* set private colormap if desired */

	xe_cm = checkCM (DefaultColormap(XtD,DefaultScreen(XtD)), 20);
	set_something (toplevel_w, XmNcolormap, (XtArgVal)xe_cm);

	/* center on screen */

	sw = WidthOfScreen (XtScreen(toplevel_w));
	set_something (toplevel_w, XmNx, (XtArgVal)((sw-MWW)/2));

	/* main manager is a rowcolumn */

	n = 0;
	XtSetArg (args[n], XmNisAligned, False); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNspacing, 10); n++;
	rc_w = XmCreateRowColumn (toplevel_w, "Splash", args, n);
	XtManageChild (rc_w);

	/* title */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	l_w = XmCreateLabel (rc_w, "title", args, n);
	set_xmstring (l_w, XmNlabelString,
				    "XEphem ... from Clear Sky Institute Inc");
	XtManageChild (l_w);

	make_logo (rc_w);

	/* subtitle */

	n = 0;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	l_w = XmCreateLabel (rc_w, "subtitle", args, n);
	sprintf (t,"Version %s -- released %s\n%s", PATCHLEVEL, PATCHDATE,
								    COPYRIGHT);
	set_xmstring (l_w, XmNlabelString, t);
	XtManageChild (l_w);

	/* add drawing area for life meter, with timer */

	n = 0;
	XtSetArg (args[n], XmNheight, LMH); n++;
	life_w = XmCreateDrawingArea (rc_w, "DA", args, n);
	XtAddCallback (life_w, XmNexposeCallback, life_exp_cb, NULL);
	XtManageChild (life_w);

	/* add text widget for progress messages */

	n = 0;
	XtSetArg (args[n], XmNrows, 15); n++;
	XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	XtSetArg (args[n], XmNblinkRate, 0); n++;
	stf_w = XmCreateText (rc_w, "messages", args, n);
	XtManageChild (stf_w);

	/* add a disable PB -- RC is to prevent it being full width */

	n = 0;
	XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	nsrc_w = XmCreateRowColumn (rc_w, "NSRC", args, n);
	XtManageChild (nsrc_w);

	    n = 0;
	    ns_w = XmCreateToggleButton (nsrc_w, "NS", args, n);
	    XtAddCallback (ns_w, XmNvalueChangedCallback, onoff_cb, NULL);
	    set_xmstring (ns_w, XmNlabelString,"Disable this splash next time");
	    XtManageChild (ns_w);

	/* connect what real xephem writes to splashpipe to our stdin */

	XtAppAddInput (xe_app, 0, (XtPointer)XtInputReadMask, inputCB, NULL);

	/* go */

	XtRealizeWidget(toplevel_w);
	XtAppMainLoop(xe_app);
}

/* called whenever input arrives from the real xephem. exit on eof */
static void
inputCB (XtPointer client, int *fdp, XtInputId *idp)
{
	char buf[1024];
	int nr;

	nr = read (*fdp, buf, sizeof(buf)-1);
	if (nr <= 0)
	    exit(0);

	buf[nr] = '\0';
	XmTextInsert (stf_w, XmTextGetLastPosition(stf_w), buf);
}

/* called when life meter gets exp -- use to init and start heartbeat */
static void
life_exp_cb (Widget w, XtPointer client, XtPointer call)
{
	Display *dsp = XtDisplay(w);
	Window win = XtWindow(w);
	Arg args[20];
	int n;

	n = 0;
	XtSetArg (args[n], XmNforeground, &lfg); n++;
	XtSetArg (args[n], XmNbackground, &lbg); n++;
	XtGetValues (stf_w, args, n);

	XtSetArg (args[n], XmNwidth, &llw); n++;
	XtSetArg (args[n], XmNheight, &llh); n++;
	XtGetValues (life_w, args, n);

	if (!lgc) {
	    /* just once: build GC, init, start timer */
	    lgc = XCreateGC (dsp, win, 0L, NULL);
	    XSetLineAttributes (dsp, lgc, LMT, LineSolid, CapRound, JoinRound);

	    XSetForeground (dsp, lgc, lfg);
	    for (n = -LMT*2; n < llw; n += LMT*2)
		life_mark (dsp, win, lgc, n);

	    XtAppAddTimeOut (xe_app, LTO, life_to, (XtPointer)1);
	}
}

/* called regularly to show activity */
static void
life_to (XtPointer client, XtIntervalId *id)
{
	static int lmx;
	Display *dsp = XtDisplay(life_w);
	Window win = XtWindow(life_w);
	int gap = (LMT*2)*((llw/NMM)/(LMT*2));
	int i;

	/* move forward, wrap */
	if ((lmx += LMT*2) > llw + LMT*2)
	    lmx = 0;

	/* point markers every gap, each way of lmx, let X clip */
	for (i = 0; i < NMM*2; i++) {
	    int dx = (i - NMM)*gap;

	    /* draw new */
	    XSetForeground (dsp, lgc, lbg);
	    life_mark (dsp, win, lgc, lmx + dx);

	    /* erase prev */
	    XSetForeground (dsp, lgc, lfg);
	    life_mark (dsp, win, lgc, lmx + dx - LMT*2);
	}

	XSync (dsp, True);

	/* repeat */
	XtAppAddTimeOut (xe_app, LTO, life_to, NULL);
}

/* draw one life marker pip */
static void
life_mark (Display *dsp, Window win, GC gc, int x)
{
	XDrawLine (dsp, win, gc, x+llh-LMT/2, LMT/2, x+LMT/2, llh-LMT/2-1);
}

/* create the splash process, connect splashpipe to its stdin */
static void
forkSplash (int argc, char *argv[], XrmOptionDescRec options[], int nops)
{
	int p[2];

	if (pipe(p) < 0)
	    exit(0);
	if (fork() == 0) {
	    dup2 (p[0], 0);
	    close (p[0]);
	    close (p[1]);
	    createSplash(argc, argv, options, nops);
	    abort();
	}
	close (p[0]);
	splashpipe = fdopen (p[1], "w");
	setbuf (splashpipe, NULL);
}

/* called to toggle splash screen now and forever */
static void
onoff_cb (Widget w, XtPointer client, XtPointer call)
{
	noteSplash (!XmToggleButtonGetState(w));
}

/* record in flag file whether we want splash */
static void
noteSplash (int on)
{
	char nos[1024];
	FILE *fp;

	sprintf (nos, "%s/%s", getPrivateDir(), nsfn);

	if (on) {
	    remove (nos);
	} else {
	    fp = fopen (nos, "w");
	    if (fp)
		fclose (fp);
	}
}

/* return 1 if flag file indicates we want splash else 0 */
static int
wantSplash()
{
	char nos[1024];
	FILE *fp;

	sprintf (nos, "%s/%s", getPrivateDir(), nsfn);
	fp = fopen (nos, "r");
	if (fp)
	    fclose (fp);
	return (!fp);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: splash.c,v $ $Date: 2006/04/02 12:56:47 $ $Revision: 1.27 $ $Name:  $"};

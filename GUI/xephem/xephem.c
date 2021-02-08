/* main() for xephem.
 * Copyright (c) 1990-2013 by Elwood Charles Downey
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#if defined(__NUTC__)
#include <winnutc.h>
#endif

#include <X11/Xlib.h>
#include <X11/IntrinsicP.h> /* for XT_REVISION */
#include <X11/cursorfont.h>

/* define WANT_EDITRES if want to try and support X11R5's EditRes feature.
 * this will require linking with -lXmu and -lXext too.
#define WANT_EDITRES
 */
#if defined(WANT_EDITRES) && (XT_REVISION >= 5)
#define	DO_EDITRES
#endif

#ifdef DO_EDITRES
#include <X11/Xmu/Editres.h>
#endif

#include <Xm/Xm.h>
#include <X11/Shell.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>

#if XmVersion >= 1002
#include <Xm/RepType.h>
#endif /* XmVersion >= 1002 */


#include "xephem.h"

extern char Version_resource[];
extern char maincategory[];

#define	NMINCOL	150	/* min colors we want for the main colormap */

/* these are used to describe and semi-automate making the main pulldown menus
 */
typedef struct {
    char *tip;		/* tip text, if any */
    char *name;		/* button name, or separator name if !cb */
    			/* N.B. watch for a few special cases */
    char *label;	/* button label (use name if 0) */
    char *acc;		/* button accelerator, if any */
    char *acctext;	/* button accelerator text, if any */
    char mne;		/* button mnemonic */
    void (*cb)();	/* button callback, or NULL if none */
    XtPointer client;	/* button callback client data */
} ButtonInfo;
typedef struct {
    char *tip;		/* tip text, if any */
    char *cb_name;	/* cascade button name */
    char *cb_label;	/* cascade button label (use name if 0) */
    char cb_mne;	/* cascade button mnemonic */
    char *pd_name;	/* pulldown menu name */
    ButtonInfo *bip;	/* array of ButtonInfos, one per button in pulldown */
    int nbip;		/* number of entries in bip[] */
} PullDownMenu;

#define	SEPARATOR_N	"MainSep"	/* special ButtonInfo->name */


static void addOurDBs (void);
static void chk_args (int argc, char *argv[]);
static void chk_pos (void);
static void chk_version (void);
static void set_title (void);
static void make_main_window (void);
static Widget make_pulldown (Widget mb_w, PullDownMenu *pdmp);
static void setup_sigs (void);
static void m_activate_cb (Widget w, XtPointer client, XtPointer call);
static void x_quit (void);
static void initialUps(void);


/* client arg to m_activate_cb().
 */
typedef enum {
    PROGRESS, QUIT, GALLERY, MSGTXT, NETSETUP, EXTIN,
    SUNV, DATA, MOONV, EARTH, MARSV, JUPMOON, SATMOON, UMOON, SKYVIEW,
    SOLARSYS, PLOT, LIST, SEARCH, GLANCE, AAVSO, CCONV, OBSLOG,
    EDB, OBJS, CLOSEOBJS, FAVS, FIELDSTARS, WEBDB, MOVIELOOP,
    ABOUT, REFERENCES, CONFIGHLP, INTRO, MAINMENU, FILEHLP, VIEWHLP, TOOLSHLP,
        OBJHLP, PREFHLP, OPERATION, CONTEXTHLP, SRCHHLP, DATETIME, NOTES
} MBOptions;

Widget toplevel_w;
#define	XtD	XtDisplay(toplevel_w)
Colormap xe_cm;
XtAppContext xe_app;
char myclass[] = "XEphem";

#define xephem_width 32
#define xephem_height 32
static unsigned char xephem_bits[] = {
   0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55, 0xfe, 0xff, 0xff, 0xbf,
   0xfd, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0x7f,
   0xfe, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0xbf,
   0xfd, 0xff, 0xff, 0x7f, 0x1e, 0x8e, 0x01, 0xbc, 0x3d, 0xc4, 0x03, 0x7c,
   0x7e, 0xe0, 0xe3, 0xbd, 0x7d, 0xe0, 0xe3, 0x7f, 0xfe, 0xf0, 0x63, 0xbf,
   0xfd, 0xf0, 0x03, 0x7f, 0xfe, 0xf0, 0x03, 0xbf, 0xfd, 0xf0, 0x63, 0x7f,
   0x7e, 0xe0, 0xe3, 0xbf, 0x7d, 0xe0, 0xe3, 0x7d, 0x3e, 0xc2, 0x03, 0xbc,
   0x1d, 0x87, 0x01, 0x7c, 0xfe, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0x7f,
   0xfe, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0xbf,
   0xfd, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0x7f,
   0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55};

static XrmOptionDescRec options[] = {
    {"--help",   ".help",    XrmoptionIsArg, NULL},
    {"-help",    ".help",    XrmoptionIsArg, NULL},
    {"-install", ".install", XrmoptionSepArg, NULL},
    {"-log",     ".log",     XrmoptionIsArg, NULL},
    {"-nowin",   ".nowin",   XrmoptionIsArg, NULL},
    {"-prfb",    ".prfb",    XrmoptionIsArg, NULL},
    {"-resfile", ".resfile", XrmoptionSepArg, NULL},
};

int
main(argc, argv)
int argc;
char *argv[];
{
	Arg args[10];
	int n;

	/* set up signal handling */
	setup_sigs();

	/* check for -nosplash or nosplash file .. can't wait for resources */
	splashOpen (&argc, argv, options, XtNumber(options));

#ifdef __APPLE__
	/* hack around two-level namespace linker problem in libXt wrt libXm.
	 * first appeared in XFree86 4.2.0 build; should remain harmless in
	 * subsequent builds even if the problem is fixed.
	 */
	{
	    extern void *topLevelShellClassRec;
	    extern void *transientShellClassRec;
	    extern void *vendorShellClassRec;
	    topLevelShellClassRec  = (void *) &vendorShellClassRec;
	    transientShellClassRec = (void *) &vendorShellClassRec;
	}
#endif

	/* check for alternate env before starting toolkit.
	 * (don't even ask why this is here)
	 */
	newEnv(&argc, argv);

	/* set this before using fallbacks[] */
	(void)sprintf (Version_resource,"%s.Version: %s", myclass, PATCHLEVEL);

	/* open display and gather standard resources.
	 * we grab fallbacks[] last
	 */
	splashMsg ("Building shell\n");
	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	toplevel_w = XtAppInitialize (&xe_app, myclass, options,
			    XtNumber(options), &argc, argv, NULL, args, n);

	/* add our resources from non-standard places */
	splashMsg ("Reading X resources\n");
	addOurDBs();

	splashMsg ("Checking args\n");
	chk_args (argc, argv);
	chk_version();
	set_title();

	/* load xe_cm and toplevel_w with default or private colormap */
	splashMsg ("Checking colormap\n");
	xe_cm = checkCM (DefaultColormap(XtD,DefaultScreen(XtD)), NMINCOL);
	set_something (toplevel_w, XmNcolormap, (XtArgVal)xe_cm);

#ifdef DO_EDITRES
	XtAddEventHandler (toplevel_w, (EventMask)0, True,
					_XEditResCheckMessages, NULL);
#endif

#if WANT_TEAR_OFF
	/* install converter so tearOffModel can be set in resource files
	 * to TEAR_OFF_{EN,DIS}ABLED.
	 */
	XmRepTypeInstallTearOffModelConverter();
#endif

	/* connect up the icon pixmap */
	splashMsg ("Building icon\n");
	setup_icon (toplevel_w);

	/* make the main menu bar and form (other stuff is in mainmenu.c) */
	splashMsg ("Building main window system\n");
	make_main_window ();

	/* set up networking, but don't manage it here */
	splashMsg ("Init networking\n");
	net_create();

	/* start in night mode? */
	splashMsg ("Checking night mode\n");
	sr_chknightv();

	/* install then monitor position */
	chk_pos();

	/* here we go */
	splashMsg ("Realizing top widget\n");
	XtRealizeWidget(toplevel_w);
	splashClose();
	if (!getXRes ("nowin", NULL))
	    initialUps();
	XtAppMainLoop(xe_app);

	printf ("XtAppMainLoop returned :-)\n");
	return (1);
}

/* called to put up or remove the watch cursor.  */
void
main_cursor (c)
Cursor c;
{
	Window win;

	if (toplevel_w && (win = XtWindow(toplevel_w)) != 0) {
	    Display *dsp = XtD;
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* look for -env arg.
 * if find replace as specified and remove from argv list.
 */
void
newEnv (int *argcp, char *argv[])
{
	int argc = *argcp;
	int i;

	/* look for any/all -env */
	for (i = 1; i < argc; i++) {
	    if (strcmp(argv[i], "-env") == 0) {
		char *argeq, *newenv;
		int n;

		/* new value is in next arg */
		if (i == argc-1) {
		    printf ("-env requires another arg\n");
		    abort();
		}
		newenv = argv[++i];

		argeq = strchr (newenv, '=');
		if (!argeq) {
		    printf ("%s: -env requires X=Y\n", newenv);
		    abort();
		}

#if defined(__NUTC__)
		{
		    /* convert filenames to internal format.
		     * N.B. assumes exactly 1 file name
		     */
		    char *cp, *cpeq;

		    cp = strcpy (malloc (PATH_MAX+1), newenv);
		    cpeq = strchr (cp, '=');
		    (void) _NutPathToNutc (argeq+1, cpeq+1, 1);
		    newenv = cp;
		}
#endif
		/* install new env */
		putenv (newenv);

		/* remove from argv */
		(*argcp) -= 2;
		for (n = i+1; n < argc; n++)
		    argv[n-2] = argv[n];
	    }
	}

	argv[*argcp] = NULL;
}

/* connect up logo.gif */
void
make_logo (Widget rc)
{
	Display *dsp = XtDisplay(toplevel_w);
	char fn[1024];
	unsigned char gif[200000];
	char why[1024];
	Arg args[20];
	Widget f_w, l_w;
	Pixmap pm;
	FILE *fp;
	int w, h;
	int ngif;
	int n;

	/* open and read the gif */
	(void) sprintf (fn, "%s/auxil/logo.gif",  getShareDir());
	fp = fopenh (fn, "rb");
	if (!fp) {
	    (void) fprintf (stderr, "%s: %s\n", fn, syserrstr());
	    return; /* darn */
	}
	ngif = fread (gif, 1, sizeof(gif), fp);
	fclose (fp);
	if (ngif < 0) {
	    (void) fprintf (stderr, "%s: %s\n", fn, syserrstr());
	    return; /* darn again */
	}
	if (ngif == sizeof(gif)) {
	    (void) fprintf (stderr, "%s: Max size = %d\n", fn,(int)sizeof(gif));
	    return; /* darn again */
	}

	/* convert to pm */
	if (gif2pm (dsp, xe_cm, gif, ngif, &w, &h, &pm, why) < 0) {
	    fprintf (stderr, "%s: %s\n", fn, why);
	    return; /* darn again */
	}

	/* put pm in a label and let it handle it from now on.
	 * put label in a form so it stretches clear across the rc
	 */
	n = 0;
	f_w = XmCreateForm (rc, "LogoForm", args, n);
	XtManageChild (f_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNlabelType, XmPIXMAP); n++;
	XtSetArg (args[n], XmNlabelPixmap, pm); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	l_w = XmCreateLabel (f_w, "LogoLabel", args, n);
	wtip (l_w, "The file <XEphem.ShareDir>/auxil/logo.gif");
	XtManageChild (l_w);
}

/* add the xephem icon to shell widget w */
void
setup_icon (Widget w)
{
	static Pixmap pm;

	if (!pm) {
	    Display *dsp = XtDisplay (w);
	    Window win = RootWindow (dsp, DefaultScreen(dsp));
	    pm = XCreateBitmapFromData (dsp, win, (char *)xephem_bits,
						xephem_width, xephem_height);
	}

	set_something (w, XmNiconPixmap, (XtArgVal)pm);
	set_something (w, XmNiconMask, (XtArgVal)pm);
}

/* merge more resource files into final db; harmless if missing.
 * finally add any fallbacks[] not already in db.
 */
static void
addOurDBs()
{
	XrmDatabase dspdb = XtDatabase (XtD);
	XrmDatabase fbdb = NULL;
	char **pp, *p;

	/* check in TELHOME */
	if ((p = getenv("TELHOME")) != NULL) {
	    char fullp[256];
	    sprintf (fullp, "%s/archive/config/XEphem", p);
	    XrmCombineFileDatabase(fullp, &dspdb, True);
	}

	/* then non-standard system-wide */
	XrmCombineFileDatabase("/etc/XEphem", &dspdb, True);

	/* then per-user's so it has max priority */
	XrmCombineFileDatabase(userResFile(), &dspdb, True);

	/* finally add any fallbacks[] not already known */
	for (pp = fallbacks; (p = *pp++) != NULL; )
	    XrmPutLineResource (&fbdb, p);	/* creates fbdb if first */
	XrmCombineDatabase (fbdb, &dspdb, False);

	/* Combine evidently uses pointers.. we die if we destroy fbdb
	XrmDestroyDatabase (fbdb);
	*/
}


/* ARGSUSED */
static void
chk_args (argc, argv)
int argc;
char *argv[];
{

	if (getXRes ("log", NULL)) {
	    /* trouble output, if any, goes to log flie */
	    char buf[1024];
	    sprintf (buf, "%s/exitlog.txt", getPrivateDir());
	    freopen (buf, "at", stdout);
	    freopen (buf, "at", stderr);
	}

	if (getXRes ("prfb", NULL)) {
	    String *fbp = fallbacks;
	    for (fbp = fallbacks; *fbp != NULL; fbp++)
		printf ("%s\n", *fbp);
	    exit (0);
	}

	if (argc > 1 || getXRes ("help", NULL)) {
	    fprintf (stderr, "Usage: xephem [options]\n");
	    fprintf (stderr, "Purpose: interactive astronomical ephemeris.\n");
	    fprintf (stderr, "Version: %s Build: %s\n", PATCHLEVEL, PATCHDATE);
	    fprintf (stderr, "Options:\n");
	    fprintf (stderr, "  -env N=V  : set alternate env Name=Value\n");
	    fprintf (stderr, "  -help     : print this message then exit\n");
	    fprintf (stderr, "  -install {yes|no|guess}: whether to install a private colormap\n");
	    fprintf (stderr, "  -log      : save exit trouble to private log\n");
	    fprintf (stderr, "  -nosplash : disable splash screen\n");
	    fprintf (stderr, "  -nowin    : do not restore windows\n");
	    fprintf (stderr, "  -prfb     : print all fallback resources then exit\n");
	    fprintf (stderr, "  -resfile f: load alternate resource file, f\n");
	    fprintf (stderr, "  -splash   : enable splash screen\n");
	    exit (0);
	}
}

/* support position of main window in our preferences system */
static void
chk_pos()
{
	Position x = (Position)atoi(getXRes ("x", "100"));	/* XEphem.x */
	Position y = (Position)atoi(getXRes ("y", "100"));	/* XEphem.y */

	set_something (toplevel_w, XmNx, (XtArgVal)x);
	set_something (toplevel_w, XmNy, (XtArgVal)y);

	sr_reg (toplevel_w, "XEphem.x", maincategory, 0);
	sr_reg (toplevel_w, "XEphem.y", maincategory, 0);
}


/* insure that resource version and (sometimes) X server matches. */
static void
chk_version()
{
	char *v = getXRes ("Version", "??");

	if (strcmp (v, PATCHLEVEL)) {
	    printf ("Version skew: Found %s but should be %s\n", v, PATCHLEVEL);
	    abort();
	}
}

static void
set_title()
{
	char title[100];

	(void) sprintf (title, "XEphem %s", PATCHLEVEL);
	set_something (toplevel_w, XmNtitle, (XtArgVal) title);
}

/* put together the menu bar, the main form, and fill in the form with the
 * initial xephem buttons.
 */
static void
make_main_window ()
{
	static ButtonInfo file_buttons[] = {
	    {"Display a dialog containing supporting informational messages",
	        "SysLog", "System log...", 0, 0, 'l', m_activate_cb,
							    (XtPointer)MSGTXT},
	    {"Display pictures from the Gallery collection",
	        "Gallery", "Gallery...", 0, 0, 'G', m_activate_cb,
							    (XtPointer)GALLERY},
	    {"Setup networking options", "Net", "Network setup...", 0, 0,
				    'N', m_activate_cb, (XtPointer)NETSETUP},
	    {"Set up to run xephem Time and Location from an external file",
		"ExtIn", "External file...", 0, 0, 'E', m_activate_cb,
							    (XtPointer)EXTIN},
	    {"Display a simple Progress meter",
		"Progress", "Progress Meter...", 0, 0, 'P', m_activate_cb,
							   (XtPointer)PROGRESS},
	    {NULL, SEPARATOR_N},
	    {"Run 1 Step forward",
		"Forward1", "Forward 1 Step", "Ctrl<Key>f", "Ctrl+f", 'F',
						    mm_go_cb, (XtPointer)1},
	    {"Run 1 Step backward",
		"Backward1", "Backward 1 Step", "Ctrl<Key>b", "Ctrl+b", 'B',
						    mm_go_cb, (XtPointer)-1},
	    {"Run or stop the main execution loop",
		"Update", "Update", "Ctrl<Key>u", "Ctrl+u", 'U',
						    mm_go_cb, (XtPointer)0},
	    {NULL, SEPARATOR_N},
	    {"Exit xephem",
		"Quit", "Quit...", "Ctrl<Key>d", "Ctrl+d", 'Q', m_activate_cb,
							    (XtPointer)QUIT}
	};
	static ButtonInfo view_buttons[] = {
	    {"Display many statistics for any objects",
		"GenData", "Data Table...", 0, 0, 'D', m_activate_cb,
							    (XtPointer)DATA},
	    {NULL, SEPARATOR_N},
	    {"Display an image of the Sun and supporting information",
		"Sun", "Sun...", 0, 0, 'S', m_activate_cb, (XtPointer)SUNV},
	    {"Display an image of the Moon and supporting information",
		"Moon", "Moon...", 0, 0, 'M', m_activate_cb, (XtPointer)MOONV},
	    {"Display a map of Earth and supporting information",
		"Earth", "Earth...", 0, 0, 'E', m_activate_cb,(XtPointer)EARTH},
	    {"Display an image of Mars and supporting information",
		"Mars", "Mars...", 0, 0, 'r', m_activate_cb, (XtPointer)MARSV},
	    {"Display a schematic of Jupiter, GRS, its moons, and other info",
		"Jupiter", "Jupiter...", 0, 0, 'J', m_activate_cb,
							(XtPointer)JUPMOON},
	    {"Display schematic of Saturn, its rings and moons, and other info",
		"Saturn", "Saturn...", 0, 0, 'a', m_activate_cb,
							(XtPointer)SATMOON},
	    {"Display schematic of Uranus, its moons, and other info",
		"Uranus", "Uranus...", 0, 0, 'U', m_activate_cb,
							(XtPointer)UMOON},
	    {NULL, SEPARATOR_N},
	    {"Display a full-featured map of the night sky",
		"SkyV", "Sky View...", 0, 0, 'V', m_activate_cb,
							    (XtPointer)SKYVIEW},
	    {"Display a map of the solar system",
		"SolSys", "Solar System...", 0, 0, 'S', m_activate_cb,
	    						(XtPointer)SOLARSYS}
	};
	static ButtonInfo ctrl_buttons[] = {
	    {"Capture any XEphem values for making and displaying plots",
		"Plot", "Plot values...", 0, 0, 'P', m_activate_cb,
							    (XtPointer)PLOT},
	    {"Capture any XEphem values in a tabular text file",
		"List", "List values...", 0, 0, 'L', m_activate_cb,
							    (XtPointer)LIST},
	    {"Define an equation of XEphem fields and solve for min, max or 0",
		"Solve", "Solve equation...", 0, 0, 'S', m_activate_cb,
							    (XtPointer)SEARCH},
	    {"Find all pairs of close objects in memory",
		"CloseObjs", "Find close pairs...", 0, 0, 'F',
					m_activate_cb, (XtPointer)CLOSEOBJS},
	    {"Show twilight and all basic objects on a 24 hour timeline map",
		"Glance", "Night at a glance...", 0, 0, 'N',
					    m_activate_cb, (XtPointer)GLANCE},
#ifdef AAVSO
	    {"Fetch light curves from AAVSO",
		"AAVSO", "AAVSO light curves...", 0, 0, 'A', m_activate_cb,
							    (XtPointer)AAVSO},
#endif
	    {"Sky coordinates converter",
		"Coordinates converter", "Coordinates converter...",0,0,'C',
					    m_activate_cb, (XtPointer)CCONV},
	    {"Manage Observers log", "ObsLog", "Observers log book...",0,0,'O',
					    m_activate_cb, (XtPointer)OBSLOG},
	    {"Movie loop control", "MLoop", "Movie loop...", 0, 0,'M',
					m_activate_cb, (XtPointer)MOVIELOOP},
	};
	static ButtonInfo objs_buttons[] = {
	    {"Load and delete .edb files to and from memory",
		"Files", "Files...", 0, 0, 'L', m_activate_cb, (XtPointer)EDB},
	    {"Search and inspect loaded objects", "Index", "Index...", 0,
					0,'S', m_activate_cb, (XtPointer)OBJS},
	    {"Manage list of favorites", "Favorites", "Favorites...", 0, 0,'F',
					    m_activate_cb, (XtPointer)FAVS},
	    {"Download .edb or .tle files from web",
		"WebDB", "Download...", 0, 0, 'I',
					m_activate_cb, (XtPointer)WEBDB},
	    {"Setup field star options and catalog locations",
		"FieldStars", "Field stars...", 0, 0, 'C',
					m_activate_cb, (XtPointer)FIELDSTARS},
	};
	static ButtonInfo help_buttons[] = {
	    {"Configure Help system find a browser",
		"Configure", "Configure help...", 0, 0, 'C', m_activate_cb,
							(XtPointer)CONFIGHLP},
	    {"Click here then roam over controls to see help tips",
		"onContext", "on Context", 0, 0, 'x', m_activate_cb,
							(XtPointer)CONTEXTHLP},
	    {"Overall features of xephem",
		"Introduction", "Introduction...", 0, 0, 'I', m_activate_cb,
							    (XtPointer)INTRO},
	    {"How to control xephem's running behavior, including looping",
		"onOperation", "on Operation...", 0, 0, 'e', m_activate_cb,
							(XtPointer)OPERATION},
	    {"Shortcuts to setting date and time formats",
		"onTriad", "on Triad formats...", 0, 0, 'f', m_activate_cb,
							(XtPointer)DATETIME},
	    {NULL, SEPARATOR_N},
	    {"Description of the overall Main xephem menu",
		"onMainMenu", "on Main Window...", 0, 0, 'M', m_activate_cb,
							(XtPointer)MAINMENU},
	    {"Description of the options available under File",
		"onFile", "on File...", 0, 0, 'F', m_activate_cb,
							    (XtPointer)FILEHLP},
	    {"Description of the options available under View",
		"onView", "on View...", 0, 0, 'V', m_activate_cb,
							    (XtPointer)VIEWHLP},
	    {"Description of the options available under Tools",
		"onTools", "on Tools...", 0, 0, 'T', m_activate_cb,
							(XtPointer)TOOLSHLP},
	    {"Description of the options available under Data",
		"onObjects", "on Data...", 0, 0, 'D', m_activate_cb,
							(XtPointer)OBJHLP},
	    {"Description of the options available under Preferences",
		"onPrefs", "on Preferences...", 0, 0, 'P', m_activate_cb,
							(XtPointer)PREFHLP},
	    {NULL, SEPARATOR_N},
	    {"Credits, references and other kudos.",
		"onReferences", "Credits...",  0, 0, 'C', m_activate_cb,
							(XtPointer)REFERENCES},
	    {"A few supporting issues",
		"Notes", "Notes...", 0, 0, 'N', m_activate_cb,(XtPointer)NOTES},
	    {"Version, copyright, fun graphic",
		"About", "About...", 0, 0, 'A', m_activate_cb, (XtPointer)ABOUT}
	};
	static PullDownMenu file_pd =
	    {"Overall control functions",
		"File", 0, 'F', "file_pd", file_buttons,XtNumber(file_buttons)};
	static PullDownMenu view_pd =
	    {"Major display options",
		"View", 0, 'V', "view_pd", view_buttons,XtNumber(view_buttons)};
	static PullDownMenu ctrl_pd =
	    {"Supporting analysis tools",
		"Tools", 0,'T',"ctrl_pd",ctrl_buttons,XtNumber(ctrl_buttons)};
	static PullDownMenu objs_pd =
	    {"Add, delete and inspect objects in memory",
		"Data",0,'D',"objs_pd",objs_buttons,XtNumber(objs_buttons)};
	static PullDownMenu help_pd =
	    {"Additional information about xephem and the Main display",
		"Help", 0, 'H', "help_pd", help_buttons,XtNumber(help_buttons)};
	    
	Widget mainrc;
	Widget mb_w;
	Widget cb_w;
	Arg args[20];
	int n;

	/*	Create main window as a vertical r/c  */
	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	XtSetArg (args[n], XmNmarginWidth, 0); n++;
	mainrc = XmCreateRowColumn (toplevel_w, "XephemMain", args, n);
	XtAddCallback (mainrc, XmNhelpCallback, m_activate_cb,
							(XtPointer)MAINMENU);
	XtManageChild (mainrc);

	/* connect actions */

	splashMsg ("Connecting actions\n");
	mm_connActions ();

	/*	Create MenuBar in mainrc  */

	n = 0;
	splashMsg ("Building main menu bar\n");
	mb_w = XmCreateMenuBar (mainrc, "MB", args, n); 
	XtManageChild (mb_w);

	    /* create each pulldown */

	    (void) make_pulldown (mb_w, &file_pd);
	    (void) make_pulldown (mb_w, &view_pd);
	    (void) make_pulldown (mb_w, &ctrl_pd);
	    (void) make_pulldown (mb_w, &objs_pd);
	    pref_create_pulldown (mb_w);
	    cb_w = make_pulldown (mb_w, &help_pd);

	    n = 0;
	    XtSetArg (args[n], XmNmenuHelpWidget, cb_w);  n++;
	    XtSetValues (mb_w, args, n);

	/* make a spot for the logo */
	splashMsg ("Building logo\n");
	make_logo (mainrc);

	/* add the remainder of the main window */

	mm_create (mainrc);
}

/* create/manage a cascade button with a pulldown menu off a menu bar.
 * return the cascade button.
 * N.B. watch for special bip->name.
 */
static Widget
make_pulldown (mb_w, pdmp)
Widget mb_w;
PullDownMenu *pdmp;
{
	Widget pulldown_w;
	Widget button;
	Widget cascade;
	XmString accstr, labstr;
	Arg args[20];
	int n;
	int i;

	/* make the pulldown menu */

	n = 0;
	pulldown_w = XmCreatePulldownMenu (mb_w, pdmp->pd_name, args, n);

	/* fill it with buttons and/or separators */

	for (i = 0; i < pdmp->nbip; i++) {
	    ButtonInfo *bip = &pdmp->bip[i];
	    int separator = !strcmp (bip->name, SEPARATOR_N);

	    if (separator) {
		Widget s = XmCreateSeparator (pulldown_w, bip->name, args, n);
		XtManageChild (s);
		continue;
	    }

	    accstr = NULL;
	    labstr = NULL;

	    n = 0;
	    if (bip->acctext && bip->acc) {
		accstr = XmStringCreate(bip->acctext, XmSTRING_DEFAULT_CHARSET);
		XtSetArg (args[n], XmNacceleratorText, accstr); n++;
		XtSetArg (args[n], XmNaccelerator, bip->acc); n++;
	    }
	    if (bip->label) {
		labstr = XmStringCreate (bip->label, XmSTRING_DEFAULT_CHARSET);
		XtSetArg (args[n], XmNlabelString, labstr); n++;
	    }
	    XtSetArg (args[n], XmNmnemonic, bip->mne); n++;
	    button = XmCreatePushButton (pulldown_w, bip->name, args, n);
	    XtManageChild (button);
	    if (bip->cb)
		XtAddCallback (button, XmNactivateCallback, bip->cb,
							(XtPointer)bip->client);
	    if (accstr)
		XmStringFree (accstr);
	    if (labstr)
		XmStringFree (labstr);

	    if (bip->tip)
		wtip (button, bip->tip);
	}

	/* create a cascade button and glue them together */

	labstr = NULL;

	n = 0;
	if (pdmp->cb_label) {
	    labstr = XmStringCreate (pdmp->cb_label, XmSTRING_DEFAULT_CHARSET);
	    XtSetArg (args[n], XmNlabelString, labstr);  n++;
	}
	XtSetArg (args[n], XmNsubMenuId, pulldown_w);  n++;
	XtSetArg (args[n], XmNmnemonic, pdmp->cb_mne); n++;
	cascade = XmCreateCascadeButton (mb_w, pdmp->cb_name, args, n);
	if (labstr)
	    XmStringFree (labstr);
	XtManageChild (cascade);
	if (pdmp->tip)
	    wtip (cascade, pdmp->tip);

	return (cascade);
}

/* this method of avoiding zombies courtesy Dean Huxley */
static void
reapchildren (int signo)
{
	while (wait3(NULL, WNOHANG, NULL) > 0)
	    continue;
}

static void
setup_sigs()
{
	/* ignore FPE, though we do have a matherr() handler in misc.c. */
	(void) signal (SIGFPE, SIG_IGN);

	/* we deal with write errors directly -- don't want the signal */
	(void) signal (SIGPIPE, SIG_IGN);

	/* no zombies */
	/* SIG_IGN doesn't do it on netbsd. better to reap. see
	 * http://www.opengroup.org/onlinepubs/007908799/xsh/exit.html
	 * http://www.opengroup.org/onlinepubs/007908799/xsh/sigaction.html
	 * if you /do/ want to wait, the temporarily block SIGCHLD.
	 */
	signal (SIGCHLD, reapchildren);
}

static void
hlp_onContext()
{
	static Cursor qc;
	Display *dsp = XtDisplay (toplevel_w);
	Window root = RootWindow(dsp, DefaultScreen(dsp));
	Window rw, cw;
	int oldtips;
	int rx, ry;
	unsigned int m;
	int x, y;

	/* make query cursor */
	if (!qc)
	    qc = XCreateFontCursor (dsp, XC_question_arrow);

	/* grab pointer and allow tips to work until press any button */
	if (XGrabPointer (dsp, root, True, ButtonPressMask, GrabModeAsync,
		    GrabModeSync, None, qc, CurrentTime) != GrabSuccess) {
	    xe_msg (1, "Could not grab pointer");
	    return;
	}
	oldtips = pref_get (PREF_TIPS);
	pref_set (PREF_TIPS, PREF_TIPSON);
	do {
	    if (!XQueryPointer (dsp, root, &rw, &cw, &rx, &ry, &x, &y, &m)){
		xe_msg (1, "XQueryPointer error");
		break;
	    }

	    while (XtAppPending(xe_app)) {
		XEvent e;
		XtAppNextEvent (xe_app, &e);
		switch (e.type) {
		case EnterNotify:
		case LeaveNotify:
		case Expose:
		    XtDispatchEvent (&e);
		    break;
		}
	    }
	} while (!(m & Button1Mask));

	XUngrabPointer (dsp, CurrentTime);

	/* restore to Preference */
	if (oldtips == PREF_NOTIPS)
	    wtip_alldown();
	pref_set (PREF_TIPS, oldtips);
}

/* bring up the initial set of desired windows */
static void
initialUps()
{
	if (atoi(getXRes (dm_viewupres(), "0")))
	    dm_manage();
	if (atoi(getXRes (sun_viewupres(), "0")))
	    sun_manage();
	if (atoi(getXRes (m_viewupres(), "0")))
	    m_manage();
	if (atoi(getXRes (e_viewupres(), "0")))
	    e_manage();
	if (atoi(getXRes (mars_viewupres(), "0")))
	    mars_manage();
	if (atoi(getXRes (jm_viewupres(), "0")))
	    jm_manage();
	if (atoi(getXRes (sm_viewupres(), "0")))
	    sm_manage();
	if (atoi(getXRes (um_viewupres(), "0")))
	    um_manage();
	if (atoi(getXRes (sv_viewupres(), "0")))
	    sv_manage();
	if (atoi(getXRes (ss_viewupres(), "0")))
	    ss_manage();
	if (atoi(getXRes (ng_viewupres(), "0")))
	    ng_manage();
	if (atoi(getXRes (mm_autortres(), "0")))
	    mm_startrt();
}

/* main menubar controls callback.
 * client is one of MBOptions.
 */
/* ARGSUSED */
static void
m_activate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int code = (long int)client;

	watch_cursor(1);

	switch (code) {
	case MSGTXT:	msg_manage(); break;
	case NETSETUP:	net_manage(); break;
	case EXTIN:	mm_external(); break;
	case PROGRESS:	pm_manage(); break;
	case QUIT:	x_quit(); break;
	case DATA:	dm_manage(); break;
	case SUNV:	sun_manage(); break;
	case EARTH:	e_manage(); break;
	case MOONV:	m_manage(); break;
	case MARSV:	mars_manage(); break;
	case JUPMOON:	jm_manage(); break;
	case SATMOON:	sm_manage(); break;
	case UMOON:	um_manage(); break;
	case GALLERY:	gal_manage(); break;
	case SKYVIEW:	sv_manage(); break;
	case SOLARSYS:	ss_manage(); break;
	case PLOT:	plot_manage(); break;
	case LIST:	lst_manage(); break;
	case SEARCH:	srch_manage(); break;
	case GLANCE:	ng_manage(); break;
	case AAVSO:	av_manage(); break;
	case CCONV:	cc_manage(); break;
	case OBSLOG:	ol_manage(); break;
	case FAVS:	fav_manage(); break;
	case OBJS:	obj_manage(); break;
	case EDB:	db_manage(); break;
	case CLOSEOBJS:	c_manage(); break;
	case FIELDSTARS:fs_manage(); break;
	case WEBDB:	wdb_manage(); break;
	case MOVIELOOP:	ml_manage(); break;
	case ABOUT:	version(); break;
	case CONFIGHLP:	hlp_config (); break;
	case REFERENCES:hlp_dialog ("Credits", NULL, 0); break;
	case INTRO:	hlp_dialog ("Intro", NULL, 0); break;
	case MAINMENU:	hlp_dialog ("MainMenu", NULL, 0); break;
	case FILEHLP:	hlp_dialog ("MainMenu_file", NULL, 0); break;
	case VIEWHLP:	hlp_dialog ("MainMenu_view", NULL, 0); break;
	case TOOLSHLP:	hlp_dialog ("MainMenu_tools", NULL, 0); break;
	case OBJHLP:	hlp_dialog ("MainMenu_objects", NULL, 0); break;
	case PREFHLP:	hlp_dialog ("MainMenu_preferences", NULL, 0); break;
	case OPERATION:	hlp_dialog ("Operation", NULL, 0); break;
	case DATETIME:	hlp_dialog ("Date_time", NULL, 0); break;
	case NOTES:	hlp_dialog ("Notes", NULL, 0); break;
	case CONTEXTHLP:hlp_onContext (); break;
	default: 	printf ("Main menu bug: code=%d\n", code); abort();
	}

	watch_cursor(0);
}

/* outta here */
static void
goodbye()
{
	XtCloseDisplay (XtDisplay (toplevel_w));
	exit(0);
}

/* user wants to quit */
static void
x_quit()
{
	int nchg = sr_refresh();

	if (confirm()) {
	    if (!nchg || (sr_autosaveon() && !sr_save(0)) || sr_isUp()) {
		query (toplevel_w, "Exit xephem?",
					"  Yes  ",  "  No  ", 0,
				        goodbye,    0,        0);
	    } else  {
		query (toplevel_w, "Review unsaved Major\nresources before exiting?",
					"  Yes  ",  "  No  ", 0,
					sr_xmanage, goodbye,  0);
	    }
	} else {
	    if (nchg > 0 && sr_autosaveon() && sr_save(0) < 0)
		return;	/* abort exit if save failed */
	    goodbye();
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: xephem.c,v $ $Date: 2013/01/14 01:04:24 $ $Revision: 1.114 $ $Name:  $"};

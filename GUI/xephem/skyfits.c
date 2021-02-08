/* code to open local or fetch FITS files for skyview from STScI or ESO.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <math.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>
#include <Xm/Scale.h>
#include <Xm/FileSB.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/Text.h>


#include "xephem.h"

#define	MAXDSSFOV	(30.)	/* max field size we retreive, arcmins*/
#define	MINDSSFOV	(5.)	/* min field size we retreive, arcmins*/


static int sf_readFile (char *name);
static void sf_create (void);
static void initFSB (Widget w);
static void initPubShared (Widget rc_w, Widget fsb_w);
static void sf_save_cb (Widget w, XtPointer client, XtPointer call);
static void save_file (void);
static void sf_open_cb (Widget w, XtPointer client, XtPointer call);
static void sf_close_cb (Widget w, XtPointer client, XtPointer call);
static void sf_help_cb (Widget w, XtPointer client, XtPointer call);
static void sf_setdate_cb (Widget w, XtPointer client, XtPointer call);
static void sf_setSaveName (char *newfn);
static char *bname (char *buf);

static void eso_fits (void);
static void stsci_fits (void);
static void fits_read_icb (XtPointer client, int *fd, XtInputId *id);
static int fitsObs (double *mjdp);
static void sf_setObsDate (void);
static int prepOpen (char fn[], char errmsg[]);
static XtInputId read_iid;	/* set while working on reading from socket  */
static XtIntervalId read_to;	/* callback to poll for read cancel */
static void fits_read_to (XtPointer client, XtIntervalId *id);
static void fits_read_abort (FImage *fip);
static int fr_socket;		/* FITS reading socket */

static Widget sf_w;		/* main dialog */
static Widget savefn_w;		/* TF for save filename */
static Widget stsci_w;		/* TB for STScI, else ESO */
static Widget fsb_w;		/* FSB for opening a file */
static Widget hdr_w;		/* ScrolledText for the FITS header */
static Widget autoname_w;	/* TB for whether to auto set save filename */ 
static Widget obsdate_w;	/* label for obs date string */
static Widget setobsdate_w;	/* PB to set main to obs date */
static Widget dss1_w;		/* TB set to use DSS 1 */
static Widget dss2r_w;		/* TB set to use DSS 2 red */
static Widget dss2b_w;		/* TB set to use DSS 2 blue */


#define	FWDT	1234		/* FITS file poll interval, ms */
static int fw_isFifo (char *name);
static void fw_to (XtPointer client, XtIntervalId *id);
static void fw_icb (XtPointer client, int *fd, XtInputId *id);
static void fw_cb (Widget w, XtPointer client, XtPointer call);
static void fw_on (int whether);
static XtIntervalId fw_tid;	/* used to poll for file naming FITS file */
static XtInputId fw_iid;	/* used to monitor FIFO for name of FITS file */
static Widget fwfn_w;		/* TF holding Watch file name */
static Widget fw_w;		/* TB whether to watch for FITS file */
static int fw_fd;		/* file watch fifo id */

/* which survey */
typedef enum {
    DSS_1, DSS_2R, DSS_2B
} Survey;
static Survey whichSurvey (void);

#define	FCPP	500		/* FITS read cancel poll period, ms */


static char fitsp[] = "FITSpattern";	/* resource name of FITS file pattern */
#if defined (__NUTC__)
static char gexe[] = "gunzip.exe";	/* gunzip executable */
#else
static char gexe[] = "gunzip";		/* gunzip executable */
#endif
static char gcmd[] = "gunzip";		/* gunzip command's argv[0] */

static char skyfitscategory[] = "Sky View -- FITS";	/* Save category */

/* called to manage the fits dialog.
 */
void
sf_manage()
{
	if (!sf_w) {
	    sf_create();
	    si_create();
	}

	XtManageChild(sf_w);
}

/* called to unmanage the fits dialog.
 */
void
sf_unmanage()
{
	if (!sf_w)
	    return;
	XtUnmanageChild (sf_w);
}

/* return 1 if dialog is up, else 0.
 */
int
sf_ismanaged()
{
	return (sf_w && XtIsManaged(sf_w));
}

/* called to put up or remove the watch cursor.  */
void
sf_cursor (Cursor c)
{
	Window win;

	if (sf_w && (win = XtWindow(sf_w)) != 0) {
	    Display *dsp = XtDisplay(sf_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* install fip as the new current image with the given name.
 * N.B. fip memory is made persistent, do not reset on return
 * last argument determines whether contrast and WCS are set automatically.
 */
void
sf_newFITS (FImage *fip, char name[], int autocon)
{
	/* install fip as current image */
	si_newfim (fip, name, autocon);

	/* set save name from image center, if enabled */
	if (XmToggleButtonGetState(autoname_w))
	    sf_setSaveName (name);

	/* set image date */
	sf_setObsDate ();
}

/* fill the hdr_w scrolled text with the FITS header entries.
 * keep hdrl up to date.
 */
void
sf_showHeader (fip)
FImage *fip;
{
	char *header;
	int i;

	if (!sf_w) {
	    sf_create();
	    si_create();
	}

	/* room for each FITS line, with nl and a final END and \0 */
	header = malloc ((fip->nvar+1)*(FITS_HCOLS+1) + 1);
	if (!header) {
	    xe_msg (0, "No memory to display FITS header");
	    return;
	}

	/* copy from fip->var to header, adding \n after each line */
	for (i = 0; i < fip->nvar; i++) {
	    memcpy(header + i*(FITS_HCOLS+1), fip->var[i], FITS_HCOLS);
	    header[(i+1)*(FITS_HCOLS+1)-1] = '\n';
	}
	
	/* add END and '\0' to make it a real string */
	(void) sprintf (&header[i*(FITS_HCOLS+1)], "END");

	XmTextSetString (hdr_w, header);
	free (header);

	/* scroll to the top */
	XmTextShowPosition (hdr_w, (XmTextPosition)0);
}

/* return copies of the current filename and OBJECT or TARGET keywords.
 * if either can not be determined, the returned string will be 0 length.
 * N.B. we assume the caller supplies "enough" space.
 */
void
sf_getName (fn, on)
char *fn;	/* filename */
char *on;	/* object name */
{
	FImage *fip;
	char *savefn;
	int i, n;

	fip = si_getFImage ();
	if (!fip) {
	    *fn = *on = '\0';
	    return;
	}

	savefn = XmTextFieldGetString (savefn_w);
	n = strlen (savefn);

	for (i = n-1; i >= 0 && savefn[i] != '/' && savefn[i] != '\\'; --i)
	    continue;
	strcpy (fn, &savefn[i+1]);
	XtFree (savefn);

	if (getStringFITS (fip, "OBJECT", on) < 0 &&
				getStringFITS (fip, "TARGET", on) < 0)
	    *on = '\0';
}

/* t00fri: include possibility to read .fth compressed files */
static int
prepOpen (fn, errmsg)
char fn[];
char errmsg[];
{
	int fd;
	int l;

	l = strlen (fn);
	if (l < 4 || strcmp(fn+l-4, ".fth")) {
	    /* just open directly */
	    fd = openh (fn, O_RDONLY);
	    if (fd < 0)
		strcpy (errmsg, syserrstr());
	} else {
	    /* ends with .fth so need to run through fdecompress
	     * TODO: this is a really lazy way to do it --
	     */
	    char cmd[2048];
	    char tmp[2048];
	    int s;

	    tempfilename (tmp, "xefts", ".fth");
	    sprintf (cmd, "cp %s %s; fdecompress -r %s", fn, tmp, tmp);
	    s = system (cmd);
	    if (s != 0) {
		sprintf (errmsg, "Can not execute `%s' ", cmd);
		if (s < 0)
		    strcat (errmsg, syserrstr());
		fd = -1;
	    } else {
		tmp[strlen(tmp)-1] = 's';
		fd = openh (tmp, O_RDONLY);
		(void) unlink (tmp);	/* once open, remove the .fts copy */
		if (fd < 0)
		    sprintf (errmsg, "Can not decompress %s: %s", tmp,
							    syserrstr());
	    }
	}

	return (fd);
}

/* open and read a FITS file.
 * if all ok return 0, else return -1.
 */
static int
sf_readFile (name)
char *name;
{
	char buf[1024];
	FImage fim, *fip = &fim;
	char errmsg[1024];
	int fd;
	int s;

	/* open the fits file */
	fd = prepOpen (name, errmsg);
	if (fd < 0) {
	    xe_msg (1, "%s: %s", name, errmsg);
	    return(-1);
	}

	/* read in */
	s = readFITS (fd, fip, buf);
	close (fd);
	if (s < 0) {
	    xe_msg (1, "%s: %s", name, buf);
	    return(-1);
	}

	/* ok!*/
	sf_newFITS (fip, name, 1);
	return (0);
}

/* create, but do not manage, the FITS file dialog */
static void
sf_create()
{
	Widget tf_w, bf_w;
	Widget rc_w, rb_w;
	Widget go_w;
	Widget pw_w;
	Widget h_w;
	Widget w;
	Arg args[20];
	int n;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNallowResize, True); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	XtSetArg (args[n], XmNmarginWidth, 5); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	sf_w = XmCreateFormDialog (svshell_w, "SkyFITS", args, n);
	set_something (sf_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (sf_w, XmNhelpCallback, sf_help_cb, NULL);
	sr_reg (XtParent(sf_w), "XEphem*SkyFITS.x", skyfitscategory, 0);
	sr_reg (XtParent(sf_w), "XEphem*SkyFITS.y", skyfitscategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "xephem Sky FITS"); n++;
	XtSetValues (XtParent(sf_w), args, n);

	/* top and bottom halves are in their own forms, then
	 * each form is in a paned window
	 */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	pw_w = XmCreatePanedWindow (sf_w, "FITSPW", args, n);
	XtManageChild (pw_w);

	/* the top form */

	n = 0;
	tf_w = XmCreateForm (pw_w, "TF", args, n);
	XtManageChild (tf_w);

	    /* controls to fetch networked images */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNtopOffset, 6); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    go_w = XmCreatePushButton (tf_w, "Get", args, n);
	    wtip (go_w, "Retrieve image of Sky View center over Internet");
	    XtAddCallback (go_w, XmNactivateCallback, sf_go_cb, NULL);
	    XtManageChild (go_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNtopOffset, 8); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, go_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 4); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    w = XmCreateLabel (tf_w, "GL", args, n);
	    set_xmstring (w,XmNlabelString,"Digitized Sky Survey image:");
	    XtManageChild (w);

	    /* institution selection */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, go_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 3); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 3); n++;
	    w = XmCreateLabel (tf_w, "From:", args, n);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, go_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 1); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 25); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	    XtSetArg (args[n], XmNspacing, 6); n++;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    rb_w = XmCreateRadioBox (tf_w, "GRB", args, n);
	    XtManageChild (rb_w);

		n = 0;
		XtSetArg (args[n], XmNspacing, 4); n++;
		stsci_w = XmCreateToggleButton (rb_w, "STScI", args, n);
		wtip (stsci_w, "Get image from Maryland USA");
		XtManageChild (stsci_w);
		sr_reg (stsci_w, NULL, skyfitscategory, 1);

		/* stsci sets logic */
		n = 0;
		XtSetArg (args[n], XmNspacing, 4); n++;
		XtSetArg(args[n],XmNset,!XmToggleButtonGetState(stsci_w)); n++;
		w = XmCreateToggleButton (rb_w, "ESO", args, n);
		wtip (stsci_w, "Get image from Germany");
		XtManageChild (w);

	    /* survey selection */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rb_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 1); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 3); n++;
	    w = XmCreateLabel (tf_w, "Survey:", args, n);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rb_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 0); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 25); n++;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    XtSetArg (args[n], XmNpacking, XmPACK_TIGHT); n++;
	    XtSetArg (args[n], XmNspacing, 6); n++;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    rb_w = XmCreateRadioBox (tf_w, "GRB", args, n);
	    XtManageChild (rb_w);

		n = 0;
		XtSetArg (args[n], XmNspacing, 4); n++;
		dss1_w = XmCreateToggleButton (rb_w, "DSS1", args, n);
		set_xmstring (dss1_w, XmNlabelString, "DSS 1");
		wtip (dss1_w, "Original DSS");
		XtManageChild (dss1_w);
		sr_reg (dss1_w, NULL, skyfitscategory, 1);

		n = 0;
		XtSetArg (args[n], XmNspacing, 4); n++;
		dss2r_w = XmCreateToggleButton (rb_w, "DSS2R", args, n);
		set_xmstring (dss2r_w, XmNlabelString, "DSS 2R");
		wtip (dss2r_w, "DSS 2, Red band (90% complete)");
		XtManageChild (dss2r_w);
		sr_reg (dss2r_w, NULL, skyfitscategory, 1);

		n = 0;
		XtSetArg (args[n], XmNspacing, 4); n++;
		dss2b_w = XmCreateToggleButton (rb_w, "DSS2B", args, n);
		set_xmstring (dss2b_w, XmNlabelString, "DSS 2B");
		wtip (dss2b_w, "DSS 2, Blue band (50% complete)");
		XtManageChild (dss2b_w);
		sr_reg (dss2b_w, NULL, skyfitscategory, 1);

	    /* header, with possible date */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rb_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 10); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    h_w = XmCreateLabel (tf_w, "Lab", args, n);
	    set_xmstring (h_w, XmNlabelString, "FITS Header:");
	    XtManageChild (h_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rb_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 10); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    setobsdate_w = XmCreatePushButton (tf_w, "SO", args, n);
	    set_xmstring (setobsdate_w, XmNlabelString, "Set time");
	    XtAddCallback (setobsdate_w, XmNactivateCallback, sf_setdate_cb, 0);
	    wtip(setobsdate_w,"Set main XEphem time to this Observation time");
	    XtManageChild (setobsdate_w);
	    XtSetSensitive (setobsdate_w, False); /* set true when have date */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rb_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 10); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, h_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, setobsdate_w); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	    obsdate_w = XmCreateLabel (tf_w, "ObsDate", args, n);
	    set_xmstring (obsdate_w, XmNlabelString, " ");
	    wtip(obsdate_w, "Best-guess of time of Observation");
	    XtManageChild (obsdate_w);

	    /* scrolled text in which to display the header */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, setobsdate_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 2); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomOffset, 10); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNautoShowCursorPosition, False); n++;
	    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	    XtSetArg (args[n], XmNeditable, False); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	    hdr_w = XmCreateScrolledText (tf_w, "Header", args, n);
	    wtip (hdr_w, "Scrolled text area containing FITS File header");
	    XtManageChild (hdr_w);

	/* the bottom form */

	n = 0;
	bf_w = XmCreateForm (pw_w, "BF", args, n);
	XtManageChild (bf_w);

	    /* auto listen */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNtopOffset, 16); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (bf_w, "FFWL", args, n);
	    set_xmstring (w, XmNlabelString, "File watch:");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNtopOffset, 15); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 45); n++;
	    fw_w = XmCreateToggleButton (bf_w, "Watch", args, n);
	    /* N.B. don't sr_reg because that can trigger before SV ever up */
	    XtAddCallback (fw_w, XmNvalueChangedCallback, fw_cb, NULL);
	    wtip (fw_w,
		    "Whether to watch this file for name of FITS file to load");
	    XtManageChild (fw_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, fw_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 2); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    fwfn_w = XmCreateTextField (bf_w, "WatchFile", args, n);
	    defaultTextFN (fwfn_w, 0, getPrivateDir(), "watch.txt");
	    sr_reg (fwfn_w, NULL, skyfitscategory, 1);
	    wtip (fwfn_w,"Name of file to watch for name of FITS file to load");
	    XtManageChild (fwfn_w);

	    /* label, go PB, Auto name TB and TF for saving a file */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, fwfn_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 16); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (bf_w, "Save", args, n);
	    set_xmstring (w, XmNlabelString, "Save as:");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, fwfn_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 15); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 45); n++;
	    w = XmCreatePushButton (bf_w, "Save", args, n);
	    set_xmstring (w, XmNlabelString, "Save now");
	    XtAddCallback (w, XmNactivateCallback, sf_save_cb, NULL);
	    wtip (w, "Save the current image to the file named below");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, fwfn_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    autoname_w = XmCreateToggleButton (bf_w, "AutoName", args, n);
	    set_xmstring (autoname_w, XmNlabelString, "Auto name");
	    XtManageChild (autoname_w);
	    wtip (autoname_w, "When on, automatically chooses a filename based on RA and Dec");
	    sr_reg (autoname_w, NULL, skyfitscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, autoname_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 2); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    savefn_w = XmCreateTextField (bf_w, "SaveFN", args, n);
	    defaultTextFN (savefn_w, 0, getPrivateDir(), "xxx.fts");
	    XtAddCallback (savefn_w, XmNactivateCallback, sf_save_cb, NULL);
	    wtip (savefn_w, "Enter name of file to write, then press Enter");
	    XtManageChild (savefn_w);

	    /* the Open FSB */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, savefn_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 16); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    w = XmCreateLabel (bf_w, "Lab", args, n);
	    set_xmstring (w, XmNlabelString, "Open:");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, savefn_w); n++;
	    XtSetArg (args[n], XmNtopOffset, 14); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    XtSetArg (args[n], XmNspacing, 5); n++;
	    rc_w = XmCreateRowColumn (bf_w, "USRB", args, n);
	    XtManageChild (rc_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    /* t00fri: keeps FILE scrolled list width correct */
            XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
	    fsb_w = XmCreateFileSelectionBox (bf_w, "FSB", args, n);
	    XtManageChild (fsb_w);
	    initFSB(fsb_w);
	    initPubShared (rc_w, fsb_w);
}

/* init the directory and pattern resources of the given FileSelectionBox.
 * we try to pull these from the basic program resources.
 */
static void
initFSB (fsb_w)
Widget fsb_w;
{
	Widget w;

	/* set default dir and pattern */
	set_xmstring (fsb_w, XmNdirectory, getPrivateDir());
	set_xmstring (fsb_w, XmNpattern, getXRes (fitsp, "*.f*t*"));

	/* change some button labels.
	 * N.B. can't add tips because these are really Gadgets.
	 */
	w = XmFileSelectionBoxGetChild (fsb_w, XmDIALOG_OK_BUTTON);
	set_xmstring (w, XmNlabelString, "Open");
	w = XmFileSelectionBoxGetChild (fsb_w, XmDIALOG_CANCEL_BUTTON);
	set_xmstring (w, XmNlabelString, "Close");

	/* some other tips */
	w = XmFileSelectionBoxGetChild (fsb_w, XmDIALOG_FILTER_TEXT);
	wtip (w, "Current directory and pattern; press `Filter' to rescan");
	w = XmFileSelectionBoxGetChild (fsb_w, XmDIALOG_TEXT);
	wtip (w, "FITS file name to be read if press `Open'");

	/* connect an Open handler */
	XtAddCallback (fsb_w, XmNokCallback, sf_open_cb, NULL);

	/* connect a Close handler */
	XtAddCallback (fsb_w, XmNcancelCallback, sf_close_cb, NULL);

	/* connect a Help handler */
	XtAddCallback (fsb_w, XmNhelpCallback, sf_help_cb, NULL);
}

/* callback from the Public dir PB */
/* ARGSUSED */
static void
sharedDirCB (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Widget fsb_w = (Widget)client;
	char buf[1024];

	(void) sprintf (buf, "%s/fits", getShareDir());
	set_xmstring (fsb_w, XmNdirectory, expand_home(buf));
}

/* callback from the Private dir PB */
/* ARGSUSED */
static void
privateDirCB (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	Widget fsb_w = (Widget)client;

	set_xmstring (fsb_w, XmNdirectory, getPrivateDir());
}

/* build a set of PB in RC rc_w so that they
 * set the XmNdirectory resource in the FSB fsb_w and invoke the Filter.
 */
static void
initPubShared (rc_w, fsb_w)
Widget rc_w, fsb_w;
{
	Arg args[20];
	char tip[1024];
	Widget w;
	int n;

	n = 0;
	w = XmCreateLabel (rc_w, "Dir", args, n);
	set_xmstring (w, XmNlabelString, "Look in:");
	XtManageChild (w);

	n = 0;
	w = XmCreatePushButton (rc_w, "Private", args, n);
	XtAddCallback(w, XmNactivateCallback, privateDirCB, (XtPointer)fsb_w);
	sprintf (tip, "Set directory to %s", getPrivateDir());
	wtip (w, XtNewString(tip));
	XtManageChild (w);

	n = 0;
	w = XmCreatePushButton (rc_w, "Shared", args, n);
	XtAddCallback(w, XmNactivateCallback, sharedDirCB, (XtPointer)fsb_w);
	sprintf (tip, "Set directory to %s/fits", getShareDir());
	wtip (w, XtNewString(tip));
	XtManageChild (w);
}

/* called when Watch TB changes */
/* ARGSUSED */
static void
fw_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	fw_on (XmToggleButtonGetState(w));
}

/* called when Get PB or toolbar PB is hit */
/* ARGSUSED */
void
sf_go_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (!sf_w) {
	    sf_create();
	    si_create();
	}

	if (read_iid) {
	    xe_msg (1, "DSS download is already in progress.");
	    return;
	}

	if (XmToggleButtonGetState(stsci_w))
	    stsci_fits();
	else
	    eso_fits();
}

/* called when CR is hit in the Save text field or the Save PB is hit */
/* ARGSUSED */
static void
sf_save_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *fn;

	if (!si_getFImage ()) {
	    xe_msg (1, "No current FITS file to save");
	    return;
	}

	fn = XmTextFieldGetString (savefn_w);
	if (!fn || (int)strlen(fn) < 1) {
	    xe_msg (1, "Please specify a filename");
	    XtFree (fn);
	    return;
	}

	if (existsh (fn) == 0 && confirm()) {
	    char buf[1024];
	    (void) sprintf (buf, "%s exists:\nOk to overwrite?", bname(fn));
	    query (sf_w, buf, "Yes -- Overwrite", "No -- Cancel",
						NULL, save_file, NULL, NULL);
	} else
	    save_file();

	XtFree (fn);
}

/* save to file named in savefn_w.
 * we already know everything is ok to just do it now.
 */
static void
save_file()
{
	FImage *fip;
	char buf[1024];
	char *fn;
	int fd;

	fn = XmTextFieldGetString (savefn_w);

	fd = openh (fn, O_CREAT|O_WRONLY, 0666);
	if (fd < 0) {
	    xe_msg (1, "%s: %s", fn, syserrstr());
	    XtFree (fn);
	    return;
	}

	fip = si_getFImage ();
	if (!fip) {
	    printf ("FImage disappeared in save_file\n");
	    abort();
	}
	si_setContrast(fip);
	if (writeFITS (fd, fip, buf, 1) < 0) {
	    xe_msg (1, "%s: %s", fn, buf);
	} else {
	    xe_msg (confirm(), "%s:\nwritten successfully", fn);
	}

	(void) close (fd);
	XtFree (fn);
}

/* called when a file selected by the FSB is to be opened */
static void
/* ARGSUSED */
sf_open_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmFileSelectionBoxCallbackStruct *s =
				    (XmFileSelectionBoxCallbackStruct *)call;
	char *sp;

	if (s->reason != XmCR_OK) {
	    printf ("%s: Unknown reason = 0x%x\n", "sf_open_cb()", s->reason);
	    abort();
	}

	watch_cursor(1);

	XmStringGetLtoR (s->value, XmSTRING_DEFAULT_CHARSET, &sp);
	sf_readFile (sp);
	XtFree (sp);

	watch_cursor(0);
}

/* ARGSUSED */
static void
sf_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (sf_w);
}

/* ARGSUSED */
static void
sf_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"Read in local FITS files or read from Network.",
"Resulting image will be displayed in Sky View."
};

	hlp_dialog ("Sky FITS", msg, sizeof(msg)/sizeof(msg[0]));
}

/* callback to set main time to match FITS */
/* ARGSUSED */
static void
sf_setdate_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	double newmjd;

	if (fitsObs(&newmjd) == 0)
	    mm_newcaldate(newmjd);
}

/* set savefn_w to newfn.
 * preserve any existing directory.
 */
static void
sf_setSaveName(newfn)
char *newfn;
{
	char buf[1024];
	char *fn;

	fn = XmTextFieldGetString (savefn_w);
	(void) sprintf (buf, "%.*s%s", (int)(bname(fn)-fn), fn, bname(newfn));
	XtFree (fn);
	XmTextFieldSetString (savefn_w, buf);
}

/* return pointer to basename portion of filename fn.
 */
static char *
bname (fn)
char *fn;
{
	char *base;

	if (!(base = strrchr(fn,'/')) && !(base = strrchr(fn,'\\')))
	    base = fn;
	else
	    base++;		/* skip the / */

	return (base);
}

/* return 0 if find the string str in buf, else -1 */
static int
chk4str (str, buf)
char str[];
char buf[];
{
	int l = strlen (str);

	while (*buf != '\0')
	    if (strncmp (str, buf++, l) == 0)
		return (0);
	return (-1);
}

/* return 0 if have gunzip else -1 */
static int
chk_gunzip()
{
#define	NOGZEXIT	88		/* any impossible gzip exit value */
	static int know = 1;		/* 0 or -1 when know for sure */
	int wstatus;
	sigset_t oss, nss;
	int pid;

	/* only really test once */
	if (know <= 0)
	    return (know);

	/* main program reaps children with a SIGCHLD handler to avoid
	 * zombies. we want to temporarily prevent that here so we can wait.
	 */
	sigemptyset (&nss);
	sigaddset (&nss, SIGCHLD);
	sigprocmask (SIG_BLOCK, &nss, &oss);

	/* fork/exec and see how it exits */
	pid = fork();
	if (pid < 0)
	    return (know = -1);
	if (pid == 0) {
	    /* new process: exec gunzip reading /dev/null else exit NOGZEXIT */
	    int nullfd = open ("/dev/null", O_RDWR);
	    if (nullfd < 0)
		_exit(NOGZEXIT);
	    dup2 (nullfd, 0);
	    dup2 (nullfd, 1);
	    dup2 (nullfd, 2);
	    execlp (gexe, gcmd, NULL);
	    /* does not return if works */
	    _exit(NOGZEXIT);
	}

	/* parent waits for exit status */
	know = (waitpid (pid, &wstatus, 0) == pid && WIFEXITED(wstatus)
				&& WEXITSTATUS(wstatus) != NOGZEXIT) ? 0 : -1;

	/* resume allowing SIGCHLD */
	sigprocmask (SIG_SETMASK, &oss, NULL);

	return (know);
}

/* return 1 if have/want to use gunzip, else 0 */
static int
use_gunzip()
{
	if (chk_gunzip() < 0) {
	    xe_msg (1,"Can not find %s.\nProceeding without compression", gcmd);
	    return (0);
	}

	return (1);
}

/* setup the pipe between gunzip and xephem to decompress the data.
 * return pid if ok, else -1.
 */
static int
setup_gunzip_pipe(int sockfd)
{
	int gzfd[2];		/* file descriptors for gunzip pipe */
	int pid;
	int errfd;

	/* make the pipe to gunzip */
	if (pipe(gzfd) < 0) {
	    xe_msg (1, "Can not make pipe for gunzip");
	    return (-1);
	}

	/* fork/exec gunzip */
	switch((pid = fork())) {
	case 0:			/* child: put gunzip between socket and us */
	    close (gzfd[0]);	/* do not need read side of pipe */
	    dup2 (sockfd, 0);	/* socket becomes gunzip's stdin */
	    close (sockfd);	/* do not need after dup */
	    dup2 (gzfd[1], 1);	/* write side of pipe becomes gunzip's stdout */
	    close (gzfd[1]);	/* do not need after dup */
	    errfd = open ("/dev/null", O_RDWR);
	    if (errfd >= 0) {
		dup2 (errfd, 2);/* dump gunzip's stderr */
		close (errfd);
	    }
	    execlp (gexe, gcmd, "-c", NULL);	/*should work, already checked*/
	    _exit(1);		/* exit in case gunzip disappeared */
	    break;		/* :) */

	case -1:	/* fork failed */
	    xe_msg (1, "Can not fork for gunzip");
	    return (-1);

	default:	/* parent */
	    break;
	}
	
	/* put gunzip between the socket and us */
	close (gzfd[1]);	/* do not need write side of pipe */
	dup2 (gzfd[0], sockfd);	/* read side of pipe masquarades as socket */
	close (gzfd[0]);	/* do not need after dup */

	/* return gunzip's pid */
	return (pid);
}

static Survey
whichSurvey()
{
	if (XmToggleButtonGetState(dss2r_w))
	    return (DSS_2R);
	if (XmToggleButtonGetState(dss2b_w))
	    return (DSS_2B);
	return (DSS_1);
}

/* start an input stream reading a FITS image from ESO */
static void
eso_fits()
{
	static char host[] = "archive.eso.org";
	static FImage fim, *fip = &fim;
	double fov, alt, az, ra, dec;
	char rastr[32], *rap, decstr[32], *decp;
	char buf[2048], msg[1024];
	char *survey;
	int gzpid;
	int aamode;
	int sawfits;

	/* do not turn off watch until completely finished */
	watch_cursor (1);

	/* let user abort */
	stopd_up();

	/* init fip (not reset because we copy the malloc'd fields to fim) */
	initFImage (fip);

	/* find current skyview center and size, in degrees */
	sv_getcenter (&aamode, &fov, &alt, &az, &ra, &dec);
	fov = 60*raddeg(fov);
	ra = radhr (ra);
	dec = raddeg (dec);

	if (fov > MAXDSSFOV)
	    fov = MAXDSSFOV;
	if (fov < MINDSSFOV)
	    fov = MINDSSFOV;

	/* get desired survey */
	switch (whichSurvey()) {
	default:
	case DSS_1:  survey = "DSS1"; break;
	case DSS_2R: survey = "DSS2-red"; break;
	case DSS_2B: survey = "DSS2-blue"; break;
	}

	/* format and send the request.
	 * N.B. ESO can't tolerate leading blanks in ra dec specs
	 */
	fs_sexa (rastr, ra, 2, 3600);
	for (rap = rastr; *rap == ' '; rap++)
	    continue;
	fs_sexa (decstr, dec, 3, 3600);
	for (decp = decstr; *decp == ' '; decp++)
	    continue;
	(void) sprintf (buf, "GET http://%s/dss/dss?ra=%s&dec=%s&equinox=J2000&Sky-Survey=%s&mime-type=%s&x=%.0f&y=%.0f HTTP/1.0\r\nUser-Agent: xephem/%s\r\n\r\n",
						host, rap, decp, survey,
			use_gunzip() ? "display/gz-fits" : "application/x-fits",
						fov, fov, PATCHLEVEL);
	xe_msg (0, "Command to %s:\n%s", host, buf);
	fr_socket = httpGET (host, buf, msg);
	if (fr_socket < 0) {
	    xe_msg (1, msg);
	    stopd_down();
	    watch_cursor (0);
	    return;
	}

	/* read back the header -- ends with a blank line */
	sawfits = 0;
	while (recvline (fr_socket, buf, sizeof(buf)) > 1) {
	    xe_msg (0, "Rcv: %s", buf);
	    if (chk4str ("application/x-fits", buf) == 0
					 || chk4str ("image/x-fits", buf) == 0)
		sawfits = 1;
	}

	/* if do not see a fits file, show what we do find */
	if (!sawfits) {
	    xe_msg (0, " ");
	    xe_msg (1, "Message from server in File->System log");
	    xe_msg (0, "------------------");
	    while (recvline (fr_socket, buf, sizeof(buf)) > 0)
		xe_msg (0, "Rcv: %s", buf);
	    xe_msg (0, "------------------");
	    xe_msg (0, "End of Message from server");
	    msg_manage();
	    watch_cursor (0);
	    close (fr_socket);
	    stopd_down();
	    return;
	}

	/* possibly connect via gunzip -- weird if have gunzip but can't */
	if (use_gunzip()) {
	    gzpid = setup_gunzip_pipe(fr_socket);
	    if (gzpid < 0) {
		watch_cursor (0);
		close (fr_socket);
		stopd_down();
		return;
	    }
	} else
	    gzpid = -1;
		

	/* read the FITS header portion */
	if (readFITSHeader (fr_socket, fip, buf) < 0) {
	    watch_cursor (0);
	    xe_msg (1, "%s", buf);
	    resetFImage (fip);
	    close (fr_socket);
	    if (gzpid > 0)
		kill (gzpid, SIGTERM);
	    stopd_down();
	    return;
	}

	/* ok, start reading the pixels and give user a way to abort */
	pm_up();	/* for your viewing pleasure */
	read_iid = XtAppAddInput (xe_app, fr_socket, (XtPointer)XtInputReadMask,
						fits_read_icb, (XtPointer)fip);
	read_to = XtAppAddTimeOut (xe_app, FCPP, fits_read_to, (XtPointer)(fip));
}

/* start an input stream reading a FITS image from STScI */
static void
stsci_fits()
{
	static char host[] = "archive.stsci.edu";
	static FImage fim, *fip = &fim;
	double fov, alt, az, ra, dec;
	char buf[1024], msg[1024];
	char *survey;
	int gzpid;
	int aamode;
	int sawfits;

	/* do not turn off watch until completely finished */
	watch_cursor (1);

	/* let user abort */
	stopd_up();

	/* init fip (not reset because we copy the malloc'd fields to fim) */
	initFImage (fip);

	/* find current skyview center and size, in degrees */
	sv_getcenter (&aamode, &fov, &alt, &az, &ra, &dec);
	fov = 60*raddeg(fov);
	ra = raddeg (ra);
	dec = raddeg (dec);

	if (fov > MAXDSSFOV)
	    fov = MAXDSSFOV;
	if (fov < MINDSSFOV)
	    fov = MINDSSFOV;

	/* get desired survey */
	switch (whichSurvey()) {
	default:
	case DSS_1:  survey = "1"; break;
	case DSS_2R: survey = "2r"; break;
	case DSS_2B: survey = "2b"; break;
	}

	/* format and send the request */
	(void) sprintf(buf,"GET http://%s/cgi-bin/dss_search?ra=%.5f&dec=%.5f&equinox=J2000&v=%s&height=%.0f&width=%.0f&format=FITS&compression=%s&version=3 HTTP/1.0\nUser-Agent: xephem/%s\r\n\r\n",
						host, ra, dec, survey, fov, fov,
						use_gunzip() ? "gz" : "NONE",
						PATCHLEVEL);
	xe_msg (0, "Command to %s:\n%s", host, buf);
	fr_socket = httpGET (host, buf, msg);
	if (fr_socket < 0) {
	    xe_msg (1, "http get: %s", msg);
	    stopd_down();
	    watch_cursor (0);
	    return;
	}

	/* read back the header -- ends with a blank line */
	sawfits = 0;
	while (recvline (fr_socket, buf, sizeof(buf)) > 1) {
	    xe_msg (0, "Rcv: %s", buf);
	    if (chk4str ("image/x-fits", buf) == 0)
		sawfits = 1;
	}

	/* if do not see a fits file, show what we do find */
	if (!sawfits) {
	    xe_msg (0, " ");
	    xe_msg (1, "Message from server in File -> System log");
	    xe_msg (0, "------------------");
	    while (recvline (fr_socket, buf, sizeof(buf)) > 0)
		xe_msg (0, "%s", buf);
	    xe_msg (0, "------------------");
	    xe_msg (0, "End of Message from server");
	    msg_manage();
	    watch_cursor (0);
	    close (fr_socket);
	    stopd_down();
	    return;
	}


	/* possibly connect via gunzip -- weird if have gunzip but can't */
	if (use_gunzip()) {
	    gzpid = setup_gunzip_pipe(fr_socket);
	    if (gzpid < 0) {
		watch_cursor (0);
		close (fr_socket);
		stopd_down();
		return;
	    }
	} else
	    gzpid = -1;

	/* read the FITS header portion */
	if (readFITSHeader (fr_socket, fip, buf) < 0) {
	    watch_cursor (0);
	    xe_msg (1, "%s", buf);
	    resetFImage (fip);
	    close (fr_socket);
	    if (gzpid > 0)
		kill (gzpid, SIGTERM);
	    stopd_down();
	    return;
	}

	/* ok, start reading the pixels and give user a way to abort */
	pm_up();	/* for your viewing pleasure */
	read_iid = XtAppAddInput (xe_app, fr_socket, (XtPointer)XtInputReadMask,
						fits_read_icb, (XtPointer)fip);
	read_to = XtAppAddTimeOut (xe_app, FCPP, fits_read_to, (XtPointer)(fip));
}

/* called whenever there is more data available on the sockfd.
 * client is *FImage being accumulated.
 */
static void
fits_read_icb (client, fd, id)
XtPointer client;
int *fd;
XtInputId *id;
{
	FImage *fip = (FImage *)client;
	int sockfd = *fd;
	double ra, dec;
	char buf[1024];

	/* read another chunk */
	if (readIncFITS (sockfd, fip, buf) < 0) {
	    xe_msg (1, "%s", buf);
	    fits_read_abort (fip);
	    return;
	}

	/* report progress */
	pm_set (fip->nbytes * 100 / fip->totbytes);
	XmUpdateDisplay (toplevel_w);

	/* keep going if expecting more */
	if (fip->nbytes < fip->totbytes)
	    return;

	/* finished reading */
	stopd_down();
	pm_down();
	close (sockfd);
	XtRemoveInput (read_iid);
	read_iid = (XtInputId)0;
	XtRemoveTimeOut (read_to);
	read_to = (XtIntervalId)0;

	/* YES! */

	/* give it a name */

	if (xy2RADec (fip, fip->sw/2.0, fip->sh/2.0, &ra, &dec) < 0) {
	    /* no headers! use time I guess */
	    struct tm *tp;
	    time_t t0;

	    time (&t0);
	    tp = gmtime (&t0);
	    if (!tp)
		localtime (&t0);

	    sprintf (buf, "%04d%02d%02dT%02d%02d%02d.fts",
		tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday, tp->tm_hour,
		tp->tm_min, tp->tm_sec);
	} else {
	    int dneg, rh, rm, dd, dm;
	    ra = radhr(ra);
	    dec = raddeg(dec);
	    if ((dneg = (dec < 0)))
		dec = -dec;
	    rh = (int)floor(ra);
	    rm = (int)floor((ra - rh)*60.0 + 0.5);
	    if (rm == 60) {
		if (++rh == 24)
		    rh = 0;
		rm = 0;
	    }
	    dd = (int)floor(dec);
	    dm = (int)floor((dec - dd)*60.0 + 0.5);
	    if (dm == 60) {
		dd++;
		dm = 0;
	    }
	    (void) sprintf (buf, "%02d%02d%c%02d%02d.fts", rh, rm,
						dneg ? '-' : '+', dd, dm);

	}

	/* commit and display */

	sf_newFITS (fip, buf, 1); /* N.B. copies fip .. do not resetFImage */
	watch_cursor (0);
}

/* called to poll cancelling FITS file download
 * client is *FImage being accumulated.
 */
static void
fits_read_to (XtPointer client, XtIntervalId *id)
{
	FImage *fip = (FImage *)client;

	if (stopd_check() < 0) {
	    fits_read_abort (fip);
	} else {
	    read_to = XtAppAddTimeOut (xe_app, FCPP, fits_read_to, (XtPointer)(fip));
	}
}

static void
fits_read_abort (FImage *fip)
{
	resetFImage (fip);
	close (fr_socket);
	XtRemoveInput (read_iid);
	read_iid = (XtInputId)0;
	XtRemoveTimeOut (read_to);
	read_to = (XtIntervalId)0;
	stopd_down();
	pm_down();
	watch_cursor (0);
}

/* poke around in the headers and try to find the mjd of the observation.
 * return 0 if think we found something, else -1
 */
static int
fitsObs(mjdp)
double *mjdp;
{
	FImage *fip = si_getFImage();
	char buf[128];
	double x;

	if (!fip)
	    return (-1);

	if (getRealFITS (fip, "JD", &x) == 0) {
	    *mjdp = x - MJD0;
	    return (0);
	}

	if (getRealFITS (fip, "EPOCH", &x) == 0) {
	    year_mjd (x, mjdp);
	    return (0);
	}

	if (getStringFITS (fip, "DATE-OBS", buf) == 0) {
	    /* try ISO 8601 then a few guesses */
	    int a, b, c, d, e, f;
	    if (sscanf (buf, "%d-%d-%dT%d:%d:%d", &a, &b, &c, &d, &e, &f) == 6){
		double day = c + (d + ((e + f/60.0)/60.0))/24.0;
		cal_mjd (b, day, a, mjdp);
		return (0);
	    }
	    if (sscanf (buf, "%d%*[/-]%d%*[/-]%d", &a, &b, &c) == 3) {
		if (a > 1900) {
		    /* yyyy-mm-dd? */
		    cal_mjd (b, (double)c, a, mjdp);
		    return (0);
		} else if (a <= 12 && b <= 31 && c < 100) {
		    /* mm-dd-yy? */
		    c += (c < 50 ? 2000 : 1900);
		    cal_mjd (a, (double)b, c, mjdp);
		    return (0);
		}
	    }
	}
	
	return (-1);
}

/* get and display the time of observation from the current FITS image */
static void
sf_setObsDate()
{
	double objsmjd;

	if (fitsObs(&objsmjd) == 0) {
	    int mm, yy, d, h, m, s;
	    double dd, dh, dm, ds;
	    char buf[128];

	    mjd_cal (objsmjd, &mm, &dd, &yy);
	    d = (int)dd;
	    dh = (dd - d)*24.;
	    h = (int)dh;
	    dm = (dh - h)*60.;
	    m = (int)dm;
	    ds = (dm - m)*60.;
	    if (ds > 59.5) {
		s = 0;
		if (++m == 60) {
		    m = 0;
		    h += 1; /* TODO: roll date if hits 24 */
		}
	    } else
		s = (int)ds;

	    sprintf (buf, "%d-%d-%d %02d:%02d:%02d", yy, mm, d, h, m, s);
	    set_xmstring (obsdate_w, XmNlabelString, buf);
	    XtSetSensitive (setobsdate_w, True);
	} else {
	    set_xmstring (obsdate_w, XmNlabelString, " ");
	    XtSetSensitive (setobsdate_w, False);
	}
}

/* turn on or off file watching.
 */
static void
fw_on (whether)
int whether;
{
	/* turn everything off */
	if (fw_tid) {
	    XtRemoveTimeOut (fw_tid);
	    fw_tid = 0;
	}
	if (fw_iid) {
	    close (fw_fd);
	    XtRemoveInput (fw_iid);
	    fw_iid = 0;
	}

	/* then maybe restart */
	if (whether) {
	    char *txt, wfn[1024];

	    /* clean scrubbed file name to watch */
	    txt = XmTextFieldGetString (fwfn_w);
	    strcpy (wfn, expand_home(txt));
	    XtFree (txt);

	    /* start timer or input depending on whether fifo */
	    if (fw_isFifo(wfn)) {
		fw_fd = open (wfn, O_RDWR);
		if (fw_fd < 0) {
		    char msg[1024];
		    sprintf (msg, "%s: %s", wfn, syserrstr());
		    XmToggleButtonSetState (fw_w, False, False);
		} else {
		    fw_iid = XtAppAddInput (xe_app, fw_fd,
				    (XtPointer)XtInputReadMask, fw_icb, NULL);
		}
	    } else {
		fw_tid = XtAppAddTimeOut (xe_app, 0, fw_to, 0);
	    }
	}
}

/* called periodically to check whether file in fwfn_w names a FITS file
 * to load. when it does, load the named file and delete the watch file
 * as a simple form of ACK.
 */
static void
fw_to (client, id)
XtPointer client;
XtIntervalId *id;
{
	char wfn[512], ffn[512];
	char *txt, *nl;
	int wfd, nr;

	/* try to open watch file */
	txt = XmTextFieldGetString (fwfn_w);
	strcpy (wfn, expand_home(txt));
	XtFree (txt);
	wfd = open (wfn, O_RDONLY|O_NONBLOCK);
	if (wfd < 0)
	    goto again;

	/* read it to get name of FITS file */
	nr = read (wfd, ffn, sizeof(ffn));
	close (wfd);
	if (nr <= 0)
	    goto again;
	ffn[nr] = '\0';
	nl = strchr (ffn, '\n');
	if (nl)
	    *nl = '\0';
	strcpy (ffn, expand_home(ffn));

	/* display and remove */
	sv_manage();
	sf_readFile (ffn);
	remove (wfn);

    again:

	fw_tid = XtAppAddTimeOut (xe_app, FWDT, fw_to, 0);
}

/* called whenever the FITS filename fifo might have something to read.
 */
static void
fw_icb (client, fdp, id)
XtPointer client;
int *fdp;
XtInputId *id;
{
	char *nl, ffn[1024];
	int nr;

	nr = read (fw_fd, ffn, sizeof(ffn));
	if (nr <= 0) {
	    if (nr < 0)
		sprintf (ffn, "FITS Watch fifo: %s", syserrstr());
	    else
		sprintf (ffn, "EOF from Watch fifo.");
	    strcat (ffn, "\nTurning FITS Watching off");
	    xe_msg (1, ffn);
	    XmToggleButtonSetState (fw_w, False, True); /* let it clean up */
	}

	ffn[nr] = '\0';
	nl = strchr (ffn, '\n');
	if (nl)
	    *nl = '\0';
	strcpy (ffn, expand_home(ffn));

	/* display */
	sv_manage();
	sf_readFile (ffn);
}

/* return whether fn claims to be a fifo */
static int
fw_isFifo (fn)
char *fn;
{
	struct stat st;
	return (!stat (fn, &st) && (st.st_mode & S_IFIFO));
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: skyfits.c,v $ $Date: 2010/10/23 02:59:13 $ $Revision: 1.66 $ $Name:  $"};

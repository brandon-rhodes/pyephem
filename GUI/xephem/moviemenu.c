/* code to manage movie loop display.
 * 
 * N.B. in order to keep the frame scale happy, we keep the min set to 0 and
 * never set the max below 1 even if there are no images. always use npixmaps
 * to tell how many frames are in the movie, not the max of the frame scale.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/DrawingA.h>
#include <Xm/CascadeB.h>
#include <Xm/SelectioB.h>
#include <Xm/Scale.h>
#include <Xm/ToggleB.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>

#include "xephem.h"
#include "png.h"

static void ml_create_shell (void);
static void ml_create_prepd_w(void);
static void ml_help_cb (Widget w, XtPointer client, XtPointer data);
static void ml_close_cb (Widget w, XtPointer client, XtPointer data);
static void ml_frame_cb (Widget w, XtPointer client, XtPointer data);
static void ml_fwrd_cb (Widget w, XtPointer client, XtPointer data);
static void ml_back_cb (Widget w, XtPointer client, XtPointer data);
static void ml_rate_cb (Widget w, XtPointer client, XtPointer data);
static void ml_delone_cb (Widget w, XtPointer client, XtPointer data);
static void ml_delall_cb (Widget w, XtPointer client, XtPointer data);
static void ml_save_cb (Widget w, XtPointer client, XtPointer data);
static void ml_exp_cb (Widget w, XtPointer client, XtPointer data);
static void ml_prefix_cb (Widget w, XtPointer client, XtPointer call);
static void ml_timer_cb (XtPointer client, XtIntervalId *id);
static void getWH (Drawable d, unsigned int *wp, unsigned int *hp);
static void delall(void);
static void showFrame(int n);
static void stopLooping(void);
static void saveMovie(char *prefix);
static int addPixmap (Drawable pm, Widget timestamp);
static int getPMPix (Pixmap pm, unsigned char **raster, unsigned int *wp,
    unsigned int *hp);
static int writePNG (char *fn, unsigned char *raster, unsigned int w,
    unsigned int h);

static Widget mlshell_w;	/* main shell */
static Widget prepd_w;		/* prefix name prompt dialog */
static Widget mlda_w;		/* main drawing area */
static Widget mlsw_w;		/* scrolled window for image */
static Widget bounce_w;		/* TB whether to bounce */
static Widget frame_w;		/* scale to show/control frame number */
static Widget rate_w;		/* scale to control movie rate */

static int bouncing;		/* whether currently moving backwards */

static Pixmap *pixmaps;		/* malloced list of pixmaps in movie */
static int npixmaps;		/* number of pixmaps in movie */

static XtIntervalId ml_id;	/* movie timer */

static char mlcategory[] = "Movie";

/* bring up the movie loop tool */
void
ml_manage()
{
	/* create shell if first time */
	if (!mlshell_w)
	    ml_create_shell();

	/* insure visible */
	XtPopup (mlshell_w, XtGrabNone);
	set_something (mlshell_w, XmNiconic, (XtArgVal)False);
}

/* add a pixmap to the movie loop, creating if first time */
void
ml_add (Drawable pm, Widget timestamp)
{
	/* avoid misuse */
	if (!pm)
	    return;

	/* create shell if first time */
	if (!mlshell_w)
	    ml_create_shell();

	/* add */
	if (timestamp)
	    XmUpdateDisplay (timestamp);
	if (addPixmap (pm, timestamp) < 0)
	    return;

	/* up and show */
	ml_manage();
	showFrame (npixmaps-1);
}

/* convenience function to add Ctrl-m accelerator to the given PB args at n.
 * return number of entries added to args[]
 */
int
ml_addacc (Arg args[], int n)
{
	XtSetArg (args[n], XmNacceleratorText,
		XmStringCreate("Ctrl+m", XmSTRING_DEFAULT_CHARSET)); n++;
	XtSetArg (args[n], XmNaccelerator, "Ctrl<Key>m"); n++;

	return (2);
}

/* called to put up or remove the watch cursor.  */
void
ml_cursor (Cursor c)
{
	Window win;

	if (mlshell_w && (win = XtWindow(mlshell_w)) != 0) {
	    Display *dsp = XtDisplay(mlshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create main shell */
static void
ml_create_shell ()
{
	Widget pd_w, cb_w, mb_w;
	Widget w, f_w, ff_w, rf_w;
	Arg args[20];
	int n;
	
	/* create outter shell and form */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Movie"); n++;
	XtSetArg (args[n], XmNiconName, "Movie"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	mlshell_w = XtCreatePopupShell ("Movie", topLevelShellWidgetClass,
							toplevel_w, args, n);
	set_something (mlshell_w, XmNcolormap, (XtArgVal)xe_cm);
	setup_icon (mlshell_w);
	sr_reg (mlshell_w, "XEphem*Movie.x", mlcategory, 0);
	sr_reg (mlshell_w, "XEphem*Movie.y", mlcategory, 0);
	sr_reg (mlshell_w, "XEphem*Movie.height", mlcategory, 0);
	sr_reg (mlshell_w, "XEphem*Movie.width", mlcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 10); n++;
	f_w = XmCreateForm (mlshell_w, "MovieF", args, n);
	XtAddCallback (f_w, XmNhelpCallback, ml_help_cb, 0);
	XtManageChild(f_w);

	/* menu bar */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	mb_w = XmCreateMenuBar (f_w, "SMB", args, n);
	XtManageChild (mb_w);

	    /* Control */

	    n = 0;
	    pd_w = XmCreatePulldownMenu (mb_w, "CPD", args, n);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'D'); n++;
		w = XmCreatePushButton (pd_w, "DelOne", args, n);
		set_xmstring (w, XmNlabelString, "Delete current frame");
		wtip (w, "Delete the current frame from the movie");
		XtAddCallback (w, XmNactivateCallback, ml_delone_cb, 0);
		XtManageChild (w);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'a'); n++;
		w = XmCreatePushButton (pd_w, "DelAll", args, n);
		set_xmstring (w, XmNlabelString, "Delete all frames");
		wtip (w, "Delete the entire movie");
		XtAddCallback (w, XmNactivateCallback, ml_delall_cb, 0);
		XtManageChild (w);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'S'); n++;
		w = XmCreatePushButton (pd_w, "Save", args, n);
		set_xmstring (w, XmNlabelString, "Save frames ...");
		wtip (w, "Save each movie frame to individual files");
		XtAddCallback (w, XmNactivateCallback, ml_save_cb, 0);
		XtManageChild (w);

		n = 0;
		w = XmCreateSeparator (pd_w, "Sep", args, n);
		XtManageChild (w);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'C'); n++;
		w = XmCreatePushButton (pd_w, "Close", args, n);
		XtAddCallback (w, XmNactivateCallback, ml_close_cb, 0);
		wtip (w, "Close this window");
		XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNmnemonic, 'C'); n++;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Control", args, n);
	    set_something (mb_w, XmNmenuHelpWidget, (XtArgVal)cb_w);
	    XtManageChild (cb_w);

	    /* help */

	    n = 0;
	    pd_w = XmCreatePulldownMenu (mb_w, "HPD", args, n);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, 'H'); n++;
		w = XmCreatePushButton (pd_w, "Help", args, n);
		wtip (w, "Get more help on making and viewing movies");
		XtAddCallback (w, XmNactivateCallback, ml_help_cb, 0);
		XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNmnemonic, 'H'); n++;
	    XtSetArg (args[n], XmNsubMenuId, pd_w); n++;
	    cb_w = XmCreateCascadeButton (mb_w, "Help", args, n);
	    set_something (mb_w, XmNmenuHelpWidget, (XtArgVal)cb_w);
	    XtManageChild (cb_w);

	/* frame control */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 45); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	ff_w = XmCreateForm (f_w, "FF", args, n);
	XtManageChild (ff_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (ff_w, "SelLbl", args, n);
	    set_xmstring (w, XmNlabelString, "Frame selection:");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    XtSetArg (args[n], XmNshowValue, True); n++;
	    XtSetArg (args[n], XmNscaleMultiple, 1); n++;
	    XtSetArg (args[n], XmNvalue, 0); n++;
	    XtSetArg (args[n], XmNmaximum, 1); n++;
	    frame_w = XmCreateScale (ff_w, "FSScale", args, n);
	    wtip (frame_w, "Displays and sets the movie frame to view");
	    XtAddCallback (frame_w, XmNdragCallback, ml_frame_cb, NULL);
	    XtAddCallback (frame_w, XmNvalueChangedCallback, ml_frame_cb, NULL);
	    XtManageChild (frame_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, frame_w); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    bounce_w = XmCreateToggleButton (ff_w, "Bounce", args, n);
	    wtip (bounce_w,"Whether to flow forward-backward or forward-reset");
	    XtManageChild (bounce_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, frame_w); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    w = XmCreatePushButton (ff_w, "Fwrd", args, n);
	    wtip (w, "Move to next frame, wrap to first from last");
	    set_xmstring (w, XmNlabelString, "Step +1");
	    XtAddCallback (w, XmNactivateCallback, ml_fwrd_cb, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, frame_w); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, w); n++;
	    XtSetArg (args[n], XmNrightOffset, 10); n++;
	    w = XmCreatePushButton (ff_w, "Back", args, n);
	    wtip (w, "Move to previous frame, wrap to last from first");
	    set_xmstring (w, XmNlabelString, "Step -1");
	    XtAddCallback (w, XmNactivateCallback, ml_back_cb, NULL);
	    XtManageChild (w);

	/* rate control */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, ff_w); n++;
	XtSetArg (args[n], XmNtopOffset, 0); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 55); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	rf_w = XmCreateForm (f_w, "RF", args, n);
	XtManageChild (rf_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    w = XmCreateLabel (rf_w, "RateLbl", args, n);
	    set_xmstring (w, XmNlabelString, "Frames per second:");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    XtSetArg (args[n], XmNshowValue, True); n++;
	    XtSetArg (args[n], XmNmaximum, 10); n++;
	    rate_w = XmCreateScale (rf_w, "FRScale", args, n);
	    wtip (rate_w, "Set the number of frames displayed per second");
	    XtAddCallback (rate_w, XmNvalueChangedCallback, ml_rate_cb, NULL);
	    XtAddCallback (rate_w, XmNdragCallback, ml_rate_cb, NULL);
	    XtManageChild (rate_w);

	/* remainder is scrolled window for images */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, mb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, ff_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	XtSetArg (args[n], XmNshadowThickness, 1); n++;
	mlsw_w = XmCreateScrolledWindow (f_w, "MovieSW", args, n);
	XtManageChild (mlsw_w);

	    /* workarea is a drawing area */

	    n = 0;
	    XtSetArg (args[n], XmNwidth, 500); n++;
	    XtSetArg (args[n], XmNheight, 500); n++;
	    mlda_w = XmCreateDrawingArea (mlsw_w, "MovieMap", args, n);
	    XtAddCallback (mlda_w, XmNexposeCallback, ml_exp_cb, NULL);
	    XtManageChild (mlda_w);

	    /* SW assumes work is its child but just to be tidy about it .. */
	    set_something (mlsw_w, XmNworkWindow, (XtArgVal)mlda_w);
}

/* ARGSUSED */
static void
ml_help_cb (Widget w, XtPointer client, XtPointer data)
{
	static char *msg[] = {
	    "This tool creates and controls a movie loop of pixmaps"
	};

	hlp_dialog ("Movie", msg, XtNumber(msg));
}

/* ARGSUSED */
static void
ml_close_cb (Widget w, XtPointer client, XtPointer data)
{
	XtPopdown (mlshell_w);
}

/* called to fill in the main drawing area with the current frame, if any */
/* ARGSUSED */
static void
ml_exp_cb (Widget wid, XtPointer client, XtPointer data)
{
	GC gc = DefaultGC(XtD,DefaultScreen(XtD));
	Window dawin = XtWindow(wid);
	unsigned int w, h;
	Pixmap pm;
	int i;

	/* might be called before any pixmaps defined */
	if (npixmaps <= 0) {
	    XClearWindow (XtD, dawin);
	    return;
	}

	/* get current pixmap.
	 * N.B. beware case of keeping XmNmax at least 1 even when only 1 frame.
	 */
	XmScaleGetValue (frame_w, &i);
	if (i == npixmaps) {
	    i = 0;
	} else if (i > npixmaps) {
	    printf ("movie frame %d > number of pixmaps %d\n", i, npixmaps);
	    abort();
	}
	pm = pixmaps[i];
	getWH (pm, &w, &h);
	XCopyArea (XtD, pm, dawin, gc, 0, 0, w, h, 0, 0);
}

/* ARGSUSED */
static void
ml_delone_cb (Widget w, XtPointer client, XtPointer data)
{
	int i;

	if (npixmaps == 0)
	    return;

	stopLooping();
	XmScaleGetValue (frame_w, &i);
	XFreePixmap (XtD, pixmaps[i]);
	memmove (&pixmaps[i], &pixmaps[i+1], (npixmaps-i-1)*sizeof(Pixmap));
	if (i == --npixmaps) {
	    i--;
	    XmScaleSetValue (frame_w, i >= 0 ? i : 0);
	}
	if (npixmaps > 0)
	    set_something (frame_w, XmNmaximum, npixmaps); /* keep max > 0 */
	showFrame (i);
}

/* ARGSUSED */
static void
ml_save_cb (Widget w, XtPointer client, XtPointer data)
{
	if (!prepd_w)
	    ml_create_prepd_w();

	if (npixmaps == 0) {
	    xe_msg (1, "Movie contains no frames");
	    return;
	}

	XtManageChild (prepd_w);
}

/* ARGSUSED */
static void
ml_delall_cb (Widget w, XtPointer client, XtPointer data)
{
	if (npixmaps == 0)
	    return;

	stopLooping();

	if (confirm())
	    query (mlshell_w, "Delete entire movie?", "No", "Yes", NULL,
							    NULL, delall, NULL);
	else
	    delall();
}

/* delete entire movie */
static void
delall()
{
	int i;

	for (i = 0; i < npixmaps; i++)
	    XFreePixmap (XtD, pixmaps[i]);

	npixmaps = 0;
	XmScaleSetValue (frame_w, 0);
	set_something (frame_w, XmNmaximum, 1);	/* keep max > 0 */
	showFrame (-1);
}

/* callback from the rate control */
static void
ml_rate_cb (Widget w, XtPointer client, XtPointer data)
{
	int r;

	XmScaleGetValue (w, &r);

	if (r == 0 && ml_id != 0) {
	    XtRemoveTimeOut (ml_id);
	    ml_id = 0;
	} else if (r > 0 && ml_id == 0 && npixmaps >= 2) {
	    ml_id = XtAppAddTimeOut (xe_app, 0, ml_timer_cb, 0);
	}
}

/* callback to move one frame forward */
static void
ml_fwrd_cb (Widget w, XtPointer client, XtPointer data)
{
	int i;

	if (npixmaps == 0) {
	    XmScaleSetValue (frame_w, 0);
	    return;
	}

	stopLooping();
	XmToggleButtonSetState (bounce_w, False, True);
	XmScaleGetValue (frame_w, &i);
	i = (i+1)%npixmaps;
	XmScaleSetValue (frame_w, i);
	showFrame (i);
}

/* callback to move one frame backward */
static void
ml_back_cb (Widget w, XtPointer client, XtPointer data)
{
	int i;

	if (npixmaps == 0) {
	    XmScaleSetValue (frame_w, 0);
	    return;
	}

	stopLooping();
	XmToggleButtonSetState (bounce_w, False, True);
	XmScaleGetValue (frame_w, &i);
	i = (i-1+npixmaps)%npixmaps;
	XmScaleSetValue (frame_w, i);
	showFrame (i);
}

/* callback from the frame control scale */
static void
ml_frame_cb (Widget w, XtPointer client, XtPointer data)
{
	int i;

	if (npixmaps == 0) {
	    XmScaleSetValue (frame_w, 0);
	    return;
	}

	stopLooping();
	XmScaleGetValue (frame_w, &i);

	/* since we keep max at least 1, beware when fewer than 2 frames */
	if (i == 1 && npixmaps < 2) {
	    XmScaleSetValue (frame_w, 0);
	    return;
	}

	showFrame (i);
}

/* function called from the interval timer used to implement the movie loop.
 * N.B. this assumes there are at least 2 images to show.
 */
/* ARGSUSED */
static void
ml_timer_cb (XtPointer client, XtIntervalId *id)
{
	int i, r;

	/* check assumption */
	if (npixmaps < 2) {
	    printf ("movie timer called with %d pixmaps\n", npixmaps);
	    abort();
	}

	/* get current image number */
	XmScaleGetValue (frame_w, &i);

	/* advance to next image */
	if (bouncing) {
	    if (--i < 0) {
		bouncing = 0;
		i = 1;
	    }
	} else {
	    if (++i == npixmaps) {
		if (XmToggleButtonGetState(bounce_w)) {
		    bouncing = 1;
		    i = npixmaps-2;
		} else
		    i = 0;
	    }
	}
	XmScaleSetValue (frame_w, i);

	/* show it */
	showFrame (i);

	/* repeat */
	XmScaleGetValue (rate_w, &r);
	ml_id = XtAppAddTimeOut (xe_app, 1000/r, ml_timer_cb, 0);
}

/* show frame i.
 * if i < 0, show a blank image.
 */
static void
showFrame (int i)
{
	if (i >= npixmaps) {
	    printf ("Bad pixmap number: %d of %d\n", i, npixmaps);
	    abort();
	}

	/* establish size */
	if (i >= 0) {
	    Pixmap pm = pixmaps[i];
	    Window dawin = XtWindow(mlda_w);
	    GC gc = DefaultGC(XtD,DefaultScreen(XtD));
	    unsigned int w, h;

	    getWH (pm, &w, &h);
	    set_something (mlda_w, XmNwidth, (XtArgVal)w);
	    set_something (mlda_w, XmNheight, (XtArgVal)h);
	    XCopyArea(XtD, pm, dawin, gc, 0, 0, w, h, 0, 0);
	} else {
	    /* N.B. setting to 0,0 crashes */
	    set_something (mlda_w, XmNwidth, (XtArgVal)1);
	    set_something (mlda_w, XmNheight, (XtArgVal)1);
	}

}

/* get the width and height of the specified drawable */
static void
getWH (Drawable dr, unsigned int *wp, unsigned int *hp)
{
	Window root;
	unsigned int b, d;
	int x, y;

	XGetGeometry (XtD, dr, &root, &x, &y, wp, hp, &b, &d);
}

/* add the given pixmap to the movie.
 * if defined, put timestamp just below.
 * return 0 if ok else -1
 */
static int
addPixmap (Drawable pm, Widget timestamp)
{
	Window tswin = timestamp ? XtWindow(timestamp) : 0;
	GC gc = DefaultGC(XtD,DefaultScreen(XtD));
	unsigned int pw, ph, tw, th;
	unsigned int b, d;
	Window root;
	int x, y;
	Pixmap newpm;

	/* get sizes */
	XGetGeometry (XtD, pm, &root, &x, &y, &pw, &ph, &b, &d);
	if (tswin)
	    getWH (tswin, &tw, &th);
	else
	    tw = th = 0;

	/* dup pm with room for timestamp below, if any */
	newpm = XCreatePixmap (XtD, pm, pw, ph+th, d);
	if (!newpm) {
	    xe_msg (1, "Can not duplicate pixmap");
	    return (-1);
	}
	XCopyArea(XtD, pm, newpm, gc, 0, 0, pw, ph, 0, 0);

	/* put timestamp beneath bottom center, if any */
	if (tswin) {
	    Pixel p;

	    get_something (timestamp, XmNbackground, (XtArgVal)&p);
	    XSetForeground (XtD, gc, p);
	    XFillRectangle (XtD, newpm, gc, 0, ph, pw, th);
	    XCopyArea (XtD, tswin, newpm, gc, 0, 0, tw, th, (pw-tw)/2, ph);
	}

	/* add newpm to list */
	pixmaps = (Pixmap *) XtRealloc ((char*)pixmaps,
						(npixmaps+1)*sizeof(Pixmap));
	pixmaps[npixmaps++] = newpm;

	/* grow frame scale -- always keep XmNmax > 0 */
	if (npixmaps > 1)
	    set_something (frame_w, XmNmaximum, (XtArgVal)(npixmaps-1));
	XmScaleSetValue (frame_w, npixmaps-1);

	return (0);
}

/* stop looping (ok if not) */
static void
stopLooping()
{
	if (ml_id != 0) {
	    XtRemoveTimeOut (ml_id);
	    ml_id = 0;
	    XmScaleSetValue (rate_w, 0);
	}
}

/* create the movie file prefix prompt */
static void
ml_create_prepd_w()
{
	Widget t_w;
	Arg args[20];
	int n;

	n = 0;
        XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg(args[n], XmNmarginWidth, 10);  n++;
	XtSetArg(args[n], XmNmarginHeight, 10);  n++;
	prepd_w = XmCreatePromptDialog (mlshell_w, "MoviePrefix", args,n);
	set_something (prepd_w, XmNcolormap, (XtArgVal)xe_cm);
	set_xmstring (prepd_w, XmNdialogTitle, "xephem Movie prefix");
	set_xmstring (prepd_w,XmNselectionLabelString,"Movie files prefix:");
	t_w = XmSelectionBoxGetChild(prepd_w, XmDIALOG_TEXT);
	defaultTextFN (t_w, 1, "mymovie", NULL);
	wtip (t_w,
	    "Enter the prefix for the set of files comprising this movie");
	XtUnmanageChild (XmSelectionBoxGetChild(prepd_w, XmDIALOG_HELP_BUTTON));
	XtAddCallback (prepd_w, XmNokCallback, ml_prefix_cb, NULL);
	XtAddCallback (prepd_w, XmNmapCallback, prompt_map_cb, NULL);
}

/* called when the Ok button is hit in the file prefix prompt */
/* ARGSUSED */
static void
ml_prefix_cb (Widget w, XtPointer client, XtPointer call)
{
	char *prefix;

	get_xmstring(w, XmNtextString, &prefix);

	if (strlen(prefix) == 0) {
	    xe_msg (1, "Please enter a prefix for the frame files.");
	    XtFree (prefix);
	    return;
	}

	saveMovie (prefix);
	XtFree (prefix);
}

/* save each frame to <privatedir>/<prefix><nnn>.png */
static void
saveMovie(char *prefix)
{
	unsigned char *raster;
	char fn[1024];
	unsigned int w, h;
	int i;

	for (i = 0; i < npixmaps; i++) {
	    /* get pixmap as rgb raster */
	    if (getPMPix (pixmaps[i], &raster, &w, &h) < 0)
		break;

	    /* create frame file */
	    sprintf (fn, "%s/%s%03d.png", getPrivateDir(), prefix, i);

	    /* write raster as png */
	    if (writePNG (fn, raster, w, h) < 0) {
		free (raster);
		break;
	    }

	    /* done */
	    free (raster);
	}
}

/* suck out the pixels from the given pixmap and return as a list of rgb bytes.
 * return size of image and malloced raster of bytes.
 * N.B. if return 0 caller must free *raster
 * return 0 if ok else -1 with xe_msg
 */
static int
getPMPix (Pixmap pm, unsigned char **raster, unsigned int *wp, unsigned int *hp)
{
	unsigned int w, h;
	int x, y;
	XColor xc;
	XImage *xim;
	unsigned char *rp;

	/* get server pixmap as a local image */
	getWH (pm, &w, &h);
	xim = XGetImage (XtD, pm, 0, 0, w, h, ~0, ZPixmap);
	if (!xim) {
	    xe_msg (1, "Can not create temp %dx%d image", w, h);
	    return (-1);
	}
	*raster = rp = malloc (w*h*3);
	if (!rp) {
	    xe_msg (1, "Can not malloc %dx%d temp raster", w, h);
	    free (xim->data);
	    xim->data = NULL;
	    XDestroyImage (xim);
	    return (-1);
	}

	/* scan image to retrieve rgb values */
	pixCache (NULL);
	for (y = 0; y < h; y++) {
	    for (x = 0; x < w; x++) {
		xc.pixel = XGetPixel (xim, x, y);
		pixCache (&xc);
		*rp++ = xc.red >> 8;
		*rp++ = xc.green >> 8;
		*rp++ = xc.blue >> 8;
	    }
	}

	/* free temp image */
	free (xim->data);
	xim->data = NULL;
	XDestroyImage (xim);

	/* return size */
	*wp = w;
	*hp = h;
	return (0);
}

/* given an rgb raster, save to the given file name in png format
 * return 0 if ok else -1 with xe_msg
 */
static int
writePNG (char *fn, unsigned char *raster, unsigned int w, unsigned int h)
{
	FILE *fp;
	png_structp png;
	png_infop info;
	png_bytepp row_pointers;
	int i;

	/* create file */
	fp = fopen (fn, "wb");
	if (!fp) {
	    xe_msg (1, "Can not create %s:\n%s", fn, syserrstr());
	    return (-1);
	}

	/* build list of pointers to each row */
	row_pointers = (png_bytepp) malloc (h * sizeof(png_bytep));
	if (!row_pointers) {
	    xe_msg (1, "Can not malloc %d PNG row pointers", h);
	    fclose (fp);
	    return (-1);
	}
	for (i = 0; i < h; i++)
	    row_pointers[i] = raster + i*(w*3);

	/* write png */
	png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct (png);
	if (setjmp (png_jmpbuf (png))) {
	    xe_msg (1, "PNG error");
	    png_destroy_write_struct (&png, &info);
	    free (row_pointers);
	    fclose (fp);
	    return (-1);
	}
	png_init_io(png, fp);
	png_set_IHDR (png, info, w, h, 8, PNG_COLOR_TYPE_RGB,
			    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			                PNG_FILTER_TYPE_DEFAULT);
	png_write_info (png, info);
	png_write_image (png, row_pointers);
	png_write_end (png, info);
	png_destroy_write_struct (&png, &info);
	free (row_pointers);

	/* check for io error */
	if (ferror(fp)) {
	    xe_msg (1, "Write error %s:\n%s", fn, syserrstr());
	    fclose (fp);
	    return (-1);
	}

	/* ok */
	fclose (fp);
	return (0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: moviemenu.c,v $ $Date: 2009/01/05 20:55:54 $ $Revision: 1.10 $ $Name:  $"};

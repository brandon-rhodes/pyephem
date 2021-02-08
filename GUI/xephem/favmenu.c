/* handle the favorites list.
 * independent copies of all favorites are kept here but pointers returned are
 * from main database if entry with same name exists.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/ScrolledW.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/ArrowB.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>

#include "xephem.h"
#include "lilxml.h"

/* pointer to object if loaded, else just name. also on/off control.
 * N.B. don't point op at &o because the list gets realloced and can move.
 */
typedef struct {
    Obj o;				/* defn */
    Obj *op;				/* db object else NULL for our o */
    int on;				/* whether to use */
} Favorite;

static Favorite * favGrow (void);
static void fav_up_cb (Widget w, XtPointer client, XtPointer call);
static void fav_dn_cb (Widget w, XtPointer client, XtPointer call);
static void fav_del_cb (Widget w, XtPointer client, XtPointer call);
static void fav_on_cb (Widget w, XtPointer client, XtPointer call);
static void fav_load_cb (Widget w, XtPointer client, XtPointer call);
static void fav_save_cb (Widget w, XtPointer client, XtPointer call);
static void fav_add_cb (Widget w, XtPointer client, XtPointer call);
static void fav_help_cb (Widget w, XtPointer client, XtPointer call);
static void fav_close_cb (Widget w, XtPointer client, XtPointer call);
static void fav_save_cb (Widget w, XtPointer client, XtPointer call);
static void fav_create (void);
static void showFavorites (void);
static void setup1Row (Widget rc_w, int i);
static void loadFavs ();
static void saveFavs (char *filename);
static void favRmAll(void);
static void bldExport(void);
static Obj *srchRealDB (char *name);
static void del_i(void);
static void chg_i(void);

/* the list */
static Favorite *favs;			/* malloced list of all favorites */
static int nfavs;			/* n in favs list */
static Obj **xfavs;			/* malloced list of fav Obj to export */
static int nxfavs;			/* n in xfavs list */

static Widget favshell_w;		/* main form shell */
static Widget sw_w;			/* list scrolled window */
static Widget save_w;			/* TF for name of file to save favs */
static Widget add_w;			/* TF for edb dfn of new favorite */

static int internal_scan;		/* flag to temp disable fav_scan */

static char favcategory[] = "Favorites";                /* Save category */

static char fav_suffix[] = ".fav";	/* suffix of Favorites files */
static char deffavfn[] = "favorites.fav";	/* default favorites to load */

/* used by query callbacks */
static int deli;
static Obj newo;

/* show, create if first time, the favorites window */
void
fav_manage()
{
	if (!favshell_w)
	    fav_create();

	XtPopup (favshell_w, XtGrabNone);
	set_something (favshell_w, XmNiconic, (XtArgVal)False);
}

/* add a new object to the favorites list if not already */
void
fav_add (Obj *op)
{
	Favorite *addfp;
	int i;

	watch_cursor(1);

	if (!favshell_w)
	    fav_create();

	/* replace if name is dup, else add */
	i = fav_already (op->o_name);
	if (i >= 0) {
	    addfp = &favs[i];
	    memcpy (&addfp->o, op, sizeof(Obj));
	    addfp->op = NULL;	/* local copy now for sure */
	} else {
	    addfp = favGrow();
	    memcpy (&addfp->o, op, sizeof(Obj));
	    addfp->op = (op->o_flags & FLDSTAR) ? NULL : op;
	}

	addfp->on = 1;

	/* display */
	bldExport();
	showFavorites();
	all_newfavs();

	watch_cursor(0);
}

/* return index into favs entry if given name is on favorites list, else -1 */
int
fav_already (char *name)
{
	int i;

	for (i = 0; i < nfavs; i++)
	    if (strcmp (favs[i].o.o_name, name) == 0)
		return (i);
	return (-1);
}

/* return a list of Favorite Obj* to be used.
 * each Obj * will point into db if loaded, else to our private copy.
 * oppp can be NULL if just want count.
 * N.B. caller should never free the list.
 */
int
fav_get_loaded (Obj ***oppp)
{
	if (!favshell_w)
	    fav_create();
	if (oppp)
	    *oppp = xfavs;
	return (nxfavs);
}

/* called when the database changes.
 * goal is to decide whether the copies we have here are now also in the db
 */
void
fav_newdb()
{
	int i;

	if (!favshell_w)
	    fav_create();

	/* update db pointer if really in db */
	for (i = 0; i < nfavs; i++)
	    favs[i].op = srchRealDB (favs[i].o.o_name);

	bldExport();
	showFavorites();
}


/* used by db_scan() to collect Favorites set on and not in the main database.
 * called with index into favs[] from which to scan and mask of desired types.
 * return next Obj with index set for next scan or NULL when no more.
 * N.B. internal calls from here can disable by setting internal_scan.
 */
Obj *
fav_scan (int *np, int typemask)
{
	int i;

	if (internal_scan)
	    return (NULL);

	for (i = *np; i < nfavs; i++) {
	    if (favs[i].on && !favs[i].op &&
				(OBJTYPE2MASK(favs[i].o.o_type) & typemask)) {
		*np = i+1;
		return (&favs[i].o);
	    }
	}

	return (NULL);
}


/* called to put up or remove the watch cursor.  */
void
fav_cursor (Cursor c)
{
	Window win;

	if (favshell_w && (win = XtWindow(favshell_w)) != 0) {
	    Display *dsp = XtDisplay(favshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create the favorites window */
static void
fav_create()
{
	static struct {
	    char *label;
	    XtCallbackProc cb;
	    char *tip;
	} btns[] = {
	    {"Close", fav_close_cb, "Close this window"},
	    {"Help", fav_help_cb, "Learn more"},
	};
	Widget w, bf_w, pb_w, sfm_w;
	Widget favform_w;
	Arg args[20];
	char *s[1];
	int i;
	int n;

	/* create the Favories shell and form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "Favorites"); n++;
	XtSetArg (args[n], XmNiconName, "Favorites"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	favshell_w = XtCreatePopupShell ("Favorites", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (favshell_w);
	set_something (favshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (favshell_w, "XEphem*Favorites.x", favcategory, 0);
	sr_reg (favshell_w, "XEphem*Favorites.y", favcategory, 0);
	sr_reg (favshell_w, "XEphem*Favorites.width", favcategory, 0);
	sr_reg (favshell_w, "XEphem*Favorites.height", favcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	favform_w = XmCreateForm (favshell_w, "FMForm", args, n);
	XtAddCallback (favform_w, XmNhelpCallback, fav_help_cb, 0);
	XtManageChild (favform_w);

	/* controls attached across the bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNfractionBase, 5*XtNumber(btns)-1); n++;
	bf_w = XmCreateForm (favform_w, "FBForm", args, n);
	XtManageChild (bf_w);

	    for (i = 0; i < XtNumber(btns); i++) {

		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNleftPosition, 5*i); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNrightPosition, 5*(i+1)-1); n++;
		w = XmCreatePushButton (bf_w, btns[i].label, args, n);
		wtip (w, btns[i].tip);
		XtAddCallback (w, XmNactivateCallback, btns[i].cb, 0);
		XtManageChild (w);
	    }

	/* favorites add control */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, bf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	pb_w = XmCreatePushButton (favform_w, "FATB", args, n);
	set_xmstring (pb_w, XmNlabelString, "Add edb:");
	wtip (pb_w, "Add or change a Favorite defined by the edb format entry at right");
	XtAddCallback (pb_w, XmNactivateCallback, fav_add_cb, NULL);
	XtManageChild (pb_w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, bf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, pb_w); n++;
	XtSetArg (args[n], XmNleftOffset, 5); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	add_w = XmCreateTextField (favform_w, "edb", args, n);
	XtAddCallback (add_w, XmNactivateCallback, fav_add_cb, NULL);
	sr_reg (add_w, NULL, favcategory, 0);
	wtip (add_w, "edb format definition of new or changed Favorite");
	XtManageChild (add_w);

	/* favorites file save control */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, add_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	pb_w = XmCreatePushButton (favform_w, "FSPB", args, n);
	set_xmstring (pb_w, XmNlabelString, "Save to:");
	wtip (pb_w, "Save above Favorites in file named at right");
	XtAddCallback (pb_w, XmNactivateCallback, fav_save_cb, NULL);
	XtManageChild (pb_w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, add_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, pb_w); n++;
	XtSetArg (args[n], XmNleftOffset, 5); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	save_w = XmCreateTextField (favform_w, "File", args, n);
	XtAddCallback (save_w, XmNactivateCallback, fav_save_cb, NULL);
	sr_reg (save_w, NULL, favcategory, 0);
	defaultTextFN (save_w, 0, deffavfn, NULL);
	wtip (save_w, "File in which to save current Favorites");
	XtManageChild (save_w);

	/* favorites load control */

	s[0] = fav_suffix;
	sfm_w = createFSM (favform_w, s, 1, "auxil", fav_load_cb);
	wtip (sfm_w, "Select existing Favorites file to load");
	set_xmstring (sfm_w, XmNlabelString, "Load file: ");

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, save_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetValues (sfm_w, args, n);

	/* SW for RC of entries stretches between top and row of controls */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, sfm_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	sw_w = XmCreateScrolledWindow (favform_w, "SW", args, n);
	XtManageChild (sw_w);

	    /* add a dummy RC just so it can be replaced when add entries */

	    n = 0;
	    w = XmCreateRowColumn (sw_w, "Dummy", args, n);
	    XtManageChild (w);

	/* init list */
	loadFavs(NULL);
	showFavorites();
}

/* display each of the favs entries in the dialog. */
static void
showFavorites()
{
	Widget ww;
	Arg args[20];
	int i, n;

	/* replace workWindow with a fresh RC */
	get_something (sw_w, XmNworkWindow, (XtArgVal)&ww);
	XtDestroyWidget (ww);
	n = 0;
	XtSetArg (args[n], XmNspacing, 0); n++;
	ww = XmCreateRowColumn (sw_w, "FavRC", args, n);
	set_something (sw_w, XmNworkWindow, (XtArgVal)ww);

	/* fill with info */
	for (i = 0; i < nfavs; i++)
	    setup1Row (ww, i);

	/* ok */
	XtManageChild (ww);
}

/* using info from favs[i] create its row in the favories window */
static void
setup1Row (rc_w, i)
Widget rc_w;
int i;
{
	Widget f_w, on_w, del_w, lbl_w, up_w, dn_w;
	Arg args[20];
	char buf[2048];
	int n;

	/* create the row's form */
	n = 0;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	f_w = XmCreateForm (rc_w, "FF", args, n);
	XtManageChild (f_w);

	/* PB to delete */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	del_w = XmCreatePushButton (f_w, "Del", args, n);
	XtAddCallback (del_w, XmNactivateCallback, fav_del_cb, (XtPointer)(long int)i);
	wtip (del_w, "Delete this Favorite");
	XtManageChild (del_w);

	/* arrow buttons for up and down */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, del_w); n++;
	XtSetArg (args[n], XmNarrowDirection, XmARROW_UP); n++;
	XtSetArg (args[n], XmNsensitive, i > 0); n++;
	up_w = XmCreateArrowButton (f_w, "Fup", args, n);
	XtAddCallback (up_w, XmNactivateCallback, fav_up_cb, (XtPointer)(long int)i);
	wtip (up_w, "Move this Favorite up one row");
	XtManageChild (up_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, up_w); n++;
	XtSetArg (args[n], XmNarrowDirection, XmARROW_DOWN); n++;
	XtSetArg (args[n], XmNsensitive, i < nfavs-1); n++;
	dn_w = XmCreateArrowButton (f_w, "Fdn", args, n);
	XtAddCallback (dn_w, XmNactivateCallback, fav_dn_cb, (XtPointer)(long int)i);
	wtip (dn_w, "Move this Favorite down one row");
	XtManageChild (dn_w);

	/* TB for on/off */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, dn_w); n++;
	XtSetArg (args[n], XmNset, favs[i].on); n++;
	on_w = XmCreateToggleButton (f_w, "Use", args, n);
	XtAddCallback (on_w, XmNvalueChangedCallback, fav_on_cb,(XtPointer)(long int)i);
	wtip (on_w, "Whether to use this entry");
	XtManageChild (on_w);

	/* label for name */
	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, on_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	lbl_w = XmCreateLabel (f_w, "FL", args, n);
	db_write_line (favs[i].op ? favs[i].op : &favs[i].o, buf);
	set_xmstring (lbl_w, XmNlabelString, buf);
	XtManageChild (lbl_w);
}

/* callback from the overall Help button.
 */
/* ARGSUSED */
static void
fav_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
	    "Collection of favorite objects",
	};

	hlp_dialog ("Favorites", msg, sizeof(msg)/sizeof(msg[0]));
}

/* callback from the Close PB */
/* ARGSUSED */
static void
fav_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtPopdown (favshell_w);
}

/* callback to load favorites list from file.
 * file name is lable of htsi widget
 */
/* ARGSUSED */
static void
fav_load_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *fn;

	get_xmstring (w, XmNlabelString, &fn);

	loadFavs(fn);
	showFavorites();
	all_newfavs();

	XtFree (fn);
}

/* callback to Save the current favorites to file named in save_w
 * N.B. do not use call arg, this is used both by PB and TF
 */
/* ARGSUSED */
static void
fav_save_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024], *fn;
	char *txt;

	/* get file name */
	fn = txt = XmTextFieldGetString (save_w);
	if (!strstr (txt, fav_suffix)) {
	    sprintf (fn = buf, "%s%s", txt, fav_suffix);
	    XmTextFieldSetString (save_w, fn);
	}

	/* save */
	saveFavs (fn);

	/* clean up */
	if (confirm())
	    xe_msg (1, "%s saved", fn);
	XtFree (txt);
}

/* callback to Add a new favorite or possibly modify an existing fav.
 * N.B. do not use call arg, this is used both by PB and TF
 */
/* ARGSUSED */
static void
fav_add_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024];
	char *txt;

	/* get edb dfn, crack and add/change if ok */
	txt = XmTextFieldGetString (add_w);
	if (db_crack_line (txt, &newo, NULL, 0, buf) < 0) {
	    /* unknown */
	    xe_msg (1, buf);
	} else if (srchRealDB (newo.o_name)) {
	    /* already in db */
	    xe_msg (1, "%s is already in loaded database", newo.o_name);
	} else if ((deli = fav_already (newo.o_name)) >= 0) {
	    /* already in favs list */
	    if (confirm()) {
		(void) sprintf (buf, "Change \"%s\"?", favs[deli].o.o_name);
		query (favshell_w, buf, "Yes -- change", "No -- ignore", NULL,
							    chg_i, NULL, NULL);
	    } else
		chg_i();
	} else {
	    /* add to favs */
	    Favorite *newfp = favGrow();
	    newfp->o = newo;
	    newfp->op = NULL;
	    newfp->on = 1;

	    /* display */
	    bldExport();
	    showFavorites();
	    all_newfavs();
	}

	XtFree (txt);
}

/* used to delete favorite entry deli */
static void
del_i()
{
	/* remove deli from favs list */
	memmove (&favs[deli], &favs[deli+1], (--nfavs-deli)*sizeof(Favorite));

	/* show list now */
	watch_cursor (1);
	bldExport();
	showFavorites();
	all_newfavs();
	watch_cursor (0);
}

/* used to change favorite[deli] to newo */
static void
chg_i()
{
	/* update entry */
	memcpy (&favs[deli].o, &newo, sizeof(Obj));
	favs[deli].op = NULL;	/* local copy now for sure */

	/* update display */
	watch_cursor (1);
	bldExport();
	showFavorites();
	all_newfavs();
	watch_cursor (0);
}

/* called when a Use toggle changes.
 * client is index into favs[]
 */
/* ARGSUSED */
static void
fav_on_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int wanton = XmToggleButtonGetState (w);
	int i = (long int)client;

	watch_cursor(1);
	favs[i].on = wanton;
	bldExport();
	all_newfavs();
	watch_cursor(0);
}

/* called when an Up arrow button is hit in the favorites list.
 * client is index into favs[]
 */
/* ARGSUSED */
static void
fav_up_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i = (long int)client;

	Favorite tmp = favs[i-1];
	favs[i-1] = favs[i];
	favs[i] = tmp;

	bldExport();
	showFavorites();
	all_newfavs();
}

/* called when a Down arrow button is hit in the favorites list.
 * client is index into favs[]
 */
/* ARGSUSED */
static void
fav_dn_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int i = (long int)client;

	Favorite tmp = favs[i+1];
	favs[i+1] = favs[i];
	favs[i] = tmp;

	bldExport();
	showFavorites();
	all_newfavs();
}

/* called when a Delete button is hit in the favorites window.
 * client is index into favs[]
 */
/* ARGSUSED */
static void
fav_del_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* save index */
	deli = (long int)client;

	/* confirm */
	if (confirm()) {
	    char buf[1024];

	    (void) sprintf (buf, "Delete \"%s\"?", favs[deli].o.o_name);
	    query (favshell_w, buf, "Yes -- delete", "No -- keep it", NULL,
							    del_i, NULL, NULL);
	} else
	    del_i();
}

/* add one more to favs and return ponter to new */
static Favorite *
favGrow()
{
	Favorite *newfp;

	favs = (Favorite *) XtRealloc ((char*)favs, (nfavs+1)*sizeof(Favorite));
	newfp = &favs[nfavs++];
	zero_mem (newfp, sizeof(*newfp));
	return (newfp);
}

static void
favRmAll()
{
	XtFree ((char *)favs);
	favs = NULL;
	nfavs = 0;
}

/* build xfavs, list of Obj* to be exported as the current set of favs */
static void
bldExport()
{
	int i;

	xfavs = (Obj **) XtRealloc ((char *)xfavs, nfavs*sizeof(Obj*)); /* WC */
	for (nxfavs = i = 0; i < nfavs; i++)
	    if (favs[i].on)
		xfavs[nxfavs++] = favs[i].op ? favs[i].op : &favs[i].o;
}

/* set the current set of favorites from fn them build the export obj list.
 * if !fn, use save_w
 */
static void
loadFavs(char *fn)
{
	Favorite *newfp;
	char msg[1024];
	XMLEle *root, *ep;
	LilXML *xp;
	FILE *fp;
	char buf[1024];

	/* get file name */
	if (!fn) {
	    char *txt = XmTextFieldGetString (save_w);
	    if (!strstr (txt, fav_suffix))
		sprintf (buf, "%s%s", txt, fav_suffix);
	    else
		strcpy (buf, txt);
	    XtFree (txt);
	    fn = buf;
	}

	/* open file */
	fp = fopend (fn, "auxil", "r");
	if (!fp) 
	    return;		/* already informed user */

	/* read */
	xp = newLilXML();
	root = readXMLFile (fp, xp, msg);
	delLilXML (xp);
	fclose (fp);
	if (!root) {
	    xe_msg (1, "%s: %s", fn, msg[0] ? msg : "malformed xml");
	    return;
	}
	if (strcmp (tagXMLEle(root), "Favorites")) {
	    xe_msg (1, "%s: not a Favorites file", fn);
	    delXMLEle (root);
	    return;
	}

	/* remove current set */
	favRmAll();

	/* add new from file */
	for (ep = nextXMLEle(root,1); ep != NULL; ep = nextXMLEle(root,0)) {
	    Obj o;

	    if (strcmp (tagXMLEle(ep), "favorite"))
		continue;
	    if (db_crack_line (pcdataXMLEle(ep), &o, NULL, 0, msg) < 0) {
		xe_msg (1, "Bad favorite: %s: %s", pcdataXMLEle(ep), msg);
		continue;
	    }

	    newfp = favGrow();
	    newfp->o = o;
	    newfp->op = srchRealDB (newfp->o.o_name);
	    newfp->on = strcmp (findXMLAttValu (ep, "on"), "true") == 0;
	}

	/* clean up */
	delXMLEle (root);

	/* build export list */
	bldExport();
}

/* save the current set of favorites to the given file.
 */
static void
saveFavs(char *fn)
{
	char line[1024];
	FILE *fp;
	int i;

	/* create */
	fp = fopend (fn, NULL, "w");
	if (!fp)
	    return;		/* already informed user */

	/* write */
	fprintf (fp, "<Favorites>\n");
	for (i = 0; i < nfavs; i++) {
	    db_write_line (favs[i].op ? favs[i].op : &favs[i].o, line);
	    fprintf (fp, "  <favorite on='%s'>%s</favorite>\n",
					favs[i].on ? "true" : "false", line);
	}
	fprintf (fp, "</Favorites>\n");

	/* finished */
	fclose (fp);
}

/* scan just the db (not favs) for entry with given name and return
 * pointer else NULL
 * N.B. this basically removes from db_scan() what fav_scan() adds
 */
static Obj *
srchRealDB (char *name)
{
	DBScan dbs;
	Obj *op;
	
	/* flag to disable fav_scan() while we use db_scan() */
	internal_scan = 1;

	for (db_scaninit (&dbs,ALLM,NULL,0); (op = db_scan(&dbs)) != NULL; )
	    if (!strcmp (op->o_name, name))
		break;

	internal_scan = 0;
	return (op);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: favmenu.c,v $ $Date: 2012/10/06 02:19:35 $ $Revision: 1.29 $ $Name:  $"};

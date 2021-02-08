/* code to manage the stuff on the search menu.
 * to show all names, entries are built from db_dups() list, not the real db.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <fnmatch.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrollBar.h>
#include <Xm/SelectioB.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>

#include "xephem.h"


static void obj_unsetinfo (void);
static void obj_setinfo (Obj *op);
static void obj_create_shell (void);
static void obj_ctl_cb (Widget w, XtPointer client, XtPointer call);
static void obj_help (void);
static void obj_type_cb (Widget w, XtPointer client, XtPointer call);
static void obj_select_cb (Widget w, XtPointer client, XtPointer call);
static void obj_scroll_cb (Widget w, XtPointer client, XtPointer call);
static void obj_srch_cb (Widget w, XtPointer client, XtPointer call);
static void obj_newlist (void);
static void obj_setnames (void);
static int srch_match (char *n1, char *n2);
static int fpFld (char *buf, int nc, double f, char *title, char *units);
static int sizeFld (char *buf, int nc, double f, char *title);
static int dateFld (char *buf, int nc, double Mjd, char *title);
static int yearFld (char *buf, int nc, double Mjd, char *title);
static int magFld (char *buf, int nc, double m1, double m2, char *title);
static int sexaFld (char *buf, int nc, double a, char *title, char *units,
    int w, int p);
static void dbFld (Obj *op);
static int strFld (char *buf, int nc, char *str, char *title, char *units);
static int strcenterFld (char *buf, int nc, char *str);
static int is_ss (Obj *op);
static int is_es (Obj *op);
static int is_st (Obj *op);
static int (*ty_test(void))(Obj *op);


#define	NBTNS		20		/* number of buttons in name list */
#define	SB_WIDTH	12		/* scroll bar width, pixels */

static Widget objshell_w;		/* overall shell */
static Widget scroll_w;			/* scroll widget */
static Widget copt_w;			/* text widet to show cop info */
static Widget showgal_w;		/* PB whether display gallery image */
static Widget edb_w;			/* text widet to show edb form */
static Widget srch_w;			/* the search text field */
static Widget ss_w;			/* display only solar system objs */
static Widget ds_w;			/* display only deep sky objs */
static Widget es_w;			/* display only earth sat objs */
static Widget st_w;			/* display only stellar objs */
static Widget bi_w;			/* display only binary stars */
static Widget all_w;			/* display all objs */
static Widget tel_w;			/*PB to send obj to external app */

static Widget namepb_w[NBTNS];		/* each name selection button */
static Obj *cop;			/* current obj being displayed */
static int newdb;			/* set when db changed & we are down */
static int myfav;			/* avoid rebuild when we set fav here */
static DupName *mtdups;			/* dupnames matching current type */
static int nmtdups;			/* number of dupnames matching type */
static int localdups;			/* whether mtdups is malloc or shared */
static int topi;			/* mtdups index of top button */

/* bottom control panel buttons */
enum {
    POINT, SETTEL, FAVORITE, SHOWGAL, CANCEL, HELP
};

static char objcategory[] = "Date index";	/* Save category */

/* called by the main menu pick.
 * create the main form, if this is the first time we've been called.
 * then we go for it.
 */
void
obj_manage()
{
	if (!objshell_w) {
	    obj_create_shell();
	    newdb = 1;	/* force a fresh list */
	}
	
	/* we don't bother to update the scrolled area while we are down
	 * but this flag tells us it has changed so we do it when coming up.
	 */
	if (newdb) {
	    obj_newlist();
	    newdb = 0;
	}

	XtSetSensitive (tel_w, telIsOn());
	XtPopup (objshell_w, XtGrabNone);
	set_something (objshell_w, XmNiconic, (XtArgVal)False);
}

/* called when a font has changed */
void
obj_newres()
{
	if (cop)
	    obj_setinfo(cop);
}

/* called when the db has changed.
 * we don't care whether it was appended or deleted because we need to 
 * rebuild the whole list either way.
 * if we are not up, don't bother building a new scrolled list, but 
 * set a flag so we know we need to the next time we do come up.
 * N.B. ignore the case when we are called because we set a new fav.
 */
/* ARGSUSED */
void
obj_newdb(appended)
int appended;
{
	if (!isUp(objshell_w)) {
	    newdb = 1;
	    return;
	}

	if (!myfav)
	    obj_newlist();
}

/* called to put up or remove the watch cursor.  */
void
obj_cursor (c)
Cursor c;
{
	Window win;

	if (objshell_w && (win = XtWindow(objshell_w)) != 0) {
	    Display *dsp = XtDisplay(objshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* like strcmp() but ignores case and blanks and takes care to compare numerics
 * by value.
 */
int
strnncmp (char *s1, char *s2)
{
	int d;
	int p;
	int q = 0;

	do {
	    p = *s1;
	    if (p == ' ') {
		s1++;
		d = 0;
		continue;
	    }
	    q = *s2++;
	    if (q == ' ') {
		d = 0;
		continue;
	    }
	    s1++;
	    if (p <= '9' && isdigit(p) && isdigit(q)) {
		int np = 0, nq = 0;
		do {
		    np = np*10 + p - '0';
		    p = *s1;
		} while (isdigit(p) && s1++);
		do {
		    nq = nq*10 + q - '0';
		    q = *s2;
		} while (isdigit(q) && s2++);
		d = np - nq;
	    } else {
	        if (p >= 'a')
		    p -= ('a' - 'A');
	        if (q >= 'a')
		    q -= ('a' - 'A');
		d = p - q;
	    }
	} while (!d && (p || q));

	return (d);
}

/* called once to build the basic shell and form.
 */
static void
obj_create_shell ()
{
	typedef struct {
	    int id;
	    Widget *wp;
	    char *name;
	    char *label;
	    char *tip;
	} Btns;
	static Btns ctlbtns[] = {
	    {CANCEL, NULL, NULL, "Close",
	    	"Center Sky View on this Object and mark"},
	    {POINT, NULL, NULL, "Sky Point",
	    	"Mark this object on Sky View, recentering if out of field"},
	    {SHOWGAL, &showgal_w, NULL, "Show in\nGallery",
	        "Show the Gallery image of this target"},
	    {FAVORITE, NULL, NULL, "Save as\nFavorite",
		"Add this object to list of Favorites"},
	    {SETTEL, &tel_w, NULL, "Telescope\nGoto",
		"Send coordinates of this Object to an external application"},
	    {HELP, NULL, NULL, "Help",
		"Display detailed help information"},
	};
	static Btns typebtns[] = {
	    {0, &all_w, "All",     "All",       "List objects of all types"},
	    {0, &ds_w,  "DeepSky", "Deep sky",  "List deep sky objects"},
	    {0, &st_w,  "Stellar", "Stellar",   "List stellar objects"},
	    {0, &bi_w,  "Binaries","Binaries",  "List binary systems"},
	    {0, &ss_w,  "SolSys",  "Sol sys",   "List objects in solar system"},
	    {0, &es_w,  "EarthSat","Earth sat", "List earth satellites"},
	};
	Widget typrb_w;
	Widget namerc_w;
	Widget objform_w;
	Widget ctl_w;
	Widget srchpb_w;
	Widget edbl_w;
	Widget w;
	Arg args[20];
	int n;
	int i;

	/* create shell */
	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Loaded Object Index"); n++;
	XtSetArg (args[n], XmNiconName, "Objects"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	objshell_w = XtCreatePopupShell ("Objects", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (objshell_w);
	set_something (objshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (objshell_w, "XEphem*Objects.x", objcategory, 0);
	sr_reg (objshell_w, "XEphem*Objects.y", objcategory, 0);

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNallowOverlap, False); n++;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 5); n++;
	XtSetArg (args[n], XmNverticalSpacing, 5); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	objform_w = XmCreateForm (objshell_w, "ObjF", args, n);
	set_something (objform_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (objform_w, XmNhelpCallback, obj_ctl_cb, (XtPointer)HELP);
	XtManageChild (objform_w);

	/* make a form to hold the bottom control buttons */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNfractionBase, 35); n++;
	ctl_w = XmCreateForm (objform_w, "CtlForm", args, n);
	XtManageChild (ctl_w);

	    /* make the control buttons */

	    for (i = 0; i < XtNumber(ctlbtns); i++) {
		Btns *bp = &ctlbtns[i];
		n = 0;
		XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNtopOffset, 10); n++;
		XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
		XtSetArg (args[n], XmNbottomOffset, 10); n++;
		XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNleftPosition, i*6); n++;
		XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
		XtSetArg (args[n], XmNrightPosition, 5 + i*6); n++;
		w = XmCreatePushButton (ctl_w, "OCB", args, n);
		XtAddCallback (w, XmNactivateCallback, obj_ctl_cb,
							(XtPointer)(long int)bp->id);
		set_xmstring (w, XmNlabelString, bp->label);
		wtip (w, bp->tip);
		XtManageChild (w);
		if (bp->wp)
		    *bp->wp = w;
	    }

	/* text field to show edb form */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, ctl_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	edbl_w = XmCreateLabel(objform_w, "EL", args, n);
	set_xmstring (edbl_w, XmNlabelString, ".edb:");
	XtManageChild (edbl_w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, ctl_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, edbl_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNeditable, False); n++;
	XtSetArg (args[n], XmNcursorPositionVisible, True); n++; /* scrollable*/
	XtSetArg (args[n], XmNblinkRate, 0); n++;
	edb_w = XmCreateTextField(objform_w, "OT", args, n);
	wtip (edb_w, "XEphem's .edb file format for this object");
	XtManageChild (edb_w);

	/* make the type control radio box . */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); n++;
	XtSetArg (args[n], XmNnumColumns, XtNumber(typebtns)); n++;
	typrb_w = XmCreateRadioBox (objform_w, "TypeRB", args, n);
	XtManageChild (typrb_w);

	    for (i = 0; i < XtNumber(typebtns); i++) {
		Btns *bp = &typebtns[i];
		n = 0;
		XtSetArg (args[n], XmNmarginLeft, 2); n++;
		w = XmCreateToggleButton (typrb_w, bp->name, args, n);
		XtAddCallback (w, XmNvalueChangedCallback, obj_type_cb, 0);
		set_xmstring (w, XmNlabelString, bp->label);
		wtip (w, bp->tip);
		sr_reg (w, NULL, objcategory, 0);
		XtManageChild (w);
		*bp->wp = w;
	    }

	/* make the scrollbar on the right edge */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, typrb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, edb_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNpageIncrement, NBTNS-1); n++;
	XtSetArg (args[n], XmNminimum, 0); n++;
	XtSetArg (args[n], XmNwidth, SB_WIDTH); n++;
	scroll_w = XmCreateScrollBar (objform_w, "ScrollB", args, n);
	XtAddCallback (scroll_w, XmNdragCallback, obj_scroll_cb, 0);
	XtAddCallback (scroll_w, XmNvalueChangedCallback, obj_scroll_cb, 0);
	XtManageChild (scroll_w);

	/* make an rc to hold the scroll buttons */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, typrb_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, edb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 60); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, scroll_w); n++;
	namerc_w = XmCreateRowColumn (objform_w, "NameRC", args, n);
	XtManageChild (namerc_w);

	    for (i = 0; i < NBTNS; i++) {
		n = 0;
		w = XmCreatePushButton (namerc_w, "NamePB", args, n);
		XtAddCallback (w, XmNactivateCallback, obj_select_cb,
								(XtPointer)(long int)i);
		XtManageChild (w);
		wtip (w, "Click to view any Object's definition");
		namepb_w[i] = w;
	    }


	/* make the search pb and its text field */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, typrb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	srchpb_w = XmCreatePushButton (objform_w, "SrchL", args, n);
	set_xmstring (srchpb_w, XmNlabelString, "Search:");
	XtAddCallback (srchpb_w, XmNactivateCallback, obj_srch_cb, NULL);
	wtip (srchpb_w,
	    "Scan for next Object in memory whose name matches glob at right");
	XtManageChild (srchpb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, typrb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, srchpb_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, namerc_w); n++;
	XtSetArg (args[n], XmNmaxLength, MAXNM-1); n++;
	srch_w = XmCreateTextField (objform_w, "SrchTF", args, n);
	wtip (srch_w, "Enter glob, click Search or type RETURN to search");
	XtAddCallback (srch_w, XmNactivateCallback, obj_srch_cb, NULL);
	XtManageChild (srch_w);

#if XmVersion >= 1001
	/* init kb focus here if possible */
	XmProcessTraversal (srch_w, XmTRAVERSE_CURRENT);
	XmProcessTraversal (srch_w, XmTRAVERSE_CURRENT);
#endif

	/* make a R/O text field to show current object info */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, srch_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, edb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, namerc_w); n++;
	XtSetArg (args[n], XmNeditable, False); n++;
	XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	XtSetArg (args[n], XmNblinkRate, 0); n++;
	copt_w = XmCreateText(objform_w, "OT", args, n);
	wtip (copt_w, "Database fields for selected object");
	XtManageChild (copt_w);
}

/* display info about op
 */
static void
obj_setinfo(Obj *op)
{
	Now *np = mm_get_now();
	DupName *dnp;
	int ndn = db_dups (&dnp);
	char buf[2048];
	DBCat *dbcp;
	short nc;
	int i, nn, l;
	int pm;

	if (!objshell_w)
	    return;

	/* get field width */
	XtVaGetValues (copt_w, XmNcolumns, &nc, NULL);

	/* format, depending on class */

	l = 0;
	l += strcenterFld (buf+l, nc, op->o_name);
	l += strcenterFld (buf+l, nc, obj_description(op));

	switch (op->o_type) {
	case BINARYSTAR:	/* FALLTHRU part way */
	case FIXED:
	    pm = op->f_pmRA || op->f_pmdec;
	    l += sexaFld (buf+l, nc, radhr(op->f_RA), "RA", " H:M:S", 4,360000);
	    if (pm)
		l += fpFld (buf+l, nc, cos(op->f_dec)*op->f_pmRA/1.327e-11,
							    "RA PM"," mas/yr on sky");
	    l += sexaFld (buf+l, nc, raddeg(op->f_dec), "Dec"," D:M:S",3,36000);
	    if (pm)
		l += fpFld (buf+l, nc,op->f_pmdec/1.327e-11,"Dec PM"," mas/yr");
	    l += yearFld (buf+l, nc, op->f_epoch, "Equinox");
	    l += fpFld (buf+l, nc, get_mag(op), "Magnitude", NULL);
	    if (op->f_size > 0) {
		double r = get_ratio(op);
		if (r < .99) {
		    l += sizeFld (buf+l, nc, op->f_size, "Major axis");
		    l += sizeFld (buf+l, nc, op->f_size*get_ratio(op),
								"Minor axis");
		} else {
		    l += sizeFld (buf+l, nc, op->f_size, "Size");
		}
	    }
	    if (op->f_spect[0]) {
                char *label = "Spectral class";
		char tbuf[sizeof(op->f_spect)+1];
		strncpy (tbuf, op->f_spect, sizeof(op->f_spect));
		tbuf[sizeof(op->f_spect)] = '\0';
                if (op->o_type == FIXED) {
                    switch (op->f_class) {
                    case 'S': case 'B': case 'D': case 'M': case 'V': case 'T':
                        break;
                    default:
                        label = "Category";
                        break;
                    }
		}
                l += strFld (buf+l, nc, tbuf, label, NULL);
	    }
	    if (op->o_type == FIXED && op->f_pa > 0)
		l += fpFld (buf+l, nc, raddeg(get_pa(op)), "Position angle",
								"° E of N");
	    
	    /* stop here if FIXED */
	    if (op->o_type == FIXED)
		break;

	    l += fpFld (buf+l, nc, op->b_2mag/MAGSCALE, "2 Magnitude", NULL);
	    if (op->b_2spect[0]) {
		char tbuf[sizeof(op->b_2spect)+1];
		strncpy (tbuf, op->b_2spect, sizeof(op->b_2spect));
		tbuf[sizeof(op->b_2spect)] = '\0';
		l += strFld (buf+l, nc, tbuf, "2 Spectral class", NULL);
	    }

	    for (i = 0; i < op->b_nbp; i++) {
		char name[64];
		sprintf (name, "Epoch %d", i+1);
		l += fpFld (buf+l, nc, op->b_bp[i].bp_ep, name, NULL);
		sprintf (name, "Separation %d", i+1);
		l += fpFld (buf+l, nc, op->b_bp[i].bp_sep, name, "\"");
		sprintf (name, "Position angle %d", i+1);
		l += fpFld (buf+l,nc,raddeg(op->b_bp[i].bp_pa),name,"° E of N");
	    }
	    if (op->b_nbp == 0) {
		l += fpFld(buf+l,nc,op->b_bo.bo_P,"Period", " years");
		l += fpFld(buf+l,nc,op->b_bo.bo_T,"Epoch of periastron", 0);
		l += fpFld(buf+l,nc,op->b_bo.bo_e,"Eccentricity", NULL);
		l += fpFld(buf+l,nc,op->b_bo.bo_o,"Argument of periastron","°");
		l += fpFld(buf+l,nc,op->b_bo.bo_O,"Longitude of node", "°");
		l += fpFld(buf+l,nc,op->b_bo.bo_i,"Inclination to sky","°");
		l += fpFld(buf+l,nc,op->b_bo.bo_a,"Semi major axis", "\"");
	    }

	    /* finished with BINARY */
	    break;

	case ELLIPTICAL:
	    l += fpFld (buf+l, nc, op->e_inc, "Inclination", "°");
	    l += fpFld (buf+l, nc, op->e_Om, "Long of Asc node", "°");
	    l += fpFld (buf+l, nc, op->e_om, "Arg of Peri", "°");
	    l += fpFld (buf+l, nc, op->e_e, "Eccentricity", NULL);
	    l += fpFld (buf+l, nc, op->e_a, "Semi-major axis", " AU");
	    l += fpFld (buf+l, nc, op->e_M, "Mean anomaly", "°");
	    if (op->e_size > 0)
		l += sizeFld (buf+l, nc, op->e_a, "Size @ 1AU");
	    l += dateFld (buf+l, nc, op->e_cepoch, "Epoch");
	    l += yearFld (buf+l, nc, op->e_epoch, "Equinox");
	    if (op->e_startok)
		l += dateFld (buf+l, nc, op->e_startok, "Valid from");
	    if (op->e_endok)
		l += dateFld (buf+l, nc, op->e_endok, "Valid to");
	    if (op->e_mag.whichm == MAG_gk)
		l += magFld (buf+l, nc, op->e_mag.m1, op->e_mag.m2, "Mag, g k");
	    else
		l += magFld (buf+l, nc, op->e_mag.m1, op->e_mag.m2, "Mag, H G");
	    break;

	case HYPERBOLIC:
	    l += fpFld (buf+l, nc, op->h_inc, "Inclination", "°");
	    l += fpFld (buf+l, nc, op->h_Om, "Long of Asc node", "°");
	    l += fpFld (buf+l, nc, op->h_om, "Arg of Peri", "°");
	    l += fpFld (buf+l, nc, op->h_e, "Eccentricity", NULL);
	    l += fpFld (buf+l, nc, op->h_qp, "Peri distance", " AU");
	    if (op->h_size > 0)
		l += sizeFld (buf+l, nc, op->h_size, "Size @ 1AU");
	    l += dateFld (buf+l, nc, op->h_ep, "Epoch of Peri");
	    l += yearFld (buf+l, nc, op->h_epoch, "Equinox");
	    if (op->h_startok)
		l += dateFld (buf+l, nc, op->h_startok, "Valid from");
	    if (op->h_endok)
		l += dateFld (buf+l, nc, op->h_endok, "Valid to");
	    l += magFld (buf+l, nc, op->h_g, op->h_k, "Mag, g k");
	    break;

	case PARABOLIC:
	    l += fpFld (buf+l, nc, op->p_inc, "Inclination", "°");
	    l += fpFld (buf+l, nc, op->p_Om, "Long of Asc node", "°");
	    l += fpFld (buf+l, nc, op->p_om, "Arg of Peri", "°");
	    l += fpFld (buf+l, nc, op->p_qp, "Peri distance", " AU");
	    if (op->p_size > 0)
		l += sizeFld (buf+l, nc, op->p_size, "Size @ 1AU");
	    l += dateFld (buf+l, nc, op->p_ep, "Epoch of Peri");
	    l += yearFld (buf+l, nc, op->p_epoch, "Equinox");
	    if (op->p_startok)
		l += dateFld (buf+l, nc, op->p_startok, "Valid from");
	    if (op->p_endok)
		l += dateFld (buf+l, nc, op->p_endok, "Valid to");
	    l += magFld (buf+l, nc, op->p_g, op->p_k, "Mag, g k");
	    break;

	case EARTHSAT:
	    l += fpFld (buf+l, nc, op->es_inc, "Inclination", "°");
	    l += fpFld (buf+l, nc, op->es_raan, "RA of Asc node", "°");
	    l += fpFld (buf+l, nc, op->es_ap, "Arg of Peri", "°");
	    l += fpFld (buf+l, nc, op->es_e, "Eccentricity", NULL);
	    l += fpFld (buf+l, nc, op->es_M, "Mean anomaly", "°");
	    l += fpFld (buf+l, nc, op->es_n, "Mean motion", " Rev/Day");
	    l += dateFld (buf+l, nc, op->es_epoch, "Epoch");
	    l += fpFld (buf+l, nc, mjd - op->es_epoch, "Age", " days");
	    if (op->es_startok)
		l += dateFld (buf+l, nc, op->es_startok, "Valid from");
	    if (op->es_endok)
		l += dateFld (buf+l, nc, op->es_endok, "Valid to");
	    l += fpFld (buf+l, nc, op->es_decay, "Orbit decay", " Rev/Day^2");
	    l += fpFld (buf+l, nc, op->es_drag,"Drag coefficient"," ERad^-1");
	    l += fpFld (buf+l, nc, (double)op->es_orbit, "Orbit number", NULL);
	    break;

	case PLANET:
	    /* fields not from catalog */
	    break;
	}

	/* alternate names */
	for (nn = i = 0; i < ndn; i++) {
	    if (dnp[i].op == op) {
		nn++;
		if (strcmp (dnp[i].nm, op->o_name))
		    l += strFld (buf+l, nc, dnp[i].nm, "Alternate name", NULL);
	    }
	}
	if (!nn) {
	    printf ("%s not in dupnames\n", op->o_name);
	    abort();
	}

	/* catalog name */
	dbcp = db_opfindcat (op);
	if (!dbcp) {
	    printf ("Object %s disappeared from its catalog\n", op->o_name);
	    abort();
	}
	if (nn > 1)
	    l += strFld (buf+l, nc, dbcp->name, "Defining catalog", NULL);
	else
	    l += strFld (buf+l, nc, dbcp->name, "Catalog", NULL);

	/* display */
	XmTextSetString (copt_w, buf);

	/* whether in gallery */
	XtSetSensitive (showgal_w, gal_opfind(op) >= 0);

	/* .edb too */
	dbFld (op);
}

static int
fpFld (char *buf, int nc, double f, char *title, char *units)
{
	char fbuf[32];
	int tl, fl, ul, gl;

	tl = strlen (title);
	fl = sprintf (fbuf, "%g", f);
	if (!units)
	    units = "";
	ul = strlen (units);
	gl = nc - (tl+fl+ul);
	return (sprintf (buf, "%s%*s%s%s\n", title, gl, "", fbuf, units));
}

static int
sizeFld (char *buf, int nc, double f, char *title)
{
	char fbuf[32];
	int tl, fl, gl;

	tl = strlen (title);
	if (f < 60)
	    fl = sprintf (fbuf, "%g\"", f);
	else if (f < 3600)
	    fl = sprintf (fbuf, "%.1f'", f/60);
	else
	    fl = sprintf (fbuf, "%.1f°", f/3600);
	gl = nc - (tl+fl);
	return (sprintf (buf, "%s%*s%s\n", title, gl, "", fbuf));
}

static int
sexaFld (char *buf, int nc, double a, char *title, char *units, int w, int p)
{
	char sbuf[32];
	int tl, sl, ul, gl;

	tl = strlen (title);
	fs_sexa (sbuf, a, w, p);
	sl = strlen (sbuf);
	if (!units)
	    units = "";
	ul = strlen (units);
	gl = nc - (tl+sl+ul);
	return (sprintf (buf, "%s%*s%s%s\n", title, gl, "", sbuf, units));
}

static int
magFld (char *buf, int nc, double m1, double m2, char *title)
{
	char mbuf[64];
	int ml, tl, gl;

	ml = sprintf (mbuf, "%g %g", m1, m2);
	tl = strlen (title);
	gl = nc - (tl+ml);
	return (sprintf (buf, "%s%*s%s\n", title, gl, "", mbuf));
}

static int
yearFld (char *buf, int nc, double Mjd, char *title)
{
	char ybuf[32];
	int tl, yl, gl;
	double y;

	tl = strlen (title);
	mjd_year (Mjd, &y);
	yl = sprintf (ybuf, "%g", y);
	gl = nc - (tl+yl);
	return (sprintf (buf, "%s%*s%s\n", title, gl, "", ybuf));
}

static int
dateFld (char *buf, int nc, double Mjd, char *title)
{
	char dbuf[64];
	int tl, dl, gl;

	tl = strlen (title);
	fs_date (dbuf, pref_get(PREF_DATE_FORMAT), Mjd);
	dl = strlen (dbuf);
	gl = nc - (tl+dl);
	return (sprintf (buf, "%s%*s%s\n", title, gl, "", dbuf));
}

static int
strFld (char *buf, int nc, char *str, char *title, char *units)
{
	int tl, sl, ul, gl;

	tl = strlen (title);
	sl = strlen (str);
	if (!units)
	    units = "";
	ul = strlen (units);
	gl = nc - (tl+sl+ul);
	return (sprintf (buf, "%s%*s%s%s\n", title, gl, "", str, units));
}

static int
strcenterFld (char *buf, int nc, char *str)
{
	int sl = strlen (str);
	int lg = (nc - sl)/2;
	int rg = nc - (sl + lg);
	return (sprintf (buf, "%*s%s%*s\n", lg, "", str, rg, ""));
}

static void
dbFld (Obj *op)
{
	char dbuf[1024];

	db_write_line (op, dbuf);
	XmTextFieldSetString (edb_w, dbuf);
}

/* erase info about object
 */
static void
obj_unsetinfo()
{
	if (!objshell_w)
	    return;

	XmTextSetString (copt_w, "");
}

/* callback from any of the botton control buttons.
 * id is in client.
 */
/* ARGSUSED */
static void
obj_ctl_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int id = (long int) client;

	switch (id) {
	case CANCEL:
	    XtPopdown (objshell_w);
	    break;

	case POINT:
	    if (cop) {
		db_update (cop);
		sv_point (cop);
	    } else
		xe_msg (1, "No current object");
	    break;

	case FAVORITE:
	    if (cop) {
		myfav = 1;
		fav_add (cop);
		myfav = 0;
	    } else
		xe_msg (1, "No current object");
	    break;

	case SHOWGAL:
	    if (cop)
		gal_opscroll (cop);
	    else
		xe_msg (1, "No current object");
	    break;

	case SETTEL:
	    if (cop) {
		db_update (cop);
		telGoto (cop);
	    } else
		xe_msg (1, "No current object");
	    break;

	case HELP:
	    obj_help();
	    break;
	}
}

static void
obj_help()
{
	static char *msg[] = {
	    "List objects in memory, search for an object by name,",
	    "or assign it to a Favorite."
	};
	hlp_dialog ("Object", msg, sizeof(msg)/sizeof(msg[0]));
}

/* callback for what type of object toggles.
 * type is just determined by which TB is set.
 */
/* ARGSUSED */
static void
obj_type_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XmToggleButtonGetState(w))
	    obj_newlist();
}

/* callback when an item is selected from the scrolled list.
 * client is the button number from 0.
 */
/* ARGSUSED */
static void
obj_select_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int bi = (long int)client;
	int idx = topi + bi;

	/* guard a rouge button off the end of the list */
	if (idx >= nmtdups)
	    return;

	cop = mtdups[idx].op;
	obj_setinfo(cop);
}

/* callback when the scroll bar is moved.
 * scrollbar value is new topi.
 */
/* ARGSUSED */
static void
obj_scroll_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmScrollBarCallbackStruct *sp = (XmScrollBarCallbackStruct *)call;
	int idx = sp->value;

	/* guard rolling off end of the list */
	if (idx < 0 || idx >= nmtdups)
	    return;

	topi = idx;
	obj_setnames ();
}

/* callback when CR is hit in the Srch text field or when the Search: PB is
 * activated.
 * N.B. Since we are used by two different kinds of widgets don't use w or call.
 */
/* ARGSUSED */
static void
obj_srch_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char last_str[MAXNM];	/* last string we searched for */
	static int last_idx;		/* index of last match for last_str */
	char *str;
	int i, idx;

	/* get string to find */
	str = XmTextFieldGetString (srch_w);

	/* start from top if new string */
	if (strcmp (str, last_str)) {
	    last_idx = -1;
	    (void) strcpy (last_str, str);
	}

	/* scan list, resuming from last */
	idx = last_idx;
	for (i = 0; i < nmtdups; i++) {

	    /* check new string after advaning with wrap */
	    idx = (idx+1) % nmtdups;
	    if (srch_match (str, mtdups[idx].nm)) {
		int scrollidx;

		/* set scroll bar, scroll only down to last whole set */
		scrollidx = idx;
		if (scrollidx > nmtdups - NBTNS)
		    scrollidx = nmtdups - NBTNS;
		set_something (scroll_w, XmNvalue, (XtArgVal)scrollidx);

		/* set buttons */
		topi = idx;
		obj_setnames ();

		/* show info about the item and make current */
		obj_setinfo(cop = mtdups[idx].op);

		/* save to continue from here if search again */
		last_idx = idx;
		break;
	    }
	}

	if (i == nmtdups)
	    xe_msg (1, "%s: not found", str);

	XtFree (str);
}

/* return 1 if pattern p match string s, else return 0.
 * N.B. we only use the first MAXNM-1 chars from each string and assume \0.
 */
static int
srch_match (char *p, char *s)
{
	int flags = 0;
#if defined(FNM_CASEFOLD)       /* GNU extension only */
        flags |= FNM_CASEFOLD;
#endif
	return (!fnmatch (p, s, flags));
}

/* set up and display first NBTNS of the dupnames list.
 */
static void
obj_newlist()
{
	int (*typef)(Obj *) = ty_test();
	DupName *dnp;
	int ndn = db_dups(&dnp);
	int i, n, ssize, max;
	Arg args[20];

	watch_cursor(1);

	/* free old matching list unless it was the real one */
	if (localdups) {
	    XtFree ((char *)mtdups);
	    mtdups = NULL;
	    nmtdups = 0;
	    localdups = 0;
	}

	/* build duplist with just our types, or use original if want all */
	if (typef) {
	    mtdups = (DupName *) XtMalloc (ndn*sizeof(DupName));
	    nmtdups = 0;
	    localdups = 1;
	    for (i = 0; i < ndn; i++)
		if ((*typef)(dnp[i].op))
		    mtdups[nmtdups++] = dnp[i];
	} else {
	    mtdups = dnp;
	    nmtdups = ndn;
	    localdups = 0;
	}

	/* set scrollbar limits, beware of 0 */
	max = nmtdups;
	if (max < 1)
	    max = 1;
	ssize = max;
	if (ssize > NBTNS)
	    ssize = NBTNS;
	n = 0;
	XtSetArg (args[n], XmNmaximum, max); n++;
	XtSetArg (args[n], XmNsliderSize, ssize); n++;
	XtSetArg (args[n], XmNvalue, 0); n++;
	XtSetValues (scroll_w, args, n);
	XtManageChild(scroll_w);

	topi = 0;
	obj_unsetinfo();
	obj_setnames();

	watch_cursor(0);
}

static int
is_ss (Obj *op)
{
	return (is_ssobj (op));
}

static int
is_es (Obj *op)
{
	return (is_type (op, EARTHSATM));
}

static int
is_bi (Obj *op)
{
	return (is_type (op, BINARYSTARM));
}

static int
is_st (Obj *op)
{
	return (is_type(op,FIXEDM) && !is_deepsky (op));
}

/* return pointer to function that when called with an Obj * will return
 * 1 or 0 depending on whether the object fits the current type selection.
 * N.B. we return NULL if all types will match.
 */
static int
(*ty_test(void))(Obj *)
{
	if (XmToggleButtonGetState(ss_w))
	    return (is_ss);
	else if (XmToggleButtonGetState(ds_w))
	    return (is_deepsky);
	else if (XmToggleButtonGetState(es_w))
	    return (is_es);
	else if (XmToggleButtonGetState(st_w))
	    return (is_st);
	else if (XmToggleButtonGetState(bi_w))
	    return (is_bi);
	else
	    return (NULL);
}

/* fill the name buttons with the next set starting at topi */
static void
obj_setnames()
{
	char name[MAXNM];
	int nb;		/* number of good buttons to set, turn off others */
	int i;		/* namepb_w[] index */

	nb = nmtdups - topi;
	if (nb > NBTNS)
	    nb = NBTNS;

	for (i = 0; i < NBTNS; i++) {
	    int sens;
	    if (i < nb) {
		sprintf (name, "%-*s", MAXNM-1, mtdups[topi+i].nm);
		buttonAsButton(namepb_w[i], 1);
		sens = 1;
	    } else {
		sprintf (name, "%-*s", MAXNM-1, "");
		buttonAsButton(namepb_w[i], 0);
		sens = 0;
	    }

	    XtSetSensitive (namepb_w[i], sens);
	    set_xmstring (namepb_w[i], XmNlabelString, name);
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: objmenu.c,v $ $Date: 2013/03/02 02:57:08 $ $Revision: 1.55 $ $Name:  $"};

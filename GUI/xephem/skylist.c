/* handle the list feature of skyview */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Scale.h>
#include <Xm/SelectioB.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>

#include "xephem.h"

static void sl_create_list_w (void);
static void sl_create_pu (void);
static void sl_cr_sorttable (Widget f_w);
static void sl_toggle_cb (Widget w, XtPointer client, XtPointer call);
static void sl_save_cb (Widget w, XtPointer client, XtPointer call);
static void sl_clear_cb (Widget w, XtPointer client, XtPointer call);
static void sl_undo_cb (Widget w, XtPointer client, XtPointer call);
static void sl_so_cb (Widget w, XtPointer client, XtPointer call);
static void sl_sort_cb (Widget w, XtPointer client, XtPointer call);
static void sl_close_cb (Widget w, XtPointer client, XtPointer call);
static void sl_help_cb (Widget w, XtPointer client, XtPointer call);
static void slpu_mark_cb (Widget w, XtPointer client, XtPointer call);
static void slpu_del_cb (Widget w, XtPointer client, XtPointer call);
static void sl_info_eh (Widget w, XtPointer client, XEvent *ev,
    Boolean *continue_to_dispatch);
static void sl_append_qcb (void);
static void sl_write_qcb (void);
static void sl_sort (void);
static void sl_colhdr (char *txt);
static int sort_name (const void *v1, const void *v2);
static int sort_size (const void *v1, const void *v2);
static int sort_spect (const void *v1, const void *v2);
static int sort_ura (const void *v1, const void *v2);
static int sort_u2k (const void *v1, const void *v2);
static int sort_msa (const void *v1, const void *v2);
static int sort_airmass (const void *v1, const void *v2);
static int sort_desc (const void *v1, const void *v2);
static int sort_ha (const void *v1, const void *v2);
static int sort_edist (const void *v1, const void *v2);
static int sort_sdist (const void *v1, const void *v2);
static int sort_ra (const void *v1, const void *v2);
static int sort_dec (const void *v1, const void *v2);
static int sort_alt (const void *v1, const void *v2);
static int sort_az (const void *v1, const void *v2);
static int sort_settm (const void *v1, const void *v2);
static int sort_setaz (const void *v1, const void *v2);
static int sort_risetm (const void *v1, const void *v2);
static int sort_riseaz (const void *v1, const void *v2);
static int sort_trantm (const void *v1, const void *v2);
static int sort_tranalt (const void *v1, const void *v2);
static int sort_elong (const void *v1, const void *v2);
static int sort_mag (const void *v1, const void *v2);
static int sort_cns (const void *v1, const void *v2);
static int sort_glat (const void *v1, const void *v2);
static int sort_glng (const void *v1, const void *v2);
static int sort_hlat (const void *v1, const void *v2);
static int sort_hlng (const void *v1, const void *v2);
static int sort_eclat (const void *v1, const void *v2);
static int sort_eclng (const void *v1, const void *v2);
static int sort_z (const void *v1, const void *v2);
static int sort_sep (const void *v1, const void *v2);

static Widget list_w;		/* object list dialog */
static Widget txttb_w;		/* text-format TB */
static Widget txtfn_w;		/* text-format file name */
static Widget edbtb_w;		/* edb-format TB */
static Widget edbfn_w;		/* edb-format file name */
static Widget nl_w;		/* count label */
static Widget clpb_w;		/* Clear PB */
static Widget undo_w;		/* Undo PB */
static Widget listt_w;		/* text for showing list */
static Widget hdrtf_w;		/* text field for showing column headings */

#define	LSTINDENT	20	/* list item indent, pixels */
#define	ATLASLEN	30	/* room for atlas info */

/* all info about one object, including some things not in an Obj */
typedef struct {
    Obj *op;			/* Obj */
    RiseSet rs;			/* rise/set info */
    double glat, glng;		/* galactic lat/long */
    double eclat, eclng;	/* ecliptic lat/long */
    double ha;			/* hour angle */
    double Z;			/* airmass */
    char *cns;			/* constellation name */
    char *desc;			/* description */
    char ura[ATLASLEN];		/* Uranometria page number */
    char u2k[ATLASLEN];		/* Uranometria 200 page number */
    char msa[ATLASLEN];		/* Millenium Star Atlas page number */
} ObjInfo;

/* handy "no rise/set" test */
#define NORS (RS_ERROR|RS_CIRCUMPOLAR|RS_NEVERUP)

/* info about each sort field */
typedef struct {
    char *lname;		/* label widget instance name */
    char *title;		/* sort description */
    int (*cmp_f)();		/* qsort-style compare function */
    Widget lw;			/* sort-order label widget */
    Widget pw;			/* sort-order pushb widget */
} SortInfo;
static SortInfo sinfo[] = {
    {"Name",   "Name",             sort_name},
    {"Type",   "Type",             sort_desc},
    {"Cns",    "Constellation",    sort_cns},
    {"RA",     "Right Asc",        sort_ra},
    {"HA",     "Hour angle",       sort_ha},
    {"Dec",    "Declination",      sort_dec},
    {"Az",     "Azimuth",          sort_az},
    {"Alt",    "Altitude",         sort_alt},
    {"Zenith", "Zenith",           sort_z},
    {"Air",    "Air mass",         sort_airmass},
    {"Spect",  "Spect class",      sort_spect},

    {"Size",   "Size",             sort_size},
    {"Mag",    "Magnitude",        sort_mag},
    {"Elong",  "Elongation",       sort_elong},
    {"HLat",   "Helio lat",        sort_hlat},
    {"HLong",  "Helio long",       sort_hlng},
    {"GLat",   "Galactic lat",     sort_glat},
    {"GLong",  "Galactic long",    sort_glng},
    {"EcLat",  "Ecliptic lat",     sort_eclat},
    {"EcLong", "Ecliptic long",    sort_eclng},
    {"EaDst",  "Earth dist",       sort_edist},
    {"SnDst",  "Sun dist",         sort_sdist},

    {"Urano",  "Uranometria",      sort_ura},
    {"Uran2k", "Uranom 2000",      sort_u2k},
    {"MillSA", "Millenium",        sort_msa},
    {"RisTm",  "Rise time",        sort_risetm},
    {"RisAz",  "Rise azimuth",     sort_riseaz},
    {"TrnTm",  "Transit time",     sort_trantm},
    {"TrnAlt", "Transit alt",      sort_tranalt},
    {"SetTm",  "Set time",         sort_settm},
    {"SetAz",  "Set azimuth",      sort_setaz},
    {"Sep",    "Separation",       sort_sep},
};
#define	NSINFO	XtNumber(sinfo)
#define	NTCOLS	4		/* desired number of columns for table */
#define	NTROWS	((NSINFO-1)/NTCOLS+1)
#define	LBLPC	25		/* % of each cell for sort-order label */

/* popup for more info on a given line */
typedef struct {
    Widget pu_w;		/* popup */
    Widget name_w;		/* name label */
    int linepos;		/* index into listt_w text at BOL */
    Obj *op;			/* named object */
} PopupInfo;
static PopupInfo pu;

/* pointers into sinfo[] in order of sort */
static SortInfo *sortorder[NSINFO];
static int nsortorder;		/* number in use */

static char skylcategory[] = "Sky View -- List";

void
sl_manage()
{
	if (!list_w) {
	    sl_create_list_w();
	    sl_create_pu();
	}
	XtManageChild (list_w);
}

void
sl_unmanage()
{
	if (list_w)
	    XtUnmanageChild (list_w);
}

/* called to put up or remove the watch cursor.  */
void
sl_cursor (c)
Cursor c;
{
	Window win;

	if (list_w && (win = XtWindow(list_w)) != 0) {
	    Display *dsp = XtDisplay(list_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create the list filename prompt */
static void
sl_create_list_w()
{
	Widget dspb_w, lbl_w, tbl_w;
	Widget w;
	Arg args[20];
	int n;

	/* create form */

	n = 0;
	XtSetArg (args[n], XmNallowShellResize, False); n++;
	XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNfractionBase, 13); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	list_w = XmCreateFormDialog (svshell_w, "SkyList", args, n);
	set_something (list_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (list_w, XmNhelpCallback, sl_help_cb, 0);
	sr_reg (XtParent(list_w), "XEphem*SkyList.x", skylcategory, 0);
	sr_reg (XtParent(list_w), "XEphem*SkyList.y", skylcategory, 0);

	/* set some stuff in the parent DialogShell.
	 * setting XmNdialogTitle in the Form didn't work..
	 */
	n = 0;
	XtSetArg (args[n], XmNtitle, "Sky list"); n++;
	XtSetValues (XtParent(list_w), args, n);

	/* title */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
	lbl_w = XmCreateLabel (list_w, "TTB", args, n);
	set_xmstring (lbl_w, XmNlabelString,
				"List, Sort and Save current Sky View objects");
	XtManageChild (lbl_w);

	/* format options */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, lbl_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	lbl_w = XmCreateLabel (list_w, "FFL", args, n);
	set_xmstring (lbl_w, XmNlabelString,
			    "Choose format and set file name for Save:");
	XtManageChild (lbl_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, lbl_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, LSTINDENT); n++;
	XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	XtSetArg (args[n], XmNmarginWidth, 4); n++;
	XtSetArg (args[n], XmNspacing, 4); n++;
	edbtb_w = XmCreateToggleButton (list_w, "EDB", args, n);
	set_xmstring (edbtb_w, XmNlabelString, ".edb:");
	XtManageChild (edbtb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, lbl_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, edbtb_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	edbfn_w = XmCreateTextField (list_w, "EdbFilename", args, n);
	defaultTextFN (edbfn_w, 1, "skylist.edb", NULL);
	wtip (edbfn_w, "Name of .edb file to create");
	XtManageChild (edbfn_w);
	sr_reg (edbfn_w, NULL, skylcategory, 1);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, edbfn_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, LSTINDENT); n++;
	XtSetArg (args[n], XmNmarginWidth, 4); n++;
	XtSetArg (args[n], XmNspacing, 4); n++;
	XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	txttb_w = XmCreateToggleButton (list_w, "Text", args, n);
	set_xmstring (txttb_w, XmNlabelString, "Text:");
	XtManageChild (txttb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, edbfn_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, txttb_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	txtfn_w = XmCreateTextField (list_w, "TextFilename", args, n);
	defaultTextFN (txtfn_w, 1, "skylist.txt", NULL);
	wtip (txtfn_w, "Name of text file to create");
	XtManageChild (txtfn_w);
	sr_reg (txtfn_w, NULL, skylcategory, 1);

	/* implement txt/edb as a radio pair */
	if (XmToggleButtonGetState(txttb_w) == XmToggleButtonGetState(edbtb_w)){
	    XmToggleButtonSetState(txttb_w, True, 0);
	    XmToggleButtonSetState(edbtb_w, False, 0);
	}
	XtAddCallback (txttb_w, XmNvalueChangedCallback, sl_toggle_cb,
							(XtPointer)edbtb_w);
	XtAddCallback (edbtb_w, XmNvalueChangedCallback, sl_toggle_cb,
							(XtPointer)txttb_w);

	sr_reg (txttb_w, NULL, skylcategory, 1);

	/* sort options heading */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, txtfn_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	lbl_w = XmCreateLabel (list_w, "STB", args, n);
	set_xmstring (lbl_w, XmNlabelString,
					    "Pick sort keys in desired order:");
	XtManageChild (lbl_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, txtfn_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, lbl_w); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNsensitive, False); n++;
	clpb_w = XmCreatePushButton (list_w, "Clear", args, n);
	XtAddCallback (clpb_w, XmNactivateCallback, sl_clear_cb, NULL);
	wtip (clpb_w, "Start key selection over");
	XtManageChild (clpb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, txtfn_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, clpb_w); n++;
	XtSetArg (args[n], XmNleftOffset, 10); n++;
	XtSetArg (args[n], XmNsensitive, False); n++;
	undo_w = XmCreatePushButton (list_w, "Undo", args, n);
	XtAddCallback (undo_w, XmNactivateCallback, sl_undo_cb, NULL);
	wtip (undo_w, "Undo last key pick");
	XtManageChild (undo_w);

	/* sort option table */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, undo_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	tbl_w = XmCreateForm (list_w, "SortTable", args, n);
	XtManageChild (tbl_w);

	sl_cr_sorttable(tbl_w);

	/* bottom controls */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 1); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 3); n++;
	w = XmCreatePushButton (list_w, "Save", args, n);
	XtAddCallback (w, XmNactivateCallback, sl_save_cb, NULL);
	wtip (w, "Write the file chosen above");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 4); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 6); n++;
	w = XmCreatePushButton (list_w, "Sort", args, n);
	XtAddCallback (w, XmNactivateCallback, sl_sort_cb, NULL);
	wtip (w, "Rebuild the list with new settings");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 7); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 9); n++;
	w = XmCreatePushButton (list_w, "Close", args, n);
	XtAddCallback (w, XmNactivateCallback, sl_close_cb, NULL);
	wtip (w, "Close this window");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 12); n++;
	w = XmCreatePushButton (list_w, "Help", args, n);
	XtAddCallback (w, XmNactivateCallback, sl_help_cb, NULL);
	wtip (w, "Get more info");
	XtManageChild (w);

	/* scrolled text in between, beneath title and column headings */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, tbl_w); n++;
	XtSetArg (args[n], XmNtopOffset, 12); n++;	/* lazy: nudge for PB */
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	nl_w = XmCreateLabel (list_w, "STB", args, n);
	set_xmstring (nl_w, XmNlabelString,
					"List: 0 objects. Set columns with");
	XtManageChild (nl_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, tbl_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, nl_w); n++;
	XtSetArg (args[n], XmNleftOffset, 4); n++;
	dspb_w = XmCreatePushButton (list_w, "STB", args, n);
	XtAddCallback (dspb_w, XmNactivateCallback, dm_setup_cb, NULL);
	wtip (dspb_w,"Bring up Data Table setup window to change list columns");
	set_xmstring (dspb_w, XmNlabelString, "Data Table setup...");
	XtManageChild (dspb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, dspb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftOffset, 12); n++; /* lazy: margin + shadow */
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNshadowThickness, 0); n++;
	XtSetArg (args[n], XmNeditable, False); n++;
	XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	XtSetArg (args[n], XmNmarginHeight, 0); n++;
	hdrtf_w = XmCreateTextField (list_w, "HdrTF", args, n);
	XtManageChild (hdrtf_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, hdrtf_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
        XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg (args[n], XmNeditable, True); n++;
	listt_w = XmCreateScrolledText (list_w, "List", args, n);
	XtAddEventHandler (listt_w, ButtonPressMask, False, sl_info_eh, 0);
	sr_reg (listt_w, "XEphem*SkyList*List.rows", skylcategory, 0);
	XtManageChild (listt_w);
}

/* create the popup skeleton */
static void
sl_create_pu()
{
	Widget w;
	Arg args[20];
	int n;

	/* make the popup shell */
	n = 0;
	XtSetArg (args[n], XmNisAligned, True); n++;
	XtSetArg (args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
	pu.pu_w = XmCreatePopupMenu (listt_w, "SLPopup", args, n);

	/* add the name, separator and controls.
	 * we always wants these so manage them now.
	 */
	pu.name_w = XmCreateLabel (pu.pu_w, "SLLab", args, 0);
	XtManageChild (pu.name_w);
	w = XmCreateSeparator (pu.pu_w, "SLSep", args, 0);
	XtManageChild (w);
	w = XmCreatePushButton (pu.pu_w, "Sky Mark", args, 0);
	XtAddCallback (w, XmNactivateCallback, slpu_mark_cb, 0);
	XtManageChild (w);
	wtip (w, "Mark this object on the Sky View");
	w = XmCreatePushButton (pu.pu_w, "Delete", args, 0);
	XtAddCallback (w, XmNactivateCallback, slpu_del_cb, 0);
	XtManageChild (w);
	wtip (w, "Delete this object from the list");
}

/* build the sort table in the given XmForm */
static void
sl_cr_sorttable(f_w)
Widget f_w;
{
	Arg args[20];
	Widget fr_w;
	Widget w;
	int n;
	int i;

	/* build in column-major order */
	for (i = 0; i < NSINFO; i++) {
	    SortInfo *sip = &sinfo[i];
	    int r = i%NTROWS;			/* row 0..(NTROWS-1)*/
	    int c = i/NTROWS;			/* col 0..(NTCOLS-1)*/
	    int t = 100*r/NTROWS;		/* top % */
	    int b = 100*(r+1)/NTROWS;		/* bottom % */
	    int l = 100*c/NTCOLS;		/* left % */
	    int h = 100*(c+1)/NTCOLS;		/* right % */
	    int p = l + LBLPC/NTCOLS;		/* left pushb % */

	    /* label to display sort position */
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNtopPosition, t); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNbottomPosition, b); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, l); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, p); n++;
	    XtSetArg (args[n], XmNshadowThickness, 2); n++;
	    XtSetArg (args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
	    fr_w = XmCreateFrame (f_w, "STFr", args, n);
	    XtManageChild (fr_w);

		n = 0;
		XtSetArg (args[n], XmNalignment, XmALIGNMENT_END); n++;
		w = XmCreateLabel (fr_w, sip->lname, args, n);
		XtManageChild (w);
		set_xmstring (w, XmNlabelString, " ");
		sip->lw = w;

	    /* push button to define sort position */
	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNtopPosition, t); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNbottomPosition, b); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, p); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, h); n++;
	    w = XmCreatePushButton (f_w, "STPB", args, n);
	    XtAddCallback (w, XmNactivateCallback, sl_so_cb,
							(XtPointer)&sinfo[i]);
	    XtManageChild (w);
	    set_xmstring (w, XmNlabelString, sip->title);
	    sip->pw = w;
	}
}

/* called from either file format TB to implement a radio pair.
 * client is the opposite TB.
 */
/* ARGSUSED */
static void
sl_toggle_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int set = XmToggleButtonGetState(w);
	Widget otherw = (Widget)client;

	XmToggleButtonSetState (otherw, !set, 0);
}

/* called when the Close button is hit in the file list prompt */
/* ARGSUSED */
static void
sl_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (list_w);
}

/* called when the help button is hit in the file list prompt */
/* ARGSUSED */
static void
sl_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
	    "List and Save Sky View objects in two formats."
	};

	hlp_dialog ("SkyView_list", msg, sizeof(msg)/sizeof(msg[0]));

}

/* clear most recent entry from sortorder[].
 * if last, desensitize Clear and Undo.
 * N.B. desensitizing is key because there is NO OVER or UNDERFLOW checking.
 */
static void
sl_clearlast()
{
	SortInfo *sip;
	
	/* clear last */
	sip = sortorder[--nsortorder];
	set_xmstring (sip->lw, XmNlabelString, "  ");
	XtSetSensitive (sip->pw, True);

	/* if none left, can not clear/undo now */
	if (!nsortorder) {
	    XtSetSensitive (clpb_w, False);
	    XtSetSensitive (undo_w, False);
	}
}

/* called when the Clear button is hit */
/* ARGSUSED */
static void
sl_clear_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* clear all */
	while (nsortorder)
	    sl_clearlast();
}

/* called when the Undo button is hit */
/* ARGSUSED */
static void
sl_undo_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	sl_clearlast();
}

/* called when any sort-order table button is hit.
 * client is pointer into sinfo[]
 */
/* ARGSUSED */
static void
sl_so_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	SortInfo *sip = (SortInfo *)client;
	char buf[10];

	/* add this function next in sortforder list */
	sortorder[nsortorder++] = sip;

	/* indicate order */
	sprintf (buf, "%d", nsortorder);
	set_xmstring (sip->lw, XmNlabelString, buf);

	/* can't use this again */
	XtSetSensitive (sip->pw, False);

	/* ok to clear and undo now */
	XtSetSensitive (clpb_w, True);
	XtSetSensitive (undo_w, True);
}

/* called when the Sort button is hit */
/* ARGSUSED */
static void
sl_sort_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	sl_sort();
}

/* called when the Save button is hit */
/* ARGSUSED */
static void
sl_save_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *fn;

	/* get and sanity-check the appropriate file name */
	if (XmToggleButtonGetState(txttb_w)) {
	    fn = XmTextFieldGetString (txtfn_w);
	    if (strlen(fn) == 0) {
		xe_msg (1, "Please enter a text file name.");
		XtFree (fn);
		return;
	    }
	} else {
	    fn = XmTextFieldGetString (edbfn_w);
	    if (strlen(fn) == 0) {
		xe_msg (1, "Please enter an .edb file name.");
		XtFree (fn);
		return;
	    }
	}

	/* go, perhaps cautiously */
	if (existd (fn,NULL) == 0 && confirm()) {
	    char buf[1024];
	    (void) sprintf (buf, "%s exists:\nAppend or Overwrite?", fn);
	    query (list_w, buf, "Append", "Overwrite", "Cancel",
				    sl_append_qcb, sl_write_qcb, NULL);
	} else
	    sl_write_qcb();	/* just hammer it */

	/* done with name */
	XtFree (fn);
}

/* called when the Mark button is hit on the popup */
/* ARGSUSED */
static void
slpu_mark_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* sanity check */
	if (!pu.op)
	    return;

	sv_point (pu.op);
}

/* called when the Delete button is hit on the popup */
/* ARGSUSED */
static void
slpu_del_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *txt;
	int end;

	/* sanity check */
	if (!pu.op)
	    return;

	/* get text */
	txt = XmTextGetString (listt_w);

	/* delete from pu.linepos to next nl or EOS */
	for (end = pu.linepos; txt[end] && txt[end] != '\n'; end++)
	    continue;
	if (txt[end] == '\n')
	    end++;
	XmTextReplace (listt_w, pu.linepos, end, "");
	XmTextShowPosition (listt_w, pu.linepos);

	XtFree (txt);
}

/* extra ButtonPress handler added to scrolled text.
 * used to provide few more options based on text under pointer.
 */
static void
sl_info_eh (w, client, ev, continue_to_dispatch)
Widget w;
XtPointer client;
XEvent *ev;
Boolean *continue_to_dispatch;
{
	char objname[MAXNM];
	DBScan dbs;
	ObjF *fs;
	Obj *op;
	int nfs;
	char *txt;
	int pos;
	int i;

	/* only care about button3 press */
	if (!(ev->type == ButtonPress && ev->xbutton.button == Button3))
	    return;

	/* where are we? */
	pos = XmTextXYToPos (w, ev->xbutton.x, ev->xbutton.y);
	if (pos < 0)
	    return;

	/* find name  at beginning of this line */
	txt = XmTextGetString (w);
	for (; pos > 0; --pos) {
	    if (txt[pos] == '\n') {
		pos++;
		break;
	    }
	}
	i = sprintf (objname, "%*.*s", MAXNM-1, MAXNM-1, &txt[pos]);

	/* finished with txt now */
	XtFree (txt);

	/* dig out pure object name */
	if (XmToggleButtonGetState(txttb_w)) {
	    while (--i >= 0 && objname[i] == ' ')
		objname[i] = '\0';
	    if (i < 0)
		return;
	} else {
	    char *comma = strchr (objname, ',');
	    if (comma)
		*comma = '\0';
	}

	/* find object with name matching that at start of this line */
	sv_getfldstars (&fs, &nfs);
	for (db_scaninit(&dbs, ALLM, fs, nfs); (op = db_scan(&dbs)) != NULL; )
	    if (!strcmp (op->o_name, objname))
		break;
	if (!op)
	    return;	/* no name found */

	/* set up for popup and go */

	/* create popup first time */
	if (!pu.pu_w)
	    sl_create_pu();

	pu.linepos = pos;
	pu.op = op;
	set_xmstring (pu.name_w, XmNlabelString, objname);
	XmMenuPosition (pu.pu_w, (XButtonPressedEvent *)ev);
	XtManageChild (pu.pu_w);
}

/* save the text widget in the appropriate file */
static void
sl_save (how)
char *how;
{
	char *fn = XmToggleButtonGetState(txttb_w)
			? XmTextFieldGetString (txtfn_w)
			: XmTextFieldGetString (edbfn_w);
	FILE *fp = fopend (fn, NULL, how);
	char *txt = XmTextGetString (listt_w);

	fprintf (fp, "%s", txt);
	fclose (fp);
	XtFree (txt);
	XtFree (fn);
}

/* called when we want to append .. filename already checked */
static void
sl_append_qcb ()
{
	sl_save ("a");
}

/* called when we want to ceate a new text-format file */
static void
sl_write_qcb ()
{
	sl_save ("w");
}

static int
spaceship (diff)
double diff;
{
	if (diff < 0)
	    return (-1);
	if (diff > 0)
	    return (1);
	return (0);
}

/* compare 2 ObjInfos for RA, ala qsort() */
static int
sort_ra (const void *v1, const void* v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp1->op->s_ra - lp2->op->s_ra));
}

/* compare 2 ObjInfos for HA, ala qsort() */
static int
sort_ha (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp1->ha - lp2->ha));
}

/* compare 2 ObjInfos for alt, ala qsort() */
static int
sort_alt (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp2->op->s_alt - lp1->op->s_alt));
}

/* compare 2 ObjInfos for zenith distance, ala qsort() */
static int
sort_z (const void *v1, const void *v2)
{
	return (-sort_alt(v1, v2));
}

/* compare 2 ObjInfos for az, ala qsort() */
static int
sort_az (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp1->op->s_az - lp2->op->s_az));
}

/* compare 2 ObjInfos for airmass, ala qsort() */
static int
sort_airmass (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp1->Z - lp2->Z));
}

/* compare 2 ObjInfos for Dec, ala qsort() */
static int
sort_dec (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp1->op->s_dec - lp2->op->s_dec));
}

/* compare 2 ObjInfos for rise time, ala qsort() */
static int
sort_risetm (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int no1 = lp1->rs.rs_flags & (RS_NORISE|NORS);
	int no2 = lp2->rs.rs_flags & (RS_NORISE|NORS);

	/* float entries with no rise info to end of list */
	if (no1)
	    return (no2 ? 0 : 1);
	else if (no2)
	    return (-1);
	else
	    return (spaceship (lp1->rs.rs_risetm - lp2->rs.rs_risetm));
}

/* compare 2 ObjInfos for rise az, ala qsort() */
static int
sort_riseaz (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int no1 = lp1->rs.rs_flags & (RS_NORISE|NORS);
	int no2 = lp2->rs.rs_flags & (RS_NORISE|NORS);

	/* float entries with no rise info to end of list */
	if (no1)
	    return (no2 ? 0 : 1);
	else if (no2)
	    return (-1);
	else
	    return (spaceship (lp1->rs.rs_riseaz - lp2->rs.rs_riseaz));
}

/* compare 2 ObjInfos for set time, ala qsort() */
static int
sort_settm (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int no1 = lp1->rs.rs_flags & (RS_NOSET|NORS);
	int no2 = lp2->rs.rs_flags & (RS_NOSET|NORS);

	/* float entries with no rise info to end of list */
	if (no1)
	    return (no2 ? 0 : 1);
	else if (no2)
	    return (-1);
	else
	    return (spaceship (lp1->rs.rs_settm - lp2->rs.rs_settm));
}

/* compare 2 ObjInfos for set az, ala qsort() */
static int
sort_setaz (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int no1 = lp1->rs.rs_flags & (RS_NOSET|NORS);
	int no2 = lp2->rs.rs_flags & (RS_NOSET|NORS);

	/* float entries with no rise info to end of list */
	if (no1)
	    return (no2 ? 0 : 1);
	else if (no2)
	    return (-1);
	else
	    return (spaceship (lp1->rs.rs_setaz - lp2->rs.rs_setaz));
}

/* compare 2 ObjInfos for transit time, ala qsort() */
static int
sort_trantm (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int no1 = lp1->rs.rs_flags & (RS_ERROR|RS_NEVERUP|RS_NOTRANS);
	int no2 = lp2->rs.rs_flags & (RS_ERROR|RS_NEVERUP|RS_NOTRANS);

	/* float entries with no transit info to end of list */
	if (no1)
	    return (no2 ? 0 : 1);
	else if (no2)
	    return (-1);
	else
	    return (spaceship (lp1->rs.rs_trantm - lp2->rs.rs_trantm));
}

/* compare 2 ObjInfos for transit alt, ala qsort() */
static int
sort_tranalt (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int no1 = lp1->rs.rs_flags & (RS_ERROR|RS_NEVERUP|RS_NOTRANS);
	int no2 = lp2->rs.rs_flags & (RS_ERROR|RS_NEVERUP|RS_NOTRANS);

	/* float entries with no transit info to end of list */
	if (no1)
	    return (no2 ? 0 : 1);
	else if (no2)
	    return (-1);
	else
	    return (spaceship (lp1->rs.rs_tranalt - lp2->rs.rs_tranalt));
}

/* compare 2 ObjInfos for galactic latitude, ala qsort() */
static int
sort_glat (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp1->glat - lp2->glat));
}

/* compare 2 ObjInfos for galactic longitude, ala qsort() */
static int
sort_glng (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp1->glng - lp2->glng));
}

/* compare 2 ObjInfos for ecliptic latitude, ala qsort() */
static int
sort_eclat (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp1->eclat - lp2->eclat));
}

/* compare 2 ObjInfos for ecliptic longitude, ala qsort() */
static int
sort_eclng (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp1->eclng - lp2->eclng));
}

/* compare 2 ObjInfos for heliocentric latitude, ala qsort() */
static int
sort_hlat (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int ss1 = is_ssobj(lp1->op);
	int ss2 = is_ssobj(lp2->op);

	/* float non-solsys objects to end of list */
	if (ss1) {
	    if (ss2)
		return (spaceship(lp1->op->s_hlat - lp2->op->s_hlat));
	    return (-1);
	} else
	    return (ss2 ? 1 : 0);
}

/* compare 2 ObjInfos for heliocentric longitude, ala qsort() */
static int
sort_hlng (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int ss1 = is_ssobj(lp1->op);
	int ss2 = is_ssobj(lp2->op);

	/* float non-solsys objects to end of list */
	if (ss1) {
	    if (ss2)
		return (spaceship(lp1->op->s_hlong - lp2->op->s_hlong));
	    return (-1);
	} else
	    return (ss2 ? 1 : 0);
}

/* compare 2 ObjInfos for earth distance, ala qsort() */
static int
sort_edist (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int ss1 = is_ssobj(lp1->op);
	int ss2 = is_ssobj(lp2->op);

	/* float non-solsys objects to end of list */
	if (ss1) {
	    if (ss2)
		return (spaceship(lp1->op->s_edist - lp2->op->s_edist));
	    return (-1);
	} else
	    return (ss2 ? 1 : 0);
}

/* compare 2 ObjInfos for sun distance, ala qsort() */
static int
sort_sdist (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	int ss1 = is_ssobj(lp1->op);
	int ss2 = is_ssobj(lp2->op);

	/* float non-solsys objects to end of list */
	if (ss1) {
	    if (ss2)
		return (spaceship(lp1->op->s_sdist - lp2->op->s_sdist));
	    return (-1);
	} else
	    return (ss2 ? 1 : 0);
}

/* compare 2 ObjInfos for elongation , ala qsort() */
static int
sort_elong (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp2->op->s_elong - lp1->op->s_elong));
}

/* compare 2 ObjInfos for magnitude, ala qsort() */
static int
sort_mag (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(get_mag(lp1->op) - get_mag(lp2->op)));
}

/* compare 2 ObjInfos for constellation name, ala qsort() */
static int
sort_cns (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (strcmp(lp1->cns, lp2->cns));
}

/* compare 2 ObjInfos for ura[], ala qsort() */
static int
sort_ura (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (strcmp(lp1->ura, lp2->ura));
}

/* compare 2 ObjInfos for u2k[], ala qsort() */
static int
sort_u2k (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (strcmp(lp1->u2k, lp2->u2k));
}

/* compare 2 ObjInfos for msa[] name, ala qsort() */
static int
sort_msa (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (strcmp(lp1->msa, lp2->msa));
}

/* compare 2 ObjInfos for f_spect[], ala qsort() */
static int
sort_spect (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	Obj *op1 = lp1->op;
	Obj *op2 = lp2->op;
	int o1ok= is_type(op1,FIXEDM) && op1->f_class!='G' && op1->f_spect[0];
	int o2ok= is_type(op2,FIXEDM) && op2->f_class!='G' && op2->f_spect[0];

	if (!o1ok)
	    return (o2ok ?  1 : 0);
	if (!o2ok)
	    return (o1ok ? -1 : 0);
	return (strncmp (op1->f_spect, op2->f_spect, sizeof(op1->f_spect)));
}

/* compare 2 ObjInfos for description, ala qsort() */
static int
sort_desc (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (strcmp(lp1->desc, lp2->desc));
}

/* compare 2 ObjInfos for name, ala qsort() */
static int
sort_name (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	char *s1 = lp1->op->o_name;
	char *s2 = lp2->op->o_name;

	return (strnncmp(s1, s2));
}

/* compare 2 ObjInfos for size, ala qsort() */
static int
sort_size (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	return (spaceship(lp2->op->s_size - lp1->op->s_size));
}

/* compare 2 ObjInfos for their separation from the current datatable sep
 * object, ala qsort()
 */
static int
sort_sep (const void *v1, const void *v2)
{
	ObjInfo *lp1 = (ObjInfo *)v1;
	ObjInfo *lp2 = (ObjInfo *)v2;
	double sep1 = dm_sep (lp1->op);
	double sep2 = dm_sep (lp2->op);
	return (spaceship (sep1 - sep2));
}

/* compare 2 ObjInfos according to each sort key, ala qsort() */
static int
mastercmp_f (const void *v1, const void *v2)
{
	int i;

	for (i = 0; i < nsortorder; i++) {
	    int c = (*sortorder[i]->cmp_f)(v1, v2);
	    if (c)
		return (c);
	}

	return (0);
}

/* append src to *dstp, growing *dstp as required.
 * update *dstp and return new final length, sans trailing '\0'.
 */
static int
mstrcat (dstp, dl, src, sl)
char **dstp, *src;
int dl, sl;
{
	char *newdst = XtRealloc (*dstp, sl+dl+1);
	strcpy (newdst+dl, src);
	*dstp = newdst;
	return (sl+dl);
}

/* fill listt_w with oinfo[noinfo] in ".edb" format */
static void
fill_edbfmt (oinfo, noinfo)
ObjInfo *oinfo;
int noinfo;
{
	char *txt = NULL;
	int txtl = 0;
	char buf[256];
	char lbuf[256];
	int l, i;

	/* just install each db object */
	for (i = 0; i < noinfo; i++) {
	    db_write_line (oinfo[i].op, lbuf);
	    l = sprintf (buf, "%s\n", lbuf);
	    txtl = mstrcat (&txt, txtl, buf, l);
	}

	/* N.B. removing last nl looks better here, but complicates line
	 * deleting and must be put back when saving.
	 */

	/* no header */
	XmTextFieldSetString (hdrtf_w, "");

	/* install in text widget, then finished */
	XmTextSetString (listt_w, txt);
	XtFree (txt);
}

/* fill listt_w with oinfo[noinfo] in ".txt" format */
static void
fill_txtfmt (oinfo, noinfo)
ObjInfo *oinfo;
int noinfo;
{
	static char dashes[] = "--------------------------";
	Now *np = mm_get_now();
	char *txt = NULL;
	int txtl = 0;
	char tsbuf[64];
	char buf[256];
	double tmp;
	int lineno;
	int l, i;

	/* summarize the current circumstances */
	mjd_year (epoch == EOD ? mjd : epoch, &tmp);
	fs_timestamp (np, tsbuf);
	l = sprintf (buf, "XEphem %s Sky List: %s %s place, Epoch %7.2f, %s\n",
		    PATCHLEVEL,
		    pref_get (PREF_EQUATORIAL) == PREF_TOPO ? "Topocentric"
							    : "Geocentric",
		    epoch == EOD ? "Apparent" : "Mean",
		    tmp, tsbuf);
	txtl = mstrcat (&txt, txtl, buf, strlen(buf));

	/* print name, Data Table columns, description.
	 * column heading spacing based on size of data.
	 * lineno is used to build up a header.. goofy but works.
	 */
	lineno = 0;
	for (i = 0; i < noinfo; i++) {
	    ObjInfo *lp = &oinfo[i];
	    Obj *op = lp->op;
	    char dbbuf[64];
	    DMCol c;

	    switch (lineno) {
	    case 0:
		l = sprintf (buf, "%-*s", MAXNM-1, "Name");
		break;
	    case 1:
		l = sprintf (buf, "%.*s", MAXNM-1, dashes);
		break;
	    default:
		l = sprintf (buf, "%-*.*s", MAXNM-1, MAXNM-1, op->o_name);
		break;
	    }
	    txtl = mstrcat (&txt, txtl, buf, l);

	    for (c = 0; c < NDMCol; c++) {
		if (!dm_colFormat (np, op, &lp->rs, c, dbbuf)) {
		    int f, h;
		    switch (lineno) {
		    case 0:				/* column heading */
			f = strlen(dbbuf);		/* width of field */
			dm_colHeader (c, dbbuf);
			h = strlen(dbbuf);		/* width of heading */
			l = sprintf (buf, " %*s%-*s", (f-h)/2, "", f-(f-h)/2,
									dbbuf);
			break;
		    case 1:				/* dashes */
			f = strlen(dbbuf);
			l = sprintf (buf, " %.*s", f, dashes);
			break;
		    default:				/* real field :) */
			l = sprintf (buf, " %s", dbbuf);
			break;
		    }
		    txtl = mstrcat (&txt, txtl, buf, l);
		}
	    }

	    switch (lineno) {
	    case 0:
		l = sprintf (buf, " Type\n");
		i--;	/* repeat */
		break;
	    case 1:
		l = sprintf (buf, " %.4s\n", dashes);
		i--;	/* repeat */
		break;
	    default:
		l = sprintf (buf, " %s\n", lp->desc);
		break;
	    }
	    txtl = mstrcat (&txt, txtl, buf, l);

	    lineno++;
	}

	/* N.B. removing last nl looks better here, but complicates line
	 * deleting and must be put back when saving.
	 */

	/* extract 2nd line and use as column heading */
	sl_colhdr (txt);

	/* install in text widget, then finished */
	XmTextSetString (listt_w, txt);
	XtFree (txt);
}

/* extract 2nd line of list and use as column heading.
 * maintain common background color.
 */
static void
sl_colhdr (txt)
char *txt;
{
	Pixel bg;
	char *nl1, *nl2;

	nl1 = strchr (txt, '\n');
	nl2 = strchr (nl1+1, '\n');

	*nl2 = '\0';
	get_something (nl_w, XmNbackground, (XtArgVal)&bg);
	set_something (hdrtf_w, XmNbackground, (XtArgVal)bg);
	XmTextFieldSetString (hdrtf_w, nl1+1);
	*nl2 = '\n';
}

/* sort, then fill listt_w with report.
 * also show count in nl_w.
 */
static void
sl_sort ()
{
	Now *np = mm_get_now();
	ObjInfo *oinfo = NULL;
	DBScan dbs;
	char buf[128];
	int noinfo;
	double e;
	ObjF *fs;
	int nfs;
	Obj *op;

	if (!nsortorder) {
	    xe_msg (1, "Please pick at least one sort key");
	    return;
	}

	watch_cursor(1);

	/* scan db for all objects on Sky screen, add to list */
	e = epoch == EOD ? mjd : epoch;
	sv_getfldstars (&fs, &nfs);
	noinfo = 0;
	for (db_scaninit(&dbs, ALLM, fs, nfs); (op = db_scan(&dbs)) != NULL; ) {
	    ObjInfo *lp;

	    if (!(op->o_flags & OBJF_ONSCREEN))
		continue;

	    /* add to list */
	    oinfo = (ObjInfo *) XtRealloc ((void*)oinfo,
						(noinfo+1)*sizeof(ObjInfo));
	    lp = &oinfo[noinfo++];
	    lp->op = op;

	    /* add supporting info for sorts */
	    dm_riset (np, op, &lp->rs);
	    lp->cns = cns_name (cns_pick (op->s_ra, op->s_dec, e));
	    eq_gal (e, op->s_ra, op->s_dec, &lp->glat, &lp->glng);
	    eq_ecl (e, op->s_gaera, op->s_gaedec, &lp->eclat, &lp->eclng);
	    airmass (op->s_alt, &lp->Z);
	    lp->desc = obj_description (op);
	    radec2ha (np, op->s_ra, op->s_dec, &lp->ha);
	    strncpy (lp->ura, um_atlas(op->s_ra, op->s_dec), sizeof(lp->ura)); 
	    strncpy (lp->u2k, u2k_atlas(op->s_ra, op->s_dec), sizeof(lp->u2k)); 
	    strncpy (lp->msa, msa_atlas(op->s_ra, op->s_dec), sizeof(lp->msa)); 
	}

	/* sort as desired */
	qsort ((void *)oinfo, noinfo, sizeof(ObjInfo), mastercmp_f);

	/* display list, else clear */
	if (!noinfo) {
	    XmTextSetString (listt_w, "");
	    XmTextFieldSetString (hdrtf_w, "");
	} else if (XmToggleButtonGetState(txttb_w))
	    fill_txtfmt (oinfo, noinfo);
	else
	    fill_edbfmt (oinfo, noinfo);

	/* show count -- N.B. match string set when created */
	sprintf (buf, "List: %d objects. Choose columns with", noinfo);
	set_xmstring (nl_w, XmNlabelString, buf);

	/* free */
	XtFree ((void *)oinfo);

	watch_cursor(0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: skylist.c,v $ $Date: 2006/02/05 21:10:43 $ $Revision: 1.42 $ $Name:  $"};

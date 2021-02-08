/* code to manage the picture gallery window
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
#include <Xm/PanedW.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/Separator.h>

#include "lilxml.h"
#include "xephem.h"

static XMLEle *selGIP (void);
static Obj *selInDB (void);
static int gt_cmp (const void *g0, const void *g1);
static int namefind (char *name);
static void gal_create_shell (void);
static void gal_help_cb (Widget w, XtPointer client, XtPointer data);
static void gal_popdown_cb (Widget w, XtPointer client, XtPointer data);
static void gal_close_cb (Widget w, XtPointer client, XtPointer data);
static void gal_rescan_cb (Widget w, XtPointer client, XtPointer data);
static void gal_sel_cb (Widget w, XtPointer client, XtPointer data);
static void gal_skypt_cb (Widget w, XtPointer client, XtPointer data);
static void galReset (void);
static void addName(XMLEle *np);
static void read1Catalog (FILE *fp, XMLEle **dp, char *idxpath);
static void readCatalogs (void);
static void fillReport (XMLEle *name);
static void fillImage (XMLEle *name);
static void fillList(void);
static char *trimws (char *s);

static Widget galshell_w;	/* main shell */
static Widget report_w;		/* TF for showing info */
static Widget sel_w;		/* SL for picking picture */
static Widget pl_w;		/* picture label */
static Widget sky_w;		/* sky-point PB */
static Widget psw_w;		/* picture scroller window */

static char *sdir;		/* handy malloced path to shared gallery dir */
static char *pdir;		/* handy malloced path to private gallery dir */

static XMLEle *galroot[2];	/* XML doc tree for the shar and priv gallery */
static XMLEle **gallery;	/* malloced pointers to all <name> elements */
static int ngallery;		/* n gallery entries in use */
static int mgallery;		/* n gallery entries malloced */
#define	NMALINC		100	/* n to grow gallery[] each time */

static char galcategory[] = "Image Gallery";

/* bring up the picture menu, creating if first time */
void
gal_manage()
{
	if (!galshell_w) {
	    gal_create_shell();
	    readCatalogs();
	    fillList();
	    if (ngallery > 0)
		XmListSelectPos (sel_w, 1, True);	/* show first */
	}

	XtPopup (galshell_w, XtGrabNone);
	set_something (galshell_w, XmNiconic, (XtArgVal)False);
}

/* return index of first gallery entry with any name for op, else -1.
 * ignore case and whitespace
 */
int
gal_opfind (Obj *op)
{
	DupName *dnp;
	int i, ndn, gi;

	/* first try primary name */
	gi = namefind (op->o_name);
	if (gi >= 0)
	    return (gi);

	/* then try other entries in dup name list */
	ndn = db_dups (&dnp);
	for (i = 0; i < ndn; i++) {
	    if (dnp[i].op == op && strcmp (dnp[i].nm, op->o_name)) {
		gi = namefind (dnp[i].nm);
		if (gi >= 0)
		    return (gi);
	    }
	}

	/* nope */
	return (-1);
}

/* scroll to and display gallery entry for given object, if any */
void
gal_opscroll (Obj *op)
{
	int i = gal_opfind (op);

	if (i < 0)
	    return;

	if (!isUp (galshell_w))
	    gal_manage();

	/* select item (1 based!) and let its callback put up the image */
	XmListSelectPos (sel_w, i+1, True);
	XmListSetPos (sel_w, i+1);	/* selecting does not scroll too */
}

/* called to put up or remove the watch cursor.  */
void
gal_cursor (Cursor c)
{
	Window win;

	if (galshell_w && (win = XtWindow(galshell_w)) != 0) {
	    Display *dsp = XtDisplay(galshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* create a shell to allow user to manage pictures . */
static void
gal_create_shell ()
{
	Widget h_w, pw_w;
	Widget galform_w;
	Widget w;
	Arg args[20];
	int n;
	
	/* create outter shell and form */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Image Gallery"); n++;
	XtSetArg (args[n], XmNiconName, "Gallery"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	galshell_w = XtCreatePopupShell ("Gallery", topLevelShellWidgetClass,
							toplevel_w, args, n);
	set_something (galshell_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (galshell_w, XmNpopdownCallback, gal_popdown_cb, NULL);
	setup_icon (galshell_w);
	sr_reg (galshell_w, "XEphem*Gallery.x", galcategory, 0);
	sr_reg (galshell_w, "XEphem*Gallery.y", galcategory, 0);
	sr_reg (galshell_w, "XEphem*Gallery.height", galcategory, 0);
	sr_reg (galshell_w, "XEphem*Gallery.width", galcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 10); n++;
	galform_w = XmCreateForm (galshell_w, "GalF", args, n);
	XtAddCallback (galform_w, XmNhelpCallback, gal_help_cb, 0);
	XtManageChild(galform_w);

	/* controls across the bottom */

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 8); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 23); n++;
	w = XmCreatePushButton (galform_w, "Close", args, n);
	XtAddCallback (w, XmNactivateCallback, gal_close_cb, NULL);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 31); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 46); n++;
	w = XmCreatePushButton (galform_w, "Rescan", args, n);
	XtAddCallback (w, XmNactivateCallback, gal_rescan_cb, NULL);
	wtip (w, "Search again for gallery images");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 54); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 69); n++;
	sky_w = XmCreatePushButton (galform_w, "GS", args, n);
	XtAddCallback (sky_w, XmNactivateCallback, gal_skypt_cb, NULL);
	set_xmstring (sky_w, XmNlabelString, "Sky Point");
	wtip (sky_w, "Zoom Sky View to this object");
	XtManageChild (sky_w);

	n = 0;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 77); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 92); n++;
	h_w = XmCreatePushButton (galform_w, "Help", args, n);
	XtAddCallback (h_w, XmNactivateCallback, gal_help_cb, NULL);
	XtManageChild (h_w);

	/* scrolled list on right */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, h_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNwidth, 150); n++;
	XtSetArg (args[n], XmNselectionPolicy, XmBROWSE_SELECT); n++;
	sel_w = XmCreateScrolledList (galform_w, "GalSL", args, n);
	XtAddCallback (sel_w, XmNbrowseSelectionCallback, gal_sel_cb, NULL);
	XtManageChild (sel_w);

	/* paned window */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNbottomWidget, h_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, XtParent(sel_w)); n++;
	XtSetArg (args[n], XmNsashHeight, 10); n++;
	XtSetArg (args[n], XmNspacing, 14); n++;
	pw_w = XmCreatePanedWindow (galform_w, "GalPW", args, n);
	XtManageChild (pw_w);

	    /* scrolled window on top for image */

	    n = 0;
	    XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	    psw_w = XmCreateScrolledWindow (pw_w, "GalSW", args, n);
	    XtManageChild (psw_w);

	    /* workarea is label for picture pixmap - easy! */

	    n = 0;
	    XtSetArg (args[n], XmNrecomputeSize, True); n++;
	    XtSetArg (args[n], XmNlabelType, XmPIXMAP); n++;
	    pl_w = XmCreateLabel (psw_w, "GalDL", args, n);
	    XtManageChild (pl_w);

	    n = 0;
	    XtSetArg (args[n], XmNworkWindow, pl_w); n++;
	    XtSetValues (psw_w, args, n);

	    /* scrolled text below for report */

	    n = 0;
	    XtSetArg (args[n], XmNeditable, False); n++;
	    XtSetArg (args[n], XmNcursorPositionVisible, False); n++;
	    XtSetArg (args[n], XmNwordWrap, True); n++;
	    XtSetArg (args[n], XmNscrollHorizontal, False); n++;
	    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	    XtSetArg (args[n], XmNskipAdjust, True); n++;
	    report_w = XmCreateScrolledText (pw_w, "Report", args, n);
	    sr_reg (report_w, "XEphem*Gallery*Report.height", galcategory, 0);
	    XtManageChild (report_w);
}

/* read files and create gallery[]
 */
static void
readCatalogs()
{
	char buf[1024];
	char *dir[2];
	int i;

	/* reset current list, if any */
	galReset();

	/* init dir names */
	if (!sdir) {
	    sprintf (buf, "%s/gallery", getShareDir());
	    sdir = XtNewString (buf);		/* beware Rescan */
	    sprintf (buf, "%s/gallery", getPrivateDir());
	    pdir = XtNewString (buf);		/* beware Rescan */
	}
	dir[0] = sdir;
	dir[1] = pdir;

	/* search shared and private dirs */
	for (i = 0; i < XtNumber(dir); i++) {
	    struct dirent *dp;
	    DIR *dirp;
	    FILE *fp;

	    dirp = opendir (dir[i]);
	    if (!dirp)
		continue;

	    while ((dp = readdir (dirp)) != NULL) {
		if (strcmp (dp->d_name+strlen(dp->d_name)-4, ".gly") == 0) {
		    sprintf (buf, "%s/%s", dir[i], dp->d_name);
		    fp = fopenh (buf, "r");
		    if (fp) {
			read1Catalog (fp, &galroot[i], buf);
			fclose (fp);
		    } else if (errno != ENOENT) {
			xe_msg (1, "%s:\n%s", buf, syserrstr());
		    }
		}
	    }

	    closedir (dirp);
	}
}

/* read in a gallery index, save XML doc in *dp, add to gallery[] */
static void
read1Catalog (FILE *fp, XMLEle **dp, char *idxpath)
{
	LilXML *lp = newLilXML();
	XMLEle *root = NULL;
	XMLEle *ep, *ip;
	int c;

	/* collect gallery element */
	while ((c = fgetc(fp)) != EOF) {
	    char buf[1024];
	    root = readXMLEle (lp, c, buf);
	    if (root)
		break;
	    if (buf[0]) {
		xe_msg (1, "%s:\n%s", idxpath, buf);
		return;
	    }
	}
	if (!root || strcmp (tagXMLEle(root), "gallery")) {
	    xe_msg (1, "%s:\ngallery element not found", idxpath);
	    return;
	}

	/* add one element to gallery[] for each name */
	for (ep = nextXMLEle(root,1); ep != NULL; ep = nextXMLEle(root,0))
	    if (!strcmp (tagXMLEle(ep), "image"))
		for (ip = nextXMLEle(ep,1); ip != NULL; ip = nextXMLEle(ep,0))
		    if (!strcmp (tagXMLEle(ip), "name"))
			addName (ip);

	/* save doc, don't need lil scanner any more */
	*dp = root;
	delLilXML (lp);
}

/* compare pcdata in two XMLEle **, qsort-style */
static int
gt_cmp (const void *e1, const void *e2)
{
	return (strnncmp (pcdataXMLEle(*(XMLEle **)e1), pcdataXMLEle(*(XMLEle **)e2)));
}


/* fill the target selection list from gallery[]  which is a list of name elems.
 */
static void
fillList()
{
	XmString *xmstrtbl;
	int i;

	/* sort galery by name */
	qsort (gallery, ngallery, sizeof(gallery[0]), gt_cmp);

	/* create tmp list of XmStrings */
	xmstrtbl = (XmString *) XtMalloc (ngallery * sizeof(XmString));
	for (i = 0; i < ngallery; i++)
	    xmstrtbl[i]= XmStringCreateSimple(trimws(pcdataXMLEle(gallery[i])));

	/* replace list */
	XtUnmanageChild (sel_w);
	XmListDeleteAllItems (sel_w);
	XmListAddItems (sel_w, xmstrtbl, ngallery, 0);
	XtManageChild (sel_w);

	/* clean up tmp list */
	for (i = 0; i < ngallery; i++)
	    XmStringFree (xmstrtbl[i]);
	XtFree ((char *)xmstrtbl);
}

/* Using info from XML name element nep, fill description in report_w */
static void
fillReport (XMLEle *nep)
{
	XMLEle *p, *e;
	int l;

	/* new report */
	XtUnmanageChild (report_w);
	XmTextSetString (report_w, "");

	/* add each name one per line */
	p = parentXMLEle (nep);
	l = 0;
	for (e = nextXMLEle(p,1); e != NULL; e = nextXMLEle(p,0)) {
	    if (strcmp (tagXMLEle(e), "name") == 0) {
		char *name = pcdataXMLEle(e);
		XmTextInsert (report_w, l, name);
		l += strlen (name);
		XmTextInsert (report_w, l++, "\n");
	    }
	}
	XmTextInsert (report_w, l++, "\n");

	/* add description. replace isolated \n with blank so widget can do
	 * the wrapping
	 */
	e = findXMLEle (p, "description");
	if (e) {
	    char *dp, *des = pcdataXMLEle (e);
	    for (dp = des; *dp; dp++)
		if (dp[0] == '\n' && dp[-1] != '\n' && dp[1] != '\n')
		    *dp = ' ';
	    XmTextInsert (report_w, l, des);
	}

	/* show */
	XmTextShowPosition (report_w, 0);
	XtManageChild (report_w);
}

/* Using info from XML name element nep, fill in the display image */
static void
fillImage (XMLEle *nep)
{
	static XColor xcol[256];
	static int xcolset;
	XMLEle *f;
	Pixmap oldpm, pm;
	char buf[1024];
	char *fn;
	FILE *fp;
	int w, h;

	/* open file, look in private then shared dirs */
	f = findXMLEle (parentXMLEle (nep), "file");
	if (!f) {
	    xe_msg (1, "Gallery entry %s has no file", pcdataXMLEle(nep));
	    return;
	}
	fn = trimws(pcdataXMLEle(f));
	sprintf (buf, "%s/%s", pdir, fn);
	fp = fopenh (buf, "r");
	if (!fp) {
	    sprintf (buf, "%s/%s", sdir, fn);
	    fp = fopenh (buf, "r");
	    if (!fp) {
		xe_msg (1, "%s:\n%s", fn, syserrstr());
		return;
	    }
	}

	/* free last batch of colors */
	if (xcolset) {
	    freeXColors (XtD, xe_cm, xcol, XtNumber(xcol));
	    xcolset = 0;
	}

	/* create the expose pixmap */
	if (jpeg2pm (XtD, xe_cm, fp, &w, &h, &pm, xcol, buf) < 0) {
	    xe_msg (1, "%s:\n%s", fn, buf);
	    fclose (fp);
	    return;
	}
	xcolset = 1;

	/* replace label pixmap and center */
	get_something (pl_w, XmNlabelPixmap, (XtArgVal)&oldpm);
	if (oldpm != XmUNSPECIFIED_PIXMAP)
	    XFreePixmap (XtD, oldpm);
	set_something (pl_w, XmNlabelPixmap, (XtArgVal)pm);
	centerScrollBars(psw_w);

	/* clean up */
	fclose (fp);
}

/* add the given <name> element to the gallery list, growing if necessary */
static void
addName(XMLEle *np)
{
	if (ngallery == mgallery)
	    gallery = (XMLEle **) XtRealloc ((char *)gallery,
					(mgallery+=NMALINC)*sizeof(XMLEle *));
	gallery[ngallery++] = np;
}

/* return index of first gallery entry with name, else -1.
 * ignore case and whitespace
 */
static int
namefind (char *name)
{
	int t, b;

	/* binary search */
	t = ngallery-1;
	b = 0;
	while (b <= t) {
	    int m = (t+b)/2;
	    int c = strnncmp (name, pcdataXMLEle(gallery[m]));
	    if (c == 0)
		return (m);
	    if (c < 0)
		t = m-1;
	    else
		b = m+1;
	}

	return (-1);
}

/* reclaim all storage used for the current gallery */
static void
galReset()
{
	if (galroot[0]) {
	    delXMLEle (galroot[0]);
	    galroot[0] = NULL;
	}
	if (galroot[1]) {
	    delXMLEle (galroot[1]);
	    galroot[1] = NULL;
	}
	if (gallery) {
	    /* pointers in this list were in galroot -- already freed */
	    XtFree ((char *)gallery);
	    gallery = NULL;
	}
	ngallery = mgallery = 0;
}

/* ARGSUSED */
static void
gal_help_cb (Widget w, XtPointer client, XtPointer data)
{
	static char *msg[] = {
"This tool opens and displays picture in the gallery"
};

	hlp_dialog ("Gallery", msg, XtNumber(msg));
}

/* ARGSUSED */
static void
gal_popdown_cb (Widget w, XtPointer client, XtPointer data)
{
}

/* ARGSUSED */
static void
gal_close_cb (Widget w, XtPointer client, XtPointer data)
{
	XtPopdown (galshell_w);
}

/* ARGSUSED */
static void
gal_rescan_cb (Widget w, XtPointer client, XtPointer data)
{
	readCatalogs();
	fillList();
	XtSetSensitive(sky_w, False);
}

/* called when a list selection is made */
/* ARGSUSED */
static void
gal_sel_cb (Widget w, XtPointer client, XtPointer data)
{
	XMLEle *nep;

	nep = selGIP();
	if (!nep)
	    return;

	watch_cursor (1);
	fillReport (nep);
	fillImage (nep);
	XtSetSensitive (sky_w, !!selInDB());
	watch_cursor (0);
}

/* ARGSUSED */
static void
gal_skypt_cb (Widget w, XtPointer client, XtPointer data)
{
	Obj *op = selInDB();

	if (op) {
	    db_update (op);
	    sv_point (op);
	}
}

/* scan db for object whose name matches that which is currently selected.
 * if find return ptr, else NULL
 */
static Obj *
selInDB ()
{
	DupName *dnp;
	int ndn = db_dups(&dnp);
	XMLEle *nep;
	int i;

	nep = selGIP();
	if (!nep)
	    return (NULL);

	for (i = 0; i < ndn; i++)
	    if (!strnncmp (pcdataXMLEle(nep), dnp[i].nm))
		return (dnp[i].op);

	return (NULL);
}

/* return pointer to <name> element of currently selected target, else NULL */
XMLEle *
selGIP ()
{
	int *pos, npos;
	XMLEle *nep;

	if (!XmListGetSelectedPos (sel_w, &pos, &npos))
	    return (NULL);
	nep = gallery[pos[0]-1];	/* List is 1-based */
	XtFree ((char *)pos);
	return (nep);
}

/* trim trailing whitespace off s[] both ends IN PLACE */
static char *
trimws (char *s)
{
	char *s0;

	while (isspace(*s))
	    s++;
	s0 = s;

	while (*s)
	    s++;

	do
	    *s-- = '\0';
	while (s >= s0 && isspace(*s));

	return (s0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: gallerymenu.c,v $ $Date: 2005/10/15 13:12:59 $ $Revision: 1.25 $ $Name:  $"};


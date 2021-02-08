/* handle setup of field star options */

#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#include "xephem.h"

#define	FOVLIM		degrad(30.0)	/* max fov we'll fetch */

static int objfd_cmp (const void *v1, const void *v2);
static int fs_nodbdups (ObjF *fop, int nfop, double ra, double dec,
    double fov);
static int scanchk (Obj *mop, double dupsep, double dupmag, Obj *op,
    double cdec);
static void fs_create_fsshell (void);
static int fs_save (void);
static void fs_setup (void);
static void apply_cb (Widget w, XtPointer client, XtPointer call);
static void ok_cb (Widget w, XtPointer client, XtPointer call);
static void gsctb_cb (Widget w, XtPointer client, XtPointer call);
static void pmtb_cb (Widget w, XtPointer client, XtPointer call);
static void cancel_cb (Widget w, XtPointer client, XtPointer call);
static void help_cb (Widget w, XtPointer client, XtPointer call);

static Widget fsshell_w;	/* the main shell */
static Widget cdtf_w, cdtb_w;	/* GSC CD-ROM text field and toggle button */
static Widget gsc23tb_w;	/* GSC 2.3 network toggle button */
static Widget gsc23tf_w;	/* GSC 2.3 network text field */
static Widget chtf_w, chtb_w;	/* GSC Cache text field and toggle button */
static Widget notf_w, notb_w;	/* USNO text field and toggle button */
static Widget gsc22tb_w;	/* GSC 2.2 toggle button */
static Widget gsc22tf_w;	/* GSC 2.2 text field */
static Widget ppmtf_w, ppmtb_w;	/* PMM text field and toggle button */
static Widget tyctf_w, tyctb_w;	/* Tycho text field and toggle button */
static Widget ucactf_w,ucactb_w;/* UCAC text field and toggle button */
static Widget nodupt_w;		/* no dups toggle button */
static Widget nodupsep_w;	/* no dups sep TF */
static Widget nodupmag_w;	/* no dups mag TF */

/* GSC TB codes */
typedef enum {
    CDROMTB, CACHETB, NETTB, GSC22TB
} TB;

static int cd_on;		/* whether GSC CDROM is currently on */
static char *cd_dn;		/* GSC CDROM dir */
static int ch_on;		/* whether GSC Cache is currently on */
static char *ch_dn;		/* GSC Cache dir */
static int gsc23_on;		/* whether GSC network connection is on */
static char *gsc23_host;		/* GSC network host name */
static int gsc22_on;		/* whether GSC 2.2 STScI connection is on */
static char *gsc22_dn;		/* GSC22 directory */
static int usno_on;		/* whether USNO is currently on */
static char *usno_fn;		/* USNO filename */
static int ppm_on;		/* whether ppm is currently on */
static char *ppm_fn;		/* ppm filename */
static int tyc_on;		/* whether tycho is currently on */
static char *tyc_fn;		/* tycho filename */
static int ucac_on;		/* whether UCAC is currently on */
static char *ucac_dir;		/* UCAC dirname */
static int nodup_on;		/* whether dup checking is on */

static char fscategory[] = "Field stars";

/* call to set up without actually bringing up the menus.
 */
void
fs_create()
{
	if (!fsshell_w) {
	    fs_create_fsshell();
	    (void) fs_save();	/* confirming here is just annoying */
	}
}

void
fs_manage()
{
	fs_create();
	fs_setup();

	XtPopup (fsshell_w, XtGrabNone);
	set_something (fsshell_w, XmNiconic, (XtArgVal)False);
}

/* set new state for nodups, return current state */
int
fs_setnodups (int new)
{
	int old = nodup_on;

	XmToggleButtonSetState (nodupt_w, nodup_on = new, False);
	return (old);
}

/* call to fetch a set of ObjF from the field star sources.
 * don't complain just if none are turned on at the moment.
 * we may eliminate any that seem to be dups of existing FIXED db objects.
 * we turn off any option that appears to not be working.
 * each o_flag in each resulting star is marked with FLDSTAR.
 * return -1 if something looks busted else count.
 * N.B. we only malloc *opp if we return > 0.
 */
int
fs_fetch (
Now *np,		/* now */
double ra, double dec,	/* at np->n_epoch */
double fov,		/* field of view, rads */
double mag,		/* limiting mag */
ObjF **opp) 		/* we set *opp to a malloced list of ObjF */
{
	ObjF *op = NULL;
	int nop = 0;
	int s = 0;
	char msg[256];
	int i, n;

	watch_cursor (1);

	/* clamp fov */
	if (fov > FOVLIM)
	    fov = FOVLIM;

	/* the fetch tools all want and return J2000 values */
	if (epoch == EOD)
	    ap_as (np, J2000, &ra, &dec);
	else
	    precess (epoch, J2000, &ra, &dec);

	/* get the PM catalogs, if desired */
	if (ppm_on) {
	    n = xe2fetch (ppm_fn, np, ra, dec, fov, mag, &op, msg);
	    if (n < 0) {
		xe_msg (1, "PPM: %s", msg);
		ppm_on = 0;
		fs_setup();
	    } else
		nop += n;
	}
	if (tyc_on) {
	    n = xe2fetch (tyc_fn, np, ra, dec, fov, mag, &op, msg);
	    if (n < 0) {
		xe_msg (1, "Tycho: %s", msg);
		tyc_on = 0;
		fs_setup();
	    } else
		nop += n;
	}
	if (ucac_on) {
	    n = UCACFetch (ra, dec, fov, mag, &op, nop, msg);
	    if (n < 0) {
		xe_msg (1, "UCAC: %s", msg);
		ucac_on = 0;
		fs_setup();
	    } else
		nop = n;
	}

	/* now add the GSC 2.3, if desired -- enforce max fov */
	if (gsc23_on) {
	    n = gsc23fetch (gsc23_host, np, ra, dec, fov, mag, &op, nop, msg);
	    if (n < 0) {
		xe_msg (1, "NET: %s", msg);
		gsc23_on = 0;
		fs_setup();
	    } else
		nop = n;
	}
	if (gsc22_on) {
	    n = xe3fetch (gsc22_dn, ra, dec, fov, mag, &op, nop, msg);
	    if (n < 0) {
		xe_msg (1, "GSC 2.2: %s", msg);
		gsc22_on = 0;
		fs_setup();
	    } else
		nop = n;
	}
	if (cd_on || ch_on) {
	    n = GSCFetch (ra, dec, fov, mag, &op, nop, msg);
	    if (n < 0) {
		xe_msg (1, "GSC: %s", msg);
		cd_on = 0;
		fs_setup();
	    } else
		nop = n;
	}

	/* now add USNO if desired -- enforce max fov */
	if (usno_on) {
	    n = USNOFetch (ra, dec, fov, mag, &op, nop, msg);
	    if (n < 0) {
		xe_msg (1, "USNO: %s", msg);
		usno_on = 0;
		fs_setup();
	    } else
		nop = n;
	}

	/* set the s_fields to np */
	for (i = 0; i < nop; i++)
	    obj_cir (np, (Obj *)&op[i]);

	/* squeeze out stars which duplicate each other or the main database.
	 */
	if (nodup_on && nop > 0) {
	    int newnop = fs_nodbdups (op, nop, ra, dec, fov);
	    if (newnop < nop) {
		/* shrink down to just newnop entries now */
		nop = newnop;
		if (nop > 0)
		    op = (ObjF *) realloc ((void *)op, nop*sizeof(ObjF));
		else {
		    free ((void *)op);	/* *all* are dups! */
		    op = NULL;
		    nop = 0;
		}
	    }
	} else {
	    /* still set the FLDSTAR flag on all entries */
	    for (i = 0; i < nop; i++)
		((Obj *)(&op[i]))->o_flags |= FLDSTAR;
	}

	/* pass back the result, if there's something left */
	if (nop > 0) {
	    *opp = op;
	    s = nop;
	} else if (nop < 0) {
	    fs_setup();
	    s = -1;
	}

	watch_cursor (0);

	return (s);
}

/* return 1 if any proper motion catalog is on enabled, else 0 */
int
fs_pmon()
{
	return (ppm_on || tyc_on || ucac_on);
}

/* called to put up or remove the watch cursor.  */
void
fs_cursor (Cursor c)
{
	Window win;

	if (fsshell_w && (win = XtWindow(fsshell_w)) != 0) {
	    if (c)
		XDefineCursor (XtDisplay(toplevel_w), win, c);
	    else
		XUndefineCursor (XtDisplay(toplevel_w), win);
	}
}

/* qsort-style comparison of two ObjF's by dec */
static int
objfd_cmp (const void* v1, const void* v2)
{
	Obj *op1 = (Obj *)v1;
	Obj *op2 = (Obj *)v2;
	double d = op1->s_dec - op2->s_dec;

	if (d < 0.0)
	    return (-1);
	if (d > 0.0)
	    return (1);
	return (0);
}

/* squeeze out all entries in fop[] which are located within dupsep and have a
 *   magnitude within dupmag of any FIXED object currently in the database. We
 *   also squeeze out any GSC or USNO stars which meet those same criteria
 *   for any PM star.
 * when finished, all remaining entries will be contiguous at the front of the
 *   fop array and each will have FLDSTAR set in o_flag.
 * return the number of objects which remain.
 * N.B. we assume FLDSTAR bit is initially off in each o_flag.
 * N.B. we assume all fop[] have already been obj_cir()'d to mm_get_now().
 * N.B. we assume all entries in fop[] are within fov of ra/dec.
 */
static int
fs_nodbdups (ObjF *fop, int nfop, double ra, double dec, double fov)
{
	double rov = fov/2;
	double dupsep, dupmag;
	Obj *op;
	DBScan dbs;
	int l, u, m;
	Obj *mop;
	double diff;
	double cdec;
	char *txt;

	/* get sep and mag */
	txt = XmTextFieldGetString (nodupsep_w);
	dupsep = degrad(atod(txt)/3600.);
	XtFree (txt);
	txt = XmTextFieldGetString (nodupmag_w);
	dupmag = atod(txt);
	XtFree (txt);

	/* sort fop by increasing dec */
	qsort ((void *)fop, nfop, sizeof(ObjF), objfd_cmp);

	/* mark all GSC stars with FLDSTAR flag which are close to any PM.
	 * don't compare GSC and PM stars with themselves.
	 */
	for (m = 0; m < nfop; m++) {
	    mop = (Obj *)&fop[m];

	    /* only check for GSC stars close to PM stars, not v.v. */
	    if (mop->o_name[0] == 'G')
		continue;

	    /* scan each way from mop and mark close GSC stars */
	    cdec = cos(mop->s_dec);
	    for (u = m+1; u < nfop; u++) {
		op = (Obj *)&fop[u];
		if (fabs(mop->s_dec - op->s_dec) >= dupsep)
		    break;
		if (op->o_name[0] == 'G'
			&& cdec*delra(mop->s_ra - op->s_ra) < dupsep
			&& fabs(get_mag(mop) - get_mag(op)) < dupmag)
		    op->o_flags |= FLDSTAR;
	    }
	    for (l = m-1; l >= 0; l--) {
		op = (Obj *)&fop[l];
		if (fabs(mop->s_dec - op->s_dec) >= dupsep)
		    break;
		if (op->o_name[0] == 'G'
			&& cdec*delra(mop->s_ra - op->s_ra) < dupsep
			&& fabs(get_mag(mop) - get_mag(op)) < dupmag)
		    op->o_flags |= FLDSTAR;
	    }
	}

	/* scan all FIXED objects and mark all field stars with FLDSTAR
	 * which appears to be the same any one.
	 */
	for (db_scaninit(&dbs, FIXEDM, NULL, 0); (op = db_scan(&dbs)) != 0; ) {
	    /* update the s_ra/dec fields */
	    db_update (op);

	    /* skip ops outside the given field */
	    cdec = cos(op->s_dec);
	    if (fabs(op->s_dec - dec) > rov || cdec*delra(op->s_ra - ra) > rov)
		continue;

	    /* binary search to find the fop closest to op */
	    l = 0;
	    u = nfop - 1;
	    while (l <= u) {
		m = (l+u)/2;
		mop = (Obj *)(&fop[m]);
		diff = mop->s_dec - op->s_dec;
		if (diff < 0.0)
		    l = m+1;
		else
		    u = m-1;
	    }

	    /* scan each way from m and mark all that are dups with FLDSTAR.
	     * N.B. here, FLDSTAR marks *dups* (not the entries to keep)
	     * N.B. remember, u and l have crossed each other by now.
	     */
	    for (; l < nfop; l++)
		if (scanchk ((Obj *)(&fop[l]), dupsep, dupmag, op, cdec) < 0)
		    break;
	    for (; u >= 0; --u)
		if (scanchk ((Obj *)(&fop[u]), dupsep, dupmag, op, cdec) < 0)
		    break;
	}

	/* squeeze all entries marked with FLDSTAR to the front of fop[]
	 * and mark those that remain with FLDSTAR (yes, the bit now finally
	 * means what it says).
	 */
	for (u = l = 0; u < nfop; u++) {
	    mop = (Obj *)(&fop[u]);
	    if (!(mop->o_flags & FLDSTAR)) {
		mop->o_flags |= FLDSTAR; /* now the mark means a *keeper* */
		if (u > l)
		    memcpy ((void *)(&fop[l]), (void *)mop, sizeof(ObjF));
		l++;
	    }
	}

	return (l);
}

/* if mop is further than dupsep from op in dec, return -1.
 * then if mop is within dupsep in ra and within dupmag in mag too set FLDSTAR
 *   in mop->o_flags.
 * return 0.
 */
static int
scanchk (Obj *mop, double dupsep, double dupmag, Obj *op, double cdec)
{
	if (fabs(mop->s_dec - op->s_dec) >= dupsep)
	    return (-1);
	if (cdec*delra(mop->s_ra - op->s_ra) < dupsep
		&& fabs(get_mag(mop) - get_mag(op)) < dupmag)
	    mop->o_flags |= FLDSTAR;
	return (0);
}

static void
fs_create_fsshell()
{
	Widget fsform_w, f, w;
	Widget l_w, rc_w;
	Arg args[20];
	int n;

	/* create outtern shell and form */

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle, "xephem Field Stars setup"); n++;
	XtSetArg (args[n], XmNiconName, "FStars"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	fsshell_w = XtCreatePopupShell ("FieldStars", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (fsshell_w);
	set_something (fsshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (fsshell_w, "XEphem*FieldStars.x", fscategory, 0);
	sr_reg (fsshell_w, "XEphem*FieldStars.y", fscategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	fsform_w = XmCreateForm (fsshell_w, "FSForm", args, n);
	XtAddCallback (fsform_w, XmNhelpCallback, help_cb, NULL);
	XtManageChild (fsform_w);

	/* make the GSC section title */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (fsform_w, "GSCT", args, n);
	set_xmstring (w, XmNlabelString, "Hubble Guide Star Catalog, GSC:");
	XtManageChild (w);

	    /* make the GSC CD-ROM toggle/text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 60); n++;
	    cdtf_w = XmCreateTextField (fsform_w, "GSCCDDirectory", args, n);
	    defaultTextFN (cdtf_w, 0, "/mnt/cdrom", NULL);
	    wtip (cdtf_w, "Pathname to the root of the CDROM when mounted");
	    XtManageChild (cdtf_w);
	    sr_reg (cdtf_w, NULL, fscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, cdtf_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 10); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    cdtb_w = XmCreateToggleButton (fsform_w, "GSCCD", args, n);
	    XtAddCallback (cdtb_w, XmNvalueChangedCallback, gsctb_cb,
							    (XtPointer)CDROMTB);
	    set_xmstring (cdtb_w, XmNlabelString, "ASP CDROM Directory:");
	    wtip (cdtb_w, "Whether to use the ASP CDROM for GSC stars");
	    XtManageChild (cdtb_w);
	    sr_reg (cdtb_w, NULL, fscategory, 1);

	    /* make the GSC Cache toggle/text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, cdtf_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 60); n++;
	    chtf_w = XmCreateTextField (fsform_w, "GSCCacheDirectory", args, n);
	    defaultTextFN (chtf_w, 0, getShareDir(), "catalogs/gsc");
	    wtip (chtf_w, "Pathname to the GSC subdirectory");
	    XtManageChild (chtf_w);
	    sr_reg (chtf_w, NULL, fscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, cdtf_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, chtf_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 10); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    chtb_w = XmCreateToggleButton (fsform_w, "GSCCache", args, n);
	    XtAddCallback (chtb_w, XmNvalueChangedCallback, gsctb_cb,
							    (XtPointer)CACHETB);
	    set_xmstring (chtb_w, XmNlabelString,  "Local Cache Directory:");
	    wtip (chtb_w, "Whether to use a local disk for GSC stars");
	    XtManageChild (chtb_w);
	    sr_reg (chtb_w, NULL, fscategory, 1);

	    /* make the GSC 2.3 network toggle/text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, chtf_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 60); n++;
	    gsc23tf_w = XmCreateTextField (fsform_w, "GSC23URL", args, n);
	    defaultTextFN (gsc23tf_w, 0,
	    	"http://gsss.stsci.edu/webservices/vo/ConeSearch.aspx", NULL);
	    wtip (gsc23tf_w, "URL from which to fetch GSC 2.3 stars");
	    sr_reg (gsc23tf_w, NULL, fscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, chtf_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, gsc23tf_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 10); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    gsc23tb_w = XmCreateToggleButton (fsform_w, "GSC23Net", args, n);
	    XtAddCallback (gsc23tb_w, XmNvalueChangedCallback, gsctb_cb,
							    (XtPointer)NETTB);
	    set_xmstring (gsc23tb_w, XmNlabelString, "Internet to GSC 2.3:");
	    wtip (gsc23tb_w,"Whether to use Internet to fetch GSC 2.3 stars");
	    sr_reg (gsc23tb_w, NULL, fscategory, 1);
	    XtManageChild (gsc23tb_w);
	    XtManageChild (gsc23tf_w);

	    /* make the GSC 2.2 toggle/text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, gsc23tf_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 60); n++;
	    gsc22tf_w = XmCreateTextField (fsform_w, "GSC22Dir", args, n);
	    wtip (gsc22tf_w, "Directory to GSC 2.2 catalog");
	    defaultTextFN (gsc22tf_w, 0, getShareDir(), "catalogs/GSC2201");
	    sr_reg (gsc22tf_w, NULL, fscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, gsc23tf_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, gsc22tf_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 10); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    gsc22tb_w = XmCreateToggleButton (fsform_w, "GSC22", args, n);
	    XtAddCallback (gsc22tb_w, XmNvalueChangedCallback, gsctb_cb,
							    (XtPointer)GSC22TB);
	    set_xmstring (gsc22tb_w, XmNlabelString, "GSC 2.2 Directory:");
	    wtip (gsc22tb_w,"Whether to use local GSC 2.2 and one PM catalog");
	    sr_reg (gsc22tb_w, NULL, fscategory, 1);
	    XtManageChild (gsc22tb_w);
	    XtManageChild (gsc22tf_w);

	/* make the USNO section title */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, gsc22tf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (fsform_w, "NOMT", args, n);
	set_xmstring (w, XmNlabelString, "USNO A or SA catalogs:");
	XtManageChild (w);

	    /* make the USNO toggle/text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 60); n++;
	    notf_w = XmCreateTextField (fsform_w, "USNODirectory", args, n);
	    wtip (notf_w,
		"Pathname of A1.0, SA1.0 etc catalog series root directory");
	    defaultTextFN (notf_w, 0, getShareDir(), "catalogs/usno");
	    XtManageChild (notf_w);
	    sr_reg (notf_w, NULL, fscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, notf_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 10); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    notb_w = XmCreateToggleButton (fsform_w, "USNO", args, n);
	    set_xmstring (notb_w, XmNlabelString, "Root directory: ");
	    wtip (notb_w, "Whether to access a USNO catalog");
	    XtManageChild (notb_w);
	    sr_reg (notb_w, NULL, fscategory, 1);

	/* make the PM section title */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, notb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (fsform_w, "PMT", args, n);
	set_xmstring (w, XmNlabelString, "Proper Motion catalogs:");
	XtManageChild (w);

	    /* make the PM toggle/text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 60); n++;
	    ppmtf_w = XmCreateTextField (fsform_w, "PPMFilename", args, n);
	    wtip (ppmtf_w, "Full pathname of ppm.xe2 file");
	    defaultTextFN (ppmtf_w, 0, getShareDir(), "catalogs/ppm.xe2");
	    XtManageChild (ppmtf_w);
	    sr_reg (ppmtf_w, NULL, fscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, ppmtf_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 10); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    ppmtb_w = XmCreateToggleButton (fsform_w, "PPM", args, n);
	    XtAddCallback (ppmtb_w, XmNvalueChangedCallback, pmtb_cb, NULL);
	    set_xmstring (ppmtb_w, XmNlabelString, "PPM catalog: ");
	    wtip (ppmtb_w, "Whether to access the PPM catalog, ppm.xe2");
	    XtManageChild (ppmtb_w);
	    sr_reg (ppmtb_w, NULL, fscategory, 1);

	    /* make the Tycho toggle/text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, ppmtf_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 60); n++;
	    tyctf_w = XmCreateTextField (fsform_w, "TychoFilename", args, n);
	    wtip (tyctf_w, "Full pathname of hiptyc2.xe2 file");
	    defaultTextFN (tyctf_w, 0, getShareDir(), "catalogs/hiptyc2.xe2");
	    XtManageChild (tyctf_w);
	    sr_reg (tyctf_w, NULL, fscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, ppmtf_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, tyctf_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 10); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    tyctb_w = XmCreateToggleButton (fsform_w, "Tycho", args, n);
	    XtAddCallback (tyctb_w, XmNvalueChangedCallback, pmtb_cb, NULL);
	    set_xmstring (tyctb_w, XmNlabelString, "Hipparcos + Tycho2: ");
	    wtip (tyctb_w, "Whether to access the combined Hipparcos and Tycho2 catalog");
	    XtManageChild (tyctb_w);
	    sr_reg (tyctb_w, NULL, fscategory, 1);

	    /* make the UCAC toggle/text */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, tyctf_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 60); n++;
	    ucactf_w = XmCreateTextField (fsform_w, "UCACDirectory", args, n);
	    wtip (ucactf_w, "Full path to directory of UCAC files");
	    defaultTextFN (ucactf_w, 0, getShareDir(), "catalogs/UCAC");
	    XtManageChild (ucactf_w);
	    sr_reg (ucactf_w, NULL, fscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, tyctf_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftOffset, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNrightWidget, ucactf_w); n++;
	    XtSetArg (args[n], XmNrightOffset, 10); n++;
	    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	    ucactb_w = XmCreateToggleButton (fsform_w, "UCAC", args, n);
	    XtAddCallback (ucactb_w, XmNvalueChangedCallback, pmtb_cb, NULL);
	    set_xmstring (ucactb_w, XmNlabelString, "UCAC: ");
	    wtip (ucactb_w,"Whether to access the USNO CCD Astrograph Catalog");
	    XtManageChild (ucactb_w);
	    sr_reg (ucactb_w, NULL, fscategory, 1);

	    /* enforce and tie together as 1-of-3 pair */
	    if (XmToggleButtonGetState(ppmtb_w) +
					XmToggleButtonGetState(tyctb_w) +
					XmToggleButtonGetState(ucactb_w) > 1) {
		xe_msg (0, "Only one PM catalog -- using Tycho");
		XmToggleButtonSetState (ppmtb_w, False, False);
		XmToggleButtonSetState (tyctb_w, True, False);
		XmToggleButtonSetState (ucactb_w, False, False);
	    }

	    /* make the No dups toggle and stats fields */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, ucactf_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 10); n++;
	    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	    rc_w = XmCreateRowColumn (fsform_w, "FSDRC", args, n);
	    XtManageChild (rc_w);

	    n = 0;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    nodupt_w = XmCreateToggleButton (rc_w, "NoDups", args, n);
	    set_xmstring (nodupt_w, XmNlabelString, "Skip likely duplicates");
	    wtip (nodupt_w,
	      "When loading, skip objects similar to ones already in memory");
	    XtManageChild (nodupt_w);
	    sr_reg (nodupt_w, NULL, fscategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    l_w = XmCreateLabel (rc_w, "FSDL1", args, n);
	    set_xmstring (l_w, XmNlabelString, "within");
	    XtManageChild (l_w);

	    n = 0;
	    XtSetArg (args[n], XmNcolumns, 4); n++;
	    nodupsep_w = XmCreateTextField (rc_w, "DupSep", args, n);
	    fixTextCursor (nodupsep_w);
	    wtip (nodupsep_w, "skip if closer than this");
	    sr_reg (nodupsep_w, NULL, fscategory, 1);
	    XtManageChild (nodupsep_w);

	    n = 0;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    l_w = XmCreateLabel (rc_w, "FSDL1", args, n);
	    set_xmstring (l_w, XmNlabelString, "arc secs and");
	    XtManageChild (l_w);

	    n = 0;
	    XtSetArg (args[n], XmNcolumns, 4); n++;
	    nodupmag_w = XmCreateTextField (rc_w, "DupMag", args, n);
	    fixTextCursor (nodupmag_w);
	    sr_reg (nodupmag_w, NULL, fscategory, 1);
	    wtip (nodupmag_w, "skip if brightness differs less than this");
	    XtManageChild (nodupmag_w);

	    n = 0;
	    XtSetArg (args[n], XmNmarginHeight, 0); n++;
	    l_w = XmCreateLabel (rc_w, "FSDL1", args, n);
	    set_xmstring (l_w, XmNlabelString, "magnitudes");
	    XtManageChild (l_w);

	/* make the controls across the bottom under a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, rc_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	w = XmCreateSeparator (fsform_w, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNfractionBase, 13); n++;
	f = XmCreateForm (fsform_w, "CF", args, n);
	XtManageChild (f);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 1); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 3); n++;
	    w = XmCreatePushButton (f, "Ok", args, n);
	    wtip (w, "Engage the settings and close if ok");
	    XtAddCallback (w, XmNactivateCallback, ok_cb, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 4); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 6); n++;
	    w = XmCreatePushButton (f, "Apply", args, n);
	    wtip (w, "Engage the settings and remain up");
	    XtAddCallback (w, XmNactivateCallback, apply_cb, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 7); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 9); n++;
	    w = XmCreatePushButton (f, "Close", args, n);
	    wtip (w, "Close this window without doing anything");
	    XtAddCallback (w, XmNactivateCallback, cancel_cb, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 10); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 12); n++;
	    w = XmCreatePushButton (f, "Help", args, n);
	    wtip (w, "More detailed descriptions");
	    XtAddCallback (w, XmNactivateCallback, help_cb, NULL);
	    XtManageChild (w);
}

/* set up the dialog according to our static state */
static void
fs_setup ()
{
	/* GSC */
	XmToggleButtonSetState (cdtb_w, cd_on, False);
	if (cd_dn)
	    XmTextFieldSetString (cdtf_w, cd_dn);

	XmToggleButtonSetState (chtb_w, ch_on, False);
	if (ch_dn)
	    XmTextFieldSetString (chtf_w, ch_dn);

	XmToggleButtonSetState (gsc23tb_w, gsc23_on, False);
	if (gsc23_host)
	    XmTextFieldSetString (gsc23tf_w, gsc23_host);

	XmToggleButtonSetState (gsc22tb_w, gsc22_on, False);
	if (gsc22_dn)
	    XmTextFieldSetString (gsc22tf_w, gsc22_dn);

	/* USNO */
	XmToggleButtonSetState (notb_w, usno_on, False);
	if (usno_fn)
	    XmTextFieldSetString (notf_w, usno_fn);

	/* PM */
	XmToggleButtonSetState (ppmtb_w, ppm_on, False);
	if (ppm_fn)
	    XmTextFieldSetString (ppmtf_w, ppm_fn);
	XmToggleButtonSetState (tyctb_w, tyc_on, False);
	if (tyc_fn)
	    XmTextFieldSetString (tyctf_w, tyc_fn);
	XmToggleButtonSetState (ucactb_w, ucac_on, False);
	if (ucac_dir)
	    XmTextFieldSetString (ucactf_w, ucac_dir);

	XmToggleButtonSetState (nodupt_w, nodup_on, False);
}

/* save the dialog as our static state.
 * we test the new settings and might change them if they seem to be
 *   unreasonable.
 * if any major trouble, issue xe_msg and return -1, else return 0.
 */
static int
fs_save ()
{
	char msg[1024];
	int allok = 1;

	watch_cursor (1);

	/* GSC CDROM and Cache */
	cd_on = XmToggleButtonGetState (cdtb_w);
	if (cd_dn)
	    XtFree (cd_dn);
	cd_dn = XmTextFieldGetString (cdtf_w);
	ch_on = XmToggleButtonGetState (chtb_w);
	if (ch_dn)
	    XtFree (ch_dn);
	ch_dn = XmTextFieldGetString (chtf_w);
	if ((cd_on || ch_on) && GSCSetup (cd_on?cd_dn:NULL, ch_on?ch_dn:NULL,
								    msg) < 0) {
	    xe_msg (1, "Cache: %s", msg);
	    cd_on = 0;
	    ch_on = 0;
	    fs_setup ();
	    allok = 0;
	}

	/* GSC 2.3 network connection */
	gsc23_on = XmToggleButtonGetState (gsc23tb_w);
	if (gsc23_host)
	    XtFree (gsc23_host);
	gsc23_host = XmTextFieldGetString (gsc23tf_w);
	if (gsc23_on) {
	    /* test by actually fetching a small patch */
	    Now *np = mm_get_now();
	    int nop;
	    ObjF *op;

	    op = NULL;
	    nop = gsc23fetch (gsc23_host, np, 1.0, 1.0, degrad(.01), 20.0, &op, 0, msg);
	    if (nop <= 0) {
		if (nop == 0)
		    xe_msg (1, "GSC 2.3 connected but returned no objects");
		else
		    xe_msg (1, "GSC 2.3 fetch: %s", msg);
		gsc23_on = 0;
		fs_setup ();
		allok = 0;
	    } else
		free ((void *)op);
	}

	/* GSC22 */
	gsc22_on = XmToggleButtonGetState (gsc22tb_w);
	if (gsc22_dn)
	    XtFree (gsc22_dn);
	gsc22_dn = XmTextFieldGetString (gsc22tf_w);
	if (gsc22_on) {
	    /* test directory */
	    if (xe3chkdir (gsc22_dn, msg) < 0) {
		xe_msg (1, msg);
		gsc22_on = 0;
		fs_setup ();
		allok = 0;
	    }
	}

	/* USNO */
	usno_on = XmToggleButtonGetState (notb_w);
	if (usno_fn)
	    XtFree (usno_fn);
	usno_fn = XmTextFieldGetString (notf_w);
	if (usno_on && USNOSetup (usno_fn, 1, msg) < 0) {
	    xe_msg (1, "USNO: %s", msg);
	    usno_on = 0;
	    fs_setup();
	    allok = 0;
	}

	/* PPM */
	ppm_on = XmToggleButtonGetState (ppmtb_w);
	if (ppm_fn)
	    XtFree (ppm_fn);
	ppm_fn = XmTextFieldGetString (ppmtf_w);
	if (ppm_on && xe2chkfile (ppm_fn, msg) < 0) {
	    xe_msg (1, "PPM: %s", msg);
	    ppm_on = 0;
	    fs_setup();
	    allok = 0;
	}

	/* Tycho */
	tyc_on = XmToggleButtonGetState (tyctb_w);
	if (tyc_fn)
	    XtFree (tyc_fn);
	tyc_fn = XmTextFieldGetString (tyctf_w);
	if (tyc_on && xe2chkfile (tyc_fn, msg) < 0) {
	    xe_msg (1, "Tycho: %s", msg);
	    tyc_on = 0;
	    fs_setup();
	    allok = 0;
	}

	/* UCAC */
	ucac_on = XmToggleButtonGetState (ucactb_w);
	if (ucac_dir)
	    XtFree (ucac_dir);
	ucac_dir = XmTextFieldGetString (ucactf_w);
	if (ucac_on && UCACSetup (ucac_dir, msg) < 0) {
	    xe_msg (1, "UCAC: %s", msg);
	    ucac_on = 0;
	    fs_setup();
	    allok = 0;
	}

	/* options */
	nodup_on = XmToggleButtonGetState (nodupt_w);

	watch_cursor (0);

	return (allok ? 0 : -1);
}

/* called from Apply */
/* ARGSUSED */
static void
apply_cb (Widget w, XtPointer client, XtPointer call)
{
	if (fs_save() == 0)
	    sv_loadfs (1);
}

/* called from Ok */
/* ARGSUSED */
static void
ok_cb (Widget w, XtPointer client, XtPointer call)
{
	if (fs_save() == 0) {
	    sv_loadfs (1);
	    XtPopdown (fsshell_w);
	}
}

/* called from Ok */
/* ARGSUSED */
static void
cancel_cb (Widget w, XtPointer client, XtPointer call)
{
	/* outta here */
	XtPopdown (fsshell_w);
}

/* called from any of the GSC CDROM, network or Cache toggle buttons.
 * client is one of the TB enums to tell us which.
 */
/* ARGSUSED */
static void
gsctb_cb (Widget w, XtPointer client, XtPointer call)
{
	if (XmToggleButtonGetState(w))
	    switch ((long int)client) {
	    case CDROMTB:	/* FALLTHRU */
	    case CACHETB:
		/* turn off net and gsc22 if turning on CDROM or Cache */
		XmToggleButtonSetState (gsc23tb_w, False, False);
		XmToggleButtonSetState (gsc22tb_w, False, False);
		break;
	    case NETTB:
		/* turn off CDROM and Cache and gsc22 if turning on network */
		XmToggleButtonSetState (cdtb_w, False, False);
		XmToggleButtonSetState (chtb_w, False, False);
		XmToggleButtonSetState (gsc22tb_w, False, False);
		/* turn on at least one PM catalog */
		if (!XmToggleButtonGetState(ppmtb_w)
				    && !XmToggleButtonGetState(tyctb_w) 
				    && !XmToggleButtonGetState(ucactb_w))
		    XmToggleButtonSetState (tyctb_w, True, False);
		break;
	    case GSC22TB:
		/* turn off CDROM and Cache and net if turning on GSC22 */
		XmToggleButtonSetState (cdtb_w, False, False);
		XmToggleButtonSetState (chtb_w, False, False);
		XmToggleButtonSetState (gsc23tb_w, False, False);
		/* turn on at least one PM catalog */
		if (!XmToggleButtonGetState(ppmtb_w)
				    && !XmToggleButtonGetState(tyctb_w) 
				    && !XmToggleButtonGetState(ucactb_w))
		    XmToggleButtonSetState (tyctb_w, True, False);
		break;
	    default:
		printf ("FS: bad client: %d\n", (int)(long int)client);
		abort();
	    }
}

/* used to maintain 1-of-3 ppm vs tycho vs ucac databases.
 * client coming on turns off the other 2.
 */
/* ARGSUSED */
static void
pmtb_cb (Widget w, XtPointer client, XtPointer call)
{
	if (XmToggleButtonGetState(w)) {
	    if (w != ppmtb_w) XmToggleButtonSetState (ppmtb_w, False, False);
	    if (w != tyctb_w) XmToggleButtonSetState (tyctb_w, False, False);
	    if (w != ucactb_w) XmToggleButtonSetState (ucactb_w, False, False);
	}
}

/* called from Ok */
/* ARGSUSED */
static void
help_cb (Widget w, XtPointer client, XtPointer call)
{
        static char *msg[] = {
"Set up which field star sources to use and where they are located."
};

	hlp_dialog ("FieldStars", msg, sizeof(msg)/sizeof(msg[0]));

}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: fsmenu.c,v $ $Date: 2009/01/05 20:55:54 $ $Revision: 1.33 $ $Name:  $"};

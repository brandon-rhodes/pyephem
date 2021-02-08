/* code to manage specifying the local horizon.
 *
 * to build a standalone test program:
 *   cc -DTEST_MAIN -I../../libastro -I../../libip -I/usr/X11R6/lib -L../../libastro -L../../libip -L/usr/X11R6/lib hznmenu.c -lXm -lXt -lX11 -lastro -lip -lm
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/TextF.h>

#include "xephem.h"

static double hzn_getdispl (void);
static int hzn_rdmap (void);
static void hzn_create (void);
static void hzn_close_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_unmap_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_help_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_save_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_edit_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_chsfn_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_displtb_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_displtf_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_filetb_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_filetf_cb (Widget w, XtPointer client, XtPointer call);
static void hzn_radio (int choose_displ);
static void hzn_choose (int choose_displ);
static void smoothProfile(void);
static void buildCon(void);
static int azcmp_f (const void *v1, const void *v2);
static void hzn_read (FILE *fp);
static void hzn_write (FILE *fp);

static Widget hznshell_w;	/* the main shell */
static Widget displtb_w;	/* fixed displacement TB */
static Widget displtf_w;	/* fixed displacement TF */
static Widget filetb_w;		/* file name TB */
static Widget filetf_w;		/* file name TF */
static Widget edittb_w;		/* TB whether editing a new profile */

#define	PSTEP	degrad(1)	/* max alt or az step in profile, rads */
#define	FLDW	20		/* text fields width */

typedef struct {
    double z, a;		/* az/alt profile, rads E of N and up */
} Profile;
static Profile *profile;	/* malloced Profile[nprofile] sorted by inc az*/
static int nprofile;		/* entries in profile[] */
static int want_smoothed = 1;	/* whether want smoothed profile[] */
static int is_smoothed;		/* whether profile[] is smoothed now */
#define	SZP	sizeof(Profile)	/* handy */


static char hzncategory[] = "Horizon Map";	/* Save category */

#if !defined (TEST_MAIN)

void
hzn_manage()
{
	if (!hznshell_w)
	    hzn_create();

        XtManageChild (hznshell_w);
}

void
hzn_unmanage()
{
	if (hznshell_w)
	    XtUnmanageChild (hznshell_w);
}

int
hznDrawing()
{
	if (!hznshell_w)
	    hzn_create();
	return (XmToggleButtonGetState (edittb_w));
}

/* call to turn off editing */
void
hznEditingOff()
{
	if (XmToggleButtonGetState (edittb_w))
	    XmToggleButtonSetState (edittb_w, False, True);
}

/* called to put up or remove the watch cursor.  */
void
hzn_cursor (c)
Cursor c;
{
	Window win;

	if (hznshell_w && (win = XtWindow(hznshell_w)) != 0) {
	    Display *dsp = XtDisplay(hznshell_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* return number of profile entries */
int
hznNProfile()
{
	if (!hznshell_w)
	    hzn_create();
	if (want_smoothed && !is_smoothed)
	    smoothProfile();
	return (nprofile);
}

/* whether to smooth profiles */
void
hznRawProfile (int on)
{
	want_smoothed = !on;
}

/* given an index into the profile return the alt and az */
void
hznProfile (int i, double *altp, double *azp)
{
	if (!hznshell_w)
	    hzn_create();
	if (i < 0 || i >= nprofile) {
	    printf ("Bogus hznProfile %d\n", i);
	    abort();
	}
	*altp = profile[i].a;
	*azp = profile[i].z;
}

/* given an az, return the horizon altitude, both in rads.
 */
double
hznAlt(az)
double az;
{
	Profile *pb, *pt;
	double daz;
	int t, b;

	if (!hznshell_w)
	    hzn_create();
	if (want_smoothed && !is_smoothed)
	    smoothProfile();

	/* binary bracket */
	range (&az, 2*PI);
	t = nprofile - 1;
	b = 0;
	while (b < t-1) {
	    int m = (t+b)/2;
	    double maz = profile[m].z;
	    if (az < maz)
		t = m;
	    else
		b = m;
	}

	pt = &profile[t];
	pb = &profile[b];
	daz = pt->z - pb->z;
	if (b == t || daz == 0)
	    return (pb->a);
	return (pb->a + (az - pb->z)*(pt->a - pb->a)/daz);
}

#endif	/* !TEST_MAIN */

/* add alt/az to profile[]
 * we remove all profile entries from this az to previous az.
 * call with start=1 to initialize "previous" position.
 */
void
hznAdd (int start, double newalt, double newaz)
{
	static int prev;		/* index of previous insertion */

	/* first time pick closest smaller entry as prev */
	if (start) {
	    for (prev = 0; prev < nprofile-1; prev++)
		if (profile[prev+1].z >= newaz)
		    break;
	}

	/* break in two when crossing through north */
	if (profile[prev].z > newaz + PI) {
	    double dz0 = 2*PI-profile[prev].z;
	    double dz = dz0 + newaz;
	    double alt0 = profile[prev].a + dz0*(newalt-profile[prev].a)/dz;
	    hznAdd (0, alt0, 2*PI);
	    prev = 0;
	    profile[prev].a = alt0;
	    profile[prev].z = 0;
	    hznAdd (0, newalt, newaz);
	    return;
	} else if (newaz > profile[prev].z + PI) {
	    double dz0 = 0-profile[prev].z;
	    double dz = dz0 - (2*PI - newaz);
	    double alt0 = profile[prev].a + dz0*(newalt-profile[prev].a)/dz;
	    hznAdd (0, alt0, 0.0);
	    prev = nprofile - 1;
	    profile[prev].a = alt0;
	    profile[prev].z = 2*PI;
	    hznAdd (0, newalt, newaz);
	    return;
	}

	/* remove portion from (prev .. new] */
	/* TODO: just compute the indeces and do one memmove() */
	if (profile[prev].z < newaz) {
	    for (++prev; prev < nprofile && profile[prev].z <= newaz; )
		memmove (profile+prev, profile+prev+1, (--nprofile-prev)*SZP);
	} else if (profile[prev].z > newaz) {
	    for (--prev; prev >= 0 && profile[prev].z >= newaz; prev--)
		memmove (profile+prev, profile+prev+1, (--nprofile-prev)*SZP);
	    prev++;
	} else { /* profile[prev].z == newaz */
	    profile[prev].a = newalt;
	    return;
	}

	/* make room for and insert new entry at prev */
	profile = (Profile *) XtRealloc ((char *) profile, (nprofile+1)*SZP);
	memmove (profile+prev+1, profile+prev, (nprofile++ -prev)*SZP);
	profile[prev].a = newalt;
	profile[prev].z = newaz;

	/* no longer smoothed */
	is_smoothed = 0;
}

#if !defined (TEST_MAIN)

static void
hzn_create()
{
	Widget w, sep_w, om_w;
	Arg args[20];
	char *s[1];
	int n;

	/* create main form */

	n = 0;
	XtSetArg (args[n], XmNautoUnmanage, False); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	XtSetArg (args[n], XmNhorizontalSpacing, 4); n++;
	XtSetArg (args[n], XmNdefaultPosition, False); n++;
	hznshell_w = XmCreateFormDialog (svshell_w, "Horizon", args, n);
	set_something (hznshell_w, XmNcolormap, (XtArgVal)xe_cm);
	set_xmstring (hznshell_w, XmNdialogTitle, "xephem Sky Horizon setup");
        XtAddCallback (hznshell_w, XmNhelpCallback, hzn_help_cb, NULL);
	XtAddCallback (hznshell_w, XmNunmapCallback, hzn_unmap_cb, NULL);
	sr_reg (XtParent(hznshell_w), "XEphem*Horizon.x", hzncategory, 0);
	sr_reg (XtParent(hznshell_w), "XEphem*Horizon.y", hzncategory, 0);

	/* fixed */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 5); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	displtb_w = XmCreateToggleButton (hznshell_w, "UseDisplacement",args,n);
	XtAddCallback (displtb_w, XmNvalueChangedCallback, hzn_displtb_cb, 0);
	wtip (displtb_w, "Define horizon with one altitude at all azimuths");
	XmToggleButtonGetState(displtb_w);
	sr_reg (displtb_w, NULL, hzncategory, 0);
	set_xmstring (displtb_w, XmNlabelString, " Constant ° ");
	XtManageChild (displtb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, displtb_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNcolumns, FLDW); n++;
	displtf_w = XmCreateTextField (hznshell_w, "Displacement", args, n);
	wtip (displtf_w, "Angle of Horizon cutoff above local horizontal");
	XtAddCallback (displtf_w, XmNactivateCallback, hzn_displtf_cb, 0);
	sr_reg (displtf_w, NULL, hzncategory, 0);
	XtManageChild (displtf_w);

	/* file field */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, displtf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 5); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	filetb_w = XmCreateToggleButton (hznshell_w, "UseMapFile", args, n);
	wtip (filetb_w, "Define horizon as a table of altitudes and azimuths");
	XtAddCallback (filetb_w, XmNvalueChangedCallback, hzn_filetb_cb, 0);
	set_xmstring (filetb_w, XmNlabelString, " File name: ");
	XtManageChild (filetb_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, displtf_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, filetb_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNcolumns, FLDW); n++;
	filetf_w = XmCreateTextField (hznshell_w, "MapFilename", args, n);
	wtip (filetf_w, "Name of file containing horizon table");
	XtAddCallback (filetf_w, XmNactivateCallback, hzn_filetf_cb, NULL);
	sr_reg (filetf_w, NULL, hzncategory, 0);
	XtManageChild (filetf_w);

	/* perform the default */
	hzn_choose (XmToggleButtonGetState(displtb_w));

	/* browse list */

	s[0] = ".hzn";
	om_w = createFSM (hznshell_w, s, 1, "auxil", hzn_chsfn_cb);
	wtip (om_w, "Select a file containing a horizon profile");

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, filetf_w); n++;
	XtSetArg (args[n], XmNtopOffset, 8); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNleftWidget, filetb_w); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetValues (om_w, args, n);

	/* save */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, filetf_w); n++;
	XtSetArg (args[n], XmNtopOffset, 10); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 10); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
	XtSetArg (args[n], XmNrightWidget, filetb_w); n++;
	w = XmCreatePushButton (hznshell_w, "Save", args, n);
	XtAddCallback (w, XmNactivateCallback, hzn_save_cb, NULL);
	wtip (w, "Save current horizon drawing to the file");
	XtManageChild (w);

	/* edit new */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, om_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 5); n++;
	XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
	edittb_w = XmCreateToggleButton (hznshell_w, "Edit", args, n);
	set_xmstring (edittb_w, XmNlabelString, "Edit with mouse");
	XtAddCallback (edittb_w, XmNvalueChangedCallback, hzn_edit_cb, NULL);
	wtip (edittb_w, "Define horizon as a table of altitudes and azimuths");
	XtManageChild (edittb_w);

	/* controls at the bottom */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, edittb_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	sep_w = XmCreateSeparator (hznshell_w, "HZS", args, n);
	XtManageChild (sep_w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 22); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 38); n++;
	w = XmCreatePushButton (hznshell_w, "Close", args, n);
        XtAddCallback (w, XmNactivateCallback, hzn_close_cb, NULL);
	wtip (w, "Close this window");
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, sep_w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNleftPosition, 62); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg (args[n], XmNrightPosition, 78); n++;
	w = XmCreatePushButton (hznshell_w, "Help", args, n);
        XtAddCallback (w, XmNactivateCallback, hzn_help_cb, NULL);
	wtip (w, "Get more information about this window");
	XtManageChild (w);
}

/* called from Close */
/* ARGSUSED */
static void
hzn_close_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XtUnmanageChild (hznshell_w);
}

/* called when unmapped for any reason */
/* ARGSUSED */
static void
hzn_unmap_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hznEditingOff();
}

/* called when edit TB changes */
/* ARGSUSED */
static void
hzn_edit_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XmToggleButtonGetState(w)) {
	    sv_hznOn();				/* public service feature */
	    hzn_radio (0);
	}
}

/* called from Help */
/* ARGSUSED */
static void
hzn_help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
        static char *msg[] = {"Specify local horizon."};

	hlp_dialog ("Horizon", msg, sizeof(msg)/sizeof(msg[0]));

}

/* called from the Save PB and the Save file name TF.
 * N.B. don't use call
 */
/* ARGSUSED */
static void
hzn_save_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char buf[1024];
	char *fn;
	FILE *fp;

	/* create file */
	fn = XmTextFieldGetString (filetf_w);
	sprintf (buf, "%s/%s", getPrivateDir(), fn);
	fp = fopen (buf, "w");
	if (!fp) {
	    sprintf (buf+strlen(buf), ":\n%s", syserrstr());
	    XtFree (fn);
	    return;
	}

	/* write profile */
	hzn_write (fp);
	
	/* finished */
	fclose (fp);
	hznEditingOff();
	XtFree (fn);
}

/* Displacement TB callback */
/* ARGSUSED */
static void
hzn_displtb_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hzn_choose (XmToggleButtonGetState (w));
}

/* Displacement TF callback */
/* ARGSUSED */
static void
hzn_displtf_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hzn_choose (1);
}

/* File TB callback */
/* ARGSUSED */
static void
hzn_filetb_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hzn_choose (!XmToggleButtonGetState (w));
}

/* File TF callback */
/* ARGSUSED */
static void
hzn_filetf_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	hzn_choose (0);
}

/* called when a file is chosen from the cascading list.
 * file name is w's label.
 */
/* ARGSUSED */
static void
hzn_chsfn_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char *fn;

	/* get file name, load into filetf_w */
	get_xmstring (w, XmNlabelString, &fn);
	XmTextFieldSetString (filetf_w, fn);
	XtFree (fn);

	/* display if possible */
	hzn_choose (0);
	hznEditingOff();
	sv_hznOn();				/* public service feature */
}

/* get displacement string into displ */
static double
hzn_getdispl()
{
	double d;
	char *str = XmTextFieldGetString (displtf_w);
	d = degrad(strtod(str,NULL));
	XtFree (str);
	return (d);
}

/* implement radio box behavior */
static void
hzn_radio (int choose_displ)
{
        XmToggleButtonSetState (displtb_w, choose_displ, False);
        XmToggleButtonSetState (filetb_w, !choose_displ, False);
}

/* called to make a choice.
 * control the Tbs as well as do the work.
 */
static void
hzn_choose (choose_displ)
int choose_displ;
{
	/* do the work */
	if (choose_displ)
	    buildCon();
	else {
	    if (hzn_rdmap() < 0) {
		choose_displ = 1;	/* revert back */
		buildCon();
	    }
	}

	/* set radio choice */
	hzn_radio (choose_displ);

	/* sky view update */
	sv_update (mm_get_now(), 1);

}

/* open the horizon map named in filetf_w and create a profile[] smoothed by
 * increasing az.
 * we make sure the profile has complete coverage from 0..360 degrees.
 * return 0 if ok, else -1
 */
static int
hzn_rdmap()
{
	char *fn = XmTextFieldGetString (filetf_w);
	FILE *fp;

	/* open file, try Private then Shared area */
	fp = fopend (fn, "auxil", "r");
	if (!fp) {
	    XtFree (fn);
	    return (-1);
	}

	/* read new profile */
	if (profile) {
	    XtFree ((char *)profile);
	    profile = NULL;
	}
	nprofile = 0;
	hzn_read (fp);
	fclose (fp);

	if (nprofile == 0) {
	    xe_msg (1, "%s:\nContains no horizon entries.\nReverting to constant displacement", fn);
	    XtFree (fn);
	    buildCon();
	    return (-1);
	} else {
	    xe_msg (0, "%s:\nRead %d horizon entr%s.", fn, nprofile,
						nprofile == 1 ? "y" : "ies");
	}

	/* if get here nprofile > 0. smooth and keep it that way */
	smoothProfile();

	/* done with filename */
	XtFree (fn);
	return (0);
}

/* fill the profile with a constant elevation model */
static void
buildCon()
{
	double a = hzn_getdispl ();

	/* seed the profile then smooth */
	profile = (Profile *) XtRealloc ((char *)profile, 1*SZP);
	profile[0].z = 0;
	profile[0].a = a;
	nprofile = 1;
	smoothProfile();
}

#endif	/* !TEST_MAIN */

/* make sure the profile has complete coverage from 0..360 degrees with
 * no steps in Alt or Az greater than PSTEP.
 */
static void
smoothProfile()
{
	Profile *newp;
	double alt0;
	int nnew, nmax;
	int i;

	if (!nprofile)
	    return;

	/* insure sorted */
	qsort ((void *)profile, nprofile, SZP, azcmp_f);

	/* seed with 0 crossing */
	alt0 = profile[0].a;
	nmax = 100;
	newp = (Profile *) XtMalloc (nmax*SZP);
	newp[0].a = alt0;
	newp[0].z = 0;
	nnew = 1;

	/* make new copy, insuring complete PSTEP coverage */
	for (i = 0; i <= nprofile; i++) {
	    double la = newp[nnew-1].a;
	    double lz = newp[nnew-1].z;
	    double da = (i == nprofile ? alt0 : profile[i].a) - la;
	    double dz = (i == nprofile ? 2*PI : profile[i].z) - lz;
	    int na = (int)ceil(fabs(da)/PSTEP);
	    int nz = (int)ceil(fabs(dz)/PSTEP);
	    int j, n = na > nz ? na : nz;
	    if (nnew + n > nmax)
		newp = (Profile *) XtRealloc((char*)newp,(nmax=nnew+n+100)*SZP);
	    for (j = 1; j <= n; j++) {
		newp[nnew].a = la + j*da/n;
		newp[nnew].z = lz + j*dz/n;
		nnew++;
	    }
	}

	/* newp is now the new profile */
	XtFree ((char *)profile);
	newp = (Profile *) XtRealloc((char*)newp, nnew*SZP);
	profile = newp;
	nprofile = nnew;

	/* ok */
	is_smoothed = 1;
}

/* compare two Profiles' az, in qsort fashion */
static int
azcmp_f (const void *v1, const void *v2)
{
	double diff = ((Profile *)v1)->z - ((Profile *)v2)->z;
	return (diff < 0 ? -1 : (diff > 0 ? 1 : 0));
}

/* read the given file into profile[] */
static void
hzn_read (FILE *fp)
{
	char buf[1024];

	/* read and store in profile[] */
	while (fgets (buf, sizeof(buf), fp)) {
	    double a, z;
	    if (sscanf (buf, "%lf %lf", &z, &a) != 2)
		continue;
	    z = degrad(z);
	    a = degrad(a);
	    radecrange (&z, &a);
	    profile = (Profile *) XtRealloc ((char *) profile,(nprofile+1)*SZP);
	    profile[nprofile].a = a;
	    profile[nprofile].z = z;
	    nprofile++;
	}
}

/* write profile to file fp */
static void
hzn_write (FILE *fp)
{
	int i;

	for (i = 0; i < nprofile; i++)
	    fprintf (fp, "%8.4f %8.4f\n", raddeg(profile[i].z),
							raddeg(profile[i].a));
}

#if defined (TEST_MAIN)

int
main (int ac, char *av[])
{
	hzn_read (stdin);
	hznAdd (0, degrad(70), degrad(20));
	hznAdd (0, degrad(40), degrad(350));
	hznAdd (0, degrad(60), degrad(300));
	hznAdd (0, degrad(40), degrad(301));
	hznAdd (0, degrad(20), degrad(302));
	hznAdd (0, degrad(10), degrad(303));
	hznAdd (0, degrad(50), degrad(304));
	smoothProfile();
	hzn_write (stdout);

	return (0);
}

#endif	/* TEST_MAIN */

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: hznmenu.c,v $ $Date: 2004/05/10 23:36:34 $ $Revision: 1.21 $ $Name:  $"};

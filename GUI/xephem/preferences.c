/* code to support the preferences facility.
 */

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Separator.h>

#include "xephem.h"



char prefcategory[] = "Main -- Preferences";	/* Save category */

/* info to build a preference control */
typedef struct {
    int prefname;	/* one of Preferences enum */
    char *pdname;	/* pulldown name */
    char *ctip;		/* tip text for the main cascade pair */
    char *cblabel;	/* cascade button label */
    char cbmne;		/* cascade button mnemonic character */
    XtCallbackProc cb;	/* callback function */
    int op1pref;	/* option 1 PREF code */
    char *op1name;	/* option 1 TB name */
    char op1mne;	/* option 1 TB mnemonic character */
    char *op1tip;	/* option 1 tip string */
    int op2pref;	/* option 2 PREF code */
    char *op2name;	/* option 2 TB name */
    char op2mne;	/* option 2 TB mnemonic character */
    char *op2tip;	/* option 2 tip string */
    int op3pref;	/* option 3 PREF code */
    char *op3name;	/* option 3 TB name */
    char op3mne;	/* option 3 TB mnemonic character */
    char *op3tip;	/* option 3 tip string */
} PrefSet;

static void pref_topogeo_cb (Widget w, XtPointer client, XtPointer call);
static void pref_date_cb (Widget w, XtPointer client, XtPointer call);
static void pref_units_cb (Widget w, XtPointer client, XtPointer call);
static void pref_tz_cb (Widget w, XtPointer client, XtPointer call);
static void pref_dpy_prec_cb (Widget w, XtPointer client, XtPointer call);
static void pref_msg_bell_cb (Widget w, XtPointer client, XtPointer call);
static void pref_prefill_cb (Widget w, XtPointer client, XtPointer call);
static void pref_tips_cb (Widget w, XtPointer client, XtPointer call);
static void pref_confirm_cb (Widget w, XtPointer client, XtPointer call);
static void pref_weekstart_cb (Widget w, XtPointer client, XtPointer call);
static void pref_build (Widget pd, PrefSet *p);

static PrefSet prefsets[] = {
    {PREF_EQUATORIAL, "Equatorial",
	"Whether RA/Dec values are topocentric or geocentric",
	"Equatorial", 'E', pref_topogeo_cb,
	PREF_TOPO, "Topocentric", 'T', "local perspective",
	PREF_GEO, "Geocentric", 'G', "Earth-centered perspective"
    },

    {PREF_DPYPREC, "Precision",
       "Whether numeric values are shown with more or fewer significant digits",
	"Precision", 'P', pref_dpy_prec_cb,
	PREF_HIPREC, "Hi", 'H', "display full precision",
	PREF_LOPREC, "Low", 'L', "use less room"
    },

    {PREF_MSG_BELL, "LogBell",
	"Whether to beep when a message is added to the System log",
	"Log Bell", 'M', pref_msg_bell_cb,
	PREF_NOMSGBELL, "Off", 'f', "other people are busy",
	PREF_MSGBELL, "On", 'O', "the beeps are useful"
    },

    {PREF_PRE_FILL, "PromptPreFill",
	"Whether prompt dialogs are prefilled with their current value",
	"Prompt Prefill", 'f', pref_prefill_cb,
	PREF_NOPREFILL, "No", 'N', "fresh prompt each time",
	PREF_PREFILL, "Yes", 'Y', "current value is often close"
    },

    {PREF_UNITS, "Units",
	"Whether to use english or metric units",
	"Units", 'U', pref_units_cb,
	PREF_ENGLISH, "English", 'E', "Feet, Fahrenheit",
	PREF_METRIC, "Metric", 'M', "Meters, Celsius"
    },

    {PREF_ZONE, "TZone",
	"Whether time stamps and the calendar are in local time or UTC",
	"Time zone", 'z', pref_tz_cb,
	PREF_LOCALTZ, "Local", 'L', "as per TZ Offset",
	PREF_UTCTZ, "UTC", 'U', "Coordinated Universal Time"
    },

    {PREF_TIPS, "Tips",
	"Whether to display these little tip boxes!",
	"Show help tips", 't', pref_tips_cb,
	PREF_NOTIPS, "No", 'N', "they are in the way",
	PREF_TIPSON, "Yes", 'Y', "they are faster than reading Help"
    },

    {PREF_CONFIRM, "Confirm",
	"Whether to ask before performing irreversible actions",
	"Confirmations", 'C', pref_confirm_cb,
	PREF_NOCONFIRM, "No", 'N', "just do it",
	PREF_CONFIRMON, "Yes", 'Y', "ask first"
    },

    {PREF_WEEKSTART, "WeekStart",
	"First day of week in calendar",
	"Start week on", 'w', pref_weekstart_cb,
	PREF_SAT, "Saturday", 'a', "start each calendar week on Saturday",
	PREF_SUN, "Sunday",   'u', "start each calendar week on Sunday",
	PREF_MON, "Monday",   'M', "start each calendar week on Monday"
    },

    {PREF_DATE_FORMAT, "DateFormat",
	"Format for displaying dates",
	"Date format", 'D', pref_date_cb,
	PREF_MDY, "M/D/Y", 'M', "Month / Day / Year",
	PREF_YMD, "Y/M/D", 'Y', "Year / Month / Day",
	PREF_DMY, "D/M/Y", 'D', "Day / Month / Year"
    },
};

/* record of preferences values */
static int prefs[NPREFS];

/* Create "Preferences" PulldownMenu.
 * use the given menu_bar widget as a base.
 * this is called early when the main menu bar is being built..
 * initialize the prefs[] array from the initial state of the toggle buttons.
 * also, tack on some Save and resource controls.
 */
void
pref_create_pulldown (menu_bar)
Widget menu_bar;
{
	Widget w, cb_w, pd;
	Arg args[20];
	int i, n;

	/* make the pulldown */
	n = 0;
	pd = XmCreatePulldownMenu (menu_bar, "Preferences", args, n);

	/* install the preferences */
	for (i = 0; i < XtNumber(prefsets); i++)
	    pref_build (pd, &prefsets[i]);

	/* glue the pulldown to the menubar with a cascade button */

	n = 0;
	XtSetArg (args[n], XmNsubMenuId, pd);  n++;
	XtSetArg (args[n], XmNmnemonic, 'P'); n++;
	cb_w = XmCreateCascadeButton (menu_bar, "PreferencesCB", args, n);
	set_xmstring (cb_w, XmNlabelString, "Preferences");
	XtManageChild (cb_w);
	wtip (cb_w, "Options effecting overall XEphem operation");

	/* tack on some Save and other resource controls */

	n = 0;
	w = XmCreateSeparator (pd, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	w = XmCreatePushButton (pd, "Fonts", args, n);
	XtAddCallback (w, XmNactivateCallback, (XtCallbackProc)srf_manage, 0);
	wtip (w, "Try different fonts");
	set_xmstring (w, XmNlabelString, "Fonts...");
	XtManageChild (w);

	n = 0;
	w = XmCreatePushButton (pd, "Colors", args, n);
	XtAddCallback (w, XmNactivateCallback, (XtCallbackProc)src_manage, 0);
	wtip (w, "Try different colors");
	set_xmstring (w, XmNlabelString, "Colors...");
	XtManageChild (w);

	n = 0;
	w = XmCreatePushButton (pd, "Save", args, n);
	XtAddCallback (w, XmNactivateCallback, (XtCallbackProc)sr_manage, 0);
	wtip (w, "Save settings to disk");
	set_xmstring (w, XmNlabelString, "Save...");
	XtManageChild (w);
}

/* called anytime we want to know a preference.
 */
int
pref_get(pref)
Preferences pref;
{
	return (prefs[pref]);
}

/* call to force a certain preference, return the old setting.
 * Use this wisely.. it does *not* change the menu system.
 */
int
pref_set (pref, new)
Preferences pref;
int new;
{
	int prior = pref_get(pref);
	prefs[pref] = new;
	return (prior);
}

/* return 1 if want to confirm, else 0 */
int
confirm()
{
	return (pref_get (PREF_CONFIRM) == PREF_CONFIRMON);
}

/* build one option off the given pulldown menu.
 * for pairs, state of first fallback[] sets other; for triples must set all 3.
 */
static void
pref_build (pd, pp)
Widget pd;	/* parent pulldown menu */
PrefSet *pp;
{
	Widget pr, cb_w;
	Widget tb1_w, tb2_w, tb3_w;
	Arg args[20];
	int n;

	n = 0;
	XtSetArg (args[n], XmNradioBehavior, True); n++;
	pr = XmCreatePulldownMenu (pd, pp->pdname, args,n);

	    /* option 1 */

	    n = 0;
	    XtSetArg (args[n], XmNmnemonic, pp->op1mne); n++;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    tb1_w = XmCreateToggleButton (pr, pp->op1name, args, n);
	    XtAddCallback (tb1_w, XmNvalueChangedCallback, pp->cb,
							(XtPointer)(long int)pp->op1pref);
	    wtip (tb1_w, pp->op1tip);
	    XtManageChild (tb1_w);
	    sr_reg (tb1_w, NULL, prefcategory, 1);

	    /* option 2 */

	    n = 0;
	    XtSetArg (args[n], XmNmnemonic, pp->op2mne); n++;
	    XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
	    tb2_w = XmCreateToggleButton (pr, pp->op2name, args, n);
	    XtAddCallback (tb2_w, XmNvalueChangedCallback, pp->cb,
							(XtPointer)(long int)pp->op2pref);
	    wtip (tb2_w, pp->op2tip);
	    XtManageChild (tb2_w);

	    /* and maybe option 3 .. key off tip */

	    if (pp->op3tip) {

		/* when have 3 must save all */
		sr_reg (tb2_w, NULL, prefcategory, 1);

		n = 0;
		XtSetArg (args[n], XmNmnemonic, pp->op3mne); n++;
		XtSetArg (args[n], XmNvisibleWhenOff, True); n++;
		tb3_w = XmCreateToggleButton (pr, pp->op3name, args, n);
		XtAddCallback (tb3_w, XmNvalueChangedCallback, pp->cb,
							(XtPointer)(long int)pp->op3pref);
		wtip (tb3_w, pp->op3tip);
		XtManageChild (tb3_w);
		sr_reg (tb3_w, NULL, prefcategory, 1);

		if (XmToggleButtonGetState(tb1_w)) {
		    prefs[pp->prefname] = pp->op1pref;
		    XmToggleButtonSetState (tb2_w, False, False);
		    XmToggleButtonSetState (tb3_w, False, False);
		} else if (XmToggleButtonGetState(tb2_w)) {
		    prefs[pp->prefname] = pp->op2pref;
		    XmToggleButtonSetState (tb1_w, False, False);
		    XmToggleButtonSetState (tb3_w, False, False);
		} else if (XmToggleButtonGetState(tb3_w)) {
		    prefs[pp->prefname] = pp->op3pref;
		    XmToggleButtonSetState (tb1_w, False, False);
		    XmToggleButtonSetState (tb2_w, False, False);
		} else {
		    printf ("No default for %s preference\n", pp->pdname);
		    abort();
		}

	    } else {
		/* only option 1 is in fallback, set 2 from it */
		int t1 = XmToggleButtonGetState(tb1_w);
		XmToggleButtonSetState (tb2_w, !t1, False);
		prefs[pp->prefname] = t1 ? pp->op1pref : pp->op2pref;
	    }

	    n = 0;
	    XtSetArg (args[n], XmNsubMenuId, pr);  n++;
	    XtSetArg (args[n], XmNmnemonic, pp->cbmne); n++;
	    cb_w = XmCreateCascadeButton (pd, "PrefCB", args, n);
	    XtManageChild (cb_w);
	    set_xmstring (cb_w, XmNlabelString, pp->cblabel);
	    wtip (cb_w, pp->ctip);
}

/* called when a PREF_DATE_FORMAT preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_date_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_DATE_FORMAT] = (long int)client;
	    redraw_screen (1);
	}
}

/* called when a PREF_UNITS preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_units_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_UNITS] = (long int)client;
	    redraw_screen (1);
	}
}

/* called when a PREF_ZONE preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_tz_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_ZONE] = (long int)client;
	    redraw_screen (1);
	}
}

/* called when a PREF_DPYPREC preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_dpy_prec_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_DPYPREC] = (long int)client;
	    redraw_screen (1);
	}
}

/* called when a PREF_EQUATORIAL preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_topogeo_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_EQUATORIAL] = (long int)client;
	    redraw_screen (1);
	}
}

/* called when a PREF_MSG_BELL preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_msg_bell_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_MSG_BELL] = (long int)client;
	}
}

/* called when a PREF_PRE_FILL preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_prefill_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_PRE_FILL] = (long int)client;
	}
}

/* called when a PREF_TIPS preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_tips_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_TIPS] = (long int)client;
	}
}

/* called when a PREF_CONFIRM preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_confirm_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_CONFIRM] = (long int)client;
	}
}

/* called when a PREF_WEEKSTART preference changes.
 * the new value is in client.
 */
/* ARGSUSED */
static void
pref_weekstart_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmToggleButtonCallbackStruct *s = (XmToggleButtonCallbackStruct *)call;

	if (s->set) {
	    prefs[PREF_WEEKSTART] = (long int)client;
	    redraw_screen (1);
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: preferences.c,v $ $Date: 2009/01/05 20:55:54 $ $Revision: 1.20 $ $Name:  $"};

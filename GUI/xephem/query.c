/* general purpose way to ask a question, in X.
 */

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/MessageB.h>


#include "xephem.h"


static Widget query_create (Widget top);
static void query_cb (Widget w, XtPointer client, XtPointer call);

static void (*funcs[3])();

/* put up an app-modal query message with up to three buttons.
 * all args can safely be NULL; buttons without labels will be turned off.
 * tw must be a shell and the query will be placed on top of the shell.
 */
void
query (
Widget tw,		/* toplevel widget */
char *msg,		/* query message */
char *label0,		/* label for button 0 */
char *label1,		/* label for button 1 */
char *label2,		/* label for button 2 */
void (*func0)(),	/* func to call if button 0 is pushed */
void (*func1)(),	/* func to call if button 1 is pushed */
void (*func2)())	/* func to call if button 2 is pushed */
{
	Widget q_w = query_create(tw);

	funcs[0] = func0;
	funcs[1] = func1;
	funcs[2] = func2;

	if (label0) {
	    set_xmstring (q_w, XmNokLabelString, label0);
	    XtManageChild (XmMessageBoxGetChild (q_w, XmDIALOG_OK_BUTTON));
	} else
	    XtUnmanageChild (XmMessageBoxGetChild (q_w, XmDIALOG_OK_BUTTON));

	if (label1) {
	    set_xmstring (q_w, XmNcancelLabelString, label1);
	    XtManageChild (XmMessageBoxGetChild (q_w, XmDIALOG_CANCEL_BUTTON));
	} else
	    XtUnmanageChild (XmMessageBoxGetChild (q_w,XmDIALOG_CANCEL_BUTTON));

	if (label2) {
	    set_xmstring (q_w, XmNhelpLabelString, label2);
	    XtManageChild (XmMessageBoxGetChild (q_w, XmDIALOG_HELP_BUTTON));
	} else
	    XtUnmanageChild (XmMessageBoxGetChild (q_w, XmDIALOG_HELP_BUTTON));

	if (msg)
	    set_xmstring (q_w, XmNmessageString, msg);
	else
	    set_xmstring (q_w, XmNmessageString, "?message?");

	XtManageChild (q_w);
}

static Widget
query_create(tw)
Widget tw;
{
	Widget q_w;
	Arg args[20];
	int n;

	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);  n++;
	XtSetArg(args[n], XmNtitle, "xephem Query");  n++;
	q_w = XmCreateQuestionDialog(tw, "Query", args, n);
	set_something (q_w, XmNcolormap, (XtArgVal)xe_cm);
	XtAddCallback (q_w, XmNokCallback, query_cb, (XtPointer)0);
	XtAddCallback (q_w, XmNcancelCallback, query_cb, (XtPointer)1);
	XtAddCallback (q_w, XmNhelpCallback, query_cb, (XtPointer)2);

	return (q_w);
}

/* called when any button is clicked.
 * w is dialog
 */
/* ARGSUSED */
static void
query_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	void (*f)() = funcs[(long int)client];

	if (f)
	    (*f)();

	XtDestroyWidget (w);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: query.c,v $ $Date: 2009/01/05 20:55:54 $ $Revision: 1.5 $ $Name:  $"};

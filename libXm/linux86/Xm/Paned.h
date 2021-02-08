/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
 * Paned.h - Paned Composite Widget's public header file.
 *
 * Updated and significantly modifided from the Athena VPaned Widget.
 *
 * Date:    March 1, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium
 *          kit@expo.lcs.mit.edu
 */

/*
 *    Copyright 1992, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Tony Auito, Chris D. Peterson
 */

#ifndef _XmPaned_h
#define _XmPaned_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Ext.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define XmPanedAskChild 0

typedef struct _XmPanedClassRec	*XmPanedWidgetClass;
typedef struct _XmPanedRec	*XmPanedWidget;

/*
 * For people used to Motif names this will make things easier.
 */

#define xmPanedWindowWidgetClass xmPanedWidgetClass

/************************************************************
 *
 *  Public Procedures 
 *
 ************************************************************/

/*	Function Name: XmPanedGetPanes
 *	Description: Returns the number of panes in the paned widget.
 *	Arguments: w - the paned widget.
 *                 panes - the list of all panes contained in this widget.
 *                 num - the number of panes.
 *	Returns: the number of panes in the paned widget.
 */

extern int XmPanedGetPanes(
#ifndef _NO_PROTO
    Widget			/* w */,
    WidgetList *		/* panes */,
    int *			/* num */
#endif
);

/*	Function Name: XmCreatePaned
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

Widget XmCreatePaned(
#ifndef _NO_PROTO
Widget, String, ArgList, Cardinal
#endif
);

/* Class record constant */
extern WidgetClass xmPanedWidgetClass;

#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmPaned_h --  DON'T ADD STUFF AFTER THIS #endif */

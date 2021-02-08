/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmOutline_h
#define _XmOutline_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Ext.h>
#include <Xm/Hierarchy.h>

/************************************************************
*	INCLUDE FILES
*************************************************************/

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _XmOutlineClassRec	*XmOutlineWidgetClass;
typedef struct _XmOutlineRec		*XmOutlineWidget;

/************************************************************
*	MACROS
*************************************************************/

/************************************************************
*	GLOBAL FUNCTION DECLARATIONS
*************************************************************/

/*	Function Name: XmCreateOutline
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

Widget XmCreateOutline(
#ifndef _NO_PROTO
Widget, String, ArgList, Cardinal
#endif
);

extern WidgetClass xmOutlineWidgetClass;

#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _Outline_h */

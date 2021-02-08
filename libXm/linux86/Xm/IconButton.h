/*
 *    Copyright 1990, 1992 -- Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmIconButton_h
#define _XmIconButton_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Ext.h>

/************************************************************
*	INCLUDE FILES
*************************************************************/

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _XmIconButtonClassRec	*XmIconButtonWidgetClass;
typedef struct _XmIconButtonRec	        *XmIconButtonWidget;

typedef enum { XmIconTop, XmIconLeft, XmIconRight, XmIconBottom,
	       XmIconOnly, XmIconNone } XmIconPlacement;

/************************************************************
*	MACROS
*************************************************************/

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

typedef struct _XmIconButtonCallbackInfo {
    Boolean state;		/* The current state of the icon button. */
    XEvent * event;		/* The event that caused this action. */
} XmIconButtonCallbackInfo;

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/

/*	Function Name: XmCreateIconButton
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

Widget XmCreateIconButton(
#ifndef _NO_PROTO
Widget, String, ArgList, Cardinal
#endif
);

extern WidgetClass xmIconButtonWidgetClass;

#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmIconButton_h */

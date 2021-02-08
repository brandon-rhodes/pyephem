/*
 *    Copyright 1992, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmFontSelector_h
#define _XmFontSelector_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
 *       INCLUDE FILES
 ************************************************************/

#include <Xm/Ext.h>

#if defined(__cplusplus)
extern "C" {
#endif

/************************************************************
 *       TYPEDEFS AND DEFINES
 ************************************************************/

typedef struct _XmFontSelectorClassRec *XmFontSelectorWidgetClass;
typedef struct _XmFontSelectorRec *XmFontSelectorWidget;

/************************************************************
 *       MACROS
 ************************************************************/

/************************************************************
 *       GLOBAL DECLARATIONS
 ************************************************************/

/************************************************************
 *       EXTERNAL DECLARATIONS
 ************************************************************/

/*	Function Name: XmCreateFontSelector
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

Widget XmCreateFontSelector(
#ifndef _NO_PROTO
Widget, String, ArgList, Cardinal
#endif
);

extern WidgetClass xmFontSelectorWidgetClass;

#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmFontSelector_h - DON'T ADD STUFF AFTER THIS #endif */

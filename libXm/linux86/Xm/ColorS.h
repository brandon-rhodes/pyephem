/*
 *    Copyright 1992, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Mark F. Antonelli, Chris D. Peterson
 *
 */

#ifndef _XmColorSelector_h
#define _XmColorSelector_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Ext.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _XmColorSelectorClassRec *XmColorSelectorWidgetClass;
typedef struct _XmColorSelectorRec *XmColorSelectorWidget;

typedef enum {XmListMode = 0, XmScaleMode = 1} XmColorMode;

/*	Function Name: XmCreateColorSelector
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

Widget XmCreateColorSelector(
#ifndef _NO_PROTO
Widget, String, ArgList, Cardinal
#endif
);

extern WidgetClass       xmColorSelectorWidgetClass;

#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmColorSelect_h DON'T ADD STUFF AFTER THIS #endif */

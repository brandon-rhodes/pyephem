/*
 *    Copyright 1991, 1992 - Integrated Computer Solutions, Inc.
 *
 *                     All Rights Reserved.
 *
 * AUTHOR: Scott Knowlton
 * 
 * Resources:
 *
 * ---------------------------------------------------------------------------
 * Name            Class             Type              InitialValue
 * ---------------------------------------------------------------------------
 * 
 * equalSize       EqualSize         Boolean           False
 * fillOption      FillOption        XmFillOption     XmFillNone
 * marginHeight    Margin            Dimension         0
 * marginWidth     Margin            Dimension         0
 * orientation     Orientation       unsigned char     XmHORIZONTAL
 * 
 * ---------------------------------------------------------------------------
 * 
 */
#ifndef _XmButtonBox_h
#define _XmButtonBox_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Ext.h>

/************************************************************
 *       INCLUDE FILES
 ************************************************************/

/************************************************************
 *       TYPEDEFS AND DEFINES
 ************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _XmButtonBoxClassRec    *XmButtonBoxWidgetClass;
typedef struct _XmButtonBoxRec	       *XmButtonBoxWidget;


/************************************************************
 *       MACROS
 ************************************************************/

/************************************************************
 *       GLOBAL DECLARATIONS
 ************************************************************/

/************************************************************
 *       EXTERNAL DECLARATIONS
 ************************************************************/

/*	Function Name: XmCreateButtonBox
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

Widget XmCreateButtonBox(
#ifndef _NO_PROTO
Widget, String, ArgList, Cardinal
#endif
);

/*
 * Variable argument list functions
 */

extern Widget XmVaCreateButtonBox(
                        Widget parent,
                        char *name,
                        ...);
extern Widget XmVaCreateManagedButtonBox(
                        Widget parent,
                        char *name,
                        ...);

extern WidgetClass xmButtonBoxWidgetClass;

#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmButtonBox_h */

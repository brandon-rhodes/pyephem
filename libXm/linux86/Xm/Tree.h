/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _Tree_h
#define _Tree_h

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

typedef struct _XmTreeClassRec		*XmTreeWidgetClass;
typedef struct _XmTreeRec		*XmTreeWidget;

typedef enum { XmTreeLadder, XmTreeDirect } XmTreeConnectStyle;

typedef enum { XmTreeCompressNone=0, XmTreeCompressLeaves=1, 
               XmTreeCompressAll=2 } XmTreeCompressStyle;

/*	Function Name: XmCreateTree
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

Widget XmCreateTree(
#ifndef _NO_PROTO
Widget, String, ArgList, Cardinal
#endif
);

extern WidgetClass xmTreeWidgetClass;

/************************************************************
*	MACROS
*************************************************************/


/************************************************************
*	GLOBAL FUNCTION DECLARATIONS
*************************************************************/

#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _Tree_h */

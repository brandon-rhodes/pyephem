/*
 *    Copyright 1990, 1992 -- Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmIconBox_h
#define _XmIconBox_h

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

#define XmIconBoxAnyCell (-5)

typedef struct _XmIconBoxClassRec	*XmIconBoxWidgetClass;
typedef struct _XmIconBoxRec		*XmIconBoxWidget;

typedef struct _XmIconBoxDropData {
    Position cell_x, cell_y;	/* drop location in cell coordinates. */
} XmIconBoxDropData;

/************************************************************
*	MACROS
*************************************************************/

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

/*	Function Name: XmIconBoxIsCellEmpty
 *	Description:   Returns true if this cell is unused.
 *	Arguments:     w - the icon box.
 *                     x, y - cell to check.
 *                     ignore - ignore this widget when checking.
 *	Returns:       Returns true if this cell is unused.
 */

Boolean XmIconBoxIsCellEmpty(
#ifndef _NO_PROTO
Widget, Position, Position, Widget
#endif
);

/*	Function Name: XmCreateIconBox
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

Widget XmCreateIconBox(
#ifndef _NO_PROTO
Widget, String, ArgList, Cardinal
#endif
);

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/

extern WidgetClass xmIconBoxWidgetClass; 

#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmIconBox_h */

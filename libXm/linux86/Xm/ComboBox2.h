/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmCominationBox2_h
#define _XmCominationBox2_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
 *	INCLUDE FILES
 ************************************************************/

#include <Xm/Ext.h>

#include <Xm/TextF.h>
#include <Xm/List.h>
#include <Xm/ArrowB.h>
#include <Xm/Label.h>

/************************************************************
 *	TYPEDEFS AND DEFINES
 ************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _XmCombinationBox2ClassRec	*XmCombinationBox2WidgetClass;
typedef struct _XmCombinationBox2Rec		*XmCombinationBox2Widget;

/************************************************************
 *	MACROS
 ************************************************************/

/************************************************************
 *	GLOBAL DECLARATIONS
 ************************************************************/

/*	Function Name: XmCombinationBox2GetValue
 *	Description:   Retreives the value from the combo box.
 *	Arguments:     w - the combination box.
 *	Returns:       The value in the text widget.
 */

String XmCombinationBox2GetValue(
#ifndef _NO_PROTO
Widget				/* Combination box widget. */
#endif
);

/*	Function Name: XmCreateCombinationBox2
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

Widget XmCreateCombinationBox2(
#ifndef _NO_PROTO
Widget, String, ArgList, Cardinal
#endif
);

/*      Function Name:  XmCombinationBox2GetLabel
 *      Description:    Returns the "label" child of the XmCombinationBox2
 *      Arguments:      w - The XmCombinationBox2 Widget
 *      Returns:        The specified child of the XmCombinationBox2
 */

Widget XmCombinationBox2GetLabel(
#ifndef _NO_PROTO
Widget
#endif
);

/*      Function Name:  XmCombinationBox2GetArrow
 *      Description:    Returns the "arrow" child of the XmCombinationBox2
 *      Arguments:      w - The XmCombinationBox2 Widget
 *      Returns:        The specified child of the XmCombinationBox2
 */

Widget XmCombinationBox2GetArrow(
#ifndef _NO_PROTO
Widget
#endif
);

/*      Function Name:  XmCombinationBox2GetText
 *      Description:    Returns the "text" child of the XmCombinationBox2
 *      Arguments:      w - The XmCombinationBox2 Widget
 *      Returns:        The specified child of the XmCombinationBox2
 */

Widget XmCombinationBox2GetText(
#ifndef _NO_PROTO
Widget
#endif
);

/*      Function Name:  XmCombinationBox2GetList
 *      Description:    Returns the "list" child of the XmCombinationBox2
 *      Arguments:      w - The XmCombinationBox2 Widget
 *      Returns:        The specified child of the XmCombinationBox2
 */

Widget XmCombinationBox2GetList(
#ifndef _NO_PROTO
Widget
#endif
);


/************************************************************
 *	EXTERNAL DECLARATIONS
 ************************************************************/

extern WidgetClass xmCombinationBox2WidgetClass;

#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmCombinationBox2_h */

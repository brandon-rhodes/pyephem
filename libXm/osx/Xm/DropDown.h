#ifndef _XmDropDown_h
#define _XmDropDown_h

#include <X11/Intrinsic.h>

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif
    
externalref WidgetClass xmDropDownWidgetClass;

typedef struct _XmDropDownClassRec	*XmDropDownWidgetClass;
typedef struct _XmDropDownRec		*XmDropDownWidget;

/* XmIsDropDown may already be defined for Fast Subclassing */
#ifndef XmIsDropDown
#define XmIsDropDown(w) XtIsSubclass(w, xmDropDownWidgetClass)
#endif /* XmIsDropDown */

/***** Public Function Declarations *****/

/*	Function Name: XmCreateDropDown
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

extern Widget XmCreateDropDown(Widget parent,
                                char *name,
                                ArgList args,
                                Cardinal argCount);

/*	Function Name: XmDropDownGetValue
 *	Description:   Retreives the value from the combo box.
 *	Arguments:     w - the combination box.
 *	Returns:       The value in the text widget.
 */

extern String XmDropDownGetValue(Widget w);

/*
 * Variable argument list functions
 */

extern Widget XmVaCreateDropDown(
                        Widget parent,
                        char *name,
                        ...);
extern Widget XmVaCreateManagedDropDown(
                        Widget parent,
                        char *name,
                        ...);

/*      Function Name:  XmDropDownGetLabel
 *      Description:    Returns the "label" child of the XmDropDown
 *      Arguments:      w - The XmCombinationBox2 Widget
 *      Returns:        The specified child of the XmDropDown
 */

extern Widget XmDropDownGetLabel(Widget w);

/*      Function Name:  XmDropDownGetArrow
 *      Description:    Returns the "arrow" child of the XmDropDown
 *      Arguments:      w - The XmCombinationBox2 Widget
 *      Returns:        The specified child of the XmDropDown
 */

extern Widget XmDropDownGetArrow(Widget w);

/*      Function Name:  XmDropDownGetText
 *      Description:    Returns the "text" child of the XmDropDown
 *      Arguments:      w - The XmCombinationBox2 Widget
 *      Returns:        The specified child of the XmDropDown
 */

extern Widget XmDropDownGetText(Widget w);

/*      Function Name:  XmDropDownGetList
 *      Description:    Returns the "list" child of the XmDropDown
 *      Arguments:      w - The XmCombinationBox2 Widget
 *      Returns:        The specified child of the XmDropDown
 */

extern Widget XmDropDownGetList(Widget w);

/*      Function Name:  XmDropDownGetChild
 *      Description:    Returns the child widget id of the XmDropDown
 *      Arguments:      w - The XmDropDown widget
                        child - Teh component within the DropDown
 *      Returns:        The specified child of the XmDropDown
 */
extern Widget XmDropDownGetChild(Widget w, int child);

#if defined(__cplusplus)
}
#endif

#endif /* _XmDropDown_h_ */

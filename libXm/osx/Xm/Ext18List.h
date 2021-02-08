#ifndef _XmExt18List_h_
#define _XmExt18List_h_

#if defined(__cplusplus)
extern "C" {
#endif

#include <Xm/MultiList.h>
#include <Xm/XmP.h>
    
externalref WidgetClass xmExt18ListWidgetClass XM_DEPRECATED;

enum {
    XmEXT18LIST_FOUND = XmMULTILIST_FOUND,
    XmEXT18LIST_NOT_FOUND = XmMULTILIST_NOT_FOUND
};

typedef XmMultiListWidgetClass	*XmExt18ListWidgetClass;
typedef XmMultiListWidget	*XmExt18ListWidget;

typedef XmMultiListRowInfo       Xm18RowInfo;

typedef XmMultiListCallbackStruct   XmExt18ListCallbackStruct;

/* 
 * Function Name: XmExt18ListGetSelectedRows
 * Description:   Takes an Extended List and returns a NULL terminated array
 *                of pointers to selected rows from the internal list
 * Arguments:     w - the extended list widget
 * Returns:       Xm18RowInfo **
 */

Xm18RowInfo ** XmExt18ListGetSelectedRows(Widget w) XM_DEPRECATED;

/*	Function Name: XmCreateExtended18List
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

extern Widget XmCreateExtended18List(Widget,
                                     String,
                                     ArgList,
                                     Cardinal) XM_DEPRECATED;

extern Widget XmCreateExt18List(Widget,
                                String,
                                ArgList,
                                Cardinal) XM_DEPRECATED;

extern Widget XmVaCreateExt18List(
                        Widget parent,
                        char *name,
                        ...) XM_DEPRECATED;

extern Widget XmVaCreateManagedExt18List(
                        Widget parent,
                        char *name,
                        ...) XM_DEPRECATED;

/*  Function Name: XmExt18ListUnselectAllItems
 *  Description:   Unselects all rows
 *  Arguments:     w - the ilist widget.
 *  Returns:       none
 */
extern void XmExt18ListUnselectAllItems(Widget w) XM_DEPRECATED;

/*  Function Name: XmExt18ListUnselectItem
 *  Description:   Unselects the row passed in
 *  Arguments:     w - the ilist widget.
 *                 row_info - ptr to the row passed in
 *  Returns:       none
 */
extern void XmExt18ListUnselectItem(Widget, Xm18RowInfo *) XM_DEPRECATED;

/*  Function Name: XmExt18ListToggleRow
 *  Description:   Toggles the selection state of a specified row
 *  Arguments:     w - the extended list widget
 *  Returns:       none
 */
extern void XmExt18ListToggleRow(Widget, short) XM_DEPRECATED;

/*  Function Name: XmExt18ListSelectItems
 *  Description:   Set selection state by matching column entries to XmString
 *  Arguments:     w - the extended list widget
 *		   item - XmString to use as selection key
 *		   column - column number (0 - N) to match (or XmANY_COLUMN)
 *		   notify - if True, call XmNsingleSelectionCallback
 *  Returns:       none
 */
extern void XmExt18ListSelectItems(Widget, XmString, int, Boolean) XM_DEPRECATED;

/*  Function Name: XmExt18ListDeselectItems
 *  Description:   Set selection state by matching column entries to XmString
 *  Arguments:     w - the extended list widget
 *		   item - XmString to use as selection key
 *		   column - column number (0 - N) to match (or XmANY_COLUMN)
 *  Returns:       none
 */
extern void XmExt18ListDeselectItems(Widget, XmString, int) XM_DEPRECATED;

/*  Function Name: XmExt18ListSelectAllItems
 *  Description:   Set selection state on all rows
 *  Arguments:     w - the extended list widget
 *		   notify - if True, call XmNsingleSelectionCallback for each
 *  Returns:       none
 */
extern void XmExt18ListSelectAllItems(Widget, Boolean) XM_DEPRECATED;

/*  Function Name: XmExt18ListSelectRow
 *  Description:   Set selection state on all rows
 *  Arguments:     w - the extended list widget
 *		   row - the row to select
 *		   notify - if True, call XmNsingleSelectionCallback
 *  Returns:       none
 */
extern void XmExt18ListSelectRow(Widget, int, Boolean) XM_DEPRECATED;

/*  Function Name: XmExt18ListDeselectRow
 *  Description:   Set selection state on all rows
 *  Arguments:     w - the extended list widget
 *		   row - the row to select
 *  Returns:       none
 */
extern void XmExt18ListDeselectRow(Widget, int) XM_DEPRECATED;

/* 
 * Function Name: XmExt18ListGetSelectedRowArray
 * Description:   Takes an Extended List and returns a NULL terminated array
 *                of pointers to selected rows from the internal list
 * Arguments:     w - the extended list widget
 *		  num_rows - pointer to the number of rows
 * Returns:       array of integer (selected) row numbers
 */
extern int *XmExt18ListGetSelectedRowArray(Widget, int *) XM_DEPRECATED;

/*  Function Name: XmExt18ListMakeRowVisible
 *  Description:   Shifts the visible extended list rows as desired
 *  Arguments:     w - the extended list widget
 *		   row - the row number wished to be made visible
 *  Returns:       none
 */
void XmExt18ListMakeRowVisible(Widget, int) XM_DEPRECATED;

#if defined(__cplusplus)
}
#endif

#endif /* _XmExt18List_h_ */

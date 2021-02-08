#ifndef _XmMultiList_h_
#define _XmMultiList_h_

#include <Xm/Ext.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define XmANY_COLUMN	-1

externalref WidgetClass xmMultiListWidgetClass;

typedef struct _XmMultiListClassRec	*XmMultiListWidgetClass;
typedef struct _XmMultiListRec		*XmMultiListWidget;

typedef struct _XmMultiListRowInfo {
    /*
     * Used by the XmIList widget.
     */
    XmString * values;		/* The array of column strings */
    Pixmap pixmap;		/* the mini-icon pixmaps. */
    Boolean selected;		/* Is this row selected. */

    /*
     * Provided for the convience of the application programmer.
     */
    short *sort_id;
    XtPointer data;

    /*
     * Private to the XmIList widget (do not modify these).
     */
    short pix_width;		/* width of the pixmap. */
    short pix_height;		/* height of the pixmap. */
    short height;		/* height of the row */
    Boolean old_sel_state;	/* previous select state. */
    short pix_depth;		/* height of the pixmap. */
} XmMultiListRowInfo;

typedef struct _XmMultiListCallbackStruct {
    int reason;		/* Why was callback called? */
    XEvent *event;	/* The X Event associated with find button press... */
    String string;	/* The search string used to do find */
    int column;		/* The column index into row values */
    XmMultiListRowInfo *row; /* The row info structure of the matching row */
    wchar_t *wc_string; /* The search wcs string used to do find */
} XmMultiListCallbackStruct;

typedef int (*Xm18SortFunction) \
            (short, const XmMultiListRowInfo *, const XmMultiListRowInfo *);

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

/* 
 * Function Name: XmMultiListGetSelectedRows
 * Description:   Takes an Extended List and returns a NULL terminated array
 *                of pointers to selected rows from the internal list
 * Arguments:     w - the extended list widget
 * Returns:       XmMultiListRowInfo **
 */

XmMultiListRowInfo ** XmMultiListGetSelectedRows(Widget w);

/*	Function Name: XmCreateExtended18List
 *	Description: Creation Routine for UIL and ADA.
 *	Arguments: parent - the parent widget.
 *                 name - the name of the widget.
 *                 args, num_args - the number and list of args.
 *	Returns: The Widget created.
 */

extern Widget XmCreateMultiList(Widget w,
                              char *name,
                              ArgList args,
                              Cardinal num_args);

extern Widget XmCreateMultiList(Widget w,
                         char *name,
                         ArgList args,
                         Cardinal num_args);

extern Widget XmVaCreateMultiList(
                        Widget parent,
                        char *name,
                        ...);

extern Widget XmVaCreateManagedMultiList(
                        Widget parent,
                        char *name,
                        ...);

/*  Function Name: XmMultiListUnselectAllItems
 *  Description:   Unselects all rows
 *  Arguments:     w - the ilist widget.
 *  Returns:       none
 */
extern void XmMultiListUnselectAllItems(Widget w);

/*  Function Name: XmMultiListUnselectItem
 *  Description:   Unselects the row passed in
 *  Arguments:     w - the ilist widget.
 *                 row_info - ptr to the row passed in
 *  Returns:       none
 */
extern void XmMultiListUnselectItem(Widget w, XmMultiListRowInfo *row_info);

/*  Function Name: XmMultiListToggleRow
 *  Description:   Toggles the selection state of a specified row
 *  Arguments:     w - the extended list widget
 *  Returns:       none
 */
extern void XmMultiListToggleRow(Widget w, short row);

/*  Function Name: XmMultiListSelectItems
 *  Description:   Set selection state by matching column entries to XmString
 *  Arguments:     w - the extended list widget
 *		   item - XmString to use as selection key
 *		   column - column number (0 - N) to match (or XmANY_COLUMN)
 *		   notify - if True, call XmNsingleSelectionCallback
 *  Returns:       none
 */
extern void
XmMultiListSelectItems(Widget w,
                       XmString item,
                       int column,
                       Boolean notify);

/*  Function Name: XmMultiListDeselectItems
 *  Description:   Set selection state by matching column entries to XmString
 *  Arguments:     w - the extended list widget
 *		   item - XmString to use as selection key
 *		   column - column number (0 - N) to match (or XmANY_COLUMN)
 *  Returns:       none
 */
extern void XmMultiListDeselectItems(Widget w,
                                     XmString item,
                                     int column);

/*  Function Name: XmMultiListSelectAllItems
 *  Description:   Set selection state on all rows
 *  Arguments:     w - the extended list widget
 *		   notify - if True, call XmNsingleSelectionCallback for each
 *  Returns:       none
 */
extern void XmMultiListSelectAllItems(Widget w, Boolean notify);

/*  Function Name: XmMultiListSelectRow
 *  Description:   Set selection state on all rows
 *  Arguments:     w - the extended list widget
 *		   row - the row to select
 *		   notify - if True, call XmNsingleSelectionCallback
 *  Returns:       none
 */
extern void XmMultiListSelectRow(Widget w, int row, Boolean notify);

/*  Function Name: XmMultiListDeselectRow
 *  Description:   Set selection state on all rows
 *  Arguments:     w - the extended list widget
 *		   row - the row to select
 *  Returns:       none
 */
extern void XmMultiListDeselectRow(Widget w, int row);

/* 
 * Function Name: XmMultiListGetSelectedRowArray
 * Description:   Takes an Extended List and returns a NULL terminated array
 *                of pointers to selected rows from the internal list
 * Arguments:     w - the extended list widget
 *		  num_rows - pointer to the number of rows
 * Returns:       array of integer (selected) row numbers
 */
extern int *XmMultiListGetSelectedRowArray(Widget w, int *num_rows);

/*  Function Name: XmMultiListMakeRowVisible
 *  Description:   Shifts the visible extended list rows as desired
 *  Arguments:     w - the extended list widget
 *		   row - the row number wished to be made visible
 *  Returns:       none
 */
extern void XmMultiListMakeRowVisible(Widget w, int row);

#if defined(__cplusplus)
}
#endif

#endif /* _XmMultiList_h_ */

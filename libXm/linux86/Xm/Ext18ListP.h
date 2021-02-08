/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmExt18ListP_h
#define _XmExt18ListP_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
*	INCLUDE FILES
*************************************************************/
#include <Xm/ManagerP.h>
#include <Xm/PrimitiveP.h>

#include <Xm/Ext18List.h>

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * IList Stuff.
 */

#define XmExt18List_DEFAULT_VISIBLE_COUNT	5	/* XmNvisibleItemCount */

/************************************************************
*	MACROS
*************************************************************/

#define XmExt18ListIndex (XmManagerIndex + 1)
#define XmI18ListIndex (XmPrimitiveIndex + 1)

extern XmOffsetPtr XmExt18List_offsets;
extern XmOffsetPtr XmExt18ListC_offsets;

extern XmOffsetPtr XmI18List_offsets;

#define I18field(w,f,t) XmField(w, XmI18List_offsets, XmI18List, f, t)
#define XmI18List_selection_policy(w) I18field(w, selection_policy, unsigned char)
#define XmI18List_num_columns(w) I18field(w, num_columns, short)
#define XmI18List_column_titles(w) I18field(w, column_titles, XmString*)
#define XmI18List_num_rows(w) I18field(w, num_rows, short)
#define XmI18List_row_data(w) I18field(w, row_data, Xm18RowInfo*)
#define XmI18List_first_col_pixmaps(w) I18field(w, first_col_pixmaps, Boolean)
#define XmI18List_font_list(w) I18field(w, font_list, XmFontList)
#define XmI18List_v_bar(w) I18field(w, v_bar, Widget)
#define XmI18List_h_bar(w) I18field(w, h_bar, Widget)
#define XmI18List_first_row(w) I18field(w, first_row, short)
#define XmI18List_first_col(w) I18field(w, first_col, short)
#define XmI18List_double_click(w) I18field(w, double_click, XtCallbackList)
#define XmI18List_single_select(w) I18field(w, single_select, XtCallbackList)
#define XmI18List_selected_header(w) I18field(w, selected_header, short)
#define XmI18List_sort_functions(w) I18field(w, sort_functions, Xm18SortFunction*)
#define XmI18List_string_direction(w) I18field(w, string_direction, unsigned char)
#define XmI18List_alignment(w) I18field(w, alignment, unsigned char)
#define XmI18List_column_widths(w) I18field(w, column_widths, short*)
#define XmI18List_end(w) I18field(w, end, short)
#define XmI18List_anchor(w) I18field(w, anchor, short)
#define XmI18List_sep_y(w) I18field(w, sep_y, int)
#define XmI18List_title_row_height(w) I18field(w, title_row_height, short)
#define XmI18List_row_height(w) I18field(w, row_height, short)
#define XmI18List_gc(w) I18field(w, gc, GC)
#define XmI18List_rev_gc(w) I18field(w, rev_gc, GC)
#define XmI18List_stippled_gc(w) I18field(w, stippled_gc, GC)
#define XmI18List_stippled_rev_gc(w) I18field(w, stippled_rev_gc, GC)
#define XmI18List_inv_gc(w) I18field(w, inv_gc, GC)
#define XmI18List_state(w) I18field(w, state, unsigned short)
#define XmI18List_timeout(w) I18field(w, timeout, XtIntervalId)
#define XmI18List_working_row(w) I18field(w, working_row, short)
#define XmI18List_working_col(w) I18field(w, working_col, short )
#define XmI18List_time(w) I18field(w, time, Time)
#define XmI18List_left_loc(w) I18field(w, left_loc, int)
#define XmI18List_search_column(w) I18field(w, search_column, short)
#define XmI18List_visible_rows(w) I18field(w, visible_rows, int)
#define XmI18List_new_visual_style(w) I18field(w, new_visual_style, Boolean)
#define XmI18List_entry_background_pixel(w) I18field(w, entry_background_pixel, Pixel)
#define XmI18List_entry_background_use(w) I18field(w, entry_background_use, Boolean)
#define XmI18List_entry_background_gc(w) I18field(w, entry_background_gc, GC)
#define XmI18List_entry_background_fill_gc(w) I18field(w, entry_background_fill_gc, GC)
#define XmI18List_entry_background_rev_gc(w) I18field(w, entry_background_rev_gc, GC)
#define XmI18List_entry_background_stippled_gc(w) I18field(w, entry_background_stippled_gc, GC)
#define XmI18List_entry_background_stippled_rev_gc(w) I18field(w, entry_background_stippled_rev_gc, GC)
#define XmI18List_entry_background_inv_gc(w) I18field(w, entry_background_inv_gc, GC)

#define E18field(w,f,t) XmField(w, XmExt18List_offsets, XmExt18List, f, t)
#define XmExt18List_title(w) E18field(w, title, XmString)
#define XmExt18List_find_label(w) E18field(w, find_label, XmString)
#define XmExt18List_double_click(w) E18field(w, double_click, XtCallbackList)
#define XmExt18List_single_select(w) E18field(w, single_select, XtCallbackList)
#define XmExt18List_show_find(w) E18field(w, show_find, Boolean)
#define XmExt18List_title_wid(w) E18field(w, title_wid, Widget)
#define XmExt18List_frame(w) E18field(w, frame, Widget)
#define XmExt18List_ilist(w) E18field(w, ilist, Widget)
#define XmExt18List_v_bar(w) E18field(w, v_bar, Widget)
#define XmExt18List_h_bar(w) E18field(w, h_bar, Widget)
#define XmExt18List_find(w) E18field(w, find, Widget)
#define XmExt18List_find_text(w) E18field(w, find_text, Widget)
#define XmExt18List_last_search(w) E18field(w, last_search, String)
#define XmExt18List_item_found(w) E18field(w, item_found, XtCallbackList)
#define XmExt18List_not_found(w) E18field(w, not_found, XtCallbackList)
#define XmExt18List_visible_rows(w) E18field(w, visible_rows, int)
#define XmExt18List_title_string(w) E18field(w, title_string, XmString)

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

/*
 * IList widget definitions.
 */

typedef struct _I18ListClassPart {
    XtPointer extension;	/* Just in case we need it later. */
} I18ListClassPart;

typedef struct _XmI18ListClassRec {
    CoreClassPart		core_class;
    XmPrimitiveClassPart	primitive;
    I18ListClassPart		ilist_class;
} XmI18ListClassRec;

typedef struct _XmI18ListPart {
    /*
     * Resources
     */

    unsigned char selection_policy; /* selection mode - kat 12-28-90 */
    short num_columns;		/* number of columns in the list. */
    XmString * column_titles;	/* title for each column. */
    short num_rows;		/* number of rows in the list. */
    Xm18RowInfo * row_data;	/* Data to put into each column. */
    Boolean first_col_pixmaps;	/* Should we put mini_icons in the first
				   column of each entry? */		
    XmFontList font_list;	/* This widget's font list. */

    Widget v_bar, h_bar;	/* Scrollbars that may be used
				   to scroll this widget. */

    short first_row;		/* which row is at the top of the display. */
    short first_col;		/* which column is at the far left. */

    XtCallbackList double_click; /* The double click callback list. */
    XtCallbackList single_select; /*The single click callback list. */

    short selected_header;	/* The currently selected header. */

    Xm18SortFunction *sort_functions; /* The client supplied sort functions */

    unsigned char string_direction;
    unsigned char alignment;

    /* 
     * Private State
     */

    short * column_widths;	/* Width of each column. */
    short end;			/* The non-anchor end point. */ 
    short anchor;		/* The anchor point for the extended 
				   selection. */

    int sep_y;			/*location of the top of the separator line.*/
    
    short title_row_height;	/* height of title row */
    short row_height;	/* height of all other data rows */

    GC gc;			/* The graphics context for normal text. */
    GC rev_gc;			/* The graphics context for inverted text. */
    GC stippled_gc;		/* The graphics context for normal text. */
    GC stippled_rev_gc;		/* The graphics context for inverted text. */
    GC inv_gc;			/* The graphics context for inverting areas. */

    unsigned short state;	/* The state of this widget. */
    XtIntervalId timeout;	/* The mulit - click timout. */

    short working_row, working_col; /* A Working row and column. */
    Time time;			/*The server time of the last button click. */

    int left_loc;		/* left margin in pixels. */

    short search_column;	/* added for I18List Find support */

    int visible_rows;		/* Visible item (row) count */

    Boolean new_visual_style;

    Pixel entry_background_pixel;
    Boolean entry_background_use;
    GC entry_background_gc;			
    GC entry_background_fill_gc;			
    GC entry_background_stippled_gc;		
    GC entry_background_stippled_rev_gc;	
    GC entry_background_inv_gc;			
    GC entry_background_rev_gc;			

} XmI18ListPart;

typedef struct _XmI18ListRec {
    CorePart		core;
    XmPrimitivePart	primitive;
    XmI18ListPart	ilist;
} XmI18ListRec;

/*
 * Extended List.
 */

typedef struct {
    XtPointer extension;	/* Just in case we need it later. */
} XmExt18ListClassPart;

typedef struct _XmExt18ListClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    XmExt18ListClassPart		ext_list_class;
} XmExt18ListClassRec;

typedef struct {
    /* resources */

    XmString title;		/* Title for the list (backwards compatible) */
    XmString find_label;	/* label for Find button */

    XtCallbackList double_click; /* The double click callbacks. */
    XtCallbackList single_select; /* The single click callbacks. -kat */

	Boolean show_find;    /* whether to display the Find button and textF */

    /* private state */

    Widget title_wid;		/* The list title widget. */
    Widget frame;		/* Frame to display list into. */
    Widget ilist;		/* The internal list widget. */
    Widget v_bar, h_bar;	/* The scrollbars. */
    Widget find, find_text;	/* Widgets used for a find. */

    String last_search;

    XtCallbackList item_found;	/* Called when find succeeds */
    XtCallbackList not_found;	/* Called when find doesn't succeed */

    int visible_rows;		/* visible items (mirrored in XmI18ListPart) */
    XmString title_string;	/* (preferred use) Title for the list */

} XmExt18ListPart;

typedef struct _XmExt18ListRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    XmExt18ListPart	ext_list;
} XmExt18ListRec;

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/

Xm18RowInfo ** XmI18ListGetSelectedRows(
#ifndef _NO_PROTO
Widget 
#endif
);

void
Xm18IListUnselectItem( 
#ifndef _NO_PROTO
Widget , Xm18RowInfo *
#endif
);

void
Xm18IListUnselectAllItems( 
#ifndef _NO_PROTO
Widget
#endif
);

void
XmI18ListToggleRow(
#ifndef _NO_PROTO
Widget , short
#endif
);

typedef struct _XmI18ListClassRec	*XmI18ListWidgetClass;
typedef struct _XmI18ListRec	*XmI18ListWidget;

extern XmExt18ListClassRec xmExt18ListClassRec;

extern XmI18ListClassRec xiI18ListClassRec;
extern WidgetClass xmI18ListWidgetClass; 

#if defined(__cplusplus)
}
#endif

/************************************************************
*	STATIC DECLARATIONS
*************************************************************/

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmExt18ListP_h */

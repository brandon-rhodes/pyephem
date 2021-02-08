/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1995 Integrated Computer Solutions
 */

#ifndef __xm_tablep_h__
#define __xm_tablep_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/Table.h>

#ifndef __SCROLL_MANAGER__
#define __SCROLL_MANAGER__
typedef struct _SmScrollNode {
    struct _SmScrollNode *next;
    struct _SmScrollNode *prev;
    int                  x;
    int                  y;
} SmScrollNodeRec, *SmScrollNode;

typedef struct _SmScrollMgr {
    int          offset_x;
    int          offset_y;
    int          scroll_count;
    SmScrollNode scroll_queue;
    Boolean      scrolling;
} SmScrollMgrRec, *SmScrollMgr;
#endif /* __SCROLL_MANAGER__ */

typedef struct _XmTableDefaultProcRec {
    String       widget_class_name;
    WidgetClass  widget_class;
    XmWidgetFunc focus_widget_func;
    XtWidgetProc select_text_proc;
    XtWidgetProc unselect_text_proc;
    XtStringProc set_value_proc;
    XmStringFunc get_value_func;
    XmFreeProc   free_proc;
    XmRenderProc render_proc;
} XmTableDefaultProcRec, * XmTableDefaultProcs;

typedef struct _XmTableClassPart {
    String      	 traversal_translations;
    String		 edit_translations;
    XmTableDefaultProcs  default_procs;
    Cardinal		 num_default_procs;
    XtPointer            extension;
} XmTableClassPart;

typedef struct _XmTableClassRec {
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    XmTableClassPart    table_class;
} XmTableClassRec;

typedef struct _XmTableConstraintPart {
    /* Public */
    
    XmString     column_title;
    Boolean	 resizable;

    XmWidgetFunc focus_widget_func;
    XtWidgetProc select_text_proc;
    XtWidgetProc unselect_text_proc;
    XtStringProc set_value_proc;
    XmStringFunc get_value_func;
    XmFreeProc   free_proc;
    XmRenderProc render_proc;

    /* Private */

    Boolean	   translations_inited;
    XtTranslations edit_translations;
    XtTranslations traversal_translations;

    XmString       _column_title;

    int            column_index;
    int		   offset;
	
    Widget	   label;

    /* new public */
    Boolean      editable;
} XmTableConstraintPart, * XmTableConstraint;

typedef struct _XmTableConstraintRec {
    XmManagerConstraintPart manager;
    XmTableConstraintPart   table;
} XmTableConstraintRec, * XmTableConstraintPtr;

typedef struct _XmTablePart {
    
    /* Public */
    
    Cardinal   num_rows;
    Boolean    resize_width;
    Boolean    resize_height;

    XmFontList font_list;
    
    Dimension  margin_width;
    Dimension  margin_height;
    Dimension  spacing;
    Dimension  line_width;

    Boolean       show_titles;
    Boolean	  set_child_color;
    unsigned char title_alignment;
    XmFontList    title_font_list;

    Pixel      title_foreground;
    Pixel      title_background;
    Pixmap     title_background_pixmap;
    Pixel      title_top_shadow_color;
    Pixmap     title_top_shadow_pixmap;
    Pixel      title_bottom_shadow_color;
    Pixmap     title_bottom_shadow_pixmap;
    Pixel      title_shade_color;
    Pixmap     title_shade_pixmap;
    Pixel      table_color;
    Pixmap     table_pixmap;
    Pixel      shade_color;
    Pixmap     shade_pixmap;
    Pixel      line_color;
    Pixmap     line_pixmap;

    XmFetchCellValueFunc fetch_cell_value_func;
    XmStoreCellValueProc store_cell_value_proc;
    XmFreeCellValueProc  free_cell_value_proc;
    
    XtCallbackList cell_traverse_verify_callback;
    XtCallbackList cell_traverse_callback;

    /* Private */
    
    Boolean    inited;

    SmScrollMgr scroll_mgr;

    int	       y_origin;
    int	       x_origin;

    int        cell_x;
    int        cell_y;
    Widget     active_cell;

    int	       move_status;
    int	       move_start_x;
    int	       move_cur_x;
    int	       half_line_width;
    Widget     move_widget;

    Cursor     move_cursor;

    XRectangle save_size;
    Dimension  save_shadow_thickness;

    int	       title_height;
    int	       table_height;
    int        table_width;
    int        row_height;
    int        valid_cnt;

    int	       *offset;
    int	       num_offsets;

    Widget     vscroll;
    Widget     hscroll;
    Widget     clip;
    Widget     title_clip;
    Widget     title_backing;

    GC foreground_GC;
    GC title_top_shadow_GC;
    GC title_bottom_shadow_GC;
    GC table_GC;
    GC line_GC;
    GC shade_GC;
    GC move_GC;
} XmTablePart;

typedef struct _XmTableRec {
    CorePart       core;
    CompositePart  composite;
    ConstraintPart constraint;
    XmManagerPart  manager;
    XmTablePart    table;
} XmTableRec;

#define XmProcTableAppend \
    { "__APPEND__", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }


#define XmTableIndex (XmManagerIndex + 1)

extern XmOffsetPtr XmTable_offsets;
extern XmOffsetPtr XmTableC_offsets;

#define XmTableCField(w,f,t) XmConstraintField(w, XmTableC_offsets, XmTable, f, t)
#define XmTableC_column_title(w) XmTableCField(w, column_title, XmString)
#define XmTableC_resizable(w) XmTableCField(w, resizable, Boolean)
#define XmTableC_editable(w) XmTableCField(w, editable, Boolean)
#define XmTableC_focus_widget_func(w) XmTableCField(w, focus_widget_func, XmWidgetFunc)
#define XmTableC_select_text_proc(w) XmTableCField(w, select_text_proc, XtWidgetProc)
#define XmTableC_unselect_text_proc(w) XmTableCField(w, unselect_text_proc, XtWidgetProc)
#define XmTableC_set_value_proc(w) XmTableCField(w, set_value_proc, XtStringProc)
#define XmTableC_get_value_func(w) XmTableCField(w, get_value_func, XmStringFunc)
#define XmTableC_free_proc(w) XmTableCField(w, free_proc, XmFreeProc)
#define XmTableC_render_proc(w) XmTableCField(w, render_proc, XmRenderProc)
#define XmTableC_translations_inited(w) XmTableCField(w, translations_inited, Boolean)
#define XmTableC_edit_translations(w) XmTableCField(w, edit_translations, XtTranslations)
#define XmTableC_traversal_translations(w) XmTableCField(w, traversal_translations, XtTranslations)
#define XmTableC__column_title(w) XmTableCField(w, _column_title, XmString)
#define XmTableC_column_index(w) XmTableCField(w, column_index, int)
#define XmTableC_offset(w) XmTableCField(w, offset, int)
#define XmTableC_label(w) XmTableCField(w, label, Widget)

#define XmTableField(w,f,t) XmField(w, XmTable_offsets, XmTable, f, t)
#define XmTable_num_rows(w) XmTableField(w, num_rows, Cardinal)
#define XmTable_resize_width(w) XmTableField(w, resize_width, Boolean)
#define XmTable_resize_height(w) XmTableField(w, resize_height, Boolean)
#define XmTable_font_list(w) XmTableField(w, font_list, XmFontList)
#define XmTable_margin_width(w) XmTableField(w, margin_width, Dimension)
#define XmTable_margin_height(w) XmTableField(w, margin_height, Dimension)
#define XmTable_spacing(w) XmTableField(w, spacing, Dimension)
#define XmTable_line_width(w) XmTableField(w, line_width, Dimension)
#define XmTable_show_titles(w) XmTableField(w, show_titles, Boolean)
#define XmTable_set_child_color(w) XmTableField(w, set_child_color, Boolean)
#define XmTable_title_alignment(w) XmTableField(w, title_alignment, unsigned char)
#define XmTable_title_font_list(w) XmTableField(w, title_font_list, XmFontList)
#define XmTable_title_foreground(w) XmTableField(w, title_foreground, Pixel)
#define XmTable_title_background(w) XmTableField(w, title_background, Pixel)
#define XmTable_title_background_pixmap(w) XmTableField(w, title_background_pixmap, Pixmap)
#define XmTable_title_top_shadow_color(w) XmTableField(w, title_top_shadow_color, Pixel)
#define XmTable_title_top_shadow_pixmap(w) XmTableField(w, title_top_shadow_pixmap, Pixmap)
#define XmTable_title_bottom_shadow_color(w) XmTableField(w, title_bottom_shadow_color, Pixel)
#define XmTable_title_bottom_shadow_pixmap(w) XmTableField(w, title_bottom_shadow_pixmap, Pixmap)
#define XmTable_title_shade_color(w) XmTableField(w, title_shade_color, Pixel)
#define XmTable_title_shade_pixmap(w) XmTableField(w, title_shade_pixmap, Pixmap)
#define XmTable_table_color(w) XmTableField(w, table_color, Pixel)
#define XmTable_table_pixmap(w) XmTableField(w, table_pixmap, Pixmap)
#define XmTable_shade_color(w) XmTableField(w, shade_color, Pixel)
#define XmTable_shade_pixmap(w) XmTableField(w, shade_pixmap, Pixmap)
#define XmTable_line_color(w) XmTableField(w, line_color, Pixel)
#define XmTable_line_pixmap(w) XmTableField(w, line_pixmap, Pixmap)
#define XmTable_fetch_cell_value_func(w) XmTableField(w, fetch_cell_value_func, XmFetchCellValueFunc)
#define XmTable_store_cell_value_proc(w) XmTableField(w, store_cell_value_proc, XmStoreCellValueProc)
#define XmTable_free_cell_value_proc(w) XmTableField(w, free_cell_value_proc, XmFreeCellValueProc)
#define XmTable_cell_traverse_verify_callback(w) XmTableField(w, cell_traverse_verify_callback, XtCallbackList)
#define XmTable_cell_traverse_callback(w) XmTableField(w, cell_traverse_callback, XtCallbackList)
#define XmTable_inited(w) XmTableField(w, inited, Boolean)
#define XmTable_scroll_mgr(w) XmTableField(w, scroll_mgr, SmScrollMgr)
#define XmTable_y_origin(w) XmTableField(w, y_origin, int)
#define XmTable_x_origin(w) XmTableField(w, x_origin, int)
#define XmTable_cell_x(w) XmTableField(w, cell_x, int)
#define XmTable_cell_y(w) XmTableField(w, cell_y, int)
#define XmTable_active_cell(w) XmTableField(w, active_cell, Widget)
#define XmTable_move_status(w) XmTableField(w, move_status, int)
#define XmTable_move_start_x(w) XmTableField(w, move_start_x, int)
#define XmTable_move_cur_x(w) XmTableField(w, move_cur_x, int)
#define XmTable_half_line_width(w) XmTableField(w, half_line_width, int)
#define XmTable_move_widget(w) XmTableField(w, move_widget, Widget)
#define XmTable_move_cursor(w) XmTableField(w, move_cursor, Cursor)
#define XmTable_save_size(w) XmTableField(w, save_size, XRectangle)
#define XmTable_save_shadow_thickness(w) XmTableField(w, save_shadow_thickness, Dimension)
#define XmTable_title_height(w) XmTableField(w, title_height, int)
#define XmTable_table_height(w) XmTableField(w, table_height, int)
#define XmTable_table_width(w) XmTableField(w, table_width, int)
#define XmTable_row_height(w) XmTableField(w, row_height, int)
#define XmTable_valid_cnt(w) XmTableField(w, valid_cnt, int)
#define XmTable_offset(w) XmTableField(w, offset, int*)
#define XmTable_num_offsets(w) XmTableField(w, num_offsets, int)
#define XmTable_vscroll(w) XmTableField(w, vscroll, Widget)
#define XmTable_hscroll(w) XmTableField(w, hscroll, Widget)
#define XmTable_clip(w) XmTableField(w, clip, Widget)
#define XmTable_title_clip(w) XmTableField(w, title_clip, Widget)
#define XmTable_title_backing(w) XmTableField(w, title_backing, Widget)
#define XmTable_foreground_GC(w) XmTableField(w, foreground_GC, GC)
#define XmTable_title_top_shadow_GC(w) XmTableField(w, title_top_shadow_GC, GC)
#define XmTable_title_bottom_shadow_GC(w) XmTableField(w, title_bottom_shadow_GC, GC)
#define XmTable_table_GC(w) XmTableField(w, table_GC, GC)
#define XmTable_line_GC(w) XmTableField(w, line_GC, GC)
#define XmTable_shade_GC(w) XmTableField(w, shade_GC, GC)
#define XmTable_move_GC(w) XmTableField(w, move_GC, GC)


#define XmYOffset(t,i)    (((i) < 0 ||  XmTable_offset(t)== NULL) ? 0 : \
			   XmTable_offset(t)[(i)])

#ifdef XmRENAME_WIDGETS
#define xmTableClassRec xmXXTableClassRec
#endif


extern void _XmTopShadowColorDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern void _XmManagerTopShadowPixmapDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;


extern XmTableClassRec xmTableClassRec;

#ifdef __cplusplus
}
#endif

#endif /* __xm_table_h__ */


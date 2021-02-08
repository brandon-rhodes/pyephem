#ifndef _XmDataFP_h
#define _XmDataFP_h

#include <Xm/DataF.h>
#include <Xm/XmP.h>
#include <Xm/ExtP.h>
#include <Xm/TextFP.h>
#include <Xm/Picture.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Motif doesn't define this:  go figure
 */
#ifndef XmTextFieldIndex
#define XmTextFieldIndex (XmPrimitiveIndex + 1)
#endif

#define XmDataFieldIndex (XmTextFieldIndex)

typedef struct _XmDataFieldClassPart {
    XtPointer extension;
} XmDataFieldClassPart;

typedef struct _XmDataFieldClassRec {
    CoreClassPart        core_class;
    XmPrimitiveClassPart primitive_class;
    XmDataFieldClassPart data_class;
} XmDataFieldClassRec;

typedef struct _XmDataFieldPart {
    unsigned char   alignment;	/* XmALIGNMENT_BEGINNING by default */
    String          picture_source;
    XmPicture       picture;
    Boolean         auto_fill;
    XtCallbackList  picture_error_cb;
    XtCallbackList  validate_cb;
} XmDataFieldPart;

typedef struct _XmDataFieldRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmDataFieldPart data;
} XmDataFieldRec;
    
extern XmDataFieldClassRec xmDataFieldClassRec;

extern XmOffsetPtr XmDataField_offsets;

#define XmTextFField(w,f,t) XmField(w, XmDataField_offsets, XmTextField, f, t)

#define XmTextF_activate_callback(w) XmTextFField(w, activate_callback, XtCallbackList)
#define XmTextF_focus_callback(w) XmTextFField(w, focus_callback, XtCallbackList)
#define XmTextF_losing_focus_callback(w) XmTextFField(w, losing_focus_callback, XtCallbackList)
#define XmTextF_modify_verify_callback(w) XmTextFField(w, modify_verify_callback, XtCallbackList)
#define XmTextF_wcs_modify_verify_callback(w) XmTextFField(w, wcs_modify_verify_callback, XtCallbackList)
#define XmTextF_motion_verify_callback(w) XmTextFField(w, motion_verify_callback, XtCallbackList)
#define XmTextF_gain_primary_callback(w) XmTextFField(w, gain_primary_callback, XtCallbackList)
#define XmTextF_lose_primary_callback(w) XmTextFField(w, lose_primary_callback, XtCallbackList)
#define XmTextF_value_changed_callback(w) XmTextFField(w, value_changed_callback, XtCallbackList)
#define XmTextF_value(w) XmTextFField(w, value, char*)
#define XmTextF_wc_value(w) XmTextFField(w, wc_value, wchar_t*)
#define XmTextF_font_list(w) XmTextFField(w, font_list, XmFontList)
#define XmTextF_font(w) XmTextFField(w, font, XFontStruct*)
#define XmTextF_selection_array(w) XmTextFField(w, selection_array, XmTextScanType*)
#define XmTextF_highlight(w) XmTextFField(w, highlight, _XmHighlightData)
#define XmTextF_gc(w) XmTextFField(w, gc, GC)
#define XmTextF_image_gc(w) XmTextFField(w, image_gc, GC)
#define XmTextF_save_gc(w) XmTextFField(w, save_gc, GC)
#define XmTextF_ibeam_off(w) XmTextFField(w, ibeam_off, Pixmap)
#define XmTextF_add_mode_cursor(w) XmTextFField(w, add_mode_cursor, Pixmap)
#define XmTextF_cursor(w) XmTextFField(w, cursor, Pixmap)
#define XmTextF_putback(w) XmTextFField(w, putback, Pixmap)
#define XmTextF_stipple_tile(w) XmTextFField(w, stipple_tile, Pixmap)
#define XmTextF_image_clip(w) XmTextFField(w, image_clip, Pixmap)
#define XmTextF_cursor_position(w) XmTextFField(w, cursor_position, XmTextPosition)
#define XmTextF_new_h_offset(w) XmTextFField(w, new_h_offset, XmTextPosition)
#define XmTextF_h_offset(w) XmTextFField(w, h_offset, XmTextPosition)
#define XmTextF_orig_left(w) XmTextFField(w, orig_left, XmTextPosition)
#define XmTextF_orig_right(w) XmTextFField(w, orig_right, XmTextPosition)
#define XmTextF_prim_pos_left(w) XmTextFField(w, prim_pos_left, XmTextPosition)
#define XmTextF_prim_pos_right(w) XmTextFField(w, prim_pos_right, XmTextPosition)
#define XmTextF_prim_anchor(w) XmTextFField(w, prim_anchor, XmTextPosition)
#define XmTextF_sec_pos_left(w) XmTextFField(w, sec_pos_left, XmTextPosition)
#define XmTextF_sec_pos_right(w) XmTextFField(w, sec_pos_right, XmTextPosition)
#define XmTextF_sec_anchor(w) XmTextFField(w, sec_anchor, XmTextPosition)
#define XmTextF_stuff_pos(w) XmTextFField(w, stuff_pos, XmTextPosition)
#define XmTextF_select_pos_x(w) XmTextFField(w, select_pos_x, Position)
#define XmTextF_prim_time(w) XmTextFField(w, prim_time, Time)
#define XmTextF_dest_time(w) XmTextFField(w, dest_time, Time)
#define XmTextF_sec_time(w) XmTextFField(w, sec_time, Time)
#define XmTextF_last_time(w) XmTextFField(w, last_time, Time)
#define XmTextF_timer_id(w) XmTextFField(w, timer_id, XtIntervalId)
#define XmTextF_select_id(w) XmTextFField(w, select_id, XtIntervalId)
#define XmTextF_blink_rate(w) XmTextFField(w, blink_rate, int)
#define XmTextF_selection_array_count(w) XmTextFField(w, selection_array_count, int)
#define XmTextF_threshold(w) XmTextFField(w, threshold, int)
#define XmTextF_size_allocd(w) XmTextFField(w, size_allocd, int)
#define XmTextF_string_length(w) XmTextFField(w, string_length, int)
#define XmTextF_cursor_height(w) XmTextFField(w, cursor_height, int)
#define XmTextF_cursor_width(w) XmTextFField(w, cursor_width, int)
#define XmTextF_sarray_index(w) XmTextFField(w, sarray_index, int)
#define XmTextF_max_length(w) XmTextFField(w, max_length, int)
#define XmTextF_max_char_size(w) XmTextFField(w, max_char_size, int)
#define XmTextF_columns(w) XmTextFField(w, columns, short)
#define XmTextF_margin_width(w) XmTextFField(w, margin_width, Dimension)
#define XmTextF_margin_height(w) XmTextFField(w, margin_height, Dimension)
#define XmTextF_average_char_width(w) XmTextFField(w, average_char_width, Dimension)
#define XmTextF_margin_top(w) XmTextFField(w, margin_top, Dimension)
#define XmTextF_margin_bottom(w) XmTextFField(w, margin_bottom, Dimension)
#define XmTextF_font_ascent(w) XmTextFField(w, font_ascent, Dimension)
#define XmTextF_font_descent(w) XmTextFField(w, font_descent, Dimension)
#define XmTextF_resize_width(w) XmTextFField(w, resize_width, Boolean)
#define XmTextF_pending_delete(w) XmTextFField(w, pending_delete, Boolean)
#define XmTextF_editable(w) XmTextFField(w, editable, Boolean)
#define XmTextF_verify_bell(w) XmTextFField(w, verify_bell, Boolean)
#define XmTextF_cursor_position_visible(w) XmTextFField(w, cursor_position_visible, Boolean)
#define XmTextF_traversed(w) XmTextFField(w, traversed, Boolean)
#define XmTextF_add_mode(w) XmTextFField(w, add_mode, Boolean)
#define XmTextF_has_focus(w) XmTextFField(w, has_focus, Boolean)
#define XmTextF_blink_on(w) XmTextFField(w, blink_on, Boolean)
#define XmTextF_cursor_on(w) XmTextFField(w, cursor_on, short int)
#define XmTextF_refresh_ibeam_off(w) XmTextFField(w, refresh_ibeam_off, Boolean)
#define XmTextF_have_inverted_image_gc(w) XmTextFField(w, have_inverted_image_gc, Boolean)
#define XmTextF_has_primary(w) XmTextFField(w, has_primary, Boolean)
#define XmTextF_has_secondary(w) XmTextFField(w, has_secondary, Boolean)
#define XmTextF_has_destination(w) XmTextFField(w, has_destination, Boolean)
#define XmTextF_sec_drag(w) XmTextFField(w, sec_drag, Boolean)
#define XmTextF_selection_move(w) XmTextFField(w, selection_move, Boolean)
#define XmTextF_pending_off(w) XmTextFField(w, pending_off, Boolean)
#define XmTextF_fontlist_created(w) XmTextFField(w, fontlist_created, Boolean)
#define XmTextF_has_rect(w) XmTextFField(w, has_rect, Boolean)
#define XmTextF_do_drop(w) XmTextFField(w, do_drop, Boolean)
#define XmTextF_cancel(w) XmTextFField(w, cancel, Boolean)
#define XmTextF_extending(w) XmTextFField(w, extending, Boolean)
#define XmTextF_sec_extending(w) XmTextFField(w, sec_extending, Boolean)
#define XmTextF_changed_visible(w) XmTextFField(w, changed_visible, Boolean)
#define XmTextF_have_fontset(w) XmTextFField(w, have_fontset, Boolean)
#define XmTextF_in_setvalues(w) XmTextFField(w, in_setvalues, Boolean)
#define XmTextF_do_resize(w) XmTextFField(w, do_resize, Boolean)
#define XmTextF_redisplay(w) XmTextFField(w, redisplay, Boolean)
#define XmTextF_overstrike(w) XmTextFField(w, overstrike, Boolean)
#define XmTextF_sel_start(w) XmTextFField(w, sel_start, Boolean)
#define XmTextF_extension(w) XmTextFField(w, extension, XtPointer)

#define XmDataFieldField(w,f,t) \
    (*(t *)(((char *) (w)) + XmDataField_offsets[XmTextFieldIndex] \
	    + sizeof(XmTextFieldPart) \
	    + XtOffsetOf(XmDataFieldPart, f)))
	 
#define XmDataField_alignment(w) XmDataFieldField(w, alignment, unsigned char)
#define XmDataField_picture_source(w) XmDataFieldField(w, picture_source, String)
#define XmDataField_picture(w) XmDataFieldField(w, picture, XmPicture)
#define XmDataField_picture_state(w) XmDataFieldField(w, picture_state, XmPictureState)
#define XmDataField_auto_fill(w) XmDataFieldField(w, auto_fill, Boolean)
#define XmDataField_picture_error_cb(w) XmDataFieldField(w, picture_error_cb, XtCallbackList)
#define XmDataField_validate_cb(w) XmDataFieldField(w, validate_cb, XtCallbackList)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _XmDataFP_h */

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
    XmTextFieldPart text;
    XmDataFieldPart data;
} XmDataFieldRec;
    
extern XmDataFieldClassRec xmDataFieldClassRec;

#define XmTextF_activate_callback(w) (((XmDataFieldWidget)(w))->text.activate_callback)
#define XmTextF_focus_callback(w) (((XmDataFieldWidget)(w))->text.focus_callback)
#define XmTextF_losing_focus_callback(w) (((XmDataFieldWidget)(w))->text.losing_focus_callback)
#define XmTextF_modify_verify_callback(w) (((XmDataFieldWidget)(w))->text.modify_verify_callback)
#define XmTextF_wcs_modify_verify_callback(w) (((XmDataFieldWidget)(w))->text.wcs_modify_verify_callback)
#define XmTextF_motion_verify_callback(w) (((XmDataFieldWidget)(w))->text.motion_verify_callback)
#define XmTextF_gain_primary_callback(w) (((XmDataFieldWidget)(w))->text.gain_primary_callback)
#define XmTextF_lose_primary_callback(w) (((XmDataFieldWidget)(w))->text.lose_primary_callback)
#define XmTextF_value_changed_callback(w) (((XmDataFieldWidget)(w))->text.value_changed_callback)
#define XmTextF_value(w) (((XmDataFieldWidget)(w))->text.value)
#define XmTextF_wc_value(w) (((XmDataFieldWidget)(w))->text.wc_value)
#define XmTextF_font_list(w) (((XmDataFieldWidget)(w))->text.font_list)
#define XmTextF_font(w) ((XFontStruct*)(((XmDataFieldWidget)(w))->text.font))
#define XmTextF_selection_array(w) (((XmDataFieldWidget)(w))->text.selection_array)
#define XmTextF_highlight(w) (((XmDataFieldWidget)(w))->text.highlight)
#define XmTextF_gc(w) (((XmDataFieldWidget)(w))->text.gc)
#define XmTextF_image_gc(w) (((XmDataFieldWidget)(w))->text.image_gc)
#define XmTextF_save_gc(w) (((XmDataFieldWidget)(w))->text.save_gc)
#define XmTextF_ibeam_off(w) (((XmDataFieldWidget)(w))->text.ibeam_off)
#define XmTextF_add_mode_cursor(w) (((XmDataFieldWidget)(w))->text.add_mode_cursor)
#define XmTextF_cursor(w) (((XmDataFieldWidget)(w))->text.cursor)
#define XmTextF_putback(w) (((XmDataFieldWidget)(w))->text.putback)
#define XmTextF_stipple_tile(w) (((XmDataFieldWidget)(w))->text.stipple_tile)
#define XmTextF_image_clip(w) (((XmDataFieldWidget)(w))->text.image_clip)
#define XmTextF_cursor_position(w) (((XmDataFieldWidget)(w))->text.cursor_position)
#define XmTextF_new_h_offset(w) (((XmDataFieldWidget)(w))->text.new_h_offset)
#define XmTextF_h_offset(w) (((XmDataFieldWidget)(w))->text.h_offset)
#define XmTextF_orig_left(w) (((XmDataFieldWidget)(w))->text.orig_left)
#define XmTextF_orig_right(w) (((XmDataFieldWidget)(w))->text.orig_right)
#define XmTextF_prim_pos_left(w) (((XmDataFieldWidget)(w))->text.prim_pos_left)
#define XmTextF_prim_pos_right(w) (((XmDataFieldWidget)(w))->text.prim_pos_right)
#define XmTextF_prim_anchor(w) (((XmDataFieldWidget)(w))->text.prim_anchor)
#define XmTextF_sec_pos_left(w) (((XmDataFieldWidget)(w))->text.sec_pos_left)
#define XmTextF_sec_pos_right(w) (((XmDataFieldWidget)(w))->text.sec_pos_right)
#define XmTextF_sec_anchor(w) (((XmDataFieldWidget)(w))->text.sec_anchor)
#define XmTextF_stuff_pos(w) (((XmDataFieldWidget)(w))->text.stuff_pos)
#define XmTextF_select_pos_x(w) (((XmDataFieldWidget)(w))->text.select_pos_x)
#define XmTextF_prim_time(w) (((XmDataFieldWidget)(w))->text.prim_time)
#define XmTextF_dest_time(w) (((XmDataFieldWidget)(w))->text.dest_time)
#define XmTextF_sec_time(w) (((XmDataFieldWidget)(w))->text.sec_time)
#define XmTextF_last_time(w) (((XmDataFieldWidget)(w))->text.last_time)
#define XmTextF_timer_id(w) (((XmDataFieldWidget)(w))->text.timer_id)
#define XmTextF_select_id(w) (((XmDataFieldWidget)(w))->text.select_id)
#define XmTextF_blink_rate(w) (((XmDataFieldWidget)(w))->text.blink_rate)
#define XmTextF_selection_array_count(w) (((XmDataFieldWidget)(w))->text.selection_array_count)
#define XmTextF_threshold(w) (((XmDataFieldWidget)(w))->text.threshold)
#define XmTextF_size_allocd(w) (((XmDataFieldWidget)(w))->text.size_allocd)
#define XmTextF_string_length(w) (((XmDataFieldWidget)(w))->text.string_length)
#define XmTextF_cursor_height(w) (((XmDataFieldWidget)(w))->text.cursor_height)
#define XmTextF_cursor_width(w) (((XmDataFieldWidget)(w))->text.cursor_width)
#define XmTextF_sarray_index(w) (((XmDataFieldWidget)(w))->text.sarray_index)
#define XmTextF_max_length(w) (((XmDataFieldWidget)(w))->text.max_length)
#define XmTextF_max_char_size(w) (((XmDataFieldWidget)(w))->text.max_char_size)
#define XmTextF_columns(w) (((XmDataFieldWidget)(w))->text.columns)
#define XmTextF_margin_width(w) (((XmDataFieldWidget)(w))->text.margin_width)
#define XmTextF_margin_height(w) (((XmDataFieldWidget)(w))->text.margin_height)
#define XmTextF_average_char_width(w) (((XmDataFieldWidget)(w))->text.average_char_width)
#define XmTextF_margin_top(w) (((XmDataFieldWidget)(w))->text.margin_top)
#define XmTextF_margin_bottom(w) (((XmDataFieldWidget)(w))->text.margin_bottom)
#define XmTextF_font_ascent(w) (((XmDataFieldWidget)(w))->text.font_ascent)
#define XmTextF_font_descent(w) (((XmDataFieldWidget)(w))->text.font_descent)
#define XmTextF_resize_width(w) (((XmDataFieldWidget)(w))->text.resize_width)
#define XmTextF_pending_delete(w) (((XmDataFieldWidget)(w))->text.pending_delete)
#define XmTextF_editable(w) (((XmDataFieldWidget)(w))->text.editable)
#define XmTextF_verify_bell(w) (((XmDataFieldWidget)(w))->text.verify_bell)
#define XmTextF_cursor_position_visible(w) (((XmDataFieldWidget)(w))->text.cursor_position_visible)
#define XmTextF_traversed(w) (((XmDataFieldWidget)(w))->text.traversed)
#define XmTextF_add_mode(w) (((XmDataFieldWidget)(w))->text.add_mode)
#define XmTextF_has_focus(w) (((XmDataFieldWidget)(w))->text.has_focus)
#define XmTextF_blink_on(w) (((XmDataFieldWidget)(w))->text.blink_on)
#define XmTextF_cursor_on(w) (((XmDataFieldWidget)(w))->text.cursor_on)
#define XmTextF_refresh_ibeam_off(w) (((XmDataFieldWidget)(w))->text.refresh_ibeam_off)
#define XmTextF_have_inverted_image_gc(w) (((XmDataFieldWidget)(w))->text.have_inverted_image_gc)
#define XmTextF_has_primary(w) (((XmDataFieldWidget)(w))->text.has_primary)
#define XmTextF_has_secondary(w) (((XmDataFieldWidget)(w))->text.has_secondary)
#define XmTextF_has_destination(w) (((XmDataFieldWidget)(w))->text.has_destination)
#define XmTextF_sec_drag(w) (((XmDataFieldWidget)(w))->text.sec_drag)
#define XmTextF_selection_move(w) (((XmDataFieldWidget)(w))->text.selection_move)
#define XmTextF_pending_off(w) (((XmDataFieldWidget)(w))->text.pending_off)
#define XmTextF_fontlist_created(w) (((XmDataFieldWidget)(w))->text.fontlist_created)
#define XmTextF_has_rect(w) (((XmDataFieldWidget)(w))->text.has_rect)
#define XmTextF_do_drop(w) (((XmDataFieldWidget)(w))->text.do_drop)
#define XmTextF_cancel(w) (((XmDataFieldWidget)(w))->text.cancel)
#define XmTextF_extending(w) (((XmDataFieldWidget)(w))->text.extending)
#define XmTextF_sec_extending(w) (((XmDataFieldWidget)(w))->text.sec_extending)
#define XmTextF_in_setvalues(w) (((XmDataFieldWidget)(w))->text.in_setvalues)
#define XmTextF_do_resize(w) (((XmDataFieldWidget)(w))->text.do_resize)
#define XmTextF_sel_start(w) (((XmDataFieldWidget)(w))->text.sel_start)
#define XmTextF_check_set_render_table(w) (((XmDataFieldWidget)(w))->text.check_set_render_table)
#define XmTextF_extension(w) (((XmDataFieldWidget)(w))->text.extension)
#define XmTextF_overstrike(w) (((XmDataFieldWidget)(w))->text.overstrike)
#define XmTextF_redisplay(w) (((XmDataFieldWidget)(w))->text.redisplay)
#define XmTextF_have_fontset(w) (((XmDataFieldWidget)(w))->text.have_fontset)
#ifdef USE_XFT
#define XmTextF_use_xft(w) (((XmDataFieldWidget)(w))->text.use_xft)
#define	XmTextF_xft_font(w) (((XftFont*)((XmDataFieldWidget)(w))->text.font))
#endif
#define XmTextF_changed_visible(w) (((XmDataFieldWidget)(w))->text.changed_visible)

#define XmDataField_alignment(w) (((XmDataFieldWidget)(w))->data.alignment)
#define XmDataField_picture_source(w) (((XmDataFieldWidget)(w))->data.picture_source)
#define XmDataField_picture(w) (((XmDataFieldWidget)(w))->data.picture)
#define XmDataField_picture_state(w) (((XmDataFieldWidget)(w))->data.picture_state)
#define XmDataField_auto_fill(w) (((XmDataFieldWidget)(w))->data.auto_fill)
#define XmDataField_picture_error_cb(w) (((XmDataFieldWidget)(w))->data.picture_error_cb)
#define XmDataField_validate_cb(w) (((XmDataFieldWidget)(w))->data.validate_cb)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _XmDataFP_h */

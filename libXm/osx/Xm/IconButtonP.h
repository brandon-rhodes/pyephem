/*
 *    Copyright 1990, 1992 -- Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmIconButtonP_h
#define _XmIconButtonP_h


/************************************************************
*	INCLUDE FILES
*************************************************************/
#include <Xm/PrimitiveP.h>
#include <Xm/IconButton.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

#define XmIconButtonIndex (XmPrimitiveIndex + 1)

/************************************************************
*	MACROS
*************************************************************/

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

typedef struct {
    XtPointer extension;	/* Just in case we need it later. */
} XmIconButtonClassPart;

typedef struct _XmIconButtonClassRec {
    CoreClassPart		core_class;
    XmPrimitiveClassPart	primitive_class;
    XmIconButtonClassPart	        icon_class;
} XmIconButtonClassRec;

typedef struct {
    /* resources */

    String label;		/* The label to display. */
    Pixmap pixmap;		/* The pixmap to display. */
    Pixel arm_color;		/* The color to arm this widget with. */
    XmFontList font_list;	/* The font in MOTIF(tm) format. */
    XmIconPlacement icon_placement; /* Where to place the icon label. */
    Boolean recompute;		/* Whether to recompute size every time. */
    Boolean set;		/* State of the button. */
    Boolean armed;		/* Armed value for button. */

    Dimension v_space;		/* The amount of space between the edges */
    Dimension h_space;		/* of the widget and the picture/text. */

    Dimension icon_text_padding; /* padding between the icon and the text. */

    XtCallbackList activate_callback; /* Called when I am selected */
    XtCallbackList double_click_callback; /* Called when I am 2 clicked. */

    /*
     * Added in 1.2 for I18N.
     */

    XmString label_string;	/* The label stored as an XmString. */
    unsigned char string_direction;
    unsigned char alignment;

    /* private state */

    Position pix_x, pix_y;	/* Location of the pixmap. */

    /* public state */
    Dimension pix_width, pix_height; /* size of the pixmap. */
    Dimension pix_depth;	/* Depth of the pixmap. */

    /* private state */
    Position text_x, text_y;	/* Location to begin drawing text. */
    Dimension max_text_width;	/* maximum space the text can take up. */
    Dimension max_text_height;	/* maximum vertical space for the text */

    XtIntervalId unset_timer;	/* The arm and activate timer. */
    GC gc;			/* The graphics context. */
    GC fill_gc;			/* The gc for filling on the arm. */
    GC pixmap_fill_gc;		/* The gc for drawing the pixmap when
				   the button is filled. */

    GC stippled_text_gc;	/* GC to use stippling text. */
    GC stippled_set_text_gc;	/* GC to use for stip. text in a set button. */
    GC stippled_set_gc;		/* GC to use for images when toggle is set. */
    GC stippled_unset_gc;	/* GC to use for images when toggle is unset.*/

    Time time;			/* The server time of the last button click. */

    Boolean label_from_name;
    Boolean check_set_render_table;

} XmIconButtonPart;

#define XmIconButton_label(w) (((XmIconButtonWidget)(w))->icon.label)
#define XmIconButton_pixmap(w) (((XmIconButtonWidget)(w))->icon.pixmap)
#define XmIconButton_arm_color(w) (((XmIconButtonWidget)(w))->icon.arm_color)
#define XmIconButton_font_list(w) (((XmIconButtonWidget)(w))->icon.font_list)
#define XmIconButton_icon_placement(w) (((XmIconButtonWidget)(w))->icon.icon_placement)
#define XmIconButton_recompute(w) (((XmIconButtonWidget)(w))->icon.recompute)
#define XmIconButton_set(w) (((XmIconButtonWidget)(w))->icon.set)
#define XmIconButton_armed(w) (((XmIconButtonWidget)(w))->icon.armed)
#define XmIconButton_v_space(w) (((XmIconButtonWidget)(w))->icon.v_space)
#define XmIconButton_h_space(w) (((XmIconButtonWidget)(w))->icon.h_space)
#define XmIconButton_icon_text_padding(w) (((XmIconButtonWidget)(w))->icon.icon_text_padding)
#define XmIconButton_activate_callback(w) (((XmIconButtonWidget)(w))->icon.activate_callback)
#define XmIconButton_double_click_callback(w) (((XmIconButtonWidget)(w))->icon.double_click_callback)
#define XmIconButton_label_string(w) (((XmIconButtonWidget)(w))->icon.label_string)
#define XmIconButton_string_direction(w) (((XmIconButtonWidget)(w))->icon.string_direction)
#define XmIconButton_alignment(w) (((XmIconButtonWidget)(w))->icon.alignment)
#define XmIconButton_pix_x(w) (((XmIconButtonWidget)(w))->icon.pix_x)
#define XmIconButton_pix_y(w) (((XmIconButtonWidget)(w))->icon.pix_y)
#define XmIconButton_pix_width(w) (((XmIconButtonWidget)(w))->icon.pix_width)
#define XmIconButton_pix_height(w) (((XmIconButtonWidget)(w))->icon.pix_height)
#define XmIconButton_pix_depth(w) (((XmIconButtonWidget)(w))->icon.pix_depth)
#define XmIconButton_text_x(w) (((XmIconButtonWidget)(w))->icon.text_x)
#define XmIconButton_text_y(w) (((XmIconButtonWidget)(w))->icon.text_y)
#define XmIconButton_max_text_width(w) (((XmIconButtonWidget)(w))->icon.max_text_width)
#define XmIconButton_max_text_height(w) (((XmIconButtonWidget)(w))->icon.max_text_height)
#define XmIconButton_unset_timer(w) (((XmIconButtonWidget)(w))->icon.unset_timer)
#define XmIconButton_gc(w) (((XmIconButtonWidget)(w))->icon.gc)
#define XmIconButton_fill_gc(w) (((XmIconButtonWidget)(w))->icon.fill_gc)
#define XmIconButton_pixmap_fill_gc(w) (((XmIconButtonWidget)(w))->icon.pixmap_fill_gc)
#define XmIconButton_stippled_text_gc(w) (((XmIconButtonWidget)(w))->icon.stippled_text_gc)
#define XmIconButton_stippled_set_text_gc(w) (((XmIconButtonWidget)(w))->icon.stippled_set_text_gc)
#define XmIconButton_stippled_set_gc(w) (((XmIconButtonWidget)(w))->icon.stippled_set_gc)
#define XmIconButton_stippled_unset_gc(w) (((XmIconButtonWidget)(w))->icon.stippled_unset_gc)
#define XmIconButton_time(w) (((XmIconButtonWidget)(w))->icon.time)
#define XmIconButton_label_from_name(w) (((XmIconButtonWidget)(w))->icon.label_from_name)

typedef struct _XmIconButtonRec {
    CorePart		core;
    XmPrimitivePart	primitive;
    XmIconButtonPart	icon;
} XmIconButtonRec;

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/

extern XmIconButtonClassRec xmIconButtonClassRec;

/************************************************************
*	STATIC DECLARATIONS
*************************************************************/

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif



void _XmPrimitiveEnter(
                 Widget wid,
                 XEvent *event,
                 String *params,
                 Cardinal *num_params) ;
void _XmPrimitiveLeave(
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;



#endif /* _XmIconButtonP_h */

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

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

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
} XmIconButtonPart;

#define XmIconButtonField(w,f,t) XmField(w, XmIconButton_offsets, XmIconButton, f, t)

extern XmOffsetPtr XmIconButton_offsets;

#define XmIconButton_label(w) XmIconButtonField(w, label, String)
#define XmIconButton_pixmap(w) XmIconButtonField(w, pixmap, Pixmap)
#define XmIconButton_arm_color(w) XmIconButtonField(w, arm_color, Pixel)
#define XmIconButton_font_list(w) XmIconButtonField(w, font_list, XmFontList)
#define XmIconButton_icon_placement(w) XmIconButtonField(w, icon_placement, XmIconPlacement)
#define XmIconButton_recompute(w) XmIconButtonField(w, recompute, Boolean)
#define XmIconButton_set(w) XmIconButtonField(w, set, Boolean)
#define XmIconButton_armed(w) XmIconButtonField(w, armed, Boolean)
#define XmIconButton_v_space(w) XmIconButtonField(w, v_space, Dimension)
#define XmIconButton_h_space(w) XmIconButtonField(w, h_space, Dimension)
#define XmIconButton_icon_text_padding(w) XmIconButtonField(w, icon_text_padding, Dimension)
#define XmIconButton_activate_callback(w) XmIconButtonField(w, activate_callback, XtCallbackList)
#define XmIconButton_double_click_callback(w) XmIconButtonField(w, double_click_callback, XtCallbackList)
#define XmIconButton_label_string(w) XmIconButtonField(w, label_string, XmString)
#define XmIconButton_string_direction(w) XmIconButtonField(w, string_direction, unsigned char)
#define XmIconButton_alignment(w) XmIconButtonField(w, alignment, unsigned char)
#define XmIconButton_pix_x(w) XmIconButtonField(w, pix_x, Position)
#define XmIconButton_pix_y(w) XmIconButtonField(w, pix_y, Position)
#define XmIconButton_pix_width(w) XmIconButtonField(w, pix_width, Dimension)
#define XmIconButton_pix_height(w) XmIconButtonField(w, pix_height, Dimension)
#define XmIconButton_pix_depth(w) XmIconButtonField(w, pix_depth, Dimension)
#define XmIconButton_text_x(w) XmIconButtonField(w, text_x, Position)
#define XmIconButton_text_y(w) XmIconButtonField(w, text_y, Position)
#define XmIconButton_max_text_width(w) XmIconButtonField(w, max_text_width, Dimension)
#define XmIconButton_max_text_height(w) XmIconButtonField(w, max_text_height, Dimension)
#define XmIconButton_unset_timer(w) XmIconButtonField(w, unset_timer, XtIntervalId)
#define XmIconButton_gc(w) XmIconButtonField(w, gc, GC)
#define XmIconButton_fill_gc(w) XmIconButtonField(w, fill_gc, GC)
#define XmIconButton_pixmap_fill_gc(w) XmIconButtonField(w, pixmap_fill_gc, GC)
#define XmIconButton_stippled_text_gc(w) XmIconButtonField(w, stippled_text_gc, GC)
#define XmIconButton_stippled_set_text_gc(w) XmIconButtonField(w, stippled_set_text_gc, GC)
#define XmIconButton_stippled_set_gc(w) XmIconButtonField(w, stippled_set_gc, GC)
#define XmIconButton_stippled_unset_gc(w) XmIconButtonField(w, stippled_unset_gc, GC)
#define XmIconButton_time(w) XmIconButtonField(w, time, Time)
#define XmIconButton_label_from_name(w) XmIconButtonField(w, label_from_name, Boolean)

#define XmIconButtonIndex (XmPrimitiveIndex + 1)


typedef struct _XmIconButtonRec {
    CorePart		core;
    XmPrimitivePart	primitive;
    XmIconButtonPart	icon;
} XmIconButtonRec;

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/

extern XmIconButtonClassRec xiIconButtonClassRec;

/************************************************************
*	STATIC DECLARATIONS
*************************************************************/

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif



extern void _XmPrimitiveEnter(
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveLeave(
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;



#endif /* _XmIconButtonP_h */

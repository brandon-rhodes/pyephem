#ifndef __TabStackP_h__
#define __TabStackP_h__

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/BulletinBP.h>
#include <Xm/ExtP.h>
#include <Xm/TabBox.h>
#include <Xm/TabList.h>
#include <Xm/TabStack.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmTabStackClassPart {
    String    drag_translations;
    XtPointer extension;
} XmTabStackClassPart;

typedef struct _XmTabStackClassRec {
    CoreClassPart            core_class;
    CompositeClassPart	     composite_class;
    ConstraintClassPart      constraint_class;
    XmManagerClassPart       manager_class;
    XmBulletinBoardClassPart bulletin_board_class;
    XmTabStackClassPart      tab_stack_class;
} XmTabStackClassRec;

typedef struct _XmTabStackPart {
    /* Resources for the Tab Stack */

    XtCallbackList       tab_select_callback;

    /* Resource to pass down to Tab Box */
    XmFontList		 font_list;
    XmTabStyle		 tab_style;
    XmTabMode		 tab_mode;
    XmTabSide            tab_side;
    XmTabOrientation	 tab_orientation;

    XmString		 tear_off_label;

    Boolean		 allow_tear_offs;
    Boolean		 uniform_tab_size;
    Boolean		 use_image_cache;
    Boolean		 stacked_effect;
    Boolean		 tab_auto_select;
    
    Dimension            tab_margin_width;
    Dimension            tab_margin_height;
    Dimension		 tab_label_spacing;
    Dimension            tab_offset;
    Dimension            highlight_thickness;

    Pixel		 select_color;
    Pixel		 select_pixmap;

    int			 tab_corner_percent;

    Widget               tab_box;

    /* Private Values */

    XmTabbedStackList		 _tab_list;

    XRectangle           _size;

    Widget               _active_child;
    GC			 _gc;

    Boolean		 _inited;
    Boolean		 _set_tab_list;

    Widget               _menu;
    Widget		 _tear_off_button;

    /* Drag And Drop Stuff */
    Pixmap               _source_pixmap;
    Pixmap               _source_mask;
    Pixmap               _invalid_pixmap;
    Pixmap               _invalid_mask;
    Widget               _source_icon;
    Widget               _invalid_icon;

    Widget		_selected_tab;	    /* used within realize method */
    Boolean		_selected_notify;   /* used within realize method */

    Boolean		do_notify;	/* for notify XmNtabSelectedCallback */

} XmTabStackPart;

#define XmTabStackField(w,f,t) XmField(w, XmTabStack_offsets, XmTabStack, f, t)

#define XmTabStack_tab_select_callback(w) XmTabStackField(w, tab_select_callback, XtCallbackList)
#define XmTabStack_font_list(w) XmTabStackField(w, font_list, XmFontList)
#define XmTabStack_tab_style(w) XmTabStackField(w, tab_style, XmTabStyle)
#define XmTabStack_tab_mode(w) XmTabStackField(w, tab_mode, XmTabMode)
#define XmTabStack_tab_side(w) XmTabStackField(w, tab_side, XmTabSide)
#define XmTabStack_tab_orientation(w) XmTabStackField(w, tab_orientation, XmTabOrientation)
#define XmTabStack_tear_off_label(w) XmTabStackField(w, tear_off_label, XmString)
#define XmTabStack_allow_tear_offs(w) XmTabStackField(w, allow_tear_offs, Boolean)
#define XmTabStack_uniform_tab_size(w) XmTabStackField(w, uniform_tab_size, Boolean)
#define XmTabStack_use_image_cache(w) XmTabStackField(w, use_image_cache, Boolean)
#define XmTabStack_stacked_effect(w) XmTabStackField(w, stacked_effect, Boolean)
#define XmTabStack_tab_auto_select(w) XmTabStackField(w, tab_auto_select, Boolean)
#define XmTabStack_tab_margin_width(w) XmTabStackField(w, tab_margin_width, Dimension)
#define XmTabStack_tab_margin_height(w) XmTabStackField(w, tab_margin_height, Dimension)
#define XmTabStack_tab_label_spacing(w) XmTabStackField(w, tab_label_spacing, Dimension)
#define XmTabStack_tab_offset(w) XmTabStackField(w, tab_offset, Dimension)
#define XmTabStack_highlight_thickness(w) XmTabStackField(w, highlight_thickness, Dimension)
#define XmTabStack_select_color(w) XmTabStackField(w, select_color, Pixel)
#define XmTabStack_select_pixmap(w) XmTabStackField(w, select_pixmap, Pixel)
#define XmTabStack_tab_corner_percent(w) XmTabStackField(w, tab_corner_percent, int)
#define XmTabStack_tab_box(w) XmTabStackField(w, tab_box, Widget)
#define XmTabStack__tab_list(w) XmTabStackField(w, _tab_list, XmTabbedStackList)
#define XmTabStack__size(w) XmTabStackField(w, _size, XRectangle)
#define XmTabStack__active_child(w) XmTabStackField(w, _active_child, Widget)
#define XmTabStack__gc(w) XmTabStackField(w, _gc, GC)
#define XmTabStack__inited(w) XmTabStackField(w, _inited, Boolean)
#define XmTabStack__set_tab_list(w) XmTabStackField(w, _set_tab_list, Boolean)
#define XmTabStack__menu(w) XmTabStackField(w, _menu, Widget)
#define XmTabStack__tear_off_button(w) XmTabStackField(w, _tear_off_button, Widget)
#define XmTabStack__source_pixmap(w) XmTabStackField(w, _source_pixmap, Pixmap)
#define XmTabStack__source_mask(w) XmTabStackField(w, _source_mask, Pixmap)
#define XmTabStack__invalid_pixmap(w) XmTabStackField(w, _invalid_pixmap, Pixmap)
#define XmTabStack__invalid_mask(w) XmTabStackField(w, _invalid_mask, Pixmap)
#define XmTabStack__source_icon(w) XmTabStackField(w, _source_icon, Widget)
#define XmTabStack__invalid_icon(w) XmTabStackField(w, _invalid_icon, Widget)
#define XmTabStack__selected_tab(w) XmTabStackField(w, _selected_tab, Widget)
#define XmTabStack__selected_notify(w) XmTabStackField(w, _selected_notify, Boolean)
#define XmTabStack_do_notify(w) XmTabStackField(w, do_notify, Boolean)

typedef struct _XmTabStackRec {
    CorePart             core;
    CompositePart        composite;
    ConstraintPart       constraint;
    XmManagerPart        manager;
    XmBulletinBoardPart  bulletin_board;
    XmTabStackPart       tab_stack;
} XmTabStackRec;

typedef struct _XmTabStackConstraintPart {
    XmString    	 tab_label_string;
    XmStringDirection    tab_string_direction;
    unsigned char        tab_alignment;
    Pixmap		 tab_label_pixmap;
    XmPixmapPlacement    tab_pixmap_placement;
    Pixel		 tab_foreground;
    Pixel		 tab_background;
    Pixmap		 tab_background_pixmap;
    Boolean		 free_tab_pixmap;

    Boolean		 tear_off_enabled;

    /* Private Memebers */
    int			index;
    Dimension		width;
    Dimension		height;
} XmTabStackConstraintPart;
#define XmTabStackCField(w, f, t) XmConstraintField(w, XmTabStackC_offsets, \
                                            XmTabStack, f, t)

#define XmTabStackC_tab_label_string(w) XmTabStackCField(w, tab_label_string, XmString)
#define XmTabStackC_tab_string_direction(w) XmTabStackCField(w, tab_string_direction, XmStringDirection)
#define XmTabStackC_tab_alignment(w) XmTabStackCField(w, tab_alignment, unsigned char)
#define XmTabStackC_tab_label_pixmap(w) XmTabStackCField(w, tab_label_pixmap, Pixmap)
#define XmTabStackC_tab_pixmap_placement(w) XmTabStackCField(w, tab_pixmap_placement, XmPixmapPlacement)
#define XmTabStackC_tab_foreground(w) XmTabStackCField(w, tab_foreground, Pixel)
#define XmTabStackC_tab_background(w) XmTabStackCField(w, tab_background, Pixel)
#define XmTabStackC_tab_background_pixmap(w) XmTabStackCField(w, tab_background_pixmap, Pixmap)
#define XmTabStackC_free_tab_pixmap(w) XmTabStackCField(w, free_tab_pixmap, Boolean)
#define XmTabStackC_tear_off_enabled(w) XmTabStackCField(w, tear_off_enabled, Boolean)
#define XmTabStackC_index(w) XmTabStackCField(w, index, int)
#define XmTabStackC_width(w) XmTabStackCField(w, width, Dimension)
#define XmTabStackC_height(w) XmTabStackCField(w, height, Dimension)

typedef struct _XmTabStackConstraintRec {
    XmManagerConstraintPart  manager;
    XmTabStackConstraintPart tab_stack;
} XmTabStackConstraintRec, * XmTabStackConstraintPtr;

extern XmOffsetPtr XmTabStack_offsets;
extern XmOffsetPtr XmTabStackC_offsets;

#define XmNillegalUniformTabSizeMsg \
"XmNuniformTabSize must be true if XmNtabMode is XmTABS_STACKED or\n\
XmTABS_STACKED_STATIC."

#define XmTabStackIndex (XmBulletinBoardIndex + 1)

#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* __TabStackP_h__ */

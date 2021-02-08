#ifndef __columnp_h__
#define __columnp_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/XmP.h>
#include <Xm/BulletinBP.h>
#include <Xm/Column.h>

typedef struct _XmColumnClassPart {
    XtPointer extension;
} XmColumnClassPart;

typedef struct _XmColumnClassRec {
    CoreClassPart            core_class;
    CompositeClassPart       composite_class;
    ConstraintClassPart      constraint_class;
    XmManagerClassPart       manager_class;
    XmBulletinBoardClassPart bulletin_board_class;
    XmColumnClassPart	     column_class;
} XmColumnClassRec;

typedef struct _XmColumnConstraintPart {

    /* Public */
    unsigned char       label_alignment;
    unsigned char       label_type;
    unsigned char       fill_style;

    Boolean		show_label;
    Boolean		stretchable;
    
    Pixmap              label_pixmap;
    XmString		label_string;

    XmFontList		label_font_list;

    /* Private */

    Widget              label_widget;
    Dimension		request_width;
    Dimension		request_height;
    XRectangle		position;
} XmColumnConstraintPart;

typedef struct _XmColumnConstraintRec {
    XmManagerConstraintPart manager;
    XmBulletinBoardConstraintPart bboard;
    XmColumnConstraintPart  column;
} XmColumnConstraintRec, * XmColumnConstraintPtr;

typedef struct _XmColumnPart {
    
    /* Public */

    unsigned char default_label_alignment;
    unsigned char default_fill_style;
    unsigned char orientation;
    unsigned char distribution;
    
    Dimension	  item_spacing;
    Dimension	  label_spacing;

    /* Private */

    Boolean	  resize_done;

} XmColumnPart;

typedef struct _XmColumnRec {
    CorePart            core;
    CompositePart       composite;
    ConstraintPart      constraint;
    XmManagerPart       manager;
    XmBulletinBoardPart bulletin_board;
    XmColumnPart        column;
} XmColumnRec;

#define XmColumnIndex (XmBulletinBoardIndex + 1)

extern XmOffsetPtr XmColumn_offsets;
extern XmOffsetPtr XmColumnC_offsets;

#define XmColCField(w,f,t) XmConstraintField(w, XmColumnC_offsets, XmColumn, f, t)
#define XmColumnC_label_alignment(w) XmColCField(w, label_alignment, unsigned char)
#define XmColumnC_label_type(w) XmColCField(w, label_type, unsigned char)
#define XmColumnC_fill_style(w) XmColCField(w, fill_style, unsigned char)
#define XmColumnC_show_label(w) XmColCField(w, show_label, Boolean)
#define XmColumnC_stretchable(w) XmColCField(w, stretchable, Boolean)
#define XmColumnC_label_pixmap(w) XmColCField(w, label_pixmap, Pixmap)
#define XmColumnC_label_string(w) XmColCField(w, label_string, XmString)
#define XmColumnC_label_font_list(w) XmColCField(w, label_font_list, XmFontList)
#define XmColumnC_label_widget(w) XmColCField(w, label_widget, Widget)
#define XmColumnC_request_width(w) XmColCField(w, request_width, Dimension)
#define XmColumnC_request_height(w) XmColCField(w, request_height, Dimension)
#define XmColumnC_position(w) XmColCField(w, position, XRectangle)

#define XmColField(w,f,t) XmField(w, XmColumn_offsets, XmColumn, f, t)
#define XmColumn_default_label_alignment(w) XmColField(w, default_label_alignment, unsigned char)
#define XmColumn_default_fill_style(w) XmColField(w, default_fill_style, unsigned char)
#define XmColumn_orientation(w) XmColField(w, orientation, unsigned char)
#define XmColumn_distribution(w) XmColField(w, distribution, unsigned char)
#define XmColumn_item_spacing(w) XmColField(w, item_spacing, Dimension)
#define XmColumn_label_spacing(w) XmColField(w, label_spacing, Dimension)
#define XmColumn_resize_done(w) XmColField(w, resize_done, Boolean)

#ifdef __cplusplus
}
#endif

#endif /* __columnp_h__ */

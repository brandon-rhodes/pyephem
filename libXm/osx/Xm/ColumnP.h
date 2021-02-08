#ifndef __Xmcolumnp_h__
#define __Xmcolumnp_h__

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

externalref XmColumnClassRec xmColumnClassRec;

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
    Boolean		check_set_render_table; /* used by CheckSetEntryLabelRenderTable */
} XmColumnConstraintPart, * XmColumnConstraint;

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
    Boolean	  check_set_render_table; /* used by CheckSetDefaultEntryLabelRenderTable */

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

#define XmColumnC_label_alignment(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.label_alignment)
#define XmColumnC_label_type(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.label_type)
#define XmColumnC_fill_style(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.fill_style)
#define XmColumnC_show_label(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.show_label)
#define XmColumnC_stretchable(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.stretchable)
#define XmColumnC_label_pixmap(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.label_pixmap)
#define XmColumnC_label_string(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.label_string)
#define XmColumnC_label_font_list(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.label_font_list)
#define XmColumnC_label_widget(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.label_widget)
#define XmColumnC_request_width(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.request_width)
#define XmColumnC_request_height(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.request_height)
#define XmColumnC_position(w) (((XmColumnConstraintPtr)((w)->core.constraints))->column.position)

#define XmColumn_default_label_alignment(w) (((XmColumnWidget)(w))->column.default_label_alignment)
#define XmColumn_default_fill_style(w) (((XmColumnWidget)(w))->column.default_fill_style)
#define XmColumn_orientation(w) (((XmColumnWidget)(w))->column.orientation)
#define XmColumn_distribution(w) (((XmColumnWidget)(w))->column.distribution)
#define XmColumn_item_spacing(w) (((XmColumnWidget)(w))->column.item_spacing)
#define XmColumn_label_spacing(w) (((XmColumnWidget)(w))->column.label_spacing)
#define XmColumn_resize_done(w) (((XmColumnWidget)(w))->column.resize_done)

#ifdef __cplusplus
}
#endif

#endif /* __columnp_h__ */

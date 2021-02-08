/*
 *    Copyright 1991, Integrated Computer Solutions, Inc.
 *
 *                     All Rights Reserved.
 *
 * AUTHOR: Scott Knowlton
 *
 */

#ifndef XmButtonBoxP_h
#define XmButtonBoxP_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
 *      INCLUDE FILES
 ************************************************************/

#include <Xm/ManagerP.h>
#include <Xm/ButtonBox.h>

/************************************************************
 *      TYPEDEFS AND DEFINES
 ************************************************************/

/************************************************************
 *      MACROS
 ************************************************************/

#define XmButtonBoxIndex (XmManagerIndex + 1)

#define XmButtonBox_equal_size(w) (((XmButtonBoxWidget)(w))->button_box.equal_size)
#define XmButtonBox_fill_option(w) (((XmButtonBoxWidget)(w))->button_box.fill_option)
#define XmButtonBox_margin_width(w) (((XmButtonBoxWidget)(w))->button_box.margin_width)
#define XmButtonBox_margin_height(w) (((XmButtonBoxWidget)(w))->button_box.margin_height)
#define XmButtonBox_spacing(w) (((XmButtonBoxWidget)(w))->button_box.spacing)
#define XmButtonBox_orientation(w) (((XmButtonBoxWidget)(w))->button_box.orientation)
#define XmButtonBox_default_button(w) (((XmButtonBoxWidget)(w))->button_box.default_button)

#define XmButtonBoxC_pref_width(w) (((XmBBoxConstraints)((Widget)(w))->core.constraints)->bbox.pref_width)
#define XmButtonBoxC_pref_height(w) (((XmBBoxConstraints)((Widget)(w))->core.constraints)->bbox.pref_height)

/************************************************************
 *      GLOBAL DECLARATIONS
 ************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmButtonBoxClassPart
{
    XtPointer extension;		/* In case its needed later */
} XmButtonBoxClassPart;

typedef struct _XmButtonBoxClassRec
{
    CoreClassPart       	core_class;
    CompositeClassPart  	composite_class;
    ConstraintClassPart         constraint_class;
    XmManagerClassPart          manager_class;
    XmButtonBoxClassPart      	buttonbox_class;

} XmButtonBoxClassRec;

typedef struct _XmBBoxConstraintsPart {
    Dimension pref_width, pref_height;
} XmBBoxConstraintsPart;
    
typedef struct _XmBBoxConstraintsRec {
    XmManagerConstraintPart	manager;
    XmBBoxConstraintsPart	bbox;
} XmBBoxConstraintsRec, *XmBBoxConstraints;

/*
 * Match XmOffset nomenclature
 */
typedef XmBBoxConstraintsPart XmButtonBoxConstraintPart;

typedef struct
{
    /* resources */

    Boolean		equal_size;
    XmFillOption	fill_option;
    Dimension		margin_width, margin_height;
    Dimension		spacing;
    unsigned char	orientation;
    Widget          default_button;
    
} XmButtonBoxPart;

typedef struct _XmButtonBoxRec
{
    CorePart        	core;
    CompositePart   	composite;
    ConstraintPart      constraint;
    XmManagerPart       manager;
    XmButtonBoxPart     button_box;

}  XmButtonBoxRec;

/************************************************************
 *       EXTERNAL DECLARATIONS
 ************************************************************/

extern XmButtonBoxClassRec xmButtonBoxClassRec;

/************************************************************
 *       STATIC DECLARATIONS
 ************************************************************/

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmButtonBoxP_h */

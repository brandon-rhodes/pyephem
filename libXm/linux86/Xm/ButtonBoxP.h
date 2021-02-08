/*
 *    Copyright 1991, Integrated Computer Solutions, Inc.
 *
 *                     All Rights Reserved.
 *
 * AUTHOR: Scott Knowlton
 *
 */

#ifndef ButtonBoxP_h
#define ButtonBoxP_h

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

extern XmOffsetPtr XmButtonBox_offsets;
extern XmOffsetPtr XmButtonBoxC_offsets;

#define BBoxField(w,f,t) XmField(w, XmButtonBox_offsets, XmButtonBox, f, t)
#define XmButtonBox_equal_size(w) BBoxField(w, equal_size, Boolean)
#define XmButtonBox_fill_option(w) BBoxField(w, fill_option, XmFillOption)
#define XmButtonBox_margin_width(w) BBoxField(w, margin_width, Dimension)
#define XmButtonBox_margin_height(w) BBoxField(w, margin_height, Dimension)
#define XmButtonBox_spacing(w) BBoxField(w, spacing, Dimension)
#define XmButtonBox_orientation(w) BBoxField(w, orientation, unsigned char)

#define BBoxCField(w,f,t) XmConstraintField(w, XmButtonBoxC_offsets, XmButtonBox, f, t)
#define XmButtonBoxC_pref_width(w) BBoxCField(w, pref_width, Dimension)
#define XmButtonBoxC_pref_height(w) BBoxCField(w, pref_height, Dimension)

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

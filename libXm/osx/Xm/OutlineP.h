/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmOutlineP_h
#define _XmOutlineP_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
*	INCLUDE FILES
*************************************************************/

#include <Xm/HierarchyP.h>
#include <Xm/Outline.h>
#include <Xm/xmlist.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

/************************************************************
*	MACROS
*************************************************************/

#define XmOutline_top_node_of_display(w) (((XmOutlineWidget)(w))->outline.top_node_of_display)
#define XmOutline_max_width(w) (((XmOutlineWidget)(w))->outline.max_width)
#define XmOutline_max_widget_width(w) (((XmOutlineWidget)(w))->outline.max_widget_width)
#define XmOutline_child_op_list(w) (((XmOutlineWidget)(w))->outline.child_op_list)
#define XmOutline_ul_point(w) (((XmOutlineWidget)(w))->outline.ul_point)
#define XmOutline_lr_point(w) (((XmOutlineWidget)(w))->outline.lr_point)
#define XmOutline_draw_gc(w) (((XmOutlineWidget)(w))->outline.draw_gc)

#define XmOutline_indent_space(w) (((XmOutlineWidget)(w))->outline.indent_space)
#define XmOutline_constrain_width(w) (((XmOutlineWidget)(w))->outline.constrain_width)
#define XmOutline_connect_nodes(w) (((XmOutlineWidget)(w))->outline.connect_nodes)



#define XmOutlineC_top_node_of_display(c) (((XmOutlineConstraintPtr)(c))->outline.top_node_of_display)
#define XmOutlineC_widget_x(c) (((XmOutlineConstraintPtr)(c))->outline.widget_x)
#define XmOutlineC_open_close_x(c) (((XmOutlineConstraintPtr)(c))->outline.open_close_x)
#define XmOutlineC_height(c) (((XmOutlineConstraintPtr)(c))->outline.height)
#define XmOutlineC_new_x(c) (((XmOutlineConstraintPtr)(c))->outline.new_x)
#define XmOutlineC_new_y(c) (((XmOutlineConstraintPtr)(c))->outline.new_y)
#define XmOutlineC_oc_new_x(c) (((XmOutlineConstraintPtr)(c))->outline.oc_new_x)
#define XmOutlineC_oc_new_y(c) (((XmOutlineConstraintPtr)(c))->outline.oc_new_y)
#define XmOutlineC_map(c) (((XmOutlineConstraintPtr)(c))->outline.map)
#define XmOutlineC_unmap(c) (((XmOutlineConstraintPtr)(c))->outline.unmap)
#define XmOutlineC_move(c) (((XmOutlineConstraintPtr)(c))->outline.move)

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

typedef void (*XmOutlineCalcLocationProc)(Widget, Boolean);
typedef int (*XmOutlineMaxWidthProc)(Widget);
    
typedef struct {
    /*Calculates the maximum width of the outline.*/
    XmOutlineMaxWidthProc calc_max_width;
    /* Calculates the locations of the objects. */
    XmOutlineCalcLocationProc calc_locations;
    /* Just in case we need it later. */
    XtPointer extension;
} OutlineClassPart;

typedef struct _XmOutlineClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    HierarchyClassPart          hierarchy_class;
    OutlineClassPart    	outline_class;
} XmOutlineClassRec;

externalref XmOutlineClassRec xmOutlineClassRec;

typedef struct _OutlineNodeInfo {
    /*
     * Public (Resource) data.
     */

    /*
     * Private data.
     */

    HierarchyConstraintRec * top_node_of_display;

    Position widget_x, open_close_x; /*location of node and open/close button*/

    Dimension height;	/* height of this row (max of node and open button). */

    Position new_x, new_y, oc_new_x, oc_new_y;
    Boolean map, unmap, move;
} OutlineNodeInfo;

typedef OutlineNodeInfo XmOutlineConstraintPart;

typedef struct _OutlineConstraintRec {
    XmManagerConstraintPart manager;
    HierNodeInfo 	hierarchy;
    OutlineNodeInfo 	outline;
} XmOutlineConstraintRec, OutlineConstraintRec, *OutlineConstraints, *XmOutlineConstraintPtr;


typedef struct _OutlinePart {
    /* Resources */
    Dimension indent_space;	/* The number of pixels to indent each level */
 
    /* Private State */

    OutlineConstraints top_node_of_display;

    Dimension max_width;	/* Width of the widest row. */
    Dimension max_widget_width;	/* Width of the widgets in the widest row. */

    XmList child_op_list;         /* List of child operations */
    XPoint ul_point, lr_point;  /* Bounding box for exposure compression */

    /* more resources */
    Boolean constrain_width;
    Boolean connect_nodes; 

    /* more private */
    GC draw_gc;

} OutlinePart;

typedef OutlinePart XmOutlinePart;

typedef struct _XmOutlineRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    HierarchyPart	hierarchy;
    OutlinePart		outline;
} XmOutlineRec;

#define XtInheritCalcMaxWidth 		((XmOutlineMaxWidthProc)_XtInherit)
#define XtInheritCalcLocations 		((XmOutlineCalcLocationProc)_XtInherit)

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/

extern XmOutlineClassRec 	xiOutlineClassRec;

/************************************************************
*	STATIC DECLARATIONS
*************************************************************/

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _OutlineP_h */

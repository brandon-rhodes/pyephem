/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _OutlineP_h
#define _Outlinels -P_h

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

#define XmOutlineIndex (XmHierarchyIndex + 1)

extern XmOffsetPtr XmOutline_offsets;
extern XmOffsetPtr XmOutlineC_offsets;

#define XmOutlineField(w,f,t) XmField(w, XmOutline_offsets, XmOutline, f, t)
#define XmOutline_top_node_of_display(w) XmOutlineField(w, top_node_of_display, OutlineConstraints)
#define XmOutline_max_width(w) XmOutlineField(w, max_width, Dimension)
#define XmOutline_max_widget_width(w) XmOutlineField(w, max_widget_width, Dimension)
#define XmOutline_child_op_list(w) XmOutlineField(w, child_op_list, XmList)
#define XmOutline_ul_point(w) XmOutlineField(w, ul_point, XPoint)
#define XmOutline_lr_point(w) XmOutlineField(w, lr_point, XPoint)
#define XmOutline_draw_gc(w) XmOutlineField(w, draw_gc, GC)

#define XmOutline_indent_space(w) XmOutlineField(w, indent_space, Dimension)
#define XmOutline_constrain_width(w) XmOutlineField(w, constrain_width, Boolean)
#define XmOutline_connect_nodes(w) XmOutlineField(w, connect_nodes, Boolean)



/*
 * WARNING!
 *
 * These macros don't use the standard fieldmacro(widget) form.  They take
 * _pointers to OutlineConstraintsRec structures_.  Be careful.
 */
#define XmOutlineCField(constraints, variable, type) \
        (*(type *)(((char *) constraints) + \
        XmOutlineC_offsets[XmOutlineIndex] + \
        XtOffsetOf(XmOutlineConstraintPart, variable)))

#define XmOutlineC_top_node_of_display(c) XmOutlineCField(c, top_node_of_display, HierarchyConstraintRec*)
#define XmOutlineC_widget_x(c) XmOutlineCField(c, widget_x, Position)
#define XmOutlineC_open_close_x(c) XmOutlineCField(c, open_close_x, Position)
#define XmOutlineC_height(c) XmOutlineCField(c, height, Dimension)
#define XmOutlineC_new_x(c) XmOutlineCField(c, new_x, Position)
#define XmOutlineC_new_y(c) XmOutlineCField(c, new_y, Position)
#define XmOutlineC_oc_new_x(c) XmOutlineCField(c, oc_new_x, Position)
#define XmOutlineC_oc_new_y(c) XmOutlineCField(c, oc_new_y, Position)
#define XmOutlineC_map(c) XmOutlineCField(c, map, Boolean)
#define XmOutlineC_unmap(c) XmOutlineCField(c, unmap, Boolean)
#define XmOutlineC_move(c) XmOutlineCField(c, move, Boolean)

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
} OutlineConstraintRec, *OutlineConstraints;


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

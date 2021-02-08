/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _TreeP_h
#define _TreeP_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
*	INCLUDE FILES
*************************************************************/

#include <Xm/HierarchyP.h>
#include <Xm/Tree.h>
#include <Xm/xmlist.h>

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/************************************************************
*	MACROS
*************************************************************/

#define XmTreeIndex (XmHierarchyIndex + 1)

extern XmOffsetPtr XmTree_offsets;
extern XmOffsetPtr XmTreeC_offsets;

#define XmTreeField(w,f,t) XmField(w, XmTree_offsets, XmTree, f, t)

#define XmTree_h_node_space(w) XmTreeField(w, h_node_space, Dimension)
#define XmTree_v_node_space(w) XmTreeField(w, v_node_space, Dimension)
#define XmTree_connect_style(w) XmTreeField(w, connect_style, XmTreeConnectStyle)
#define XmTree_max_width(w) XmTreeField(w, max_width, Dimension)
#define XmTree_max_height(w) XmTreeField(w, max_height, Dimension)
#define XmTree_child_op_list(w) XmTreeField(w, child_op_list, XmList)
#define XmTree_ul_point(w) XmTreeField(w, ul_point, XPoint)
#define XmTree_lr_point(w) XmTreeField(w, lr_point, XPoint)
#define XmTree_orientation(w) XmTreeField(w, orientation, unsigned char)
#define XmTree_compress_style(w) XmTreeField(w, compress_style, XmTreeCompressStyle)
#define XmTree_vertical_delta(w) XmTreeField(w, vertical_delta, Dimension)
#define XmTree_horizontal_delta(w) XmTreeField(w, horizontal_delta, Dimension)

/*
 * WARNING!
 *
 * These macros don't use the standard fieldmacro(widget) form.  They take
 * _pointers to TreeConstraints structures_.  Be careful.
 */
#define XmTreeCField(constraints, variable, type) \
        (*(type *)(((char *) constraints) + \
        XmTreeC_offsets[XmTreeIndex] + \
        XtOffsetOf(XmTreeConstraintPart, variable)))

#define XmTreeC_open_close_padding(c) XmTreeCField(c, open_close_padding, int)
#define XmTreeC_box_x(c) XmTreeCField(c, box_x, Position)
#define XmTreeC_box_y(c) XmTreeCField(c, box_y, Position)
#define XmTreeC_bb_width(c) XmTreeCField(c, bb_width, Dimension)
#define XmTreeC_bb_height(c) XmTreeCField(c, bb_height, Dimension)
#define XmTreeC_widget_offset(c) XmTreeCField(c, widget_offset, Dimension)
#define XmTreeC_placed(c) XmTreeCField(c, placed, Boolean)
#define XmTreeC_color(c) XmTreeCField(c, color, Pixel)
#define XmTreeC_background_color(c) XmTreeCField(c, background_color, Pixel)
#define XmTreeC_line_width(c) XmTreeCField(c, line_width, int)
#define XmTreeC_line_style(c) XmTreeCField(c, line_style, int)
#define XmTreeC_gc(c) XmTreeCField(c, gc, GC)
#define XmTreeC_new_x(c) XmTreeCField(c, new_x, Position)
#define XmTreeC_new_y(c) XmTreeCField(c, new_y, Position)
#define XmTreeC_oc_new_x(c) XmTreeCField(c, oc_new_x, Position)
#define XmTreeC_oc_new_y(c) XmTreeCField(c, oc_new_y, Position)
#define XmTreeC_map(c) XmTreeCField(c, map, Boolean)
#define XmTreeC_unmap(c) XmTreeCField(c, unmap, Boolean)
#define XmTreeC_move(c) XmTreeCField(c, move, Boolean)
#define XmTreeC_is_compressed(c) XmTreeCField(c, is_compressed, Boolean)

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

typedef struct {
    XtPointer extension;	/* Just in case we need it later. */
} TreeClassPart;

typedef struct _XmTreeClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    HierarchyClassPart          hierarchy_class;
    TreeClassPart    		tree_class;
} XmTreeClassRec;

typedef struct _TreeNodeInfo {
    /*
     * Public (Resource) data.
     */

    int open_close_padding;	/* Number of pixels to leave between o/c button
				   and node button (can be negative). */

    /*
     * Private data.
     */

    Position box_x, box_y;
    Dimension bb_width, bb_height; /*Bounding box for myself and my children*/
    Dimension widget_offset;	/* Amount of space to leave for the open
				   close button to the left of the node.*/
    Boolean placed;

    Pixel color;		/* color to draw line in. */
    int line_width;

    GC gc;

    Position new_x, new_y, oc_new_x, oc_new_y;
    Boolean map, unmap, move;
    Boolean is_compressed;  /* for space compression, is this node moved? */

    /* more resources */
    int line_style;
    Pixel background_color;		/* color to draw line in. */
} TreeNodeInfo;

typedef struct _TreeConstraintRec {
    XmManagerConstraintPart manager;
    HierNodeInfo 	hierarchy;
    TreeNodeInfo 	tree;
} TreeConstraintRec, *TreeConstraints;


typedef struct _TreePart {
    /* Resources */

    Dimension h_node_space, v_node_space; /* Space between various nodes. */

    XmTreeConnectStyle connect_style; /* The connection style. */


    /* Private State */

    Dimension max_width;	/* Our new desired width. */
    Dimension max_height;	/* Our new desired height. */
    
    XmList child_op_list;		/* List of child operations */
    XPoint ul_point, lr_point;	/* Bounding box for exposure compression. */

    /* more resources */
    unsigned char orientation;     /* XmHORIZONTAL or XmVERTICAL */

    XmTreeCompressStyle compress_style;  /* how to do space compression */

    Dimension vertical_delta; /* if doing space compression, how many pixels */
                              /* to offset alternating siblings vertically */
    Dimension horizontal_delta;  /* or horizontally */
} TreePart;

typedef struct _XmTreeRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    HierarchyPart	hierarchy;
    TreePart		tree;
} XmTreeRec;

/*
 * Typedefs to conform to the XmField macro's naming convention
 */
typedef TreePart XmTreePart;
typedef TreeNodeInfo XmTreeConstraintPart;

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/

extern XmTreeClassRec 	xmTreeClassRec;

/************************************************************
*	STATIC DECLARATIONS
*************************************************************/

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _TreeP_h */







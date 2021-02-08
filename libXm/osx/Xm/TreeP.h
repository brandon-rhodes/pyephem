/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmTreeP_h_
#define _XmTreeP_h_


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

#define XmTree_h_node_space(w) (((XmTreeWidget)(w))->tree.h_node_space)
#define XmTree_v_node_space(w) (((XmTreeWidget)(w))->tree.v_node_space)
#define XmTree_connect_style(w) (((XmTreeWidget)(w))->tree.connect_style)
#define XmTree_max_width(w) (((XmTreeWidget)(w))->tree.max_width)
#define XmTree_max_height(w) (((XmTreeWidget)(w))->tree.max_height)
#define XmTree_child_op_list(w) (((XmTreeWidget)(w))->tree.child_op_list)
#define XmTree_ul_point(w) (((XmTreeWidget)(w))->tree.ul_point)
#define XmTree_lr_point(w) (((XmTreeWidget)(w))->tree.lr_point)
#define XmTree_orientation(w) (((XmTreeWidget)(w))->tree.orientation)
#define XmTree_compress_style(w) (((XmTreeWidget)(w))->tree.compress_style)
#define XmTree_vertical_delta(w) (((XmTreeWidget)(w))->tree.vertical_delta)
#define XmTree_horizontal_delta(w) (((XmTreeWidget)(w))->tree.horizontal_delta)

#define XmTreeC_open_close_padding(c) (((XmTreeConstraintPtr)(c))->tree.open_close_padding)
#define XmTreeC_box_x(c) (((XmTreeConstraintPtr)(c))->tree.box_x)
#define XmTreeC_box_y(c) (((XmTreeConstraintPtr)(c))->tree.box_y)
#define XmTreeC_bb_width(c) (((XmTreeConstraintPtr)(c))->tree.bb_width)
#define XmTreeC_bb_height(c) (((XmTreeConstraintPtr)(c))->tree.bb_height)
#define XmTreeC_widget_offset(c) (((XmTreeConstraintPtr)(c))->tree.widget_offset)
#define XmTreeC_placed(c) (((XmTreeConstraintPtr)(c))->tree.placed)
#define XmTreeC_color(c) (((XmTreeConstraintPtr)(c))->tree.color)
#define XmTreeC_background_color(c) (((XmTreeConstraintPtr)(c))->tree.background_color)
#define XmTreeC_line_width(c) (((XmTreeConstraintPtr)(c))->tree.line_width)
#define XmTreeC_line_style(c) (((XmTreeConstraintPtr)(c))->tree.line_style)
#define XmTreeC_gc(c) (((XmTreeConstraintPtr)(c))->tree.gc)
#define XmTreeC_new_x(c) (((XmTreeConstraintPtr)(c))->tree.new_x)
#define XmTreeC_new_y(c) (((XmTreeConstraintPtr)(c))->tree.new_y)
#define XmTreeC_oc_new_x(c) (((XmTreeConstraintPtr)(c))->tree.oc_new_x)
#define XmTreeC_oc_new_y(c) (((XmTreeConstraintPtr)(c))->tree.oc_new_y)
#define XmTreeC_map(c) (((XmTreeConstraintPtr)(c))->tree.map)
#define XmTreeC_unmap(c) (((XmTreeConstraintPtr)(c))->tree.unmap)
#define XmTreeC_move(c) (((XmTreeConstraintPtr)(c))->tree.move)
#define XmTreeC_is_compressed(c) (((XmTreeConstraintPtr)(c))->tree.is_compressed)

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
externalref  XmTreeClassRec xmTreeClassRec;

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
} XmTreeConstraintRec, TreeConstraintRec, *TreeConstraints, *XmTreeConstraintPtr;


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







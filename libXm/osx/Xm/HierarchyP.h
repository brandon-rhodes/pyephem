/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmHierarchyP_h
#define _XmHierarchyP_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
*	INCLUDE FILES
*************************************************************/
#include <Xm/ManagerP.h>
#include <Xm/ExtP.h>
#include <Xm/Hierarchy.h>

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#define IS_MAPPED	(1L << 1)
#define IS_COMPRESSED   (1L << 2)
#define PARENT_GONE	(1L << 3)
#define IS_SELECTED	(1L << 4)

/************************************************************
*	MACROS
*************************************************************/

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

typedef struct _HierNodeInfo {
    /*
     * Public (Resource) data.
     */

    XmHierarchyNodeState state;	/* State of the node. */
    Widget parent;		/* parent of this child. */
    Widget insert_before;	/* Sibling to insert this node before;
				   NULL will place it at the end. */
    Pixmap open_folder, close_folder; /* Images for open/close buttons. */

    /*
     * Private data.
     */

    Widget widget;		/* Back pointer to this node's widget. */
    Widget open_close_button;	/* The open or close button. */

    struct _HierarchyConstraintRec ** children;       /* norm children. */
    Cardinal num_children;	/* number of each type of children. */
    Cardinal alloc_attrs, alloc;   /* amount of allocated space for each */

    unsigned char status;	/* 8 status bits. */
} HierNodeInfo;

typedef struct _HierarchyConstraintRec {
    XmManagerConstraintPart manager;
    HierNodeInfo hierarchy;
} HierarchyConstraintRec, XmHierarchyConstraintRec, *HierarchyConstraints;

typedef void	(*XmHierarchyNodeProc)(HierarchyConstraints);
typedef void	(*XmHierarchyExtraNodeProc)(Widget, HierarchyConstraints);
typedef void	(*XmHierarchyBuildTableProc)(Widget, 
					     HierarchyConstraints, Cardinal *);
typedef void	(*XmHierarchyResetButtonProc)(Widget, HierarchyConstraints);
    
typedef struct {
    /* Class function for changing node state. */
    XmHierarchyNodeProc		change_node_state;
    /* map or unmap a given node. */
    XmHierarchyNodeProc 	map_node;
    XmHierarchyNodeProc		unmap_node;
    /* Unmaps all the extra nodes. */
    XmHierarchyExtraNodeProc	unmap_all_extra_nodes;
    /* Builds the node table. */
    XmHierarchyBuildTableProc	build_node_table;
    /* Correctly sets the state of the open/close button. */
    XmHierarchyResetButtonProc	reset_open_close_button;
    /* Toggles state of a node. */
    XtCallbackProc 		toggle_node_state;
    /* Just in case we need it later. */
    XtPointer 			extension;	
} HierarchyClassPart;

typedef struct _XmHierarchyClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    HierarchyClassPart          hierarchy_class;
} XmHierarchyClassRec;

externalref XmHierarchyClassRec xmHierarchyClassRec;

typedef struct _HierarchyPart {
    /* resources */

    Boolean auto_close;		/* Auto-close children when parent is closed?*/
    Boolean refigure_mode;	/* Do refigures? */

    Dimension h_margin;		/* The horizontal margin. */
    Dimension v_margin;		/* The vertical margin. */
    Pixmap open_folder, close_folder; /* Images for open/close buttons. */

    XtCallbackList node_state_callback;	/* Called when the node button is
					   clicked */
    XtCallbackList node_state_changed_callback;	/* Called when the node state
						   changes */
    XtCallbackList node_state_beg_end_callback; /* Called when beginning
						     or ending a set of node
						     state changes */

    /* private state */

    HierarchyConstraintRec ** node_table;
    HierarchyConstraints top_node; 
    Cardinal num_nodes, alloc_nodes;
    Pixmap def_open_folder, def_close_folder; /* Default folder button Images*/

    XtWorkProcId work_proc_id;  /* work proc id for the move nodes wp */
} HierarchyPart;

typedef struct _XmHierarchyRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    HierarchyPart	hierarchy;
} XmHierarchyRec;

/*
 * These are necessary because the XmResolvePartOffset macros assume a
 * certain naming convention
 */
typedef HierarchyPart XmHierarchyPart;
typedef HierNodeInfo  XmHierarchyConstraintPart;

#define XmHierarchy_auto_close(w) (((XmHierarchyWidget)(w))->hierarchy.auto_close)
#define XmHierarchy_refigure_mode(w) (((XmHierarchyWidget)(w))->hierarchy.refigure_mode)
#define XmHierarchy_h_margin(w) (((XmHierarchyWidget)(w))->hierarchy.h_margin)
#define XmHierarchy_v_margin(w) (((XmHierarchyWidget)(w))->hierarchy.v_margin)
#define XmHierarchy_open_folder(w) (((XmHierarchyWidget)(w))->hierarchy.open_folder)
#define XmHierarchy_close_folder(w) (((XmHierarchyWidget)(w))->hierarchy.close_folder)
#define XmHierarchy_node_state_callback(w) (((XmHierarchyWidget)(w))->hierarchy.node_state_callback)
#define XmHierarchy_node_state_changed_callback(w) (((XmHierarchyWidget)(w))->hierarchy.node_state_changed_callback)
#define XmHierarchy_node_state_beg_end_callback(w) (((XmHierarchyWidget)(w))->hierarchy.node_state_beg_end_callback)
#define XmHierarchy_node_table(w) (((XmHierarchyWidget)(w))->hierarchy.node_table)
#define XmHierarchy_top_node(w) (((XmHierarchyWidget)(w))->hierarchy.top_node)
#define XmHierarchy_num_nodes(w) (((XmHierarchyWidget)(w))->hierarchy.num_nodes)
#define XmHierarchy_alloc_nodes(w) (((XmHierarchyWidget)(w))->hierarchy.alloc_nodes)
#define XmHierarchy_def_open_folder(w) (((XmHierarchyWidget)(w))->hierarchy.def_open_folder)
#define XmHierarchy_def_close_folder(w) (((XmHierarchyWidget)(w))->hierarchy.def_close_folder)
#define XmHierarchy_work_proc_id(w) (((XmHierarchyWidget)(w))->hierarchy.work_proc_id)

#define XmHierarchyC_state(constraints) ((constraints)->hierarchy.state)
#define XmHierarchyC_parent(constraints) ((constraints)->hierarchy.parent)
#define XmHierarchyC_insert_before(constraints) ((constraints)->hierarchy.insert_before)
#define XmHierarchyC_open_folder(constraints) ((constraints)->hierarchy.open_folder)
#define XmHierarchyC_close_folder(constraints) ((constraints)->hierarchy.close_folder)
#define XmHierarchyC_widget(constraints) ((constraints)->hierarchy.widget)
#define XmHierarchyC_open_close_button(constraints) ((constraints)->hierarchy.open_close_button)
#define XmHierarchyC_children(constraints) ((constraints)->hierarchy.children)
#define XmHierarchyC_num_children(constraints) ((constraints)->hierarchy.num_children)
#define XmHierarchyC_alloc_attrs(constraints) ((constraints)->hierarchy.alloc_attrs)
#define XmHierarchyC_alloc(constraints) ((constraints)->hierarchy.alloc)
#define XmHierarchyC_status(constraints) ((constraints)->hierarchy.status)

#define XtInheritChangeNodeState       ((XmHierarchyNodeProc)_XtInherit)
#define XtInheritUnmapAllExtraNodes    ((XmHierarchyExtraNodeProc)_XtInherit)
#define XtInheritUnmapNode 	       ((XmHierarchyNodeProc)_XtInherit)
#define XtInheritMapNode 	       ((XmHierarchyNodeProc)_XtInherit)
#define XtInheritBuildNodeTable        ((XmHierarchyBuildTableProc)_XtInherit)
#define XtInheritResetOpenCloseButton  ((XmHierarchyResetButtonProc)_XtInherit)
#define XtInheritToggleNodeState       ((XtCallbackProc)_XtInherit)

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/


/************************************************************
*	STATIC DECLARATIONS
*************************************************************/

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _HierarchyP_h */

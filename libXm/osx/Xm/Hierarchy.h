/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmHierarchy_h
#define _XmHierarchy_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Ext.h>

/************************************************************
*	INCLUDE FILES
*************************************************************/

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _XmHierarchyClassRec	*XmHierarchyWidgetClass;
typedef struct _XmHierarchyRec		*XmHierarchyWidget;

typedef struct _XmHierarchyNodeStateData {
    Widget widget;
    XmHierarchyNodeState state;
} XmHierarchyNodeStateData;

/************************************************************
*	MACROS
*************************************************************/

/************************************************************
*	GLOBAL FUNCTION DECLARATIONS
*************************************************************/

/*	Function Name: XmHierarchyOpenAllAncestors
 *	Description: This function opens all ancestors of the given node.
 *	Arguments: nw - the node (widget) that will be changed.
 *	Returns: none
 */

void XmHierarchyOpenAllAncestors(Widget);

WidgetList XmHierarchyGetChildNodes(Widget);

extern WidgetClass xmHierarchyWidgetClass;

#if defined(__cplusplus)
}
#endif

#endif /* _Hierarchy_h */

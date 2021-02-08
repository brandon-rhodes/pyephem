/*
 *    Copyright 1990, 1992 -- Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmIconBoxP_h
#define _XmIconBoxP_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
*	INCLUDE FILES
*************************************************************/
#include <Xm/ManagerP.h>
#include <Xm/IconBox.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/


/*
 * Hack to get around naming conventions.  The XmConstraintPartOffset macro
 * requires our contraint part structure to have this name
 */
#define XmIconBoxConstraintPart IconInfo

#define XmIconBoxIndex (XmManagerIndex + 1)

extern XmOffsetPtr XmIconBox_offsets;   /* instance offsets */
extern XmOffsetPtr XmIconBoxC_offsets;  /* contraint offsets */

/*
 * Access macros for widget instance fields
 */
#define XmIconBoxField(w,f,t) XmField(w, XmIconBox_offsets, XmIconBox, f, t)

#define XmIconBox_min_v_cells(w)	XmIconBoxField(w, min_v_cells, Dimension)
#define XmIconBox_min_h_cells(w)	XmIconBoxField(w, min_h_cells, Dimension)
#define XmIconBox_v_margin(w)	XmIconBoxField(w, v_margin, Dimension)
#define XmIconBox_h_margin(w)	XmIconBoxField(w, h_margin, Dimension)
#define XmIconBox_min_cell_width(w)  XmIconBoxField(w, min_cell_width, Dimension)
#define XmIconBox_min_cell_height(w) XmIconBoxField(w, min_cell_height, Dimension)
#define XmIconBox_cell_width(w)	XmIconBoxField(w, cell_width, Dimension)
#define XmIconBox_cell_height(w)	XmIconBoxField(w, cell_height, Dimension)

/*
 * Access macros for widget constraint fields
 */
#define XmIconBoxCField(w,f,t) \
             XmConstraintField(w, XmIconBoxC_offsets, XmIconBox, f, t)

#define XmIconBoxC_cell_x(w)	XmIconBoxCField(w, cell_x, short)
#define XmIconBoxC_cell_y(w)	XmIconBoxCField(w, cell_y, short)
#define XmIconBoxC_pref_width(w)	XmIconBoxCField(w, pref_width, Dimension)
#define XmIconBoxC_pref_height(w)	XmIconBoxCField(w, pref_height, Dimension)

/************************************************************
*	MACROS
*************************************************************/

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

typedef struct {
    XtPointer extension;	/* Just in case we need it later. */
} XmIconBoxClassPart;

typedef struct _XmIconBoxClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    XmIconBoxClassPart	        box_class;
} XmIconBoxClassRec;

typedef struct {
    /* resources */

    Dimension min_v_cells;	/* Default number of cells in the vert dir. */
    Dimension min_h_cells;	/* Default number of cells in the horiz dir. */
    Dimension v_margin;		/* Amount of space to leave between cells */
    Dimension h_margin;		/* and window edges. */
    Dimension min_cell_width;	/* Minimum width of the cells. */
    Dimension min_cell_height;	/* Minimum height of the cells. */

    /* private state */

    Dimension cell_width;	/* Width and height of all cells. */
    Dimension cell_height;

} XmIconBoxPart;


typedef struct _XmIconBoxRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    XmIconBoxPart	box;
} XmIconBoxRec;

typedef struct _IconInfo {

    /*
     * Resources.
     */

    short cell_x;		/* X location of this icon in cell space. */
    short cell_y;		/* Y location of this icon in cell space. */

    /*
     * Private state.
     */

    Dimension pref_width, pref_height; /* The preferred size of this widget. */
} IconInfo;

typedef struct _XmIconBoxConstraintsRec {
    XmManagerConstraintPart	manager;
    IconInfo			icon;
} XmIconBoxConstraintsRec, *XmIconBoxConstraints;

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/

extern XmIconBoxClassRec xmIconBoxClassRec;

/************************************************************
*	STATIC DECLARATIONS
*************************************************************/

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmIconBoxP_h */

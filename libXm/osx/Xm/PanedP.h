/***********************************************************

Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
 * PanedP.h - Paned Composite Widget's private header file.
 *
 * Updated and significantly modified from the Athena VPaned Widget.
 *
 * Date:    March 1, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium
 *          kit@expo.lcs.mit.edu
 */

/*
 *    Copyright 1992, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Tony Auito, Chris D. Peterson
 */

#ifndef _XmPanedP_h
#define _XmPanedP_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/ManagerP.h>
#include <Xm/Paned.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 *
 * Paned Widget Private Data
 *
 *********************************************************************/

/* New fields for the Paned widget class record */

typedef struct _XmPanedClassPart {
    XtPointer extension;	
} XmPanedClassPart;

/* Full Class record declaration */
typedef struct _XmPanedClassRec {
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart	manager_class;
    XmPanedClassPart     paned_class;
} XmPanedClassRec;

extern XmPanedClassRec xmPanedClassRec;

/* Paned constraint record */
typedef struct _XmPanedConstraintsPart {
  /* Resources. */
    Dimension	min;		/* Minimum height */
    Dimension	max;		/* Maximum height */
    Boolean	allow_resize;	/* TRUE iff child resize requests are ok */
    Boolean     show_sash;	/* TRUE iff child will have sash below it,
				   when it is not the bottom pane. */
    Boolean	skip_adjust;	/* TRUE iff child's height should not be */
				/* changed without explicit user action. */
    int		position;	/* position location in Paned (relative to
				   other children) ** NIY ** */
    Dimension   preferred_size;	/* The Preferred size of the pane.
				   Iff this is zero then ask child for size.*/
    Boolean     resize_to_pref;	/* resize this pane to its preferred size
				   on a resize or change managed after 
				   realize. */
    Boolean     is_a_pane;	/* INTERNAL INFO */


  /* Private state. */
    Position	delta;		/* Desired Location */
    Position	olddelta;	/* The last value of delta. */
    Dimension	wp_size;	/* widget's preferred on size */ 
    Dimension	wp_off_size;	/* widget's preferred off size */ 
    int         size;		/* the size the widget will actually get. */
    Widget	sash;		/* The sash for this child */
    Widget      separator;      /* The separator for this child */

    Boolean     prefs_inited;	/* Preferences have been inited... */
} XmPanedConstraintsPart, *Pane;

typedef struct _XmPanedConstraintsRec {
    XmManagerConstraintPart	manager;
    XmPanedConstraintsPart	paned;
} XmPanedConstraintsRec, *XmPanedConstraints;

/*
 * Ugliness:  the XmOffset macros require this naming convention, yet
 * epak already depends on the pluralized name ;(
 */
typedef XmPanedConstraintsPart XmPanedConstraintPart;

/*
 * The Pane Stack Structure.
 */

typedef struct _PaneStack {
    struct _PaneStack * next;	/* The next element on the stack. */
    Pane pane;			/* The pane in this element on the stack. */
    int start_size;		/* The size of this element when it was pushed
				   onto the stack. */
} PaneStack;

#define NO_ADJUST    ((char) 0)
#define BEGAN_ADJUST ((char) 1)

/* New Fields for the XmPaned widget record */
typedef struct {
    /* resources */
    Position    sash_indent;               /* Location of sashs (per motif) */
    Boolean     refiguremode;              /* Whether to refigure changes 
					      right now */
    XtTranslations sash_translations;      /* sash translation table */
    Dimension   internal_bw;	           /* internal border width. */
    unsigned char orientation;	           /* Orientation of paned widget. */

    Cursor	cursor;		/* Cursor for paned window */

    /* Things from Motif behaviour */
    Boolean     separator_on;        /* make separator visible */
    Dimension   margin_width;        /* space between right and left edges of
                                        Paned window and it's children */
    Dimension   margin_height;       /* space between top and bottom edges of
                                        Paned window and it's children */

    /* sash modifying resources */
    Dimension   sash_width;            /* Modify sash width */
    Dimension   sash_height;           /* Modify sash height */
    Dimension   sash_shadow_thickness; /* Modify sash shadow_thickness */

    /* Private */
    Boolean	recursively_called;        /* for ChangeManaged */
    Boolean	resize_children_to_pref;   /* override constraint resources
					      and resize all children to
					      preferred size. */
    short       increment_count;           /* Sash increment count */
    char	repane_status;		   /* current adjust state. */
    Position    start_loc;	           /* mouse origin when adjusting */
    GC          flipgc;                    /* GC to use when animating
					      borders */
    short	num_panes;                 /* count of managed panes */
    short       num_slots;            	   /*number of avail. slots for kids */

    PaneStack * stack;		           /* The pane stack for this widget.*/
    WidgetList  managed_children;	   /* keep track of managed children */
 
    Boolean     allow_unused_space;      /* should the paned widget allow
                                          * a pane to be shrunk to the point
                                          * that there is unused space at
                                          * the bottom/right of the widget */
} XmPanedPart;

/**************************************************************************
 *
 * Full instance record declaration
 *
 **************************************************************************/

typedef struct _XmPanedRec {
    CorePart       core;
    CompositePart  composite;
    ConstraintPart constraint;
    XmManagerPart  manager;
    XmPanedPart     paned;
} XmPanedRec;

/************************************************************
 *
 *  Functions
 *
 ************************************************************/
void _XmFromPanedPixels(Widget, int, XtArgVal *);
XmImportOperator _XmToPanedPixels(Widget, int, XtArgVal *);


/************************************************************
 *
 *  Macros
 *
 ************************************************************/
/*
 * XmResolvePartOffsets stuff follows
 */

#define XmPaned_sash_indent(w) (((XmPanedWidget)(w))->paned.sash_indent)
#define XmPaned_refiguremode(w) (((XmPanedWidget)(w))->paned.refiguremode)
#define XmPaned_sash_translations(w) (((XmPanedWidget)(w))->paned.sash_translations)
#define XmPaned_internal_bw(w) (((XmPanedWidget)(w))->paned.internal_bw)
#define XmPaned_orientation(w) (((XmPanedWidget)(w))->paned.orientation)
#define XmPaned_cursor(w) (((XmPanedWidget)(w))->paned.cursor)
#define XmPaned_separator_on(w) (((XmPanedWidget)(w))->paned.separator_on)
#define XmPaned_margin_width(w) (((XmPanedWidget)(w))->paned.margin_width)
#define XmPaned_margin_height(w) (((XmPanedWidget)(w))->paned.margin_height)
#define XmPaned_sash_width(w) (((XmPanedWidget)(w))->paned.sash_width)
#define XmPaned_sash_height(w) (((XmPanedWidget)(w))->paned.sash_height)
#define XmPaned_sash_shadow_thickness(w) (((XmPanedWidget)(w))->paned.sash_shadow_thickness)
#define XmPaned_recursively_called(w) (((XmPanedWidget)(w))->paned.recursively_called)
#define XmPaned_resize_children_to_pref(w) (((XmPanedWidget)(w))->paned.resize_children_to_pref)
#define XmPaned_increment_count(w) (((XmPanedWidget)(w))->paned.increment_count)
#define XmPaned_repane_status(w) (((XmPanedWidget)(w))->paned.repane_status)
#define XmPaned_start_loc(w) (((XmPanedWidget)(w))->paned.start_loc)
#define XmPaned_flipgc(w) (((XmPanedWidget)(w))->paned.flipgc)
#define XmPaned_num_panes(w) (((XmPanedWidget)(w))->paned.num_panes)
#define XmPaned_num_slots(w) (((XmPanedWidget)(w))->paned.num_slots)
#define XmPaned_stack(w) (((XmPanedWidget)(w))->paned.stack)
#define XmPaned_managed_children(w) (((XmPanedWidget)(w))->paned.managed_children)
#define XmPaned_allow_unused_space(w) (((XmPanedWidget)(w))->paned.allow_unused_space)

#define XmPanedC_min(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.min)
#define XmPanedC_max(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.max)
#define XmPanedC_allow_resize(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.allow_resize)
#define XmPanedC_show_sash(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.show_sash)
#define XmPanedC_skip_adjust(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.skip_adjust)
#define XmPanedC_position(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.position)
#define XmPanedC_preferred_size(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.preferred_size)
#define XmPanedC_resize_to_pref(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.resize_to_pref)
#define XmPanedC_is_a_pane(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.is_a_pane)
#define XmPanedC_delta(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.delta)
#define XmPanedC_olddelta(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.olddelta)
#define XmPanedC_wp_size(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.wp_size)
#define XmPanedC_wp_off_size(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.wp_off_size)
#define XmPanedC_size(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.size)
#define XmPanedC_sash(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.sash)
#define XmPanedC_separator(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.separator)
#define XmPanedC_prefs_inited(w) (((XmPanedConstraintsRec*)(w)->core.constraints)->paned.prefs_inited)

#define ForceSashOff(pane) ((pane)->min == (pane)->max)
#define PaneConsRec(w)	((XmPanedConstraints)(w)->core.constraints)
#define PaneInfo(w) (&(((XmPanedConstraintsRec*)((w)->core.constraints))->paned))
#define HasSash(w)	(XmPanedC_sash(w) != NULL)
#define HasSep(w)	(XmPanedC_separator(w) != NULL)

#define PaneIndex(w)	(XmPanedC_position(w))
#define IsVert(w)       (XmPaned_orientation(w) == XmVERTICAL)

#define IsLastPane(pw, childP) ((XmPaned_managed_children((pw)) + \
				 XmPaned_num_panes((pw)) - 1) == childP)

#define ForAllPaned(pw, childP) \
  for ( ((childP) = (XmPaned_managed_children((pw)))) ; \
	((childP) < ((XmPaned_managed_children((pw))) \
                    + (XmPaned_num_panes((pw))))) ; \
	(childP)++ )

#define NthPane(pw, paneIndex) (XmPaned_managed_children((pw)) + (paneIndex))

#ifdef _cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmPanedP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

/* $XConsortium: NotebookP.h /main/4 1995/07/15 20:53:46 drk $ */
/*
 *  @OPENGROUP_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 *  
 *  This software is subject to an open license. It may only be
 *  used on, with or for operating systems which are themselves open
 *  source systems. You must contact The Open Group for a license
 *  allowing distribution and sublicensing of this software on, with,
 *  or for operating systems which are not Open Source programs.
 *  
 *  See http://www.opengroup.org/openmotif/license for full
 *  details of the license agreement. Any use, reproduction, or
 *  distribution of the program constitutes recipient's acceptance of
 *  this agreement.
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 *  PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 *  WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 *  OR FITNESS FOR A PARTICULAR PURPOSE
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 *  NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 *  EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGES.
 */
/*
 * HISTORY
 */
#ifndef _XmNotebookP_h
#define _XmNotebookP_h

#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/ScrollFrameT.h>
#include <Xm/Notebook.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Notebook's constraint info. fields */
typedef struct _XmNotebookConstraintPart
{
    /* resources */
    int page_number;			/* page number */
    unsigned char child_type;		/* notebook child type */
    Boolean resizable;			/* is this child resizable? */

    /* private variables */
    Boolean active;			/* True if the child is active */
} XmNotebookConstraintPart, *XmNotebookConstraint;

typedef struct _XmNotebookConstraintRec
{
    XmManagerConstraintPart manager;
    XmNotebookConstraintPart notebook;
} XmNotebookConstraintRec, *XmNotebookConstraintPtr;


/* New fields for the Notebook widget class record */
typedef struct _XmNotebookClassPart
{
    XtPointer extension;
} XmNotebookClassPart;


/* Full class record declaration */
typedef struct _NotebookClassRec
{
    CoreClassPart core_class;
    CompositeClassPart composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart manager_class;
    XmNotebookClassPart	notebook_class;
} XmNotebookClassRec;

externalref XmNotebookClassRec xmNotebookClassRec;


/* New fields for the Notebook widget instance record */
typedef struct _XmNotebookPart
{
    /* resources */
    int current_page_number;		/* the current page number */
    int first_page_number;		/* the first page number */
    int last_page_number;		/* the last page number */
    unsigned char orientation;		/* notebook orientation */
    unsigned char back_page_pos;	/* position of back pages */
    Cardinal back_page_number;		/* the of back page lines */
    Dimension back_page_size;		/* the thickness of back pages */
    Pixel back_page_foreground;         /* foreground color for bk pgs */
    Pixel back_page_background;         /* background color for bk pgs */
    Pixel frame_background;		/* background color for frame */
    unsigned char binding_type;		/* binding type */
    Pixmap binding_pixmap;		/* pixmap for the binding */
    Pixmap spiral_pixmap;		/* pixmap for the spiral binding */
    Dimension binding_width;		/* the width of the binding */
    Dimension margin_width;		/* horizontal margin between widgets */
    Dimension margin_height;		/* vertical margin between widgets */
    Dimension major_spacing;		/* gap between major tabs */
    Dimension minor_spacing;		/* gap between minor tabs */
    Dimension shadow_thickness;		/* notebook frame shadow thickness */
    XtCallbackList page_change_callback;/* the page change callback */

    /* child widgets */
    Widget scroller;			/* the page scroller widget */
    Widget scroller_child;		/* TextF child of def page scroller */
    Widget next_major;			/* next major tab scroll button */
    Widget prev_major;			/* prev major tab scroll button */
    Widget next_minor;			/* next minor tab scroll button */
    Widget prev_minor;			/* prev minor tab scroll button */

    /* preferred children sizes */
    Dimension real_binding_width;	/* real binding width */
    Dimension real_back_page_number;	/* real back page number */
    Dimension page_width;		/* width of page widgets */
    Dimension page_height;		/* height of page widgets */
    Dimension status_width;		/* width of the status areas */
    Dimension status_height;		/* height of the status  areas */
    Dimension major_width;		/* width of major tabs */
    Dimension major_height;		/* height of major tabs */
    Dimension minor_width;		/* width of minor tabs */
    Dimension minor_height;		/* height of minor tabs */
    Dimension scroller_width;		/* width of the page scroller */
    Dimension scroller_height;		/* height of the page scroller */
    Dimension major_scroller_width;	/* width of major scrollers */
    Dimension major_scroller_height;	/* height of major scroller */
    Dimension minor_scroller_width;	/* width of minor scrollers */
    Dimension minor_scroller_height;	/* height of minor scrollers */
    Dimension frame_width;		/* width of the frame */
    Dimension frame_height;		/* height of the frame */

    /* for layouting tabs */
    Widget first_major;			/* the first major tab */
    Widget old_top_major;		/* the old top major tab */
    Widget top_major;			/* the top major tab */
    Widget last_major;			/* the last major tab */
    Widget first_minor;			/* the first minor tab */
    Widget old_top_minor;		/* the old top minor tab */
    Widget top_minor;			/* the top minor tab */
    Widget last_minor;			/* the last minor tab */
    Widget constraint_child;		/* changing geom during ConstraintSV */

    /* shadow thickness state for current page major and minor tab */
    Dimension major_shadow_thickness;	/* joined major tab shadow thickness */
    Dimension minor_shadow_thickness;	/* joined minor tab shadow thickness */
    Widget major_shadow_child;		/* saved shadow thickness tab */
    Widget minor_shadow_child;		/* saved shadow thickness tab */
    Boolean in_setshadow;		/* setting tab shadow thickness */

    /* extra position information */
    unsigned char major_pos;		/* position of major tabs */
    unsigned char minor_pos;		/* position of minor tabs */
    unsigned char binding_pos;		/* binding position */

    /* other misc. variables */
    unsigned char which_tab;		/* currently active tab type */
    int last_alloc_num;			/* lastly allocated page number */
    unsigned char scroller_status;	/* status of the page scroller */
    unsigned short need_scroller;	/* need for tab scrollers */
    Boolean dynamic_last_page_num;	/* True if using dynamic last page# */
    Boolean in_callback;		/* True if processing a callback */
    GC back_page_gc;                    /* GC for drawing backpages */
    GC frame_gc;			/* GC for drawing frame */
    GC binding_gc;                      /* GC for drawing binding */
    GC foreground_gc;                   /* GC for drawing foreground */
    GC background_gc;                   /* GC for drawing background */

    Boolean first_change_managed;	/* flags 1st call to ChangeManaged */
    XmScrollFrameData scroll_frame_data; /* data for ScrollFrame trait */
} XmNotebookPart;


/* Full instance record declaration */
typedef struct _XmNotebookRec
{
    CorePart core;
    CompositePart composite;
    ConstraintPart constraint;
    XmManagerPart manager;
    XmNotebookPart notebook;
} XmNotebookRec;


/******************************************************************************
 *                                                                            *
 *                         constants & useful macros                          *
 *                                                                            *
 ******************************************************************************/

/* internal child types, must not conflict with XmRNBChildType enum */
#define XmMAJOR_TAB_SCROLLER            12
#define XmMINOR_TAB_SCROLLER            13
#define XmTAB_SCROLLER                  14

#ifdef __cplusplus
}
#endif

#endif /* _XmNotebookP_h */


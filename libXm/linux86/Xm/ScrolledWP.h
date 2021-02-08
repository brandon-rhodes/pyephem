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
/* $XConsortium: ScrolledWP.h /main/13 1995/09/19 23:08:30 cde-sun $ */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScrolledWindowP_h
#define _XmScrolledWindowP_h

#include <Xm/ManagerP.h>
#include <Xm/ScrolledW.h>

#include <Xm/ScrollBar.h>
#include <Xm/DrawingA.h>

#include <Xm/ScrollFrameT.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*XmGetAutoDragRectsProc)(Widget, XRectangle **, Cardinal *) ;

#define XmInheritGetAutoDragRectsProc ((XmGetAutoDragRectsProc) _XtInherit)

#define XmScrolledWindowClassExtVersion 1L

typedef struct _XmScrolledWindowClassExtRec{
    XtPointer           next_extension;
    XrmQuark            record_type;
    long                version;
    Cardinal            record_size;
    XmGetAutoDragRectsProc get_hor_rects;
    XmGetAutoDragRectsProc get_vert_rects;
} XmScrolledWindowClassExtRec, *XmScrolledWindowClassExt;

/* New fields for the ScrolledWindow widget class record */
typedef struct {
     XtPointer extension;   /* extension to new class methods */
} XmScrolledWindowClassPart;

/****************
 *
 * Class record declaration
 *
 ****************/
typedef struct _XmScrolledWindowClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    XmScrolledWindowClassPart	swindow_class;
} XmScrolledWindowClassRec;

externalref XmScrolledWindowClassRec xmScrolledWindowClassRec;

/****************
 *
 * Scrolled Window instance structure.
 *
 ****************/
typedef struct {

   int vmin;		  /*  slider minimum coordinate position     */
   int vmax;		  /*  slider maximum coordinate position     */
   int vOrigin;		  /*  slider edge location                  */
   int vExtent;		  /*  slider size                           */

   int hmin;		  /*  Same as above for horizontal bar.     */
   int hmax;
   int hOrigin;
   int hExtent;

   Position hsbX,hsbY;
   Dimension hsbWidth,hsbHeight;    /* Dimensions for the horiz bar */

   Position vsbX,vsbY;
   Dimension vsbWidth,vsbHeight;    /* Dimensions for the vertical bar */

   Position    GivenHeight, GivenWidth;
   /* these 2 fields are now obsolete, we re-use them for
      a different purpose, to track the initial position
      of the scrollbars */
#define sw_prev_x GivenWidth
#define sw_prev_y GivenHeight

   Dimension	AreaWidth,AreaHeight;
   Dimension	WidthPad,HeightPad;
   Position	XOffset, YOffset;

   Dimension	pad;

   Boolean	hasHSB;
   Boolean	hasVSB;
   Boolean	InInit;
   Boolean	FromResize;

   unsigned char	VisualPolicy;
   unsigned char	ScrollPolicy;
   unsigned char	ScrollBarPolicy;
   unsigned char	Placement;
   
   XmScrollBarWidget   	hScrollBar;
   XmScrollBarWidget   	vScrollBar;
   XmDrawingAreaWidget 	ClipWindow;
   Widget              	WorkWindow;
   
   XtCallbackList       traverseObscuredCallback;

   XtEnum       auto_drag_model;
   XtIntervalId auto_drag_timer; 

   Boolean scroll_frame_inited ;
   XmScrollFrameData scroll_frame_data ;

   XtPointer	auto_drag_rects;
   XtPointer	auto_drag_closure;
} XmScrolledWindowPart;


/************************************************************************
 *									*
 * Full instance record declaration					*
 *									*
 ************************************************************************/

typedef struct _XmScrolledWindowRec {
    CorePart	    core;
    CompositePart   composite;
    ConstraintPart constraint;
    XmManagerPart   manager;
    XmScrolledWindowPart   swindow;
} XmScrolledWindowRec;


/*  Frame constraint records  */

typedef struct _XmScrolledWindowConstraintPart
{
   unsigned char child_type;
   Position orig_x;
   Position orig_y;
} XmScrolledWindowConstraintPart, * XmScrolledWindowConstraint;

typedef struct _XmScrolledWindowConstraintRec
{
   XmManagerConstraintPart manager;
   XmScrolledWindowConstraintPart   swindow;
} XmScrolledWindowConstraintRec, * XmScrolledWindowConstraintPtr;


#define DEFAULT_HEIGHT	20
#define DEFAULT_WIDTH	20

#define RESOURCE_DEFAULT (-1)

#define GetSWConstraint(w) \
        (&((XmScrolledWindowConstraintPtr) (w)->core.constraints)->swindow)


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScrolledWindowP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

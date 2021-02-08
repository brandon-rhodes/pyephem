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
/*   $XConsortium: DrawnBP.h /main/13 1995/07/14 10:29:40 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDButtonP_h
#define _XmDButtonP_h

#include <Xm/DrawnB.h>
#include <Xm/LabelP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DrawnButton class structure */

typedef struct _XmDrawnButtonClassPart
{
   XtPointer extension;   /* Pointer to extension record */
} XmDrawnButtonClassPart;


/* Full class record declaration for DrawnButton class */

typedef struct _XmDrawnButtonClassRec {
    CoreClassPart	  core_class;
    XmPrimitiveClassPart  primitive_class;
    XmLabelClassPart      label_class;
    XmDrawnButtonClassPart drawnbutton_class;
} XmDrawnButtonClassRec;


externalref  XmDrawnButtonClassRec xmDrawnButtonClassRec;


/* DrawnButton instance record */

typedef struct _XmDrawnButtonPart
{
   Boolean 	    pushbutton_enabled;
   unsigned char    shadow_type;
   XtCallbackList   activate_callback;
   XtCallbackList   arm_callback;
   XtCallbackList   disarm_callback;
   XtCallbackList   expose_callback;
   XtCallbackList   resize_callback;

   Boolean 	    armed;
   Dimension        old_width;
   Dimension        old_height;
   Dimension        old_shadow_thickness;
   Dimension        old_highlight_thickness;
   XtIntervalId     timer;
   unsigned char    multiClick;         /* KEEP/DISCARD resource */
   int              click_count;
   Time		    armTimeStamp;

} XmDrawnButtonPart;


/* Full instance record declaration */

typedef struct _XmDrawnButtonRec {
    CorePart	     core;
    XmPrimitivePart  primitive;
    XmLabelPart      label;
    XmDrawnButtonPart drawnbutton;
} XmDrawnButtonRec;


/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDButtonP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

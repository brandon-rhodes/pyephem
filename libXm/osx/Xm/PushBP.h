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
/*   $XConsortium: PushBP.h /main/12 1995/07/13 17:44:19 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmPButtonP_h
#define _XmPButtonP_h

#include <Xm/PushB.h>
#include <Xm/LabelP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PushButton class structure */

typedef struct _XmPushButtonClassPart
{
   XtPointer extension;   /* Pointer to extension record */
} XmPushButtonClassPart;


/* Full class record declaration for PushButton class */

typedef struct _XmPushButtonClassRec {
    CoreClassPart	  core_class;
    XmPrimitiveClassPart  primitive_class;
    XmLabelClassPart      label_class;
    XmPushButtonClassPart pushbutton_class;
} XmPushButtonClassRec;


externalref XmPushButtonClassRec xmPushButtonClassRec;

/* PushButton instance record */

typedef struct _XmPushButtonPart
{
   Boolean 	    fill_on_arm;
   Dimension        show_as_default;
   Pixel	    arm_color;
   Pixmap	    arm_pixmap;
   XtCallbackList   activate_callback;
   XtCallbackList   arm_callback;
   XtCallbackList   disarm_callback;

   Boolean 	    armed;
   Pixmap	    unarm_pixmap;
   GC               fill_gc;
   GC               background_gc;
   XtIntervalId     timer;	
   unsigned char    multiClick;		/* KEEP/DISCARD resource */
   int		    click_count;
   Time		    armTimeStamp;
   Boolean      compatible;   /* if false it is Motif 1.1 else Motif 1.0  */
   Dimension    default_button_shadow_thickness;  
		/* New resource - always add it
                    to widgets dimension. */

} XmPushButtonPart;


/* Full instance record declaration */

typedef struct _XmPushButtonRec {
    CorePart	     core;
    XmPrimitivePart  primitive;
    XmLabelPart      label;
    XmPushButtonPart pushbutton;
} XmPushButtonRec;


/********    Private Function Declarations    ********/

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPButtonP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

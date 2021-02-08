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
/* $XConsortium: ToggleBP.h /main/14 1996/03/29 18:56:59 pascale $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/********************************************
 *
 *   No new fields need to be defined
 *   for the Toggle widget class record
 *
 ********************************************/

#ifndef _XmToggleButtonP_h
#define _XmToggleButtonP_h

#include <Xm/ToggleB.h>
#include <Xm/LabelP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmToggleButtonClassPart
 {
   XtPointer extension;   /* Pointer to extension record */
 } XmToggleButtonClassPart;


/****************************************************
 *
 * Full class record declaration for Toggle class
 *
 ****************************************************/
typedef struct _XmToggleButtonClassRec {
    CoreClassPart	  	core_class;
    XmPrimitiveClassPart  	primitive_class;
    XmLabelClassPart      	label_class;
    XmToggleButtonClassPart	toggle_class;
} XmToggleButtonClassRec;


externalref XmToggleButtonClassRec xmToggleButtonClassRec;


/********************************************
 *
 * No new fields needed for instance record
 *
 ********************************************/

typedef struct _XmToggleButtonPart
{ 
   unsigned char	ind_type;
   Boolean		visible;
   Dimension		spacing;
   Dimension		indicator_dim;
   Boolean		indicator_set;
   Pixmap		on_pixmap; 
   Pixmap		insen_pixmap; 
   unsigned char	set;
   unsigned char	visual_set; /* used for visuals and does not reflect
                                        the true state of the button */
   unsigned char	ind_on;
   Boolean		fill_on_select;
   Pixel		select_color;
   GC			select_GC;
   GC			background_gc;
   GC                   arm_GC;    /* used in menus when enableEtchedInMenu 
				      is set. */
   XtCallbackList 	value_changed_CB,
                        arm_CB,
                        disarm_CB;
   Boolean      	Armed;
   unsigned char        toggle_mode;
   Boolean		reversed_select;
   Pixmap               indeterminate_pixmap;
   Pixmap               indeterminate_insensitive_pixmap;
   Pixel                unselect_color;
   GC                   unselect_GC;
   GC                   indeterminate_GC;
   GC                   indeterminate_box_GC;
   Dimension		ind_left_delta;
   Dimension		ind_right_delta;
   Dimension		ind_top_delta;
   Dimension		ind_bottom_delta;
   Dimension detail_shadow_thickness ;
} XmToggleButtonPart;



/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmToggleButtonRec {
    CorePart	        core;
    XmPrimitivePart     primitive;
    XmLabelPart		label;
    XmToggleButtonPart  toggle;
} XmToggleButtonRec;


/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmToggleButtonP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

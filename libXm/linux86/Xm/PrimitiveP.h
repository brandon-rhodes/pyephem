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
/* $XConsortium: PrimitiveP.h /main/10 1996/03/28 15:59:54 daniel $ */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1989, 1990 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
#ifndef _XmPrimitiveP_h
#define _XmPrimitiveP_h

#ifndef _XmNO_BC_INCL
#define _XmNO_BC_INCL
#endif

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Access Macros */

#define Prim_ShadowThickness(w) (((XmPrimitiveWidget)(w))->primitive.shadow_thickness)
#define Prim_HaveTraversal(w) (((XmPrimitiveWidget)(w))->primitive.have_traversal)

#define PCEPTR(wc)  ((XmPrimitiveClassExt *)(&(((XmPrimitiveWidgetClass)(wc))->primitive_class.extension)))
#define _XmGetPrimitiveClassExtPtr(wc, owner) \
  ((*PCEPTR(wc) && (((*PCEPTR(wc))->record_type) == owner))\
   ? PCEPTR(wc) \
   :((XmPrimitiveClassExt *) _XmGetClassExtensionPtr(((XmGenericClassExt *)PCEPTR(wc)), owner)))


#define XmPrimitiveClassExtVersion 1L


typedef struct _XmPrimitiveClassExtRec{
    XtPointer           next_extension;
    XrmQuark            record_type;
    long                version;
    Cardinal            record_size;
    XmWidgetBaselineProc widget_baseline;
    XmWidgetDisplayRectProc  widget_display_rect;
    XmWidgetMarginsProc widget_margins;
} XmPrimitiveClassExtRec, *XmPrimitiveClassExt;

typedef struct _XmPrimitiveClassPart
{
   XtWidgetProc         border_highlight;
   XtWidgetProc         border_unhighlight;
   String               translations;
   XtActionProc         arm_and_activate;
   XmSyntheticResource * syn_resources;   
   int                  num_syn_resources;   
   XtPointer            extension;
} XmPrimitiveClassPart;

typedef struct _XmPrimitiveClassRec
{
    CoreClassPart        core_class;
    XmPrimitiveClassPart primitive_class;
} XmPrimitiveClassRec;

externalref XmPrimitiveClassRec xmPrimitiveClassRec;


/*  The Primitive instance record  */

typedef struct _XmPrimitivePart
{
   Pixel   foreground;

   Dimension   shadow_thickness;
   Pixel   top_shadow_color;
   Pixmap  top_shadow_pixmap;
   Pixel   bottom_shadow_color;
   Pixmap  bottom_shadow_pixmap;

   Dimension   highlight_thickness;
   Pixel   highlight_color;
   Pixmap  highlight_pixmap;

   XtCallbackList help_callback;
   XtPointer      user_data;

   Boolean traversal_on;
   Boolean highlight_on_enter;
   Boolean have_traversal;

   unsigned char unit_type;
   XmNavigationType navigation_type;

   Boolean highlight_drawn;
   Boolean highlighted;

   GC      highlight_GC;
   GC      top_shadow_GC;
   GC      bottom_shadow_GC;

   /* New fields in Motif 2.0 */

#ifndef XM_PART_BC

   XtCallbackList   convert_callback;       /* Selection convert callback */
   XtCallbackList   popup_handler_callback;

   XmDirection layout_direction;
#endif
   XmString tool_tip_string;
} XmPrimitivePart;

#ifdef XM_PART_BC
extern XmDirection XmPrimLayoutDir ;
#define XmPrim_layout_direction(w) (XmPrimLayoutDir)
#else
#define XmPrim_layout_direction(w) ((w)->primitive.layout_direction)
#endif


typedef struct _XmPrimitiveRec
{
   CorePart        core;
   XmPrimitivePart primitive;
} XmPrimitiveRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPrimitiveP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

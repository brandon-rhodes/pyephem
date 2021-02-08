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
/* $XConsortium: GadgetP.h /main/8 1995/07/13 17:27:17 drk $ */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1989, 1990 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
#ifndef _XmGadgetP_h
#define _XmGadgetP_h

#ifndef _XmNO_BC_INCL
#define _XmNO_BC_INCL
#endif

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif


/*  Masks to define input the gadget is interested in  */

#define XmNO_EVENT              0x000
#define XmENTER_EVENT           0x001
#define XmLEAVE_EVENT           0x002
#define XmFOCUS_IN_EVENT        0x004
#define XmFOCUS_OUT_EVENT       0x008
#define XmMOTION_EVENT          0x010
#define XmARM_EVENT             0x020
#define XmACTIVATE_EVENT        0x040
#define XmHELP_EVENT            0x080
#define XmKEY_EVENT             0x100
#define XmMULTI_ARM_EVENT       0x200
#define XmMULTI_ACTIVATE_EVENT  0x400
#define XmBDRAG_EVENT		0x800
#define XmALL_EVENT             0xFFF


/* Gadget access Macros */
#define G_ShadowThickness(g) \
	(((XmGadget)(g))->gadget.shadow_thickness)
#define G_HighlightThickness(g) \
	(((XmGadget)(g))->gadget.highlight_thickness)

#define GCEPTR(wc)  ((XmGadgetClassExt *)(&(((XmGadgetClass)(wc))->gadget_class.extension)))
#define _XmGetGadgetClassExtPtr(wc, owner) \
  ((*GCEPTR(wc) && (((*GCEPTR(wc))->record_type) == owner))\
   ? GCEPTR(wc) \
   :((XmGadgetClassExt *) _XmGetClassExtensionPtr(((XmGenericClassExt *)GCEPTR(wc)), owner)))

#define XmGadgetClassExtVersion 2L


/* Gadget cache header for each gadget's Cache Part */

typedef struct _XmGadgetCache
{
   struct _XmGadgetCache	*next;
   struct _XmGadgetCache	*prev;
   int				ref_count;
} XmGadgetCache, *XmGadgetCachePtr;


/* A cache entry for each class which desires gadget caching */

typedef struct _XmCacheClassPart 
{
   XmGadgetCache	cache_head;
   XmCacheCopyProc	cache_copy;
   XmGadgetCacheProc	cache_delete;
   XmCacheCompareProc   cache_compare;
} XmCacheClassPart, *XmCacheClassPartPtr;

/* A struct for properly aligning the data part of the cache entry. */

typedef struct _XmGadgetCacheRef
{
   XmGadgetCache	cache;
   XtArgVal		data;
} XmGadgetCacheRef, *XmGadgetCacheRefPtr;

/*  Gadget class structure  */

typedef struct _XmGadgetClassExtRec{
    XtPointer           next_extension;
    XrmQuark            record_type;
    long                version;
    Cardinal            record_size;
    XmWidgetBaselineProc widget_baseline;
    XmWidgetDisplayRectProc  widget_display_rect;
    XmWidgetMarginsProc widget_margins;
}XmGadgetClassExtRec, *XmGadgetClassExt;

typedef struct _XmGadgetClassPart
{
   XtWidgetProc         border_highlight;
   XtWidgetProc         border_unhighlight;
   XtActionProc         arm_and_activate;
   XmWidgetDispatchProc input_dispatch;
   XmVisualChangeProc   visual_change;		/* unused in Motif 2.0 */
   XmSyntheticResource * syn_resources;   
   int                  num_syn_resources;   
   XmCacheClassPartPtr	cache_part;
   XtPointer		extension;
} XmGadgetClassPart;


/*  Full class record declaration for Gadget class  */

typedef struct _XmGadgetClassRec
{
   RectObjClassPart  rect_class;
   XmGadgetClassPart gadget_class;
} XmGadgetClassRec;

externalref XmGadgetClassRec xmGadgetClassRec;


/*  The Gadget instance record  */

typedef struct _XmGadgetPart
{
   Dimension shadow_thickness;
   Dimension highlight_thickness;

   XtCallbackList help_callback;
   XtPointer      user_data;

   Boolean traversal_on;
   Boolean highlight_on_enter;
   Boolean have_traversal;

   unsigned char unit_type;
   XmNavigationType navigation_type;

   Boolean highlight_drawn;
   Boolean highlighted;
   Boolean visible;

   Mask event_mask;
   XmDirection layout_direction;
#ifdef OM22_COMPATIBILITY   
   XmString tool_tip_string;
#endif
} XmGadgetPart;

/*  Full instance record declaration  */

typedef struct _XmGadgetRec
{
   ObjectPart   object;
   RectObjPart  rectangle;
   XmGadgetPart gadget;
} XmGadgetRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmGadgetP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */






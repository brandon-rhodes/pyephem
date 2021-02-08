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
/* $XConsortium: ManagerP.h /main/10 1996/03/28 15:59:43 daniel $ */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1989, 1990 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
#ifndef _XmManagerP_h
#define _XmManagerP_h

#ifndef _XmNO_BC_INCL
#define _XmNO_BC_INCL
#endif

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Access Macros  */

#define XmParentTopShadowGC(w) 		\
	(((XmManagerWidget) XtParent(w))->manager.top_shadow_GC)

#define XmParentBottomShadowGC(w)	\
	(((XmManagerWidget) XtParent(w))->manager.bottom_shadow_GC)

#define XmParentHighlightGC(w)		\
	(((XmManagerWidget) XtParent(w))->manager.highlight_GC)

#define XmParentBackgroundGC(w)		\
	(((XmManagerWidget) XtParent(w))->manager.background_GC)


#define MGR_KeyboardList(m)		\
			(((XmManagerRec *) (m))->manager.keyboard_list)
#define MGR_NumKeyboardEntries(m)	\
			(((XmManagerRec *) (m))->manager.num_keyboard_entries)
#define MGR_SizeKeyboardList(m)		\
			(((XmManagerRec *) (m))->manager.size_keyboard_list)
#define MGR_ShadowThickness(m)		\
			(((XmManagerRec *) (m))->manager.shadow_thickness)


#define XmInheritTraversalChildrenProc ((XmTraversalChildrenProc) _XtInherit)
#define XmInheritObjectAtPointProc     ((XmObjectAtPointProc) _XtInherit)

typedef Boolean (*XmTraversalChildrenProc)( Widget, Widget **, Cardinal *) ;
typedef Widget (*XmObjectAtPointProc)(Widget, Position, Position) ;


/*  Structure used for storing accelerator and mnemonic information.  */

typedef struct 
{
   unsigned int eventType;
   KeySym       keysym;
   KeyCode      key;
   unsigned int modifiers;
   Widget       component;
   Boolean      needGrab;
   Boolean      isMnemonic;
} XmKeyboardData;


/*  The class definition  */

typedef struct {
    XtPointer next_extension;
    XrmQuark record_type;
    long version;
    Cardinal record_size;
    XmTraversalChildrenProc traversal_children ;
    XmObjectAtPointProc     object_at_point ;
} XmManagerClassExtRec, *XmManagerClassExt ;

#define XmManagerClassExtVersion 1L

typedef struct _XmManagerClassPart
{
   String               translations;
   XmSyntheticResource * syn_resources;   
   int                  num_syn_resources;   
   XmSyntheticResource * syn_constraint_resources;   
   int                  num_syn_constraint_resources;   
   XmParentProcessProc  parent_process;
   XtPointer            extension;
} XmManagerClassPart;

typedef struct _XmManagerClassRec
{
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
} XmManagerClassRec;

externalref XmManagerClassRec xmManagerClassRec;


/*  The instance definition  */

typedef struct _XmManagerPart
{
   Pixel   foreground;

   Dimension   shadow_thickness;
   Pixel   top_shadow_color;
   Pixmap  top_shadow_pixmap;
   Pixel   bottom_shadow_color;
   Pixmap  bottom_shadow_pixmap;

   Pixel   highlight_color;
   Pixmap  highlight_pixmap;

   XtCallbackList help_callback;
   XtPointer      user_data;

   Boolean traversal_on;
   unsigned char unit_type;
   XmNavigationType navigation_type;
   
   Boolean event_handler_added;
   Widget  active_child;
   Widget  highlighted_widget;
   Widget  accelerator_widget;

   Boolean has_focus;

   XmStringDirection string_direction;

   XmKeyboardData * keyboard_list;
   short num_keyboard_entries;
   short size_keyboard_list;

   XmGadget selected_gadget;
   XmGadget eligible_for_multi_button_event;

   GC      background_GC;
   GC      highlight_GC;
   GC      top_shadow_GC;
   GC      bottom_shadow_GC;

   Widget  initial_focus;

#ifndef XM_PART_BC
   XtCallbackList   popup_handler_callback;
#endif

} XmManagerPart;

typedef struct _XmManagerRec
{
   CorePart       core;
   CompositePart  composite;
   ConstraintPart constraint;
   XmManagerPart  manager;
} XmManagerRec;


/*  The constraint definition  */

typedef struct _XmManagerConstraintPart
{
   int unused;
} XmManagerConstraintPart;

typedef struct _XmManagerConstraintRec
{
   XmManagerConstraintPart manager;
} XmManagerConstraintRec, * XmManagerConstraintPtr;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmManagerP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

/* $XConsortium: ComboBoxP.h /main/8 1995/09/19 23:00:21 cde-sun $ */
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
/*	ComboBoxP.h  */
#ifndef _XmComboBoxP_H
#define _XmComboBoxP_H

#include <Xm/ManagerP.h>
#include <Xm/ComboBox.h> 


#ifdef __cplusplus
extern "C" {
#endif

/* New fields for the ComboBox widget class record. */

typedef struct _XmComboBoxClassPart {
  XtPointer extension;		/* Pointer to extension record. */
} XmComboBoxClassPart;


/* Full class record declaration. */	
typedef struct _XmComboBoxClassRec {
  CoreClassPart		core_class;
  CompositeClassPart	composite_class;
  ConstraintClassPart	constraint_class;
  XmManagerClassPart	manager_class;
  XmComboBoxClassPart	combo_box_class;
} XmComboBoxClassRec;

externalref XmComboBoxClassRec xmComboBoxClassRec;

/*
 * New fields for the ComboBox widget record.	
 */

typedef struct _XmComboBoxPart {
  /* Resources */
  unsigned char 	type;
  unsigned char		match_behavior;
  Dimension 		highlight_thickness;
  Dimension 		arrow_size;
  Dimension 		arrow_spacing;
  Dimension 		margin_width;
  Dimension 		margin_height;
  XtCallbackList	selection_callback;
  XmString		selected_item; /* synthetic, not updated */
  int			selected_position;
  XmFontList		render_table;

  /* Internal data */
  Widget		list_shell; 
  Widget		list;		/* Now accessible as a resource */
  Widget		scrolled_w; 
  Widget		vsb;
  Widget		hsb;
  int 			ideal_ebheight;
  int			ideal_ebwidth;
  GC 			arrow_GC;
  XRectangle 		hit_rect;
  Dimension 		arrow_shadow_width;
  Boolean 		arrow_pressed;
  Boolean		highlighted;
  Boolean		scrolling;
  XtEnum		shell_state;
  /* NOTE that text_changed is also used for MT_safe resolution of
   * the XmNRenderTable, XmNFontList resource settings 
   */
  Boolean		text_changed;

  /* New resources/data for CDE compatibility. */
  Widget		edit_box;
  XmStringTable		items;
  int			item_count;
  int			visible_item_count;
  short			columns;
  XtEnum		position_mode;

} XmComboBoxPart;


/* Full instance record declaration. */	

typedef struct _XmComboBoxRec {
  CorePart		core;
  CompositePart		composite;
  ConstraintPart	constraint;
  XmManagerPart		manager;
  XmComboBoxPart 	combo_box;
} XmComboBoxRec;


/********    Private Function Declarations    ********/

/********    End Private Function Declarations    ********/


/* Access macros */
#define CB_ArrowPressed(w)   (((XmComboBoxWidget)(w))->combo_box.arrow_pressed)
#define CB_ArrowSize(w)	     (((XmComboBoxWidget)(w))->combo_box.arrow_size)
#define CB_ArrowSpacing(w)   (((XmComboBoxWidget)(w))->combo_box.arrow_spacing)
#define CB_EditBox(w) 	     (((XmComboBoxWidget)(w))->combo_box.edit_box)
#define CB_HighlightThickness(w)	\
	(((XmComboBoxWidget)(w))->combo_box.highlight_thickness)
#define CB_Highlighted(w)    (((XmComboBoxWidget)(w))->combo_box.highlighted)
#define CB_HitRect(w)	     (((XmComboBoxWidget)(w))->combo_box.hit_rect)
#define CB_List(w) 	     (((XmComboBoxWidget)(w))->combo_box.list)
#define CB_ListShell(w)      (((XmComboBoxWidget)(w))->combo_box.list_shell)
#define CB_MarginHeight(w)   (((XmComboBoxWidget)(w))->combo_box.margin_height)
#define CB_MarginWidth(w)    (((XmComboBoxWidget)(w))->combo_box.margin_width)
#define CB_MatchBehavior(w)		\
	(((XmComboBoxWidget)(w))->combo_box.match_behavior)
#define CB_PositionMode(w)   (((XmComboBoxWidget)(w))->combo_box.position_mode)
#define CB_RenderTable(w)    (((XmComboBoxWidget)(w))->combo_box.render_table)
#define CB_ScrolledW(w)      (((XmComboBoxWidget)(w))->combo_box.scrolled_w)
#define CB_SelectionCB(w)		\
	(((XmComboBoxWidget)(w))->combo_box.selection_callback)
#define CB_ShellState(w)     (((XmComboBoxWidget)(w))->combo_box.shell_state)
#define CB_TextChanged(w)    (((XmComboBoxWidget)(w))->combo_box.text_changed)
#define CB_Type(w) 	     (((XmComboBoxWidget)(w))->combo_box.type)


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* _XmComboBoxP_H */
/* DON'T ADD ANYTHING AFTER THIS #endif */

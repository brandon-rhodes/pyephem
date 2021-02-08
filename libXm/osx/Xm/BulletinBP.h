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
/*   $XConsortium: BulletinBP.h /main/13 1995/09/19 22:59:44 cde-sun $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmBulletinBoardP_h
#define _XmBulletinBoardP_h

#include <Xm/BulletinB.h>
#include <Xm/ManagerP.h>

#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
 * this suffix is added to dialog shells created by Xm convenience routines *
 * so that, for example, a call to create a form dialog named f generates a *
 * dialog shell named f_popup in addition to a form named f                 *
 ****************************************************************************/

#define XmDIALOG_SUFFIX		"_popup"
#define XmDIALOG_SUFFIX_SIZE	6



typedef struct _XmBulletinBoardConstraintPart
{
   char unused;
} XmBulletinBoardConstraintPart, * XmBulletinBoardConstraint;


/*  New fields for the BulletinBoard widget class record  */

typedef struct
{
  Boolean		always_install_accelerators;
  XmGeoCreateProc       geo_matrix_create;
  XmFocusMovedProc	focus_moved_proc;
  XtPointer		extension;
} XmBulletinBoardClassPart;


/* Full class record declaration */

typedef struct _XmBulletinBoardClassRec
{
  CoreClassPart			core_class;
  CompositeClassPart		composite_class;
  ConstraintClassPart		constraint_class;
  XmManagerClassPart		manager_class;
  XmBulletinBoardClassPart	bulletin_board_class;
} XmBulletinBoardClassRec;

externalref XmBulletinBoardClassRec xmBulletinBoardClassRec;


/* New fields for the BulletinBoard widget record */

typedef struct
{
  Dimension	margin_width;		/*  margins		*/
  Dimension	margin_height;

  Widget	default_button;		/*  widgets		*/
  Widget	dynamic_default_button;	/*  widgets		*/
  Widget	cancel_button;
  Widget	dynamic_cancel_button;

  XtCallbackList focus_callback;	/*  callback lists	*/
  XtCallbackList map_callback;
  XtCallbackList unmap_callback;

  XtTranslations text_translations;

  XmFontList	button_font_list;	/*  font lists		*/
  XmFontList	label_font_list;
  XmFontList	text_font_list;

  Boolean	allow_overlap;		/*  policies		*/
  Boolean	default_position;
  Boolean	auto_unmanage;
  unsigned char	resize_policy;
  
  Dimension	old_width;		/*  shadow resources	*/
  Dimension	old_height;
  Dimension	old_shadow_thickness;
  unsigned char	shadow_type;

  Boolean	in_set_values;		/*  internal flag	*/
  Boolean	initial_focus;

  Boolean	no_resize;		/*  dialog resources	*/
  unsigned char	dialog_style;
  XmString	dialog_title;
  Widget	shell;
  Widget	_UNUSED;

  XmGeoMatrix   geo_cache;		/* Cache for geometry management. */

  unsigned char check_set;	/* For XmNfontList & XmNRenderTable */
} XmBulletinBoardPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmBulletinBoardRec
{
  CorePart		core;
  CompositePart		composite;
  ConstraintPart	constraint;
  XmManagerPart		manager;
  XmBulletinBoardPart	bulletin_board;
} XmBulletinBoardRec;


/* Access macros */
#define BB_CancelButton(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.cancel_button)
#define BB_DynamicCancelButton(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.dynamic_cancel_button)
#define BB_DefaultButton(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.default_button)
#define BB_DynamicDefaultButton(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.dynamic_default_button)
#define BB_MarginHeight(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.margin_height)
#define BB_MarginWidth(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.margin_width)
#define BB_ButtonFontList(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.button_font_list)
#define BB_LabelFontList(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.label_font_list)
#define BB_TextFontList(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.text_font_list)
#define BB_StringDirection(w) (XmDirectionToStringDirection\
	(((XmBulletinBoardWidget) w)->manager.string_direction))
#define BB_ResizePolicy(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.resize_policy)
#define BB_InSetValues(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.in_set_values)
#define BB_InitialFocus(w) \
	(((XmBulletinBoardWidget) w)->bulletin_board.initial_focus)


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmBulletinBoardP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

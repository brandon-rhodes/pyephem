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
 
/* (c) Copyright 2002, Integrated Computer Solutions Cambridge, MASS. */

#ifndef _XmTabStackP_h_
#define _XmTabStackP_h_


#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/BulletinBP.h>
#include <Xm/ExtP.h>
#include <Xm/TabBox.h>
#include <Xm/TabList.h>
#include <Xm/TabStack.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmTabStackClassPart {
    String    drag_translations;
    XtPointer extension;
} XmTabStackClassPart;


typedef struct _XmTabStackClassRec {
    CoreClassPart            core_class;
    CompositeClassPart	     composite_class;
    ConstraintClassPart      constraint_class;
    XmManagerClassPart       manager_class;
    XmBulletinBoardClassPart bulletin_board_class;
    XmTabStackClassPart      tab_stack_class;
} XmTabStackClassRec;
externalref XmTabStackClassRec xmTabStackClassRec;

typedef struct _XmTabStackPart {
    /* Resources for the Tab Stack */

    XtCallbackList       tab_select_callback;

    /* Resource to pass down to Tab Box */
    XmFontList		 font_list;
    XmTabStyle		 tab_style;
    XmTabMode		 tab_mode;
    XmTabSide            tab_side;
    XmTabOrientation	 tab_orientation;

    XmString		 tear_off_label;

    Boolean		 allow_tear_offs;
    Boolean		 uniform_tab_size;
    Boolean		 use_image_cache;
    Boolean		 stacked_effect;
    Boolean		 tab_auto_select;
    
    Dimension            tab_margin_width;
    Dimension            tab_margin_height;
    Dimension		 tab_label_spacing;
    Dimension            tab_offset;
    Dimension            highlight_thickness;

    Pixel		 select_color;
    Pixel		 select_pixmap;

    int			 tab_corner_percent;

    Widget               tab_box;

    /* Private Values */

    XmTabbedStackList		 _tab_list;

    XRectangle           _size;

    Widget               _active_child;
    GC			 _gc;

    Boolean		 _inited;
    Boolean		 _set_tab_list;

    Widget               _menu;
    Widget		 _tear_off_button;

    /* Drag And Drop Stuff */
    Pixmap               _source_pixmap;
    Pixmap               _source_mask;
    Pixmap               _invalid_pixmap;
    Pixmap               _invalid_mask;
    Widget               _source_icon;
    Widget               _invalid_icon;

    Widget		_selected_tab;	    /* used within realize method */
    Boolean		_selected_notify;   /* used within realize method */

    Boolean		do_notify;	/* for notify XmNtabSelectedCallback */

    Boolean		check_set_render_table;
  
} XmTabStackPart;

#define XmTabStack_tab_select_callback(w) (((XmTabStackWidget)(w))->tab_stack.tab_select_callback)
#define XmTabStack_font_list(w) (((XmTabStackWidget)(w))->tab_stack.font_list)
#define XmTabStack_tab_style(w) (((XmTabStackWidget)(w))->tab_stack.tab_style)
#define XmTabStack_tab_mode(w) (((XmTabStackWidget)(w))->tab_stack.tab_mode)
#define XmTabStack_tab_side(w) (((XmTabStackWidget)(w))->tab_stack.tab_side)
#define XmTabStack_tab_orientation(w) (((XmTabStackWidget)(w))->tab_stack.tab_orientation)
#define XmTabStack_tear_off_label(w) (((XmTabStackWidget)(w))->tab_stack.tear_off_label)
#define XmTabStack_allow_tear_offs(w) (((XmTabStackWidget)(w))->tab_stack.allow_tear_offs)
#define XmTabStack_uniform_tab_size(w) (((XmTabStackWidget)(w))->tab_stack.uniform_tab_size)
#define XmTabStack_use_image_cache(w) (((XmTabStackWidget)(w))->tab_stack.use_image_cache)
#define XmTabStack_stacked_effect(w) (((XmTabStackWidget)(w))->tab_stack.stacked_effect)
#define XmTabStack_tab_auto_select(w) (((XmTabStackWidget)(w))->tab_stack.tab_auto_select)
#define XmTabStack_tab_margin_width(w) (((XmTabStackWidget)(w))->tab_stack.tab_margin_width)
#define XmTabStack_tab_margin_height(w) (((XmTabStackWidget)(w))->tab_stack.tab_margin_height)
#define XmTabStack_tab_label_spacing(w) (((XmTabStackWidget)(w))->tab_stack.tab_label_spacing)
#define XmTabStack_tab_offset(w) (((XmTabStackWidget)(w))->tab_stack.tab_offset)
#define XmTabStack_highlight_thickness(w) (((XmTabStackWidget)(w))->tab_stack.highlight_thickness)
#define XmTabStack_select_color(w) (((XmTabStackWidget)(w))->tab_stack.select_color)
#define XmTabStack_select_pixmap(w) (((XmTabStackWidget)(w))->tab_stack.select_pixmap)
#define XmTabStack_tab_corner_percent(w) (((XmTabStackWidget)(w))->tab_stack.tab_corner_percent)
#define XmTabStack_tab_box(w) (((XmTabStackWidget)(w))->tab_stack.tab_box)
#define XmTabStack__tab_list(w) (((XmTabStackWidget)(w))->tab_stack._tab_list)
#define XmTabStack__size(w) (((XmTabStackWidget)(w))->tab_stack._size)
#define XmTabStack__active_child(w) (((XmTabStackWidget)(w))->tab_stack._active_child)
#define XmTabStack__gc(w) (((XmTabStackWidget)(w))->tab_stack._gc)
#define XmTabStack__inited(w) (((XmTabStackWidget)(w))->tab_stack._inited)
#define XmTabStack__set_tab_list(w) (((XmTabStackWidget)(w))->tab_stack._set_tab_list)
#define XmTabStack__menu(w) (((XmTabStackWidget)(w))->tab_stack._menu)
#define XmTabStack__tear_off_button(w) (((XmTabStackWidget)(w))->tab_stack._tear_off_button)
#define XmTabStack__source_pixmap(w) (((XmTabStackWidget)(w))->tab_stack._source_pixmap)
#define XmTabStack__source_mask(w) (((XmTabStackWidget)(w))->tab_stack._source_mask)
#define XmTabStack__invalid_pixmap(w) (((XmTabStackWidget)(w))->tab_stack._invalid_pixmap)
#define XmTabStack__invalid_mask(w) (((XmTabStackWidget)(w))->tab_stack._invalid_mask)
#define XmTabStack__source_icon(w) (((XmTabStackWidget)(w))->tab_stack._source_icon)
#define XmTabStack__invalid_icon(w) (((XmTabStackWidget)(w))->tab_stack._invalid_icon)
#define XmTabStack__selected_tab(w) (((XmTabStackWidget)(w))->tab_stack._selected_tab)
#define XmTabStack__selected_notify(w) (((XmTabStackWidget)(w))->tab_stack._selected_notify)
#define XmTabStack_do_notify(w) (((XmTabStackWidget)(w))->tab_stack.do_notify)

typedef struct _XmTabStackRec {
    CorePart             core;
    CompositePart        composite;
    ConstraintPart       constraint;
    XmManagerPart        manager;
    XmBulletinBoardPart  bulletin_board;
    XmTabStackPart       tab_stack;
} XmTabStackRec;

typedef struct _XmTabStackConstraintPart {
    XmString    	 tab_label_string;
    XmStringDirection    tab_string_direction;
    unsigned char        tab_alignment;
    Pixmap		 tab_label_pixmap;
    XmPixmapPlacement    tab_pixmap_placement;
    Pixel		 tab_foreground;
    Pixel		 tab_background;
    Pixmap		 tab_background_pixmap;
    Boolean		 free_tab_pixmap;

    Boolean		 tear_off_enabled;

    /* Private Memebers */
    int			index;
    Dimension		width;
    Dimension		height;
} XmTabStackConstraintPart;

#define XmTabStackC_tab_label_string(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.tab_label_string)
#define XmTabStackC_tab_string_direction(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.tab_string_direction)
#define XmTabStackC_tab_alignment(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.tab_alignment)
#define XmTabStackC_tab_label_pixmap(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.tab_label_pixmap)
#define XmTabStackC_tab_pixmap_placement(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.tab_pixmap_placement)
#define XmTabStackC_tab_foreground(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.tab_foreground)
#define XmTabStackC_tab_background(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.tab_background)
#define XmTabStackC_tab_background_pixmap(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.tab_background_pixmap)
#define XmTabStackC_free_tab_pixmap(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.free_tab_pixmap)
#define XmTabStackC_tear_off_enabled(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.tear_off_enabled)
#define XmTabStackC_index(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.index)
#define XmTabStackC_width(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.width)
#define XmTabStackC_height(w) \
    (((XmTabStackConstraintPtr)((w)->core.constraints))->tab_stack.height)

typedef struct _XmTabStackConstraintRec {
    XmManagerConstraintPart  manager;
    XmTabStackConstraintPart tab_stack;
} XmTabStackConstraintRec, * XmTabStackConstraintPtr;

#define XmNillegalUniformTabSizeMsg \
"XmNuniformTabSize must be true if XmNtabMode is XmTABS_STACKED or\n\
XmTABS_STACKED_STATIC."

#define XmTabStackIndex (XmBulletinBoardIndex + 1)

#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration */
#endif


#endif /* _TabStackP_h_ */

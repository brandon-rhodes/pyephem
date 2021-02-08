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
/* $XConsortium: RowColumnP.h /main/13 1996/05/21 12:03:34 pascale $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
#ifndef _XmRowColumnP_h
#define _XmRowColumnP_h

#include <Xm/RowColumn.h>
#include <Xm/ManagerP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Button Event Status Record for popup verification at manage time */
typedef struct _XmButtonEventStatusRec
{
	Time time;
	Boolean verified;
	Boolean waiting_to_be_managed;
	XButtonEvent event;
} XmButtonEventStatusRec;

/* replay info rec - last menu that was posted before event replay */
typedef struct _XmReplayInfoRec
{
	Time time;
	Widget toplevel_menu;
} XmReplayInfoRec;

typedef struct _XmMenuFocusRec
{
	Widget  oldWidget;
 	Window  oldFocus;
	int	oldRevert;
	Time	oldTime;
} XmMenuFocusRec;

/* Menu State is per screen */
typedef struct _XmMenuStateRec
{
   Widget RC_LastSelectToplevel;
   XmButtonEventStatusRec RC_ButtonEventStatus;
   XmReplayInfoRec RC_ReplayInfo;
/*
 * needed for funky menubar mode so that the traversal can be restored
 * to the correct highlighted item when we are done.
 */
   Widget RC_activeItem;
   XmMenuFocusRec RC_menuFocus;
/*
 * A workaround is provided to allow applications to get insensitive
 * menu items.  This is useful for context "sensitive-shared-tear off-
 * accelerated" menu items.  Accessed via internal (for now) function,
 * _XmAllowAcceleratedInsensitiveUmanagedMenuItems().
 */
   Boolean RC_allowAcceleratedInsensitiveUnmanagedMenuItems;
   Time MS_LastManagedMenuTime;
   Boolean MU_InDragMode;
   Widget MU_CurrentMenuChild;
   Boolean MU_InPMMode;
} XmMenuStateRec, *XmMenuState;


typedef struct _XmRCKidGeometryRec
{
  Widget kid;
  XtWidgetGeometry  box;
  Dimension margin_top;
  Dimension margin_bottom;
  Dimension baseline;
} XmRCKidGeometryRec, *XmRCKidGeometry;

/* The RowColumn instance record */

typedef	struct _XmRowColumnPart
{
    Dimension       margin_height;  /* margin around inside of widget */
    Dimension       margin_width;


    Dimension       spacing;        /* pixels between entries */
    Dimension       entry_border;   /* size of entry borders */

                    /* next only used w/ menubars */

    Widget      help_pushbutton;    /* ptr to help pushbutton widget */

    Widget      cascadeBtn;         /* if this menu is pulled down by a */
                                    /* pulldown widget this will point */
                                    /* at the pulldown.  needed to go */
                                    /* up the cascade */

                    /* next two only used w/ option menus */
                    /* they are really only temporary */
                    /* since the data is passed off to */
                    /* the pulldown widget which is */
                    /* automatically built */

    XmString  option_label;         /* label for option menu pulldown */

    Widget      option_submenu;     /* which submenu to pulldown */


    XmRCKidGeometry   boxes;          /* when doing menu layouts is an */
                                    /* array of geo req's to make it easy */

    WidgetClass     entry_class;    /* if homogeneous, what class */

    XtCallbackList  entry_callback; /* a child fired off */
    XtCallbackList  map_callback;   /* about to be mapped call back */
    XtCallbackList  unmap_callback; /* about to be unmapped call back */

    Widget      memory_subwidget;   /* id of last subwidget that */
                                    /* fired off.  Recorded by the */
                                    /* entry_fired proc, can be set too */
                                    /* this causes mouse/muscle memory */
                                    /* to also be reset */

    short       num_columns;        /* if columnar packing this is how */
                                    /* many columns to use */

    String	    menuPost;	    /* a translation for posting popups */
    unsigned int    postButton;     /* active mouse button */
    int             postEventType;  /* active mouse event type */
    unsigned int    postModifiers;  /* active mouse modifier */

    String      menu_accelerator;
    KeySym	    mnemonic;
    XmStringCharSet mnemonicCharSet;

    unsigned char   entry_alignment; /* type of label alignment */
                                     /* our children should have */

                    /* next two are layout, Tight is the */
                    /* standard menubar packing.  Columns */
                    /* is radio box style, orientation */
                    /* determines if it is column or row */
                    /* major, Vert = column major */

    unsigned char   packing;    /* entry packing (layout) style */

    unsigned char   type;       /* temporary: diff between menu/bar */

    unsigned char   orientation;    /* horizontal or vertical */

                    /* next two indicate how the widget */
                    /* responds to size changes if there */
                    /* is no geo mgr.  If true then the */
                    /* dimension is never changed.  Set */
                    /* to true if dimension is spec'd */
                    /* at create time */

    Boolean     armed;      /* controls whether pulldowns work */
                            /* or not, button down in any part of */
                            /* the menubar arms it, this is a bit field  */
                            /* used for other internal flags, see macros */ 

                    /* next is only valid for popup menus */

    Boolean     adjust_margin;  /* T/F, indicating if we should force */
                                /* all subwidgets to have similar */
                                /* margins */
    
    Boolean     adjust_last;    /* Indicates whether or not the last row */
                                /* row or column should be stretched to  */
                                /* the edge of the row_column widget.    */

    Boolean     do_alignment;   /* T/F, do we force alignment on all */
                                /* our children */

    Boolean     radio;          /* T/F, do we do the toggle button */
                                /* 'only-one-down' enforcement */

    Boolean     radio_one;      /* T/F, must have one radio button */
                                /* set to on */


    Boolean     homogeneous;    /* T/F, do we only allow a single */
                                /* class of children */

    Boolean     resize_width;
    Boolean     resize_height;

    XtEnum      popup_enabled;

    Dimension	old_width;		/* save the old width, etc to use  */
    Dimension	old_height;		/* at resize time since it now has */
    Dimension	old_shadow_thickness;   /* NW gravity                      */

    Widget *	postFromList;		/* list for sharing menupanes */
    int		postFromCount;		/* count of the list */
    int		postFromListSize;	/* size of the malloc'ed list */

    Widget      lastSelectToplevel;     /* returned in XmGetPostedFromWidget*/ 
    Widget	popupPosted;		/* popup submenu currently posted */

    unsigned char oldFocusPolicy;	/* save when menus begin traversal */

    /***************** 1.2 ***************/
    unsigned char	TearOffModel;	/* enable/disable flag */
    Widget		ParentShell;	/* Save the parent shell when torn */
    Widget		tear_off_control;
    Boolean		to_state;	/* tear off state */
    /* tear off activate/deactivate callbacks */
    XtCallbackList	tear_off_activated_callback;
    XtCallbackList	tear_off_deactivated_callback;
    Widget		tear_off_lastSelectToplevel;
    Widget		tear_off_focus_item;	/* when tear off is inactive */
    
    unsigned char	entry_vertical_alignment;
    unsigned char	popup_menu_click;
    XtWorkProcId	popup_workproc;
    XmString		tear_off_title;
} XmRowColumnPart;


/* Full instance record declaration */

typedef struct _XmRowColumnRec
{
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    XmRowColumnPart	row_column;
} XmRowColumnRec;

typedef struct _XmRowColumnWidgetRec /* OBSOLETE (for compatibility only).*/
{
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    XmRowColumnPart	row_column;
} XmRowColumnWidgetRec;



/* RowColumn class structure */

typedef struct _XmRowColumnClassPart
{
    XmMenuProc	menuProcedures; /* proc to interface with menu widgets */
    XtActionProc armAndActivate; /* proc triggered by acclerator */
    XmMenuTraversalProc traversalHandler;/* proc to handle menu traversal */
    XtPointer   extension;      /* Pointer to extension record */
} XmRowColumnClassPart;



typedef struct _XmRowColumnClassRec 
{
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    XmRowColumnClassPart	row_column_class;
} XmRowColumnClassRec, *XmRowColumnClass;

externalref XmRowColumnClassRec     xmRowColumnClassRec;



/* Constraint Definition */

/* No constraint resources */

typedef struct _XmRowColumnConstraintPart
{
	Boolean	was_managed;
        Dimension margin_top;
        Dimension margin_bottom;
        Dimension baseline;
	short position_index;
} XmRowColumnConstraintPart;

typedef struct _XmRowColumnConstraintRec
{
	XmManagerConstraintPart manager;
	XmRowColumnConstraintPart row_column;
} XmRowColumnConstraintRec;


/* Access macros */

#define XmRC_ARMED_BIT	      (1 << 0)	
#define XmRC_BEING_ARMED_BIT  (1 << 1)		/* bits in menu's armed byte */
#define XmRC_EXPOSE_BIT       (1 << 2)		/* used in both menu and */
#define XmRC_WINDOW_MOVED_BIT (1 << 3)		/* popup menu, careful */
#define XmRC_WIDGET_MOVED_BIT (1 << 4)
#define XmRC_POPPING_DOWN_BIT (1 << 5)
#define XmRC_FROM_RESIZE_BIT  (1 << 6)

#define RC_IsArmed(m)	 (((XmRowColumnWidget)(m))->row_column.armed & XmRC_ARMED_BIT)
#define RC_BeingArmed(m) (((XmRowColumnWidget)(m))->row_column.armed & XmRC_BEING_ARMED_BIT)
#define RC_DoExpose(m)	 (((XmRowColumnWidget)(m))->row_column.armed & XmRC_EXPOSE_BIT)
#define RC_WidgetHasMoved(m) (((XmRowColumnWidget)(m))->row_column.armed & XmRC_WIDGET_MOVED_BIT)
#define RC_WindowHasMoved(m) (((XmRowColumnWidget)(m))->row_column.armed & XmRC_WINDOW_MOVED_BIT)
#define RC_PoppingDown(m) (((XmRowColumnWidget)(m))->row_column.armed & XmRC_POPPING_DOWN_BIT)
#define RC_FromResize(m) (((XmRowColumnWidget)(m))->row_column.armed & XmRC_FROM_RESIZE_BIT)


#define RC_SetBit(byte,bit,v)  byte = (byte & (~bit)) | (v ? bit : 0)

#define RC_SetArmed(m,v)  RC_SetBit (((XmRowColumnWidget)(m))->row_column.armed, XmRC_ARMED_BIT, v)
#define RC_SetBeingArmed(m,v)  RC_SetBit (((XmRowColumnWidget)(m))->row_column.armed, XmRC_BEING_ARMED_BIT, v)
#define RC_SetExpose(m,v) RC_SetBit (((XmRowColumnWidget)(m))->row_column.armed, XmRC_EXPOSE_BIT, v)
#define RC_SetWidgetMoved(m,v) RC_SetBit(((XmRowColumnWidget)(m))->row_column.armed, XmRC_WIDGET_MOVED_BIT,v)
#define RC_SetWindowMoved(m,v) RC_SetBit(((XmRowColumnWidget)(m))->row_column.armed, XmRC_WINDOW_MOVED_BIT,v)
#define RC_SetPoppingDown(m,v) RC_SetBit(((XmRowColumnWidget)(m))->row_column.armed, XmRC_POPPING_DOWN_BIT,v)
#define RC_SetFromResize(m,v) RC_SetBit(((XmRowColumnWidget)(m))->row_column.armed, XmRC_FROM_RESIZE_BIT,v)

#define RC_MarginW(m)	    (((XmRowColumnWidget)(m))->row_column.margin_width)
#define RC_MarginH(m)	    (((XmRowColumnWidget)(m))->row_column.margin_height)

#define RC_Entry_cb(m)	    (((XmRowColumnWidget)(m))->row_column.entry_callback)
#define RC_Map_cb(m)	    (((XmRowColumnWidget)(m))->row_column.map_callback)
#define RC_Unmap_cb(m)	    (((XmRowColumnWidget)(m))->row_column.unmap_callback)

#define RC_Orientation(m)   (((XmRowColumnWidget)(m))->row_column.orientation)
#define RC_Spacing(m)	    (((XmRowColumnWidget)(m))->row_column.spacing)
#define RC_EntryBorder(m)   (((XmRowColumnWidget)(m))->row_column.entry_border)
#define RC_HelpPb(m)	    (((XmRowColumnWidget)(m))->row_column.help_pushbutton)
#define RC_DoMarginAdjust(m)  (((XmRowColumnWidget)(m))->row_column.adjust_margin)
#define RC_EntryAlignment(m)  (((XmRowColumnWidget)(m))->row_column.entry_alignment)
#define RC_EntryVerticalAlignment(m)  (((XmRowColumnWidget)(m))->row_column.entry_vertical_alignment)
#define RC_Packing(m)	   (((XmRowColumnWidget)(m))->row_column.packing)
#define RC_NCol(m)	   (((XmRowColumnWidget)(m))->row_column.num_columns)
#define RC_AdjLast(m)	   (((XmRowColumnWidget)(m))->row_column.adjust_last)
#define RC_AdjMargin(m)	   (((XmRowColumnWidget)(m))->row_column.adjust_margin)
#define RC_MemWidget(m)	   (((XmRowColumnWidget)(m))->row_column.memory_subwidget)
#define RC_CascadeBtn(m)   (((XmRowColumnWidget)(m))->row_column.cascadeBtn)
#define RC_OptionLabel(m)  (((XmRowColumnWidget)(m))->row_column.option_label)
#define RC_OptionSubMenu(m)  (((XmRowColumnWidget)(m))->row_column.option_submenu)
#define RC_RadioBehavior(m)  (((XmRowColumnWidget)(m))->row_column.radio)
#define RC_RadioAlwaysOne(m) (((XmRowColumnWidget)(m))->row_column.radio_one)
#define RC_PopupPosted(m)    (((XmRowColumnWidget)(m))->row_column.popupPosted)
#define RC_ResizeHeight(m)    (((XmRowColumnWidget)(m))->row_column.resize_height)
#define RC_ResizeWidth(m)     (((XmRowColumnWidget)(m))->row_column.resize_width)
#define RC_Type(m)           (((XmRowColumnWidget)(m))->row_column.type)
#define RC_EntryClass(m)     (((XmRowColumnWidget)(m))->row_column.entry_class)
#define RC_IsHomogeneous(m)  (((XmRowColumnWidget)(m))->row_column.homogeneous)
#define RC_Boxes(m)          (((XmRowColumnWidget)(m))->row_column.boxes)
#define RC_PopupEnabled(m)   (((XmRowColumnWidget)(m))->row_column.popup_enabled)
#define RC_MenuAccelerator(m)  (((XmRowColumnWidget)(m))->row_column.menu_accelerator)
#define RC_Mnemonic(m)   (((XmRowColumnWidget)(m))->row_column.mnemonic)
#define RC_MnemonicCharSet(m)   (((XmRowColumnWidget)(m))->row_column.mnemonicCharSet)
#define RC_MenuPost(m) (((XmRowColumnWidget) m)->row_column.menuPost)
#define RC_PostButton(m) (((XmRowColumnWidget) m)->row_column.postButton)
#define RC_PostModifiers(m) (((XmRowColumnWidget) m)->row_column.postModifiers)
#define RC_PostEventType(m) (((XmRowColumnWidget) m)->row_column.postEventType)

#define RC_OldFocusPolicy(m) (((XmRowColumnWidget) m)->row_column.oldFocusPolicy)
#define RC_ParentShell(m) (((XmRowColumnWidget) m)->row_column.ParentShell)
#define RC_TearOffControl(m) (((XmRowColumnWidget) m)->row_column.tear_off_control)
#define RC_TearOffModel(m) (((XmRowColumnWidget) m)->row_column.TearOffModel)

#define RC_popupMenuClick(m) (((XmRowColumnWidget)(m))->row_column.popup_menu_click)

#define RC_TearOffTitle(m) (((XmRowColumnWidget)(m))->row_column.tear_off_title)

/* Tear Off State */
 
#define XmTO_TORN_OFF_BIT	(1 << 0)
#define XmTO_FROM_INIT_BIT	(1 << 1)
#define XmTO_VISUAL_DIRTY_BIT	(1 << 2)
#define XmTO_ACTIVE_BIT		(1 << 3)

#define RC_SetTornOff(m,v) RC_SetBit(((XmRowColumnWidget)(m))->row_column.to_state, XmTO_TORN_OFF_BIT,v)

#define RC_TornOff(m) (((XmRowColumnWidget)(m))->row_column.to_state & XmTO_TORN_OFF_BIT)

#define RC_SetFromInit(m,v) RC_SetBit(((XmRowColumnWidget)(m))->row_column.to_state, XmTO_FROM_INIT_BIT,v)

#define RC_FromInit(m)	(((XmRowColumnWidget)(m))->row_column.to_state & XmTO_FROM_INIT_BIT)

#define RC_SetTearOffDirty(m,v) RC_SetBit(((XmRowColumnWidget)(m))->row_column.to_state, XmTO_VISUAL_DIRTY_BIT,v)

#define RC_TearOffDirty(m) (((XmRowColumnWidget)(m))->row_column.to_state & XmTO_VISUAL_DIRTY_BIT)

#define RC_TearOffActive(m) (((XmRowColumnWidget)(m))->row_column.to_state & XmTO_ACTIVE_BIT)

#define RC_SetTearOffActive(m,v) RC_SetBit(((XmRowColumnWidget)(m))->row_column.to_state, XmTO_ACTIVE_BIT,v)

#define initial_value 0

/* Defines used when calling _XmProcessMenuTree() */

#define XmADD     0
#define XmDELETE  1
#define XmREPLACE 2


/* Defines used when calling _XmMenuIsAccessible() */

#define XmWEAK_CHECK 1
#define XmMEDIUM_CHECK 2
#define XmSTRONG_CHECK 3

#define XmMENU_BEGIN 0
#define XmMENU_MIDDLE 1
#define XmMENU_END 2

/* Defines used when calling find_first_managed_child() */
#define ANY_CHILD 0
#define FIRST_BUTTON 1


#define XmInheritMenuProceduresProc	((XmMenuProc) _XtInherit)
#define XmInheritArmAndActivateProc	((XtActionProc) _XtInherit)
#define XmInheritMenuTraversalProc	((XmMenuTraversalProc) _XtInherit)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* _XmRowColumnP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

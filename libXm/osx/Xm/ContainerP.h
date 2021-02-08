/* $XConsortium: ContainerP.h /main/8 1996/06/13 16:45:53 pascale $ */
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
#ifndef	_XmContainerP_h
#define _XmContainerP_h
 
#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/DragCP.h>
#include <Xm/Container.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * One _XmCwidNodeRec structure is allocated by Container for each of 
 * it's children (except for OutlineButtons).  Information about the 
 * relationship of the child to other Container children (parentage and
 * order) is maintained here by Container.
 *
 * _XmCwidNodeRec structures are XtCalloc'd by Container in the 
 * ConstraintInitialize method and XtFree'd in the ConstraintDestroy method.
 * They are linked/unlinked to other _XmCwidNodeRec structures in the
 * ChangeManaged method.
 */
typedef struct _XmCwidNodeRec
        {
	struct	_XmCwidNodeRec *	next_ptr;
	struct	_XmCwidNodeRec *	prev_ptr;
	struct	_XmCwidNodeRec *	child_ptr;
	struct	_XmCwidNodeRec *	parent_ptr;
	Widget			widget_ptr;
	}	XmCwidNodeRec, *CwidNode;

/*
 * Container allocates a _XmContainerXfrActionRec structure to store
 * the data from a ContainerStartTransfer action until it can determine
 * whether the action should start a primary transfer or begin a drag.
 */
typedef	struct	_XmContainerXfrActionRec
	{
	Widget		wid;
	XEvent		*event;
	String		*params;
	Cardinal	*num_params;
	Atom		operation;
	}	XmContainerXfrActionRec, *ContainerXfrAction;

/*
 * Container allocates an array of _XmContainerCwidCellInfoRec structures
 * to use in calculating an ideal size in the GetSpatialSize procedure when
 * XmNspatialStyle is XmCELLS.  The array is created and destroyed in the 
 * GetSpatialSize procedure.
 */
typedef	struct	_XmContainerCwidCellInfoRec
	{
	int	cwid_width_in_cells;
	int	cwid_height_in_cells;
	}	XmContainerCwidCellInfoRec, *ContainerCwidCellInfo;

/* Container constraint class part record */
typedef	struct	_XmContainerConstraintPart
	{
	Widget		entry_parent;		/* XmNentryParent */
	Widget		related_cwid;
	CwidNode	node_ptr;
	int		position_index;		/* XmNpositionIndex */
	int		depth;
	int		cell_idx;
	Boolean		visible_in_outline;
	Position	user_x;
	Position	user_y;
	unsigned char	outline_state;		/* XmNoutlineState */
	unsigned char	selection_visual;
	unsigned char	selection_state;
	unsigned char	cwid_type;
	}	XmContainerConstraintPart, * XmContainerConstraint;

typedef	struct	_XmContainerConstraintRec
	{
	XmManagerConstraintPart		manager;
	XmContainerConstraintPart	container;
	}	XmContainerConstraintRec, * XmContainerConstraintPtr;

/* move the other typedef here */
typedef void (*XmSpatialGetSize)(Widget, Dimension *, Dimension *);

/* Container widget class record  */
typedef	struct	_XmContainerClassPart
	{
	XmSpatialTestFitProc		test_fit_item;
	XmSpatialPlacementProc		place_item;
	XmSpatialRemoveProc		remove_item;
	XmSpatialGetSize		get_spatial_size;
	XtPointer			extension;
	}	XmContainerClassPart;

/* Full class record declaration */
typedef	struct	_XmContainerClassRec
	{
	CoreClassPart		core_class;
	CompositeClassPart  	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	XmContainerClassPart		container_class;
	}	XmContainerClassRec, *XmContainerClass;

externalref	XmContainerClassRec	xmContainerClassRec;

/* Container instance record */
typedef	struct	_XmContainerPart
	{
	WidgetList      selected_items;         /* XmNselectedItems */
	Widget		icon_header;
	Widget		anchor_cwid;
	Widget		druggee;
	Widget		size_ob;
	Widget		drag_context;
	CwidNode        first_node;
        CwidNode        last_node;
	Cardinal *      detail_order;           /* XmNdetailOrder */
	XmString *	detail_heading;         /* XmNdetailColumnHeading */
	XSegment *      outline_segs;
	XtCallbackList  convert_cb;             /* XmNconvertCallback */
        XtCallbackList  default_action_cb;      /* XmNdefaultActionCallback */
        XtCallbackList  destination_cb;         /* XmNdestinationCallback */
	XtCallbackList  outline_cb;             /* XmNoutlineChangedCallback */
	XtCallbackList  selection_cb;           /* XmNselectionCallback */
	XmTabList       detail_tablist;         /* XmNdetailTabList */
	XmFontList      render_table;           /* XmNfontList */
	Pixel		select_color;		/* XmNselectColor */
	Pixmap		collapsed_state_pixmap;	/* XmNcollapsedStatePixmap */
	Pixmap		expanded_state_pixmap;	/* XmNexpandedStatePixmap */
	GC		normalGC;
	GC              marqueeGC;
        Time            last_click_time;
	Region		cells_region;
	ContainerXfrAction transfer_action;
	XtIntervalId    transfer_timer_id;
	XPoint		anchor_point;
	XPoint		marquee_start;
	XPoint		marquee_end;
	XPoint		marquee_smallest;
	XPoint		marquee_largest;
	XPoint		dropspot;
	unsigned long	dynamic_resource;
	int		max_depth;
	int		outline_seg_count;
	int	 	*cells;
	int		cell_count;
	int		next_free_cell;
	int		current_width_in_cells;
	int		current_height_in_cells;
	int		drag_offset_x;
	int		drag_offset_y;
	unsigned int    selected_item_count;    /* XmNselectedItemCount */
	Cardinal        detail_heading_count; /* XmNdetailColumnHeadingCount */
        Cardinal        saved_detail_heading_count; 
        Cardinal        detail_order_count;     /* XmNdetailOrderCount */
	Dimension       first_col_width;        /* XmNoutlineColumnWidth */
	Dimension       real_first_col_width;    
	Dimension       large_cell_height;      /* XmNlargeCellHeight */
        Dimension       large_cell_width;       /* XmNlargeCellWidth */
	Dimension	small_cell_height;	/* XmNsmallCellHeight */
	Dimension	small_cell_width;	/* XmNsmallCellWidth */
	Dimension	real_large_cellh;
	Dimension	real_large_cellw;
	Dimension	real_small_cellh;
	Dimension	real_small_cellw;
        Dimension       margin_h;               /* XmNmarginHeight */
        Dimension       margin_w;               /* XmNmarginWidth */
        Dimension       outline_indent;         /* XmNoutlineIndentation */
	Dimension       ob_width;
	Dimension       ob_height;
	Dimension       prev_width;
        Dimension       ideal_width;
        Dimension       ideal_height;
	/* Note: first_change_managed is also used to resolve between
	 * XmRenderTable & XmFontList when setting up the resource table
	 */
	Boolean		first_change_managed;
	Boolean         extending_mode;
        Boolean         marquee_mode;
        Boolean         self;
        Boolean         toggle_pressed;
        Boolean         extend_pressed;
	Boolean		ob_pressed;
        Boolean         cancel_pressed;
        Boolean         kaddmode;
        Boolean         no_auto_sel_changes;
        Boolean         started_in_anchor;
	Boolean         marquee_drawn;
	Boolean         have_primary;
	Boolean         selecting;
	Boolean		large_cell_dim_fixed;
	Boolean		small_cell_dim_fixed;
	unsigned char   automatic;              /* XmNautomaticSelection */
	unsigned char   entry_viewtype;         /* XmNentryViewType */
	unsigned char   include_model;          /* XmNspatialIncludeModel */
        unsigned char   layout_type;            /* XmNlayoutType */
	unsigned char   ob_policy;         	/* XmNoutlineButtonPolicy */
	unsigned char   outline_sep_style;      /* XmNoutlineLineStyle */
        unsigned char   spatial_style;          /* XmNspatialStyle */
        unsigned char   primary_ownership;      /* XmNprimaryOwnership */
        unsigned char   resize_model;           /* XmNspatialResizeModel */
	unsigned char   selection_policy;       /* XmNselectionPolicy */
        unsigned char   selection_technique;    /* XmNselectionTechnique */
	unsigned char   snap_model;             /* XmNspatialSnapModel */
	unsigned char   create_cwid_type;
        unsigned char   selection_state;
	unsigned char	LeaveDir;               /* leave direction */
	XtIntervalId    scroll_proc_id;         /* scroll TimeOutProc */
	int             last_xmotion_x;
	int             last_xmotion_y;
	XmString *	cache_detail_heading;  /* XmNdetailColumnHeading 
						  getValues */
	}	XmContainerPart;

/* Full instance record declaration */
typedef	struct	_XmContainerRec
	{
	CorePart	core;
	CompositePart   composite;
	ConstraintPart	constraint;
	XmManagerPart	manager;
	XmContainerPart	container;
	}	XmContainerRec;

/* enums to keep up with cwid types */
enum {	CONTAINER_ICON,
	CONTAINER_OUTLINE_BUTTON,
	CONTAINER_HEADER};

#define	TABLIST	(1L<<0)
#define	FIRSTCW (1L<<1)
#define	CtrIsDynamic(w,mask) \
	(((XmContainerWidget)(w))->container.dynamic_resource & mask)

#define CtrDynamicSmallCellHeight(w) \
	(((XmContainerWidget)(w))->container.small_cell_height == 0)
#define	CtrDynamicSmallCellWidth(w) \
	(((XmContainerWidget)(w))->container.small_cell_width == 0)
#define CtrDynamicLargeCellHeight(w) \
        (((XmContainerWidget)(w))->container.large_cell_height == 0)
#define CtrDynamicLargeCellWidth(w) \
        (((XmContainerWidget)(w))->container.large_cell_width == 0)

#define CtrIsAUTO_SELECT(w) \
        ((((XmContainerWidget)(w))->container.automatic == XmAUTO_SELECT) && \
         (((XmContainerWidget)(w))->container.selection_policy \
                                                        != XmSINGLE_SELECT))
#define	CtrViewIsLARGE_ICON(w) \
	(((XmContainerWidget)(w))->container.entry_viewtype == XmLARGE_ICON)
#define CtrViewIsSMALL_ICON(w) \
	(((XmContainerWidget)(w))->container.entry_viewtype == XmSMALL_ICON)
#define	CtrViewIsANY_ICON(w) \
	(((XmContainerWidget)(w))->container.entry_viewtype == XmANY_ICON)
#define	CtrIsHORIZONTAL(w) \
	(XmDirectionMatchPartial \
	(((XmContainerWidget)(w))->manager.string_direction,\
				XmDEFAULT_DIRECTION,XmPRECEDENCE_HORIZ_MASK))
#define	CtrIsVERTICAL(w) \
	(XmDirectionMatchPartial \
	(((XmContainerWidget)(w))->manager.string_direction,\
				XmDEFAULT_DIRECTION,XmPRECEDENCE_VERT_MASK))
#define	CtrLayoutIsDETAIL(w) \
	(((XmContainerWidget)(w))->container.layout_type == XmDETAIL)
#define CtrLayoutIsOUTLINE_DETAIL(w) \
	((((XmContainerWidget)(w))->container.layout_type == XmDETAIL) || \
	(((XmContainerWidget)(w))->container.layout_type == XmOUTLINE))

#define CtrDrawLinesOUTLINE(w) \
	(CtrLayoutIsOUTLINE_DETAIL(w) && \
	 (((XmContainerWidget)(w))->container.outline_sep_style \
					== XmSINGLE))
#define CtrLayoutIsSPATIAL(w) \
	(((XmContainerWidget)(w))->container.layout_type == XmSPATIAL)
#define	CtrSpatialStyleIsNONE(w) \
	(((XmContainerWidget)(w))->container.spatial_style == XmNONE)
#define CtrSpatialStyleIsGRID(w) \
	(((XmContainerWidget)(w))->container.spatial_style == XmGRID)
#define CtrSpatialStyleIsCELLS(w) \
	(((XmContainerWidget)(w))->container.spatial_style == XmCELLS)
#define	CtrIncludeIsAPPEND(w) \
	(((XmContainerWidget)(w))->container.include_model == XmAPPEND)
#define	CtrIncludeIsCLOSEST(w) \
	(((XmContainerWidget)(w))->container.include_model == XmCLOSEST)
#define	CtrIncludeIsFIRST_FIT(w) \
	(((XmContainerWidget)(w))->container.include_model == XmFIRST_FIT)
#define	CtrSnapModelIsNONE(w) \
	(((XmContainerWidget)(w))->container.snap_model == XmNONE)
#define CtrSnapModelIsSNAP(w) \
	(((XmContainerWidget)(w))->container.snap_model == XmSNAP_TO_GRID)
#define CtrSnapModelIsCENTER(w) \
	(((XmContainerWidget)(w))->container.snap_model == XmCENTER)
#define	CtrResizeModelIsGROW_MINOR(w) \
	(((XmContainerWidget)(w))->container.resize_model == XmGROW_MINOR)
#define CtrResizeModelIsGROW_MAJOR(w) \
	(((XmContainerWidget)(w))->container.resize_model == XmGROW_MAJOR)
#define CtrResizeModelIsGROW_BALANCED(w) \
	(((XmContainerWidget)(w))->container.resize_model == XmGROW_BALANCED)
#define	CtrPolicyIsSINGLE(w) \
	(((XmContainerWidget)(w))->container.selection_policy \
					== XmSINGLE_SELECT)
#define	CtrPolicyIsBROWSE(w) \
	(((XmContainerWidget)(w))->container.selection_policy \
					== XmBROWSE_SELECT)
#define	CtrPolicyIsMULTIPLE(w) \
	(((XmContainerWidget)(w))->container.selection_policy \
					== XmMULTIPLE_SELECT)
#define	CtrPolicyIsEXTENDED(w) \
	(((XmContainerWidget)(w))->container.selection_policy \
					== XmEXTENDED_SELECT)
#define	CtrTechIsTOUCH_OVER(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmTOUCH_OVER)
#define	CtrTechIsTOUCH_ONLY(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmTOUCH_ONLY)
#define	CtrTechIsMARQUEE(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmMARQUEE)
#define CtrTechIsMARQUEE_ES(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmMARQUEE_EXTEND_START)
#define	CtrTechIsMARQUEE_EB(w) \
	(((XmContainerWidget)(w))->container.selection_technique \
					== XmMARQUEE_EXTEND_BOTH)
#define	CtrOB_PRESENT(w) \
	(((XmContainerWidget)(w))->container.ob_policy \
					== XmOUTLINE_BUTTON_PRESENT)
#define	CtrOB_ABSENT(w) \
	(((XmContainerWidget)(w))->container.ob_policy \
					== XmOUTLINE_BUTTON_ABSENT)
#define GetContainerConstraint(w) \
	(&((XmContainerConstraintPtr) (w)->core.constraints)->container)
#define	CtrItemIsPlaced(w) \
	(((XmContainerConstraintPtr)(w)->core.constraints)->container.cell_idx \
					!= NO_CELL)
#define	CtrICON(w) \
	(((XmContainerConstraintPtr)(w)->core.constraints)->container.cwid_type\
					== CONTAINER_ICON)
#define CtrOUTLINE_BUTTON(w) \
	(((XmContainerConstraintPtr)(w)->core.constraints)->container.cwid_type\
					== CONTAINER_OUTLINE_BUTTON)
#define	CtrHEADER(w) \
	(((XmContainerConstraintPtr)(w)->core.constraints)->container.cwid_type\
					== CONTAINER_HEADER)

#define	XmInheritSpatialTestFitProc	((XmSpatialTestFitProc) _XtInherit)
#define	XmInheritSpatialPlacementProc	((XmSpatialPlacementProc) _XtInherit)
#define	XmInheritSpatialRemoveProc	((XmSpatialRemoveProc) _XtInherit)
#define	XmInheritSpatialGetSize		((XmSpatialGetSize) _XtInherit)

/* possible directions when leaving the container */
#define	TOPLEAVE	(1<<0)
#define	BOTTOMLEAVE	(1<<1)
#define	LEFTLEAVE	(1<<2)
#define	RIGHTLEAVE	(1<<3)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmContainerP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */



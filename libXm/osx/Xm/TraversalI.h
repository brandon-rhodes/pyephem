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
/*   $XConsortium: TraversalI.h /main/11 1995/07/13 18:16:58 drk $ */
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTraversalI_h
#define _XmTraversalI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif


#define NavigTypeIsTabGroup(navigation_type) \
  ((navigation_type == XmTAB_GROUP) || \
   (navigation_type == XmSTICKY_TAB_GROUP) || \
   (navigation_type == XmEXCLUSIVE_TAB_GROUP))


typedef enum {
    XmUnrelated,
    XmMyAncestor,
    XmMyDescendant,
    XmMyCousin,
    XmMySelf
} XmGeneology;

typedef struct _XmTravGraphRec
{   
    union _XmTraversalNodeRec *head ;
    Widget top ;
    union _XmTraversalNodeRec *current ;
    unsigned short num_entries ;
    unsigned short num_alloc ;
    unsigned short next_alloc ;
    unsigned short exclusive ;
    unsigned short tab_list_alloc ;
    unsigned short num_tab_list ;
    Widget *excl_tab_list ;
} XmTravGraphRec, * XmTravGraph ;


typedef struct _XmFocusDataRec {
    Widget	active_tab_group;
    Widget	focus_item;
    Widget	old_focus_item;
    Widget	pointer_item;
    Widget	old_pointer_item;
    Boolean	needToFlush;
    XCrossingEvent lastCrossingEvent;
    XmGeneology focalPoint;
    unsigned char focus_policy ; /* Mirrors focus_policy resource when focus */
    XmTravGraphRec trav_graph ;  /*   data retrieved using _XmGetFocusData().*/
    Widget      first_focus ;
} XmFocusDataRec ;

typedef enum
{
  XmTAB_GRAPH_NODE, XmTAB_NODE, XmCONTROL_GRAPH_NODE, XmCONTROL_NODE
} XmTravGraphNodeType ;

typedef union _XmDeferredGraphLink
{
  int offset ;
  struct _XmGraphNodeRec *link ;
} XmDeferredGraphLink ;

typedef struct _XmAnyNodeRec               /* Common */
{
  unsigned char type ;
  XmNavigationType nav_type ;
  XmDeferredGraphLink tab_parent ;
  Widget widget ;
  XRectangle rect ;
  union _XmTraversalNodeRec *next ;
  union _XmTraversalNodeRec *prev ;
} XmAnyNodeRec, *XmAnyNode ;

typedef struct _XmControlNodeRec
{
  XmAnyNodeRec any ;
  union _XmTraversalNodeRec *up ;
  union _XmTraversalNodeRec *down ;
} XmControlNodeRec, *XmControlNode ;

typedef struct _XmTabNodeRec
{
  XmAnyNodeRec any ;
} XmTabNodeRec, *XmTabNode ;

typedef struct _XmGraphNodeRec
{
  XmAnyNodeRec any ;
  union _XmTraversalNodeRec *sub_head ;
  union _XmTraversalNodeRec *sub_tail ;
} XmGraphNodeRec, *XmGraphNode ;

typedef union _XmTraversalNodeRec
{
  XmAnyNodeRec any ;
  XmControlNodeRec control ;
  XmTabNodeRec tab ;
  XmGraphNodeRec graph ;
} XmTraversalNodeRec, *XmTraversalNode ;

typedef struct
{
  XmTraversalNode *items;
  XmTraversalNode lead_item;
  Cardinal num_items;
  Cardinal max_items;
  Position min_hint;
  Position max_hint;
} XmTraversalRow;


/********    Private Function Declarations for Traversal.c    ********/

extern XmFocusData _XmCreateFocusData( void ) ;
extern void _XmDestroyFocusData( 
                        XmFocusData focusData) ;
extern void _XmSetActiveTabGroup( 
                        XmFocusData focusData,
                        Widget tabGroup) ;
extern Widget _XmGetActiveItem( 
                        Widget w) ;
extern void _XmNavigInitialize( 
                        Widget request,
                        Widget new_wid,
                        ArgList args,
                        Cardinal *num_args) ;
extern Boolean _XmNavigSetValues( 
                        Widget current,
                        Widget request,
                        Widget new_wid,
                        ArgList args,
                        Cardinal *num_args) ;
extern void _XmNavigResize( 
                        Widget wid) ;
extern void _XmValidateFocus( 
                        Widget wid) ;
extern void _XmNavigDestroy( 
                        Widget wid) ;
extern Boolean _XmCallFocusMoved( 
                        Widget old,
                        Widget new_wid,
                        XEvent *event) ;
extern Boolean _XmMgrTraversal( 
                        Widget wid,
                        XmTraversalDirection direction) ;
extern void _XmClearFocusPath( 
                        Widget wid) ;
extern Boolean _XmFocusIsHere( 
                        Widget w) ;
extern unsigned char _XmGetFocusPolicy( 
                        Widget w) ;
extern Widget _XmFindTopMostShell( 
                        Widget w) ;
extern void _XmFocusModelChanged( 
                        Widget wid,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern XmFocusData _XmGetFocusData( 
                        Widget wid) ;
extern Boolean _XmComputeVisibilityRect( 
                        Widget w,
                        XRectangle *rectPtr,
			Boolean include_initial_border,
			Boolean allow_scrolling) ;
extern Boolean _XmGetPointVisibility(Widget w,
				     int root_x, 
				     int root_y);
extern void _XmSetRect( 
                        register XRectangle *rect,
                        Widget w) ;
extern int _XmIntersectRect( 
                        register XRectangle *srcRectA,
                        register Widget widget,
                        register XRectangle *dstRect) ;
extern int _XmEmptyRect( 
                        register XRectangle *r) ;
extern void _XmClearRect( 
                        register XRectangle *r) ;
extern Boolean _XmIsNavigable( 
                        Widget wid) ;
extern void _XmWidgetFocusChange( 
                        Widget wid,
                        XmFocusChange change) ;
extern Widget _XmNavigate( 
                        Widget wid,
                        XmTraversalDirection direction) ;
extern void _XmSetInitialOfTabGroup( 
                        Widget tab_group,
                        Widget init_focus) ;
extern void _XmResetTravGraph( 
                        Widget wid) ;
extern Boolean _XmShellIsExclusive( 
                        Widget wid) ;
extern Widget _XmGetFirstFocus( 
                        Widget wid) ;

/********    End Private Function Declarations    ********/

/********    Private Function Declarations for TraversalI.c    ********/

extern XmNavigability _XmGetNavigability( 
                        Widget wid) ;
extern Boolean _XmIsViewable( 
                        Widget wid) ;
extern Widget _XmIsScrollableClipWidget( 
                        Widget work_window,
			Boolean scrollable,
                        XRectangle *visRect) ;
extern Boolean _XmGetEffectiveView( 
                        Widget wid,
                        XRectangle *visRect) ;
extern Boolean _XmIntersectionOf( 
                        register XRectangle *srcRectA,
                        register XRectangle *srcRectB,
                        register XRectangle *destRect) ;
extern XmNavigationType _XmGetNavigationType( 
                        Widget widget) ;
extern Widget _XmGetActiveTabGroup( 
                        Widget wid) ;
extern Widget _XmTraverseAway( 
                        XmTravGraph list,
                        Widget wid,
#if NeedWidePrototypes
                        int wid_is_control) ;
#else
                        Boolean wid_is_control) ;
#endif /* NeedWidePrototypes */
extern Widget _XmTraverse( 
                        XmTravGraph list,
                        XmTraversalDirection action,
                        XmTraversalDirection *local_dir,
                        Widget reference_wid) ;
extern void _XmFreeTravGraph( 
                        XmTravGraph trav_list) ;
extern void _XmTravGraphRemove( 
                        XmTravGraph tgraph,
                        Widget wid) ;
extern void _XmTravGraphAdd( 
                        XmTravGraph tgraph,
                        Widget wid) ;
extern void _XmTravGraphUpdate( 
                        XmTravGraph tgraph,
                        Widget wid) ;
extern Boolean _XmNewTravGraph( 
                        XmTravGraph trav_list,
                        Widget top_wid,
                        Widget init_current) ;
extern Boolean _XmSetInitialOfTabGraph( 
                        XmTravGraph trav_graph,
                        Widget tab_group,
                        Widget init_focus) ;
extern void _XmTabListAdd( 
                        XmTravGraph graph,
                        Widget wid) ;
extern void _XmTabListDelete( 
                        XmTravGraph graph,
                        Widget wid) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTraversalI_h */

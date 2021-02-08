/*
 * @OPENGROUP_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF for
 * the full copyright text.
 * 
 * This software is subject to an open license. It may only be
 * used on, with or for operating systems which are themselves open
 * source systems. You must contact The Open Group for a license
 * allowing distribution and sublicensing of this software on, with,
 * or for operating systems which are not Open Source programs.
 * 
 * See http://www.opengroup.org/openmotif/license for full
 * details of the license agreement. Any use, reproduction, or
 * distribution of the program constitutes recipient's acceptance of
 * this agreement.
 * 
 * EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 * PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 * WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 * OR FITNESS FOR A PARTICULAR PURPOSE
 * 
 * EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 * NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 * EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 */
/*
 * HISTORY
 */
/* $XConsortium: RCMenuI.h /main/5 1995/07/13 17:45:45 drk $ */
#ifndef _XmRCMenuI_h
#define _XmRCMenuI_h

#include <Xm/RowColumn.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern void _XmMenuBtnUp(
        Widget wid,
        XEvent *event,
        String *params,
        Cardinal *num_params);
extern void _XmMenuBtnDown(
        Widget wid,
        XEvent *event,
        String *params,
        Cardinal *num_params);
extern void _XmHandleMenuButtonPress(
				     Widget wid,
				     XEvent *event);
extern Boolean _XmMatchBDragEvent( 
				    Widget wid,
				    XEvent *event);
extern Boolean _XmMatchBSelectEvent( 
				    Widget wid,
				    XEvent *event);
extern void _XmGetActiveTopLevelMenu(
				     Widget wid,
				     Widget *rwid);
extern void _XmMenuFocus(
			 Widget w,
			 int operation,
			 Time _time );
extern void _XmSetSwallowEventHandler(
				      Widget widget, 
#if NeedWidePrototypes
				      int add_handler );
#else
                                      Boolean add_handler );
#endif /* NeedWidePrototypes */
extern void _XmMenuPopDown(
			   Widget w,
			   XEvent *event,
			   Boolean *popped_up );
extern Boolean _XmGetPopupMenuClick(
				    Widget wid );
extern void _XmSetPopupMenuClick(
				 Widget wid,
#if NeedWidePrototypes
				 int popupMenuClick);
#else
                                 Boolean popupMenuClick);
#endif /* NeedWidePrototypes */
extern void _XmRC_DoProcessMenuTree( 
                        Widget w,
                        int mode) ;

extern void _XmRC_ProcessSingleWidget( 
                        Widget w,
                        int mode) ;
extern void _XmRC_AddToPostFromList( 
                        XmRowColumnWidget m,
                        Widget widget) ;
extern void _XmRC_UpdateOptionMenuCBG( 
                        Widget cbg,
                        Widget memWidget) ;
extern void _XmRC_SetMenuHistory( 
                        XmRowColumnWidget m,
                        RectObj child) ;
extern void _XmRC_SetOptionMenuHistory( 
                        XmRowColumnWidget m,
                        RectObj child) ;
extern void _XmRCMenuProcedureEntry(
				    int proc,
				    Widget widget,
				    ... ) ;
extern void _XmRCArmAndActivate(
				Widget w,
				XEvent *event,
				String *parms,
				Cardinal *num_parms );
extern void _XmRCGetTopManager(
			       Widget w,
			       Widget *topManager ) ;
extern void _XmMenuFocusOut( 
                        Widget cb,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmMenuFocusIn( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmGetMenuKidMargins( 
                        XmRowColumnWidget m,
                        Dimension *width,
                        Dimension *height,
                        Dimension *left,
                        Dimension *right,
                        Dimension *top,
                        Dimension *bottom) ;
extern void _XmMenuUnmap( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmMenuBarGadgetSelect( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmMenuGadgetTraverseCurrent(
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmMenuGadgetTraverseCurrentUp(
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmMenuGadgetDrag(
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern int _XmRC_PopupMenuHandler(Widget, XEvent*);

extern Boolean _XmRC_PostTimeOut( XtPointer wid );

extern void _XmRC_RemoveHandlersFromPostFromWidget( 
                        Widget popup,
                        Widget widget) ;
extern void _XmRC_AddPopupEventHandlers( 
                        XmRowColumnWidget pane) ;
extern void _XmRC_RemovePopupEventHandlers( 
                        XmRowColumnWidget pane) ;

void _XmRC_RemoveFromPostFromList( 
                        XmRowColumnWidget m,
                        Widget widget) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmRCMenuI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

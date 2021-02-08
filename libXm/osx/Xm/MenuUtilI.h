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
/* $XConsortium: MenuUtilI.h /main/6 1996/07/23 16:45:49 pascale $ */
#ifndef _XmMenuUtilI_h
#define _XmMenuUtilI_h

#include <Xm/MenuUtilP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern void _XmMenuHelp(
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern Boolean _XmIsActiveTearOff (
                         Widget w) ;
extern int _XmGrabPointer( 
                        Widget widget,
                        int owner_events,
                        unsigned int event_mask,
                        int pointer_mode,
                        int keyboard_mode,
                        Window confine_to,
                        Cursor cursor,
                        Time time) ;
extern int _XmGrabKeyboard( 
                        Widget widget,
                        int owner_events,
                        int pointer_mode,
                        int keyboard_mode,
                        Time time) ;
extern int _XmMenuGrabKeyboardAndPointer(
                        Widget widget,
			Time time) ; 
extern void _XmMenuSetInPMMode( 
			Widget wid,
#if NeedWidePrototypes
                        int flag) ;
#else
                        Boolean flag) ;
#endif /* NeedWidePrototypes */
extern void _XmSetMenuTraversal( 
                        Widget wid,
#if NeedWidePrototypes
                        int traversalOn) ;
#else
                        Boolean traversalOn) ;
#endif /* NeedWidePrototypes */
extern void _XmLeafPaneFocusOut( 
                        Widget wid) ;
extern void _XmMenuTraverseLeft( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmMenuTraverseRight( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmMenuTraverseUp( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmMenuTraverseDown( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmMenuEscape( 
                        Widget w,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmRC_GadgetTraverseDown( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmRC_GadgetTraverseUp( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmRC_GadgetTraverseLeft( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmRC_GadgetTraverseRight( 
                        Widget wid,
                        XEvent *event,
                        String *param,
                        Cardinal *num_param) ;
extern void _XmMenuTraversalHandler( 
                        Widget w,
                        Widget pw,
                        XmTraversalDirection direction) ;
/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMenuUtilI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

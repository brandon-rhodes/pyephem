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
/* $XConsortium: TextFI.h /main/5 1995/07/13 18:05:32 drk $ */
#ifndef _XmTextFI_h
#define _XmTextFI_h

#include <Xm/TextFP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern int _XmTextFieldCountBytes( 
                        XmTextFieldWidget tf,
                        wchar_t *wc_value,
                        int num_chars) ;
extern void _XmTextFToggleCursorGC( 
                        Widget widget) ;
extern void _XmTextFieldDrawInsertionPoint( 
                        XmTextFieldWidget tf,
#if NeedWidePrototypes
                        int turn_on) ;
#else
                        Boolean turn_on) ;
#endif /* NeedWidePrototypes */
extern void _XmTextFieldSetClipRect( 
                        XmTextFieldWidget tf) ;
extern void _XmTextFieldSetCursorPosition( 
                        XmTextFieldWidget tf,
                        XEvent *event,
                        XmTextPosition position,
#if NeedWidePrototypes
                        int adjust_flag,
                        int call_cb) ;
#else
                        Boolean adjust_flag,
                        Boolean call_cb) ;
#endif /* NeedWidePrototypes */
extern Boolean _XmTextFieldReplaceText( 
                        XmTextFieldWidget tf,
                        XEvent *event,
                        XmTextPosition replace_prev,
                        XmTextPosition replace_next,
                        char *insert,
                        int insert_length,
#if NeedWidePrototypes
                        int move_cursor) ;
#else
                        Boolean move_cursor) ;
#endif /* NeedWidePrototypes */
extern void _XmTextFieldDeselectSelection( 
                        Widget w,
#if NeedWidePrototypes
                        int disown,
#else
                        Boolean disown,
#endif /* NeedWidePrototypes */
                        Time sel_time) ;
extern Boolean _XmTextFieldSetDestination( 
                        Widget w,
                        XmTextPosition position,
                        Time set_time) ;
extern void _XmTextFieldStartSelection( 
                        XmTextFieldWidget tf,
                        XmTextPosition left,
                        XmTextPosition right,
                        Time sel_time) ;
extern void _XmTextFieldSetSel2( 
                        Widget w,
                        XmTextPosition left,
                        XmTextPosition right,
#if NeedWidePrototypes
                        int disown,
#else
                        Boolean disown,
#endif /* NeedWidePrototypes */
                        Time sel_time) ;
extern void _XmTextFieldHandleSecondaryFinished(Widget w,
						XEvent *event);
extern int _XmTextFieldCountCharacters(XmTextFieldWidget tf,
				       char *ptr,
				       int n_bytes);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextFI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

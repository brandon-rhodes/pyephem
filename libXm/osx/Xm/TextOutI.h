/* * @OPENGROUP_COPYRIGHT@
/* * COPYRIGHT NOTICE
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
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
/*
 * HISTORY
 */
/* $XConsortium: TextOutI.h /main/7 1995/11/02 12:05:38 cde-fuj $ */
#ifndef _XmTextOutI_h
#define _XmTextOutI_h

#include <Xm/TextOutP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmTextFreeContextData( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;
extern void _XmTextResetClipOrigin( 
                        XmTextWidget tw,
                        XmTextPosition position,
#if NeedWidePrototypes
                        int clip_mask_reset) ;
#else
                        Boolean clip_mask_reset) ;
#endif /* NeedWidePrototypes */
extern void _XmTextAdjustGC( 
                        XmTextWidget tw) ;
extern Boolean _XmTextShouldWordWrap( 
                        XmTextWidget widget) ;
extern Boolean _XmTextScrollable( 
                        XmTextWidget widget) ;
extern XmTextPosition _XmTextFindLineEnd( 
                        XmTextWidget widget,
                        XmTextPosition position,
                        LineTableExtra *extra) ;
extern void _XmTextOutputGetSecResData( 
                        XmSecondaryResourceData *secResDataRtn) ;
extern int _XmTextGetNumberLines( 
                        XmTextWidget widget) ;
extern void _XmTextMovingCursorPosition( 
                        XmTextWidget tw,
                        XmTextPosition position) ;
extern void _XmTextChangeBlinkBehavior( 
                        XmTextWidget widget,
#if NeedWidePrototypes
                        int newvalue) ;
#else
                        Boolean newvalue) ;
#endif /* NeedWidePrototypes */
extern void _XmTextOutputCreate( 
                        Widget wid,
                        ArgList args,
                        Cardinal num_args) ;
extern Boolean _XmTextGetBaselines( 
                        Widget widget,
                        Dimension **baselines,
                        int *line_count) ;
extern Boolean _XmTextGetDisplayRect( 
                        Widget w,
                        XRectangle *display_rect) ;
extern void _XmTextMarginsProc( 
                        Widget w,
                        XmBaselineMargins *margins_rec) ;
extern void _XmTextChangeHOffset( 
                        XmTextWidget widget,
                        int length) ;
extern void _XmTextChangeVOffset( 
                        XmTextWidget widget,
                        int length) ;
extern void _XmTextToggleCursorGC( 
                        Widget widget) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextOutI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

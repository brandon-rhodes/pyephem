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
/*   $XConsortium: TravActI.h /main/9 1995/07/13 18:15:46 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTravActI_h
#define _XmTravActI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmFOCUS_RESET	1<<0
#define XmFOCUS_IGNORE	1<<1

/********    Private Function Declarations    ********/

extern unsigned short _XmGetFocusFlag(
			Widget w,
			unsigned int mask) ;
extern void _XmSetFocusFlag(
			Widget w,
			unsigned int mask,
#if NeedWidePrototypes
        		int value ) ;
#else
        		Boolean value ) ;
#endif /* NeedWidePrototypes */
extern void _XmTrackShellFocus( 
                        Widget widget,
                        XtPointer client_data,
                        XEvent *event,
                        Boolean *dontSwallow) ;
extern void _XmPrimitiveEnter( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveLeave( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveUnmap( 
                        Widget pw,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveFocusInInternal( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveFocusOut( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveFocusIn( 
                        Widget pw,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmEnterGadget( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmLeaveGadget( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmFocusInGadget( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmFocusOutGadget( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerEnter( 
                        Widget wid,
                        XEvent *event_in,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerLeave( 
                        Widget wid,
                        XEvent *event_in,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerFocusInInternal( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerFocusIn( 
                        Widget mw,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerFocusOut( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerUnmap( 
                        Widget mw,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTravActI_h */

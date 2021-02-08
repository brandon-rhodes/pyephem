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
/* $XConsortium: DragOverSI.h /main/6 1995/07/14 10:26:24 drk $ */
#ifndef _XmDragOverSI_h
#define _XmDragOverSI_h

#include <Xm/DragOverSP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for DragOverS.c    ********/

extern void _XmDragOverHide( 
                        Widget w,
#if NeedWidePrototypes
                        int clipOriginX,
                        int clipOriginY,
#else
                        Position clipOriginX,
                        Position clipOriginY,
#endif /* NeedWidePrototypes */
                        XmRegion clipRegion) ;
extern void _XmDragOverShow( 
                        Widget w,
#if NeedWidePrototypes
                        int clipOriginX,
                        int clipOriginY,
#else
                        Position clipOriginX,
                        Position clipOriginY,
#endif /* NeedWidePrototypes */
                        XmRegion clipRegion) ;
extern void _XmDragOverMove( 
                        Widget w,
#if NeedWidePrototypes
                        int x,
                        int y) ;
#else
                        Position x,
                        Position y) ;
#endif /* NeedWidePrototypes */
extern void _XmDragOverChange( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int dropSiteStatus) ;
#else
                        unsigned char dropSiteStatus) ;
#endif /* NeedWidePrototypes */
extern void _XmDragOverFinish( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int completionStatus) ;
#else
                        unsigned char completionStatus) ;
#endif /* NeedWidePrototypes */

extern Cursor _XmDragOverGetActiveCursor(
			Widget w) ;
extern void _XmDragOverSetInitialPosition(
			Widget w,
#if NeedWidePrototypes
			int initialX,
			int initialY) ;
#else
			Position initialX,
			Position initialY) ;
#endif /* NeedWidePrototypes */

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragOverSI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

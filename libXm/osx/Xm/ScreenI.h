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
/* $XConsortium: ScreenI.h /main/5 1995/07/13 17:53:44 drk $ */
#ifndef _XmScreenI_h
#define _XmScreenI_h

#include <Xm/ScreenP.h>

#ifdef __cplusplus
extern "C" {
#endif


externalref XrmQuark _XmInvalidCursorIconQuark ;
externalref XrmQuark _XmValidCursorIconQuark ;
externalref XrmQuark _XmNoneCursorIconQuark ;
externalref XrmQuark _XmDefaultDragIconQuark ;
externalref XrmQuark _XmMoveCursorIconQuark ;
externalref XrmQuark _XmCopyCursorIconQuark ;
externalref XrmQuark _XmLinkCursorIconQuark ;


/********    Private Function Declarations    ********/

extern XmDragIconObject _XmScreenGetOperationIcon( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int operation) ;
#else
                        unsigned char operation) ;
#endif /* NeedWidePrototypes */
extern XmDragIconObject _XmScreenGetStateIcon( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int state) ;
#else
                        unsigned char state) ;
#endif /* NeedWidePrototypes */
extern XmDragIconObject _XmScreenGetSourceIcon( 
                        Widget w) ;
extern Pixmap _XmAllocScratchPixmap( 
                        XmScreen xmScreen,
#if NeedWidePrototypes
                        unsigned int depth,
                        int width,
                        int height) ;
#else
                        Cardinal depth,
                        Dimension width,
                        Dimension height) ;
#endif /* NeedWidePrototypes */
extern void _XmFreeScratchPixmap( 
                        XmScreen xmScreen,
                        Pixmap pixmap) ;
extern XmDragCursorCache * _XmGetDragCursorCachePtr( 
                        XmScreen xmScreen) ;
extern Cursor _XmGetMenuCursorByScreen( 
                        Screen *screen) ;
extern Boolean _XmGetMoveOpaqueByScreen( 
                        Screen *screen) ;
extern unsigned char _XmGetUnpostBehavior( 
                        Widget wid) ;
extern int _XmGetFontUnit( 
                        Screen *screen,
                        int dimension) ;
extern void _XmScreenRemoveFromCursorCache(
			XmDragIconObject icon) ;
extern XmScreenColorProc _XmGetColorCalculationProc( 
                        Screen *screen) ;
extern XmAllocColorProc _XmGetColorAllocationProc(
			Screen *screen) ;
extern Pixmap _XmGetInsensitiveStippleBitmap(
			Widget w) ;
extern XtEnum _XmGetBitmapConversionModel(
			Screen *screen) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScreenI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

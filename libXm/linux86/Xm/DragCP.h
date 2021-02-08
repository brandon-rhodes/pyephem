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
/*   $XConsortium: DragCP.h /main/12 1996/10/17 16:45:27 cde-osf $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDragCP_h
#define _XmDragCP_h

#include <Xm/XmP.h>
#include <Xm/DragC.h>

#include <X11/Shell.h>
#include <X11/ShellP.h>

#include <Xm/DragIcon.h>
#include <Xm/DragOverS.h>
#include <Xm/DropSMgrP.h>

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
 *
 * DragContext (RootWrapper) Widget Private Data
 *
 ***********************************************************************/

typedef void	(*XmDragStartProc)( XmDragContext, Widget, XEvent *);
typedef void 	(*XmDragCancelProc)( XmDragContext) ;


typedef struct {
    XmDragStartProc		start;
    XmDragCancelProc		cancel;
    XtPointer       		extension;
} XmDragContextClassPart;

typedef struct _XmDragContextClassRec {
    CoreClassPart	      	core_class;
    XmDragContextClassPart	drag_class;
} XmDragContextClassRec;

externalref XmDragContextClassRec xmDragContextClassRec;

#define XtDragByPoll 	0
#define XtDragByEvent	1

typedef struct {
    Window		frame;
    Window		window;
    Widget		shell;
    unsigned char	flags;
    unsigned char	dragProtocolStyle;
    int			xOrigin, yOrigin;
    unsigned int	width, height;
    unsigned int	depth;
    XtPointer		iccInfo;
} XmDragReceiverInfoStruct, *XmDragReceiverInfo;


typedef union _XmConvertSelectionRec
  {
    XtConvertSelectionIncrProc sel_incr ;
    XtConvertSelectionProc     sel ;
  } XmConvertSelectionRec ;
  

typedef struct _XmDragContextPart{
    /****  resources ****/

    Atom			*exportTargets;
    Cardinal			numExportTargets;
    XmConvertSelectionRec	convertProc;
    XtPointer			clientData;
    XmDragIconObject		sourceCursorIcon;
    XmDragIconObject		stateCursorIcon;
    XmDragIconObject		operationCursorIcon;
    XmDragIconObject		sourcePixmapIcon;
    Pixel			cursorBackground;
    Pixel			cursorForeground;
    Pixel			validCursorForeground;
    Pixel			invalidCursorForeground;
    Pixel			noneCursorForeground;
    XtCallbackList		dragMotionCallback;
    XtCallbackList		operationChangedCallback;
    XtCallbackList		siteEnterCallback;
    XtCallbackList		siteLeaveCallback;
    XtCallbackList		topLevelEnterCallback;
    XtCallbackList		topLevelLeaveCallback;
    XtCallbackList		dropStartCallback;
    XtCallbackList		dropFinishCallback;
    XtCallbackList		dragDropFinishCallback;
    unsigned char		dragOperations;
    Boolean			incremental;
    unsigned char		blendModel;

    /* private resources */
    Window			srcWindow;
    Time			dragStartTime;
    Atom			iccHandle;
    Widget			sourceWidget;
    Boolean			sourceIsExternal;

    /**** instance data ****/
    Boolean			topWindowsFetched;
    unsigned char 		commType;
    unsigned char		animationType;

    unsigned char		operation;
    unsigned char		operations;
    unsigned int		lastEventState;
    unsigned char		dragCompletionStatus;
    unsigned char		dragDropCompletionStatus;
    Boolean			forceIPC;
    Boolean			serverGrabbed;
    Boolean			useLocal;
    Boolean			inDropSite;
    XtIntervalId 		dragTimerId;
    
    Time			roundOffTime;
    Time			lastChangeTime;
    Time			crossingTime;

    Time			dragFinishTime;
    Time			dropFinishTime;
    
    Atom			dropSelection;
    Widget			srcShell;
	Position		startX, startY;

    XmID			siteID;

    Screen			*currScreen;
    Window			currWmRoot;
    XmDragOverShellWidget	curDragOver;
    XmDragOverShellWidget	origDragOver;

    XmDragReceiverInfoStruct	*currReceiverInfo;
    XmDragReceiverInfoStruct	*rootReceiverInfo;
    XmDragReceiverInfoStruct	*receiverInfos;
    Cardinal			numReceiverInfos;
    Cardinal			maxReceiverInfos;

    unsigned char		trackingMode;
    unsigned char		activeProtocolStyle;
    unsigned char               activeBlendModel;
    Boolean			dragDropCancelEffect;
    long 			SaveEventMask; 		/* Save the current root eventMask so that D&D works for MWM */
} XmDragContextPart;


typedef  struct _XmDragContextRec{
    CorePart	 		core;
    XmDragContextPart		drag;
} XmDragContextRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragCP_h */

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
/* $XConsortium: DragCI.h /main/10 1995/07/14 10:22:36 drk $ */
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDragCI_h
#define _XmDragCI_h

#include <Xm/XmP.h>
#include <Xm/DragCP.h>
#include <Xm/DragIconP.h>
#include <Xm/DropSMgrP.h>
#include <Xm/DisplayP.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _XmDragStart(dc, srcW, event) \
  ((*((XmDragContextClass)XtClass(dc))->drag_class.start) (dc, srcW, event))

#define _XmDragCancel(dc) \
  ((*((XmDragContextClass)XtClass(dc))->drag_class.cancel) (dc))

#define _XmDCtoDD(dc)	((XmDisplay)XtParent(dc))

#define _XmDRAG_MASK_BASE \
	(ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#ifdef DRAG_USE_MOTION_HINTS
#define _XmDRAG_GRAB_MASK \
	(_XmDRAG_MASK_BASE PointerMotionHintMask)
#else
#define _XmDRAG_GRAB_MASK _XmDRAG_MASK_BASE
#endif /* _XmDRAG_USE_MOTION_HINTS */

#define _XmDRAG_EVENT_MASK(dc) \
  ((((XmDragContext)dc)->drag.trackingMode == XmDRAG_TRACK_WM_QUERY) \
   ? (_XmDRAG_GRAB_MASK | EnterWindowMask | LeaveWindowMask) \
   : (_XmDRAG_GRAB_MASK))

enum{	XmCR_DROP_SITE_TREE_ADD = _XmNUMBER_DND_CB_REASONS,
	XmCR_DROP_SITE_TREE_REMOVE
	} ;
/*
 *  values for dragTrackingMode 
 */
enum { 
  XmDRAG_TRACK_WM_QUERY, XmDRAG_TRACK_MOTION, XmDRAG_TRACK_WM_QUERY_PENDING
};


/* Strings to use for the intern atoms */
typedef String	XmCanonicalString;

#define XmMakeCanonicalString( s) \
	(XmCanonicalString) XrmQuarkToString(XrmStringToQuark(s))

#define _XmAllocAndCopy( data, len) \
	memcpy((XtPointer) XtMalloc(len), (XtPointer)(data), (len))


typedef struct _XmDragTopLevelClientDataStruct{
    Widget	destShell;
    Position	xOrigin, yOrigin;
	Dimension	width, height;
    XtPointer	iccInfo;
    Boolean	sourceIsExternal;
	Window	window;
	Widget	dragOver;
} XmDragTopLevelClientDataStruct, *XmDragTopLevelClientData;

typedef struct _XmDragMotionClientDataStruct{
    Window	window;
    Widget	dragOver;
} XmDragMotionClientDataStruct, *XmDragMotionClientData;


/*
 * dsm to dragDisplay comm
 */
/* Move to DropSMgrI.h */
typedef struct _XmDropSiteTreeAddCallbackStruct{
    int		    	reason;
    XEvent          	*event;
    Widget		rootShell;
    Cardinal		numDropSites;
    Cardinal		numArgsPerDSHint;
} XmDropSiteTreeAddCallbackStruct, *XmDropSiteTreeAddCallback;

/* Move to DropSMgrI.h */
typedef struct _XmDropSiteTreeRemoveCallbackStruct{
    int			reason;
    XEvent          	*event;
    Widget		rootShell;
} XmDropSiteTreeRemoveCallbackStruct, *XmDropSiteTreeRemoveCallback;

/* Move to DropSMgrI.h */
typedef struct _XmDropSiteTreeUpdateCallbackStruct{
    int			reason;
    XEvent          	*event;
    Widget		rootShell;
    Cardinal		numDropSites;
    Cardinal		numArgsPerDSHint;
} XmDropSiteTreeUpdateCallbackStruct, *XmDropSiteTreeUpdateCallback;

typedef struct _XmDropSiteEnterPendingCallbackStruct{
    int                 reason;
    XEvent              *event;
    Time                timeStamp;
    Boolean		enter_pending;
} XmDropSiteEnterPendingCallbackStruct, *XmDropSiteEnterPendingCallback;

/* Move to DropSMgrI.h */
typedef struct _XmAnimationData {
    Widget		dragOver;
    Window		window;
    Position		windowX, windowY;
    Screen		*screen;
    XmRegion		clipRegion;
    XmRegion		dropSiteRegion;
    XtPointer		saveAddr;
} XmAnimationDataRec, *XmAnimationData;


/********    Private Function Declarations    ********/

extern XmDragReceiverInfo _XmAllocReceiverInfo( 
                        XmDragContext dc) ;
extern unsigned char _XmGetActiveProtocolStyle( 
                        Widget w) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragCI_h */

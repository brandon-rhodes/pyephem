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
/*   $TOG: DragICCI.h /main/14 1997/06/18 17:38:28 samborn $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDragICCI_h
#define _XmDragICCI_h

#include <Xm/XmP.h>
#include <Xm/Display.h>
#include <X11/Xmd.h>
#include "DragCI.h"
#include "DropSMgrI.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 *  Xm ICC internal definitions 
 *
 */

#define MAXSHORT 32767
#define MINSHORT -MAXSHORT 

/*
 *  Swap the byte order of 4- and 2- byte quantities.
 *  These macros work for bitfields.
 */

#define swap4bytes(l) {\
	struct {\
	  unsigned t :32;\
	} bit32;\
        char n,	*tp = (char *) &bit32;\
	bit32.t = l;\
	n = tp[0]; tp[0] = tp[3]; tp[3] = n;\
	n = tp[1]; tp[1] = tp[2]; tp[2] = n;\
        l = bit32.t;\
}

#define swap2bytes(s) {\
	struct {\
	  unsigned t :16;\
	} bit16;\
        char n, *tp = (char *) &bit16;\
	bit16.t = s;\
	n = tp[0]; tp[0] = tp[1]; tp[1] = n;\
        s = bit16.t;\
}



typedef struct {
/* the message type field contains the following:
 *
 *  80      originator
 *  7F      message_type
 */

    BYTE		message_type;
    BYTE		byte_order;
/*
 * the flags field contains the following:
 *
 *	000F	operation
 *	00F0	dropSiteStatus
 *	0F00	operations
 *	F000	completionStatus
 */
    CARD16		flags B16;
    CARD32		time B32;
}xmICCAnyMessageStruct, *xmICCAnyMessage;

typedef struct {
    BYTE		message_type;
    BYTE		byte_order;
    CARD16		flags B16;
    CARD32		time B32;
    CARD32		src_window B32;
    CARD32		icc_handle B32;
}xmICCTopLevelEnterMessageStruct;

typedef struct {
    BYTE		message_type;
    BYTE		byte_order;
    CARD16		flags B16;
    CARD32		time B32;
    CARD32		src_window B32;
}xmICCTopLevelLeaveMessageStruct;

typedef struct {
    BYTE		message_type;
    BYTE		byte_order;
    CARD16		flags B16;
    CARD32		time B32;
    INT16		x B16;
    INT16		y B16;
}xmICCDragMotionMessageStruct;

typedef struct {
    BYTE		message_type;
    BYTE		byte_order;
    CARD16		flags B16;
    CARD32		time B32;
}xmICCOperationChangedMessageStruct;

typedef struct {
    BYTE		message_type;
    BYTE		byte_order;
    CARD16		flags B16;
    CARD32		time B32;
    INT16		x B16;
    INT16		y B16;
}xmICCDropSiteEnterMessageStruct;

typedef struct {
    BYTE		message_type;
    BYTE		byte_order;
    CARD16		flags B16;
    CARD32		time B32;
}xmICCDropSiteLeaveMessageStruct;

typedef struct {
    BYTE		message_type;
    BYTE		byte_order;
    CARD16		flags B16;
    CARD32		time B32;
    INT16		x B16;
    INT16		y B16;
    CARD32		icc_handle B32;
    CARD32		src_window B32;
}xmICCDropMessageStruct;

typedef union _xmICCMessageStruct{
    xmICCAnyMessageStruct			any;
    xmICCTopLevelEnterMessageStruct		topLevelEnter;
    xmICCTopLevelLeaveMessageStruct		topLevelLeave;
    xmICCDragMotionMessageStruct		dragMotion;
    xmICCOperationChangedMessageStruct		operationChanged;
    xmICCDropSiteEnterMessageStruct		dropSiteEnter;
    xmICCDropSiteLeaveMessageStruct		dropSiteLeave;
    xmICCDropMessageStruct			drop;
}xmICCMessageStruct, *xmICCMessage;


typedef union _XmICCCallbackStruct{
    XmAnyICCCallbackStruct			any;
    XmTopLevelEnterCallbackStruct		topLevelEnter;
    XmTopLevelLeaveCallbackStruct		topLevelLeave;
    XmDragMotionCallbackStruct			dragMotion;
    XmOperationChangedCallbackStruct		operationChanged;
    XmDropSiteEnterCallbackStruct		dropSiteEnter;
    XmDropSiteLeaveCallbackStruct		dropSiteLeave;
    XmDropSiteTreeAddCallbackStruct		dropSiteTreeAdd;
    XmDropSiteTreeRemoveCallbackStruct		dropSiteTreeRemove;
    XmDropSiteTreeUpdateCallbackStruct		dropSiteTreeUpdate;
}XmICCCallbackStruct, *XmICCCallback;

typedef struct _xmByteBufRec{
    BYTE	*bytes;
    BYTE	*stack;
    BYTE	*curr;
    size_t	size;
    Cardinal	max;
}xmByteBufRec; 

typedef struct _xmPropertyBufferRec{
    xmByteBufRec	data;
    xmByteBufRec	heap;
}xmPropertyBufferRec, *xmPropertyBuffer;

/* for argument passing between DropSite and ICC routines */

typedef struct _XmICCDropSiteHeaderRec {
    unsigned char	dropType;
    unsigned char	dropActivity;
    unsigned char	traversalType;
    unsigned char	operations;
    unsigned char	animationStyle;
    unsigned short	importTargetsID;
    XmRegion		region;
} XmICCDropSiteHeaderRec, *XmICCDropSiteHeader;

typedef struct _XmICCDropSiteNoneDataRec {
    Dimension		borderWidth;
} XmICCDropSiteNoneDataRec, *XmICCDropSiteNoneData;

typedef struct _XmICCDropSiteHighlightDataRec {
    Dimension		borderWidth;
    Dimension		highlightThickness;
    Pixel		background;
    Pixel		highlightColor;
    Pixmap		highlightPixmap;
} XmICCDropSiteHighlightDataRec, *XmICCDropSiteHighlightData;

typedef struct _XmICCDropSiteShadowDataRec {
    Dimension		borderWidth;
    Dimension		highlightThickness;
    Dimension		shadowThickness;	
    Pixel		foreground;
    Pixel		topShadowColor;
    Pixmap		topShadowPixmap;
    Pixel		bottomShadowColor;
    Pixmap		bottomShadowPixmap;
} XmICCDropSiteShadowDataRec, *XmICCDropSiteShadowData;

typedef struct _XmICCDropSitePixmapDataRec {
    Dimension		borderWidth;
    Dimension		highlightThickness;
    Dimension		shadowThickness;	
    Pixel		foreground;
    Pixel		background;
    Pixmap		animationPixmap;
    Cardinal		animationPixmapDepth;
    Pixmap		animationMask;
} XmICCDropSitePixmapDataRec, *XmICCDropSitePixmapData;

typedef struct _XmICCDropSiteNoneRec {
    XmICCDropSiteHeaderRec		header;
    XmICCDropSiteNoneDataRec	animation_data;
} XmICCDropSiteNoneRec, *XmICCDropSiteNone;

typedef struct _XmICCDropSiteHighlightRec {
    XmICCDropSiteHeaderRec		header;
    XmICCDropSiteHighlightDataRec	animation_data;
} XmICCDropSiteHighlightRec, *XmICCDropSiteHighlight;

typedef struct _XmICCDropSiteShadowRec {
    XmICCDropSiteHeaderRec	header;
    XmICCDropSiteShadowDataRec	animation_data;
} XmICCDropSiteShadowRec, *XmICCDropSiteShadow;

typedef struct _XmICCDropSitePixmapRec {
    XmICCDropSiteHeaderRec	header;
    XmICCDropSitePixmapDataRec	animation_data;
} XmICCDropSitePixmapRec, *XmICCDropSitePixmap;

typedef union _XmICCDropSiteInfoStruct {
    XmICCDropSiteHeaderRec	header;
    XmICCDropSiteHighlightRec	highlightDS;
    XmICCDropSiteShadowRec	shadowDS;
    XmICCDropSitePixmapRec	pixmapDS;
} XmICCDropSiteInfoStruct, *XmICCDropSiteInfo;

typedef struct _XmReceiverDSTreeStruct{
    xmPropertyBufferRec	propBufRec;
    unsigned char	byteOrder;
    Cardinal		numDropSites;
    Cardinal		currDropSite;
}XmReceiverDSTreeStruct, *XmReceiverDSTree;

/*
 *  The following structures are for property access.
 *  They must have 64-bit multiple lengths to support 64-bit architectures.
 */

typedef struct _xmDragInitiatorInfoStruct{
    BYTE	byte_order;
    BYTE	protocol_version;
    CARD16	targets_index B16;
    CARD32	icc_handle B32;
}xmDragInitiatorInfoStruct;

typedef struct _xmDragReceiverInfoStruct{
    BYTE	byte_order;
    BYTE	protocol_version;
    BYTE	drag_protocol_style;
    BYTE	pad1;
    CARD32	proxy_window B32;
    CARD16	num_drop_sites B16;
    CARD16	pad2 B16;
    CARD32	heap_offset B32;
}xmDragReceiverInfoStruct;

typedef struct {
    INT16	x1 B16;
    INT16	x2 B16;
    INT16	y1 B16;
    INT16	y2 B16;
}xmICCRegBoxRec;

typedef struct _xmDSHeaderStruct{
/*
 * the flags field contains the following:
 *
 *	0003	traversalType
 *	000C	dropActivity
 *	00F0	dropType
 *	0F00	operations
 *	F000	animationStyle
 */
    CARD16	flags B16;
    CARD16	import_targets_id B16;
    CARD32	dsRegionNumBoxes B32;
}xmDSHeaderStruct, *xmDSHeader;

typedef struct _xmDSNoneDataStruct{
    CARD16	borderWidth B16;
    CARD16	pad1 B16;
    CARD32	pad2 B32;
}xmDSNoneDataStruct, *xmDSNoneData;

typedef struct _xmDSHighlightDataStruct{
    CARD16	borderWidth B16;
    CARD16	highlightThickness B16;

    CARD32	background B32;
    CARD32	highlightColor B32;
    CARD32	highlightPixmap B32;
}xmDSHighlightDataStruct, *xmDSHighlightData;

typedef struct _xmDSShadowDataStruct{
    CARD16	borderWidth B16;
    CARD16	highlightThickness B16;
    CARD16	shadowThickness B16;
    CARD16	pad1 B16;

    CARD32	foreground B32;
    CARD32	topShadowColor B32;
    CARD32	bottomShadowColor B32;
    CARD32	topShadowPixmap B32;
    CARD32	bottomShadowPixmap B32;
    CARD32	pad2 B32;
}xmDSShadowDataStruct, *xmDSShadowData;

typedef struct _xmDSPixmapDataStruct{
    CARD16	borderWidth B16;
    CARD16	highlightThickness B16;
    CARD16	shadowThickness B16;
    CARD16	animationPixmapDepth B16;

    CARD32	foreground B32;
    CARD32	background B32;
    CARD32	animationPixmap B32;
    CARD32	animationMask B32;
}xmDSPixmapDataStruct, *xmDSPixmapData;


/* Macros for the manipulation of ICCmessages and xmDSData */

#define _XM_TRAVERSAL_TYPE_MASK ((CARD16) 0x0003)
#define _XM_TRAVERSAL_TYPE_SHIFT 0

#define _XM_DS_ACTIVITY_MASK ((CARD16) 0x000C)
#define _XM_DS_ACTIVITY_SHIFT 2

#define _XM_DND_OPERATION_MASK ((CARD16) 0x000F)
#define _XM_DND_OPERATION_SHIFT 0

#define _XM_DND_SITE_STATUS_MASK ((CARD16) 0x00F0)
#define _XM_DND_SITE_STATUS_SHIFT 4

#define _XM_DS_TYPE_MASK ((CARD16) 0x00F0)
#define _XM_DS_TYPE_SHIFT 4

#define _XM_DND_MULTIOPS_MASK ((CARD16) 0x0F00)
#define _XM_DND_MULTIOPS_SHIFT 8

#define _XM_DND_COMPLETION_MASK ((CARD16) 0xF000)
#define _XM_DND_COMPLETION_SHIFT 12

#define _XM_ANIMATION_STYLE_MASK ((CARD16) 0xF000)
#define _XM_ANIMATION_STYLE_SHIFT 12

#define GET_OPERATION(flags) \
  ((unsigned char) \
   ((int)((flags) & _XM_DND_OPERATION_MASK) >> _XM_DND_OPERATION_SHIFT))

#define PUT_OPERATION(operation) \
  (((CARD16)(operation) << _XM_DND_OPERATION_SHIFT)\
   & _XM_DND_OPERATION_MASK)

#define GET_SITE_STATUS(flags) \
  ((unsigned char) \
   ((int)((flags) & _XM_DND_SITE_STATUS_MASK) >> _XM_DND_SITE_STATUS_SHIFT))

#define PUT_SITE_STATUS(site_status) \
  (((CARD16)(site_status) << _XM_DND_SITE_STATUS_SHIFT)\
   & _XM_DND_SITE_STATUS_MASK)

#define GET_MULTIOPS(flags) \
  ((unsigned char) \
   ((int)((flags) & _XM_DND_MULTIOPS_MASK) >> _XM_DND_MULTIOPS_SHIFT))

#define PUT_MULTIOPS(operation) \
  (((CARD16)(operation) << _XM_DND_MULTIOPS_SHIFT)\
   & _XM_DND_MULTIOPS_MASK)

#define GET_COMPLETION(flags) \
  ((unsigned char) \
   ((int)((flags) & _XM_DND_COMPLETION_MASK) >> _XM_DND_COMPLETION_SHIFT))

#define PUT_COMPLETION(completion) \
  (((CARD16)(completion) << _XM_DND_COMPLETION_SHIFT)\
   & _XM_DND_COMPLETION_MASK)

#define GET_TRAVERSAL_TYPE(flags) \
  ((unsigned char) \
   ((int)((flags) & _XM_TRAVERSAL_TYPE_MASK) >> _XM_TRAVERSAL_TYPE_SHIFT))

#define PUT_TRAVERSAL_TYPE(traversal_type) \
  (((CARD16)(traversal_type) << _XM_TRAVERSAL_TYPE_SHIFT)\
   & _XM_TRAVERSAL_TYPE_MASK)

#define GET_DS_TYPE(flags) \
  ((unsigned char) \
   ((int)((flags) & _XM_DS_TYPE_MASK) >> _XM_DS_TYPE_SHIFT))

#define PUT_DS_TYPE(ds_type) \
  (((CARD16)(ds_type) << _XM_DS_TYPE_SHIFT)\
   & _XM_DS_TYPE_MASK)

#define GET_DS_ACTIVITY(flags) \
  ((unsigned char) \
   ((int)((flags) & _XM_DS_ACTIVITY_MASK) >> _XM_DS_ACTIVITY_SHIFT))

#define PUT_DS_ACTIVITY(ds_activity) \
  (((CARD16)(ds_activity) << _XM_DS_ACTIVITY_SHIFT)\
   & _XM_DS_ACTIVITY_MASK)

#define GET_ANIMATION_STYLE(flags) \
  ((unsigned char) \
   ((int)((flags) & _XM_ANIMATION_STYLE_MASK) >> _XM_ANIMATION_STYLE_SHIFT))

#define PUT_ANIMATION_STYLE(animation_style) \
  (((CARD16)(animation_style) << _XM_ANIMATION_STYLE_SHIFT)\
   & _XM_ANIMATION_STYLE_MASK)


/*
 * We consume the high order bit of the messageType in order
 * to indicate whether this is an initiator generated event
 * or a receiver generated event.
 *
 * This is all wrapped in macros in case we want to use more bits
 * later.
 */

typedef enum {
	XmICC_INITIATOR_EVENT,
	XmICC_RECEIVER_EVENT
} XmICCEventType;

#define _XM_ICC_EVENT_TYPE_MASK  ((BYTE)0x80)
#define _XM_ICC_EVENT_TYPE_SHIFT 7

#define GET_ICC_EVENT_TYPE(type) \
  ((XmICCEventType) \
    ((int)((type) & _XM_ICC_EVENT_TYPE_MASK) >> _XM_ICC_EVENT_TYPE_SHIFT))

#define PUT_ICC_EVENT_TYPE(type) \
  (((BYTE)(type) << _XM_ICC_EVENT_TYPE_SHIFT) \
    & _XM_ICC_EVENT_TYPE_MASK)

#define CLEAR_ICC_EVENT_TYPE  ((BYTE)0x7F)


#define _MOTIF_DRAG_PROTOCOL_VERSION	(BYTE)0


externalref unsigned char _XmByteOrderChar;


/********    Private Function Declarations    ********/

extern unsigned char _XmReasonToMessageType( 
                        int reason) ;
extern unsigned int _XmMessageTypeToReason( 
#if NeedWidePrototypes
                        unsigned int messageType) ;
#else
                        unsigned char messageType) ;
#endif /* NeedWidePrototypes */
extern void _XmICCCallbackToICCEvent( 
                        Display *display,
                        Window window,
                        XmICCCallback callback,
                        XClientMessageEvent *cmev,
                        XmICCEventType type) ;
extern void _XmSendICCCallback( 
                        Display *display,
                        Window window,
                        XmICCCallback callback,
                        XmICCEventType type) ;
extern Boolean _XmICCEventToICCCallback( 
                        XClientMessageEvent *msgEv,
                        XmICCCallback callback,
                        XmICCEventType type) ;
extern CARD16 _XmReadDragBuffer( 
                        xmPropertyBuffer propBuf,
#if NeedWidePrototypes
                        int which,
#else
                        BYTE which,
#endif /* NeedWidePrototypes */
                        BYTE *ptr,
                        CARD32 size) ;
extern CARD16 _XmWriteDragBuffer( 
                        xmPropertyBuffer propBuf,
#if NeedWidePrototypes
                        int which,
#else
                        BYTE which,
#endif /* NeedWidePrototypes */
                        BYTE *ptr,
                        CARD32 size) ;
extern void _XmWriteInitiatorInfo( 
                        Widget dc) ;
extern void _XmReadInitiatorInfo( 
                        Widget dc) ;
extern Boolean _XmGetDragReceiverInfo( 
                        Display *display,
                        Window window,
                        XmDragReceiverInfoStruct *receiverInfoRtn) ;
extern Boolean _XmReadDSFromStream( 
                        XmDropSiteManagerObject dsm,
                        XtPointer iccInfo,
                        XmICCDropSiteInfo dropSiteInfoRtn) ;
extern void _XmWriteDSToStream( 
                        XmDropSiteManagerObject dsm,
                        XtPointer stream,
                        XmICCDropSiteInfo dropSiteInfo) ;
extern void _XmFreeDragReceiverInfo( 
                        XtPointer info) ;
extern void _XmClearDragReceiverInfo( 
                        Widget shell) ;
extern void _XmSetDragReceiverInfo( 
                        XmDisplay dd,
                        Widget shell) ;
extern void _XmInitByteOrderChar( void ) ;

/********    End Private Function Declarations    ********/

#define _XmInitDragICC(dd)	_XmInitByteOrderChar()


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragICCI_h */

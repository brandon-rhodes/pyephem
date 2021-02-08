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
/*   $XConsortium: CutPaste.h /main/13 1995/07/14 10:17:18 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmCutPaste_h
#define _XmCutPaste_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* XmClipboard return status definitions */

typedef enum {
  XmClipboardFail = 0,
  XmClipboardSuccess = 1,
  XmClipboardTruncate = 2,
  XmClipboardLocked = 4,
  XmClipboardBadFormat = 5,
  XmClipboardNoData = 6
} XmClipboardStatus;

/* XmClipboard pre-1.2 definitions */

#define ClipboardFail     	0
#define ClipboardSuccess  	1 
#define ClipboardTruncate 	2
#define ClipboardLocked   	4
#define ClipboardBadFormat   	5
#define ClipboardNoData   	6

typedef struct {
    long DataId;
    long PrivateId;
} XmClipboardPendingRec, *XmClipboardPendingList;

typedef void (*XmCutPasteProc)( Widget w, long * data_id, long * private_id,
							        int * reason) ;
typedef void (*VoidProc)( Widget w, int * data_id, int * private_id,
							        int * reason) ;


/********    Public Function Declarations    ********/

extern int XmClipboardBeginCopy( 
                        Display *display,
                        Window window,
                        XmString label,
                        Widget widget,
                        VoidProc callback,
                        long *itemid) ;
extern int XmClipboardStartCopy( 
                        Display *display,
                        Window window,
                        XmString label,
                        Time timestamp,
                        Widget widget,
                        XmCutPasteProc callback,
                        long *itemid) ;
extern int XmClipboardCopy( 
                        Display *display,
                        Window window,
                        long itemid,
                        char *format,
                        XtPointer buffer,
                        unsigned long length,
                        long private_id,
                        long *dataid) ;
extern int XmClipboardEndCopy( 
                        Display *display,
                        Window window,
                        long itemid) ;
extern int XmClipboardCancelCopy( 
                        Display *display,
                        Window window,
                        long itemid) ;
extern int XmClipboardWithdrawFormat( 
                        Display *display,
                        Window window,
                        long data) ;
extern int XmClipboardCopyByName( 
                        Display *display,
                        Window window,
                        long data,
                        XtPointer buffer,
                        unsigned long length,
                        long private_id) ;
extern int XmClipboardUndoCopy( 
                        Display *display,
                        Window window) ;
extern int XmClipboardLock( 
                        Display *display,
                        Window window) ;
extern int XmClipboardUnlock( 
                        Display *display,
                        Window window,
#if NeedWidePrototypes
                        int all_levels) ;
#else
                        Boolean all_levels) ;
#endif /* NeedWidePrototypes */
extern int XmClipboardStartRetrieve( 
                        Display *display,
                        Window window,
                        Time timestamp) ;
extern int XmClipboardEndRetrieve( 
                        Display *display,
                        Window window) ;
extern int XmClipboardRetrieve( 
                        Display *display,
                        Window window,
                        char *format,
                        XtPointer buffer,
                        unsigned long length,
                        unsigned long *outlength,
                        long *private_id) ;
extern int XmClipboardInquireCount( 
                        Display *display,
                        Window window,
                        int *count,
                        unsigned long *maxlength) ;
extern int XmClipboardInquireFormat( 
                        Display *display,
                        Window window,
                        int n,
                        XtPointer buffer,
                        unsigned long bufferlength,
                        unsigned long *outlength) ;
extern int XmClipboardInquireLength( 
                        Display *display,
                        Window window,
                        char *format,
                        unsigned long *length) ;
extern int XmClipboardInquirePendingItems( 
                        Display *display,
                        Window window,
                        char *format,
                        XmClipboardPendingList *list,
                        unsigned long *count) ;
extern int XmClipboardRegisterFormat( 
                        Display *display,
                        char *format_name,
                        int format_length) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCutPaste_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

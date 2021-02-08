/* $TOG: Transfer.h /main/8 1998/02/03 14:55:22 csn $ */
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

#ifndef _XmTransfer_H
#define _XmTransfer_H

#include <Xm/DragC.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Proc typedefs */

#define XmConvertCallbackProc 		XtCallbackProc
#define XmSelectionDoneProc   		XtSelectionDoneProc
#define XmCancelSelectionProc 		XtCancelConvertSelectionProc
#define XmDestinationCallbackProc	XtCallbackProc
#define XmSelectionCallbackProc		XtSelectionCallbackProc

/* Flags */

typedef enum { XmTRANSFER_DONE_SUCCEED = 0, XmTRANSFER_DONE_FAIL, 
	       XmTRANSFER_DONE_CONTINUE, XmTRANSFER_DONE_DEFAULT 
	     } XmTransferStatus;

enum { XmSELECTION_DEFAULT = 0, XmSELECTION_INCREMENTAL,
       XmSELECTION_PERSIST, XmSELECTION_SNAPSHOT,
       XmSELECTION_TRANSACT };

enum { XmCONVERTING_NONE = 0, 
       XmCONVERTING_SAME = 1, 
       XmCONVERTING_TRANSACT = 2,
       XmCONVERTING_PARTIAL = 4 };

enum { XmCONVERT_DEFAULT = 0, XmCONVERT_MORE, 
       XmCONVERT_MERGE, XmCONVERT_REFUSE, XmCONVERT_DONE };

/* Callback structures */

typedef struct {
	int		reason;
	XEvent		*event;
	Atom		selection;
	Atom		target;
	XtPointer	source_data;
	XtPointer	location_data;
	int		flags;
	XtPointer	parm;
	int		parm_format;
	unsigned long	parm_length;
	Atom		parm_type;
	int		status;
	XtPointer	value;
	Atom		type;
	int		format;
	unsigned long	length;
} XmConvertCallbackStruct;

typedef struct {
	int		reason;
  	XEvent		*event;
	Atom		selection;
	XtEnum		operation;	
	int		flags;
	XtPointer	transfer_id;
	XtPointer	destination_data;
	XtPointer	location_data;
	Time		time;
} XmDestinationCallbackStruct;

typedef struct {
	int		reason;
  	XEvent		*event;
	Atom		selection;
	Atom		target;
	Atom		type;
	XtPointer	transfer_id;
	int		flags;
	int		remaining;
	XtPointer	value;
	unsigned long	length;
	int		format;
} XmSelectionCallbackStruct;

typedef struct {
	int		reason;
  	XEvent		*event;
	Atom		selection;
	XtPointer	transfer_id;
	XmTransferStatus	status;
	XtPointer	client_data;
} XmTransferDoneCallbackStruct;

typedef void (*XmSelectionFinishedProc)(Widget, XtEnum,
					XmTransferDoneCallbackStruct*);

void XmTransferDone(XtPointer, XmTransferStatus);
void XmTransferValue(XtPointer, Atom, XtCallbackProc,
		     XtPointer,	Time);
void XmTransferSetParameters(XtPointer, XtPointer, int,	unsigned long, Atom);
void XmTransferStartRequest(XtPointer);
void XmTransferSendRequest(XtPointer, Time);

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTransfer_H */

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
/* $XConsortium: TransferI.h /main/6 1996/10/16 16:57:37 drk $ */
#ifndef _XmTransferI_H
#define _XmTransferI_H

#include <Xm/TransferT.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/* Structure which is stored for each transfer request.  This   */
/* is where XmTransferValue will keep the data for the internal */
/* wrapper. 							*/
/****************************************************************/

#define TB_NONE		    0
#define TB_IGNORE	    1
#define TB_INTERNAL   	    2

typedef struct {
  XtPointer		next;
  XtPointer		client_data;
  XtPointer		location_data;
  int			flags;
  Atom			target;
  XtCallbackProc	selection_proc;
} TransferBlockRec, *TransferBlock;

/****************************************************************/
/* This structure forms the context block for each transfer_id. */
/* These structures are chained to allow for multiple           */
/* concurrent outstanding data transfers.                       */
/****************************************************************/

typedef struct {
  XtPointer	next;
  XtPointer	prev;
  Widget	widget;
  Atom		selection;
  Atom		real_selection;
  XtEnum	op;
  int		count;
  int		outstanding;
  int		flags;
  int		status;
  Widget	drag_context;
  Widget	drop_context;
  XmSelectionFinishedProc	*doneProcs;
  int		numDoneProcs;
  XtCallbackProc		auto_proc;
  XtPointer	client_data;
  XmDestinationCallbackStruct	*callback_struct;
  TransferBlock last;
  TransferBlock requests;
} TransferContextRec, *TransferContext;

enum{ TC_NONE = 0, TC_FLUSHED = 1, TC_CALLED_WIDGET = 2,
      TC_CALLED_CALLBACKS = 4, TC_EXITED_DH = 8, 
      TC_DID_DELETE = 16, TC_IN_MULTIPLE = 32 };

/****************************************************************/
/* The convertProc has a small context block which is           */
/* setup by the source calls and deleted in the loseProc        */
/****************************************************************/

typedef struct {
  long		op;	/* Make it big so it can hold a variety of data */
  int		flags;
  long		itemid;
  XtPointer	location_data;
  XtPointer	client_data;
  Widget	drag_context;
} ConvertContextRec, *ConvertContext;

enum{ CC_NONE = 0};

/****************************************************************/
/* Need to keep a count of outstanding SNAPSHOT requests.       */
/****************************************************************/

typedef struct {
  long		outstanding;
  Atom		distinguisher;
} SnapshotRequestRec, *SnapshotRequest;

/* Internal functions */

extern char * _XmTextToLocaleText(Widget, XtPointer, Atom, int,
				  unsigned long, Boolean *);

extern void _XmConvertHandlerSetLocal(void);

extern Boolean _XmConvertHandler(Widget wid, Atom *selection, Atom *target, 
			  Atom *type, XtPointer *value, 
			  unsigned long *size, int *fmt);

extern Boolean _XmDestinationHandler(Widget wid, Atom selection, XtEnum op,
			      XmSelectionFinishedProc done_proc,
			      XtPointer location_data, Time time,
			      XSelectionRequestEvent *event);

extern void _XmConvertComplete(Widget wid, XtPointer value, 
			unsigned long size, int format, Atom type,
			XmConvertCallbackStruct *cs);

extern XmDestinationCallbackStruct
       *_XmTransferGetDestinationCBStruct(XtPointer);

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTransferI_H */


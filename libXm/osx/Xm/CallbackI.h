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
/*   $XConsortium: CallbackI.h /main/8 1995/07/14 10:13:23 drk $ */
/* (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/* (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmCallbackI_h
#define _XmCallbackI_h

#include "XmI.h"

#ifdef __cplusplus
extern "C" {
#endif


#define _XtCBCalling		1
#define _XtCBFreeAfterCalling	2

/* The internal callback list is a little header followed by an array of
 * XtCallbackRec structs.  Declaring the first element of the array with 
 * the header allows the compiler to take care of any alignment required.
 */
typedef struct internalCallbackRec {
  unsigned short count;
  char           is_padded;	/* contains NULL padding for external form */
  char           call_state;	/* combination of _XtCB{FreeAfter}Calling */
  XtCallbackRec  callbacks;	/* first XtCallback record declared here */
} InternalCallbackRec, *InternalCallbackList;


/********    Private Function Declarations    ********/

extern void _XmAddCallback(InternalCallbackList *callbacks,
			   XtCallbackProc callback,
			   XtPointer closure);
extern void _XmRemoveCallback(InternalCallbackList *callbacks,
			      XtCallbackProc callback,
			      XtPointer closure);
extern void _XmCallCallbackList(Widget widget,
				XtCallbackList callbacks,
				XtPointer call_data);
extern void _XmRemoveAllCallbacks(InternalCallbackList *callbacks);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCallbackI_h */
/* DON'T ADD STUFF AFTER THIS #endif */

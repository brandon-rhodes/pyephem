/* $XConsortium: IconHP.h /main/4 1995/07/15 20:52:25 drk $ */
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
#ifndef _XmIconHP_h
#define _XmIconHP_h

#include <Xm/XmP.h>
#include <Xm/IconGP.h>
#include <Xm/IconH.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IconHeader class record */
typedef struct _XmIconHeaderClassPart
	{
	XtPointer extension ;
	} 	XmIconHeaderClassPart;


/* Full class record declaration */
typedef struct _XmIconHeaderClassRec
	{
	RectObjClassPart	rect_class;
	XmGadgetClassPart	gadget_class;
	XmIconGadgetClassPart	icong_class;
	XmIconHeaderClassPart	iconh_class;
	} 	XmIconHeaderClassRec;

extern	XmIconHeaderClassRec 	xmIconHeaderClassRec;

/* IconHeader instance record */
typedef struct _XmIconHeaderPart
	{
	Widget container_ID;	                /* XmNcontainerID */
	} 	XmIconHeaderPart;

/* Full instance record declaration */
typedef struct _XmIconHeaderRec
	{
	ObjectPart	object;
	RectObjPart	rectangle;
	XmGadgetPart	gadget;
	XmIconGadgetPart icong;
	XmIconHeaderPart iconh;
	} 	XmIconHeaderRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmIconHP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

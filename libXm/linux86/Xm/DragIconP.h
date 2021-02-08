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
/* $XConsortium: DragIconP.h /main/11 1995/07/14 10:25:42 drk $ */
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDragIconP_h
#define _XmDragIconP_h

#include <Xm/VendorSEP.h>
#include <Xm/DragIcon.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*XmCloneVisualProc) (XmDragIconObject, Widget, Widget);
typedef void (*XmMovePixmapProc)  (XmDragIconObject, 
				   XmDragIconObject, 
				   XmDragIconObject,
#if NeedWidePrototypes
				   int, int);
#else
				   Position, Position);
#endif /* NeedWidePrototypes */

typedef struct {
  XtPointer		extension;
} XmDragIconClassPart;

typedef struct _XmDragIconClassRec{
  RectObjClassPart		rectangle_class;
  XmDragIconClassPart		dragIcon_class;
} XmDragIconClassRec;

typedef struct {
  Cardinal	depth;
  Pixmap	pixmap;
  Dimension	width, height;
  Pixmap	mask;
  Position	hot_x, hot_y;
  Position	offset_x, offset_y;
  unsigned char	attachment;
  Boolean	isDirty;
  Region        region;
  Region        restore_region;
  Position	x_offset, y_offset;
} XmDragIconPart, *XmDragIconPartPtr;

externalref XmDragIconClassRec 	xmDragIconClassRec;

typedef struct _XmDragIconRec{
  ObjectPart		object;
  RectObjPart		rectangle;
  XmDragIconPart	drag;
} XmDragIconRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragIconP_h */

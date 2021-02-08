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
/*   $XConsortium: DrawingAP.h /main/13 1996/04/01 15:22:11 daniel $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDrawingAreaP_h
#define _XmDrawingAreaP_h

#include <Xm/ManagerP.h>
#include <Xm/DrawingA.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmRESIZE_SWINDOW	10


/* Constraint part record for DrawingArea widget */

typedef struct _XmDrawingAreaConstraintPart
{
   char unused;
} XmDrawingAreaConstraintPart, * XmDrawingAreaConstraint;

/*  New fields for the DrawingArea widget class record  */

typedef struct
{
   XtPointer extension;   /* Pointer to extension record */
} XmDrawingAreaClassPart;


/* Full class record declaration */

typedef struct _XmDrawingAreaClassRec
{
  CoreClassPart		core_class;
  CompositeClassPart	composite_class;
  ConstraintClassPart	constraint_class;
  XmManagerClassPart	manager_class;
  XmDrawingAreaClassPart	drawing_area_class;
} XmDrawingAreaClassRec;

externalref XmDrawingAreaClassRec xmDrawingAreaClassRec;


/* New fields for the DrawingArea widget record */

typedef struct
{
  Dimension		margin_width;
  Dimension		margin_height;

  XtCallbackList	resize_callback;
  XtCallbackList	expose_callback;
  XtCallbackList	input_callback;

  unsigned char		resize_policy;
  
#ifndef XM_PART_BC
  XtCallbackList	convert_callback;
  XtCallbackList	destination_callback;
#endif
} XmDrawingAreaPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmDrawingAreaRec
{
  CorePart		core;
  CompositePart		composite;
  ConstraintPart	constraint;
  XmManagerPart		manager;
  XmDrawingAreaPart	drawing_area;
} XmDrawingAreaRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDrawingAreaP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

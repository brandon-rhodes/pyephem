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
/*   $XConsortium: TearOffBP.h /main/11 1995/10/25 20:20:56 cde-sun $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
 *  TearOffBP.h - Private definitions for TearOffButton widget 
 *  (Used by RowColumn Tear Off Menupanes)
 *
 */

#ifndef _XmTearOffBP_h
#define _XmTearOffBP_h

#include <Xm/PushBP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 *
 * TearOffButton Widget Private Data
 *
 *****************************************************************************/

/* New fields for the TearOffButton widget class record */
typedef struct _XmTearOffButtonClassPart
{
    String translations;
} XmTearOffButtonClassPart;

/* Full Class record declaration */
typedef struct _XmTearOffButtonClassRec {
    CoreClassPart         core_class;
    XmPrimitiveClassPart  primitive_class;
    XmLabelClassPart      label_class;
    XmPushButtonClassPart pushbutton_class;
    XmTearOffButtonClassPart    tearoffbutton_class;
} XmTearOffButtonClassRec;

typedef struct _XmTearOffButtonClassRec *XmTearOffButtonWidgetClass;

externalref XmTearOffButtonClassRec xmTearOffButtonClassRec;

/* New fields for the TearOffButton widget record */
typedef struct {
   Dimension      margin;
   unsigned char  orientation;
   unsigned char separator_type;
   GC separator_GC;
   Boolean 	set_recompute_size;
} XmTearOffButtonPart;

/*****************************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************************/

typedef struct _XmTearOffButtonRec {
   CorePart         core;
   XmPrimitivePart  primitive;
   XmLabelPart      label;
   XmPushButtonPart pushbutton;
   XmTearOffButtonPart tear_off_button;
} XmTearOffButtonRec;

typedef struct _XmTearOffButtonRec      *XmTearOffButtonWidget;

/* Class Record Constant */

externalref WidgetClass xmTearOffButtonWidgetClass;

#ifndef XmIsTearOffButton
#define XmIsTearOffButton(w)	XtIsSubclass(w, xmTearOffButtonWidgetClass)
#endif /* XmIsTearOffButton */


/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTearOffButtonP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

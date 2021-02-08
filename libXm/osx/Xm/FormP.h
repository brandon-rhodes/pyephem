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
/*   $TOG: FormP.h /main/13 1998/03/25 12:25:28 csn $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmFormP_h
#define _XmFormP_h


#include <Xm/Form.h>
#include <Xm/BulletinBP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* define index constants which are used to access attribute array of Form 
 * so that subclass of Form can make use of them.
 */

#define _XmFORM_LEFT    0
#define _XmFORM_RIGHT   1
#define _XmFORM_TOP     2
#define _XmFORM_BOTTOM  3


typedef struct _XmFormAttachmentRec 
{
   unsigned char type;
   Widget w;
   int percent;
   int offset;
   int value;
   int tempValue;
} XmFormAttachmentRec, * XmFormAttachment;


#ifdef att
#undef att
#endif

typedef struct _XmFormConstraintPart
{
   XmFormAttachmentRec att[4];
   Widget next_sibling;
   Boolean sorted;
   Boolean resizable;
   Dimension preferred_width, preferred_height;
} XmFormConstraintPart, * XmFormConstraint;

typedef struct _XmFormConstraintRec
{
   XmManagerConstraintPart manager;
   XmFormConstraintPart    form;
} XmFormConstraintRec, * XmFormConstraintPtr;


/*  Form class structure  */

typedef struct _XmFormClassPart
{
   XtPointer extension;   /* Pointer to extension record */
} XmFormClassPart;


/*  Full class record declaration for form class  */

typedef struct _XmFormClassRec
{
   CoreClassPart       core_class;
   CompositeClassPart  composite_class;
   ConstraintClassPart constraint_class;
   XmManagerClassPart  manager_class;
   XmBulletinBoardClassPart  bulletin_board_class;
   XmFormClassPart     form_class;
} XmFormClassRec;

externalref XmFormClassRec xmFormClassRec;


/*  The Form instance record  */

typedef struct _XmFormPart
{
   Dimension horizontal_spacing;
   Dimension vertical_spacing;
   int fraction_base;
   Boolean rubber_positioning;
   Widget first_child;
   Boolean initial_width, initial_height;
   Boolean processing_constraints;
} XmFormPart;


/*  Full instance record declaration  */

typedef struct _XmFormRec
{
   CorePart	  core;
   CompositePart  composite;
   ConstraintPart constraint;
   XmManagerPart  manager;
   XmBulletinBoardPart  bulletin_board;
   XmFormPart     form;
} XmFormRec;

/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmFormP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

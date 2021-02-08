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
/*   $XConsortium: CommandP.h /main/11 1995/07/14 10:16:43 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmCommandP_h
#define _XmCommandP_h

#include <Xm/SelectioBP.h>
#include <Xm/Command.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constraint part record for Command widget */

typedef struct _XmCommandConstraintPart
{
  char unused;
} XmCommandConstraintPart, * XmCommandConstraint;

/*  New fields for the Command widget class record  */

typedef struct
{
  XtPointer           extension;      /* Pointer to extension record */
} XmCommandClassPart;


/* Full class record declaration */

typedef struct _XmCommandClassRec
{
  CoreClassPart            core_class;
  CompositeClassPart       composite_class;
  ConstraintClassPart      constraint_class;
  XmManagerClassPart       manager_class;
  XmBulletinBoardClassPart bulletin_board_class;
  XmSelectionBoxClassPart  selection_box_class;
  XmCommandClassPart       command_class;
} XmCommandClassRec;

externalref XmCommandClassRec xmCommandClassRec;

/* New fields for the Command widget record */

typedef struct
{
  XtCallbackList   callback;
  XtCallbackList   value_changed_callback;
  int              history_max_items;
  Boolean          error;        /* error has been made visible in list */
} XmCommandPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmCommandRec
{
    CorePart	        core;
    CompositePart       composite;
    ConstraintPart      constraint;
    XmManagerPart       manager;
    XmBulletinBoardPart bulletin_board;
    XmSelectionBoxPart  selection_box;
    XmCommandPart       command;
} XmCommandRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCommandP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

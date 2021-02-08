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
/* $XConsortium: MessageBP.h /main/10 1995/07/13 17:38:00 drk $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmessageP_h
#define _XmessageP_h

#include <Xm/BulletinBP.h>
#include <Xm/MessageB.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constraint part record for MessageBox widget */

typedef struct _XmMessageBoxConstraintPart
{
   char unused;
} XmMessageBoxConstraintPart, * XmMessageBoxConstraint;


/*  New fields for the MessageBox widget class record  */

typedef struct
{
   XtPointer extension;   /* Pointer to extension record */
} XmMessageBoxClassPart;


/* Full class record declaration */

typedef struct _XmMessageBoxClassRec
{
   CoreClassPart             core_class;
   CompositeClassPart        composite_class;
   ConstraintClassPart       constraint_class;
   XmManagerClassPart        manager_class;
   XmBulletinBoardClassPart  bulletin_board_class;
   XmMessageBoxClassPart     message_box_class;
} XmMessageBoxClassRec;

externalref XmMessageBoxClassRec xmMessageBoxClassRec;


/* New fields for the MessageBox widget record */

typedef struct
{
    unsigned char           dialog_type;
    unsigned char           default_type;
    Boolean		    internal_pixmap;
    Boolean                 minimize_buttons;

    unsigned char           message_alignment;
    XmString                message_string;
    Widget                  message_wid;

    Pixmap                  symbol_pixmap;
    Widget                  symbol_wid;

    XmString                ok_label_string;
    XtCallbackList          ok_callback;
    Widget                  ok_button;

    XmString                cancel_label_string;
    XtCallbackList          cancel_callback;

    XmString                help_label_string;
    Widget                  help_button;

    Widget                  separator;

} XmMessageBoxPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmMessageBoxRec
{
    CorePart	         core;
    CompositePart        composite;
    ConstraintPart       constraint;
    XmManagerPart        manager;
    XmBulletinBoardPart  bulletin_board; 
    XmMessageBoxPart     message_box;
} XmMessageBoxRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMessage_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

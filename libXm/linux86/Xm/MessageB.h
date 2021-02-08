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
/*   $XConsortium: MessageB.h /main/11 1995/07/13 17:37:42 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmMessage_h
#define _XmMessage_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmMessageBoxWidgetClass;

typedef struct _XmMessageBoxClassRec * XmMessageBoxWidgetClass;
typedef struct _XmMessageBoxRec      * XmMessageBoxWidget;

/* fast XtIsSubclass define */
#ifndef XmIsMessageBox
#define XmIsMessageBox(w) XtIsSubclass (w, xmMessageBoxWidgetClass)
#endif 


/********    Public Function Declarations    ********/

extern Widget XmCreateMessageBox( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateMessageDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateErrorDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateInformationDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateQuestionDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateWarningDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateWorkingDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateTemplateDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmMessageBoxGetChild( 
                        Widget widget,
#if NeedWidePrototypes
                        unsigned int child) ;
#else
                        unsigned char child) ;
#endif /* NeedWidePrototypes */

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMessage_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

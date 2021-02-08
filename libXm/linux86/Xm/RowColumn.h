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
/*   $TOG: RowColumn.h /main/13 1999/01/19 14:07:48 mgreess $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
#ifndef _XmRowColumn_h
#define _XmRowColumn_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmRowColumnWidgetClass;

typedef struct _XmRowColumnClassRec * XmRowColumnWidgetClass;
typedef struct _XmRowColumnRec      * XmRowColumnWidget;

#ifndef XmIsRowColumn
#define XmIsRowColumn(w) XtIsSubclass((w),xmRowColumnWidgetClass)
#endif


/********    Public Function Declarations    ********/

extern void XmMenuPosition( 
                        Widget p,
                        XButtonPressedEvent *event) ;
extern Widget XmCreateRowColumn( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateWorkArea( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateRadioBox( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateOptionMenu( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmOptionLabelGadget( 
                        Widget m) ;
extern Widget XmOptionButtonGadget( 
                        Widget m) ;
extern Widget XmCreateMenuBar( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreatePopupMenu( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreatePulldownMenu( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmGetPostedFromWidget( 
                        Widget menu) ;
extern Widget XmGetTearOffControl(
			Widget menu) ;

extern void XmAddToPostFromList(
			Widget m,
			Widget widget );
extern void XmRemoveFromPostFromList(
			Widget m,
			Widget widget );

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmRowColumn_h  */
/* DON'T ADD STUFF AFTER THIS #endif */

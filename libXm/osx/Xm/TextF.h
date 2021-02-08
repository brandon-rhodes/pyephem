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
/*   $XConsortium: TextF.h /main/11 1995/07/13 18:05:20 drk $ */
/*
*  (c) Copyright 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTextF_h
#define _XmTextF_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************
 * type defines   *
 ******************/
typedef struct _XmTextFieldClassRec *XmTextFieldWidgetClass;
typedef struct _XmTextFieldRec *XmTextFieldWidget;

/******************
 * extern class   *
 ******************/
externalref WidgetClass       xmTextFieldWidgetClass;


/************************
 * fast subclass define *
 ************************/
#ifndef XmIsTextField
#define XmIsTextField(w)     XtIsSubclass(w, xmTextFieldWidgetClass)
#endif /* XmIsTextField */


/********************
 * public functions *
 ********************/

/********    Public Function Declarations    ********/

char * XmTextFieldGetString( 
                 Widget w);
int XmTextFieldGetSubstring( 
                 Widget widget,
                 XmTextPosition start,
                 int num_chars,
                 int buf_size,
                 char *buffer);
wchar_t * XmTextFieldGetStringWcs( 
                 Widget w);
int XmTextFieldGetSubstringWcs( 
                 Widget widget,
                 XmTextPosition start,
                 int num_chars,
                 int buf_size,
                 wchar_t *buffer);
XmTextPosition XmTextFieldGetLastPosition( 
                 Widget w);
void XmTextFieldSetString( 
                 Widget w,
                 char *value);
void XmTextFieldSetStringWcs( 
                 Widget w,
                 wchar_t *wc_value);
void XmTextFieldReplace( 
                 Widget w,
                 XmTextPosition from_pos,
                 XmTextPosition to_pos,
                 char *value);
void XmTextFieldReplaceWcs( 
                 Widget w,
                 XmTextPosition from_pos,
                 XmTextPosition to_pos,
                 wchar_t *wc_value);
void XmTextFieldInsert( 
                 Widget w,
                 XmTextPosition position,
                 char *value);
void XmTextFieldInsertWcs( 
                 Widget w,
                 XmTextPosition position,
                 wchar_t *wcstring);
void XmTextFieldSetAddMode( 
                        Widget w,
#if NeedWidePrototypes
                        int state);
#else
                        Boolean state);
#endif /* NeedWidePrototypes */
Boolean XmTextFieldGetAddMode( 
                 Widget w);
Boolean XmTextFieldGetEditable( 
                 Widget w);
void XmTextFieldSetEditable( 
                        Widget w,
#if NeedWidePrototypes
                        int editable);
#else
                        Boolean editable);
#endif /* NeedWidePrototypes */
int XmTextFieldGetMaxLength( 
                 Widget w);
void XmTextFieldSetMaxLength( 
                 Widget w,
                 int max_length);
XmTextPosition XmTextFieldGetCursorPosition( 
                 Widget w);
XmTextPosition XmTextFieldGetInsertionPosition( 
                 Widget w);
void XmTextFieldSetCursorPosition( 
                 Widget w,
                 XmTextPosition position);
void XmTextFieldSetInsertionPosition( 
                 Widget w,
                 XmTextPosition position);
Boolean XmTextFieldGetSelectionPosition( 
                 Widget w,
                 XmTextPosition *left,
                 XmTextPosition *right);
char * XmTextFieldGetSelection( 
                 Widget w);
wchar_t * XmTextFieldGetSelectionWcs( 
                 Widget w);
Boolean XmTextFieldRemove( 
                 Widget w);
Boolean XmTextFieldCopy( 
                 Widget w,
                 Time clip_time);
Boolean XmTextFieldCopyLink( 
                 Widget w,
                 Time clip_time);
Boolean XmTextFieldCut( 
                 Widget w,
                 Time clip_time);
Boolean XmTextFieldPaste( 
                 Widget w);
Boolean XmTextFieldPasteLink( 
                 Widget w);
void XmTextFieldClearSelection( 
                 Widget w,
                 Time sel_time);
void XmTextFieldSetSelection( 
                 Widget w,
                 XmTextPosition first,
                 XmTextPosition last,
                 Time sel_time);
XmTextPosition XmTextFieldXYToPos( 
                        Widget w,
#if NeedWidePrototypes
                        int x,
                        int y);
#else
                        Position x,
                        Position y);
#endif /* NeedWidePrototypes */
Boolean XmTextFieldPosToXY( 
                 Widget w,
                 XmTextPosition position,
                 Position *x,
                 Position *y);
void XmTextFieldShowPosition( 
                 Widget w,
                 XmTextPosition position);
void XmTextFieldSetHighlight( 
                 Widget w,
                 XmTextPosition left,
                 XmTextPosition right,
                 XmHighlightMode mode);
int XmTextFieldGetBaseline( 
                 Widget w);
Widget XmCreateTextField( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount);
Widget XmVaCreateTextField(
                        Widget parent,
                        char *name,
                        ...);
Widget XmVaCreateManagedTextField(
                        Widget parent,
                        char *name,
                        ...);

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextF_h */
/* DON'T ADD STUFF AFTER THIS #endif */

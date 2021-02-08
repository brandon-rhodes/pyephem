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

extern char * XmTextFieldGetString( 
                        Widget w) ;
extern int XmTextFieldGetSubstring( 
                        Widget widget,
                        XmTextPosition start,
                        int num_chars,
                        int buf_size,
                        char *buffer) ;
extern wchar_t * XmTextFieldGetStringWcs( 
                        Widget w) ;
extern int XmTextFieldGetSubstringWcs( 
                        Widget widget,
                        XmTextPosition start,
                        int num_chars,
                        int buf_size,
                        wchar_t *buffer) ;
extern XmTextPosition XmTextFieldGetLastPosition( 
                        Widget w) ;
extern void XmTextFieldSetString( 
                        Widget w,
                        char *value) ;
extern void XmTextFieldSetStringWcs( 
                        Widget w,
                        wchar_t *wc_value) ;
extern void XmTextFieldReplace( 
                        Widget w,
                        XmTextPosition from_pos,
                        XmTextPosition to_pos,
                        char *value) ;
extern void XmTextFieldReplaceWcs( 
                        Widget w,
                        XmTextPosition from_pos,
                        XmTextPosition to_pos,
                        wchar_t *wc_value) ;
extern void XmTextFieldInsert( 
                        Widget w,
                        XmTextPosition position,
                        char *value) ;
extern void XmTextFieldInsertWcs( 
                        Widget w,
                        XmTextPosition position,
                        wchar_t *wcstring) ;
extern void XmTextFieldSetAddMode( 
                        Widget w,
#if NeedWidePrototypes
                        int state) ;
#else
                        Boolean state) ;
#endif /* NeedWidePrototypes */
extern Boolean XmTextFieldGetAddMode( 
                        Widget w) ;
extern Boolean XmTextFieldGetEditable( 
                        Widget w) ;
extern void XmTextFieldSetEditable( 
                        Widget w,
#if NeedWidePrototypes
                        int editable) ;
#else
                        Boolean editable) ;
#endif /* NeedWidePrototypes */
extern int XmTextFieldGetMaxLength( 
                        Widget w) ;
extern void XmTextFieldSetMaxLength( 
                        Widget w,
                        int max_length) ;
extern XmTextPosition XmTextFieldGetCursorPosition( 
                        Widget w) ;
extern XmTextPosition XmTextFieldGetInsertionPosition( 
                        Widget w) ;
extern void XmTextFieldSetCursorPosition( 
                        Widget w,
                        XmTextPosition position) ;
extern void XmTextFieldSetInsertionPosition( 
                        Widget w,
                        XmTextPosition position) ;
extern Boolean XmTextFieldGetSelectionPosition( 
                        Widget w,
                        XmTextPosition *left,
                        XmTextPosition *right) ;
extern char * XmTextFieldGetSelection( 
                        Widget w) ;
extern wchar_t * XmTextFieldGetSelectionWcs( 
                        Widget w) ;
extern Boolean XmTextFieldRemove( 
                        Widget w) ;
extern Boolean XmTextFieldCopy( 
                        Widget w,
                        Time clip_time) ;
extern Boolean XmTextFieldCopyLink( 
                        Widget w,
                        Time clip_time) ;
extern Boolean XmTextFieldCut( 
                        Widget w,
                        Time clip_time) ;
extern Boolean XmTextFieldPaste( 
                        Widget w) ;
extern Boolean XmTextFieldPasteLink( 
                        Widget w) ;
extern void XmTextFieldClearSelection( 
                        Widget w,
                        Time sel_time) ;
extern void XmTextFieldSetSelection( 
                        Widget w,
                        XmTextPosition first,
                        XmTextPosition last,
                        Time sel_time) ;
extern XmTextPosition XmTextFieldXYToPos( 
                        Widget w,
#if NeedWidePrototypes
                        int x,
                        int y) ;
#else
                        Position x,
                        Position y) ;
#endif /* NeedWidePrototypes */
extern Boolean XmTextFieldPosToXY( 
                        Widget w,
                        XmTextPosition position,
                        Position *x,
                        Position *y) ;
extern void XmTextFieldShowPosition( 
                        Widget w,
                        XmTextPosition position) ;
extern void XmTextFieldSetHighlight( 
                        Widget w,
                        XmTextPosition left,
                        XmTextPosition right,
                        XmHighlightMode mode) ;
extern int XmTextFieldGetBaseline( 
                        Widget w) ;
extern Widget XmCreateTextField( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextF_h */
/* DON'T ADD STUFF AFTER THIS #endif */

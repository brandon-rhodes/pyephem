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
/*   $XConsortium: Text.h /main/15 1996/01/29 13:20:20 daniel $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
#ifndef _XmText_h
#define _XmText_h

#include <Xm/Xm.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------- *
 *   type defines *
 * -------------- */
typedef struct _XmTextSourceRec *XmTextSource;
typedef struct _XmTextClassRec *XmTextWidgetClass;
typedef struct _XmTextRec *XmTextWidget;

/* -------------- *
 * extern class   *
 * -------------- */
externalref WidgetClass       xmTextWidgetClass;


/* --------------------------------------- *
 *  text widget fast subclassing fallback  *
 * --------------------------------------- */

#ifndef XmIsText
#define XmIsText(w)	XtIsSubclass(w, xmTextWidgetClass)
#endif /* XmIsText */


/* ----------------------------------- *
 *   text widget public functions      *
 * ----------------------------------- */

/********    Public Function Declarations    ********/

extern void XmTextSetHighlight( 
                        Widget w,
                        XmTextPosition left,
                        XmTextPosition right,
                        XmHighlightMode mode);
extern Widget XmCreateScrolledText( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount);
extern Widget XmCreateText( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount);
extern Widget XmVaCreateText(
                        Widget parent,
                        char *name,
                        ...);
extern Widget XmVaCreateManagedText(
                        Widget parent,
                        char *name,
                        ...);
extern int XmTextGetSubstring( 
                        Widget widget,
                        XmTextPosition start,
                        int num_chars,
                        int buf_size,
                        char *buffer);
extern int XmTextGetSubstringWcs( 
                        Widget widget,
                        XmTextPosition start,
                        int num_chars,
                        int buf_size,
                        wchar_t *buffer);
extern char * XmTextGetString( 
                        Widget widget);
extern wchar_t * XmTextGetStringWcs( 
                        Widget widget);
extern XmTextPosition XmTextGetLastPosition( 
                        Widget widget);
extern void XmTextSetString( 
                        Widget widget,
                        char *value);
extern void XmTextSetStringWcs( 
                        Widget widget,
                        wchar_t *wc_value);
extern void XmTextReplace( 
                        Widget widget,
                        XmTextPosition frompos,
                        XmTextPosition topos,
                        char *value);
extern void XmTextReplaceWcs( 
                        Widget widget,
                        XmTextPosition frompos,
                        XmTextPosition topos,
                        wchar_t *value);
extern void XmTextInsert( 
                        Widget widget,
                        XmTextPosition position,
                        char *value);
extern void XmTextInsertWcs( 
                        Widget widget,
                        XmTextPosition position,
                        wchar_t *wc_value);
extern void XmTextSetAddMode( 
                        Widget widget,
#if NeedWidePrototypes
                        int state);
#else
                        Boolean state);
#endif /* NeedWidePrototypes */
extern Boolean XmTextGetAddMode( 
                        Widget widget);
extern Boolean XmTextGetEditable( 
                        Widget widget);
extern void XmTextSetEditable( 
                        Widget widget,
#if NeedWidePrototypes
                        int editable);
#else
                        Boolean editable);
#endif /* NeedWidePrototypes */
extern int XmTextGetMaxLength( 
                        Widget widget);
extern void XmTextSetMaxLength( 
                        Widget widget,
                        int max_length);
extern XmTextPosition XmTextGetTopCharacter( 
                        Widget widget);
extern void XmTextSetTopCharacter( 
                        Widget widget,
                        XmTextPosition top_character);
extern XmTextPosition XmTextGetCursorPosition( 
                        Widget widget);
extern XmTextPosition XmTextGetInsertionPosition( 
                        Widget widget);
extern void XmTextSetInsertionPosition( 
                        Widget widget,
                        XmTextPosition position);
extern void XmTextSetCursorPosition( 
                        Widget widget,
                        XmTextPosition position);
extern Boolean XmTextRemove( 
                        Widget widget);
extern Boolean XmTextCopy( 
                        Widget widget,
                        Time copy_time);
extern Boolean XmTextCopyLink( 
                        Widget widget,
                        Time copy_time);
extern Boolean XmTextCut( 
                        Widget widget,
                        Time cut_time);
extern Boolean XmTextPaste( 
                        Widget widget);
extern Boolean XmTextPasteLink( 
                        Widget widget);
extern char * XmTextGetSelection( 
                        Widget widget);
extern wchar_t * XmTextGetSelectionWcs( 
                        Widget widget);
extern void XmTextSetSelection( 
                        Widget widget,
                        XmTextPosition first,
                        XmTextPosition last,
                        Time set_time);
extern void XmTextClearSelection( 
                        Widget widget,
                        Time clear_time);
extern Boolean XmTextGetSelectionPosition( 
                        Widget widget,
                        XmTextPosition *left,
                        XmTextPosition *right);
extern XmTextPosition XmTextXYToPos( 
                        Widget widget,
#if NeedWidePrototypes
                        int x,
                        int y);
#else
                        Position x,
                        Position y);
#endif /* NeedWidePrototypes */
extern Boolean XmTextPosToXY( 
                        Widget widget,
                        XmTextPosition position,
                        Position *x,
                        Position *y);
extern XmTextSource XmTextGetSource( 
                        Widget widget);
extern void XmTextSetSource( 
                        Widget widget,
                        XmTextSource source,
                        XmTextPosition top_character,
                        XmTextPosition cursor_position);
extern void XmTextShowPosition( 
                        Widget widget,
                        XmTextPosition position);
extern void XmTextScroll( 
                        Widget widget,
                        int n);
extern int XmTextGetBaseline( 
                        Widget widget);
extern int XmTextGetCenterline( 
                        Widget widget);
extern void XmTextDisableRedisplay( 
                        Widget widget);
extern void XmTextEnableRedisplay( 
                        Widget widget);
extern Boolean XmTextFindString( 
                        Widget w,
                        XmTextPosition start,
                        char *search_string,
                        XmTextDirection direction,
                        XmTextPosition *position);
extern Boolean XmTextFindStringWcs( 
                        Widget w,
                        XmTextPosition start,
                        wchar_t *wc_string,
                        XmTextDirection direction,
                        XmTextPosition *position);

/********    End Public Function Declarations    ********/

/* tmp: go to XmStrDefs */
#define XmNtotalLines "totalLines"
#define XmCTotalLines "TotalLines"

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmText_h */
/* DON'T ADD STUFF AFTER THIS #endif */

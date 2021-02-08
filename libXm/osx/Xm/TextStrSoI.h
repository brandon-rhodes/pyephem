/*
 * @OPENGROUP_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF for
 * the full copyright text.
 * 
 * This software is subject to an open license. It may only be
 * used on, with or for operating systems which are themselves open
 * source systems. You must contact The Open Group for a license
 * allowing distribution and sublicensing of this software on, with,
 * or for operating systems which are not Open Source programs.
 * 
 * See http://www.opengroup.org/openmotif/license for full
 * details of the license agreement. Any use, reproduction, or
 * distribution of the program constitutes recipient's acceptance of
 * this agreement.
 * 
 * EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 * PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 * WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 * OR FITNESS FOR A PARTICULAR PURPOSE
 * 
 * EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 * NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 * EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 */
/*
 * HISTORY
 */
/* $XConsortium: TextStrSoI.h /main/5 1995/07/13 18:10:59 drk $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTextStrSoI_h
#define _XmTextStrSoI_h

#include <Xm/TextStrSoP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern char  * _XmStringSourceGetString(XmTextWidget tw,
				        XmTextPosition from,
				        XmTextPosition to,
#if NeedWidePrototypes
				        int want_wchar);
#else
                                        Boolean want_wchar);
#endif /* NeedWidePrototypes */
extern Boolean _XmTextFindStringBackwards(Widget w,
					  XmTextPosition start,
					  char *search_string,
					  XmTextPosition *position);
extern Boolean _XmTextFindStringForwards(Widget w,
					 XmTextPosition start,
					 char *search_string,
					 XmTextPosition *position);
extern void    _XmStringSourceSetGappedBuffer(XmSourceData data,
					      XmTextPosition position);
extern Boolean _XmTextModifyVerify(XmTextWidget initiator,
				   XEvent *event,
				   XmTextPosition *start,
				   XmTextPosition *end,
				   XmTextPosition *cursorPos,
				   XmTextBlock block,
				   XmTextBlock newblock,
				   Boolean *freeBlock);
extern XmTextSource _XmStringSourceCreate(char *value,
#if NeedWidePrototypes
					  int is_wchar);
#else
                                          Boolean is_wchar);
#endif /* NeedWidePrototypes */
extern void    _XmStringSourceDestroy(XmTextSource source);
extern char  * _XmStringSourceGetValue(XmTextSource source,
#if NeedWidePrototypes
				       int want_wchar);
#else
                                       Boolean want_wchar);
#endif /* NeedWidePrototypes */
extern void    _XmStringSourceSetValue(XmTextWidget widget,
				       char *value);
extern Boolean _XmStringSourceHasSelection(XmTextSource source);
extern Boolean _XmStringSourceGetEditable(XmTextSource source);
extern void    _XmStringSourceSetEditable(XmTextSource source,
#if NeedWidePrototypes
					  int editable);
#else
                                         Boolean editable);
#endif /* NeedWidePrototypes */
extern int     _XmStringSourceGetMaxLength(XmTextSource source);
extern void    _XmStringSourceSetMaxLength(XmTextSource source,
					   int max);
extern int _XmTextBytesToCharacters(char *characters,
				    char *bytes,
				    int num_chars,
#if NeedWidePrototypes
				    int add_null_terminator,
#else
				    Boolean add_null_terminator,
#endif /* NeedWidePrototypes */
				    int max_char_size);
extern int _XmTextCharactersToBytes(char *bytes,
				    char *characters,
				    int num_chars,
				    int max_char_size);
extern void    _XmTextValueChanged(XmTextWidget initiator,
				   XEvent *event);
extern Boolean *_XmStringSourceGetPending(XmTextWidget widget);
extern void    _XmStringSourceSetPending(XmTextWidget widget,
					 Boolean *pending);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /*  _XmTextStrSoI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

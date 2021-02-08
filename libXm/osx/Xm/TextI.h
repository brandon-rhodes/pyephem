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
/* $XConsortium: TextI.h /main/6 1996/05/29 13:45:16 pascale $ */
#ifndef _XmTextI_h
#define _XmTextI_h

#include <Xm/TextP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern XmTextPosition _XmTextFindScroll(XmTextWidget widget,
					XmTextPosition start,
					int delta);
extern int _XmTextGetTotalLines(Widget widget);
extern XmTextLineTable _XmTextGetLineTable(Widget widget,
					   int *total_lines);
extern void _XmTextRealignLineTable(XmTextWidget widget,
				    XmTextLineTable *temp_table,
				    int *temp_table_size,
				    register unsigned int cur_index,
				    register XmTextPosition cur_start,
				    register XmTextPosition cur_end);
extern unsigned int _XmTextGetTableIndex(XmTextWidget widget,
					 XmTextPosition pos);
extern void _XmTextUpdateLineTable(Widget widget,
				   XmTextPosition start,
				   XmTextPosition end,
				   XmTextBlock block,
#if NeedWidePrototypes
				   int update
#else
				   Boolean update
#endif /* NeedWidePrototypes */
				   );
extern void _XmTextMarkRedraw(XmTextWidget widget,
			      XmTextPosition left,
			      XmTextPosition right);
extern LineNum _XmTextNumLines(XmTextWidget widget);
extern void _XmTextLineInfo(XmTextWidget widget,
			    LineNum line,
			    XmTextPosition *startpos,
			    LineTableExtra *extra);
extern LineNum _XmTextPosToLine(XmTextWidget widget,
				XmTextPosition position);
extern void _XmTextInvalidate(XmTextWidget widget,
			      XmTextPosition position,
			      XmTextPosition topos,
			      long delta);
extern void _XmTextSetTopCharacter(Widget widget,
				   XmTextPosition top_character);
extern int _XmTextCountCharacters(char *str,
				  int num_count_bytes);
extern void _XmTextSetCursorPosition(Widget widget,
				     XmTextPosition position);
extern void _XmTextDisableRedisplay(XmTextWidget widget,
#if NeedWidePrototypes
				    int losesbackingstore);
#else
                                    Boolean losesbackingstore);
#endif /* NeedWidePrototypes */
extern void _XmTextEnableRedisplay(XmTextWidget widget);  

extern void _XmTextSetHighlight(Widget, XmTextPosition,
                                XmTextPosition, XmHighlightMode);
extern void _XmTextShowPosition(Widget, XmTextPosition);
extern void _XmTextSetEditable(Widget widget,
#if NeedWidePrototypes
			       int editable);
#else
                               Boolean editable);
#endif /* NeedWidePrototypes */
extern void _XmTextResetIC(Widget widget);
extern Boolean _XmTextNeedsPendingDeleteDis(XmTextWidget tw,
                                            XmTextPosition *left,
                                            XmTextPosition *right,
                                            int check_add_mode);
extern void _XmTextReplace(Widget widget,
                           XmTextPosition frompos,
                           XmTextPosition topos,
	                   char *value, 
#if NeedWidePrototypes
                           int is_wchar);
#else
                           Boolean is_wchar);
#endif /* NeedWidePrototypes */
extern void _XmTextValidate(XmTextPosition *start,
		            XmTextPosition *end,
		            int maxsize);

extern XmTextPosition _XmTextSetPreeditPosition(Widget w,
                                                XmTextPosition cursor_position);
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

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
/* $XConsortium: XmTabListI.h /main/5 1995/07/13 18:28:19 drk $ */
#ifndef _XmTabListI_h
#define _XmTabListI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TAB_OPTIMIZED_BITS	1
#define TAB_MARK_BITS		TAB_OPTIMIZED_BITS
#define TAB_REFCOUNT_BITS	(16 - TAB_OPTIMIZED_BITS)

typedef struct __XmTabRec
{
  unsigned int		mark : TAB_MARK_BITS;
  unsigned int		ref_count : TAB_REFCOUNT_BITS;
  float			value;
  unsigned char		units;
  XmOffsetModel		offsetModel;
  unsigned char		alignment;
  char			*decimal;
  XmTab			next, prev;
} _XmTabRec, *_XmTab;

typedef struct __XmTabListRec
{
  unsigned int	count;
  XmTab		start;
} _XmTabListRec, *_XmTabList;


/*
 * Macros for tab data structure access
 */

#define _XmTabMark(tab)		((_XmTab)(tab))->mark
#define _XmTabValue(tab)	((_XmTab)(tab))->value
#define _XmTabUnits(tab)	((_XmTab)(tab))->units
#define _XmTabPrev(tab)		((_XmTab)(tab))->prev
#define _XmTabNext(tab)		((_XmTab)(tab))->next
#define _XmTabModel(tab)	((_XmTab)(tab))->offsetModel
#define _XmTabAlign(tab)	((_XmTab)(tab))->alignment
#define _XmTabDecimal(tab)	((_XmTab)(tab))->decimal

#define _XmTabLStart(tl)	((_XmTabList)(tl))->start
#define _XmTabLCount(tl)	((_XmTabList)(tl))->count



/********    Private Function Declarations for XmTabList.c    ********/

extern XmTab _XmTabCopy(XmTab tab);
extern Widget _XmCreateTabList(Widget parent,
                               String name,
                               ArgList arglist,
                               Cardinal argcount); 
extern Widget _XmCreateTab(Widget parent,
			   String name,
			   ArgList arglist,
			   Cardinal argcount); 
extern Position _XmTabListGetPosition(
				     Screen * screen,
				     XmTabList tab_list,
                                     unsigned char unit_type,
				     Cardinal tab_position);


/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTabListI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

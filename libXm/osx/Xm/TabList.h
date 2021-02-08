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
#ifndef _Xm_TabList_h_
#define _Xm_TabList_h_

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmCOLOR_DYNAMIC ((Pixel)-1)
#define XmPIXMAP_DYNAMIC ((Pixmap) 3)
#define XmTAB_LAST_POSITION -1
#define XmTAB_NOT_FOUND -1

#define XmTAB_LABEL_STRING      (1L<<0)
#define XmTAB_LABEL_PIXMAP      (1L<<1)
#define XmTAB_PIXMAP_PLACEMENT  (1L<<2)
#define XmTAB_BACKGROUND        (1L<<3)
#define XmTAB_FOREGROUND        (1L<<4)
#define XmTAB_VALUE_MODE        (1L<<5)
#define XmTAB_LABEL_ALIGNMENT   (1L<<6)
#define XmTAB_STRING_DIRECTION  (1L<<7)
#define XmTAB_BACKGROUND_PIXMAP	(1L<<8)
#define XmTAB_SENSITIVE         (1L<<9)
#define XmTAB_ALL_FLAGS         (XmTAB_LABEL_STRING|XmTAB_LABEL_PIXMAP|\
				 XmTAB_PIXMAP_PLACEMENT|XmTAB_BACKGROUND|\
				 XmTAB_FOREGROUND|XmTAB_VALUE_MODE|\
				 XmTAB_LABEL_ALIGNMENT|XmTAB_STRING_DIRECTION|\
				 XmTAB_BACKGROUND_PIXMAP|XmTAB_SENSITIVE)

typedef struct _XmTabAttributeRec {
    XmString          label_string;	/* default: NULL                     */
    XmStringDirection string_direction; /* default: XmSTRING_DIRECTION_L_TO_R*/
    Pixmap            label_pixmap;	/* default: XmUNSPECIFIED_PIXMAP     */
    int               label_alignment;  /* default: XmALIGNEMENT_CENTER      */
    XmPixmapPlacement pixmap_placement; /* default: XmPIXMAP_RIGHT           */
    Pixel             foreground;       /* default: XmCOLOR_DYNAMIC          */
    Pixel             background;       /* default: XmCOLOR_DYNAMIC          */
    Pixmap            background_pixmap;/* default: XmPIXMAP_DYNAMIC         */
    Boolean	      sensitive;        /* default: True                     */
    XmTabValueMode    value_mode;       /* default: XmTAB_VALUE_COPY         */
} XmTabAttributeRec, * XmTabAttributes;

typedef struct _XmTabbedStackListRec *XmTabbedStackList;

XmTabbedStackList XmTabbedStackListCreate(
#ifndef _NO_PROTO
void
#endif
);

XmTabbedStackList XmTabbedStackListCopy(
#ifndef _NO_PROTO
XmTabbedStackList
#endif
);

void XmTabbedStackListFree(
#ifndef _NO_PROTO
XmTabbedStackList
#endif
);

void XmTabbedStackListRemove(
#ifndef _NO_PROTO
XmTabbedStackList, int
#endif
);

int XmTabbedStackListInsert(
#ifndef _NO_PROTO
XmTabbedStackList, int, XtValueMask, XmTabAttributes
#endif
);

int XmTabbedStackListAppend(
#ifndef _NO_PROTO
XmTabbedStackList, XtValueMask, XmTabAttributes
#endif
);

void XmTabbedStackListModify(
#ifndef _NO_PROTO
XmTabbedStackList, int, XtValueMask, XmTabAttributes
#endif
);

void XmTabbedStackListQuery(
#ifndef _NO_PROTO
XmTabbedStackList, int, XmTabAttributes
#endif
);

int XmTabbedStackListFind(
#ifndef _NO_PROTO
XmTabbedStackList, XmString
#endif
);

void XmTabbedStackListSimpleRemove(
#ifndef _NO_PROTO
XmTabbedStackList, XmString
#endif
);

int XmTabbedStackListSimpleInsert(
#ifndef _NO_PROTO
XmTabbedStackList, int, XmString
#endif
);

int XmTabbedStackListSimpleAppend(
#ifndef _NO_PROTO
XmTabbedStackList, XmString
#endif
);

void XmTabbedStackListSimpleModify(
#ifndef _NO_PROTO
XmTabbedStackList, int, XmString
#endif
);

XmString XmTabbedStackListSimpleQuery(
#ifndef _NO_PROTO
XmTabbedStackList, int
#endif
);

XmTabResult XmTabbedStackListCompare(
#ifndef _NO_PROTO
XmTabbedStackList, XmTabbedStackList
#endif
);

void XmTabAttibutesFree(
#ifndef _NO_PROTO
XmTabAttributes
#endif
);

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif


#endif 

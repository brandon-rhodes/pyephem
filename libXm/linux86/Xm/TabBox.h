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

#ifndef _XmTabBox_h_
#define _XmTabBox_h_

#include <Xm/Ext.h>
#include <Xm/DrawUtils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {XmTABS_SQUARED, XmTABS_ROUNDED, XmTABS_BEVELED} XmTabStyle;
typedef enum {XmTABS_BASIC, XmTABS_STACKED, XmTABS_STACKED_STATIC,
	      XmTABS_SCROLLED, XmTABS_OVERLAYED} XmTabMode;

typedef enum {XmTAB_ORIENTATION_DYNAMIC, XmTABS_RIGHT_TO_LEFT,
	      XmTABS_LEFT_TO_RIGHT, XmTABS_TOP_TO_BOTTOM,
	      XmTABS_BOTTOM_TO_TOP} XmTabOrientation;

typedef enum {XmTAB_EDGE_TOP_LEFT, XmTAB_EDGE_BOTTOM_RIGHT} XmTabEdge;

typedef enum {XmTAB_ARROWS_ON_RIGHT, XmTAB_ARROWS_ON_LEFT,
	      XmTAB_ARROWS_SPLIT} XmTabArrowPlacement;

enum {XmCR_TAB_SELECTED, XmCR_TAB_UNSELECTED};

typedef struct _XmTabBoxCallbackStruct {
    int       reason;
    XEvent    *event;
    int       tab_index;
    int       old_index;
} XmTabBoxCallbackStruct;

typedef struct _XmTabBoxRec      *XmTabBoxWidget;
typedef struct _XmTabBoxClassRec *XmTabBoxWidgetClass;

extern WidgetClass               xiTabBoxWidgetClass;

#ifndef XmIsTabBox
#define XmIsTabBox(w) XtIsSubclass(w, xiTabBoxWidgetClass)
#endif /* XmIsTabBox */

Widget XmCreateTabBox(Widget, String, ArgList, Cardinal);
int XmTabBoxGetIndex(Widget, int, int);
int XmTabBoxGetNumRows(Widget);
int XmTabBoxGetNumColumns(Widget);
int XmTabBoxGetNumTabs(Widget);
int XmTabBoxGetTabRow(Widget, int);
int XmTabBoxXYToIndex(Widget, int, int);

#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* __TabBox_h__ */

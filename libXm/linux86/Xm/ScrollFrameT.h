/* $TOG: ScrollFrameT.h /main/6 1997/07/25 16:49:23 samborn $ */
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
#ifndef _XmScrollFrameT_H
#define _XmScrollFrameT_H

#include <Xm/Xm.h>
#include <Xm/NavigatorT.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTscrollFrame;

/* Trait structures and typedefs, place typedefs first */

typedef void (*XmScrollFrameInitProc)(Widget sf, 
				      XtCallbackProc moveCB,
				      Widget scrollable);
typedef Boolean  (*XmScrollFrameGetInfoProc)(Widget sf,
					     Cardinal * dimension,
					     Widget ** nav_list,
					     Cardinal * num_nav_list);
typedef void (*XmScrollFrameAddNavigatorProc)(Widget sf, 
					      Widget nav,
					      Mask dimMask);
typedef void (*XmScrollFrameRemoveNavigatorProc)(Widget sf, 
						 Widget nav);
typedef void (*XmScrollFrameUpdateOrigGeomProc)(Widget sf, 
						Widget child,
						XtWidgetGeometry *geom);


/* Version 1: added updateOrigGeom */

typedef struct _XmScrollFrameTraitRec {
  int				    version;		/* 1 */
  XmScrollFrameInitProc		    init;       
  XmScrollFrameGetInfoProc	    getInfo; 
  XmScrollFrameAddNavigatorProc     addNavigator;
  XmScrollFrameRemoveNavigatorProc  removeNavigator;
  XmScrollFrameUpdateOrigGeomProc   updateOrigGeom;
} XmScrollFrameTraitRec, *XmScrollFrameTrait;


/* This one gets allocated per instance by the scrollFrame
   class. It is just a convenient structure reusable by other scrollFrame
   and it needs not to be part of the public trait API */

typedef struct _XmScrollFrameDataRec {
   XtCallbackProc move_cb ;
   Widget         scrollable ;
   Widget *       nav_list;
   Cardinal       num_nav_list ;
   Cardinal       num_nav_slots;
} XmScrollFrameDataRec, *XmScrollFrameData;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScrollFrameT_H */

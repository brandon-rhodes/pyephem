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
/* $XConsortium: LabelGI.h /main/5 1995/07/13 17:31:31 drk $ */
#ifndef _XMLABELGI_H
#define _XMLABELGI_H

#include <Xm/LabelGP.h>
#include <Xm/MenuT.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern int _XmLabelCacheCompare( 
                        XtPointer A,
                        XtPointer B) ;
extern void _XmCalcLabelGDimensions( 
                        Widget wid) ;
extern void _XmReCacheLabG( 
                        Widget wid) ;
extern void _XmAssignLabG_MarginHeight( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmLabelGCalcTextRect( 
                        Widget wid) ;
extern void _XmAssignLabG_MarginWidth( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignLabG_MarginLeft( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignLabG_MarginRight( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignLabG_MarginTop( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignLabG_MarginBottom( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmProcessDrag( 
                        Widget w,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern Boolean _XmLabelGCVTRedraw (Widget kid, 
				   Widget cur_parent,
				   Widget new_parent,
				   Mask visual_flag);

extern void _XmRedisplayLabG (Widget      w,
			      XEvent     *event,
			      Region      region,
			      LRectangle *background_box);

extern void _XmLabelGCloneMenuSavvy(WidgetClass, XmMenuSavvyTrait);

extern void _XmLabelSetBackgroundGC(XmLabelGadget lw);
extern void _XmLabelGCalcTextRect(Widget wid);
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XMLABELGI_H */
/* DON'T ADD ANYTHING AFTER THIS #endif */

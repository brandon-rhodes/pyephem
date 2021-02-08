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
/*   $XConsortium: GMUtilsI.h /main/9 1995/07/13 17:26:45 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmGMUtilsI_h
#define _XmGMUtilsI_h


/* Include files:
*/
#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern void _XmGMCalcSize( 
                        XmManagerWidget manager,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        Dimension *replyWidth,
                        Dimension *replyHeight) ;
extern Boolean _XmGMDoLayout( 
                        XmManagerWidget manager,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        int resize_policy,
#if NeedWidePrototypes
                        int queryonly) ;
#else
                        Boolean queryonly) ;
#endif /* NeedWidePrototypes */
extern void _XmGMEnforceMargin( 
                        XmManagerWidget manager,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
                        int setvalue) ;
#else
                        Dimension margin_width,
                        Dimension margin_height,
                        Boolean setvalue) ;
#endif /* NeedWidePrototypes */
extern XtGeometryResult _XmGMHandleQueryGeometry( 
                        Widget widget,
                        XtWidgetGeometry *intended,
                        XtWidgetGeometry *desired,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        int resize_policy) ;
extern Boolean _XmGMOverlap( 
                        XmManagerWidget manager,
                        Widget w) ;
extern XtGeometryResult _XmGMHandleGeometryManager( 
                        Widget parent,
                        Widget w,
                        XtWidgetGeometry *request,
                        XtWidgetGeometry *reply,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        int resize_policy,
                        int allow_overlap) ;
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmGMUtilsI_h */
 /* DON'T ADD STUFF AFTER THIS #endif */

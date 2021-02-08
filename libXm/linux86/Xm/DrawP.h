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
/* $XConsortium: DrawP.h /main/10 1995/07/14 10:27:48 drk $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDrawP_h
#define _XmDrawP_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------*/
/*   Functions used by Xm widgets for the Motif visual drawing   */
/*---------------------------------------------------------------*/
/* All these functions have an Xlib draw like API: 
      a Display*, a Drawable, then GCs, Positions and Dimensions 
      and finally some specific paramaters */

/******** The Draw.c file has been split in several module for
          a better link profile *********/

/*---------------------------------------------------------------
  XmeDrawShadows, 
       use in place of the 1.1 _XmDrawShadow and _XmDrawShadowType
       with changes to the interface (widget vs window, offsets, new order)
       and in the implementation (uses XSegments instead of XRectangles).
       Both etched and regular shadows use now a single private routine
       xmDrawSimpleShadow.
    XmeDrawHighlight.
       Implementation using FillRectangles, for solid highlight only. 
    _XmDrawHighlight.
       Highlight using wide lines, so that dash mode works. 
    XmeClearBorder,    
       new name for _XmEraseShadow  (_XmClearShadowType, which clear half a 
       shadow with a 'widget' API stays in Manager.c ) 
       XmClearBorder is only usable on window, not on drawable.
    XmeDrawSeparator, 
       use in place of the duplicate redisplay method of both separator and 
       separatorgadget (highlight_thickness not used, must be incorporated
       in the function call parameters). use xmDrawSimpleShadow.
       Has 2 new separator types for dash shadowed lines.
    XmeDrawDiamond, 
       new interface for _XmDrawDiamondButton (_XmDrawSquareButton is
       really a simple draw shadow and will be in the widget file as is).
    XmeDrawArrow, 
       same algorithm as before but in one function that re-uses the malloced
       rects and does not store anything in the wigdet instance.
    XmeDrawPolygonShadow,
       new one that use the RegionDrawShadow API to implement an Xme call 
    XmeDrawCircle,
       new one for toggle visual
    XmeDrawIndicator
       new one for toggle drawing
---------------------------------------------------------------------------*/


/********    Private Function Declarations    ********/

extern void XmeDrawShadows( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shad_thick,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shad_thick,
#endif /* NeedWidePrototypes */
                        unsigned int shad_type);
extern void XmeClearBorder( 
                        Display *display,
                        Window w,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick);
#endif /* NeedWidePrototypes */
extern void XmeDrawSeparator( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
                        GC separator_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        int margin,
                        unsigned int orientation,
                        unsigned int separator_type);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        Dimension margin,
                        unsigned char orientation,
                        unsigned char separator_type);
#endif /* NeedWidePrototypes */
extern void XmeDrawDiamond( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
                        GC center_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        int margin);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        Dimension margin);
#endif /* NeedWidePrototypes */

extern void XmeDrawCircle( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
                        GC center_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        int margin);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        Dimension margin);
#endif /* NeedWidePrototypes */

extern void XmeDrawHighlight( 
                        Display *display,
                        Drawable d,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int highlight_thick
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension highlight_thick
#endif /* NeedWidePrototypes */
                        );
extern void XmeDrawArrow( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bot_gc,
                        GC cent_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        unsigned int direction);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        unsigned char direction);
#endif /* NeedWidePrototypes */

extern void XmeDrawPolygonShadow(
		      Display *dpy,
		      Drawable d,
		      GC topGC,
		      GC bottomGC,
		      XPoint *points,
		      int n_points,
#if NeedWidePrototypes
		      int shadowThickness,
		      unsigned int shadowType);
#else
		      Dimension shadowThickness,
		      unsigned char shadowType);
#endif /* NeedWidePrototypes */

extern void XmeDrawIndicator(Display *display, 
		 Drawable d, 
		 GC gc, 
#if NeedWidePrototypes
		 int x, int y, 
		 int width, int height, 
		 int margin,
		 int type);
#else
                 Position x, Position y, 
                 Dimension width, Dimension height,
		 Dimension margin, 
                 XtEnum type);
#endif /* NeedWidePrototypes */

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDrawP_h */

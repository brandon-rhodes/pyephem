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
/* $XConsortium: RegionI.h /main/10 1995/07/13 17:47:08 drk $ */
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1987, 1989, 1990 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/* (c) Copyright 1988 MICROSOFT CORPORATION */

#ifndef _XmRegionI_h
#define _XmRegionI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISEMPTY(r) ((r)->numRects == 0)

/*  1 if two Boxs overlap.
 *  0 if two Boxs do not overlap.
 *  Remember, x2 and y2 are not in the region 
 */
#define EXTENTCHECK(r1, r2) \
	((r1)->x2 > (r2)->x1 && \
	 (r1)->x1 < (r2)->x2 && \
	 (r1)->y2 > (r2)->y1 && \
	 (r1)->y1 < (r2)->y2)

/*
 *  update region extents
 */
#define EXTENTS(r,idRect){\
            if((r)->x1 < (idRect)->extents.x1)\
              (idRect)->extents.x1 = (r)->x1;\
            if((r)->y1 < (idRect)->extents.y1)\
              (idRect)->extents.y1 = (r)->y1;\
            if((r)->x2 > (idRect)->extents.x2)\
              (idRect)->extents.x2 = (r)->x2;\
            if((r)->y2 > (idRect)->extents.y2)\
              (idRect)->extents.y2 = (r)->y2;\
        }

/*
 *   Check to see if there is enough memory in the present region.
 */
#define MEMCHECK(reg, rect, firstrect){\
        if ((reg)->numRects >= ((reg)->size - 1)){\
          (firstrect) = (XmRegionBox *) XtRealloc \
          ((char *)(firstrect), \
	  (unsigned) (2*(sizeof(XmRegionBox))*((reg)->size)));\
          if ((firstrect) == 0)\
            return;\
          (reg)->size *= 2;\
          (rect) = &(firstrect)[(reg)->numRects];\
         }\
       }

/*  this routine checks to see if the previous rectangle is the same
 *  or subsumes the new rectangle to add.
 */

#define CHECK_PREVIOUS(Reg, R, Rx1, Ry1, Rx2, Ry2)\
               (!(((Reg)->numRects > 0)&&\
                  ((R-1)->y1 == (Ry1)) &&\
                  ((R-1)->y2 == (Ry2)) &&\
                  ((R-1)->x1 <= (Rx1)) &&\
                  ((R-1)->x2 >= (Rx2))))

/*  add a rectangle to the given XmRegion */
#define ADDRECT(reg, r, fr, rx1, ry1, rx2, ry2){\
    if (((rx1) < (rx2)) && ((ry1) < (ry2)) && \
        CHECK_PREVIOUS((reg), (r), (rx1), (ry1), (rx2), (ry2))){\
              if ((reg)->numRects == (reg)->size){\
                 if (!(reg)->size) (reg)->size = 1;\
                 else (reg)->size += (reg)->numRects;\
                 (reg)->rects = (XmRegionBox *) realloc((reg)->rects,\
                                                        sizeof(XmRegionBox) *\
                                                        (reg)->size);\
                 fr = REGION_BOXPTR(reg);\
                 r = fr + (reg)->numRects;\
              }\
              (r)->x1 = (rx1);\
              (r)->y1 = (ry1);\
              (r)->x2 = (rx2);\
              (r)->y2 = (ry2);\
              EXTENTS((r), (reg));\
              (reg)->numRects++;\
              (r)++;\
            }\
        }



/*  add a rectangle to the given XmRegion */
#define ADDRECTNOX(reg, r, rx1, ry1, rx2, ry2){\
            if ((rx1 < rx2) && (ry1 < ry2) &&\
                CHECK_PREVIOUS((reg), (r), (rx1), (ry1), (rx2), (ry2))){\
              (r)->x1 = (rx1);\
              (r)->y1 = (ry1);\
              (r)->x2 = (rx2);\
              (r)->y2 = (ry2);\
              (reg)->numRects++;\
              (r)++;\
            }\
        }

#define INBOX(r, x, y) \
      ( ( ((r).x2 >= x)) && \
        ( ((r).x1 <= x)) && \
        ( ((r).y2 >= y)) && \
        ( ((r).y1 <= y)) )

/*
 * used by _XmRegionDrawShadow
 */

#define TL_OPEN		(1 << 0)
#define BL_OPEN		(1 << 1)
#define TR_OPEN		(1 << 2)
#define BR_OPEN		(1 << 3)
#define TL_MATCH	(1 << 4)
#define BL_MATCH	(1 << 5)
#define TR_MATCH	(1 << 6)
#define BR_MATCH	(1 << 7)

/*
 * The following macro were ported from the X server include file regionstr.h
 */
#define REGION_BOXPTR(reg) ((XmRegionBox *)((reg)->rects))


/********    Private Function Declarations for Region.c    ********/

extern XmRegion _XmRegionCreate( void ) ;
extern XmRegion _XmRegionCreateSize(
			long size) ;
extern void _XmRegionComputeExtents(
			XmRegion r) ;
extern void _XmRegionGetExtents( 
                        XmRegion r,
                        XRectangle *rect) ;
extern void _XmRegionUnionRectWithRegion( 
                        XRectangle *rect,
                        XmRegion source,
                        XmRegion dest) ;
extern void _XmRegionIntersectRectWithRegion( 
                        XRectangle *rect,
                        XmRegion source,
                        XmRegion dest) ;
extern long _XmRegionGetNumRectangles(
			XmRegion r) ;
extern void _XmRegionGetRectangles( 
                        XmRegion r,
                        XRectangle **rects,
                        long *nrects) ;
extern void _XmRegionSetGCRegion( 
                        Display *dpy,
                        GC gc,
			int x_origin,
			int y_origin,
                        XmRegion r) ;
extern void _XmRegionDestroy( 
                        XmRegion r) ;
extern void _XmRegionOffset( 
                        XmRegion pRegion,
                        int x,
                        int y) ;
extern void _XmRegionIntersect( 
                        XmRegion reg1,
                        XmRegion reg2,
                        XmRegion newReg) ;
extern void _XmRegionUnion( 
                        XmRegion reg1,
                        XmRegion reg2,
                        XmRegion newReg) ;
extern void _XmRegionSubtract( 
                        XmRegion regM,
                        XmRegion regS,
                        XmRegion regD) ;
extern Boolean _XmRegionIsEmpty( 
                        XmRegion r) ;
extern Boolean _XmRegionEqual( 
                        XmRegion r1,
                        XmRegion r2) ;
extern Boolean _XmRegionPointInRegion( 
                        XmRegion pRegion,
                        int x,
                        int y) ;
extern void _XmRegionClear(
			XmRegion r ) ;
extern void _XmRegionShrink(
			XmRegion r,
			int dx,
                        int dy) ;
extern void _XmRegionDrawShadow(
			Display	*display,
			Drawable d,
			GC top_gc,
			GC bottom_gc,
			XmRegion region,
#if NeedWidePrototypes
			int border_thick,
			int shadow_thick,
#else
			Dimension border_thick,
			Dimension shadow_thick,
#endif /* NeedWidePrototypes */
			unsigned int shadow_type ) ;

extern XmRegion _XmRegionFromImage(
			XImage *image );

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmRegionI_h */
/* DON'T ADD STUFF AFTER THIS #endif */

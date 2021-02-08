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
/*   $XConsortium: ArrowBGP.h /main/13 1995/07/14 10:09:51 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmArrowGadgetP_h
#define _XmArrowGadgetP_h

#include <Xm/ArrowBG.h>
#include <Xm/GadgetP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Arrow class structure  */

typedef struct _XmArrowButtonGadgetClassPart
{
   XtPointer extension;
} XmArrowButtonGadgetClassPart;


/*  Full class record declaration for Arrow class  */

typedef struct _XmArrowButtonGadgetClassRec
{
  RectObjClassPart             rect_class;
  XmGadgetClassPart            gadget_class;
  XmArrowButtonGadgetClassPart arrow_button_class;
} XmArrowButtonGadgetClassRec;

externalref XmArrowButtonGadgetClassRec xmArrowButtonGadgetClassRec;

/*  The Arrow instance record  */

typedef struct _XmArrowButtonGadgetPart
{
  XtCallbackList activate_callback;
  XtCallbackList arm_callback;
  XtCallbackList disarm_callback;
  unsigned char  direction;	/* The direction the arrow is pointing. */

  Boolean	 selected;

  short		 top_count;
  short		 cent_count;
  short		 bot_count;
  XRectangle	*top;
  XRectangle	*cent;
  XRectangle	*bot;

  Position	 old_x;
  Position	 old_y;

  GC		 arrow_GC;
  XtIntervalId	 timer;	
  unsigned char	 multiClick;	/* KEEP/DISCARD resource */
  int		 click_count;
  GC		 insensitive_GC;

   
  GC		 background_GC;
  GC		 top_shadow_GC;
  GC		 bottom_shadow_GC;
  GC		 highlight_GC;
   
  Pixel		 foreground;
  Pixel		 background;
   
  Pixel		 top_shadow_color;
  Pixmap	 top_shadow_pixmap;
   
  Pixel		 bottom_shadow_color;
  Pixmap	 bottom_shadow_pixmap;
   
  Pixel		 highlight_color;
  Pixmap	 highlight_pixmap;

  Boolean	 fill_bg_box;
  Dimension detail_shadow_thickness ;
} XmArrowButtonGadgetPart;


/*  Full instance record declaration  */

typedef struct _XmArrowButtonGadgetRec
{
   ObjectPart              object;
   RectObjPart             rectangle;
   XmGadgetPart            gadget;
   XmArrowButtonGadgetPart arrowbutton;
} XmArrowButtonGadgetRec;



#define ArrowBG_BackgroundGC(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. background_GC)
#define ArrowBG_TopShadowGC(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. top_shadow_GC)
#define ArrowBG_BottomShadowGC(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. bottom_shadow_GC)
#define ArrowBG_HighlightGC(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. highlight_GC)
#define ArrowBG_Foreground(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. foreground)
#define ArrowBG_Background(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. background)
#define ArrowBG_TopShadowColor(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. top_shadow_color)
#define ArrowBG_TopShadowPixmap(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. top_shadow_pixmap)
#define ArrowBG_BottomShadowColor(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. bottom_shadow_color)
#define ArrowBG_BottomShadowPixmap(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. bottom_shadow_pixmap)
#define ArrowBG_HighlightColor(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. highlight_color)
#define ArrowBG_HighlightPixmap(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. highlight_pixmap)

/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmArrowGadgetP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

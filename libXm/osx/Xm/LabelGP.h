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
/* $XConsortium: LabelGP.h /main/12 1995/10/25 20:08:21 cde-sun $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmLabelGP_h
#define _XmLabelGP_h

#include <Xm/LabelG.h>
#include <Xm/GadgetP.h>
#include <Xm/ExtObjectP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************/
/* The Label Gadget Cache Object's class and instance records*/
/*************************************************************/

/* Enumerations for fill_bg_box. */
enum { _XmPLAIN_BG_BOX, _XmFILL_BG_BOX, _XmALWAYS_FILL_BG_BOX };

typedef struct _XmLabelGCacheObjClassPart
{
  int foo;
} XmLabelGCacheObjClassPart;


typedef struct _XmLabelGCacheObjClassRec     /* label cache class record */
{
  ObjectClassPart		object_class;
  XmExtClassPart                ext_class;
  XmLabelGCacheObjClassPart	label_class_cache;
} XmLabelGCacheObjClassRec;

externalref XmLabelGCacheObjClassRec xmLabelGCacheObjClassRec;


/*  The Label Gadget Cache instance record  */

typedef struct _XmLabelGCacheObjPart
{
  unsigned char	label_type;
  unsigned char	alignment;
  unsigned char	string_direction;
  
  Dimension	margin_height;   /* margin around widget */
  Dimension	margin_width;
  
  Dimension	margin_left;    /* additional margins on */
  Dimension	margin_right;   /* each side of widget */
  Dimension	margin_top;     /* text (or pixmap) is placed */
  Dimension	margin_bottom;  /* inside the margins */
  
  Boolean	recompute_size;
  
  Boolean	skipCallback;	/* set by RowColumn with entryCallback */
  unsigned char menu_type;

  /*
   * Following are color resources and instance variables for Gadgets.
   * Because of problems involving the history of the gadget cache these
   * variables appear here rather than in XmGadget.
   *
   * XmArrowButtonGadget and XmSeparatorGadget have effectively the same
   * set of color resources implemented for each.
   * 
   * XmLabelGadget does not use all of these variables (e.g top_shadow_color).
   * It initializes them and maintains them but does not use all of them
   * to render the label gadget.  They are here for use by subclasses.
   *
   *  Sterling Barrett
   *  sterling@ics.com
   */
  
  GC		background_GC;
  GC		top_shadow_GC;
  GC		bottom_shadow_GC;
  GC		highlight_GC;
  
  Pixel		foreground;
  Pixel		background;
  
  Pixel		top_shadow_color;
  Pixmap	top_shadow_pixmap;
  
  Pixel		bottom_shadow_color;
  Pixmap	bottom_shadow_pixmap;
  
  Pixel		highlight_color;
  Pixmap	highlight_pixmap;
} XmLabelGCacheObjPart;

typedef struct _XmLabelGCacheObjRec
{
  ObjectPart            object;
  XmExtPart		ext;
  XmLabelGCacheObjPart  label_cache;
} XmLabelGCacheObjRec;

/*  The Label Widget Class and instance records  */
/*************************************************/

typedef struct _XmLabelGadgetClassPart     /* label class record */
{
  XtWidgetProc		setOverrideCallback;
  XmMenuProc		menuProcs;
  XtPointer		extension;
} XmLabelGadgetClassPart;

typedef struct _XmLabelGadgetClassRec
{
  RectObjClassPart       rect_class;
  XmGadgetClassPart      gadget_class;
  XmLabelGadgetClassPart label_class;
} XmLabelGadgetClassRec;

externalref XmLabelGadgetClassRec xmLabelGadgetClassRec;

typedef struct _XmLabelGadgetPart
{
  _XmString	  _label;  /* String sent to this widget */
  _XmString       _acc_text;
  KeySym          mnemonic;
  XmStringCharSet mnemonicCharset;
  char 		  *accelerator;
  XmFontList	  font;
  
  Pixmap	  pixmap; 
  Pixmap	  pixmap_insen; 
  
  /* PRIVATE members -- values computed by LabelWidgetClass methods */
  
  GC		  normal_GC;   /* GC for text */	
  GC		  insensitive_GC;
  XRectangle	  TextRect;     /* The bounding box of the text or clip */
  XRectangle	  acc_TextRect; /* rectangle of the window; whichever is
				 smaller */
  
  XmLabelGCacheObjPart *cache;
  
  Dimension	  acc_left_delta;  /* Amount we increased the margins */
  Dimension	  acc_right_delta; /* to accomodate accelerator text */

  Dimension	* baselines;	/* Cached baseline information. */

  XtEnum          fill_bg_box;     /* Computed by LabelGadget or */
				   /* pinned by subclasses. */
  Boolean	  check_set_render_table;

  XmPixmapPlacement pixmap_placement; /* Where to place the icon label. */
  Dimension pixmap_text_padding; /* padding between the icon and the text. */

  XRectangle	  PixmapRect;   /* The bounding box of the pixmap in TextRect */
  XRectangle	  StringRect;   /* The bounding box of the string in TextRect */

} XmLabelGadgetPart;


typedef struct _XmLabelGadgetRec
{
  ObjectPart        object;
  RectObjPart       rectangle;
  XmGadgetPart      gadget;
  XmLabelGadgetPart label;
} XmLabelGadgetRec;

typedef struct _LRectangle
{
  int x, y;
  int width, height;
} LRectangle;

/* Inherited  Functions exported by label */

#define XmInheritSetOverrideCallback	((XtWidgetProc) _XtInherit)
#define XmInheritResize			((XtWidgetProc) _XtInherit)

#define INVALID_PIXEL	((Pixel) -1)
#define INVALID_PIXMAP	((Pixmap) -1)
/* Padding between label text and accelerator text */

#define LABELG_ACC_PAD	15

/* MACROS */
/********
 * Macros for cached instance fields
 */
#define LabG_LabelType(w)		(((XmLabelGadget)(w)) -> \
					   label.cache-> label_type)
#define LabG_Alignment(w)		(((XmLabelGadget)(w)) -> \
					   label.cache-> alignment)
#define LabG_StringDirection(w)		(((XmLabelGadget)(w)) -> \
					   label.cache-> string_direction)
#define LabG_MarginHeight(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> margin_height)
#define LabG_MarginWidth(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> margin_width)
#define LabG_MarginLeft(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> margin_left)
#define LabG_MarginRight(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> margin_right)
#define LabG_MarginTop(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> margin_top)
#define LabG_MarginBottom(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> margin_bottom)
#define LabG_RecomputeSize(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> recompute_size)
#define LabG_SkipCallback(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> skipCallback)
#define LabG_MenuType(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> menu_type)
#define LabG_BackgroundGC(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> background_GC)
#define LabG_TopShadowGC(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> top_shadow_GC)
#define LabG_BottomShadowGC(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> bottom_shadow_GC)
#define LabG_HighlightGC(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> highlight_GC)
#define LabG_Foreground(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> foreground)
#define LabG_Background(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> background)
#define LabG_TopShadowColor(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> top_shadow_color)
#define LabG_TopShadowPixmap(w)		(((XmLabelGadget)(w)) -> \
                                           label.cache-> top_shadow_pixmap)
#define LabG_BottomShadowColor(w)	(((XmLabelGadget)(w)) -> \
                                           label.cache-> bottom_shadow_color)
#define LabG_BottomShadowPixmap(w)	(((XmLabelGadget)(w)) -> \
                                           label.cache-> bottom_shadow_pixmap)
#define LabG_HighlightColor(w)	        (((XmLabelGadget)(w)) -> \
                                           label.cache-> highlight_color)
#define LabG_HighlightPixmap(w)	        (((XmLabelGadget)(w)) -> \
                                           label.cache-> highlight_pixmap)
/********
 * Macros for UNcached instance fields
 */
#define LabG__label(w)			(((XmLabelGadget)(w)) -> \
					   label._label)
#define LabG__acceleratorText(w)	(((XmLabelGadget)(w)) -> \
                                           label._acc_text)
#define LabG_Font(w)			(((XmLabelGadget)(w)) -> \
					   label.font)
#define LabG_Mnemonic(w)		(((XmLabelGadget)(w)) -> \
					   label.mnemonic)
#define LabG_MnemonicCharset(w)         (((XmLabelGadget)(w)) -> \
                                           label.mnemonicCharset)
#define LabG_Accelerator(w)		(((XmLabelGadget)(w)) -> \
                                           label.accelerator)
#define LabG_Pixmap(w)			(((XmLabelGadget)(w)) -> \
                                           label.pixmap)
#define LabG_PixmapInsensitive(w)	(((XmLabelGadget)(w)) -> \
                                           label.pixmap_insen)
#define LabG_NormalGC(w)		(((XmLabelGadget)(w)) -> \
                                           label.normal_GC)
#define LabG_InsensitiveGC(w)		(((XmLabelGadget)(w)) -> \
                                           label.insensitive_GC)
#define LabG_TextRect(w)		(((XmLabelGadget)(w)) -> \
                                           label.TextRect)
#define LabG_AccTextRect(w)		(((XmLabelGadget)(w)) -> \
                                           label.acc_TextRect)
#define LabG_PixmapRect(w)		(((XmLabelGadget)(w)) -> \
                                           label.PixmapRect)
#define LabG_StringRect(w)		(((XmLabelGadget)(w)) -> \
                                           label.StringRect)
#define LabG_PixmapPlacement(w)		(((XmLabelGadget)(w)) -> \
                                           label.pixmap_placement)
#define LabG_PixmapTextPadding(w)	(((XmLabelGadget)(w)) -> \
                                           label.pixmap_text_padding)


/********
 * Convenience Macros 
 */
#define LabG_TextRect_x(w)		(LabG_TextRect(w).x)
                                           
#define LabG_TextRect_y(w)		(LabG_TextRect(w).y)
                                           
#define LabG_TextRect_width(w)		(LabG_TextRect(w).width)
                                           
#define LabG_TextRect_height(w)		(LabG_TextRect(w).height)

#define LabG_IsText(w)			(LabG_LabelType(w) == XmSTRING)

#define LabG_IsPixmap(w)		(LabG_LabelType(w) == XmPIXMAP)

#define LabG_IsPixmapAndText(w)		(LabG_LabelType(w) == \
					XmPIXMAP_AND_STRING)

#define LabG_Cache(w)			(((XmLabelGadget)(w))-> \
					   label.cache)
#define LabG_Shadow(w)    (((XmLabelGadget)(w))->gadget.shadow_thickness)
#define LabG_Highlight(w) (((XmLabelGadget)(w))->gadget.highlight_thickness)
#define LabG_Baseline(w)  (_XmStringBaseline ((LabG_Font(w)), (LabG__Label(w))))
#define LabG_ClassCachePart(w)	\
	(((XmLabelGadgetClass)xmLabelGadgetClass)->gadget_class.cache_part)

#define LabG_IsMenupane(w)	((LabG_MenuType(w) == XmMENU_POPUP) ||	\
				 (LabG_MenuType(w) == XmMENU_PULLDOWN))

#define _XmAssignLabG_MarginHeight_r(cache, val) \
		cache->margin_height = val
#define _XmAssignLabG_MarginWidth_r(cache, val) \
		cache->margin_width = val
#define _XmAssignLabG_MarginLeft_r(cache, val) \
		cache->margin_left = val
#define _XmAssignLabG_MarginRight_r(cache, val) \
		cache->margin_right = val
#define _XmAssignLabG_MarginTop_r(cache, val) \
		cache->margin_top = val
#define _XmAssignLabG_MarginBottom_r(cache, val) \
		cache->margin_bottom = val

extern void _XmQualifyLabelLocalCache(XmLabelGCacheObjPart *, XmLabelGadget);
extern void _XmReCacheLabG_r(XmLabelGCacheObjPart *, XmLabelGadget);

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmLabelGP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

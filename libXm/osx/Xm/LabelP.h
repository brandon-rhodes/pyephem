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
/* $XConsortium: LabelP.h /main/13 1995/10/25 20:08:32 cde-sun $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmLabelP_h_
#define _XmLabelP_h_

#include <Xm/Label.h>
#include <Xm/PrimitiveP.h>


#ifdef __cplusplus
extern "C" {
#endif

/*  The Label Widget Class and instance records  */

typedef struct _XmLabelClassPart     /* label class record */
{
  XtWidgetProc	setOverrideCallback;
  XmMenuProc	menuProcs;
  String	translations;
  XtPointer	extension;
} XmLabelClassPart;

typedef struct _XmLabelClassRec
{
  CoreClassPart		core_class;
  XmPrimitiveClassPart	primitive_class;
  XmLabelClassPart	label_class;
} XmLabelClassRec;

externalref XmLabelClassRec xmLabelClassRec;


/* Inherited  Functions exported by label */

#define XmInheritSetOverrideCallback	((XtWidgetProc) _XtInherit)
#define XmInheritResize			((XtWidgetProc) _XtInherit)
#define XmInheritRealize		((XtRealizeProc) _XtInherit)

/* The padding between label text and accelerator text */

# define LABEL_ACC_PAD 		15

/*  The Label instance record  */

typedef struct _XmLabelPart
{
  _XmString	  _label;	 /* String sent to this widget */
  _XmString	  _acc_text;
  KeySym	  mnemonic;
  XmStringCharSet mnemonicCharset;
  char		 *accelerator;
  unsigned char	  label_type;
  unsigned char	  alignment;
  unsigned char	  string_direction;
  XmFontList	  font;
	
  Dimension	  margin_height;  /* margin around widget */
  Dimension	  margin_width;
  
  Dimension 	  margin_left;    /* additional margins on */
  Dimension	  margin_right;   /* each side of widget */
  Dimension	  margin_top;     /* text (or pixmap) is placed */
  Dimension	  margin_bottom;  /* inside the margins */
  
  Boolean	  recompute_size;
  
  Pixmap	  pixmap; 
  Pixmap	  pixmap_insen; 
  
  /* PRIVATE members -- values computed by LabelWidgetClass methods */
  
  GC		  normal_GC;   /* GC for text */	
  GC		  insensitive_GC;
  XRectangle	  TextRect;	/* The bounding box of the text or clip */
  XRectangle	  acc_TextRect; /* rectangle of the window; whichever is */
				/* smaller */
  
  Boolean	  skipCallback; /* set by RowColumn with entryCallback. */
  unsigned char   menu_type;
  Boolean	  computing_size; /* suppresses DrawnB resize callbacks. */
  
  Dimension	  acc_left_delta;  /* Amount we increased the margins */
  Dimension	  acc_right_delta; /* to accomodate accelerator text. */

  Dimension	* baselines;	/* Cached baseline information */
  
  Boolean	  check_set_render_table;

  XmPixmapPlacement pixmap_placement; /* Where to place the icon label. */
  Dimension pixmap_text_padding; /* padding between the icon and the text. */

  XRectangle	  PixmapRect;   /* The bounding box of the pixmap in TextRect */
  XRectangle	  StringRect;   /* The bounding box of the string in TextRect */
} XmLabelPart;


typedef struct _XmLabelRec
{
   CorePart         core;
   XmPrimitivePart  primitive;
   XmLabelPart	    label;
} XmLabelRec;


/* MACROS */

#define Lab_MarginWidth(w)	(((XmLabelWidget)(w)) ->label.margin_width)
#define Lab_MarginHeight(w)	(((XmLabelWidget)(w)) ->label.margin_height)
#define Lab_MarginTop(w)	(((XmLabelWidget)(w)) ->label.margin_top)
#define Lab_MarginBottom(w)	(((XmLabelWidget)(w)) ->label.margin_bottom)
#define Lab_MarginRight(w)	(((XmLabelWidget)(w)) ->label.margin_right)
#define Lab_MarginLeft(w)	(((XmLabelWidget)(w)) ->label.margin_left)
#define Lab_TextRect_x(w)	(((XmLabelWidget)(w)) ->label.TextRect.x)
#define Lab_TextRect_y(w)	(((XmLabelWidget)(w)) ->label.TextRect.y)
#define Lab_TextRect_width(w)	(((XmLabelWidget)(w)) ->label.TextRect.width)
#define Lab_TextRect_height(w)	(((XmLabelWidget)(w)) ->label.TextRect.height)

#define Lab_IsText(w)	(((XmLabelWidget)(w)) ->label.label_type == XmSTRING)
#define Lab_IsPixmap(w)	(((XmLabelWidget)(w)) ->label.label_type == XmPIXMAP)
#define Lab_IsPixmapAndText(w)	(((XmLabelWidget)(w)) ->label.label_type == \
				XmPIXMAP_AND_STRING)

#define Lab_Font(w)		(((XmLabelWidget)(w)) ->label.font)

#define Lab_Mnemonic(w)		(((XmLabelWidget)(w)) ->label.mnemonic)
#define Lab_Accelerator(w)	(((XmLabelWidget)(w)) ->label.accelerator)
#define Lab_AcceleratorText(w)	(((XmLabelWidget)(w)) ->label.acc_text)
#define Lab_MenuType(w)		(((XmLabelWidget)(w))->label.menu_type)
#define Lab_Shadow(w)	 (((XmLabelWidget)(w))->primitive.shadow_thickness)
#define Lab_Highlight(w) (((XmLabelWidget)(w))->primitive.highlight_thickness)
#define Lab_Baseline(w)						\
  (_XmStringBaseline (((XmLabelWidget)(w))->label.font,		\
		      ((XmLabelWidget)(w))->label._label))

#define Lab_ComputingSize(w)	(((XmLabelWidget)(w))->label.computing_size)
#define Lab_IsMenupane(w)	((Lab_MenuType(w) == XmMENU_POPUP) ||	\
				 (Lab_MenuType(w) == XmMENU_PULLDOWN))


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmLabelP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

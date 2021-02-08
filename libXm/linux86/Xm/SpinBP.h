/* $TOG: SpinBP.h /main/7 1999/09/01 17:14:17 mgreess $ */
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

#ifndef _SpinBP_h
#define _SpinBP_h

#include <Xm/LabelP.h>
#include <Xm/ManagerP.h>
#include <Xm/SpinB.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Spin constraints */

typedef struct _XmSpinBoxConstraint
{
  XmStringTable  values;        /* array of XmString */
  int            position;      /* position of current selection */
  int            num_values;    /* number of XmString in the array */

  int            minimum_value; /* parameters for autonumeric mode */
  int            maximum_value;
  int            increment_value;
  short	         decimal_points;

  unsigned char  sb_child_type;
  unsigned char  arrow_sensitivity;
  Boolean	 wrap;
  unsigned char  position_type; /* governs interpretation of .position:
					XmPOSITION_{ARRAY,VALUE} */
} XmSpinBoxConstraintPart, *XmSpinBoxConstraint;

typedef struct _XmSpinBoxConstraintRec
{
  XmManagerConstraintPart manager;
  XmSpinBoxConstraintPart spinBox;
} XmSpinBoxConstraintRec, *XmSpinBoxConstraintPtr;

typedef Widget	(*XmGetCallbackWidgetProc)(Widget);

/*  Spin class structure  */
typedef struct _XmSpinBoxClassPart
{
  XmGetCallbackWidgetProc    get_callback_widget;
  XtPointer                  extension;			/* not used */
} XmSpinBoxClassPart;


/*  Full class record declaration for Spin class  */

typedef struct _XmSpinBoxClassRec
{
  CoreClassPart              core_class;
  CompositeClassPart         composite_class;
  ConstraintClassPart        constraint_class;
  XmManagerClassPart         manager_class;
  XmSpinBoxClassPart         spinBox_class;
} XmSpinBoxClassRec;

externalref XmSpinBoxClassRec xmSpinBoxClassRec;

/*  The Spin instance record  */

typedef struct _XmSpinBoxPart
{
      Dimension	     arrow_size;
      Dimension	     margin_width;
      Dimension	     margin_height;
      Dimension	     spacing;
      unsigned int   initial_delay;
      unsigned int   repeat_delay;
      XtCallbackList modify_verify_cb; /* CB list for new selection */
      XtCallbackList value_changed_cb; /* CB list for new selection */
      XRectangle     up_arrow_rect;
      XRectangle     down_arrow_rect;
      Widget         textw;         /* text widget */
      int	     boundary;
      int	     last_hit;
      int	     make_change;
      Dimension	     ideal_height;
      Dimension	     ideal_width;
      GC	     arrow_gc;
      GC	     insensitive_gc;
      Mask	     dim_mask; 	    /* for the navigator trait */
      XtIntervalId   spin_timer;
      Boolean	     up_arrow_pressed;
      Boolean	     down_arrow_pressed;
      unsigned char  arrow_layout;
      unsigned char  default_arrow_sensitivity;
      Dimension	     detail_shadow_thickness;
      unsigned char  arrow_orientation;	/* Xm_ARROWS_{VERTICAL,HORIZONTAL} */
} XmSpinBoxPart;


/*  Full instance record declaration  */

typedef struct _XmSpinBoxRec
{
  CorePart              core;
  CompositePart         composite;
  ConstraintPart        constraint;
  XmManagerPart         manager;
  XmSpinBoxPart         spinBox;
} XmSpinBoxRec;

/*  Macros */
#define XmRPositionValue	"PositionValue"

#define SB_GetConstraintRec(w) \
  ((XmSpinBoxConstraint)(&((XmSpinBoxConstraintPtr) (w)->core.constraints)->spinBox))

#define SB_WithChild(w) \
  (((XmSpinBoxRec *) (w))->spinBox.textw != NULL)

#define SB_ChildCount(w) \
  (((XmSpinBoxRec *) (w))->composite.num_children)

#define SB_GetArrowOrientation(w) \
  (((XmSpinBoxRec *) (w))->spinBox.arrow_orientation)

#define SB_ChildIsNumeric(wc) \
  (((XmSpinBoxConstraint) (wc))->sb_child_type == (unsigned char) XmNUMERIC)

#define SB_ChildPositionTypeIsValue(wc) \
  (((XmSpinBoxConstraint) (wc))->position_type == \
   (unsigned char) XmPOSITION_VALUE)

#define SB_ChildMaximumPositionValue(wc) \
  ( ( ((wc) == (XmSpinBoxConstraint) NULL) ?  \
        0 : \
        ( (SB_ChildIsNumeric(wc)) ? \
	    (wc)->maximum_value : \
            ( ((wc)->num_values  > 0) ? \
		((wc)->num_values - 1) : \
		0 \
	    ) \
	) \
    ) \
  )

#define SB_ChildMinimumPositionValue(wc) \
  ( ( ((wc) == (XmSpinBoxConstraint) NULL) ?  \
        0 : \
        ( (SB_ChildIsNumeric(wc)) ? \
          (wc)->minimum_value : \
          0 \
        ) \
    ) \
  )

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* _SpinBP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */




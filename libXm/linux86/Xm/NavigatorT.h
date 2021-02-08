/* $XConsortium: NavigatorT.h /main/5 1995/07/15 20:53:08 drk $ */
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
#ifndef _XmNavigatorT_H
#define _XmNavigatorT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTnavigator;

/* Trait structures and typedefs, place typedefs first */


/* this structure is equivalent to an XPoint but in int,
   not in Position, which are short */
typedef struct _TwoDInt {
    int x;
    int y;
} TwoDIntRec, *TwoDInt;


/* this one can be expanded in the future */
typedef struct _XmNavigatorDataRec {
    Mask valueMask ;
    Mask dimMask ;
    TwoDIntRec value;
    TwoDIntRec minimum;
    TwoDIntRec maximum;
    TwoDIntRec slider_size;
    TwoDIntRec increment;
    TwoDIntRec page_increment;
} XmNavigatorDataRec, *XmNavigatorData;

#define NavAllValid             (OxFFFF)
#define NavDimMask		(1L<<0)
#define NavValue  		(1L<<1)
#define NavMinimum              (1L<<2)
#define NavMaximum		(1L<<3)
#define NavSliderSize		(1L<<4)
#define NavIncrement            (1L<<5)
#define NavPageIncrement	(1L<<6)



typedef void (*XmNavigatorMoveCBProc)(Widget nav, 
				      XtCallbackProc moveCB,
				      XtPointer closure,
				      Boolean setunset);
typedef void (*XmNavigatorSetValueProc)(Widget nav, 
					XmNavigatorData nav_data,
					Boolean notify);
typedef void (*XmNavigatorGetValueProc)(Widget nav, 
					XmNavigatorData nav_data);



/* Version 0: initial release. */

typedef struct _XmNavigatorTraitRec {
  int			  version;		/* 0 */
  XmNavigatorMoveCBProc   changeMoveCB;
  XmNavigatorSetValueProc setValue;
  XmNavigatorGetValueProc getValue;
} XmNavigatorTraitRec, *XmNavigatorTrait;


#define NavigDimensionX			(1L<<0)  
#define NavigDimensionY			(1L<<1)  

/* convenience Macros */
#define ACCESS_DIM(mask,field) ((mask & NavigDimensionX)?(field.x):(field.y))

#define ASSIGN_DIM(mask,field,val)	\
  {					\
    if (mask & NavigDimensionX)		\
      (field.x)=(val);			\
    else				\
      (field.y)=(val);			\
  }


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmNavigatorT_H */

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
/* $XConsortium: CascadeBGP.h /main/13 1996/03/25 14:52:33 pascale $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef  _XmCascadeBGP_h
#define  _XmCascadeBGP_h

#include <Xm/CascadeBG.h>
#include <Xm/LabelGP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * The Arrow Cache record for the menu cascade button
 *********************************************************************/
typedef struct _XmArrowPixmap
{
   Dimension height, width;
   unsigned int depth;
   unsigned char direction;
   Pixel top_shadow_color;
   Pixel bottom_shadow_color;
   Pixel foreground_color;
   Display *display;
   Screen *screen;
   Pixmap pixmap;
} XmArrowPixmap;


/*********************************************************************/
/* The CascadeButton Gadget Cache Object's class and instance records*/
/*********************************************************************/


typedef struct _XmCascadeButtonGCacheObjClassPart
{
  int foo;
} XmCascadeButtonGCacheObjClassPart;


typedef struct _XmCascadeButtonGCacheObjClassRec
{
  ObjectClassPart                     object_class;
  XmExtClassPart                      ext_class;
  XmLabelGCacheObjClassPart           label_class_cache;
  XmCascadeButtonGCacheObjClassPart   cascade_button_class_cache;
} XmCascadeButtonGCacheObjClassRec;

externalref XmCascadeButtonGCacheObjClassRec xmCascadeButtonGCacheObjClassRec;


typedef struct _XmCascadeButtonGCacheObjPart
{
  Pixmap              cascade_pixmap;         /* pixmap for the cascade */
  int                 map_delay;              /* time delay for posting */
  Pixmap	      armed_pixmap;
  GC                  arm_gc;
  GC                  background_gc;
} XmCascadeButtonGCacheObjPart;

typedef struct _XmCascadeButtonGCacheObjRec
{
  ObjectPart                   object;
  XmExtPart                    ext;
  XmLabelGCacheObjPart         label_cache;
  XmCascadeButtonGCacheObjPart cascade_button_cache;
} XmCascadeButtonGCacheObjRec;

/* The CascadeButtonGadget instance record */

typedef	struct _XmCascadeButtonGadgetPart
{			/* resources */
  Widget		submenu;		/* the menu to pull down */
  XtCallbackList	activate_callback;	/* widget fired callback */
  XtCallbackList	cascade_callback;	/* optional callback, called */
						/* when the menu is about */
						/* to be pulled down */
			/* internal fields */
  Boolean		armed;			/* armed flag */
  XRectangle		cascade_rect;		/* location of cascade*/
  XtIntervalId		timer;			/* timeout id */
  XmCascadeButtonGCacheObjPart         *cache;
} XmCascadeButtonGadgetPart;


/* Full instance record declaration */

typedef struct _XmCascadeButtonGadgetRec
{
  ObjectPart		     object;
  RectObjPart                rectangle;
  XmGadgetPart               gadget;
  XmLabelGadgetPart          label;
  XmCascadeButtonGadgetPart  cascade_button;
} XmCascadeButtonGadgetRec;


/* CascadeButton class structure */

typedef struct 
{
  XtPointer	extension;	/* Pointer to extension record */
} XmCascadeButtonGadgetClassPart;


/* Full class record declaration for CascadeButton class */

typedef struct _XmCascadeButtonGadgetClassRec 
{
  RectObjClassPart               rect_class;
  XmGadgetClassPart              gadget_class;
  XmLabelGadgetClassPart         label_class;
  XmCascadeButtonGadgetClassPart cascade_button_class;
} XmCascadeButtonGadgetClassRec;


externalref XmCascadeButtonGadgetClassRec   xmCascadeButtonGadgetClassRec;


/* Access macro definitions  for UNcached fields*/

#define CBG_Submenu(cb)		(((XmCascadeButtonGadget) 		    \
                                  cb)->cascade_button.submenu)
#define CBG_ActivateCall(cb)	(((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.activate_callback)
#define CBG_CascadeCall(cb)	(((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.cascade_callback)
#define CBG_Armed(cb)		(((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.armed)
#define CBG_CascadeRect(cb)	(((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.cascade_rect)
#define CBG_Timer(cb)           (((XmCascadeButtonGadget)                    \
				  cb)->cascade_button.timer)
#define CBG_Cascade_x(cb)	(((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.cascade_rect.x)
#define CBG_Cascade_y(cb)	(((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.cascade_rect.y)
#define CBG_Cascade_width(cb)	(((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.cascade_rect.width)
#define CBG_Cascade_height(cb)	(((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.cascade_rect.height)
#define CBG_HasCascade(cb)      (((LabG_MenuType(cb) == XmMENU_PULLDOWN)  || \
			          (LabG_MenuType(cb) == XmMENU_POPUP) ||     \
                                  (LabG_MenuType(cb) == XmMENU_OPTION)) &&   \
			         (CBG_Submenu(cb)))

#define XmCBG_ARMED_BIT	       (1 << 0)
#define XmCBG_TRAVERSE_BIT     (1 << 1)
#define XmCBG_WAS_POSTED_BIT   (1 << 2)

#define CBG_IsArmed(cb)	 (((XmCascadeButtonGadget)(cb))->cascade_button.armed \
			  & XmCBG_ARMED_BIT)

#define CBG_Traversing(cb) (((XmCascadeButtonGadget)                          \
			    (cb))->cascade_button.armed & XmCBG_TRAVERSE_BIT)

#define CBG_WasPosted(cb) (((XmCascadeButtonGadget)                          \
			    (cb))->cascade_button.armed & XmCBG_WAS_POSTED_BIT)

#define CBG_SetBit(byte,bit,v)  byte = (byte & (~bit)) | (v ? bit : 0)

#define CBG_SetArmed(cb,v)  CBG_SetBit (((XmCascadeButtonGadget)	     \
				       (cb))->cascade_button.armed,          \
				      XmCBG_ARMED_BIT, v)

#define CBG_SetTraverse(cb,v)  CBG_SetBit (((XmCascadeButtonGadget)	     \
				       (cb))->cascade_button.armed,          \
				      XmCBG_TRAVERSE_BIT, v)

#define CBG_SetWasPosted(cb,v)  CBG_SetBit (((XmCascadeButtonGadget)	     \
					     (cb))->cascade_button.armed,          \
					    XmCBG_WAS_POSTED_BIT, v)


				  
/* Access macro definitions  for Cached fields*/

#define CBG_CascadePixmap(cb)   (((XmCascadeButtonGadget)                    \
				  cb)->cascade_button.cache->cascade_pixmap)
#define CBG_MapDelay(cb)        (((XmCascadeButtonGadget)                    \
				  cb)->cascade_button.cache->map_delay)
#define CBG_ArmedPixmap(cb)	(((XmCascadeButtonGadget)                    \
				  cb)->cascade_button.cache->armed_pixmap)
#define CBG_ArmGC(cb)            (((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.cache->arm_gc)
#define CBG_BackgroundGC(cb)     (((XmCascadeButtonGadget)                    \
                                  cb)->cascade_button.cache->background_gc)


/******************************/
/* Convenience Macros         */
/******************************/


#define CBG_Cache(w)                    (((XmCascadeButtonGadget)(w))->\
					   cascade_button.cache)
#define CBG_ClassCachePart(w) \
	(((XmCascadeButtonGadgetClass)xmCascadeButtonGadgetClass)->gadget_class.cache_part)


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* _XmCascadeBGP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

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
/*   $XConsortium: SeparatoGP.h /main/11 1995/07/13 17:59:16 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmSeparatorGadgetP_h
#define _XmSeparatorGadgetP_h

#include <Xm/SeparatoG.h>
#include <Xm/GadgetP.h>
#include <Xm/ExtObjectP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************/
/* The Separator Gadget Cache Object's class and instance records*/
/*****************************************************************/

typedef struct _XmSeparatorGCacheObjClassPart
{
    int foo;
} XmSeparatorGCacheObjClassPart;


/* separator cache class record */
typedef struct _XmSeparatorGCacheObjClassRec
{
    ObjectClassPart                     object_class;
    XmExtClassPart                      ext_class;
    XmSeparatorGCacheObjClassPart       separator_class_cache;
} XmSeparatorGCacheObjClassRec;

externalref XmSeparatorGCacheObjClassRec xmSeparatorGCacheObjClassRec;


/*  The Separator Gadget Cache instance record  */

typedef struct _XmSeparatorGCacheObjPart
{
   Dimension	  margin;
   unsigned char  orientation;
   unsigned char  separator_type;
   GC             separator_GC;
   
   GC               background_GC;
   GC               top_shadow_GC;
   GC               bottom_shadow_GC;
   
   Pixel            foreground;
   Pixel            background;
   
   Pixel            top_shadow_color;
   Pixmap           top_shadow_pixmap;
   
   Pixel            bottom_shadow_color;
   Pixmap           bottom_shadow_pixmap;
} XmSeparatorGCacheObjPart;

typedef struct _XmSeparatorGCacheObjRec
{
  ObjectPart                object;
  XmExtPart		    ext;
  XmSeparatorGCacheObjPart  separator_cache;
} XmSeparatorGCacheObjRec;


/*****************************************************/
/*  The Separator Widget Class and instance records  */
/*****************************************************/

typedef struct _XmSeparatorGadgetClassPart
{
   XtPointer               extension;
} XmSeparatorGadgetClassPart;


/*  Full class record declaration for Separator class  */

typedef struct _XmSeparatorGadgetClassRec
{
   RectObjClassPart            rect_class;
   XmGadgetClassPart           gadget_class;
   XmSeparatorGadgetClassPart  separator_class;
} XmSeparatorGadgetClassRec;

externalref XmSeparatorGadgetClassRec xmSeparatorGadgetClassRec;

typedef struct _XmSeparatorGadgetPart
{
  XmSeparatorGCacheObjPart  *cache;
  Boolean fill_bg_box;
} XmSeparatorGadgetPart;

/*  Full instance record declaration  */

typedef struct _XmSeparatorGadgetRec
{
   ObjectPart             object;
   RectObjPart            rectangle;
   XmGadgetPart           gadget;
   XmSeparatorGadgetPart  separator;
} XmSeparatorGadgetRec;


/* MACROS for accessing instance fields*/
#define SEPG_Margin(w)		\
	(((XmSeparatorGadget)(w))->separator.cache->margin)
#define SEPG_Orientation(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->orientation)
#define SEPG_SeparatorType(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->separator_type)
#define SEPG_SeparatorGC(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->separator_GC)
#define SEPG_BackgroundGC(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->background_GC)
#define SEPG_TopShadowGC(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->top_shadow_GC)
#define SEPG_BottomShadowGC(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->bottom_shadow_GC)
#define SEPG_Foreground(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->foreground)
#define SEPG_Background(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->background)
#define SEPG_TopShadowColor(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->top_shadow_color)
#define SEPG_TopShadowPixmap(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->top_shadow_pixmap)
#define SEPG_BottomShadowColor(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->bottom_shadow_color)
#define SEPG_BottomShadowPixmap(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->bottom_shadow_pixmap)

/* Convenience Macros */
#define SEPG_Cache(w)		\
	(((XmSeparatorGadget)(w))->separator.cache)
#define SEPG_ClassCachePart(w) \
        (((XmSeparatorGadgetClass)xmSeparatorGadgetClass)->gadget_class.cache_part)


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSeparatorGadgetP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

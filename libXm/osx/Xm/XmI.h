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
/*   $TOG: XmI.h /main/19 1997/06/18 17:47:55 samborn $ */
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmI_h
#define _XmI_h

#ifndef _XmNO_BC_INCL
#define _XmNO_BC_INCL
#endif

#include <stdio.h>
#include <limits.h>
#include <Xm/XmP.h>
#include "XmStrDefsI.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEBUG
# define assert(assert_exp)
#elif (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
# define assert(assert_exp)						\
  (((assert_exp) ? (void) 0 :						\
    (void) (fprintf(stderr, "assert(%s) failed at line %d in %s\n",	\
                    #assert_exp, __LINE__, __FILE__), abort())))
#else
# define assert(assert_exp)						\
  (((assert_exp) ? 0 :							\
    (void) (fprintf(stderr, "assert(%s) failed at line %d in %s\n",	\
		    "assert_exp", __LINE__, __FILE__), abort())))
#endif


#define ASSIGN_MAX(a, b) 	((a) = ((a) > (b) ? (a) : (b)))
#define ASSIGN_MIN(a, b) 	((a) = ((a) < (b) ? (a) : (b)))

#ifndef MAX
#define MAX(x,y)	((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y)	((x) > (y) ? (y) : (x))
#endif

#ifndef ABS
#define ABS(x)		(((x) >= 0) ? (x) : -(x))
#endif

#define GMode(g)	    ((g)->request_mode)
#define IsX(g)		    (GMode (g) & CWX)
#define IsY(g)		    (GMode (g) & CWY)
#define IsWidth(g)	    (GMode (g) & CWWidth)
#define IsHeight(g)	    (GMode (g) & CWHeight)
#define IsBorder(g)	    (GMode (g) & CWBorderWidth)
#define IsWidthHeight(g)    (GMode (g) & (CWWidth | CWHeight))
#define IsQueryOnly(g)      (GMode (g) & XtCWQueryOnly)

#define XmStrlen(s)      ((s) ? strlen(s) : 0)


#define XmStackAlloc(size, stack_cache_array)	\
    ((((char*)(stack_cache_array) != NULL) &&	\
     ((size) <= sizeof(stack_cache_array)))	\
     ?  (char *)(stack_cache_array)		\
     :  XtMalloc((unsigned)(size)))

#define XmStackFree(pointer, stack_cache_array) \
    if ((pointer) != ((char*)(stack_cache_array))) XtFree(pointer);


/******** _XmCreateImage ********/

#ifdef NO_XM_1_2_BC

/* The _XmCreateImage macro is used to create XImage with client
   specific data for the bit and byte order.
   We still have to do the following because XCreateImage
   will stuff here display specific data and we want 
   client specific values (i.e the bit orders we used for 
   creating the bitmap data in Motif) -- BUG 4262 */
/* Used in Motif 1.2 in DragIcon.c, MessageB.c, ReadImage.c and
   ImageCache.c */

#define _XmCreateImage(IMAGE, DISPLAY, DATA, WIDTH, HEIGHT, BYTE_ORDER) {\
    IMAGE = XCreateImage(DISPLAY,\
			 DefaultVisual(DISPLAY, DefaultScreen(DISPLAY)),\
			 1,\
			 XYBitmap,\
			 0,\
			 DATA,\
			 WIDTH, HEIGHT,\
			 8,\
			 (WIDTH+7) >> 3);\
    IMAGE->byte_order = BYTE_ORDER;\
    IMAGE->bitmap_unit = 8;\
    IMAGE->bitmap_bit_order = LSBFirst;\
}

#endif /* NO_XM_1_2_BC */


/****************************************************************
 *
 *  Macros for Right-to-left Layout
 *
 ****************************************************************/

#define GetLayout(w)     (_XmGetLayoutDirection((Widget)(w)))
#define LayoutM(w)       (XmIsManager(w) ? \
			  ((XmManagerWidget)w)->manager.string_direction : \
			  GetLayout(w))
#define LayoutP(w)       (XmIsPrimitive(w) ? \
			  XmPrim_layout_direction(((XmPrimitiveWidget)w)) :\
			  GetLayout(w))
#define LayoutG(w)       (XmIsGadget(w) ? \
			  ((XmGadget)w)->gadget.layout_direction : \
			  GetLayout(w))

#define LayoutIsRtoL(w)      \
  (XmDirectionMatchPartial(GetLayout(w), XmRIGHT_TO_LEFT, XmHORIZONTAL_MASK))
#define LayoutIsRtoLM(w)     \
  (XmDirectionMatchPartial(LayoutM(w), XmRIGHT_TO_LEFT, XmHORIZONTAL_MASK))
#define LayoutIsRtoLP(w)     \
  (XmDirectionMatchPartial(LayoutP(w), XmRIGHT_TO_LEFT, XmHORIZONTAL_MASK))
#define LayoutIsRtoLG(w)     \
  (XmDirectionMatchPartial(LayoutG(w), XmRIGHT_TO_LEFT, XmHORIZONTAL_MASK))


/********    Private Function Declarations for Direction.c    ********/

extern void _XmDirectionDefault(Widget widget,
  			        int offset,
  			        XrmValue *value );
extern void _XmFromLayoutDirection( 
                        Widget widget,
                        int offset,
                        XtArgVal *value) ;

extern XmImportOperator _XmToLayoutDirection( 
                        Widget widget,
                        int offset,
                        XtArgVal *value) ;
extern XmDirection _XmGetLayoutDirection(Widget w);


/********    Private Function Declarations for thickness  ********/
extern void _XmSetThickness( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern void _XmSetThicknessDefault0( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;

/********    Private Function Declarations for Xm.c    ********/

extern void _XmReOrderResourceList( 
			WidgetClass widget_class,
			String res_name,
                        String insert_after) ;
extern void _XmSocorro( 
                        Widget w,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern Boolean _XmParentProcess( 
                        Widget widget,
                        XmParentProcessData data) ;
extern void _XmClearShadowType( 
                        Widget w,
#if NeedWidePrototypes
                        int old_width,
                        int old_height,
                        int old_shadow_thickness,
                        int old_highlight_thickness) ;
#else
                        Dimension old_width,
                        Dimension old_height,
                        Dimension old_shadow_thickness,
                        Dimension old_highlight_thickness) ;
#endif /* NeedWidePrototypes */
#ifdef NO_XM_1_2_BC
extern void _XmDestroyParentCallback( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
#endif
extern Time _XmValidTimestamp(
			Widget w);
extern void _XmWarningMsg(Widget w,
                          char *type,
			  char *message,
			  char **params,
			  Cardinal num_params);
extern Display *_XmGetDefaultDisplay(void);
extern Boolean _XmIsISO10646(Display *dpy,
                             XFontStruct *font);
extern XChar2b* _XmUtf8ToUcs2(char *draw_text,
                              size_t seg_len,
			      size_t *ret_str_len);


/********    End Private Function Declarations    ********/

/********    Conditionally defined macros for thread_safe Motif ******/
#if defined(XTHREADS) && defined(XUSE_MTSAFE_API)

# define _XmWidgetToAppContext(w) \
        XtAppContext app = XtWidgetToApplicationContext(w)

# define _XmDisplayToAppContext(d) \
        XtAppContext app = XtDisplayToApplicationContext(d)

# define _XmAppLock(app)	XtAppLock(app)
# define _XmAppUnlock(app)	XtAppUnlock(app)
# define _XmProcessLock()	XtProcessLock()
# define _XmProcessUnlock()	XtProcessUnlock()

/* Remove use of _XtProcessLock when Xt provides API to query its MT-status */
extern void (*_XtProcessLock)();
# define _XmIsThreadInitialized() (_XtProcessLock)

#else

# define _XmWidgetToAppContext(w)
# define _XmDisplayToAppContext(d)
# define _XmAppLock(app)
# define _XmAppUnlock(app)
# define _XmProcessLock()
# define _XmProcessUnlock()
# define _XmIsThreadInitialized()	(FALSE)

#endif /* XTHREADS && XUSE_MTSAFE_API */


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#define FIX_1381
#define FIX_1396

#ifdef FIX_1381
#define RGB_GREY_VALUE 128
#define RGB_GREY_PRESISE 50
extern Pixel _XmAssignInsensitiveColor(Widget w);
#endif

#define FIX_1375
#define FIX_1395 1

#define FIX_1388
#define FIX_1398
#define FIX_1402
#define FIX_1445

#endif /* _XmI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

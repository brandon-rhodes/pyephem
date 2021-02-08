/**
 *
 * $Id: SlideC.h,v 1.2 2002/04/01 15:14:20 jimk Exp $
 *
 **/

#ifndef _SLIDEC_H
#define _SLIDEC_H

#include <X11/Intrinsic.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsSlideContext
#define XmIsSlideContext(w) XtIsSubclass(w,xmSlideContextClass)
#endif
#ifndef XmNslideFinishCallback
#define XmNslideFinishCallback "slideFinishCallback"
#endif
#ifndef XmCSlideFinishCallback
#define XmCSlideFinishCallback "SlideFinishCallback"
#endif
#ifndef XmNslideMotionCallback
#define XmNslideMotionCallback "slideMotionCallback"
#endif
#ifndef XmCSlideMotionCallback
#define XmCSlideMotionCallback "SlideMotionCallback"
#endif
#ifndef XmNslideWidget
#define XmNslideWidget "slideWidget"
#endif
#ifndef XmCSlideWidget
#define XmCSlideWidget "SlideWidget"
#endif
#ifndef XmNslideInterval
#define XmNslideInterval "slideInterval"
#endif
#ifndef XmCSlideInterval
#define XmCSlideInterval "SlideInterval"
#endif
#ifndef XmNslideDestWidth
#define XmNslideDestWidth "slideDestWidth"
#endif
#ifndef XmCSlideDestWidth
#define XmCSlideDestWidth "SlideDestWidth"
#endif
#ifndef XmNslideDestHeight
#define XmNslideDestHeight "slideDestHeight"
#endif
#ifndef XmCSlideDestHeight
#define XmCSlideDestHeight "SlideDestHeight"
#endif
#ifndef XmNslideDestX
#define XmNslideDestX "slideDestX"
#endif
#ifndef XmCSlideDestX
#define XmCSlideDestX "SlideDestX"
#endif
#ifndef XmNslideDestY
#define XmNslideDestY "slideDestY"
#endif
#ifndef XmCSlideDestY
#define XmCSlideDestY "SlideDestY"
#endif

extern WidgetClass xmSlideContextWidgetClass;

typedef struct _XmSlideContextRec *XmSlideContextWidget;
typedef struct _XmSlideContextClassRec *XmSlideContextWidgetClass;

typedef struct _XmSlideStruct {
	Widget w;
	XtWidgetGeometry dest;
	unsigned long interval;
	XtIntervalId id;
} XmSlideStruct, *XmSlidePtr;

void XmSlide(XmSlidePtr slide_info);

#ifdef __cplusplus
}
#endif
#endif

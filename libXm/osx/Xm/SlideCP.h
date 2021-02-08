/**
 *
 * $Id: SlideCP.h,v 1.3 2002/04/01 15:14:21 jimk Exp $
 *
 **/

#ifndef _SLIDECP_H
#define _SLIDECP_H

#include <Xm/SlideC.h>
#include <X11/IntrinsicP.h>
#include <X11/ObjectP.h>
#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmUNSPECIFIED_POSITION
#define XmUNSPECIFIED_POSITION (-1)
#endif

typedef struct {
    XtPointer extension;
} XmSlideContextClassPart;

typedef struct _XmSlideContextClassRec {
	ObjectClassPart object_class;
	XmSlideContextClassPart slide_class;
} XmSlideContextClassRec;

extern XmSlideContextClassRec xmSlideContextClassRec;

typedef struct _XmSlideContextPart {
	XtIntervalId id;
	XtCallbackList slideFinishCallback;
	XtCallbackList slideMotionCallback;
	Widget slide_widget;
	unsigned long interval;
	Dimension dest_width;
	Dimension dest_height;
	Position dest_x;
	Position dest_y;
} XmSlideContextPart;

typedef struct _XmSlideContextRec {
	ObjectPart object;
	XmSlideContextPart slide;
} XmSlideContextRec;

#define Slide_Id(w) (((XmSlideContextWidget)w)->slide.id)
#define Slide_Widget(w) (((XmSlideContextWidget)w)->slide.slide_widget)
#define Slide_Interval(w) (((XmSlideContextWidget)w)->slide.interval)
#define Slide_DestWidth(w) (((XmSlideContextWidget)w)->slide.dest_width)
#define Slide_DestHeight(w) (((XmSlideContextWidget)w)->slide.dest_height)
#define Slide_DestX(w) (((XmSlideContextWidget)w)->slide.dest_x)
#define Slide_DestY(w) (((XmSlideContextWidget)w)->slide.dest_y)
#define Slide_FinishCallback(w) (((XmSlideContextWidget)w)->slide.slideFinishCallback)
#define Slide_MotionCallback(w) (((XmSlideContextWidget)w)->slide.slideMotionCallback)

#ifdef __cplusplus
}
#endif

#endif

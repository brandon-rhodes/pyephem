#ifndef __TabStack_h__
#define __TabStack_h__

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Ext.h>
#include <Xm/TabBox.h>
#include <Xm/TabList.h>
#include <Xm/DrawUtils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {XmTABS_ON_TOP, XmTABS_ON_BOTTOM, XmTABS_ON_RIGHT,
	      XmTABS_ON_LEFT} XmTabSide;

typedef struct _XmTabStackCallbackStruct {
    int 	reason;
    XEvent      *event;
    Widget      selected_child;
} XmTabStackCallbackStruct;

typedef struct _XmTabStackRec      *XmTabStackWidget;
typedef struct _XmTabStackClassRec *XmTabStackWidgetClass;
extern WidgetClass                 xmTabStackWidgetClass;

#ifndef XmIsTabStack
#define XmIsTabStack(w) XtIsSubclass(w, xmTabStackWidgetClass)
#endif /* XmIsTabStack */

#ifdef _NO_PROTO
extern Widget XmCreateTabStack();
extern Widget XmTabStackGetSelectedTab();
extern void   XmTabStackSelectTab();
extern Widget XmTabStackIndexToWidget();
extern Widget XmTabStackXYToWidget();
#else
extern Widget XmCreateTabStack(Widget, String, ArgList, Cardinal);
extern Widget XmTabStackGetSelectedTab(Widget);
extern void   XmTabStackSelectTab(Widget, Boolean);
extern Widget XmTabStackIndexToWidget(Widget, int);
extern Widget XmTabStackXYToWidget(Widget, int, int);
#endif

#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* __TabStack_h__ */

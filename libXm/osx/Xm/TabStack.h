#ifndef __XmTabStack_h__
#define __XmTabStack_h__

#include <Xm/Ext.h>
#include <Xm/TabBox.h>
#include <Xm/TabList.h>
#include <Xm/DrawUtils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmTabStackRec      *XmTabStackWidget;
typedef struct _XmTabStackClassRec *XmTabStackWidgetClass;
extern WidgetClass                 xmTabStackWidgetClass;

#ifndef XmIsTabStack
#define XmIsTabStack(w) XtIsSubclass(w, xmTabStackWidgetClass)
#endif /* XmIsTabStack */

extern Widget XmCreateTabStack(Widget, String, ArgList, Cardinal);
extern Widget XmTabStackGetSelectedTab(Widget);
extern void   XmTabStackSelectTab(Widget, Boolean);
extern Widget XmTabStackIndexToWidget(Widget, int);
extern Widget XmTabStackXYToWidget(Widget, int, int);

/*
 * Variable argument list functions
 */

extern Widget XmVaCreateTabStack(
                        Widget parent,
                        char *name,
                        ...);
extern Widget XmVaCreateManagedTabStack(
                        Widget parent,
                        char *name,
                        ...);

#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration */
#endif


#endif /* __TabStack_h__ */

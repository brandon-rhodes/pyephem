#ifndef __Xmcolumn_h__
#define __Xmcolumn_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/Xm.h>
#include <Xm/Ext.h>

extern WidgetClass xmColumnWidgetClass;

typedef struct _XmColumnClassRec * XmColumnWidgetClass;
typedef struct _XmColumnRec      * XmColumnWidget;

#ifndef XmIsColumn
#define XmIsColumn(w) (XtIsSubclass(w, xmColumnWidgetClass))
#endif

extern Widget XmCreateColumn(Widget, String, ArgList, Cardinal);

/*
 * Variable argument list functions
 */

extern Widget XmVaCreateColumn(
                        Widget parent,
                        char *name,
                        ...);
extern Widget XmVaCreateManagedColumn(
                        Widget parent,
                        char *name,
                        ...);
#ifdef __cplusplus
}
#endif

#endif /* __column_h__ */

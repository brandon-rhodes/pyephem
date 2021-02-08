/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1995 Integrated Computer Solutions
 */


#ifndef __xm_table_h__
#define __xm_table_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/Ext.h>

#ifndef XmIsTable
#define XmIsTable(w) XtIsSubclass((w), xmTableWidgetClass)
#endif

extern WidgetClass xmTableWidgetClass;

typedef struct _XmTableClassRec *XmTableWidgetClass;
typedef struct _XmTableRec      *XmTableWidget;

#if NeedFunctionPrototypes
typedef String (*XmStringFunc)(Widget);
typedef void   (*XmFreeProc)(XtPointer);
typedef void   (*XmRenderProc)(Widget, Widget, XRectangle*, GC, String);
typedef String (*XmFetchCellValueFunc)(Widget, Widget, int);
typedef void   (*XmStoreCellValueProc)(Widget, Widget, int, String);
typedef void   (*XmFreeCellValueProc)(Widget, String);

extern Widget XmCreateTable(Widget, String, ArgList, Cardinal);
extern void XmTableGotoCell(Widget, Widget, int);
extern void XmTableUpdate(Widget);
#else
typedef String (*XmStringFunc)();
typedef void   (*XmFreeProc)();
typedef void   (*XmRenderProc)();
typedef String (*XmFetchCellValueFunc)();
typedef void   (*XmStoreCellValueProc)();
typedef void   (*XmFreeCellValueProc)();

extern Widget XmCreateTable();
extern void XmTableGotoCell();
extern void XmTableUpdate();
#endif

#define XmCR_CELL_VERIFY_TRAVERSE 0
#define XmCR_CELL_TRAVERSE        1
typedef struct _XmTableCallbackStruct {
    int     reason;
    XEvent  *event;
    Widget  from_cell;
    int     from_row;
    Widget  to_cell;
    int     to_row;
    Boolean confirm;
} XmTableCallbackStruct;

#ifdef __cplusplus
}
#endif

#endif /* __xm_table_h__ */


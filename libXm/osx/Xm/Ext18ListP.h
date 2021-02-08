#ifndef _XmExt18ListP_h_
#define _XmExt18ListP_h_

#include "XmP.h"

#undef XmIsExt18List
#define XmIsExt18List(w)  \
  (_XmIsFastSubclass(XtClass(w), XmEXT18LIST_BIT))


#include <Xm/ManagerP.h>
#include <Xm/PrimitiveP.h>

#include <Xm/MultiListP.h>
#include <Xm/XmP.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define XmExt18List_DEFAULT_VISIBLE_COUNT	XmMULTILIST_DEFAULT_VISIBLE_COUNT

#define XmExt18ListIndex (XmManagerIndex + 1)

#define XmExt18List_title(w) (((XmExt18ListWidget)(w))->ext_list.title)
#define XmExt18List_find_label(w) (((XmExt18ListWidget)(w))->ext_list.find_label)
#define XmExt18List_double_click(w) (((XmExt18ListWidget)(w))->ext_list.double_click)
#define XmExt18List_single_select(w) (((XmExt18ListWidget)(w))->ext_list.single_select)
#define XmExt18List_show_find(w) (((XmExt18ListWidget)(w))->ext_list.show_find)
#define XmExt18List_title_wid(w) (((XmExt18ListWidget)(w))->ext_list.title_wid)
#define XmExt18List_frame(w) (((XmExt18ListWidget)(w))->ext_list.frame)
#define XmExt18List_ilist(w) (((XmExt18ListWidget)(w))->ext_list.ilist)
#define XmExt18List_v_bar(w) (((XmExt18ListWidget)(w))->ext_list.v_bar)
#define XmExt18List_h_bar(w) (((XmExt18ListWidget)(w))->ext_list.h_bar)
#define XmExt18List_find(w) (((XmExt18ListWidget)(w))->ext_list.find)
#define XmExt18List_find_text(w) (((XmExt18ListWidget)(w))->ext_list.find_text)
#define XmExt18List_last_search(w) (((XmExt18ListWidget)(w))->ext_list.last_search)
#define XmExt18List_item_found(w) (((XmExt18ListWidget)(w))->ext_list.item_found)
#define XmExt18List_not_found(w) (((XmExt18ListWidget)(w))->ext_list.not_found)
#define XmExt18List_visible_rows(w) (((XmExt18ListWidget)(w))->ext_list.visible_rows)
#define XmExt18List_title_string(w) (((XmExt18ListWidget)(w))->ext_list.title_string)

typedef XmMultiListClassPart XmExt18ListClassPart;

typedef XmMultiListClassRec XmExt18ListClassRec;

typedef XmMultiListPart XmExt18ListPart;

typedef XmMultiListRec XmExt18ListRec;

extern XmExt18ListClassRec xmExt18ListClassRec XM_DEPRECATED;

extern XmI18ListClassRec xiI18ListClassRec;
extern WidgetClass xmI18ListWidgetClass;

#if defined(__cplusplus)
}
#endif

#endif /* _XmExt18ListP_h_ */

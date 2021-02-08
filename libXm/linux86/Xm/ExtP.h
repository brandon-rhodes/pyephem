/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmExtP_h_
#define _XmExtP_h_

#include <Xm/Ext.h>

/************************************************************
*	INCLUDE FILES
*************************************************************/

#if defined(hpux) && OS_MAJOR_VERSION < 10
#include <nl_ctype.h>
#endif

/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*XmVoidFunc)(void);
typedef int (*XmIntFunc)(void);
typedef unsigned int (*XmUnsignedIntFunc)(void);

extern String xm_std_filter[], xm_std_constraint_filter[];

/************************************************************
*	MACROS
*************************************************************/

#define streq(a, b) (((a) != NULL) && ((b) != NULL) && (strcmp((a), (b)) == 0))

#define ForAllChildren(w, childP)                                          \
  for ( (childP) = (w)->composite.children ;                               \
        (childP) < (w)->composite.children + (w)->composite.num_children ; \
        (childP)++ )

/*
 * Math Stuff 
 *
 * Some Systems define MIN and MAX so I have to undef them before I make
 * my own definitions.
 */

#undef MIN			
#undef MAX			
#undef ABS

#define MIN(a,b)        (((int)(a) < (int)(b)) ? (a) : (b))
#define MAX(a,b)        (((int)(a) > (int)(b)) ? (a) : (b))
#define ABS(a)          (((int)(a) >= 0) ? (a) : -(a))

#define ASSIGN_MAX(a, b) ((a) = ((int)(a) > (int)(b) ? (a) : (b)))
#define ASSIGN_MIN(a, b) ((a) = ((int)(a) < (int)(b) ? (a) : (b)))

#define XM_ICON_BUTTON_CLASS_NAME ("XmIconButton")
#define XM_EXT_LIST_CLASS_NAME ("XmExtendedList")
#define XM_ILIST_CLASS_NAME ("XmIList")
#define XM_EXT_18_LIST_CLASS_NAME ("XmExtended18List")
#define XM_I18LIST_CLASS_NAME ("XmI18List")


/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

   

void XmResolveAllPartOffsets64(WidgetClass, XmOffsetPtr*, XmOffsetPtr*);
void _XmMoveWidget(Widget, Position, Position);
void _XmResizeWidget(Widget, Dimension, Dimension, Dimension);
void _XmConfigureWidget(Widget, Position, Position, 
                        Dimension, Dimension, Dimension);

XtGeometryResult _XmRequestNewSize(Widget, Boolean, Dimension,
                                   Dimension,
                                   Dimension *, Dimension *);

XtGeometryResult _XmHWQuery(Widget, XtWidgetGeometry*, XtWidgetGeometry *);

void _XmGetFocus(Widget, XEvent *, String *, Cardinal *);

void _XmFilterArgs(ArgList, Cardinal, String *,
                   ArgList *, Cardinal *);

void _XmSetValuesOnChildren(Widget, ArgList, Cardinal);

Boolean _XmGadgetWarning(Widget);

String _XmGetMBStringFromXmString(XmString);

/*
 * Context Managment Routines.
 */
    
void _XmSetContextData(Widget, XContext, XtPointer);
void _XmDeleteContextData(Widget, XContext);
Boolean _XmGetContextData(Widget, XContext, XtPointer *);
Boolean _XmUtilIsSubclassByNameQ(Widget, XrmQuark);
void _XmInitialIzeConverters(Widget);

void _XmExtHighlightBorder(Widget);
void _XmExtUnhighlightBorder(Widget);


/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/

#if defined(__cplusplus)
}
#endif

#endif 

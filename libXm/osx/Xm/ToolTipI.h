/**
 *
 * $Id: ToolTipI.h,v 1.6.2.1 2008/02/07 11:49:31 yura Exp $
 *
 **/
#ifndef _XmToolTip_h
#define _XmToolTip_h

#include <X11/Intrinsic.h>
#include "Xm.h"
#include "XmI.h"

#ifdef __cplusplus
extern "C" {
#endif
    
void _XmToolTipEnter(Widget wid, 
                     XEvent *event, 
                     String *params, 
                     Cardinal *num_params);

void _XmToolTipLeave(Widget wid, 
                     XEvent *event, 
                     String *params, 
                     Cardinal *num_params);

#ifdef FIX_1388                     
void _XmToolTipRemove(Widget wid);
#endif

XmString XmGetToolTipString (Widget w);

void XmSetToolTipString (Widget w,
                         XmString s);
    
#ifdef __cplusplus
}
#endif

#endif

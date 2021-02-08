/**
 *
 * $Id: ToolTipT.h,v 1.1 2004/11/18 08:54:33 yura Exp $
 *
 **/

#ifndef _XmToolTipT_H
#define _XmToolTipT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/Xm.h>

externalref XrmQuark XmQTtoolTip;

/* Trait structures and typedefs, place typedefs first */

typedef struct _XmToolTipTraitRec
{
    int version;
    XmString tool_tip_string;
} XmToolTipTraitRec, *XmToolTipTrait;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmToolTipT_H */

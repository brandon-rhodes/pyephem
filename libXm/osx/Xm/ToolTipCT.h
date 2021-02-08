/**
 *
 * $Id: ToolTipCT.h,v 1.1 2004/11/18 08:54:33 yura Exp $
 *
 **/

#ifndef _XmToolTipCT_H
#define _XmToolTipCT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/Xm.h>

externalref XrmQuark XmQTtoolTipConfig;

/* Trait structures and typedefs, place typedefs first */

typedef struct _XmToolTipConfigTraitRec
{
    int version;
    Widget label;             /* XmLabel for the tips */
    int post_delay;           /* delay before posting XmNtoolTipPostDelay */
    int post_duration;        /* duration XmNtoolTipPostDuration */
    XtIntervalId timer;       /* timer for post delay */
    XtIntervalId duration_timer;  /* timer for duration */
    Time leave_time;          /* time of the last leave event */
    Widget slider;            /* the XmSlideContext used to slide in the tip */
    Boolean enable;           /* flag to disable all this stuff */
} XmToolTipConfigTraitRec, *XmToolTipConfigTrait;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmToolTipCT_H */

/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */
#ifndef __XmDraw_h__
#define __XmDraw_h__

#include <Xm/Xm.h>
#include <Xm/Ext.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {XmBEVEL_BOTTOM, XmBEVEL_TOP, XmBEVEL_BOTH} XmBevelOption;

void XmDrawBevel(
#ifndef _NO_PROTO
Display*, Drawable, GC, GC, int, int, unsigned int, XmBevelOption
#endif
);

#if defined(__cplusplus)
}
#endif

#endif /* __XmDraw_h__ */

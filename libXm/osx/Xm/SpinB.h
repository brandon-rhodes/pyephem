/* $XConsortium: SpinB.h /main/6 1995/07/15 20:55:46 drk $ */
/*
 * (c) Copyright 1995 Digital Equipment Corporation.
 * (c) Copyright 1995 Hewlett-Packard Company.
 * (c) Copyright 1995 International Business Machines Corp.
 * (c) Copyright 1995 Sun Microsystems, Inc.
 * (c) Copyright 1995 Novell, Inc. 
 * (c) Copyright 1995 FUJITSU LIMITED.
 * (c) Copyright 1995 Hitachi.
 */
/*
 * HISTORY
 */

#ifndef _XmSpinB_h
#define _XmSpinB_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SpinB Widget
 */
externalref WidgetClass xmSpinBoxWidgetClass;

typedef struct _XmSpinBoxClassRec *XmSpinBoxWidgetClass;
typedef struct _XmSpinBoxRec      *XmSpinBoxWidget;


/*
 * Spin externs for application accessible functions
 */

Widget	XmCreateSpinBox(Widget	parent,
				char	*name,
				ArgList	arglist,
				Cardinal argcount);
Widget XmVaCreateSpinBox(
                                Widget parent,
                                char *name,
                                ...);
Widget XmVaCreateManagedSpinBox(
                                Widget parent,
                                char *name,
                                ...);
int	XmSpinBoxValidatePosition(
				Widget	text_field,
				int	*position_value);



#ifdef __cplusplus
}
#endif

#endif /* _SpinB_h */

/* $XConsortium: SSpinB.h /main/4 1995/07/15 20:54:58 drk $ */
/*
 * (c) Copyright 1995 Digital Equipment Corporation.
 * (c) Copyright 1995 Hewlett-Packard Company.
 * (c) Copyright 1995 International Business Machines Corp.
 * (c) Copyright 1995 Sun Microsystems, Inc.
 * (c) Copyright 1995 Novell, Inc. 
 * (c) Copyright 1995 FUJITSU LIMITED.
 * (c) Copyright 1995 Hitachi.
 */
/******************************************************************************
 *
 *	File:	SSpinB.h
 *	Date:	June 1, 1995
 *	Author:	Mitchell Greess
 *
 *	Contents:
 *		Public header file for the XmSimpleSpinBox widget.
 *		Implements the XmSimpleSpinBox.
 *
 ******************************************************************************/

#ifndef _XmSSpinB_h
#define _XmSSpinB_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* XmSimpleSpinBox Widget */
externalref WidgetClass xmSimpleSpinBoxWidgetClass;

typedef struct _XmSimpleSpinBoxClassRec *XmSimpleSpinBoxWidgetClass;
typedef struct _XmSimpleSpinBoxRec      *XmSimpleSpinBoxWidget;

/* Spin externs for application accessible functions */
extern Widget XmCreateSimpleSpinBox(
		Widget		parent,
		char		*name,
		ArgList		arglist,
		Cardinal	argcount);

extern void XmSimpleSpinBoxAddItem(
                Widget          widget,
                XmString        item,
                int             pos);

extern void XmSimpleSpinBoxDeletePos(
                Widget          widget,
                int             pos);

extern void XmSimpleSpinBoxSetItem(
                Widget          widget,
                XmString        item);

/*
 * Variable argument list functions
 */

extern Widget XmVaCreateSimpleSpinBox(
                        Widget parent,
                        char *name,
                        ...);

extern Widget XmVaCreateManagedSimpleSpinBox(
                        Widget parent,
                        char *name,
                        ...);

#ifdef __cplusplus
}
#endif

#endif /* _SSpinB_h */


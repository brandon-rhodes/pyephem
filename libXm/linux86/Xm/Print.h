/* $XConsortium: Print.h /main/14 1996/10/29 15:50:44 drk $ */
/*
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1996 Hewlett-Packard Company.
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */
#ifndef _XmPrintShell_h
#define _XmPrintShell_h

#include <Xm/Xm.h>
#include <X11/extensions/Print.h>
  
#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmPrintShellWidgetClass;

typedef struct _XmPrintShellClassRec * XmPrintShellWidgetClass;
typedef struct _XmPrintShellRec      * XmPrintShellWidget;


#ifndef XmIsPrintShell
#define XmIsPrintShell(w)  (XtIsSubclass (w, xmPrintShellWidgetClass))
#endif

/********    Public Function Declarations    ********/

extern Widget XmPrintSetup(
             Widget           video_widget,
             Screen           *print_screen,
             String            print_shell_name,
             ArgList           args,
             Cardinal          num_args);

extern void XmRedisplayWidget(Widget widget) ;

extern XtEnum XmPrintToFile(Display *dpy, 
			    char *file_name,
			    XPFinishProc finish_proc, 
			    XPointer client_data) ;

extern XtEnum XmPrintPopupPDM(Widget print_shell,
			      Widget transient_for);

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPrintShell_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

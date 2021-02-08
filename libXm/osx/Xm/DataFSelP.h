/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: DataFSelP.h,v $ $Revision: 1.5 $ $Date: 2003/10/06 10:10:23 $ */
/*
*  (c) Copyright 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDataFSelP_h
#define _XmDataFSelP_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    External (DataF.c) function declaration *******/
#ifdef _NO_PROTO
extern Widget _XmDataFieldGetDropReciever() ;
#else
extern Widget _XmDataFieldGetDropReciever( Widget w ) ;
#endif /* _NO_PROTO */


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern Boolean _XmDataFieldConvert() ;
extern void _XmDataFieldLoseSelection() ;

#else

extern Boolean _XmDataFieldConvert( 
                        Widget w,
                        Atom *selection,
                        Atom *target,
                        Atom *type,
                        XtPointer *value,
                        unsigned long *length,
                        int *format) ;
extern void _XmDataFieldLoseSelection( 
                        Widget w,
                        Atom *selection) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDataFSelP_h */
/* DON't ADD STUFF AFTER THIS #endif */

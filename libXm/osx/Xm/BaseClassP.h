/* 
 *  @OPENGROUP_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 *  
 *  This software is subject to an open license. It may only be
 *  used on, with or for operating systems which are themselves open
 *  source systems. You must contact The Open Group for a license
 *  allowing distribution and sublicensing of this software on, with,
 *  or for operating systems which are not Open Source programs.
 *  
 *  See http://www.opengroup.org/openmotif/license for full
 *  details of the license agreement. Any use, reproduction, or
 *  distribution of the program constitutes recipient's acceptance of
 *  this agreement.
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 *  PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 *  WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 *  OR FITNESS FOR A PARTICULAR PURPOSE
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 *  NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 *  EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGES.
*/ 
/* 
 * HISTORY
*/ 
/* $XConsortium: BaseClassP.h /main/11 1995/10/25 19:53:53 cde-sun $ */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1989, 1990 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
#ifndef _XmBaseClassP_h
#define _XmBaseClassP_h

#ifndef _XmNO_BC_INCL
#define _XmNO_BC_INCL
#endif

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif


#define _XmBCEPTR(wc)	((XmBaseClassExt *)(&(((WidgetClass)(wc))\
					      ->core_class.extension)))
#define _XmBCE(wc)	((XmBaseClassExt)(((WidgetClass)(wc))\
					  ->core_class.extension))

#define _XmGetBaseClassExtPtr(wc, owner) \
    ((_XmBCE(wc) && (((_XmBCE(wc))->record_type) == owner)) ? \
     _XmBCEPTR(wc) :  \
     ((XmBaseClassExt *) _XmGetClassExtensionPtr( \
						 ((XmGenericClassExt *)  \
						  _XmBCEPTR( wc)),  \
						 owner)))

/* defines for 256 bit (at least) bit field
 */
#define _XmGetFlagsBit(field, bit) \
	(field[ (bit >> 3) ]) & (1 << (bit & 0x07))

#define _XmSetFlagsBit(field, bit) \
	    (field[ (bit >> 3) ] |= (1 << (bit & 0x07)))


#ifndef XTHREADS
#define _XmFastSubclassInit(wc, bit_field) { \
	if((_Xm_fastPtr = _XmGetBaseClassExtPtr( wc, XmQmotif)) && \
	   (*_Xm_fastPtr)) \
		_XmSetFlagsBit((*_Xm_fastPtr)->flags, bit_field) ; \
   }

/* _XmGetBaseClassExtPtr can return NULL or a pointer to a NULL extension,
 * for non Motif classes in particular, so we check that up front.
 * We use the global _Xm_fastPtr for that purpose, this variable exists
 * already in BaseClass.c for apparently no other use.
 */

#define _XmIsFastSubclass(wc, bit) \
	((_Xm_fastPtr = _XmGetBaseClassExtPtr((wc),XmQmotif)) && \
         (*_Xm_fastPtr)) ? \
	     (_XmGetFlagsBit(((*_Xm_fastPtr)->flags), bit) ? TRUE : FALSE) \
		 : FALSE

#else
extern void _XmFastSubclassInit(WidgetClass, unsigned int);
extern Boolean _XmIsFastSubclass(WidgetClass, unsigned int);
#endif  /* XTHREADS */

#define XmBaseClassExtVersion 2L
#define XmBaseClassExtVersion 2L


typedef Cardinal (*XmGetSecResDataFunc)( WidgetClass,
					    XmSecondaryResourceData **);

typedef struct _XmObjectClassExtRec{
    XtPointer 		next_extension;	
    XrmQuark 		record_type;	
    long 		version;	
    Cardinal 		record_size;	
} XmObjectClassExtRec, *XmObjectClassExt;

typedef struct _XmGenericClassExtRec{
    XtPointer 		next_extension;	
    XrmQuark 		record_type;	
    long 		version;	
    Cardinal 		record_size;	
} XmGenericClassExtRec, *XmGenericClassExt;

typedef struct _XmWrapperDataRec{
    struct _XmWrapperDataRec *next;
    WidgetClass		widgetClass;
    XtInitProc		initializeLeaf;
    XtSetValuesFunc	setValuesLeaf;
    XtArgsProc		getValuesLeaf;
    XtRealizeProc	realize;
    XtWidgetClassProc	classPartInitLeaf;
    XtWidgetProc	resize;
    XtGeometryHandler   geometry_manager;

    /* init_depth is obselete now .. */
    Cardinal		init_depth;

    int                 initializeLeafCount;
    int                 setValuesLeafCount;
    int                 getValuesLeafCount;
    XtInitProc          constraintInitializeLeaf;
    XtSetValuesFunc     constraintSetValuesLeaf;
    int                 constraintInitializeLeafCount;
    int 		constraintSetValuesLeafCount;
} XmWrapperDataRec, *XmWrapperData;

typedef struct _XmBaseClassExtRec{
    XtPointer 		next_extension;	
    XrmQuark 		record_type;	
    long 		version;	
    Cardinal 		record_size;	
    XtInitProc		initializePrehook;
    XtSetValuesFunc 	setValuesPrehook;
    XtInitProc		initializePosthook;
    XtSetValuesFunc 	setValuesPosthook;
    WidgetClass		secondaryObjectClass;
    XtInitProc		secondaryObjectCreate;
    XmGetSecResDataFunc	getSecResData;
    unsigned char	flags[32];
    XtArgsProc		getValuesPrehook;
    XtArgsProc		getValuesPosthook;
    XtWidgetClassProc	classPartInitPrehook;
    XtWidgetClassProc	classPartInitPosthook;
    XtResourceList	ext_resources;
    XtResourceList	compiled_ext_resources;
    Cardinal		num_ext_resources;
    Boolean		use_sub_resources;
    XmWidgetNavigableProc widgetNavigable;
    XmFocusChangeProc	focusChange;
    XmWrapperData	wrapperData;
} XmBaseClassExtRec, *XmBaseClassExt;


typedef struct _XmWidgetExtDataRec{
    Widget		widget;
    Widget		reqWidget;
    Widget		oldWidget;
} XmWidgetExtDataRec, *XmWidgetExtData;

externalref XrmQuark	     XmQmotif;
externalref int		     _XmInheritClass;
externalref XmBaseClassExt * _Xm_fastPtr;
  
/********    Private Function Declarations    ********/


extern XmGenericClassExt * _XmGetClassExtensionPtr( 
                        XmGenericClassExt *listHeadPtr,
                        XrmQuark owner) ;
extern Boolean _XmIsSubclassOf(WidgetClass wc, WidgetClass sc);


/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmBaseClassP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

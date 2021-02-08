/*
 *    Copyright 1991, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 * This file contains all routines for dealing with the hierarchy
 * browser when shown as a tree or outline.
 */

#ifndef _LIST_H
#define _LIST_H

#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Xm.h>
#include <Xm/Ext.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************
 *
 * Stack Data structure.  
 *
 ************************************************************/

typedef struct _XmStackRec {
    int top, alloc;    		/* The top node of the stack, and the number
				   of allocated nodes. */
    XtPointer * elems;		/* The stack elements. */
} XmStackRec, *XmStack;

/************************************************************
 *
 *  Global function defs.
 *
 ************************************************************/

XmStack _XmStackInit(void);
void _XmStackFree(XmStack), _XmStackPush(XmStack, XtPointer);
XtPointer _XmStackPop(XmStack);

/************************************************************
 *
 * Queue Data structure.  
 *
 ************************************************************/

typedef struct __XmQElem {
    struct __XmQElem *next, *prev;	/* doubly linked list. */
    XtPointer data;		/* The data associated with this element. */
    Boolean alloced;
} _XmQElem;

typedef struct _XmQueueRec {
    _XmQElem *first, *last;	/* the first and last elements. */
    _XmQElem *free_elems;		/* Unused elements. */
} XmQueueRec, *XmQueue;

/************************************************************
 *
 *  Global function defs.
 *
 ************************************************************/

XmQueue _XmQueueInit(void);
void _XmQueueFree(XmQueue), _XmQueuePush(XmQueue, XtPointer);
XtPointer _XmQueuePop(XmQueue);
int _XmQueueCount(XmQueue);

/* 
 * Internal functions used only by other parts of the utils library.
 */

void _Xm_AddQueue(XmQueue, _XmQElem *, _XmQElem *);
_XmQElem * _Xm_RemQueue(_XmQElem **);
_XmQElem * _Xm_GetNewElement(XmQueue);

/************************************************************
 *
 * New types.
 *
 ************************************************************/

typedef _XmQElem XmListElem;
typedef XmQueueRec *XmList;
typedef Boolean (*XmListFunc)(XmListElem *, XtPointer);

/************************************************************
 *
 * Macros.
 *
 ************************************************************/

#define XmListElemNext(elem) (elem)->next
#define XmListElemPrev(elem) (elem)->prev
#define XmListElemData(elem) (elem)->data

#define XmListFirst(list) (list)->first
#define XmListLast(list) (list)->last

/************************************************************
 *
 *  Global function defs.
 *
 ************************************************************/

void _XmListFree(XmList), _XmListRemove(XmList, XmListElem *);

XmListElem * _XmListAddAfter(XmList, XmListElem *, XtPointer);
XmListElem * _XmListAddBefore(XmList, XmListElem *, XtPointer); 

XmList _XmListInit(void);

int _XmListCount(XmList);

XmListElem *_XmListExec(XmList, XmListElem *, XmListElem *, XmListFunc, XtPointer);

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif

/* #ifdef XmRENAME_WIDGETS */
/* #define USE_OLD_NAMES */
/* #endif */

#ifdef USE_OLD_NAMES

#define ListAddAfter	_XmListAddAfter
#define ListAddBefore	_XmListAddBefore
#define ListCount	_XmListCount
#define ListExec	_XmListExec
#define ListFree	_XmListFree
#define ListInit	_XmListInit
#define ListRemove	_XmListRemove
#define QueueCount	_XmQueueCount
#define QueueFree	_XmQueueFree
#define QueueInit	_XmQueueInit
#define QueuePop	_XmQueuePop
#define QueuePush	_XmQueuePush
#define StackFree	_XmStackFree
#define StackInit	_XmStackInit
#define StackPop	_XmStackPop
#define StackPush	_XmStackPush
#define _AddQueue	_Xm_AddQueue
#define _GetNewElement	_Xm_GetNewElement
#define _RemQueue	_Xm_RemQueue

#define Stack		XmStack
#define StackRec	XmStackRec
#define QElem		_XmQElem
#define QueueRec	XmQueueRec
#define Queue		XmQueue
#define ListElem	XmListElem
#define List		XmList
#define ListFunc	XmListFunc

#define ListElemNext	XmListElemNext
#define ListElemPrev	XmListElemPrev
#define ListElemData	XmListElemData
#define ListFirst	XmListFirst
#define ListLast	XmListLast

#endif /* USE_OLD_NAMES */

#endif /* _LIST_H */

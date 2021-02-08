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
/*   $XConsortium: DropSMgrP.h /main/11 1995/07/14 10:31:14 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDropSMgrP_h
#define _XmDropSMgrP_h

#include <Xm/XmP.h>
#include <Xm/DropSMgr.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*XmDSMCreateInfoProc)
	(XmDropSiteManagerObject, Widget, ArgList, Cardinal);
typedef void (*XmDSMDestroyInfoProc)
	(XmDropSiteManagerObject, Widget);
typedef void (*XmDSMStartUpdateProc)
	(XmDropSiteManagerObject, Widget);
typedef void (*XmDSMRetrieveInfoProc)
	(XmDropSiteManagerObject, Widget, ArgList, Cardinal);
typedef void (*XmDSMUpdateInfoProc)
	(XmDropSiteManagerObject, Widget, ArgList, Cardinal);
typedef void (*XmDSMEndUpdateProc)
	(XmDropSiteManagerObject, Widget);
typedef void (*XmDSMUpdateProc)
	(XmDropSiteManagerObject, XtPointer, XtPointer);
typedef void (*XmDSMProcessMotionProc)
	(XmDropSiteManagerObject, XtPointer, XtPointer);
typedef void (*XmDSMProcessDropProc)
	(XmDropSiteManagerObject, XtPointer, XtPointer);
typedef void (*XmDSMOperationChangedProc)
	(XmDropSiteManagerObject, XtPointer, XtPointer);
typedef void (*XmDSMChangeRootProc)
	(XmDropSiteManagerObject, XtPointer, XtPointer);
typedef void (*XmDSMInsertInfoProc)
	(XmDropSiteManagerObject, XtPointer, XtPointer);
typedef void (*XmDSMRemoveInfoProc)
	(XmDropSiteManagerObject, XtPointer);
typedef void (*XmDSMSyncTreeProc)
	(XmDropSiteManagerObject, Widget);
typedef int  (*XmDSMGetTreeFromDSMProc)
	(XmDropSiteManagerObject, Widget, XtPointer);
typedef void (*XmDSMCreateDSInfoTable)
	(XmDropSiteManagerObject);
typedef void (*XmDSMDestroyDSInfoTable)
	(XmDropSiteManagerObject);
typedef void (*XmDSMRegisterInfoProc)
	(XmDropSiteManagerObject, Widget, XtPointer);
typedef XtPointer (*XmDSMWidgetToInfoProc)
	(XmDropSiteManagerObject, Widget);
typedef void (*XmDSMUnregisterInfoProc)
	(XmDropSiteManagerObject, XtPointer);

typedef struct {
    XmDSMCreateInfoProc		createInfo;
    XmDSMDestroyInfoProc	destroyInfo;
    XmDSMStartUpdateProc	startUpdate;
    XmDSMRetrieveInfoProc	retrieveInfo;
    XmDSMUpdateInfoProc		updateInfo;
    XmDSMEndUpdateProc		endUpdate;
    
    /* Used by DragController Object */
    
    XmDSMUpdateProc		updateDSM;
    
    /* Used by update proc */
    
    XmDSMProcessMotionProc	processMotion;
    XmDSMProcessDropProc	processDrop;
    XmDSMOperationChangedProc	operationChanged;
    XmDSMChangeRootProc		changeRoot;
    
    /* Used to manage DropSites */
    
    XmDSMInsertInfoProc		insertInfo;
    /* Need a get and a put function for update? */
    XmDSMRemoveInfoProc		removeInfo;

	/* Used to manage the pre-register information */

    XmDSMSyncTreeProc		syncTree;
    XmDSMGetTreeFromDSMProc	getTreeFromDSM;
    
    /* Used to hash between widgets and info */
    
    XmDSMCreateDSInfoTable	createTable;
    XmDSMDestroyDSInfoTable 	destroyTable;
    XmDSMRegisterInfoProc	registerInfo;
    XmDSMWidgetToInfoProc	widgetToInfo;
    XmDSMUnregisterInfoProc 	unregisterInfo;
    
    XtPointer			extension;
} XmDropSiteManagerClassPart;

/*  Full class record declaration for dropSite class  */

typedef struct _XmDropSiteManagerClassRec{
    ObjectClassPart		object_class;
    XmDropSiteManagerClassPart		dropManager_class;
} XmDropSiteManagerClassRec;

externalref XmDropSiteManagerClassRec 	xmDropSiteManagerClassRec;

/* Macros for calling methods */

#define DSMCreateInfo(dsm, widget, args, numArgs) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.createInfo) \
		((dsm), (widget), (args), (numArgs))

#define DSMDestroyInfo(dsm, widget) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.destroyInfo) \
		((dsm), (widget))

#define DSMStartUpdate(dsm, widget) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.startUpdate) \
		((dsm), (widget))

#define DSMRetrieveInfo(dsm, widget, args, numArgs) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.retrieveInfo) \
		((dsm), (widget), (args), (numArgs))

#define DSMUpdateInfo(dsm, widget, args, numArgs) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.updateInfo) \
		((dsm), (widget), (args), (numArgs))

#define DSMEndUpdate(dsm, widget) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.endUpdate) \
		((dsm), (widget))

#define DSMUpdate(dsm, clientData, callData) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.updateDSM) \
		((dsm), (clientData), (callData))

#define DSMProcessMotion(dsm, clientData, callData) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.processMotion) \
		((dsm), (clientData), (callData))

#define DSMProcessDrop(dsm, clientData, callData) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.processDrop) \
		((dsm),(clientData), (callData))

#define DSMOperationChanged(dsm, clientData, callData) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.operationChanged) \
		((dsm),(clientData), (callData))

#define DSMChangeRoot(dsm, clientData, callData) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.changeRoot) \
		((dsm), (clientData), (callData))

#define DSMInsertInfo(dsm, info, call_data) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.insertInfo) \
		((dsm), (info), (call_data))

#define DSMRemoveInfo(dsm, info) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.removeInfo) \
		((dsm), (info))

#define DSMSyncTree(dsm, shell) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.syncTree) \
		((dsm), (shell))

#define DSMGetTreeFromDSM(dsm, shell, dataPtr) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.getTreeFromDSM) \
		((dsm), (shell), (dataPtr))

#define DSMCreateTable(dsm) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.createTable) \
		((dsm))

#define DSMDestroyTable(dsm) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.destroyTable) \
		((dsm))

#define DSMRegisterInfo(dsm, widget, info) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.registerInfo) \
		((dsm), (widget), (info))

#define DSMWidgetToInfo(dsm, widget) \
	(XtPointer) ((((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.widgetToInfo) \
		((dsm), (widget)))

#define DSMUnregisterInfo(dsm, info) \
	(((XmDropSiteManagerObjectClass) XtClass(dsm))-> \
		dropManager_class.unregisterInfo) \
		  ((dsm), (info))

/* Internal update struct */

typedef struct __XmDropSiteUpdateInfoRec {
  XmDropSiteManagerObject		dsm;
  Widget				refWidget;
  struct __XmDropSiteUpdateInfoRec	*next;
} _XmDropSiteUpdateInfoRec, *_XmDropSiteUpdateInfo;

/* DropSiteManager instance record */

typedef struct _XmDropSiteManagerPart{
    XtCallbackProc	notifyProc;
    XtCallbackProc	treeUpdateProc;
    XtPointer		client_data;
    XtPointer		dragUnderData;
    XtPointer		curInfo;
    Time		curTime;
    Position		curX, curY, oldX, oldY;
    unsigned char	curDropSiteStatus;
    Widget		curDragContext;
    Boolean		curAnimate;
    unsigned char	curOperations;
    unsigned char	curOperation;
    XmRegion		curAncestorClipRegion;
    XmRegion		newAncestorClipRegion;
    XtPointer		dsTable;
    XtPointer		dsRoot;
    Position		rootX, rootY;
    Dimension		rootW, rootH;
    XtPointer		clipperList;
    _XmDropSiteUpdateInfo	updateInfo;
    XtIntervalId	updateTimeOutId;
} XmDropSiteManagerPart, *XmDropSiteManagerPartPtr;

/* Full instance record declaration */

typedef struct _XmDropSiteManagerRec{
    ObjectPart			object;
    XmDropSiteManagerPart	dropManager;
} XmDropSiteManagerRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDropSMgrP_h */

/* $XConsortium: MenuT.h /main/5 1995/07/15 20:53:03 drk $ */
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

#ifndef _XmMenuT_H
#define _XmMenuT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Quick fix for Linux-ARM where "arm" is the #define symbol */
#ifdef arm
# undef arm
#endif

/* Menu System Traits */
externalref XrmQuark XmQTmenuSystem;
externalref XrmQuark XmQTmenuSavvy;

/* Trait structures and typedefs, place typedefs first */

/* Used by the disable callback method of the menu savvy trait */
typedef enum {
    XmDISABLE_ACTIVATE,   /* defer to the entryCallback */
    XmENABLE_ACTIVATE     /* invoke own activate callback */
} XmActivateState;

/* Menu trait typedefs */

typedef int (*XmMenuSystemWidgetProc)(Widget);
typedef Boolean (*XmMenuSystemVerifyProc)(Widget, XEvent*);
typedef void (*XmMenuSystemControlTraversalProc)(Widget, Boolean);
typedef void (*XmMenuSystemCascadeProc)(Widget, Widget, XEvent*);
typedef void (*XmMenuSystemPositionProc)(Widget, XEvent*);
typedef Boolean (*XmMenuSystemPopdownProc)(Widget, XEvent*);
typedef void (*XmMenuSystemEntryCallbackProc)(Widget, Widget, XtPointer);
typedef Boolean (*XmMenuSystemUpdateHistoryProc)(Widget, Widget, Boolean);
typedef void (*XmMenuSystemUpdateBindingsProc)(Widget, int);
typedef void (*XmMenuSystemRecordPostFromWidgetProc)(Widget, Widget, Boolean);
typedef void (*XmMenuSystemDisarmProc)(Widget);
typedef Widget (*XmMenuSystemPopupPostedProc)(Widget);
typedef void (*XmMenuSavvyDisableProc)(Widget, XmActivateState);
typedef char* (*XmMenuSavvyGetAcceleratorProc)(Widget);
typedef KeySym (*XmMenuSavvyGetMnemonicProc)(Widget);
typedef char* (*XmMenuSavvyGetActivateCBNameProc)();
#define XmMenuSystemTypeProc		XmMenuSystemWidgetProc
#define XmMenuSystemStatusProc		XmMenuSystemWidgetProc
#define XmMenuSystemGetPostedFromWidgetProc	XmMenuSystemDisarmProc
#define XmMenuSystemArmProc		XmMenuSystemDisarmProc
#define XmMenuSystemMenuBarCleanupProc	XmMenuSystemDisarmProc
#define XmMenuSystemTearOffArmProc	XmMenuSystemDisarmProc
#define XmMenuSystemReparentProc	XmMenuSystemPositionProc
#define XmMenuSystemPopdownAllProc	XmMenuSystemPositionProc
#define XmMenuSystemChildFocusProc	XmMenuSystemDisarmProc

/* XmTmenuProcTrait */

/* Version 0: initial release. */

typedef struct _XmMenuSystemTraitRec
{
  int					version;		/* 0 */
  XmMenuSystemTypeProc			type;
  XmMenuSystemStatusProc		status;
  XmMenuSystemCascadeProc		cascade;
  XmMenuSystemVerifyProc		verifyButton;
  XmMenuSystemControlTraversalProc	controlTraversal;
  XmMenuSystemMenuBarCleanupProc	menuBarCleanup;
  XmMenuSystemPopdownProc		popdown;
  XmMenuSystemPopdownProc		buttonPopdown;
  XmMenuSystemReparentProc		reparentToTearOffShell;
  XmMenuSystemReparentProc		reparentToMenuShell;
  XmMenuSystemArmProc			arm;
  XmMenuSystemDisarmProc		disarm;
  XmMenuSystemTearOffArmProc		tearOffArm;
  XmMenuSystemEntryCallbackProc		entryCallback;
  XmMenuSystemUpdateHistoryProc		updateHistory;
  XmMenuSystemGetPostedFromWidgetProc	getLastSelectToplevel;
  XmMenuSystemPositionProc		position;
  XmMenuSystemUpdateBindingsProc	updateBindings;
  XmMenuSystemRecordPostFromWidgetProc	recordPostFromWidget;
  XmMenuSystemPopdownAllProc		popdownEveryone;
  XmMenuSystemChildFocusProc		childFocus;
  XmMenuSystemPopupPostedProc		getPopupPosted;
} XmMenuSystemTraitRec, *XmMenuSystemTrait;

/* XmTmenuSavvyTrait */

/* Version 0: initial release. */

typedef struct _XmMenuSavvyTraitRec
{
  int					version;		/* 0 */
  XmMenuSavvyDisableProc		disableCallback;
  XmMenuSavvyGetAcceleratorProc 	getAccelerator;
  XmMenuSavvyGetMnemonicProc		getMnemonic;
  XmMenuSavvyGetActivateCBNameProc	getActivateCBName;
} XmMenuSavvyTraitRec, *XmMenuSavvyTrait;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMenuT_H */

/* $XConsortium: DialogSavvyT.h /main/5 1995/07/15 20:50:29 drk $ */
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
#ifndef _XmDialogSavvyT_H
#define _XmDialogSavvyT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTdialogShellSavvy;
/* This trait also requires a resource named "defaultPosition".
   If the child has the trait, the resource will be get and set by 
   the DialogShell ChangeManaged */
   
/* Trait structures and typedefs, place typedefs first */

typedef void (*XmDialogSavvyMapUnmapProc)(Widget wid, 
					  Boolean map_unmap);


/* Version 0: initial release. */

typedef struct _XmDialogSavvyTraitRec	 {
  int			    version;		/* 0 */
  XmDialogSavvyMapUnmapProc callMapUnmapCB;
} XmDialogSavvyTraitRec,*XmDialogSavvyTrait;


/* This macro is part of the trait and is used for the following situation
   DialogShells always mimic the child position on themselves.
   If the SetValues on a bb child position was 0,
   which is always the _current_ position of the bb in a DialogShell,
   Xt does not see a change and therefore not trigerred a geometry request.
   So BB (or any dialogShellSavvy child) has to catch this case
   and change the position request to use a special value in its
   SetValues method, XmDIALOG_SAVVY_FORCE_ORIGIN, to notify the Dialog that 
   it really wants to move in 0 */

#define XmDIALOG_SAVVY_FORCE_ORIGIN ((Position)~0L)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDialogSavvyT_H */

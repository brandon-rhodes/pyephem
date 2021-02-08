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
/* $XConsortium: MenuShellP.h /main/11 1995/07/13 17:36:37 drk $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmMenuShellP_h
#define _XmMenuShellP_h

#include <Xm/MenuShell.h>
#include <Xm/XmP.h>
#include <X11/ShellP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The MenuShell instance record */

typedef struct 
{
    unsigned char	    focus_policy;
    XmFocusData		    focus_data;
    Boolean                 private_shell;
    XmFontList      	    default_font_list;
    XmFontList              button_font_list;
    XmFontList              label_font_list;
    XmDirection             layout_direction;
    Boolean                 animate;
} XmMenuShellPart;


/* Full instance record declaration */

typedef  struct _XmMenuShellRec 
{
    CorePart		    core;
    CompositePart	    composite;
    ShellPart		    shell;
    OverrideShellPart	    override;
    XmMenuShellPart	    menu_shell;
} XmMenuShellRec;

typedef  struct _XmMenuShellWidgetRec /* OBSOLETE (for compatibility only).*/
{
    CorePart		    core;
    CompositePart	    composite;
    ShellPart		    shell;
    OverrideShellPart	    override;
    XmMenuShellPart	    menu_shell;
} XmMenuShellWidgetRec;



/* MenuShell class structure */

typedef struct 
{
    XtActionProc    popdownOne;	     /* unpost portion or all of menu */
    XtActionProc    popdownEveryone; /* unpost portion of menu */
    XtActionProc    popdownDone;     /* unpost all menus */
    XmMenuPopupProc popupSharedMenupane; /* post shared menus */
				         /* (should be popupSharedMenuShell) */
    XtPointer	    extension;	     /* Pointer to extension record */
} XmMenuShellClassPart;


/* Full class record declaration */

typedef struct _XmMenuShellClassRec 
{
    CoreClassPart	    core_class;
    CompositeClassPart	    composite_class;
    ShellClassPart	    shell_class;
    OverrideShellClassPart  override_shell_class;
    XmMenuShellClassPart    menu_shell_class;
} XmMenuShellClassRec;


externalref XmMenuShellClassRec  xmMenuShellClassRec;

#define MS_FocusPolicy(m) \
	(((XmMenuShellWidget)m)->menu_shell.focus_policy)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMenuShellP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

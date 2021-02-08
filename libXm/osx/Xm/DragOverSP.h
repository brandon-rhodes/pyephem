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
/*   $XConsortium: DragOverSP.h /main/9 1995/07/14 10:26:38 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDragOverSP_h
#define _XmDragOverSP_h

#include <X11/IntrinsicP.h>

#include <X11/Shell.h>
#include <X11/ShellP.h>
#include <Xm/XmP.h>
#include <Xm/DragIconP.h>
#include <Xm/DragOverS.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DOExpose(do) \
	((XtClass(do))->core_class.expose) ((Widget)(do), NULL, NULL)

/* 
 * DRAGOVER SHELL
 */
typedef struct 
{
    XtPointer				extension;
} XmDragOverShellClassPart;

/* Full class record declaration */

typedef struct _XmDragOverShellClassRec 
{
    CoreClassPart 		core_class;
    CompositeClassPart 		composite_class;
    ShellClassPart 		shell_class;
    WMShellClassPart	        wm_shell_class;
    VendorShellClassPart 	vendor_shell_class;
    XmDragOverShellClassPart 	dragOver_shell_class;
} XmDragOverShellClassRec;

externalref XmDragOverShellClassRec xmDragOverShellClassRec;

typedef struct _XmBackingRec{
    Position	x, y;
    Pixmap	pixmap;
}XmBackingRec, *XmBacking;

typedef struct _XmDragOverBlendRec{
    XmDragIconObject		sourceIcon;	/* source icon */
    Position			sourceX;	/* source location in blend */
    Position			sourceY;	/* source location in blend */
    XmDragIconObject		mixedIcon;	/* blended icon */
    GC				gc;		/* appropriate depth */
}XmDragOverBlendRec, *XmDragOverBlend;

typedef struct _XmDragOverShellPart{
    Position		hotX;		/* current hotX */
    Position		hotY;		/* current hotY */
    unsigned char	cursorState;	/* current cursor state */
    unsigned char	mode;
    unsigned char	activeMode;

    Position		initialX;	/* initial hotX */
    Position		initialY;	/* initial hotY */

    XmDragIconObject	stateIcon;	/* current state icon */
    XmDragIconObject	opIcon;		/* current operation icon */

    XmDragOverBlendRec	cursorBlend;	/* cursor blending */
    XmDragOverBlendRec	rootBlend;	/* pixmap or window blending */
    Pixel		cursorForeground;
    Pixel		cursorBackground;
    Cursor		ncCursor;	/* noncached cursor */
    Cursor		activeCursor;	/* the current cursor */

    XmBackingRec	backing; 	/* backing store for pixdrag */
    Pixmap		tmpPix;		/* temp storage for pixdrag */
    Pixmap		tmpBit;		/* temp storage for pixdrag */
    Boolean             isVisible;	/* shell is visible */

    /* Added for ShapedWindow dragging */
    /* Resources */
    Boolean		installColormap;/* Install the colormap */

    /* locals */
    Boolean		holePunched;	/* true if hole is punched */

    /* the following variables are used to make sure the correct colormap */
    /* is installed.  colormapWidget is initially the parent widget, but */
    /* can be changed by calling DragShellColormapWidget.		*/
    Widget		colormapWidget;	/* The widget I'm dragging from */
    Widget		colormapShell;	/* It's shell, install colormap here */
    Boolean		colormapOverride; /* shell is override rediirect */
    Colormap*		savedColormaps;	/* used with override redirect */
    int			numSavedColormaps;
}XmDragOverShellPart;

typedef  struct _XmDragOverShellRec{
    CorePart	 	core;
    CompositePart 	composite;
    ShellPart 		shell;
    WMShellPart		wm;
    VendorShellPart	vendor;
    XmDragOverShellPart	drag;
} XmDragOverShellRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragOverSP_h */

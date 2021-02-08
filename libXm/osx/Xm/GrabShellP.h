/* $XConsortium: GrabShellP.h /main/5 1995/07/15 20:51:26 drk $ */
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
#ifndef _XmGrabShellP_h
#define _XmGrabShellP_h

#include <Xm/GrabShell.h>
#include <Xm/XmP.h>
#include <X11/ShellP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The GrabShell instance record */

typedef struct 
{
  Cursor	cursor;
  Dimension	shadow_thickness;
  Pixel		top_shadow_color;
  Pixmap  	top_shadow_pixmap;
  Pixel   	bottom_shadow_color;
  Pixmap  	bottom_shadow_pixmap;
  GC      	top_shadow_GC;
  GC      	bottom_shadow_GC;
  Boolean	owner_events;
  int		grab_style;
  /* Internal fields */
  Time		post_time;
  Time		unpost_time;
  Boolean	mapped;
  Window	old_focus;
  int		old_revert_to;
} XmGrabShellPart;


/* Full instance record declaration */

typedef  struct _XmGrabShellRec 
{
  CorePart		core;
  CompositePart		composite;
  ShellPart		shell;
  WMShellPart		wm_shell;
  VendorShellPart	vendor_shell;
  XmGrabShellPart	grab_shell;
} XmGrabShellRec;

typedef  struct _XmGrabShellWidgetRec /* OBSOLETE (for compatibility only).*/
{
  CorePart		core;
  CompositePart		composite;
  ShellPart		shell;
  WMShellPart		wm_shell;
  VendorShellPart	vendor_shell;
  XmGrabShellPart	grab_shell;
} XmGrabShellWidgetRec;



/* GrabShell class structure */

typedef struct 
{
  XtPointer		extension;	 /* Pointer to extension record */
} XmGrabShellClassPart;


/* Full class record declaration */

typedef struct _XmGrabShellClassRec 
{
  CoreClassPart	    	core_class;
  CompositeClassPart	composite_class;
  ShellClassPart	shell_class;
  WMShellClassPart	wm_shell_class;
  VendorShellClassPart	vendor_shell_class;
  XmGrabShellClassPart  grab_shell_class;
} XmGrabShellClassRec;


externalref XmGrabShellClassRec  xmGrabShellClassRec;

/********    Private Function Declarations    ********/

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmGrabShellP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

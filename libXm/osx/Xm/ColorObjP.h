/* $XConsortium: ColorObjP.h /main/10 1996/12/16 18:30:49 drk $ */
/*
 * @OPENGROUP_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF for
 * the full copyright text.
 * 
 * This software is subject to an open license. It may only be
 * used on, with or for operating systems which are themselves open
 * source systems. You must contact The Open Group for a license
 * allowing distribution and sublicensing of this software on, with,
 * or for operating systems which are not Open Source programs.
 * 
 * See http://www.opengroup.org/openmotif/license for full
 * details of the license agreement. Any use, reproduction, or
 * distribution of the program constitutes recipient's acceptance of
 * this agreement.
 * 
 * EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 * PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 * WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 * OR FITNESS FOR A PARTICULAR PURPOSE
 * 
 * EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 * NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 * EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 */
/*
 * HISTORY
 */
#ifndef _XmColorObjP_h
#define _XmColorObjP_h

#include <Xm/VendorSP.h>

#ifdef __cplusplus
extern "C" {
#endif

/** misc structures, defines, and functions for using ColorObj **/

#define XmCO_MAX_NUM_COLORS	 8
#define XmCO_NUM_COLORS		 XmCO_MAX_NUM_COLORS
#define XmPIXEL_SET_PROP_VERSION '1'

/* Constants for color usage */
enum { XmCO_BLACK_WHITE, XmCO_LOW_COLOR, XmCO_MEDIUM_COLOR, XmCO_HIGH_COLOR };

typedef struct {
    Pixel fg;
    Pixel bg;
    Pixel ts;
    Pixel bs;
    Pixel sc;
} XmPixelSet;

typedef XmPixelSet Colors[XmCO_NUM_COLORS];

typedef struct _XmColorObjPart {
    XtArgsProc          RowColInitHook;
    XmPixelSet       	*myColors;     /* colors for my (application) screen */
    int             	myScreen;
    Display             *display;     /* display connection for "pseudo-app" */
    Colors         	*colors;      /* colors per screen for workspace mgr */
    int             	numScreens;   /*               for workspace manager */
    Atom           	*atoms;       /* to identify colorsrv screen numbers */
    Boolean         	colorIsRunning;   /* used for any color problem      */
    Boolean         	done;
    int            	*colorUse;
    int             	primary;
    int             	secondary;
    int             	text;          /* color set id for text widgets */
    int             	active;
    int             	inactive;
    Boolean         	useColorObj;  /* read only resource variable */
    Boolean             useText;        /* use text color set id for text? */
    Boolean             useTextForList; /* use text color set id for lists? */
    
    Boolean		useMask;
    Boolean		useMultiColorIcons;
    Boolean		useIconFileCache;

} XmColorObjPart;


typedef struct _XmColorObjRec {
    CorePart 		core;
    CompositePart 	composite;
    ShellPart 		shell;
    WMShellPart		wm;
    XmColorObjPart	color_obj;
} XmColorObjRec;

typedef struct _XmColorObjClassPart {
    XtPointer        extension;
} XmColorObjClassPart;

/* 
 * we make it a appShell subclass so it can have it's own instance
 * hierarchy
 */
typedef struct _XmColorObjClassRec{
    CoreClassPart      		core_class;
    CompositeClassPart 		composite_class;
    ShellClassPart  		shell_class;
    WMShellClassPart   		wm_shell_class;
    XmColorObjClassPart		color_obj_class;
} XmColorObjClassRec;


externalref XmColorObjClassRec xmColorObjClassRec;


#ifndef XmIsColorObj
#define XmIsColorObj(w) (XtIsSubclass(w, xmColorObjClass))
#endif /* XmIsXmDisplay */

externalref WidgetClass  xmColorObjClass;
typedef struct _XmColorObjClassRec *XmColorObjClass;
typedef struct _XmColorObjRec      *XmColorObj;


#define  XmCO_DitherTopShadow(display, screen, pixelSet) \
                        ((pixelSet)->bs == BlackPixel((display), (screen)))

#define  XmCO_DitherBottomShadow(display, screen, pixelSet) \
                        ((pixelSet)->ts == WhitePixel((display), (screen)))

#define  XmCO_DITHER     XmS50_foreground
#define  XmCO_NO_DITHER  XmSunspecified_pixmap


/********    Private Function Declarations    ********/

extern Boolean XmeGetIconControlInfo( 
                        Screen *screen,
                        Boolean *useMaskRtn,
                        Boolean *useMultiColorIconsRtn,
                        Boolean *useIconFileCacheRtn) ;

extern Boolean XmeUseColorObj( void ) ;


extern Boolean XmeGetColorObjData(
                   Screen * screen,
                   int *colorUse,
		   XmPixelSet *pixelSet,
		   unsigned short num_pixelSet,
		   short *active_id,
		   short *inactive_id,
		   short *primary_id,
		   short *secondary_id,
		   short *text_id) ;

extern Boolean XmeGetDesktopColorCells (
                         Screen * screen, 
			 Colormap colormap, 
			 XColor * colors,  
			 int n_colors,     
			 int * ncolors_returns) ;

extern Boolean XmeGetPixelData (
			int screen_number,
			int *colorUse,
			XmPixelSet *pixelSet,
			short *a,
			short *i,
			short *p,
			short *s) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmColorObjP_h */


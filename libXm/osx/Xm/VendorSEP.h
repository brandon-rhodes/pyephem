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
/* $XConsortium: VendorSEP.h /main/14 1996/05/21 12:11:50 pascale $ */
/* (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/* (c) Copyright 1988 MICROSOFT CORPORATION */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
#ifndef _XmVendorSEP_h
#define _XmVendorSEP_h

#include <Xm/ShellEP.h>
#include <Xm/MwmUtil.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmIsVendorShellExt
#define XmIsVendorShellExt(w)	XtIsSubclass(w, xmVendorShellExtObjectClass)
#endif /* XmIsVendorShellExt */

typedef struct _XmVendorShellExtRec *XmVendorShellExtObject;
typedef struct _XmVendorShellExtClassRec *XmVendorShellExtObjectClass;
externalref WidgetClass xmVendorShellExtObjectClass;


#define XmInheritProtocolHandler	((XtCallbackProc)_XtInherit)

typedef struct _XmVendorShellExtClassPart{
    XtCallbackProc	delete_window_handler;
    XtCallbackProc	offset_handler;
    XtPointer		extension;
}XmVendorShellExtClassPart, *XmVendorShellExtClassPartPtr;

typedef struct _XmVendorShellExtClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmShellExtClassPart		shell_class;
    XmVendorShellExtClassPart 	vendor_class;
}XmVendorShellExtClassRec;

typedef struct {
 XmFontList		default_font_list;
 unsigned char		focus_policy;
 XmFocusData		focus_data;
 unsigned char		delete_response;
 unsigned char		unit_type;
 MwmHints		mwm_hints;
 MwmInfo		mwm_info;
 String			mwm_menu;
 XtCallbackList		focus_moved_callback;
 /*
  * internal fields
  */
 Widget			old_managed;
 Position		xAtMap, yAtMap, xOffset, yOffset;
 unsigned long		lastOffsetSerial;
 unsigned long		lastMapRequest;
 Boolean		externalReposition;

 /* mapStyle is an unused field. I'm using this field to keep
  * track of the *font_list resource values. Refer 
  * CheckSetRenderTable in VendorSE.c
  */
 unsigned char		mapStyle;

 XtCallbackList		realize_callback;
 XtGrabKind		grab_kind;
 Boolean		audible_warning;
 XmFontList             button_font_list;
 XmFontList             label_font_list;
 XmFontList             text_font_list;
 String			input_method_string;
 String			preedit_type_string;
 unsigned int           light_threshold;
 unsigned int           dark_threshold;
 unsigned int           foreground_threshold;
 unsigned int		im_height;
 XtPointer		im_info;
 Boolean		im_vs_height_set;

 /* New public resources for Motif 2.0 */
 XmDirection            layout_direction;
 XmInputPolicy		input_policy;

 Boolean 		verify_preedit;

 /* toolTip related stuff */
 Widget label;			/* XmLabel for the tips */
 int post_delay;		/* delay before posting XmNtoolTipPostDelay */
 int post_duration;		/* duration XmNtoolTipPostDuration */
 XtIntervalId timer;		/* timer for post delay */
 XtIntervalId duration_timer;	/* timer for duration */
 Time leave_time;		/* time of the last leave event */
 Widget slider;			/* the XmSlideContext used to slide in the tip */
 Boolean enable;		/* flag to disable all this stuff */

} XmVendorShellExtPart, *XmVendorShellExtPartPtr;

externalref XmVendorShellExtClassRec 	xmVendorShellExtClassRec;

typedef struct _XmVendorShellExtRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmShellExtPart		shell;
    XmVendorShellExtPart 	vendor;
} XmVendorShellExtRec;


/******** Xme Functions ********/

void XmeAddFocusChangeCallback(Widget, XtCallbackProc, XtPointer);
void XmeRemoveFocusChangeCallback(Widget, XtCallbackProc, XtPointer);

/******** End Xme Functions ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* _XmVendorSEP_h */

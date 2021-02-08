/* $XConsortium: AccColorT.h /main/5 1995/07/15 20:47:59 drk $ */
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

#ifndef _XmAccessColorsT_H
#define _XmAccessColorsT_H

#include <Xm/Xm.h>
#include <X11/Xresource.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTaccessColors;

/* this one can be expanded in the future */
typedef struct _XmAccessColorDataRec {
    Mask  valueMask ;
    Pixel foreground  ;
    Pixel background  ;
    Pixel highlight_color  ;
    Pixel top_shadow_color  ;
    Pixel bottom_shadow_color ;
    Pixel select_color ;
} XmAccessColorDataRec, *XmAccessColorData;

typedef void (*XmAccessColorsGetProc)(Widget widget, 
				      XmAccessColorData color_data);
typedef void (*XmAccessColorsSetProc)(Widget widget, 
				      XmAccessColorData color_data);

/* Trait structures and typedefs, place typedefs first */

/* Version 0: initial release. */

typedef struct _XmAccessColorsTraitRec {
  int			version;	/* 0 */
  XmAccessColorsGetProc getColors;
  XmAccessColorsGetProc setColors;
} XmAccessColorsTraitRec, *XmAccessColorsTrait;

#define AccessColorInvalid         0L
#define AccessForeground           (1L<<0)  
#define AccessBackgroundPixel      (1L<<1)   
#define AccessHighlightColor       (1L<<2)   
#define AccessTopShadowColor       (1L<<3)   
#define AccessBottomShadowColor    (1L<<4)   
#define AccessSelectColor          (1L<<5)   

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

/* to do:

 add it to PushB/G and ToggleB/G so that they can report their
   select color
 implement the setValues ?

*/

#endif /* _XmAccessColorsT_H */

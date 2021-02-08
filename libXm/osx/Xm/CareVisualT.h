/* $XConsortium: CareVisualT.h /main/5 1995/07/15 20:48:21 drk $ */
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
#ifndef _XmCareVisualT_H
#define _XmCareVisualT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTcareParentVisual;

/* Trait structures and typedefs, place typedefs first */

typedef Boolean (*XmCareVisualRedrawProc)(Widget kid, 
					  Widget cur_parent,
					  Widget new_parent,
					  Mask visual_flag);

/* Version 0: initial release. */

typedef struct _XmCareVisualTraitRec {
  int 			 version;	/* 0 */
  XmCareVisualRedrawProc redraw;
} XmCareVisualTraitRec, *XmCareVisualTrait;


#define NoVisualChange                    0L
#define VisualForeground                  (1L<<0)  
#define VisualHighlightPixmap             (1L<<1)                              
#define VisualHighlightColor              (1L<<2)   
#define VisualBottomShadowPixmap          (1L<<3)   
#define VisualBottomShadowColor           (1L<<4)   
#define VisualTopShadowPixmap             (1L<<5)   
#define VisualTopShadowColor              (1L<<6)   
#define VisualBackgroundPixel             (1L<<7)   
#define VisualBackgroundPixmap            (1L<<8)   
#define VisualSelectColor                 (1L<<9)   


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCareVisualT_H */

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
/*   $XConsortium: VirtKeys.h /main/10 1995/07/13 18:20:33 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmVirtKeys_h
#define _XmVirtKeys_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _OSF_Keysyms
#define _OSF_Keysyms

#define osfXK_Activate		0x1004FF44
#define osfXK_AddMode		0x1004FF31
#define osfXK_BackSpace		0x1004FF08
#define osfXK_BackTab		0x1004FF07
#define osfXK_BeginData		0x1004FF5A
#define osfXK_BeginLine		0x1004FF58
#define osfXK_Cancel		0x1004FF69
#define osfXK_Clear		0x1004FF0B
#define osfXK_Copy		0x1004FF02
#define osfXK_Cut		0x1004FF03
#define osfXK_Delete		0x1004FFFF
#define osfXK_DeselectAll	0x1004FF72
#define osfXK_Down		0x1004FF54
#define osfXK_EndData		0x1004FF59
#define osfXK_EndLine		0x1004FF57
#define osfXK_Escape		0x1004FF1B
#define osfXK_Extend		0x1004FF74
#define osfXK_Help		0x1004FF6A
#define osfXK_Insert		0x1004FF63
#define osfXK_Left		0x1004FF51
#define osfXK_LeftLine		0x1004FFF8
#define osfXK_Menu		0x1004FF67
#define osfXK_MenuBar		0x1004FF45
#define osfXK_Next		0x1004FF56
#define osfXK_NextField		0x1004FF5E
#define osfXK_NextMenu		0x1004FF5C
#define osfXK_NextMinor		0x1004FFF5
#define osfXK_PageDown		0x1004FF42
#define osfXK_PageLeft		0x1004FF40
#define osfXK_PageRight		0x1004FF43
#define osfXK_PageUp		0x1004FF41
#define osfXK_Paste		0x1004FF04
#define osfXK_PrevField		0x1004FF5D
#define osfXK_PrevMenu		0x1004FF5B
#define osfXK_PrimaryPaste	0x1004FF32
#define osfXK_Prior		0x1004FF55
#define osfXK_PriorMinor	0x1004FFF6
#define osfXK_QuickPaste	0x1004FF33
#define osfXK_Reselect		0x1004FF73
#define osfXK_Restore		0x1004FF78
#define osfXK_Right		0x1004FF53
#define osfXK_RightLine		0x1004FFF7
#define osfXK_Select		0x1004FF60
#define osfXK_SelectAll		0x1004FF71
#define osfXK_SwitchDirection	0x1004FF7E
#define osfXK_Undo		0x1004FF65
#define osfXK_Up		0x1004FF52

#endif  /* OSF_Keysyms */


/********    Public Function Declarations    ********/

extern void XmTranslateKey( 
                        Display *dpy,
#if NeedWidePrototypes
                        unsigned int keycode,
#else
                        KeyCode keycode,
#endif /* NeedWidePrototypes */
                        Modifiers modifiers,
                        Modifiers *modifiers_return,
                        KeySym *keysym_return) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmVirtKeys_h */

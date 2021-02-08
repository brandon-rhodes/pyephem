/* $XConsortium: XmIm.h /main/7 1996/05/21 12:13:36 pascale $ */
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
#ifndef _XmIm_h
#define _XmIm_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Public Function Declarations    ********/

extern void XmImRegister( 
                        Widget w,
			unsigned int reserved) ;
extern void XmImUnregister( 
                        Widget w) ;
extern void XmImSetFocusValues( 
                        Widget w,
                        ArgList args,
                        Cardinal num_args) ;
extern void XmImSetValues( 
                        Widget w,
                        ArgList args,
                        Cardinal num_args) ;
extern void XmImUnsetFocus( 
                        Widget w) ;
extern XIM XmImGetXIM( 
                        Widget w) ;
extern void XmImCloseXIM(
                        Widget w) ;

extern int XmImMbLookupString( 
                        Widget w,
                        XKeyPressedEvent *event,
                        char *buf,
                        int nbytes,
                        KeySym *keysym,
                        int *status) ;
extern void XmImVaSetFocusValues( 
                        Widget w,
                        ...) ;
extern void XmImVaSetValues( 
                        Widget w,
                        ...) ;
extern XIC XmImGetXIC(
		        Widget 		w,
#if NeedWidePrototypes
		        unsigned int 	input_policy,
#else
		        XmInputPolicy	input_policy,
#endif /*NeedWidePrototypes*/
		        ArgList		args,
		        Cardinal	num_args) ;
extern XIC XmImSetXIC(
			Widget w,
			XIC    input_context) ;
extern void XmImFreeXIC(
			Widget w,
			XIC    input_context) ;

extern void XmImMbResetIC(
			Widget w,
			char **mb);

extern XIMResetState XmImGetXICResetState(
			Widget w);

/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmIm_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

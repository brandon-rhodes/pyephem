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
/*   $XConsortium: Protocols.h /main/11 1995/07/13 17:41:53 drk $ */
/*
*  (c) Copyright 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmProtocols_h
#define _XmProtocols_h

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* should be in XmP.h */

#ifndef XmCR_WM_PROTOCOLS
#define XmCR_WM_PROTOCOLS 6666
#endif /* XmCR_WM_PROTOCOLS */

/* define the XM_PROTOCOLS atom for use in  routines */
#ifdef XA_WM_PROTOCOLS
#define XM_WM_PROTOCOL_ATOM(shell) XA_WM_PROTOCOLS
#else
#define XM_WM_PROTOCOL_ATOM(shell) \
    XInternAtom(XtDisplay(shell),"WM_PROTOCOLS",FALSE)
#endif /* XA_WM_PROTOCOLS */


#define XmAddWMProtocols(shell, protocols, num_protocols) \
      XmAddProtocols(shell, XM_WM_PROTOCOL_ATOM(shell), \
			 protocols, num_protocols)

#define XmRemoveWMProtocols(shell, protocols, num_protocols) \
      XmRemoveProtocols(shell, XM_WM_PROTOCOL_ATOM(shell), \
			protocols, num_protocols)

#define XmAddWMProtocolCallback(shell, protocol, callback, closure) \
      XmAddProtocolCallback(shell, XM_WM_PROTOCOL_ATOM(shell), \
			    protocol, callback, closure)

#define XmRemoveWMProtocolCallback(shell, protocol, callback, closure) \
  XmRemoveProtocolCallback(shell, XM_WM_PROTOCOL_ATOM(shell), \
			    protocol, callback, closure)

#define XmActivateWMProtocol(shell, protocol) \
      XmActivateProtocol(shell, XM_WM_PROTOCOL_ATOM(shell), protocol)

#define XmDeactivateWMProtocol(shell, protocol) \
      XmDeactivateProtocol(shell, XM_WM_PROTOCOL_ATOM(shell), protocol)

#define XmSetWMProtocolHooks(shell, protocol, pre_h, pre_c, post_h, post_c) \
      XmSetProtocolHooks(shell, XM_WM_PROTOCOL_ATOM(shell), \
			 protocol, pre_h, pre_c, post_h, post_c)


/********    Public Function Declarations    ********/

extern void XmAddProtocols( 
                        Widget shell,
                        Atom property,
                        Atom *protocols,
                        Cardinal num_protocols) ;
extern void XmRemoveProtocols( 
                        Widget shell,
                        Atom property,
                        Atom *protocols,
                        Cardinal num_protocols) ;
extern void XmAddProtocolCallback( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom,
                        XtCallbackProc callback,
                        XtPointer closure) ;
extern void XmRemoveProtocolCallback( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom,
                        XtCallbackProc callback,
                        XtPointer closure) ;
extern void XmActivateProtocol( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom) ;
extern void XmDeactivateProtocol( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom) ;
extern void XmSetProtocolHooks( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom,
                        XtCallbackProc pre_hook,
                        XtPointer pre_closure,
                        XtCallbackProc post_hook,
                        XtPointer post_closure) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmProtocols_h */

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
/* $XConsortium: SelectioBI.h /main/5 1995/07/13 17:57:50 drk $ */
#ifndef _XmSelectioBI_h
#define _XmSelectioBI_h

#include <Xm/SelectioBP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmSelectionBoxCreateListLabel( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateSelectionLabel( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateList( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateText( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateSeparator( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateOkButton( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateApplyButton( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateCancelButton( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateHelpButton( 
                        XmSelectionBoxWidget sel) ;
extern XmGeoMatrix _XmSelectionBoxGeoMatrixCreate( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *desired) ;
extern Boolean _XmSelectionBoxNoGeoRequest( 
                        XmGeoMatrix geoSpec) ;
extern void _XmSelectionBoxGetSelectionLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetListLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetTextColumns( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetTextString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetListItems( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetListItemCount( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetListVisibleItemCount( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetOkLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetApplyLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetCancelLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetHelpLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxUpOrDown( 
                        Widget wid,
                        XEvent *event,
                        String *argv,
                        Cardinal *argc) ;
extern void _XmSelectionBoxRestore( 
                        Widget wid,
                        XEvent *event,
                        String *argv,
                        Cardinal *argc) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSelectioBI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

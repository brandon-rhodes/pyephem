/* $XConsortium: Notebook.h /main/5 1995/07/15 20:53:41 drk $ */
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

#ifndef _XmNotebook_h
#define _XmNotebook_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmNotebookWidgetClass;

typedef struct _XmNotebookClassRec *XmNotebookWidgetClass;
typedef struct _XmNotebookRec *XmNotebookWidget;


/************************************************************************
 *  Notebook Defines
 ************************************************************************/

/* XmNotebookPageStatus */
typedef enum
{
    XmPAGE_FOUND,		/* page widget found */
    XmPAGE_INVALID,		/* page number out of the range */
    XmPAGE_EMPTY,		/* no page widget found */
    XmPAGE_DUPLICATED		/* there are more than one page widgets */
} XmNotebookPageStatus;

/* Notebook page information structure */
typedef struct
{
    int         page_number;
    Widget      page_widget;
    Widget	status_area_widget;
    Widget      major_tab_widget;
    Widget	minor_tab_widget;
} XmNotebookPageInfo;


/************************************************************************
 *  Public Functions
 ************************************************************************/

#ifndef XmIsNotebook
#define XmIsNotebook(w) XtIsSubclass((w), xmNotebookWidgetClass)
#endif

extern Widget XmCreateNotebook(
			Widget		parent,
			String		name,
			ArgList		arglist,
			Cardinal	argcount);

extern XmNotebookPageStatus XmNotebookGetPageInfo(
			Widget notebook,
			int page_number,
			XmNotebookPageInfo *page_info) ;

extern Widget XmVaCreateNotebook(
                        Widget parent,
                        char *name,
                        ...);

extern Widget XmVaCreateManagedNotebook(
                        Widget parent,
                        char *name,
                        ...);


#ifdef __cplusplus
}
#endif

#endif /* _XmNotebook_h  */


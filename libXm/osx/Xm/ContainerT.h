/* $XConsortium: ContainerT.h /main/6 1996/02/09 15:05:04 drk $ */
/*
 * @OPENGROUP_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 * ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
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
 */
/*
 * HISTORY
 */

#ifndef _XmContainerT_H
#define _XmContainerT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/Xm.h>

externalref XrmQuark XmQTcontainer;

/* Trait structures and typedefs, place typedefs first */

/* this one can be expanded in the future */
typedef struct _XmContainerDataRec {
    Mask valueMask ;

    Cardinal * detail_order ;
    Cardinal detail_order_count ;
    XmTabList detail_tablist ;
    Dimension first_column_width ;
    XtEnum  selection_mode ;    /* XmNORMAL_MODE,	XmADD_MODE */
    Pixel   select_color ;

} XmContainerDataRec, *XmContainerData;

#define ContAllValid             (0xFFFF)
#define ContDetailOrder	         (1L<<0)
#define ContDetailTabList        (1L<<1)
#define ContFirstColumnWidth     (1L<<2)
#define ContSelectionMode        (1L<<3)
#define ContSelectColor          (1L<<4)


typedef void (*XmContainerGetValuesProc)(Widget w, 
					XmContainerData contData);

/* Version 0: initial release. */

typedef struct _XmContainerTraitRec {
  int			   version;	/* 0 */
  XmContainerGetValuesProc getValues;
} XmContainerTraitRec, *XmContainerTrait;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmContainerT_H */

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
/* $XConsortium: TextStrSoP.h /main/9 1995/07/13 18:11:11 drk $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTextStrSoP_h
#define _XmTextStrSoP_h

#include <Xm/XmP.h>
#include <Xm/Text.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Definitions for use by sources and source users.
 *
 ****************************************************************/

typedef struct _XmSourceDataRec {
  XmTextSource source;	/* Backpointer to source record. */
  XmTextWidget *widgets;	/* Array of widgets displaying this source. */
  XmTextPosition left, right; /* Left and right extents of selection. */
  char * ptr;			/* Actual string data. */
  char * value;		/* Value of the string data. */
  char * gap_start;		/* Gapped buffer start pointer */
  char * gap_end;		/* Gapped buffer end pointer */
  char * PSWC_NWLN;           /* Holder for char*, short*, int* rep of NWLN */
  int length;			/* Number of chars of data. */
  int maxlength;		/* Space allocated. */
  int old_length;		/* Space allocated for value pointer. */
  int numwidgets;		/* Number of entries in above. */
  int maxallowed;		/* The user is not allowed to grow source */
  /* to a size greater than this. */
  Time prim_time;             /* time of primary selection */
  Boolean hasselection;	/* Whether we own the selection. */
  Boolean editable;		/* Whether we allow any edits. */
  Boolean take_selection;	/* Whether we should take the selection. */
} XmSourceDataRec, *XmSourceData;

typedef void (*AddWidgetProc)(XmTextSource,
			      XmTextWidget);

typedef int (*CountLinesProc)(XmTextSource,
			      XmTextPosition,
			      unsigned long);

typedef void (*RemoveWidgetProc)(XmTextSource,
				 XmTextWidget);

typedef XmTextPosition (*ReadProc)(XmTextSource,
				   XmTextPosition,	/* starting position */
				   XmTextPosition,	/* The last position 
							   we're interested in.
							   Don't return info 
							   about any later
							   positions. */
				   XmTextBlock);	/* RETURN: text read */

typedef XmTextStatus (*ReplaceProc)(XmTextWidget,
				    XEvent *,
				    XmTextPosition *,
				    XmTextPosition *,
				    XmTextBlock,
#if NeedWidePrototypes
				    int);
#else
                                    Boolean);
#endif /* NeedsWidePrototypes */

typedef XmTextPosition (*ScanProc)(XmTextSource,
				   XmTextPosition,
				   XmTextScanType,
				   XmTextScanDirection,	/*XmsdLeft/XmsdRight*/
				   int,
#if NeedWidePrototypes
				   int);
#else
    		                   Boolean);
#endif /* NeedsWidePrototypes */

typedef Boolean (*GetSelectionProc)(XmTextSource,
                                    XmTextPosition *,
                                    XmTextPosition *);

typedef void (*SetSelectionProc)(XmTextSource,
				 XmTextPosition,
				 XmTextPosition,
				 Time);


typedef struct _XmTextSourceRec {
  struct _XmSourceDataRec *data;   /* Source-defined data (opaque type). */
  AddWidgetProc	AddWidget;
  CountLinesProc	CountLines;
  RemoveWidgetProc	RemoveWidget;
  ReadProc		ReadSource;
  ReplaceProc		Replace;
  ScanProc		Scan;
  GetSelectionProc	GetSelection;
  SetSelectionProc	SetSelection;
} XmTextSourceRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /*  _XmTextStrSoP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

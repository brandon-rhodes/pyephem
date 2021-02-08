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
/* $TOG: DragBSI.h /main/12 1998/03/18 15:10:55 csn $ */
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDragBSI_h
#define _XmDragBSI_h

#include <Xm/XmP.h>
#include <X11/Xmd.h>		/* for CARD32, B32, etc. */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * defalut values for XmNnumImportTargets and XmNimportTargets. 
 */
#define _XmDefaultNumImportTargets	0;
#define _XmDefaultImportTargets		NULL;


/*
 *  atoms and targets table structures
 */

typedef struct {
  Atom		atom;
  Time		time;
} xmAtomsTableEntryRec, *xmAtomsTableEntry;

typedef struct {
  Cardinal	numEntries;
  xmAtomsTableEntry entries;
} xmAtomsTableRec, *xmAtomsTable;

typedef struct {
    Cardinal	numTargets;
    Atom	*targets;
} xmTargetsTableEntryRec, *xmTargetsTableEntry;

typedef struct {
    Cardinal	numEntries;
    xmTargetsTableEntry entries;
} xmTargetsTableRec, *xmTargetsTable;

/*
 *  The following are structures for property access.
 *  They must have 64-bit multiple lengths to support 64-bit architectures.
 */

typedef struct {
    CARD32	atom B32;
    CARD16	name_length B16;
    CARD16	pad B16;
} xmMotifAtomPairRec;

typedef struct {
    BYTE	byte_order;
    BYTE	protocol_version;
    CARD16	num_atom_pairs B16;
    CARD32	heap_offset B32;
    /* xmMotifAtomPairRec 	 atomPairs[];	*/
} xmMotifAtomPairPropertyRec;

typedef struct {
    CARD32	atom B32;
    CARD32	time B32;
} xmMotifAtomsTableRec;

typedef struct {
    BYTE	byte_order;
    BYTE	protocol_version;
    CARD16	num_atoms B16;
    CARD32	heap_offset B32;
    /* xmMotifAtomsTableRec atoms[]; 	*/
} xmMotifAtomsPropertyRec;

typedef struct {
    BYTE	byte_order;
    BYTE	protocol_version;
    CARD16	num_target_lists B16;
    CARD32	heap_offset B32;
} xmMotifTargetsPropertyRec;

/********    Private Function Declarations for DragBS.c   ********/

extern void _XmInitTargetsTable( 
                        Display *display) ;
extern void _XmClearDisplayTables (Display *display);
extern Cardinal _XmIndexToTargets( 
                        Widget shell,
                        Cardinal t_index,
                        Atom **targetsRtn) ;
extern Cardinal _XmTargetsToIndex( 
                        Widget shell,
                        Atom *targets,
                        Cardinal numTargets) ;
extern Atom _XmAllocMotifAtom( 
                        Widget shell,
                        Time time) ;
extern void _XmFreeMotifAtom( 
                        Widget shell,
                        Atom atom) ;
extern void _XmDestroyMotifWindow( 
                        Display *dpy) ;
extern Window _XmGetDragProxyWindow(
			Display *display) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragBSI_h */

/* $XConsortium: TraitP.h /main/5 1995/07/15 20:56:18 drk $ */
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

#ifndef _XmTraitP_H
#define _XmTraitP_H 1

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

#define XmeTraitRemove(w, t) XmeTraitSet((XtPointer) w, t, NULL)


/********    Private Function Declarations    ********/

/*
 * XmeTraitGet(object, trait) returns a pointer to the trait_record
 * from looking up the trait on this object.  If the trait
 * is not found then NULL is returned.  This can therefore be used
 * in the following cliche'
 *
 * if (trait_rec = XmeTraitGet(XtClass(w), XmQTactivate)) {
 *   trait_rec -> activate();
 *   trait_rec -> disarm();
 * }
 */

extern XtPointer XmeTraitGet(XtPointer, XrmQuark);

/* 
 * Boolean XmeTraitSet(object, traitname, traitrecord)
 *
 * Installs the trait on the object.  Boolean will indicate
 * success of the installation.  
 * 
 * Install will use the direct pointer to traitrecord given.  The
 * implementation is therefore not allowed to use automatic
 * storage for traitrecord,  but can use malloc or static initialization
 *
 */

extern Boolean XmeTraitSet(XtPointer, XrmQuark, XtPointer);

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTraitP_H */


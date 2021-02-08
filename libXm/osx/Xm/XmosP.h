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
/* $TOG: XmosP.h /main/15 1997/03/25 14:45:55 dbl $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmosP_h
#define _XmosP_h

/* Some SVR4 systems don't have bzero. */
#include <Xm/Xmfuncs.h>		/* for bzero et al */

/*
 * Fix for 8975 - using LOGNAME instead of USER on SYSV and SVR4 
*/

#ifndef USER_VAR
#if defined(SYSV) || defined(SVR4)
#define USER_VAR "LOGNAME"
#else
#define USER_VAR "USER"
#endif
#endif

/*
 * Fix for 5222 - if NO_MEMMOVE is defined, some systems will still
 *                require stdlib.h.
 */
#ifdef NO_MEMMOVE
#ifdef bcopy
#undef bcopy
#endif
#ifdef memmove
#undef memmove
#endif
#define memmove( p1, p2, p3 )   bcopy( p2, p1, p3 )
#endif

#ifndef X_NOT_STDC_ENV
#include <stdlib.h> /* Needed for MB_CUR_MAX, mbtowc, mbstowcs and mblen */
#endif

/* On Sun systems, mblen is broken. It doesn't return 0 when the
   string is empty. Here's a patch. NOTE: On Sun systems, mblen
   is a macro wrapper around mbtowc. Hence the implementation below. */
#if defined(sun)
#undef  mblen
#define mblen(ptr, size) \
  ((ptr && *(ptr) == '\0') ? 0 : mbtowc((wchar_t *)0, (ptr), (size)))
#endif

#include <limits.h>		/* for MB_LEN_MAX et al */

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif
#ifndef LONG_MAX
#define LONG_MAX 2147483647
#endif
#ifdef BOGUS_MB_MAX  /* some systems don't properly set MB_[CUR|LEN]_MAX */
#undef MB_LEN_MAX
#define MB_LEN_MAX 1 /* temp fix for ultrix */
#undef MB_CUR_MAX
#define MB_CUR_MAX 1 /* temp fix for ultrix */
#endif /* BOGUS_MB_MAX */

/**********************************************************************/
/* here we duplicate Xtos.h, since we can't include this private file */

#if defined(INCLUDE_ALLOCA_H) || defined(HAVE_ALLOCA_H)
#include <alloca.h>
#endif

#ifdef CRAY
#define WORD64
#endif

/* Don't Use Alloca On Solaris */
#if defined(sun)
#define NO_ALLOCA
#endif

/* stolen from server/include/os.h */
#ifndef NO_ALLOCA
/*
 * os-dependent definition of local allocation and deallocation
 * If you want something other than XtMalloc/XtFree for ALLOCATE/DEALLOCATE
 * LOCAL then you add that in here.
 */
#if defined(__HIGHC__)

#if HCVERSION < 21003
#define ALLOCATE_LOCAL(size)	alloca(size)
pragma on(alloca);
#else /* HCVERSION >= 21003 */
#define	ALLOCATE_LOCAL(size)	_Alloca(size)
#endif /* HCVERSION < 21003 */

#define DEALLOCATE_LOCAL(ptr)  /* as nothing */

#endif /* defined(__HIGHC__) */


#ifdef __GNUC__

#ifndef alloca /* gnu itself might have done that already */
#define alloca __builtin_alloca
#endif

#define ALLOCATE_LOCAL(size) alloca(size)
#define DEALLOCATE_LOCAL(ptr)  /* as nothing */
#else /* ! __GNUC__ */
/*
 * warning: mips alloca is unsuitable, do not use.
 */
#if defined(vax) || defined(sun) || defined(apollo) || defined(stellar)
/*
 * Some System V boxes extract alloca.o from libPW.a; if you
 * decide that you don't want to use alloca, you might want to fix it here.
 */
char *alloca();
#define ALLOCATE_LOCAL(size) alloca(size)
#define DEALLOCATE_LOCAL(ptr)  /* as nothing */
#endif /* who does alloca */
#endif /* __GNUC__ */

#endif /* NO_ALLOCA */

#ifndef ALLOCATE_LOCAL
#define ALLOCATE_LOCAL(size)	XtMalloc(size)
#define DEALLOCATE_LOCAL(ptr)	XtFree(ptr)
#endif /* ALLOCATE_LOCAL */

/* End of Xtos.h */
/*****************/

#include <Xm/XmP.h>

/* For padding structures in Mrm we need to know how big pointers are. */
#if !defined(CRAY) && !defined(__alpha)
#define MrmShortPtr
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define MATCH_CHAR 'P'  /* referenced in InitPath strings and in the files 
			 that uses it (ImageCache.c and Mrmhier.c) */

/* OS-dependent file info for VirtKeys */

#define XMBINDDIR "XMBINDDIR"
#ifndef XMBINDDIR_FALLBACK
#define XMBINDDIR_FALLBACK "/usr/lib/Xm/bindings"
#endif
#define XMBINDFILE "xmbind.alias"
#define MOTIFBIND ".motifbind"

typedef enum {
  XmOS_METHOD_NULL,
  XmOS_METHOD_DEFAULTED,
  XmOS_METHOD_REPLACED
} XmOSMethodStatus;

typedef XmDirection (*XmCharDirectionProc)(XtPointer   /* char */,
					   XmTextType  /* type */,
					   XmStringTag /* locale */);

typedef Status  (*XmInitialDirectionProc)(XtPointer      /* chars */,
					  XmTextType     /* type */,
					  XmStringTag    /* locale */,
					  unsigned int * /* num_bytes */,
					  XmDirection *  /* direction */);


/********    Private Function Declarations    ********/

extern XmOSMethodStatus XmOSGetMethod(Widget w,
				      String method_name,
				      XtPointer * method,
				      XtPointer * os_data);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* _XmosP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

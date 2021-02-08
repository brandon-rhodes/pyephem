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
/* $XConsortium: XmosI.h /main/6 1995/07/13 18:28:56 drk $ */
#ifndef _XmosI_h
#define _XmosI_h

#include <Xm/XmosP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Vendor dependent macro for XmCvtXmStringToCT */
/* Sample implementation treats unmapped charsets as locale encoded text. */
#define _XmOSProcessUnmappedCharsetAndText(tag, ctext, sep, outc, outl, prev) \
	processCharsetAndText(XmFONTLIST_DEFAULT_TAG, (ctext), (sep), \
			      (outc), (outl), (prev))


/********    Private Function Declarations    ********/

extern String _XmOSFindPatternPart( 
                        String fileSpec) ;
extern void _XmOSQualifyFileSpec( 
                        String dirSpec,
                        String filterSpec,
                        String *pQualifiedDir,
                        String *pQualifiedPattern) ;
extern void _XmOSGetDirEntries( 
                        String qualifiedDir,
                        String matchPattern,
#if NeedWidePrototypes
                        unsigned int fileType,
                        int matchDotsLiterally,
                        int listWithFullPath,
#else
                        unsigned char fileType,
                        Boolean matchDotsLiterally,
                        Boolean listWithFullPath,
#endif /* NeedWidePrototypes */
                        String **pEntries,
                        unsigned int *pNumEntries,
                        unsigned int *pNumAlloc) ;
extern void _XmOSBuildFileList( 
                        String dirPath,
                        String pattern,
#if NeedWidePrototypes
                        unsigned int typeMask,
#else
                        unsigned char typeMask,
#endif /* NeedWidePrototypes */
                        String **pEntries,
                        unsigned int *pNumEntries,
                        unsigned int *pNumAlloc) ;
extern int _XmOSFileCompare( 
                        XmConst void *sp1,
                        XmConst void *sp2) ;
extern String _XmOSInitPath( 
                        String file_name,
                        String env_pathname,
                        Boolean *user_path) ;
extern String _XmOSBuildFileName(
			String file,
			String path) ;
extern int _XmOSPutenv(
		       char *string);
extern void _XmOSGenerateMaskName( 
				  String imageName,
				  String	maskNameBuf) ;

extern Status _XmOSGetInitialCharsDirection(XtPointer     characters,
					    XmTextType    type,
					    XmStringTag   locale,
					    unsigned int *num_bytes,
					    XmDirection  *direction) ;

extern XmDirection _XmOSGetCharDirection(XtPointer   character,
					 XmTextType  type,
					 XmStringTag locale) ;

extern int _XmOSKeySymToCharacter(KeySym keysym,
				  char	 *locale,
				  char	 *buffer);
extern void _XmOSFindPathParts(String path, 
			       String *filenameRtn, 
			       String *suffixRtn);
extern Boolean _XmOSAbsolutePathName( 
                        String path,
                        String *pathRtn,
                        String buf) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmosI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

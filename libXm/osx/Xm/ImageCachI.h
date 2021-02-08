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
/* $XConsortium: ImageCachI.h /main/7 1996/01/29 13:19:43 daniel $ */
#ifndef _XmImageCacheI_h
#define _XmImageCacheI_h

#include <Xm/XmP.h>

/* this name is used by XmeGetPixmapData to cache a pixmap in the
   pixmap cache with no associated name. _XmCachePixmap knows about it
   and will not add this one in the pixmap_data cache because it is a
   _name_ based pixmap cache used during conversion */
#define DIRECT_PIXMAP_CACHED ""

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for ImageCache.c    ********/

extern Boolean _XmInstallImage( 
                        XImage *image,
                        char *image_name,
                        int hot_x,
                        int hot_y) ;
extern Boolean _XmGetImage( 
                        Screen *screen,
                        char *image_name,
                        XImage **image) ;
extern Boolean _XmCachePixmap( 
			      Pixmap pixmap,
			      Screen *screen,
			      char *image_name,
			      Pixel foreground,
			      Pixel background,
			      int depth,
			      Dimension width,
			      Dimension height) ;
extern Pixmap _XmGetColoredPixmap(Screen *screen,
				  char *image_name,
				  XmAccessColorData acc_color,
				  int depth,
#if NeedWidePrototypes
				  int only_if_exists) ;
#else
				  Boolean only_if_exists) ;
#endif /* NeedWidePrototypes */

extern Boolean _XmGetPixmapData(
		   Screen *screen,
		   Pixmap pixmap,
		   char **image_name,
		   int *depth,
		   Pixel *foreground,
		   Pixel *background,
		   int *hot_x,
		   int *hot_y,
		   unsigned int *width,
		   unsigned int *height) ;
extern Boolean _XmInImageCache(
			       String image_name);

extern Pixmap _XmGetScaledPixmap(
    Screen *screen,
    Widget widget,
    char *image_name,
    XmAccessColorData acc_color,
    int depth,
#if NeedWidePrototypes
    int only_if_exists,
#else
    Boolean only_if_exists,
#endif /* NeedWidePrototypes */
    double scaling_ratio);

extern void _XmPutScaledImage (    
    Display*		 display ,
    Drawable		 d ,
    GC			 gc ,
    XImage*		 src_image ,
    int			 src_x ,
    int			 src_y ,
    int			 dest_x ,
    int			 dest_y ,
    unsigned int	 src_width ,
    unsigned int	 src_height, 
    unsigned int	 dest_width ,
    unsigned int	 dest_height);

extern void _XmCleanPixmapCache(Screen * screen, Widget shell);

/* for Xm.h */
extern Pixmap XmGetScaledPixmap(
    Widget widget,
    char *image_name,
    Pixel foreground,
    Pixel background,
    int depth,
    double scaling_ratio);

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmImageCacheI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

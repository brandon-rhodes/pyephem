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
/* $XConsortium: GeoUtilsI.h /main/5 1995/07/13 17:28:18 drk $ */
#ifndef _XmGeoUtilsI_h
#define _XmGeoUtilsI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for GeoUtils.c    ********/

extern XtGeometryResult _XmHandleQueryGeometry( 
                        Widget wid,
                        XtWidgetGeometry *intended,
                        XtWidgetGeometry *desired,
#if NeedWidePrototypes
                        unsigned int policy,
#else
                        unsigned char policy,
#endif /* NeedWidePrototypes */
                        XmGeoCreateProc createMatrix) ;
extern XtGeometryResult _XmHandleGeometryManager( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *desired,
                        XtWidgetGeometry *allowed,
#if NeedWidePrototypes
                        unsigned int policy,
#else
                        unsigned char policy,
#endif /* NeedWidePrototypes */
                        XmGeoMatrix *cachePtr,
                        XmGeoCreateProc createMatrix) ;
extern void _XmHandleSizeUpdate( 
                        Widget wid,
#if NeedWidePrototypes
                        unsigned int policy,
#else
                        unsigned char policy,
#endif /* NeedWidePrototypes */
                        XmGeoCreateProc createMatrix) ;
extern XmGeoMatrix _XmGeoMatrixAlloc( 
                        unsigned int numRows,
                        unsigned int numBoxes,
                        unsigned int extSize) ;
extern void _XmGeoMatrixFree( 
                        XmGeoMatrix geo_spec) ;
extern Boolean _XmGeoSetupKid( 
                        XmKidGeometry geo,
                        Widget kidWid) ;
extern void _XmGeoMatrixGet( 
                        XmGeoMatrix geoSpec,
                        int geoType) ;
extern void _XmGeoMatrixSet( 
                        XmGeoMatrix geoSpec) ;
extern void _XmGeoAdjustBoxes( 
                        XmGeoMatrix geoSpec) ;
extern void _XmGeoGetDimensions( 
                        XmGeoMatrix geoSpec) ;
extern void _XmGeoArrangeBoxes( 
                        XmGeoMatrix geoSpec,
#if NeedWidePrototypes
                        int x,
                        int y,
#else
                        Position x,
                        Position y,
#endif /* NeedWidePrototypes */
                        Dimension *pW,
                        Dimension *pH) ;
extern Dimension _XmGeoBoxesSameWidth( 
                        XmKidGeometry rowPtr,
#if NeedWidePrototypes
                        int width) ;
#else
                        Dimension width) ;
#endif /* NeedWidePrototypes */
extern Dimension _XmGeoBoxesSameHeight( 
                        XmKidGeometry rowPtr,
#if NeedWidePrototypes
                        int height) ;
#else
                        Dimension height) ;
#endif /* NeedWidePrototypes */
extern void _XmSeparatorFix( 
                        XmGeoMatrix geoSpec,
                        int action,
                        XmGeoMajorLayout layoutPtr,
                        XmKidGeometry rowPtr) ;
extern void _XmMenuBarFix( 
                        XmGeoMatrix geoSpec,
                        int action,
                        XmGeoMajorLayout layoutPtr,
                        XmKidGeometry rowPtr) ;
extern void _XmGeoLoadValues( 
                        Widget wid,
                        int geoType,
                        Widget instigator,
                        XtWidgetGeometry *request,
                        XtWidgetGeometry *geoResult) ;
extern int _XmGeoCount_kids( 
                        register CompositeWidget c) ;
extern XmKidGeometry _XmGetKidGeo( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *request,
                        int uniform_border,
#if NeedWidePrototypes
                        int border,
#else
                        Dimension border,
#endif /* NeedWidePrototypes */
                        int uniform_width_margins,
                        int uniform_height_margins,
                        Widget help,
                        int geo_type) ;
extern void _XmGeoClearRectObjAreas( 
                        RectObj r,
                        XWindowChanges *old) ;
extern void _XmSetKidGeo( 
                        XmKidGeometry kg,
                        Widget instigator) ;
extern Boolean _XmGeometryEqual( 
                        Widget wid,
                        XtWidgetGeometry *geoA,
                        XtWidgetGeometry *geoB) ;
extern Boolean _XmGeoReplyYes( 
                        Widget wid,
                        XtWidgetGeometry *desired,
                        XtWidgetGeometry *response) ;
extern XtGeometryResult _XmMakeGeometryRequest( 
                        Widget w,
                        XtWidgetGeometry *geom) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmGeoUtilsI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

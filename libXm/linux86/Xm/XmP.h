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
/*   $TOG: XmP.h /main/23 1997/09/15 14:22:29 cshi $ */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        XmP.h
 **
 **   Description: This include file contains the class and instance record
 **                definitions for all meta classes.  It also contains externs
 **                for internally shared functions and defines for internally 
 **                shared values.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _XmP_h
#define _XmP_h

#include <Xm/Xm.h>
#include <X11/IntrinsicP.h>
#include <X11/ObjectP.h>
#include <Xm/ColorP.h>
#include <Xm/AccColorT.h>


#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
 *
 *  Macros replacing toolkit macros so that gadgets are handled properly.
 * 
 ***************************************************************************/

/* Temporary hack until we can clean up our own code. ??? */
#ifndef NO_XM_1_2_XTMACROS
#define XM_1_2_XTMACROS		1
#endif

#ifdef XM_1_2_XTMACROS

/* XtClass is a macro in IntrinsicP.h, but it does no casting 
   so removing this one would certainly generate warnings everywhere, 
   we can keep it */
#ifdef XtClass
#undef XtClass
#endif
#define XtClass(widget)	(((Object)(widget))->object.widget_class)


/* Exist in IntrinsicP.h, but does no casting, so removing this
   one will probably generate a lot of warnings */
#ifdef XtParent
#undef XtParent
#endif
#define XtParent(widget) (((Object)(widget))->object.parent)


/* The following routines exist in Xt, but do not accept Gadgets. */

#ifdef XtDisplay
#undef XtDisplay
#endif
#define XtDisplay(widget) 	XtDisplayOfObject((Widget) widget)

#ifdef XtScreen
#undef XtScreen
#endif
#define XtScreen(widget) 	XtScreenOfObject((Widget) widget)

#ifdef XtWindow
#undef XtWindow
#endif
#define XtWindow(widget) 	XtWindowOfObject((Widget) widget)


/* The following macros are not provided by Xt */
#define XtX(w)		   ((w)->core.x)
#define XtY(w)		   ((w)->core.y)
#define XtWidth(w)	   ((w)->core.width)
#define XtHeight(w)	   ((w)->core.height)
#define XtBorderWidth(w)   ((w)->core.border_width)
#define XtBackground(w)	   ((w)->core.background_pixel)
#define XtCoreProc(w,proc) ((w)->core.widget_class->core_class.proc)

#endif /* XM_1_2_XTMACROS */


/***********************************************************************
 *
 * Miscellaneous SemiPrivate Defines
 *
 ***********************************************************************/

/* new for the initialized gadget checking */
#define XmNdotCache   ".cache"
#define XmCDotCache   ".Cache"

#define XmDELAYED_PIXMAP  (XmUNSPECIFIED_PIXMAP - 1)

#define XmUNSPECIFIED		(~0)
#define XmUNSPECIFIED_COUNT	(~0)


/* Used by conversion routine in ResConvert.c, RepType.c, IconG.c, etc */

#define _XM_CONVERTER_DONE( to_rtn, type, value, failure )	\
    {							\
      static type buf ;					\
							\
      if (to_rtn->addr)					\
        {						\
          if (to_rtn->size < sizeof(type))		\
            {						\
              failure					\
              to_rtn->size = sizeof(type);		\
              return FALSE;				\
            }						\
          else						\
	    {					  	\
	      *((type *) (to_rtn->addr)) = value;	\
            }						\
        }						\
      else						\
        {						\
          buf = value;					\
          to_rtn->addr = (XPointer) &buf;		\
        }						\
      to_rtn->size = sizeof(type);			\
      return TRUE;					\
    } 



/* defines needed for 3D visual enhancement of defaultButtonshadow and
 *  implementation of ToggleButton Indicatorsize. **/

#define Xm3D_ENHANCE_PIXEL		2
#define XmINDICATOR_SHADOW_THICKNESS	2

#define XmINVALID_DIMENSION		0xFFFF

/***********************************************************************
 *
 * Const stuff
 *
 ***********************************************************************/

#ifndef XmConst
#if defined(__STDC__) || !defined( NO_CONST )
#define XmConst const
#else
#define XmConst
#endif /* __STDC__ */
#endif /* XmConst */


/***********************************************************************
 *
 * Status for menus
 *
 ***********************************************************************/

/* Defines used for menu/button communication */
enum{	XmMENU_POPDOWN,			XmMENU_PROCESS_TREE,
	XmMENU_TRAVERSAL,		XmMENU_SHELL_POPDOWN,
	XmMENU_CALLBACK,		XmMENU_BUTTON,
	XmMENU_CASCADING,		XmMENU_SUBMENU,
	XmMENU_ARM,			XmMENU_DISARM,
	XmMENU_BAR_CLEANUP,		XmMENU_STATUS,
	XmMENU_MEMWIDGET_UPDATE,	XmMENU_BUTTON_POPDOWN,
	XmMENU_RESTORE_EXCLUDED_TEAROFF_TO_TOPLEVEL_SHELL,
	XmMENU_RESTORE_TEAROFF_TO_TOPLEVEL_SHELL,
	XmMENU_RESTORE_TEAROFF_TO_MENUSHELL,
	XmMENU_GET_LAST_SELECT_TOPLEVEL,
	XmMENU_TEAR_OFF_ARM
	} ;



#define XmMENU_TORN_BIT                         (1 << 0)
#define XmMENU_TEAR_OFF_SHELL_DESCENDANT_BIT    (1 << 1)
#define XmMENU_POPUP_POSTED_BIT			(1 << 2)
#define XmMENU_IN_DRAG_MODE_BIT			(1 << 3)

#define XmIsTorn(mask)				\
	(mask & XmMENU_TORN_BIT)
#define XmIsTearOffShellDescendant(mask)	\
	(mask & XmMENU_TEAR_OFF_SHELL_DESCENDANT_BIT)
#define XmPopupPosted(mask)        		\
	(mask & XmMENU_POPUP_POSTED_BIT)
#define XmIsInDragMode(mask)			\
	(mask & XmMENU_IN_DRAG_MODE_BIT)

typedef void (*XmMenuProc)( int, Widget, ...) ;

/***********************************************************************
 *
 * Simple Menu Structure
 *
 ***********************************************************************/

typedef struct _XmSimpleMenuRec {
	int count;
	int post_from_button;
	XtCallbackProc callback;
	XmStringTable label_string;
	String *accelerator;
	XmStringTable accelerator_text;
	XmKeySymTable mnemonic;
	XmStringCharSetTable mnemonic_charset;
	XmButtonTypeTable button_type;
	int button_set;
	XmString option_label;
        KeySym option_mnemonic;
} XmSimpleMenuRec, * XmSimpleMenu;


/* For MapEvent: _XmMatchBtnEvent */
#define XmIGNORE_EVENTTYPE      -1

/* Default minimum Toggle indicator dimension */
#define XmDEFAULT_INDICATOR_DIM   9





/************************************************************************
 *
 *  SyntheticP.h
 *
 ************************************************************************/

typedef enum{ XmSYNTHETIC_NONE, XmSYNTHETIC_LOAD } XmImportOperator ;

typedef void (*XmExportProc)( Widget, int, XtArgVal *) ;
typedef XmImportOperator (*XmImportProc)( Widget, int, XtArgVal *) ;

typedef struct _XmSyntheticResource
{
   String   resource_name;
   Cardinal resource_size;
   Cardinal resource_offset;
   XmExportProc export_proc;
   XmImportProc import_proc;
} XmSyntheticResource;



/***********************************************************************
 *
 *  ParProcP.h
 *
 ***********************************************************************/


typedef struct
{
   int          process_type ;  /* Common to all parent process records. */
   } XmParentProcessAnyRec ;

typedef struct
{ 
   int          process_type ;  /* Common to all parent process records. */
   XEvent *     event ;
   int          action ;
   String *     params ;
   Cardinal *   num_params ;
} XmParentInputActionRec ;

typedef union
{
   XmParentProcessAnyRec  any ;
   XmParentInputActionRec input_action ;
} XmParentProcessDataRec, * XmParentProcessData ;

enum{   XmPARENT_PROCESS_ANY,  XmINPUT_ACTION
	} ;
enum{	XmPARENT_ACTIVATE,		XmPARENT_CANCEL
	} ;
#define XmRETURN XmPARENT_ACTIVATE       /* For Motif 1.1 BC. */
#define XmCANCEL XmPARENT_CANCEL         /* For Motif 1.1 BC. */


/***********************************************************************
 *
 * BaselineP.h
 *
 ***********************************************************************/

enum{	XmBASELINE_GET,			XmBASELINE_SET
	} ;

typedef struct _XmBaselineMargins
{
  unsigned char get_or_set;
  Dimension margin_top;
  Dimension margin_bottom;
  Dimension shadow;
  Dimension highlight;
  Dimension text_height;
  Dimension margin_height;
} XmBaselineMargins;


typedef enum{ XmFOCUS_IN, XmFOCUS_OUT, XmENTER, XmLEAVE } XmFocusChange ;

typedef enum{
        XmNOT_NAVIGABLE,                XmCONTROL_NAVIGABLE,
	XmTAB_NAVIGABLE,                XmDESCENDANTS_NAVIGABLE,
	XmDESCENDANTS_TAB_NAVIGABLE
  } XmNavigability ;

/***********************************************************************
 *
 * Various proc types
 *
 ***********************************************************************/

#define XmVoidProc      XtProc


typedef Boolean (*XmParentProcessProc)( Widget, XmParentProcessData) ;
typedef void (*XmWidgetDispatchProc)( Widget, XEvent *, Mask) ;
typedef void (*XmGrabShellPopupProc)( Widget, Widget, XEvent *) ;
typedef void (*XmMenuPopupProc)( Widget, Widget, XEvent *) ;
typedef void (*XmMenuTraversalProc)( Widget, Widget, XmTraversalDirection) ;
typedef void (*XmResizeFlagProc)(
			Widget,
#if NeedWidePrototypes
			int) ;
#else
			Boolean) ;
#endif /* NeedWidePrototypes */
typedef void (*XmRealizeOutProc)( Widget, Mask *, XSetWindowAttributes *) ;
typedef Boolean (*XmVisualChangeProc)( Widget, Widget, Widget) ;
typedef void (*XmTraversalProc)( Widget, XtPointer, XtPointer, int) ;
typedef void (*XmFocusMovedProc)( Widget, XtPointer, XtPointer) ;
typedef void (*XmCacheCopyProc)( XtPointer, XtPointer, size_t) ;
typedef void (*XmGadgetCacheProc)( XtPointer) ;
typedef int (*XmCacheCompareProc)( XtPointer, XtPointer) ;
typedef Boolean (*XmWidgetBaselineProc)(Widget, Dimension **, int *);
typedef Boolean (*XmWidgetDisplayRectProc)(Widget, XRectangle *);
typedef void (*XmWidgetMarginsProc)(Widget, XmBaselineMargins *);
typedef XmNavigability (*XmWidgetNavigableProc)( Widget) ;
typedef void (*XmFocusChangeProc)(Widget, XmFocusChange);
typedef Boolean (*XmSpatialPlacementProc)(Widget, Widget, unsigned char);
typedef Boolean (*XmSpatialRemoveProc)(Widget, Widget);
typedef Boolean (*XmSpatialTestFitProc)(Widget, Widget, Position, Position);


/****************
 *
 * Data structure for building a real translation table out of a 
 * virtual string.
 *
 ****************/

typedef struct {
  Modifiers mod;
  char      *key;
  char      *action;
} _XmBuildVirtualKeyStruct;
              
typedef struct _XmKeyBindingRec
{
  KeySym	keysym;
  Modifiers	modifiers;
} XmKeyBindingRec, *XmKeyBinding;


/***********************************************************************
 *
 * Types shared by text widgets
 *
 ***********************************************************************/

typedef enum { XmsdLeft, XmsdRight } XmTextScanDirection;


/*
 * This struct is for support of Insert Selection targets.
 */
typedef struct {
    Atom selection;
    Atom target;
} _XmTextInsertPair;

typedef struct {
    XmTextPosition position;    /* Starting position. */
    XmHighlightMode mode;       /* Highlighting mode for this position. */
} _XmHighlightRec;

typedef struct {
    Cardinal number;            /* Number of different highlight areas. */
    Cardinal maximum;           /* Number we've allocated space for. */
    _XmHighlightRec *list;      /* Pointer to array of highlight data. */
} _XmHighlightData;

typedef enum { XmDEST_SELECT, XmPRIM_SELECT } XmSelectType;

typedef struct {
    Boolean done_status;	/* completion status of insert selection */
    Boolean success_status;	/* success status of insert selection */
    XmSelectType select_type;	/* insert selection type */
    XSelectionRequestEvent *event; /* event that initiated the
				      insert selection */
} _XmInsertSelect;

typedef struct {
    XEvent *event;
    String *params;
    Cardinal *num_params;
} _XmTextActionRec;

typedef struct {
    Widget widget;
    XmTextPosition insert_pos;
    int num_chars;
    Time timestamp;
    Boolean move;
} _XmTextDropTransferRec;

typedef struct {
    XmTextPosition position;
    Atom target;
    Time time;
    int num_chars;
    int ref_count;
} _XmTextPrimSelect;

typedef struct {
    Screen *screen;
    XContext context;
    unsigned char type;
} XmTextContextDataRec, *XmTextContextData;

enum {_XM_IS_DEST_CTX, _XM_IS_GC_DATA_CTX, _XM_IS_PIXMAP_CTX};

#define XmTEXT_DRAG_ICON_WIDTH	64
#define XmTEXT_DRAG_ICON_HEIGHT 64
#define XmTEXT_DRAG_ICON_X_HOT	10
#define XmTEXT_DRAG_ICON_Y_HOT	 4


/***********************************************************************
 *
 * GeoUtilsP.h
 *
 ***********************************************************************/

/* Defines used by geometry manager utilities */

enum{	XmGET_ACTUAL_SIZE = 1,		XmGET_PREFERRED_SIZE,
	XmGEO_PRE_SET,			XmGEO_POST_SET
	} ;

/* Defaults for Geometry Utility defines are always 0. */
enum{	XmGEO_EXPAND,			XmGEO_CENTER,
	XmGEO_PACK
	} ;
enum{	XmGEO_PROPORTIONAL,		XmGEO_AVERAGING,
	XmGEO_WRAP
	} ;
enum{	XmGEO_ROW_MAJOR,		XmGEO_COLUMN_MAJOR
	} ;
/* XmGEO_COLUMN_MAJOR is not yet supported. */


typedef struct _XmGeoMatrixRec *XmGeoMatrix ;
typedef union _XmGeoMajorLayoutRec *XmGeoMajorLayout ;
typedef struct _XmKidGeometryRec
{
    Widget   kid;				/* ptr to kid */
    XtWidgetGeometry	box;			/* kid geo box */
} XmKidGeometryRec, *XmKidGeometry;

typedef void (*XmGeoArrangeProc)( XmGeoMatrix,
#if NeedWidePrototypes
				 int, int,
#else
				 Position, Position,
#endif /* NeedWidePrototypes */
				 Dimension *, Dimension *) ;
typedef Boolean (*XmGeoExceptProc)( XmGeoMatrix ) ;
typedef void (*XmGeoExtDestructorProc)( XtPointer ) ;
typedef void (*XmGeoSegmentFixUpProc)( XmGeoMatrix, int, XmGeoMajorLayout,
                                                               XmKidGeometry) ;

typedef struct
{   Boolean         end ;        /* Flag to mark end of rows.                */
    XmGeoSegmentFixUpProc fix_up ;/* Used for non-ordinary layouts.          */
    Dimension       even_width ; /* If non-zero, set all boxes to same width.*/
    Dimension       even_height ;/* If non-zero, set all boxes to same height*/
    Dimension       min_height ; /* Minimum height, if stretch_height TRUE.  */
    Boolean         stretch_height ;/* Stretch height to fill vertically.    */
    Boolean         uniform_border ;/* Enforce on all kids this row, dflt F. */
    Dimension       border ;        /* Value to use if uniform_border set.   */
    unsigned char   fill_mode ; /* Possible values: XmGEO_PACK, XmGEO_CENTER,*/
				/*   or XmGEO_EXPAND (default).              */
    unsigned char   fit_mode ;  /* Method for fitting boxes into space,      */
                /* XmGEO_PROPORTIONAL (dflt), XmGEO_AVERAGING, or XmGEO_WRAP.*/
    Boolean         sticky_end ;  /* Last box in row sticks to edge, dflt F. */
    Dimension       space_above ; /* Between-line spacing, default 0.        */
    Dimension       space_end ;   /* End spacing (XmGEO_CENTER), default 0.  */
    Dimension       space_between ; /* Internal spacing, default 0.          */
    Dimension       max_box_height ;/* Set during arrange routine.           */
    Dimension       boxes_width ;   /* Set during arrange routine.           */
    Dimension       fill_width ;    /* Set during arrange routine.           */
    Dimension       box_count ;     /* Set during arrange routine.           */
    } XmGeoRowLayoutRec, *XmGeoRowLayout ;

typedef struct
{   Boolean         end ;        /* Flag to mark end of columns.             */
    XmGeoSegmentFixUpProc fix_up ;/* Used for non-ordinary layouts.          */
    Dimension       even_height ;/* If non-zero, set all boxes to same height*/
    Dimension       even_width ; /* If non-zero, set all boxes to same width.*/
    Dimension       min_width ;  /* Minimum width, if stretch_width TRUE.  */
    Boolean         stretch_width ;/* Stretch width to fill horizontally.    */
    Boolean         uniform_border ;/* Enforce on all kids this row, dflt F. */
    Dimension       border ;        /* Value to use if uniform_border set.   */
    unsigned char   fill_mode ; /* Possible values: XmGEO_PACK, XmGEO_CENTER,*/
				/*   or XmGEO_EXPAND (default).              */
    unsigned char   fit_mode ;  /* Method for fitting boxes into space,      */
                /* XmGEO_PROPORTIONAL (dflt), XmGEO_AVERAGING, or XmGEO_WRAP.*/
    Boolean         sticky_end ;  /* Last box in row sticks to edge, dflt F. */
    Dimension       space_left ;  /* Between-column spacing, default 0.      */
    Dimension       space_end ;   /* End spacing (XmGEO_CENTER), default 0.  */
    Dimension       space_between ; /* Internal spacing, default 0.          */
    Dimension       max_box_width ; /* Set during arrange routine.           */
    Dimension       boxes_height ;  /* Set during arrange routine.           */
    Dimension       fill_height ;   /* Set during arrange routine.           */
    Dimension       box_count ;     /* Set during arrange routine.           */
    } XmGeoColumnLayoutRec, *XmGeoColumnLayout ;

typedef union _XmGeoMajorLayoutRec
{
  XmGeoRowLayoutRec row ;
  XmGeoColumnLayoutRec col ;
} XmGeoMajorLayoutRec ;

typedef struct _XmGeoMatrixRec
{   Widget          composite ;     /* Widget managing layout.               */
    Widget          instigator ;    /* Widget initiating re-layout.          */
    XtWidgetGeometry instig_request ;/* Geometry layout request of instigatr.*/
    XtWidgetGeometry parent_request ;/* Subsequent layout request to parent. */
    XtWidgetGeometry *in_layout ;   /* Geo. of instig. in layout (after Get).*/
    XmKidGeometry   boxes ;/* Array of boxes, lines separated by NULL record.*/
    XmGeoMajorLayout layouts ;      /* Array of major_order format info.     */
    Dimension       margin_w ;/*Sum of margin, highlight, & shadow thickness.*/
    Dimension       margin_h ;/*Sum of margin, highlight, & shadow thickness.*/
    Boolean         stretch_boxes ; /* Set during arrange routine.           */
    Boolean         uniform_border ;/* Enforce on all kids, default FALSE.   */
    Dimension       border ;	    /* Value to use if uniform_border TRUE.  */
    Dimension       max_major ;     /* Set during arrange routine.           */
    Dimension       boxes_minor ;   /* Set during arrange routine.           */
    Dimension       fill_minor ;    /* Set during arrange routine.           */
    Dimension       width ;         /* Set during arrange routine.           */
    Dimension       height ;        /* Set during arrange routine.           */
    XmGeoExceptProc set_except ;
    XmGeoExceptProc almost_except ;
    XmGeoExceptProc no_geo_request ;
    XtPointer       extension ;
    XmGeoExtDestructorProc ext_destructor ;
    XmGeoArrangeProc arrange_boxes ;/* For user-defined arrangement routine. */
    unsigned char   major_order ;
    } XmGeoMatrixRec;

typedef XmGeoMatrix (*XmGeoCreateProc)( Widget, Widget, XtWidgetGeometry *) ;

/***********************************************************************
 *
 * XmInheritP.h
 *
 ***********************************************************************/

#define XmInheritCallbackProc ((XtCallbackProc) _XtInherit)
#define XmInheritTraversalProc ((XmTraversalProc) _XtInherit)
#define XmInheritParentProcess ((XmParentProcessProc) _XtInherit)
#define XmInheritWidgetProc ((XtWidgetProc) _XtInherit)
#define XmInheritMenuProc ((XmMenuProc) _XtInherit)
#define XmInheritTranslations XtInheritTranslations
#define XmInheritCachePart	((XmCacheClassPartPtr) _XtInherit)
#define XmInheritBaselineProc ((XmWidgetBaselineProc) _XtInherit)
#define XmInheritDisplayRectProc ((XmWidgetDisplayRectProc) _XtInherit)
#define XmInheritMarginsProc ((XmWidgetMarginsProc) _XtInherit)
#define XmInheritGeoMatrixCreate ((XmGeoCreateProc) _XtInherit)
#define XmInheritFocusMovedProc ((XmFocusMovedProc) _XtInherit)
#define XmInheritClass		   ((WidgetClass) &_XmInheritClass)
#define XmInheritInitializePrehook ((XtInitProc) _XtInherit)
#define XmInheritSetValuesPrehook  ((XtSetValuesFunc) _XtInherit)
#define XmInheritGetValuesPrehook  ((XtArgsProc) _XtInherit)
#define XmInheritInitializePosthook ((XtInitProc) _XtInherit)
#define XmInheritSetValuesPosthook  ((XtSetValuesFunc) _XtInherit)
#define XmInheritGetValuesPosthook  ((XtArgsProc) _XtInherit)
#define XmInheritSecObjectCreate   ((XtInitProc) _XtInherit)
#define XmInheritGetSecResData	   ((XmGetSecResDataFunc) _XtInherit)
#define XmInheritInputDispatch	   ((XmWidgetDispatchProc) _XtInherit)
#define XmInheritVisualChange	   ((XmVisualChangeProc) _XtInherit)
#define XmInheritArmAndActivate	   ((XtActionProc) _XtInherit)
#define XmInheritActionProc	   ((XtActionProc) _XtInherit)
#define XmInheritFocusChange       ((XmFocusChangeProc) _XtInherit)
#define XmInheritWidgetNavigable   ((XmWidgetNavigableProc) _XtInherit)
#define XmInheritClassPartInitPrehook ((XtWidgetClassProc) _XtInherit)
#define XmInheritClassPartInitPosthook ((XtWidgetClassProc) _XtInherit)
#define XmInheritBorderHighlight   ((XtWidgetProc) _XtInherit)
#define XmInheritBorderUnhighlight   ((XtWidgetProc) _XtInherit)


/************************************************************************
 *
 *  Fast subclassing macros and definitions
 *
 ************************************************************************/
/* WARNING:  Application subclasses which choose to use fast
 *           subclassing must use only those bits between
 *           192 (XmFIRST_APPLICATION_SUBCLASS_BIT) and 255.
 *           All other fast subclass bits are reserved for
 *           future use.  Use of reserved fast subclass bits
 *           will cause binary compatibility breaks with
 *           future Motif versions.
 */
#define XmFIRST_APPLICATION_SUBCLASS_BIT    192

enum{	XmCASCADE_BUTTON_BIT = 1,	XmCASCADE_BUTTON_GADGET_BIT,
	XmCOMMAND_BOX_BIT,		XmDIALOG_SHELL_BIT,
	XmLIST_BIT,			XmFORM_BIT,
	XmTEXT_FIELD_BIT,		XmGADGET_BIT,
	XmLABEL_BIT,			XmLABEL_GADGET_BIT,
	XmMAIN_WINDOW_BIT,		XmMANAGER_BIT,
	XmMENU_SHELL_BIT,		XmDRAWN_BUTTON_BIT,
	XmPRIMITIVE_BIT,		XmPUSH_BUTTON_BIT,
	XmPUSH_BUTTON_GADGET_BIT,	XmROW_COLUMN_BIT,
	XmSCROLL_BAR_BIT,		XmSCROLLED_WINDOW_BIT,
	XmSELECTION_BOX_BIT,		XmSEPARATOR_BIT,
	XmSEPARATOR_GADGET_BIT,		XmTEXT_BIT,
	XmTOGGLE_BUTTON_BIT,		XmTOGGLE_BUTTON_GADGET_BIT,
	XmDROP_TRANSFER_BIT,		XmDROP_SITE_MANAGER_BIT,
	XmDISPLAY_BIT,			XmSCREEN_BIT,
	XmPRINT_SHELL_BIT,		XmARROW_BUTTON_BIT,
	XmARROW_BUTTON_GADGET_BIT,	XmBULLETIN_BOARD_BIT,
	XmDRAWING_AREA_BIT,		XmFILE_SELECTION_BOX_BIT,
	XmFRAME_BIT,			XmMESSAGE_BOX_BIT,
	XmSASH_BIT,			XmSCALE_BIT,
	XmPANED_WINDOW_BIT,		XmVENDOR_SHELL_BIT,
	XmCLIP_WINDOW_BIT,	        XmDRAG_ICON_BIT,
	XmTEAROFF_BUTTON_BIT,		XmDRAG_OVER_SHELL_BIT,
	XmDRAG_CONTEXT_BIT,		XmCONTAINER_BIT,
	XmICONGADGET_BIT,		XmNOTEBOOK_BIT,
	XmCSTEXT_BIT,		        XmGRAB_SHELL_BIT,
	XmCOMBO_BOX_BIT,		XmSPINBOX_BIT,		
	XmICONHEADER_BIT,	

	XmFAST_SUBCLASS_TAIL_BIT /* New entries precede this. */
	} ;

#define XmLAST_FAST_SUBCLASS_BIT (XmFAST_SUBCLASS_TAIL_BIT - 1) 


#undef XmIsCascadeButton
#define XmIsCascadeButton(w)  \
  (_XmIsFastSubclass(XtClass(w), XmCASCADE_BUTTON_BIT))

#undef XmIsCascadeButtonGadget
#define XmIsCascadeButtonGadget(w)  \
  (_XmIsFastSubclass(XtClass(w), XmCASCADE_BUTTON_GADGET_BIT))

#undef XmIsClipWindow
#define XmIsClipWindow(w)  \
  (_XmIsFastSubclass(XtClass(w), XmCLIP_WINDOW_BIT))

#undef XmIsComboBox
#define XmIsComboBox(w)  \
  (_XmIsFastSubclass(XtClass(w), XmCOMBO_BOX_BIT))

#undef XmIsCommandBox
#define XmIsCommandBox(w)  \
  (_XmIsFastSubclass(XtClass(w), XmCOMMAND_BOX_BIT))

#undef XmIsContainer
#define XmIsContainer(w) \
  (_XmIsFastSubclass(XtClass(w), XmCONTAINER_BIT))

#undef XmIsDialogShell
#define XmIsDialogShell(w)  \
  (_XmIsFastSubclass(XtClass(w), XmDIALOG_SHELL_BIT))

#undef XmIsDisplay
#define XmIsDisplay(w)  \
  (_XmIsFastSubclass(XtClass(w), XmDISPLAY_BIT))

#undef XmIsGrabShell
#define XmIsGrabShell(w)  \
  (_XmIsFastSubclass(XtClass(w), XmGRAB_SHELL_BIT))

#undef XmIsIconGadget
#define XmIsIconGadget(w) \
  (_XmIsFastSubclass(XtClass(w), XmICONGADGET_BIT))

#undef XmIsList
#define XmIsList(w)  \
  (_XmIsFastSubclass(XtClass(w), XmLIST_BIT))

#undef XmIsForm
#define XmIsForm(w)  \
  (_XmIsFastSubclass(XtClass(w), XmFORM_BIT))

#undef XmIsTextField
#define XmIsTextField(w)  \
  (_XmIsFastSubclass(XtClass(w), XmTEXT_FIELD_BIT))

#undef XmIsGadget
#define XmIsGadget(w)  \
  (_XmIsFastSubclass(XtClass(w), XmGADGET_BIT))

#undef XmIsLabel
#define XmIsLabel(w)  \
  (_XmIsFastSubclass(XtClass(w), XmLABEL_BIT))

#undef XmIsLabelGadget
#define XmIsLabelGadget(w)  \
  (_XmIsFastSubclass(XtClass(w), XmLABEL_GADGET_BIT))

#undef XmIsMainWindow
#define XmIsMainWindow(w)  \
  (_XmIsFastSubclass(XtClass(w), XmMAIN_WINDOW_BIT))

#undef XmIsManager
#define XmIsManager(w)  \
  (_XmIsFastSubclass(XtClass(w), XmMANAGER_BIT))

#undef XmIsMenuShell
#define XmIsMenuShell(w)  \
  (_XmIsFastSubclass(XtClass(w), XmMENU_SHELL_BIT))

#undef XmIsNotebook
#define XmIsNotebook(w) \
  (_XmIsFastSubclass(XtClass(w), XmNOTEBOOK_BIT))

#undef XmIsDragIcon
#define XmIsDragIcon(w)  \
  (_XmIsFastSubclass(XtClass(w), XmDRAG_ICON_BIT))

#undef XmIsDropSiteManager
#define XmIsDropSiteManager(w)  \
  (_XmIsFastSubclass(XtClass(w), XmDROP_SITE_MANAGER_BIT))

#undef XmIsDropTransfer
#define XmIsDropTransfer(w)  \
  (_XmIsFastSubclass(XtClass(w), XmDROP_TRANSFER_BIT))

#undef XmIsDragOverShell
#define XmIsDragOverShell(w)  \
  (_XmIsFastSubclass(XtClass(w), XmDRAG_OVER_SHELL_BIT))

#undef XmIsDragContext
#define XmIsDragContext(w)  \
  (_XmIsFastSubclass(XtClass(w), XmDRAG_CONTEXT_BIT))

#undef XmIsDrawnButton
#define XmIsDrawnButton(w)  \
  (_XmIsFastSubclass(XtClass(w), XmDRAWN_BUTTON_BIT))

#undef XmIsPrimitive
#define XmIsPrimitive(w)  \
  (_XmIsFastSubclass(XtClass(w), XmPRIMITIVE_BIT))

#undef XmIsPushButton
#define XmIsPushButton(w)  \
  (_XmIsFastSubclass(XtClass(w), XmPUSH_BUTTON_BIT))

#undef XmIsPushButtonGadget
#define XmIsPushButtonGadget(w)  \
  (_XmIsFastSubclass(XtClass(w), XmPUSH_BUTTON_GADGET_BIT))

#undef XmIsRowColumn
#define XmIsRowColumn(w)  \
  (_XmIsFastSubclass(XtClass(w), XmROW_COLUMN_BIT))

#undef XmIsScreen
#define XmIsScreen(w)  \
  (_XmIsFastSubclass(XtClass(w), XmSCREEN_BIT))

#undef XmIsScrollBar
#define XmIsScrollBar(w)  \
  (_XmIsFastSubclass(XtClass(w), XmSCROLL_BAR_BIT))

#undef XmIsScrolledWindow
#define XmIsScrolledWindow(w)  \
  (_XmIsFastSubclass(XtClass(w), XmSCROLLED_WINDOW_BIT))

#undef XmIsSelectionBox
#define XmIsSelectionBox(w)  \
  (_XmIsFastSubclass(XtClass(w), XmSELECTION_BOX_BIT))

#undef XmIsSeparator
#define XmIsSeparator(w)  \
  (_XmIsFastSubclass(XtClass(w), XmSEPARATOR_BIT))

#undef XmIsSeparatorGadget
#define XmIsSeparatorGadget(w)  \
  (_XmIsFastSubclass(XtClass(w), XmSEPARATOR_GADGET_BIT))

#undef XmIsSpinButton
#define XmIsSpinButton(w)  \
  (_XmIsFastSubclass(XtClass(w), XmSPINBUTTON_BIT))

#undef XmIsText
#define XmIsText(w)  \
  (_XmIsFastSubclass(XtClass(w), XmTEXT_BIT))

#undef XmIsTearOffButton
#define XmIsTearOffButton(w)  \
  (_XmIsFastSubclass(XtClass(w), XmTEAROFF_BUTTON_BIT))

#undef XmIsToggleButton
#define XmIsToggleButton(w)  \
  (_XmIsFastSubclass(XtClass(w), XmTOGGLE_BUTTON_BIT))

#undef XmIsToggleButtonGadget
#define XmIsToggleButtonGadget(w)  \
  (_XmIsFastSubclass(XtClass(w), XmTOGGLE_BUTTON_GADGET_BIT))

#undef XmIsPrintShell
#define XmIsPrintShell(w)  \
  (_XmIsFastSubclass(XtClass(w), XmPRINT_SHELL_BIT))

#undef XmIsArrowButton
#define XmIsArrowButton(w)  \
  (_XmIsFastSubclass(XtClass(w), XmARROW_BUTTON_BIT))

#undef XmIsArrowButtonGadget
#define XmIsArrowButtonGadget(w)  \
  (_XmIsFastSubclass(XtClass(w), XmARROW_BUTTON_GADGET_BIT))

#undef XmIsBulletinBoard
#define XmIsBulletinBoard(w)  \
  (_XmIsFastSubclass(XtClass(w), XmBULLETIN_BOARD_BIT))

#undef XmIsDrawingArea
#define XmIsDrawingArea(w)  \
  (_XmIsFastSubclass(XtClass(w), XmDRAWING_AREA_BIT))

#undef XmIsFileSelectionBox
#define XmIsFileSelectionBox(w)  \
  (_XmIsFastSubclass(XtClass(w), XmFILE_SELECTION_BOX_BIT))

#undef XmIsFrame
#define XmIsFrame(w)  \
  (_XmIsFastSubclass(XtClass(w), XmFRAME_BIT))

#undef XmIsMessageBox
#define XmIsMessageBox(w)  \
  (_XmIsFastSubclass(XtClass(w), XmMESSAGE_BOX_BIT))

#undef XmIsSash
#define XmIsSash(w)  \
  (_XmIsFastSubclass(XtClass(w), XmSASH_BIT))

#undef XmIsScale
#define XmIsScale(w)  \
  (_XmIsFastSubclass(XtClass(w), XmSCALE_BIT))

#undef XmIsPanedWindow
#define XmIsPanedWindow(w)  \
  (_XmIsFastSubclass(XtClass(w), XmPANED_WINDOW_BIT))

#undef XmIsCSText
#define XmIsCSText(w)  \
  (_XmIsFastSubclass(XtClass(w), XmCSTEXT_BIT))


/************************************************************************
 *
 *  ResolveP.h
 *
 ************************************************************************/


/*  Widget class indices used with XmPartOffset and XmField macros  */

#define XmObjectIndex 		0
#define ObjectIndex 		XmObjectIndex
#define XmRectObjIndex		(XmObjectIndex + 1)
#define RectObjIndex		XmRectObjIndex
#define XmWindowObjIndex	(XmRectObjIndex + 1)
#define WindowObjIndex		XmWindowObjIndex
#define XmCoreIndex 		0
#define CoreIndex 		XmCoreIndex
#define XmCompositeIndex 	(XmWindowObjIndex + 2)
#define CompositeIndex 		XmCompositeIndex
#define XmConstraintIndex 	(XmCompositeIndex + 1)
#define ConstraintIndex 	XmConstraintIndex
#define XmGadgetIndex	 	(XmRectObjIndex + 1)
#define XmPrimitiveIndex 	(XmWindowObjIndex + 2)
#define XmManagerIndex	 	(XmConstraintIndex + 1)

#define XmArrowBIndex		(XmPrimitiveIndex + 1)
#define XmArrowButtonIndex	XmArrowBIndex
#define XmLabelIndex		(XmPrimitiveIndex + 1)
#define XmListIndex		(XmPrimitiveIndex + 1)
#define XmScrollBarIndex	(XmPrimitiveIndex + 1)
#define XmSeparatorIndex	(XmPrimitiveIndex + 1)
#define XmTextIndex		(XmPrimitiveIndex + 1)
#define XmTextFieldIndex	(XmPrimitiveIndex + 1)
#define XmCSTextIndex		(XmPrimitiveIndex + 1)

#define XmCascadeBIndex		(XmLabelIndex + 1)
#define XmCascadeButtonIndex	XmCascadeBIndex
#define XmDrawnBIndex		(XmLabelIndex + 1)
#define XmDrawnButtonIndex	XmDrawnBIndex
#define XmPushBIndex		(XmLabelIndex + 1)
#define XmPushButtonIndex	XmPushBIndex
#define XmToggleBIndex		(XmLabelIndex + 1)
#define XmToggleButtonIndex	XmToggleBIndex
#define XmTearOffButtonIndex	(XmPushBIndex + 1)

#define XmArrowBGIndex		(XmGadgetIndex + 1)
#define XmArrowButtonGadgetIndex XmArrowBGIndex
#define XmLabelGIndex		(XmGadgetIndex + 1)
#define XmLabelGadgetIndex	XmLabelGIndex
#define XmSeparatoGIndex	(XmGadgetIndex + 1)
#define XmSeparatorGadgetIndex	XmSeparatoGIndex

#define XmCascadeBGIndex	(XmLabelGIndex + 1)
#define XmCascadeButtonGadgetIndex XmCascadeBGIndex
#define XmPushBGIndex		(XmLabelGIndex + 1)
#define XmPushButtonGadgetIndex	XmPushBGIndex
#define XmToggleBGIndex		(XmLabelGIndex + 1)
#define XmToggleButtonGadgetIndex XmToggleBGIndex
#define XmIconGadgetIndex	(XmGadgetIndex + 1)

#define XmBulletinBIndex	(XmManagerIndex + 1)
#define XmBulletinBoardIndex	XmBulletinBIndex
#define XmDrawingAIndex		(XmManagerIndex + 1)
#define XmDrawingAreaIndex	XmDrawingAIndex
#define XmClipWindowIndex	(XmDrawingAIndex + 1)
#define XmFrameIndex		(XmManagerIndex + 1)
#define XmPanedWIndex		(XmManagerIndex + 1)
#define XmPanedWindowIndex	XmPanedWIndex
#define XmSashIndex		(XmPrimitiveIndex + 1)
#define XmRowColumnIndex	(XmManagerIndex + 1)
#define XmScaleIndex		(XmManagerIndex + 1)
#define XmScrolledWIndex	(XmManagerIndex + 1)
#define XmScrolledWindowIndex	XmScrolledWIndex

#define XmFormIndex		(XmBulletinBIndex + 1)
#define XmMessageBIndex		(XmBulletinBIndex + 1)
#define XmMessageBoxIndex	XmMessageBIndex
#define XmSelectioBIndex	(XmBulletinBIndex + 1)
#define XmSelectionBoxIndex	XmSelectioBIndex

#define XmMainWIndex		(XmScrolledWIndex + 1)
#define XmMainWindowIndex	XmMainWIndex

#define XmCommandIndex		(XmSelectioBIndex + 1)
#define XmFileSBIndex		(XmSelectioBIndex + 1)
#define XmFileSelectionBoxIndex	XmFileSBIndex

#define XmShellIndex 		(XmCompositeIndex + 1)
#define ShellIndex 		XmShellIndex
#define XmOverrideShellIndex 	(XmShellIndex + 1)
#define OverrideShellIndex 	XmOverrideShellIndex
#define XmWMShellIndex	 	(XmShellIndex + 1)
#define WMShellIndex	 	XmWMShellIndex
#define XmVendorShellIndex 	(XmWMShellIndex + 1)
#define VendorShellIndex 	XmVendorShellIndex
#define XmTransientShellIndex	(XmVendorShellIndex + 1)
#define TransientShellIndex	XmTransientShellIndex
#define XmTopLevelShellIndex 	(XmVendorShellIndex + 1)
#define TopLevelShellIndex 	XmTopLevelShellIndex
#define XmApplicationShellIndex (XmTopLevelShellIndex + 1)
#define ApplicationShellIndex 	XmApplicationShellIndex
#define XmGrabShellIndex	(XmVendorShellIndex + 1)
#define XmDisplayIndex		(XmApplicationShellIndex + 1)

#define XmDialogSIndex		(XmTransientShellIndex + 1)
#define XmDialogShellIndex	XmDialogSIndex
#define XmMenuShellIndex	(XmOverrideShellIndex + 1)

#define XmContainerIndex	(XmManagerIndex + 1)
#define XmNotebookIndex		(XmManagerIndex + 1)
#define XmSpinButtonIndex	(XmManagerIndex + 1)
#define XmComboBoxIndex		(XmManagerIndex + 1)

#define XmDragIconIndex		(XmRectObjIndex + 1)
#define XmDropSiteManagerIndex  (XmObjectIndex + 1)
#define XmDropTransferIndex	(XmObjectIndex + 1)
#define XmDragOverShellIndex	(XmVendorShellIndex + 1)
#define XmDragContextIndex	(XmCoreIndex + 1)

/* 
 * XmOFFSETBITS is the number of bits used for the part offset within the
 * resource_offset field in the XmPartResource struct.  XmOFFSETMASK is the 
 * bitmask to mask for the part offset.
 */
#define XmOFFSETBITS (sizeof(Cardinal)*8/2)
#define XmOFFSETMASK ((1<<XmOFFSETBITS)-1)

typedef struct _XmPartResource {
    String     resource_name;	/* Resource name			    */
    String     resource_class;	/* Resource class			    */
    String     resource_type;	/* Representation type desired		    */
    Cardinal   resource_size;	/* Size in bytes of representation	    */
    Cardinal   resource_offset;	/* Index within & offset within part 	    */
    String     default_type;	/* representation type of specified default */
    XtPointer  default_addr;   	/* Address of default resource		    */
} XmPartResource;

#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(__cplusplus) || defined(ANSICPP)
# define XmPartOffset(part, variable) \
        ((part##Index) << XmOFFSETBITS) + XtOffsetOf( part##Part, variable)

# define XmConstraintPartOffset(part, variable) \
        ((part##Index) << XmOFFSETBITS) + \
	XtOffsetOf( part##ConstraintPart, variable)

# define XmGetPartOffset(r, offset) \
       ((r)->resource_offset & XmOFFSETMASK) + \
	(*(offset))[(r)->resource_offset >> XmOFFSETBITS]

# define XmField(widget, offsetrecord, part, variable, type) \
	(*(type *)(((char *) (widget)) + offsetrecord[part##Index] + \
		XtOffsetOf( part##Part, variable)))

# define XmConstraintField(widget, offsetrecord, part, variable, type) \
	(*(type *)(((char *) (widget)->core.constraints) + \
	offsetrecord[part##Index] + \
	XtOffsetOf( part##ConstraintPart, variable)))
#else
# define XmPartOffset(part, variable) \
        ((part/**/Index) << XmOFFSETBITS) + XtOffsetOf( part/**/Part, variable)

# define XmConstraintPartOffset(part, variable) \
        ((part/**/Index) << XmOFFSETBITS) + \
	XtOffsetOf( part/**/ConstraintPart, variable)

# define XmGetPartOffset(r, offset) \
       ((r)->resource_offset & XmOFFSETMASK) + \
	(*(offset))[(r)->resource_offset >> XmOFFSETBITS];

# define XmField(widget, offsetrecord, part, variable, type) \
	(*(type *)(((char *) (widget)) + offsetrecord[part/**/Index] + \
	XtOffsetOf( part/**/Part, variable)))

# define XmConstraintField(widget, offsetrecord, part, variable, type) \
	(*(type *)(((char *) (widget)->core.constraints) + \
	offsetrecord[part/**/Index] + \
	XtOffsetOf( part/**/ConstraintPart, variable)))
#endif

/***********************************************************************
 *
 *  RegionP.h
 *
 *  This structure must match the opaque libX Region structure.
 ***********************************************************************/

typedef struct {
    short x1, x2, y1, y2;
} XmRegionBox;

typedef struct _XmRegion {
    long	size;
    long	numRects;
    XmRegionBox	*rects;
    XmRegionBox	extents;
} XmRegionRec, *XmRegion;


/********    ResConvert.c    ********/

enum{	XmLABEL_FONTLIST = 1,		XmBUTTON_FONTLIST,
	XmTEXT_FONTLIST
	} ;

enum {
 XmLABEL_RENDER_TABLE = 1,
 XmBUTTON_RENDER_TABLE,
 XmTEXT_RENDER_TABLE
} ;

/**** Private Defines, Typedefs, and Function Declarations for XmString.c ****/

/* For _XmStringIndexCacheTag() and _XmStringCacheTag() length. */
#define XmSTRING_TAG_STRLEN		-1

/* For _XmStringGetNextTabWidth.  EOS = End Of String. */
typedef enum { XmTAB_NEXT, XmTAB_NEWLINE, XmTAB_EOS } NextTabResult; 
  
/********    End Private Function Declarations    ********/

/********    Traversal.c    ********/

#define XmTAB_ANY	((XmNavigationType) 255)
#define XmNONE_OR_BC	((XmNavigationType) 254)

typedef struct _XmFocusMovedCallbackStruct{
    int			 reason;
    XEvent		*event;
    Boolean		 cont;
    Widget		 old_focus;
    Widget		 new_focus;
    unsigned char	 focus_policy;
    XmTraversalDirection direction; 
} XmFocusMovedCallbackStruct, *XmFocusMovedCallback;

typedef struct _XmFocusDataRec *XmFocusData;


/********    ResInd.c    ********/

typedef enum { 
  XmPARSE_ERROR, XmPARSE_NO_UNITS, XmPARSE_UNITS_OK 
} XmParseResult;



/********    Function Declarations for Xme        ********/

    /* GadgetUtil.c */
extern void XmeRedisplayGadgets( 
                        Widget w,
                        register XEvent *event,
                        Region region) ;
extern void XmeConfigureObject( 
                        Widget g,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int border_width) ;
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension border_width) ;
#endif /* NeedWidePrototypes */
    /* Traversal.c */
extern void XmeNavigChangeManaged( 
                        Widget wid) ;
extern Boolean XmeFocusIsInShell( 
                        Widget wid) ;
    /* ResInd.c */
extern XmImportOperator XmeToHorizontalPixels( 
                        Widget widget,
                        int offset,
                        XtArgVal *value) ;
extern XmImportOperator XmeToVerticalPixels( 
                        Widget widget,
                        int offset,
                        XtArgVal *value) ;
extern void XmeFromHorizontalPixels( 
                        Widget widget,
                        int offset,
                        XtArgVal *value) ;
extern void XmeFromVerticalPixels( 
                        Widget widget,
                        int offset,
                        XtArgVal *value) ;
extern XmParseResult XmeParseUnits(String spec, int *unitType);
    /* DragIcon. c */
extern Widget XmeGetTextualDragIcon(Widget w) ;
    /* BulletinB.c */
extern Widget XmeCreateClassDialog(
	                WidgetClass w_class,
			Widget ds_p,
                        String name,
                        ArgList bb_args,
                        Cardinal bb_n ) ;
    /* ImageCache.c */
extern Boolean XmeGetPixmapData( 
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
extern Pixmap XmeGetMask(
                        Screen *screen,
                        char *image_name) ;
    /* VirtKeys.c */
extern int XmeVirtualToActualKeysyms(
                         Display *dpy,
			 KeySym virtKeysym,
                         XmKeyBinding *actualKeyData) ;
    /* Screen.c */
extern Cursor XmeGetNullCursor(Widget w) ;
extern void XmeQueryBestCursorSize(
			Widget w,
			Dimension *width,
			Dimension *height );
    /* Xm.c */
extern void XmeWarning( Widget w, char *message ) ;
    /* ResConvert.c */
extern XmFontList XmeGetDefaultRenderTable(
        Widget w,
#if NeedWidePrototypes
        unsigned int fontListType );
#else
        unsigned char fontListType );
#endif /* NeedWidePrototypes */
extern Boolean XmeNamesAreEqual(
        register char *in_str,
        register char *test_str );
    /* Primitive.c */
extern void XmeResolvePartOffsets(
			WidgetClass w_class,
			XmOffsetPtr *offset,
			XmOffsetPtr *constraint_offset ) ;
    /* XmString.c */
extern Boolean XmeStringIsValid( XmString string ) ;
extern void XmeSetWMShellTitle(
			XmString xmstr,
			Widget shell) ;
extern XmIncludeStatus XmeGetDirection(XtPointer *in_out,
				       XtPointer text_end,
				       XmTextType type,
				       XmStringTag locale_tag,
				       XmParseMapping entry,
				       int pattern_length,
				       XmString *str_include,
				       XtPointer call_data);
extern XmIncludeStatus XmeGetNextCharacter(XtPointer *in_out,
					   XtPointer text_end,
					   XmTextType type,
					   XmStringTag locale_tag,
					   XmParseMapping entry,
					   int pattern_length,
					   XmString *str_include,
					   XtPointer call_data);
extern XmStringComponentType XmeStringGetComponent(_XmStringContext context, 
						   Boolean	    update_context,
						   Boolean	    copy_data,
						   unsigned int    *length,
						   XtPointer       *value);
    /* XmFontList.c */
extern Boolean XmeRenderTableGetDefaultFont(
			XmFontList fontlist,
			XFontStruct **font_struct ) ;
    /* GMUtils.c */
extern XtGeometryResult XmeReplyToQueryGeometry(
			Widget widget,
			XtWidgetGeometry * intended,
			XtWidgetGeometry * desired) ;
    /* Color.c */
extern void XmeGetDefaultPixel(
                        Widget widget,
                        int type,
                        int offset,
                        XrmValue *value) ;
    /* Xmos.c */
extern String XmeGetHomeDirName(void) ;
extern int XmeMicroSleep( 
                        long secs) ;
extern XmString XmeGetLocalizedString( 
                        char *reserved,
                        Widget widget,
                        char *resource,
                        String string) ;

/********    End Function Declarations for Xme        ********/

/********        ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif



#include <Xm/BaseClassP.h>       


/***********************************************************************
 *
 *  Motif 1.2 BC compilation.
 *
 ***********************************************************************/

#ifndef NO_XM_1_2_BC

/*
 * These routines have all been made obsolete by public Xme functions.
 * The declarations here are intended solely as an aid to porting,
 * and will be removed in a future release.  All applications should
 * name the Xme methods directly.
 *
 * _XmVirtualToActualKeysym, _XmResizeObject, and _XmMoveObject have
 * Xme counterparts with slightly different semantics or parameters,
 * so a simple rename will not work for them.
 */

#define _XmClearBorder			XmeClearBorder
#define _XmConfigureObject		XmeConfigureObject
#define _XmDrawArrow			XmeDrawArrow
#define _XmDrawDiamond			XmeDrawDiamond
#define _XmDrawSeparator		XmeDrawSeparator
#define _XmDrawShadows			XmeDrawShadows
#define _XmDrawSimpleHighlight		XmeDrawHighlight
#define _XmFontListGetDefaultFont	XmeRenderTableGetDefaultFont
#define _XmFromHorizontalPixels		XmeFromHorizontalPixels
#define _XmFromVerticalPixels		XmeFromVerticalPixels
#define _XmGMReplyToQueryGeometry	XmeReplyToQueryGeometry
#define _XmGetDefaultFontList		XmeGetDefaultRenderTable
#define _XmGetMaxCursorSize		XmeQueryBestCursorSize
#define _XmGetNullCursor		XmeGetNullCursor
#define _XmGetTextualDragIcon		XmeGetTextualDragIcon
#define _XmInputInGadget		XmObjectAtPoint
#define _XmMicroSleep			XmeMicroSleep
#define _XmNavigChangeManaged		XmeNavigChangeManaged
#define _XmOSGetHomeDirName		XmeGetHomeDirName
#define _XmOSGetLocalizedString		XmeGetLocalizedString
#define _XmRedisplayGadgets		XmeRedisplayGadgets
#define _XmStringIsXmString		XmeStringIsValid
#define _XmStringUpdateWMShellTitle	XmeSetWMShellTitle
#define _XmStringsAreEqual		XmeNamesAreEqual
#define _XmToHorizontalPixels		XmeToHorizontalPixels
#define _XmToVerticalPixels		XmeToVerticalPixels
#define _XmWarning			XmeWarning


/*
 * These routines are really undocumented and internal, but have been
 * used widely enough as data that they are preserved here for source
 * compatibility.
 */

extern void _XmDestroyParentCallback( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;


/*
 * Use of these internal macros is sufficiently widespread that they
 * are still made available here for source compatibility.
 */

/* The _XmCreateImage macro is used to create XImage with client
 * specific data for the bit and byte order.  We still have to do the
 * following because XCreateImage will stuff here display specific
 * data and we want client specific values (i.e the bit orders we used
 * for creating the bitmap data in Motif) */
#define _XmCreateImage(IMAGE, DISPLAY, DATA, WIDTH, HEIGHT, BYTE_ORDER) {\
    IMAGE = XCreateImage(DISPLAY,\
			 DefaultVisual(DISPLAY, DefaultScreen(DISPLAY)),\
			 1,\
			 XYBitmap,\
			 0,\
			 DATA,\
			 WIDTH, HEIGHT,\
			 8,\
			 (WIDTH+7) >> 3);\
    IMAGE->byte_order = BYTE_ORDER;\
    IMAGE->bitmap_unit = 8;\
    IMAGE->bitmap_bit_order = LSBFirst;\
}

#endif /* NO_XM_1_2_BC */


#endif /* _XmP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

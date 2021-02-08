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
/* $TOG: Xm.h /main/38 1999/10/18 14:50:22 samborn $
 *
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1987-1992,1996 Hewlett-Packard Company.
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */

#ifndef _Xm_h
#define _Xm_h

#if    !defined(__STDC__) \
    && !defined(__cplusplus) && !defined(c_plusplus) \
    && !defined(FUNCPROTO) && !defined(XTFUNCPROTO) && !defined(XMFUNCPROTO)
#define _NO_PROTO
#endif /* __STDC__ */

#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <X11/extensions/Print.h>
#include <Xm/XmStrDefs.h>
#include <Xm/VirtKeys.h>
#include <Xm/Transfer.h>
#include <Xm/Primitive.h>
#include <Xm/Manager.h>
#include <Xm/Gadget.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmVERSION	2
#define XmREVISION	2
#define XmUPDATE_LEVEL	3
#define XmVersion	(XmVERSION * 1000 + XmREVISION)
#define XmVERSION_STRING "@(#)Motif Version 2.2.2"

externalref int xmUseVersion;


/* define used to denote an unspecified pixmap  */

#define	XmUNSPECIFIED_PIXMAP		2

/* define for an unspecified position */

#define XmUNSPECIFIED_POSITION          -1

/*******************
 *  
 * Defines for resources to be defaulted by vendors.
 * String are initialized in Xmos.c
 *
 ****************/

#define	XmSTRING_OS_CHARSET		XmSTRING_ISO8859_1
#ifndef	XmFALLBACK_CHARSET
#define	XmFALLBACK_CHARSET		XmSTRING_ISO8859_1
#endif

#define XmDEFAULT_FONT                  _XmSDEFAULT_FONT
#define XmDEFAULT_BACKGROUND            _XmSDEFAULT_BACKGROUND
#define XmDEFAULT_DARK_THRESHOLD        20
#define XmDEFAULT_LIGHT_THRESHOLD       93
#define XmDEFAULT_FOREGROUND_THRESHOLD  70

externalref  char    _XmSDEFAULT_FONT[];            /* In Xmos.c */
externalref  char    _XmSDEFAULT_BACKGROUND[];      /* In Xmos.c */

typedef unsigned char	XmDirection;

#define XmDIRECTION_IGNORED            0x30

#define XmRIGHT_TO_LEFT_MASK           0x01 /* 0x01 for bc */
#define XmLEFT_TO_RIGHT_MASK           0x02
#define XmHORIZONTAL_MASK              0x03
#define XmTOP_TO_BOTTOM_MASK           0x04
#define XmBOTTOM_TO_TOP_MASK           0x08
#define XmVERTICAL_MASK                0x0c
#define XmPRECEDENCE_HORIZ_MASK        0x40
#define XmPRECEDENCE_VERT_MASK         0x80
#define XmPRECEDENCE_MASK              0xc0

enum {
  XmRIGHT_TO_LEFT_TOP_TO_BOTTOM = 
      XmRIGHT_TO_LEFT_MASK | XmTOP_TO_BOTTOM_MASK | XmPRECEDENCE_HORIZ_MASK,
  XmLEFT_TO_RIGHT_TOP_TO_BOTTOM = 
      XmLEFT_TO_RIGHT_MASK | XmTOP_TO_BOTTOM_MASK | XmPRECEDENCE_HORIZ_MASK,
  XmRIGHT_TO_LEFT_BOTTOM_TO_TOP = 
      XmRIGHT_TO_LEFT_MASK | XmBOTTOM_TO_TOP_MASK | XmPRECEDENCE_HORIZ_MASK,
  XmLEFT_TO_RIGHT_BOTTOM_TO_TOP = 
      XmLEFT_TO_RIGHT_MASK | XmBOTTOM_TO_TOP_MASK | XmPRECEDENCE_HORIZ_MASK,
  XmTOP_TO_BOTTOM_RIGHT_TO_LEFT = 
      XmRIGHT_TO_LEFT_MASK | XmTOP_TO_BOTTOM_MASK | XmPRECEDENCE_VERT_MASK,
  XmTOP_TO_BOTTOM_LEFT_TO_RIGHT = 
      XmLEFT_TO_RIGHT_MASK | XmTOP_TO_BOTTOM_MASK | XmPRECEDENCE_VERT_MASK,
  XmBOTTOM_TO_TOP_RIGHT_TO_LEFT = 
      XmRIGHT_TO_LEFT_MASK | XmBOTTOM_TO_TOP_MASK | XmPRECEDENCE_VERT_MASK,
  XmBOTTOM_TO_TOP_LEFT_TO_RIGHT = 
      XmLEFT_TO_RIGHT_MASK | XmBOTTOM_TO_TOP_MASK | XmPRECEDENCE_VERT_MASK,
  XmTOP_TO_BOTTOM = 
      XmTOP_TO_BOTTOM_MASK | XmHORIZONTAL_MASK | XmPRECEDENCE_MASK,
  XmBOTTOM_TO_TOP = 
      XmBOTTOM_TO_TOP_MASK | XmHORIZONTAL_MASK | XmPRECEDENCE_MASK,
  XmRIGHT_TO_LEFT = 
      XmRIGHT_TO_LEFT_MASK | XmVERTICAL_MASK | XmPRECEDENCE_MASK,
  XmLEFT_TO_RIGHT = 
      XmLEFT_TO_RIGHT_MASK | XmVERTICAL_MASK | XmPRECEDENCE_MASK,
  XmDEFAULT_DIRECTION = 0xff
  };


extern Boolean XmDirectionMatch(XmDirection d1, 
				XmDirection d2);
extern Boolean XmDirectionMatchPartial(XmDirection d1, 
				       XmDirection d2, 
				       XmDirection dmask);

/****************
 *
 * XmString structure defines. These must be here (at the start of the file) 
 * becaused they are used later on.
 *
 ****************/
typedef enum{ XmFONT_IS_FONT, XmFONT_IS_FONTSET
	      } XmFontType;

enum { XmSTRING_DIRECTION_L_TO_R,
       XmSTRING_DIRECTION_R_TO_L,
       XmSTRING_DIRECTION_UNSET = 3,
       XmSTRING_DIRECTION_DEFAULT = XmDEFAULT_DIRECTION
       };

typedef unsigned char 	XmStringDirection;
typedef union __XmStringRec	*XmString;		/* opaque to outside */
typedef XmString *	XmStringTable;		/* opaque to outside */
typedef char *		XmStringCharSet;	/* Null term string */
typedef char *		XmStringTag;		/* Null term string */
typedef unsigned char	XmStringComponentType;	/* component tags */

typedef struct __XmRenditionRec	     **XmFontListEntry; /* opaque to outside */
typedef struct __XmRenderTableRec    **XmFontList;      /* opaque to outside */
typedef struct _XmFontListContextRec *XmFontContext;   /* opaque to outside */

typedef struct __XmStringContextRec *_XmStringContext; /* opaque to outside */
typedef union __XmStringRec        *_XmString;        /* opaque to outside */
typedef struct __XmStringContextRec *XmStringContext;  /* opaque to outside */

enum{	XmSTRING_COMPONENT_UNKNOWN,	  XmSTRING_COMPONENT_CHARSET,
	XmSTRING_COMPONENT_TEXT,	  XmSTRING_COMPONENT_DIRECTION,
        XmSTRING_COMPONENT_SEPARATOR,     XmSTRING_COMPONENT_LOCALE_TEXT,
	XmSTRING_COMPONENT_LOCALE,        XmSTRING_COMPONENT_WIDECHAR_TEXT,
	XmSTRING_COMPONENT_LAYOUT_PUSH,   XmSTRING_COMPONENT_LAYOUT_POP,
	XmSTRING_COMPONENT_RENDITION_BEGIN, XmSTRING_COMPONENT_RENDITION_END,
	XmSTRING_COMPONENT_TAB
	/* 13-125 reserved */
	} ;

#define XmSTRING_COMPONENT_FONTLIST_ELEMENT_TAG	XmSTRING_COMPONENT_CHARSET

#define XmSTRING_COMPONENT_TAG		XmSTRING_COMPONENT_CHARSET

#define XmSTRING_COMPONENT_END		((XmStringComponentType) 126)

#define XmSTRING_COMPONENT_USER_BEGIN	((XmStringComponentType) 128)
			/* 128-255 are user tags */
#define XmSTRING_COMPONENT_USER_END	((XmStringComponentType) 255)

typedef enum {
    XmCHARSET_TEXT,			XmMULTIBYTE_TEXT,
    XmWIDECHAR_TEXT,                    XmNO_TEXT
    } XmTextType;

typedef enum {
    XmOUTPUT_ALL,			XmOUTPUT_BETWEEN,
    XmOUTPUT_BEGINNING,			XmOUTPUT_END,
    XmOUTPUT_BOTH
    } XmParseModel;

typedef unsigned char XmIncludeStatus;
enum {
  XmINSERT,	XmTERMINATE,	XmINVOKE
  };

/* We are making an attempt (perhaps unnecessaryily) to keep our style
   constants the same as the equivalent Xlib style constants. The first
   Motif specific style constant starts at 32 so that the consortium can
   add constants to their list without overlapping with ours. */
typedef enum {
    XmSTYLE_STRING = XStringStyle,
    XmSTYLE_COMPOUND_TEXT = XCompoundTextStyle,
    XmSTYLE_TEXT = XTextStyle,
    XmSTYLE_STANDARD_ICC_TEXT = XStdICCTextStyle,
    XmSTYLE_LOCALE = 32,
    XmSTYLE_COMPOUND_STRING
    } XmICCEncodingStyle;

/****************
 *
 * XmParseTable structure defines. These must be here (at the start of
 * the file) because they are used later on.
 *
 ****************/

typedef struct __XmParseMappingRec *XmParseMapping;	/* opaque */
typedef XmParseMapping             *XmParseTable;

/* A special pattern used to match a change of character direction. */
#define XmDIRECTION_CHANGE	NULL


typedef XmIncludeStatus (*XmParseProc) (XtPointer     *in_out,
					XtPointer      text_end,
					XmTextType     type,
					XmStringTag    locale_tag,
					XmParseMapping entry,
					int            pattern_length,
					XmString      *str_include,
					XtPointer      call_data);


/****************
 *
 * XmTabList structure defines. These must be here (at the start of the file) 
 * becaused they are used later on.
 *
 ****************/
typedef enum { 
  XmABSOLUTE,		XmRELATIVE
  } XmOffsetModel;

typedef struct __XmTabRec	*XmTab;		  /* opaque */
typedef struct __XmTabListRec	*XmTabList;	  /* opaque */


/****************
 *
 * XmRenderTable structure defines. These must be here (at the start of the file) 
 * becaused they are used later on.
 *
 ****************/
/* XmRendition declarations */
typedef struct __XmRenditionRec		**XmRendition;   /* opaque */
typedef struct __XmRenderTableRec	**XmRenderTable; /* opaque */

typedef enum { 
  XmSKIP,		XmMERGE_REPLACE,
  XmMERGE_OLD,		XmMERGE_NEW, 
  XmDUPLICATE	/* For XmFontListAdd and XmFontListAppendEntry. */
  } XmMergeMode; 

#define XmAS_IS			255
#define XmFORCE_COLOR		1

#define XmUNSPECIFIED_PIXEL	 ((Pixel) (~0))
#define XmDEFAULT_SELECT_COLOR   XmUNSPECIFIED_PIXEL	
#define XmREVERSED_GROUND_COLORS (XmDEFAULT_SELECT_COLOR - 1)	
#define XmHIGHLIGHT_COLOR        (XmREVERSED_GROUND_COLORS - 1)	

enum { XmUNSPECIFIED_LOAD_MODEL, XmLOAD_DEFERRED, XmLOAD_IMMEDIATE }; 



/************************************************************************
 *  Primitive Resources and define values
 ************************************************************************/

/* size policy values  */

enum{	XmCHANGE_ALL,			XmCHANGE_NONE,
	XmCHANGE_WIDTH,			XmCHANGE_HEIGHT
	} ;

/*  unit type values  */

enum{	XmPIXELS,			Xm100TH_MILLIMETERS,
	Xm1000TH_INCHES,		Xm100TH_POINTS,
	Xm100TH_FONT_UNITS,		XmINCHES,
        XmCENTIMETERS,                  XmMILLIMETERS,
        XmPOINTS,                       XmFONT_UNITS
	} ;

/* DeleteResponse values */

enum{	XmDESTROY,			XmUNMAP,
	XmDO_NOTHING
	} ;
enum{	XmEXPLICIT,			XmPOINTER
	} ;
/************************************************************************
 *  Navigation defines 
 ************************************************************************/

enum{	XmNONE,				XmTAB_GROUP,
	XmSTICKY_TAB_GROUP,		XmEXCLUSIVE_TAB_GROUP
	} ;

#define	XmDYNAMIC_DEFAULT_TAB_GROUP	(255)

/************************************************************************
 * Audible warning
 ************************************************************************/

enum{	/* XmNONE */			XmBELL = 1
	} ;

/************************************************************************
 * Input Manager defines
 ************************************************************************/

enum {
        XmPER_SHELL,			XmPER_WIDGET,
        XmINHERIT_POLICY = 255
	} ;

typedef unsigned char XmInputPolicy;

/************************************************************************
 *  Menu defines
 ************************************************************************/

enum{	XmNO_ORIENTATION,		XmVERTICAL,
	XmHORIZONTAL
	} ;
enum{	XmWORK_AREA,			XmMENU_BAR,
	XmMENU_PULLDOWN,		XmMENU_POPUP,
	XmMENU_OPTION
	} ;
enum{	XmNO_PACKING,			XmPACK_TIGHT,
	XmPACK_COLUMN,			XmPACK_NONE
	} ;
enum{/* XmALIGNMENT_BASELINE_TOP,	XmALIGNMENT_CENTER,
	XmALIGNMENT_BASELINE_BOTTOM, */	XmALIGNMENT_CONTENTS_TOP = 3,
	XmALIGNMENT_CONTENTS_BOTTOM
	} ;
enum{	XmTEAR_OFF_ENABLED,		XmTEAR_OFF_DISABLED
	} ;
enum{	XmUNPOST,		 	XmUNPOST_AND_REPLAY
	} ;
enum{   XmLAST_POSITION = -1,           XmFIRST_POSITION
	} ;
enum{	XmPOPUP_DISABLED = 0,	XmPOPUP_KEYBOARD = 1,
	XmPOPUP_AUTOMATIC,	XmPOPUP_AUTOMATIC_RECURSIVE };

/************************************************************************
 *  ComboBox defines
 ************************************************************************/

enum{	XmCOMBO_BOX=0,		XmDROP_DOWN_COMBO_BOX,
	XmDROP_DOWN_LIST
	} ;

enum{	/* XmNONE */ XmQUICK_NAVIGATE = 1, XmINVALID_MATCH_BEHAVIOR
	} ;

enum{   XmZERO_BASED,		XmONE_BASED
        } ;

#define XmINVALID_POSITION -1

/************************************************************************
 *  Label/Frame defines
 ************************************************************************/

enum{	XmALIGNMENT_BEGINNING,		XmALIGNMENT_CENTER,
	XmALIGNMENT_END, XmALIGNMENT_UNSPECIFIED
	} ;
enum{   XmALIGNMENT_BASELINE_TOP,    /* XmALIGNMENT_CENTER, */
	XmALIGNMENT_BASELINE_BOTTOM = 2, XmALIGNMENT_WIDGET_TOP,
	XmALIGNMENT_WIDGET_BOTTOM
   	} ;
/* new enum introduced in 2.0 to clear up the confusion in
   widget top/bottom attachment */
#define XmALIGNMENT_CHILD_TOP XmALIGNMENT_WIDGET_BOTTOM
#define XmALIGNMENT_CHILD_BOTTOM XmALIGNMENT_WIDGET_TOP

/************************************************************************
 *  Frame defines
 ************************************************************************/

enum{	XmFRAME_GENERIC_CHILD,          XmFRAME_WORKAREA_CHILD,
        XmFRAME_TITLE_CHILD
	} ;
/************************************************************************
 *  ToggleButton  defines
 ************************************************************************/

enum{	XmN_OF_MANY = 1,		XmONE_OF_MANY,
	XmONE_OF_MANY_ROUND,		XmONE_OF_MANY_DIAMOND
	} ;
/************************************************************************
 *  Form defines
 ************************************************************************/

enum{	XmATTACH_NONE,			XmATTACH_FORM,
	XmATTACH_OPPOSITE_FORM,		XmATTACH_WIDGET,
	XmATTACH_OPPOSITE_WIDGET,	XmATTACH_POSITION,
	XmATTACH_SELF
	} ;
enum{	XmRESIZE_NONE,			XmRESIZE_GROW,
	XmRESIZE_ANY
	} ;

/****************************************************************************
 *  Callback reasons 
 ****************************************************************************/

enum{	XmCR_NONE,			XmCR_HELP,
	XmCR_VALUE_CHANGED,		XmCR_INCREMENT,
	XmCR_DECREMENT,			XmCR_PAGE_INCREMENT,
	XmCR_PAGE_DECREMENT,		XmCR_TO_TOP,
	XmCR_TO_BOTTOM,			XmCR_DRAG,
	XmCR_ACTIVATE,			XmCR_ARM,
	XmCR_DISARM,			XmCR_MAP = 16,
	XmCR_UNMAP,			XmCR_FOCUS,
	XmCR_LOSING_FOCUS,		XmCR_MODIFYING_TEXT_VALUE,
	XmCR_MOVING_INSERT_CURSOR,	XmCR_EXECUTE,
	XmCR_SINGLE_SELECT,		XmCR_MULTIPLE_SELECT,
	XmCR_EXTENDED_SELECT,		XmCR_BROWSE_SELECT,
	XmCR_DEFAULT_ACTION,		XmCR_CLIPBOARD_DATA_REQUEST,
	XmCR_CLIPBOARD_DATA_DELETE,	XmCR_CASCADING,
	XmCR_OK,			XmCR_CANCEL,
	XmCR_APPLY = 34,		XmCR_NO_MATCH,
	XmCR_COMMAND_ENTERED,		XmCR_COMMAND_CHANGED,
	XmCR_EXPOSE,			XmCR_RESIZE,
	XmCR_INPUT,			XmCR_GAIN_PRIMARY,
	XmCR_LOSE_PRIMARY,		XmCR_CREATE,
	XmCR_TEAR_OFF_ACTIVATE,		XmCR_TEAR_OFF_DEACTIVATE,
	XmCR_OBSCURED_TRAVERSAL,	XmCR_FOCUS_MOVED,
	XmCR_REPOST = 54,		XmCR_COLLAPSED,
	XmCR_EXPANDED,			XmCR_SELECT,
	XmCR_DRAG_START,		XmCR_NO_FONT,
	XmCR_NO_RENDITION,		XmCR_POST,
	XmCR_SPIN_NEXT,			XmCR_SPIN_PRIOR,
	XmCR_SPIN_FIRST,		XmCR_SPIN_LAST,
	XmCR_PAGE_SCROLLER_INCREMENT,   XmCR_PAGE_SCROLLER_DECREMENT,
	XmCR_MAJOR_TAB,                 XmCR_MINOR_TAB,
	XmCR_START_JOB,			XmCR_END_JOB,
	XmCR_PAGE_SETUP,		XmCR_PDM_NONE,
	XmCR_PDM_UP,			XmCR_PDM_START_ERROR,
	XmCR_PDM_START_VXAUTH,		XmCR_PDM_START_PXAUTH, 
	XmCR_PDM_OK,			XmCR_PDM_CANCEL,
	XmCR_PDM_EXIT_ERROR,
        XmCR_UPDATE_SHELL,              XmCR_UPDATE_TEXT,
        XmCR_VERIFY_TEXT,               XmCR_VERIFY_TEXT_FAILED,
        XmCR_ENTER_CHILD,               XmCR_LEAVE_CHILD,
	XmCR_PROTOCOLS = 6666 /* required for BC. See CR 9158 */
	} ;

/************************************************************************
 *  new ScrollBar showArrows  define
 ************************************************************************/

enum{	/* XmNONE */		XmEACH_SIDE = 1,
	XmMAX_SIDE,             XmMIN_SIDE
	} ;


/************************************************************************
 *  Sliding mode
 ************************************************************************/

enum{	XmSLIDER,		XmTHERMOMETER} ;


/************************************************************************
 *  Slider Visual
 ************************************************************************/

enum{	XmBACKGROUND_COLOR,     XmFOREGROUND_COLOR, 
        XmTROUGH_COLOR,         XmSHADOWED_BACKGROUND} ;


/************************************************************************
 *  Slider Mark
 ************************************************************************/

enum{	/* XmNONE, */           XmETCHED_LINE = 1, 
        XmTHUMB_MARK,     XmROUND_MARK } ;


/************************************************************************
 *  new Scale showValue 
 ************************************************************************/

enum{	/* XmNONE */		XmNEAR_SLIDER = 1,
	XmNEAR_BORDER
	} ;


/************************************************************************
 *  new ScrolledWindow/MainWindow chidType
 ************************************************************************/

/* XmWORK_AREA, XmMENU_BAR and XmSEPARATOR have to match the existing ones */
enum{	/* XmWORK_AREA = 0, XmMENU_BAR = 1, */ 
        XmHOR_SCROLLBAR = 2, 
	XmVERT_SCROLLBAR,
	XmCOMMAND_WINDOW, 
        /* XmSEPARATOR = 5 */
        XmMESSAGE_WINDOW = 6,
        XmSCROLL_HOR, XmSCROLL_VERT, XmNO_SCROLL,
	XmCLIP_WINDOW, XmGENERIC_CHILD
	} ;

/************************************************************************
 *  new ScrolledWindow auto drag enum
 ************************************************************************/

enum{	XmAUTO_DRAG_ENABLED,		XmAUTO_DRAG_DISABLED
	} ;

/************************************************************************
 *  new Display enable warp enum
 ************************************************************************/

enum{	XmENABLE_WARP_ON,		XmENABLE_WARP_OFF
	} ;

/************************************************************************
 *  new Display enable btn1 transfer enum
 ************************************************************************/

enum{   XmOFF,				XmBUTTON2_ADJUST,
	XmBUTTON2_TRANSFER };

/************************************************************************
 * auto_selection_type
 ************************************************************************/

enum{   XmAUTO_UNSET,                   XmAUTO_BEGIN,
        XmAUTO_MOTION,                  XmAUTO_CANCEL,
        XmAUTO_NO_CHANGE,               XmAUTO_CHANGE
        };

/************************************************************************
 *  Callback structures 
 ************************************************************************/

typedef struct
{
    int     reason;
    XEvent  *event;
} XmAnyCallbackStruct;

typedef struct
{
    int     reason;
    XEvent  *event;
    int	    click_count;
} XmArrowButtonCallbackStruct;

typedef struct _XmDragStartCallbackStruct {
    int                 reason;
    XEvent              *event;
    Widget              widget;
    Boolean             doit;
} XmDragStartCallbackStruct, *XmDragStartCallback;

typedef struct
{
    int     reason;
    XEvent  *event;
    XmString item_or_text;
    int     item_position;
} XmComboBoxCallbackStruct;

typedef struct
{
    int     reason;
    XEvent  *event;
    Window  window;
} XmDrawingAreaCallbackStruct;

typedef struct
{
    int     reason;
    XEvent  *event;
    Window  window;
    int	    click_count;
} XmDrawnButtonCallbackStruct;

typedef struct
{
    int     reason;
    XEvent  *event;
    int	    click_count;
} XmPushButtonCallbackStruct;

typedef struct
{
    int     reason;
    XEvent  *event;
    Widget  widget;
    char    *data;
    char    *callbackstruct;
} XmRowColumnCallbackStruct;

typedef struct
{
   int reason;
   XEvent * event;
   int value;
   int pixel;
} XmScrollBarCallbackStruct;

typedef struct
{
   int reason;
   XEvent * event;
   int set;
} XmToggleButtonCallbackStruct;

typedef struct
{
   int 	     reason;
   XEvent    *event;
   XmString  item;
   int       item_length;
   int       item_position;
   XmString  *selected_items;
   int       selected_item_count;
   int       *selected_item_positions;
   char      selection_type;
   char	     auto_selection_type;
} XmListCallbackStruct;

typedef struct
{
    int reason;
    XEvent	*event;
    XmString	value;
    int		length;
} XmSelectionBoxCallbackStruct;

typedef struct
{
    int reason;
    XEvent	*event;
    XmString	value;
    int		length;
} XmCommandCallbackStruct;

typedef struct
{
    int 	reason;
    XEvent	*event;
    XmString	value;
    int		length;
    XmString	mask;
    int		mask_length;
    XmString	dir ;
    int		dir_length ;
    XmString    pattern ;
    int		pattern_length ;
} XmFileSelectionBoxCallbackStruct;


typedef struct 
{
   int reason;
   XEvent * event;
   int value;
} XmScaleCallbackStruct;

typedef struct
{
  int      reason;
  XEvent   *event;
  Widget   menuToPost;
  Boolean  postIt;
  Widget   target;
} XmPopupHandlerCallbackStruct;

typedef struct
{
  int         reason;
  XEvent      *event;
  Widget      item;
  unsigned char       new_outline_state;
} XmContainerOutlineCallbackStruct;

typedef struct
{
  int             reason;
  XEvent          *event;
  WidgetList      selected_items;
  int             selected_item_count;
  unsigned char   auto_selection_type;
} XmContainerSelectCallbackStruct;

typedef struct
{
  int         reason;
  XEvent      *event;
  int         page_number;
  Widget      page_widget;
  int         prev_page_number;
  Widget      prev_page_widget;
} XmNotebookCallbackStruct;

typedef struct
{
    int     		reason;
    XEvent  		*event;
    XmRendition		rendition;
    char		*font_name;
    XmRenderTable	render_table;
    XmStringTag		tag;
} XmDisplayCallbackStruct;

typedef struct
{
    int		reason;	   /* XmCR_START_JOB, XmCR_END_JOB, XmCR_PAGE_SETUP */
    XEvent	*event;
    XPContext	context;
    Boolean	last_page; /* in_out */
    XtPointer	detail;
} XmPrintShellCallbackStruct;


/************************************************************************
 *  PushButton defines
 ************************************************************************/

enum{	XmMULTICLICK_DISCARD,		XmMULTICLICK_KEEP
	} ;
/************************************************************************
 *  DrawnButton defines
 ************************************************************************/

enum{	XmSHADOW_IN = 7,		XmSHADOW_OUT
	} ;
/************************************************************************
 *  Arrow defines
 ************************************************************************/

enum{	XmARROW_UP,			XmARROW_DOWN,
	XmARROW_LEFT,			XmARROW_RIGHT
	} ;
/************************************************************************
 *  Separator defines
 *  Note: XmINVALID_SEPARATOR_TYPE marks the last+1 separator type
 ************************************************************************/

enum{	XmNO_LINE,			XmSINGLE_LINE,
	XmDOUBLE_LINE,			XmSINGLE_DASHED_LINE,
	XmDOUBLE_DASHED_LINE,		XmSHADOW_ETCHED_IN,
	XmSHADOW_ETCHED_OUT,		XmSHADOW_ETCHED_IN_DASH,
	XmSHADOW_ETCHED_OUT_DASH,	XmINVALID_SEPARATOR_TYPE
	} ;

enum{	XmPIXMAP = 1,			XmSTRING
	} ;

/************************************************************************
 *  Drag and Drop defines
 ************************************************************************/

enum{	XmWINDOW,		     /* XmPIXMAP, */
	XmCURSOR = 2,		     XmDRAG_WINDOW = 3
	} ;

/************************************************************************
 *  ScrollBar defines
 ************************************************************************/

enum{	XmMAX_ON_TOP,			XmMAX_ON_BOTTOM,
	XmMAX_ON_LEFT,			XmMAX_ON_RIGHT
	} ;
/************************************************************************
 *									*
 * List Widget defines							*
 *									*
 ************************************************************************/

enum{	XmSINGLE_SELECT,		XmMULTIPLE_SELECT,
	XmEXTENDED_SELECT,		XmBROWSE_SELECT
	} ;
enum{	XmSTATIC,			XmDYNAMIC
	} ;
enum{ XmNORMAL_MODE,			XmADD_MODE
        } ;
/************************************************************************
 *                                                                      *
 * Container Widget defines                                             *
 *                                                                      *
 ************************************************************************/

        /* XmRAutomaticSelection */
enum {  XmNO_AUTO_SELECT,
        XmAUTO_SELECT
        };

        /* XmRLineStyle */
enum {  /* XmNO_LINE */
        XmSINGLE = 1
        };

        /* XmREntryViewType */
enum {  /* XmLARGE_ICON */
        /* XmSMALL_ICON */
        XmANY_ICON = 2
        };

        /* XmRSpatialIncludeModel */
enum {  XmAPPEND,
        XmCLOSEST,
        XmFIRST_FIT
        };

        /* XmRLayoutType */
enum {  XmOUTLINE,
        XmSPATIAL,
	XmDETAIL
        };

	/* XmNoutlineButtonPolicy */
enum {	XmOUTLINE_BUTTON_PRESENT,
	XmOUTLINE_BUTTON_ABSENT
	};

        /* XmRSpatialPlaceStyle */
enum {  /* XmNONE */
	XmGRID = 1,
	XmCELLS
        };

	/* XmRPrimaryOwnership */
enum {	XmOWN_NEVER,
	XmOWN_ALWAYS,
	XmOWN_MULTIPLE,
	XmOWN_POSSIBLE_MULTIPLE
	};

        /* XmRSpatialResizeModel */
enum {  XmGROW_MINOR,
	XmGROW_MAJOR,
        XmGROW_BALANCED
        };

        /* XmRSelectionTechnique */
enum {  XmMARQUEE,
        XmMARQUEE_EXTEND_START,
        XmMARQUEE_EXTEND_BOTH,
        XmTOUCH_ONLY,
        XmTOUCH_OVER
        };

        /* XmRSpatialSnapModel */
enum {  /* XmNONE */
        XmSNAP_TO_GRID = 1,
        XmCENTER
        };

        /* XmROutlineState */
enum {  XmCOLLAPSED,
        XmEXPANDED
        };

/************************************************************************
 *                                                                      *
 * IconGadget defines                                                   *
 *                                                                      *
 ************************************************************************/

        /* XmRViewType */
enum {  XmLARGE_ICON,
        XmSMALL_ICON
        };

        /* XmRVisualEmphasis */
enum {  XmSELECTED,
        XmNOT_SELECTED
        };

/************************************************************************
 *                                                                      *
 * Notebook Widget defines                                              *
 *                                                                      *
 ************************************************************************/

#define XmUNSPECIFIED_PAGE_NUMBER       (-32768)

        /* XmRBindingType */
enum {  /* XmNONE */
        /* XmPIXMAP */
        XmSOLID = 2,
        XmSPIRAL,
        XmPIXMAP_OVERLAP_ONLY
        };

        /* XmRNBChildType */
enum {  /* XmNONE */
        XmPAGE = 1,
        XmMAJOR_TAB,
        XmMINOR_TAB,
        XmSTATUS_AREA,
        XmPAGE_SCROLLER
        };

/************************************************************************
 *									*
 * Spin button defines.							*
 *									*
 ************************************************************************/

/* XmNarrowOrientation */
enum
{
    XmARROWS_VERTICAL,
    XmARROWS_HORIZONTAL
};

/* XmNarrowLayout */
enum
{
    XmARROWS_END,
    XmARROWS_BEGINNING,
    XmARROWS_SPLIT,
    XmARROWS_FLAT_END,
    XmARROWS_FLAT_BEGINNING
};

/* XmNarrowSensitivity and XmNdefaultArrowSensitivity */
/* Please note that these arrows form the proper values 
   for a bit mask. */
enum
{
    XmARROWS_INSENSITIVE,
    XmARROWS_INCREMENT_SENSITIVE,
    XmARROWS_DECREMENT_SENSITIVE,
    XmARROWS_SENSITIVE,
    XmARROWS_DEFAULT_SENSITIVITY
};

/* XmNpositionType */
enum
{
    XmPOSITION_INDEX,
    XmPOSITION_VALUE
};

/* XmNspinButtonChildType */
enum
{
    /* XmPIXMAP = 1 */
    /* XmSTRING */
    XmNUMERIC = 3
};

/* Return values for Xm[Simple]SpinBoxValidatePosition */
enum
{
    XmVALID_VALUE,
    XmCURRENT_VALUE,
    XmMAXIMUM_VALUE,
    XmMINIMUM_VALUE,
    XmINCREMENT_VALUE
};

typedef struct
{
    int		reason;
    XEvent	*event;
    Widget      widget;
    Boolean     doit;
    int		position;
    XmString	value;
    Boolean	crossed_boundary;
} XmSpinBoxCallbackStruct;


/************************************************************************
 *									*
 * Scrolled Window defines.						*
 *									*
 ************************************************************************/

enum{	XmVARIABLE,			XmCONSTANT,
	XmRESIZE_IF_POSSIBLE
	} ;
enum{	XmAUTOMATIC,			XmAPPLICATION_DEFINED
	} ;
enum{	/* XmSTATIC */			XmAS_NEEDED = 1
	} ;

#define SW_TOP		1
#define SW_BOTTOM	0
#define SW_LEFT		2
#define SW_RIGHT	0

#define XmTOP_LEFT	(SW_TOP | SW_LEFT)
#define XmBOTTOM_LEFT	(SW_BOTTOM  | SW_LEFT)
#define XmTOP_RIGHT	(SW_TOP | SW_RIGHT)
#define XmBOTTOM_RIGHT	(SW_BOTTOM  | SW_RIGHT)

/************************************************************************
 *									*
 * MainWindow Resources                                                 *
 *									*
 ************************************************************************/

enum{	XmCOMMAND_ABOVE_WORKSPACE,	XmCOMMAND_BELOW_WORKSPACE
	} ;
/************************************************************************
 *									*
 * Text Widget defines							*
 *									*
 ************************************************************************/

enum{	XmMULTI_LINE_EDIT,		XmSINGLE_LINE_EDIT
	} ;

typedef enum{
	XmTEXT_FORWARD,
	XmTEXT_BACKWARD
	} XmTextDirection;

typedef long XmTextPosition;
typedef Atom XmTextFormat;

#define XmFMT_8_BIT	((XmTextFormat) XA_STRING)	/* 8-bit text. */
#define XmFMT_16_BIT	((XmTextFormat) 2)		/* 16-bit text. */

#define FMT8BIT		XmFMT_8_BIT	/* For backwards compatibility only.*/
#define FMT16BIT	XmFMT_16_BIT	/* For backwards compatibility only.*/

typedef enum{
	XmSELECT_POSITION,		XmSELECT_WHITESPACE,
	XmSELECT_WORD,			XmSELECT_LINE,
	XmSELECT_ALL,			XmSELECT_PARAGRAPH,
	XmSELECT_OUT_LINE
	} XmTextScanType ;

typedef enum{
	XmHIGHLIGHT_NORMAL,		XmHIGHLIGHT_SELECTED,
	XmHIGHLIGHT_SECONDARY_SELECTED,	XmSEE_DETAIL
	} XmHighlightMode ;

/* XmTextBlock's are used to pass text around. */

typedef struct {
    char *ptr;                  /* Pointer to data. */
    int length;                 /* Number of bytes of data. */
    XmTextFormat format;       /* Representations format */
} XmTextBlockRec, *XmTextBlock;

typedef struct
{
    int reason;
    XEvent  *event;
    Boolean doit;
    long currInsert, newInsert;
    long startPos, endPos;
    XmTextBlock text;
} XmTextVerifyCallbackStruct, *XmTextVerifyPtr;

/* XmTextBlockWcs's are used in 1.2 modifyVerifyWcs callbacks for Text[Field]
 * widgets. */

typedef struct {
    wchar_t *wcsptr;            /* Pointer to data. */
    int length;                 /* Number of characters (not bytes) of data. */
} XmTextBlockRecWcs, *XmTextBlockWcs;

typedef struct
{
    int reason;
    XEvent  *event;
    Boolean doit;
    long currInsert, newInsert;
    long startPos, endPos;
    XmTextBlockWcs text;
} XmTextVerifyCallbackStructWcs, *XmTextVerifyPtrWcs;

/* functions renamed after 1.0 release due to resource name overlap */
#define XmTextGetTopPosition                XmTextGetTopCharacter
#define XmTextSetTopPosition                XmTextSetTopCharacter

#define XmCOPY_FAILED		0
#define XmCOPY_SUCCEEDED	1
#define XmCOPY_TRUNCATED	2

/************************************************************************
 *									*
 *  DIALOG defines..  BulletinBoard and things common to its subclasses *
 *          CommandBox    MessageBox    Selection    FileSelection      *
 *									*
 ************************************************************************/

/* child type defines for Xm...GetChild() */

enum{	XmDIALOG_NONE,			XmDIALOG_APPLY_BUTTON,
	XmDIALOG_CANCEL_BUTTON,		XmDIALOG_DEFAULT_BUTTON,
	XmDIALOG_OK_BUTTON,		XmDIALOG_FILTER_LABEL,
	XmDIALOG_FILTER_TEXT,		XmDIALOG_HELP_BUTTON,
	XmDIALOG_LIST,			XmDIALOG_LIST_LABEL,
	XmDIALOG_MESSAGE_LABEL,		XmDIALOG_SELECTION_LABEL,
	XmDIALOG_SYMBOL_LABEL,		XmDIALOG_TEXT,
	XmDIALOG_SEPARATOR,		XmDIALOG_DIR_LIST,
	XmDIALOG_DIR_LIST_LABEL
	} ;

#define XmDIALOG_HISTORY_LIST     	XmDIALOG_LIST
#define XmDIALOG_PROMPT_LABEL     	XmDIALOG_SELECTION_LABEL
#define XmDIALOG_VALUE_TEXT       	XmDIALOG_TEXT
#define XmDIALOG_COMMAND_TEXT     	XmDIALOG_TEXT
#define XmDIALOG_FILE_LIST        	XmDIALOG_LIST
#define XmDIALOG_FILE_LIST_LABEL  	XmDIALOG_LIST_LABEL

/*  dialog style defines  */

enum{	XmDIALOG_MODELESS,		XmDIALOG_PRIMARY_APPLICATION_MODAL,
	XmDIALOG_FULL_APPLICATION_MODAL,XmDIALOG_SYSTEM_MODAL
	} ;

/* The following is for compatibility only. Its use is deprecated.
 */
#define XmDIALOG_APPLICATION_MODAL	XmDIALOG_PRIMARY_APPLICATION_MODAL

/************************************************************************
 * XmSelectionBox, XmFileSelectionBox and XmCommand - misc. stuff       *
 ***********************************************************************/

/* Defines for Selection child placement
*/
enum{	XmPLACE_TOP,			XmPLACE_ABOVE_SELECTION,
	XmPLACE_BELOW_SELECTION
	} ;

/* Defines for file type mask:
*/
#define XmFILE_DIRECTORY (1 << 0)
#define XmFILE_REGULAR   (1 << 1)
#define XmFILE_ANY_TYPE  (XmFILE_DIRECTORY | XmFILE_REGULAR)

/* Defines for selection dialog type:
*/
enum{	XmDIALOG_WORK_AREA,		XmDIALOG_PROMPT,
	XmDIALOG_SELECTION,		XmDIALOG_COMMAND,
	XmDIALOG_FILE_SELECTION
	} ;

/************************************************************************
 *  XmMessageBox           stuff not common to other dialogs            *
 ***********************************************************************/

/* defines for dialog type */

enum{	XmDIALOG_TEMPLATE,		XmDIALOG_ERROR,
	XmDIALOG_INFORMATION,		XmDIALOG_MESSAGE,
	XmDIALOG_QUESTION,		XmDIALOG_WARNING,
	XmDIALOG_WORKING
	} ;

/*  Traversal types  */

typedef enum{
	XmVISIBILITY_UNOBSCURED,	XmVISIBILITY_PARTIALLY_OBSCURED,
	XmVISIBILITY_FULLY_OBSCURED
	} XmVisibility ;


typedef enum{
	XmTRAVERSE_CURRENT,		XmTRAVERSE_NEXT,
	XmTRAVERSE_PREV,		XmTRAVERSE_HOME,
	XmTRAVERSE_NEXT_TAB_GROUP,	XmTRAVERSE_PREV_TAB_GROUP,
	XmTRAVERSE_UP,			XmTRAVERSE_DOWN,
	XmTRAVERSE_LEFT,		XmTRAVERSE_RIGHT,
	XmTRAVERSE_GLOBALLY_FORWARD,	XmTRAVERSE_GLOBALLY_BACKWARD
	} XmTraversalDirection ;

typedef struct _XmTraverseObscuredCallbackStruct
{	int			reason ;
	XEvent *		event ;
	Widget			traversal_destination ;
	XmTraversalDirection	direction ;
	} XmTraverseObscuredCallbackStruct ;

typedef unsigned char   XmNavigationType;


/***********************************************************************
 *
 * SimpleMenu declarations and definitions.
 *
 ***********************************************************************/

typedef unsigned char XmButtonType;
typedef XmButtonType * XmButtonTypeTable;
typedef KeySym * XmKeySymTable;
typedef XmStringCharSet * XmStringCharSetTable;

enum{	XmPUSHBUTTON = 1,		XmTOGGLEBUTTON,
	XmRADIOBUTTON,			XmCASCADEBUTTON,
	XmSEPARATOR,			XmDOUBLE_SEPARATOR,
	XmTITLE
	} ;
#define XmCHECKBUTTON			XmTOGGLEBUTTON


/***********************************************************************
 *
 * BitmapConversionModel
 *
 ***********************************************************************/

enum{	XmMATCH_DEPTH, XmDYNAMIC_DEPTH } ;


/************************************************************************
 *  PrintShell defines
 ************************************************************************/

enum { XmPDM_NOTIFY_FAIL, XmPDM_NOTIFY_SUCCESS } ;


/* This one cannot be put at the beginning because it needs 
   XmStringTable */
#include <Xm/TxtPropCv.h>


/********    BaseClass.c    ********/
typedef XtPointer	(*XmResourceBaseProc)( Widget, XtPointer) ;

typedef struct _XmSecondaryResourceDataRec{
    XmResourceBaseProc	base_proc;
    XtPointer		client_data;
    String		name;
    String		res_class;
    XtResourceList	resources;
    Cardinal		num_resources;
}XmSecondaryResourceDataRec, *XmSecondaryResourceData;

/********    Public Function Declarations for BaseClass.c    ********/

extern Cardinal XmGetSecondaryResourceData( 
                        WidgetClass w_class,
                        XmSecondaryResourceData **secondaryDataRtn) ;

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for ImageCache.c    ********/

extern Boolean XmInstallImage( 
                        XImage *image,
                        char *image_name) ;
extern Boolean XmUninstallImage( 
                        XImage *image) ;
extern Pixmap XmGetPixmap( 
                        Screen *screen,
                        char *image_name,
                        Pixel foreground,
                        Pixel background) ;
extern Pixmap XmGetPixmapByDepth( 
                        Screen *screen,
                        char *image_name,
                        Pixel foreground,
                        Pixel background,
			int depth) ;
extern Boolean XmDestroyPixmap( 
                        Screen *screen,
                        Pixmap pixmap) ;

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for Resolve.c    ********/
/*-- XmeResolvePartOffsets is defined in XmP.h.
     These also belong there but for source compatibility, we let 
     them here --- */

typedef long XmOffset;
typedef XmOffset *XmOffsetPtr;



extern void XmResolveAllPartOffsets( 
                        WidgetClass w_class,
                        XmOffsetPtr *offset,
                        XmOffsetPtr *constraint_offset) ;
extern void XmResolvePartOffsets( 
                        WidgetClass w_class,
                        XmOffsetPtr *offset) ;

/********    End Public Function Declarations    ********/



/********    Public Function Declarations for Xm.c    ********/


extern void XmUpdateDisplay( 
                        Widget w) ;
extern Widget XmObjectAtPoint(
			Widget wid,
			Position x,
			Position y ) ;

extern Boolean XmWidgetGetBaselines(
                        Widget wid,
                        Dimension **baselines,
                        int *line_count);
extern Boolean XmWidgetGetDisplayRect(
                        Widget wid,
                        XRectangle *displayrect);

/********    End Public Function Declarations    ********/



/********    Primitive.c    ********/

/********    Public Function Declarations for Primitive.c    ********/

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for ResConvert.c    ********/

extern void XmCvtStringToUnitType( 
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val) ;
extern char * XmRegisterSegmentEncoding( 
                        char *fontlist_tag,
                        char *ct_encoding) ;
extern char * XmMapSegmentEncoding( 
                        char *fontlist_tag) ;
extern XmString XmCvtCTToXmString( 
                        char *text) ;
extern Boolean XmCvtTextToXmString( 
                        Display *display,
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val,
                        XtPointer *converter_data) ;
extern char * XmCvtXmStringToCT( 
                        XmString string) ;
extern Boolean XmCvtXmStringToText( 
                        Display *display,
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val,
                        XtPointer *converter_data) ;

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for ResInd.c    ********/

extern int XmConvertStringToUnits(
				  Screen *screen, 
				  String spec,
				  int orientation,
				  int to_type,
				  XtEnum *parse_error);
extern int XmConvertUnits( 
                        Widget widget,
                        int dimension,
                        register int from_type,
                        register int from_val,
                        register int to_type) ;
extern int XmCvtToHorizontalPixels( 
                        Screen *screen,
                        register int from_val,
                        register int from_type) ;
extern int XmCvtToVerticalPixels( 
                        Screen *screen,
                        register int from_val,
                        register int from_type) ;
extern int XmCvtFromHorizontalPixels( 
                        Screen *screen,
                        register int from_val,
                        register int to_type) ;
extern int XmCvtFromVerticalPixels( 
                        Screen *screen,
                        register int from_val,
                        register int to_type) ;
extern void XmSetFontUnits( 
                        Display *display,
                        int h_value,
                        int v_value) ;
extern void XmSetFontUnit( 
                        Display *display,
                        int value) ;

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for MenuUtil.c    ********/

extern void XmSetMenuCursor( 
                        Display *display,
                        Cursor cursorId) ;
extern Cursor XmGetMenuCursor( 
                        Display *display) ;

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for Simple.c    ********/

extern Widget XmCreateSimpleMenuBar( 
                        Widget parent,
                        String name,
                        ArgList args,
                        Cardinal arg_count) ;
extern Widget XmCreateSimplePopupMenu( 
                        Widget parent,
                        String name,
                        ArgList args,
                        Cardinal arg_count) ;
extern Widget XmCreateSimplePulldownMenu( 
                        Widget parent,
                        String name,
                        ArgList args,
                        Cardinal arg_count) ;
extern Widget XmCreateSimpleOptionMenu( 
                        Widget parent,
                        String name,
                        ArgList args,
                        Cardinal arg_count) ;
extern Widget XmCreateSimpleRadioBox( 
                        Widget parent,
                        String name,
                        ArgList args,
                        Cardinal arg_count) ;
extern Widget XmCreateSimpleCheckBox( 
                        Widget parent,
                        String name,
                        ArgList args,
                        Cardinal arg_count) ;

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for VaSimple.c   ********/
extern Widget XmVaCreateSimpleMenuBar( 
			Widget parent,
			String name,
			...) ;
extern Widget XmVaCreateSimplePopupMenu(
			Widget parent,
			String name,
			XtCallbackProc callback,
			...) ;
extern Widget XmVaCreateSimplePulldownMenu( 
			Widget parent,
			String name,
			int post_from_button,
			XtCallbackProc callback,
			...) ;
extern Widget XmVaCreateSimpleOptionMenu(
			Widget parent,
			String name,
                        XmString option_label,
                        KeySym option_mnemonic,
                        int button_set,
                        XtCallbackProc callback,
			...) ;
extern Widget XmVaCreateSimpleRadioBox( 
			Widget parent,
			String name,
			int button_set,
			XtCallbackProc callback,
			...) ;
extern Widget XmVaCreateSimpleCheckBox( 
			Widget parent,
			String name,
			XtCallbackProc callback,
			...) ;
/********    End Public Function Declarations    ********/

/********    Public Function Declarations for TrackLoc.c    ********/

extern Widget XmTrackingEvent( 
                        Widget widget,
                        Cursor cursor,
#if NeedWidePrototypes
                        int confineTo,
#else
                        Boolean confineTo,
#endif /* NeedWidePrototypes */
                        XEvent *pev) ;
extern Widget XmTrackingLocate( 
                        Widget widget,
                        Cursor cursor,
#if NeedWidePrototypes
                        int confineTo) ;
#else
                        Boolean confineTo) ;
#endif /* NeedWidePrototypes */

/********    End Public Function Declarations    ********/

/********    Visual.c    ********/
typedef void (*XmColorProc) (XColor *bg_color, XColor *fg_color,
	XColor *sel_color, XColor *ts_color, XColor *bs_color);

/********    Public Function Declarations for Visual.c    ********/

extern XmColorProc XmSetColorCalculation( 
                        XmColorProc proc) ;
extern XmColorProc XmGetColorCalculation( void ) ;
extern void XmGetColors( 
                        Screen *screen,
                        Colormap color_map,
                        Pixel background,
                        Pixel *foreground_ret,
                        Pixel *top_shadow_ret,
                        Pixel *bottom_shadow_ret,
                        Pixel *select_ret) ;
extern void XmChangeColor(
                        Widget widget,
                        Pixel background) ;

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for XmString.c    ********/

extern XmString XmStringCreate( 
                        char *text,
                        XmStringCharSet charset) ;
extern XmString XmStringCreateSimple( 
                        char *text) ;
extern XmString XmStringCreateLocalized( 
                        String text) ;
extern XmString XmStringDirectionCreate( 
#if NeedWidePrototypes
                        int direction) ;
#else
                        XmStringDirection direction) ;
#endif /* NeedWidePrototypes */
extern XmString XmStringSeparatorCreate( void ) ;
extern XmString XmStringSegmentCreate( 
                        char *text,
                        XmStringCharSet charset,
#if NeedWidePrototypes
                        int direction,
                        int separator) ;
#else
                        XmStringDirection direction,
                        Boolean separator) ;
#endif /* NeedWidePrototypes */
extern XmString XmStringLtoRCreate( 
                        char *text,
                        XmStringCharSet charset) ;
extern XmString XmStringCreateLtoR( 
                        char *text,
                        XmStringCharSet charset) ;
extern Boolean XmStringInitContext( 
                        XmStringContext *context,
                        XmString string) ;
extern void XmStringFreeContext( 
                        XmStringContext context) ;
extern XmStringComponentType XmStringGetNextComponent( 
                        XmStringContext context,
                        char **text,
                        XmStringCharSet *charset,
                        XmStringDirection *direction,
                        XmStringComponentType *unknown_tag,
                        unsigned short *unknown_length,
                        unsigned char **unknown_value) ;
extern XmStringComponentType XmStringPeekNextComponent( 
                        XmStringContext context) ;
extern Boolean XmStringGetNextSegment( 
                        XmStringContext context,
                        char **text,
                        XmStringCharSet *charset,
                        XmStringDirection *direction,
                        Boolean *separator) ;
extern Boolean XmStringGetLtoR( 
                        XmString string,
                        XmStringCharSet charset,
                        char **text) ;
extern XmFontListEntry XmFontListEntryCreate( 
                        char *tag,
                        XmFontType type,
                        XtPointer font) ;
extern XmFontListEntry XmFontListEntryCreate_r(
                        char *tag,
                        XmFontType type,
                        XtPointer font,
                        Widget wid) ;
extern void XmFontListEntryFree( 
                        XmFontListEntry *entry) ;
extern XtPointer XmFontListEntryGetFont( 
                        XmFontListEntry entry,
                        XmFontType *typeReturn) ;
extern char * XmFontListEntryGetTag( 
                        XmFontListEntry entry) ;
extern XmFontList XmFontListAppendEntry( 
                        XmFontList old,
                        XmFontListEntry entry) ;
extern XmFontListEntry XmFontListNextEntry( 
                        XmFontContext context) ;
extern XmFontList XmFontListRemoveEntry( 
                        XmFontList old,
                        XmFontListEntry entry) ;
extern XmFontListEntry XmFontListEntryLoad( 
                        Display *display,
                        char *fontName,
                        XmFontType type,
                        char *tag) ;
extern XmFontList XmFontListCreate( 
                        XFontStruct *font,
                        XmStringCharSet charset) ;
extern XmFontList XmFontListCreate_r(
                        XFontStruct *font,
                        XmStringCharSet charset,
                        Widget wid) ;
extern XmFontList XmStringCreateFontList( 
                        XFontStruct *font,
                        XmStringCharSet charset) ;
extern XmFontList XmStringCreateFontList_r(
                        XFontStruct *font,
                        XmStringCharSet charset,
                        Widget wid) ;
extern void XmFontListFree( 
                        XmFontList fontlist) ;
extern XmFontList XmFontListAdd( 
                        XmFontList old,
                        XFontStruct *font,
                        XmStringCharSet charset) ;
extern XmFontList XmFontListCopy( 
                        XmFontList fontlist) ;
extern Boolean XmFontListInitFontContext( 
                        XmFontContext *context,
                        XmFontList fontlist) ;
extern Boolean XmFontListGetNextFont( 
                        XmFontContext context,
                        XmStringCharSet *charset,
                        XFontStruct **font) ;
extern void XmFontListFreeFontContext( 
                        XmFontContext context) ;
extern XmString XmStringConcat( 
                        XmString a,
                        XmString b) ;
extern XmString XmStringConcatAndFree(
			XmString a, 
			XmString b) ;
extern XmString XmStringNConcat( 
                        XmString first,
                        XmString second,
                        int n) ;
extern XmString XmStringCopy( 
                        XmString string) ;
extern XmString XmStringNCopy( 
                        XmString str,
                        int n) ;
extern Boolean XmStringByteCompare( 
                        XmString a1,
                        XmString b1) ;
extern Boolean XmStringCompare( 
                        XmString a,
                        XmString b) ;
extern int XmStringLength( 
                        XmString string) ;
extern Boolean XmStringEmpty( 
                        XmString string) ;
extern Boolean XmStringIsVoid(XmString string); 
extern Boolean XmStringHasSubstring( 
                        XmString string,
                        XmString substring) ;
extern void XmStringFree( 
                        XmString string) ;
extern Dimension XmStringBaseline( 
                        XmFontList fontlist,
                        XmString string) ;
extern Dimension XmStringWidth( 
                        XmFontList fontlist,
                        XmString string) ;
extern Dimension XmStringHeight( 
                        XmFontList fontlist,
                        XmString string) ;
extern void XmStringExtent( 
                        XmFontList fontlist,
                        XmString string,
                        Dimension *width,
                        Dimension *height) ;
extern int XmStringLineCount( 
                        XmString string) ;
extern void XmStringDraw( 
                        Display *d,
                        Window w,
                        XmFontList fontlist,
                        XmString string,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        unsigned int align,
                        unsigned int lay_dir,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        unsigned char align,
                        unsigned char lay_dir,
#endif /* NeedWidePrototypes */
                        XRectangle *clip) ;
extern void XmStringDrawImage( 
                        Display *d,
                        Window w,
                        XmFontList fontlist,
                        XmString string,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        unsigned int align,
                        unsigned int lay_dir,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        unsigned char align,
                        unsigned char lay_dir,
#endif /* NeedWidePrototypes */
                        XRectangle *clip) ;
extern void XmStringDrawUnderline( 
                        Display *d,
                        Window w,
                        XmFontList fntlst,
                        XmString str,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        unsigned int align,
                        unsigned int lay_dir,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        unsigned char align,
                        unsigned char lay_dir,
#endif /* NeedWidePrototypes */
                        XRectangle *clip,
                        XmString under) ;
extern unsigned int XmCvtXmStringToByteStream(
			XmString string,
        	        unsigned char **prop_return);
extern XmString XmCvtByteStreamToXmString(
        	        unsigned char *property);
extern unsigned int XmStringByteStreamLength(unsigned char *string);
extern XmStringComponentType XmStringPeekNextTriple(XmStringContext context);
extern XmStringComponentType XmStringGetNextTriple(XmStringContext context,
						   unsigned int *length,
						   XtPointer *value);
extern XmString XmStringComponentCreate(XmStringComponentType tag,
					unsigned int length,
					XtPointer value);
extern XtPointer XmStringUnparse(XmString string,
				 XmStringTag tag,
				 XmTextType tag_type,
				 XmTextType output_type,
				 XmParseTable parse_table,
				 Cardinal parse_count,
				 XmParseModel parse_model);
extern XmString XmStringParseText(XtPointer text,
				  XtPointer *text_end,
				  XmStringTag tag,
				  XmTextType type,
				  XmParseTable parse_table,
				  Cardinal parse_count,
				  XtPointer call_data);
extern Cardinal XmStringToXmStringTable(XmString string,
					XmString break_comp,
					XmStringTable *table);
extern XmString XmStringTableToXmString(XmStringTable table,
					Cardinal count,
					XmString break_component);
extern XtPointer *XmStringTableUnparse(XmStringTable table,
				       Cardinal count,
				       XmStringTag tag,
				       XmTextType tag_type,
				       XmTextType output_type,
				       XmParseTable parse,
				       Cardinal parse_count,
				       XmParseModel parse_model);
extern XmStringTable XmStringTableParseStringArray(XtPointer *strings,
						   Cardinal count,
						   XmStringTag tag,
						   XmTextType type,
						   XmParseTable parse,
						   Cardinal parse_count,
						   XtPointer call_data);

extern XmStringDirection XmDirectionToStringDirection(XmDirection dir);
extern XmDirection XmStringDirectionToDirection(XmStringDirection dir);

extern XmString XmStringGenerate(XtPointer   text,
				 XmStringTag tag,
				 XmTextType  type,
				 XmStringTag rendition);
extern XmString XmStringPutRendition(XmString string,
				     XmStringTag rendition); 

extern XmParseMapping XmParseMappingCreate(ArgList  arg_list,
					   Cardinal arg_count);
extern void XmParseMappingSetValues(XmParseMapping parse_mapping,
				    ArgList        arg_list,
				    Cardinal       arg_count);
extern void XmParseMappingGetValues(XmParseMapping parse_mapping,
				    ArgList        arg_list,
				    Cardinal       arg_count);
extern void XmParseMappingFree(XmParseMapping parse_mapping);
extern void XmParseTableFree(XmParseTable parse_table,
			     Cardinal     parse_count);

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for XmTabList.c    ********/

extern XmTabList 
  XmStringTableProposeTablist(XmStringTable strings, 
			      Cardinal num_strings,
			      Widget widget,
			      float pad_value,
			      XmOffsetModel offset_model);
extern void XmTabSetValue(XmTab xmtab, float value);
extern float 
  XmTabGetValues(XmTab xmtab, 
		 unsigned char *units, 
		 XmOffsetModel *offset, 
		 unsigned char *alignment, 
		 char **decimal);
extern void XmTabFree(XmTab xmtab);
extern XmTab XmTabCreate(float value, 
			 unsigned char units, 
			 XmOffsetModel offset_model, 
			 unsigned char alignment, 
			 char *decimal);
extern XmTabList 
  XmTabListRemoveTabs(XmTabList oldlist, 
		      Cardinal *position_list, 
		      Cardinal position_count);
extern XmTabList 
  XmTabListReplacePositions(XmTabList oldlist, 
			    Cardinal *position_list, 
			    XmTab *tabs, 
			    Cardinal tab_count);
extern XmTab XmTabListGetTab(XmTabList tablist, Cardinal position);
extern Cardinal XmTabListTabCount(XmTabList tablist);
extern XmTabList XmTabListCopy(XmTabList tablist, int offset, Cardinal count);
extern void XmTabListFree(XmTabList tablist);
extern XmTabList XmTabListInsertTabs(XmTabList oldlist,
				     XmTab *tabs, 
				     Cardinal tab_count, 
				     int position);
/********    End Public Function Declarations    ********/

/********    Public Function Declarations for XmRenderTable.c    ********/

extern XmRenderTable XmRenderTableCvtFromProp(Widget, char *prop, unsigned int len);
extern unsigned int XmRenderTableCvtToProp(Widget, XmRenderTable table, char **prop_return);
extern void XmRenditionUpdate(XmRendition rendition, ArgList arglist, Cardinal argcount);
extern void XmRenditionRetrieve(XmRendition rendition, 
				ArgList arglist,
				Cardinal argcount);
extern void XmRenditionFree(XmRendition rendition);
extern XmRendition XmRenditionCreate(Widget widget,
				     XmStringTag tag,
				     ArgList arglist,
				     Cardinal argcount);
extern XmRendition 
  *XmRenderTableGetRenditions(XmRenderTable table, 
			      XmStringTag *tags,
			      Cardinal tag_count);
extern XmRendition XmRenderTableGetRendition(XmRenderTable table,
					     XmStringTag tag);
extern int XmRenderTableGetTags(XmRenderTable table,
				XmStringTag **tag_list);
extern void XmRenderTableFree(XmRenderTable table);
extern XmRenderTable XmRenderTableCopy(XmRenderTable table,
				       XmStringTag *tags, 
				       int tag_count);
extern XmRenderTable 
  XmRenderTableRemoveRenditions(XmRenderTable oldtable,
				XmStringTag *tags, 
				int tag_count);
extern XmRenderTable 
  XmRenderTableAddRenditions(XmRenderTable oldtable, 
			     XmRendition *renditions, 
			     Cardinal rendition_count, 
			     XmMergeMode merge_mode);

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for Dest.c    ********/

extern Widget XmGetDestination( 
                        Display *display) ;

/********    End Public Function Declarations    ********/

/********    Public Function Declarations for Traversal.c    ********/

extern Boolean XmIsTraversable( 
                        Widget wid) ;
extern XmVisibility XmGetVisibility( 
                        Widget wid) ;
extern Widget XmGetTabGroup( 
                        Widget wid) ;
extern Widget XmGetFocusWidget( 
                        Widget wid) ;
extern Boolean XmProcessTraversal( 
                        Widget w,
                        XmTraversalDirection dir) ;
extern void XmAddTabGroup( 
                        Widget tabGroup) ;
extern void XmRemoveTabGroup( 
                        Widget w) ;

/********    End Public Function Declarations    ********/

/********        ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

/*
 * The following includes are for source compatibility.  They might be
 *    removed at some future time.
 */
#include <Xm/VendorS.h>
#include <Xm/XmIm.h>

#define XmINDICATOR_3D_BOX		0x01
#define XmINDICATOR_FLAT_BOX		0x02
#define XmINDICATOR_CHECK_GLYPH		0x10
#define XmINDICATOR_CROSS_GLYPH		0x20

enum {
  XmINDICATOR_NONE      = 0, 
  XmINDICATOR_FILL      = 1,	/* Treated as _BOX or _CHECK_BOX */
  XmINDICATOR_BOX	= 255,	/* Treated as XmINDICATOR_3D_BOX */
  XmINDICATOR_CHECK     = XmINDICATOR_CHECK_GLYPH,
  XmINDICATOR_CHECK_BOX = XmINDICATOR_CHECK_GLYPH + XmINDICATOR_3D_BOX,
  XmINDICATOR_CROSS     = XmINDICATOR_CROSS_GLYPH,
  XmINDICATOR_CROSS_BOX = XmINDICATOR_CROSS_GLYPH + XmINDICATOR_3D_BOX
};

enum { XmUNSET, XmSET, XmINDETERMINATE };
enum { XmTOGGLE_BOOLEAN, XmTOGGLE_INDETERMINATE };
typedef unsigned char XmToggleButtonState;

/* Shared text enum. */
typedef enum { EditDone, EditError, EditReject } XmTextStatus;

/* XmDisplay.XmNdefaultButtonEmphasis enum */
enum { XmEXTERNAL_HIGHLIGHT, XmINTERNAL_HIGHLIGHT };

/* new for XmString */
#define _MOTIF_DEFAULT_LOCALE "_MOTIF_DEFAULT_LOCALE"

enum { XmPATH_MODE_FULL, XmPATH_MODE_RELATIVE };
enum { XmFILTER_NONE, XmFILTER_HIDDEN_FILES} ;
  
#endif /* _Xm_h */
 /* DON'T ADD STUFF AFTER THIS #endif */

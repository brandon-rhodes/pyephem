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
/* $TOG: TextFP.h /main/17 1997/03/04 17:00:01 dbl $ */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTextFP_h
#define _XmTextFP_h

#include <Xm/PrimitiveP.h>
#include <Xm/TextF.h>
#ifdef USE_XFT
#include <X11/Xft/Xft.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Defines for different cursors
 */

#define IBEAM_WIDTH	3
#define CARET_WIDTH	9
#define CARET_HEIGHT	5

/*
 * Here is the Text Field Widget class structure.
 */

typedef struct _XmTextFieldClassPart {
  XtPointer extension;		/* Pointer to extension record. */
} XmTextFieldClassPart;

typedef struct _XmTextFieldClassRec {
  CoreClassPart core_class;  /* Not RectObjClassPart so I can reference
				  core_class s */
  XmPrimitiveClassPart primitive_class;
  XmTextFieldClassPart text_class;
} XmTextFieldClassRec;

externalref XmTextFieldClassRec xmTextFieldClassRec;

/*
 * On the spot support.
 */
typedef struct _OnTheSpotData {
  XmTextPosition start;
  XmTextPosition end;
  XmTextPosition cursor;
  int over_len;
  int over_maxlen;
  char *over_str;
  int under_preedit;
  Boolean under_verify_preedit;
  Boolean verify_commit;
  int pad;
} OnTheSpotDataRec, *OnTheSpotData;

/*
 * Here is the Text Field Widget instance structures.
 */

typedef struct _XmTextFieldPart {
    XtCallbackList activate_callback;	       /* Command activate callback */
    XtCallbackList focus_callback;             /* Verify gain focus callback */
    XtCallbackList losing_focus_callback;      /* Verify losing focus 
						  callback */
    XtCallbackList modify_verify_callback;     /* Verify value to change 
						  callback */
    XtCallbackList wcs_modify_verify_callback; /* Verify value to change 
						  callback */
    XtCallbackList motion_verify_callback;     /* Verify insert cursor position
						  to change callback */
    XtCallbackList gain_primary_callback;      /* Gained ownership of Primary
						  Selection */
    XtCallbackList lose_primary_callback;      /* Lost ownership of Primary
						  Selection */
    XtCallbackList value_changed_callback;     /* Notify that value has changed
						  callback */
    char * value;		/* pointer to widget value stored as char * */
    wchar_t * wc_value;		/* pointer to widget value stored as 
				   wchar_t * */

    XmFontList font_list;	/* Uses only the font portion of fontlist */
#if USE_XFT
    XtPointer font;	        /* font retrieved from the fontlist */
#else
    XFontStruct *font;	        /* font retrieved from the fontlist */
#endif
    XmTextScanType *selection_array; /* Description of what to cycle
					through on selections */
    _XmHighlightData highlight;      /* Info on the highlighting regions. */

    GC gc;			/* Normal GC for drawing text and cursor */
    GC image_gc;		/* Image GC for drawing text cursor*/
    GC save_gc;                 /* GC for saving/restoring under IBeam */

    Pixmap ibeam_off;		/* pixmap for area under the IBeam */
    Pixmap add_mode_cursor;	/* The add mode cursor pixmap */
    Pixmap cursor;		/* The ibeam cursor stencil */
    Pixmap putback;		/* AVAILABLE: was in 1.1 but not really used */
    Pixmap stipple_tile;	/* The tile pattern for the stippled I-beam */
    Pixmap image_clip;		/* AVAILABLE: was in 1.2 but not used now */

    XmTextPosition cursor_position;  /* Character location of the insert 
					cursor */
    XmTextPosition new_h_offset;     /* AVAILABLE: was in 1.1 but not used */
    XmTextPosition h_offset;  	     /* The x position of the first character
					(relative to left edge of widget) */
    XmTextPosition orig_left;        /* Left primary selection prior to 
					extend */
    XmTextPosition orig_right;       /* Right primary selection prior to
					extend */
    XmTextPosition prim_pos_left;    /* Left primary selection position */
    XmTextPosition prim_pos_right;   /* Right primary selection position */
    XmTextPosition prim_anchor;	     /* Primary selection pivot point */

    XmTextPosition sec_pos_left;     /* Left secondary selection position */
    XmTextPosition sec_pos_right;    /* Right secondary selection position */
    XmTextPosition sec_anchor;	     /* Secondary selection pivot point */

    XmTextPosition stuff_pos;	/* Position to stuff the primary selection */

    Position select_pos_x;      /* x position for timer-based scrolling */

    Time prim_time;             /* Timestamp of primary selection */
    Time dest_time;             /* Timestamp of destination selection */
    Time sec_time;              /* Timestamp of secondary selection */
    Time last_time;             /* Time of last selection event */

    XtIntervalId timer_id;	/* Blinking cursor timer */
    XtIntervalId select_id;     /* Timer based scrolling identifier */

    int blink_rate;		/* Rate of blinking text cursor in msec */
    int selection_array_count;  /* Selection array count */
    int threshold;		/* Selection threshold */
    int size_allocd;		/* Size allocated for value string */
    int string_length;          /* The number of characters in the string 
				   (including the trailing NULL) */
    int cursor_height;		/* Save cursor dimensions */
    int cursor_width;		/* Save cursor dimensions */
    int sarray_index;		/* Index into selection array */
    int max_length;		/* Maximum number of character that can be
				   inserted into the text field widget */

    int max_char_size;          /* Max bytes per character in cur locale */
    short columns;		/* The number of characters in the width */

    Dimension margin_width;	/* Height between text borders and text */
    Dimension margin_height;	/* Width between text borders and text */
    Dimension average_char_width;   /* Average character width based on font */
    Dimension margin_top;       /* Height between text borders and top of 
				   text */
    Dimension margin_bottom;    /* Height between text borders and bottom of 
				   text */
    Dimension font_ascent;      /* Ascent of font or fontset used by widget */
    Dimension font_descent;     /* Descent of font or fontset used by widget */

    Boolean resize_width;	/* Allows the widget to grow horizontally
				   when borders are reached */
    Boolean pending_delete;	/* Delete primary selection on insert when
				   set to True */
    Boolean editable;		/* Sets editablility of text */
    Boolean verify_bell;        /* Determines if bell is sounded when verify
				   callback returns doit - False */
    Boolean cursor_position_visible; /* Sets visibility of insert cursor */

    Boolean traversed;          /* Flag used with losing focus verification to
                                   indicate a traversal key pressed event */
    Boolean add_mode;		/* Add mode for cursor movement */
    Boolean has_focus;		/* Flag that indicates whether the widget
			           has input focus */
    Boolean blink_on;		/* State of Blinking insert cursor */
    short int cursor_on;	/* Indicates whether the cursor is visible */
    Boolean refresh_ibeam_off;	/* Indicates whether the area under IBeam needs
				   to be re-captured */
    Boolean have_inverted_image_gc;  /* fg/bg of image gc have been swapped */
    Boolean has_primary;	/* Indicates that is has the
				   primary selection */
    Boolean has_secondary;	/* Indicates that is has the
				   secondary selection */
    Boolean has_destination;	/* Indicates that is has the
				   destination selection */
    Boolean sec_drag;           /* Indicates a secondary drag was made */ 
    Boolean selection_move;	/* Indicates that the action requires a
				   secondary move (i.e. copy & cut) */
    Boolean pending_off;	/* indicates pending delete state */
    Boolean fontlist_created;   /* Indicates that the text field widget created
				   it's own fontlist */
    Boolean has_rect;		/* currently has clipping rectangle */
    Boolean do_drop;		/* Indicates that the widget the recieved the
				   button release, did not have a previous
                                   button press, so it is o.k. to request
				   the MOTIF_DROP selection. */
    Boolean cancel;		/* Cancels selection actions when true */
    Boolean extending;		/* Indicates extending primary selection */
    Boolean sec_extending;      /* Indicates extending secondary selection */
    Boolean changed_visible;    /* Indicates whether the dest_visible flag
                                   is in a temporary changed state */
    Boolean have_fontset;       /* The widgets font is a fontset, not a 
				   fontstruct... use R5 draw routines */
    Boolean in_setvalues;	/* used to disable unnecessary redisplays */
    Boolean do_resize;		/* used to prevent inappropriate resizes */
    Boolean redisplay;		/* used to set redisplay flag in setvalues */
    Boolean overstrike;		/* overstrike mode for character input */
    Boolean sel_start;		/* overstrike mode for character input */
    XtPointer extension;	/* Pointer to extension record. */

    XtCallbackList  destination_callback;   /* Selection destination cb */
    Boolean selection_link;	/* Indicates that the action requires a
				   link */
    /* New for 2.0 */
    Boolean take_primary;	/* Indicates that is has to take the
				   primary selection */
    GC cursor_gc;               /* 1-bit depth GC for creating the I-beam 
				   stipples (normal & add mode) */
    XtIntervalId drag_id;       /* timer to start btn1 drag */
    _XmTextActionRec *transfer_action;  /* to keep track of delayed action */
    /* Boolean rt_save; */  		/* used for MT work */
    OnTheSpotData onthespot;    /* data for on-the-spot im support */

    Boolean check_set_render_table; /* used for MT safe work */
    Boolean programmatic_highlights;	/* XmTextFieldSetHighlight called */
#ifdef USE_XFT
    Boolean use_xft;
#endif
} XmTextFieldPart;

typedef struct _XmTextFieldRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmTextFieldPart text;
} XmTextFieldRec;


/****************
 *
 * Macros for the uncached data
 *
 ****************/

#define TextF_ActivateCallback(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.activate_callback)
#define TextF_LosingFocusCallback(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.losing_focus_callback)
#define TextF_FocusCallback(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.focus_callback)
#define TextF_ModifyVerifyCallback(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.modify_verify_callback)
#define TextF_ModifyVerifyCallbackWcs(tfg) \
	(((XmTextFieldWidget)(tfg)) -> text.wcs_modify_verify_callback)
#define TextF_MotionVerifyCallback(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.motion_verify_callback)
#define TextF_ValueChangedCallback(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.value_changed_callback)
#define TextF_Value(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.value)
#define TextF_WcValue(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.wc_value)
#define TextF_MarginHeight(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.margin_height)
#define TextF_MarginWidth(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.margin_width)
#define TextF_CursorPosition(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.cursor_position)
#define TextF_Columns(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.columns)
#define TextF_MaxLength(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.max_length)
#define TextF_BlinkRate(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.blink_rate)
#define TextF_FontList(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.font_list)
#define TextF_Font(tfg)			\
	((XFontStruct*)(((XmTextFieldWidget)(tfg)) -> text.font))
#define TextF_FontAscent(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.font_ascent)
#define TextF_FontDescent(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.font_descent)
#define TextF_SelectionArray(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.selection_array)
#define TextF_SelectionArrayCount(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.selection_array_count)
#define TextF_ResizeWidth(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.resize_width)
#define TextF_PendingDelete(tfg)	\
	(((XmTextFieldWidget)(tfg)) -> text.pending_delete)
#define TextF_Editable(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.editable)
#define TextF_CursorPositionVisible(tfg) \
	(((XmTextFieldWidget)(tfg)) -> text.cursor_position_visible)
#define TextF_Threshold(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.threshold)
#define TextF_UseFontSet(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.have_fontset)
#ifdef USE_XFT
#define TextF_UseXft(tfg)		\
	(((XmTextFieldWidget)(tfg)) -> text.use_xft)
#define	TextF_XftFont(tfg)		\
	((XftFont*)(((XmTextFieldWidget)(tfg)) -> text.font))
#endif

/*
 * On the spot support.
 */
#define PreStart(tfg)                           (((XmTextFieldWidget)(tfg)) -> \
                                           text.onthespot->start)
#define PreEnd(tfg)                             (((XmTextFieldWidget)(tfg)) -> \
                                           text.onthespot->end)
#define PreCursor(tfg)                          (((XmTextFieldWidget)(tfg)) -> \
                                           text.onthespot->cursor)
#define FUnderVerifyPreedit(tfg)	  	(((XmTextFieldWidget)(tfg)) -> \
					 text.onthespot->under_verify_preedit)
#define FVerifyCommitNeeded(tfg)		(((XmTextFieldWidget)(tfg)) -> \
					 text.onthespot->verify_commit)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextFieldWidgetP_h */

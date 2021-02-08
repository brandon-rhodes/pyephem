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
/* $XConsortium: TextP.h /main/16 1996/05/21 12:09:47 pascale $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
#ifndef _XmTextP_h
#define _XmTextP_h

#include <Xm/PrimitiveP.h>
#include <Xm/TextOutP.h>
#include <Xm/TextInP.h>
#include <Xm/TransferT.h>

typedef struct _InputRec *Input;
typedef struct _OutputRec *Output;

#ifdef __cplusplus
extern "C" {
#endif

#define NODELTA		LONG_MAX

#define TEXTWIDGETCLASS	"Text"	/* Resource class for the text widget. */

#define GetSrc(widget)  (((XmTextWidget) (widget))->text.source)

typedef struct {
  XtPointer		extension;	/* Pointer to extension record */
} XmTextClassPart;

typedef struct _XmTextClassRec {
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmTextClassPart text_class;
} XmTextClassRec;

externalref XmTextClassRec	xmTextClassRec;

typedef struct {
    XmTextPosition start;	/* First position in this line. */
    Boolean changed;		/* TRUE if something in this line changed. */
    XmTextPosition changed_position; /* First position within the line that
					has changed, if any. */
    Boolean past_end;		/* TRUE if this line is past the end of any */
				/* line actually on the screen. */
    LineTableExtra extra;	/* Extra info the output module keeps. */
} LineRec, *Line;

typedef struct _XmTextLineTableRec{
    unsigned int start_pos:31;
    unsigned int virt_line:1;
} XmTextLineTableRec, *XmTextLineTable;

typedef struct {
    XmTextPosition from, to;	/* Information on one range to repaint. */
} RangeRec;

typedef struct {
    Cardinal number;		/* Number of ranges defined. */
    Cardinal maximum;		/* Number of ranges we have space for. */
    RangeRec *range;		/* Pointer to array of ranges. */
} Ranges;

/*
 * The data for on-the-spot support.
 */
typedef struct _OnTheSpotDataTW {
    XmTextPosition start;
    XmTextPosition end;
    XmTextPosition cursor;
    XmTextPosition over_len;
    XmTextPosition over_maxlen;
    char *over_str;
    int under_preedit;
    Boolean under_verify_preedit;
    Boolean verify_commit;
    int pad;
} OnTheSpotDataRecTW, *OnTheSpotDataTW;

#define PreStartTW(tw) ((tw)->text.onthespot->start)
#define PreEndTW(tw) ((tw)->text.onthespot->end)
#define PreCursorTW(tw) ((tw)->text.onthespot->cursor)
#define PreOverLen(tw) ((tw)->text.onthespot->over_len)
#define PreOverMaxLen(tw) ((tw)->text.onthespot->over_maxlen)
#define PreOverStr(tw) ((tw)->text.onthespot->over_str)
#define PreUnder(tw) ((tw)->text.onthespot->under_preedit)
#define UnderVerifyPreedit(tw) ((tw)->text.onthespot->under_verify_preedit)
#define VerifyCommitNeeded(tw) ((tw)->text.onthespot->verify_commit)


/*
 * Structure for main text info. 
 */

typedef struct _XmTextPart {
    XmTextSource source;		   /* The source for this widget. */
    XtCallbackList activate_callback;      /* command activate callback. */
    XtCallbackList focus_callback;	   /* Focus callback. */
    XtCallbackList losing_focus_callback;  /* Losing focus callback. */
    XtCallbackList value_changed_callback; /* Value changed callback. */
    XtCallbackList modify_verify_callback; /* Verify value to change callback.*/
    XtCallbackList wcs_modify_verify_callback; /* Verify value to change 
						* callback.*/
    XtCallbackList motion_verify_callback; /* Insert cursor position 
					      change callback. */
    XtCallbackList gain_primary_callback; /* Gained ownership of Primary
					     Selection */
    XtCallbackList lose_primary_callback; /* Lost ownership of Primary
					     Selection */
    char *value;		    /* The sring value in the widget */
    wchar_t *wc_value;              /* Pointer for wchar_t value set by app */
    Dimension margin_height;        /* height between text borders and text */
    Dimension margin_width;         /* width between text borders and text */
    Position cursor_position_x;     /* x pixel location of cursor */
    OutputCreateProc output_create; /* Routine to create the output portion. */
    InputCreateProc input_create;   /* Routine to create the input portion. */
    /* The naming incongruity amongst the next three items was introduced */
    /* due to a collision with top_position as used as a Form constraint; */
    /* It has no other implications. */
    XmTextPosition top_character;    /* First position to display. */
    XmTextPosition bottom_position; /* Last position to display. */
    XmTextPosition cursor_position; /* Location of the insertion point. */
    int max_length;		    /* Sets the max. length of string */
    int edit_mode;		    /* Sets the line editing mode
				       (i.e. sinlge_line, multi_line, ...) */
    Boolean auto_show_cursor_position; /* If true, automatically try to show
					  the cursor position whenever it
					  changes. */
    Boolean editable;		  /* Determines if text is editable */
    Boolean verify_bell; 	  /* Determines if bell is sounded when verify
				   *  callback returns doit - False
                                   */
    Boolean add_mode;		  /* Determines the state of add moder */
    Boolean traversed;            /* Flag used with losing focus verification to
				     indicate a traversal key pressed event */
    Boolean in_redisplay;	  /* Whether currently in the redisplay proc. */
    Boolean needs_redisplay;	  /* Whether we need to repaint or refigure. */
    Boolean in_refigure_lines;	  /* Whether currently in refigurelines proc. */
    Boolean needs_refigure_lines; /* Whether we need to refigure. */
    Boolean in_setvalues;         /* Use to reduce unnecessary redisplays and
				     geometry requsets */
    Boolean in_resize;		  /* Make sure there are no geometry requests
				     while we are in resize procedure. */
    Boolean in_expose;		  /* Make sure there are no geometry requests
				     while we are in expose procedure. */
    Boolean highlight_changed;	  /* Whether highlighting recently changed. */
    Boolean pendingoff;         /* TRUE if we shouldn't do pending delete on
                                   the current selection. */
    char char_size;             /* number of bytes for storing a character */

    OnOrOff on_or_off;		  /* used to halt unecessary motion verify
				     callback calls during primary moves.*/
    Output output;		   /* The output portion. */
    Input input;		   /* The input portion. */

    XmTextPosition first_position; /* First legal position in the source. */
    XmTextPosition last_position;  /* Last legal position in the source. */
    XmTextPosition forget_past;	   /* Forget all about positions past this. */
    XmTextPosition force_display;  /* Force this position to be displayed. */
    XmTextPosition new_top;	   /* Desired new top position. */
    XmTextPosition last_top_char;  /* unused - available. */
    XmTextPosition dest_position;  /* Location of the destination point. */
    int disable_depth;		   /* How many levels of disable we've done. */

    int pending_scroll;		/* Number of lines we want to scroll. */
    int total_lines;		/* Total number of lines in the text widget */
    int top_line;		/* Line number of the top visible line */
    int vsbar_scrolling;	/* scrolling using the vertical scrollbar */

    Cardinal number_lines;	/* Number of line table entries. */
    Cardinal maximum_lines;	/* Maximum number of line table entries. */
    Line line;			/* Pointer to array of line table entries. */

    Ranges repaint;		/* Info on the repaint ranges. */
    _XmHighlightData highlight;	/* Info on the highlighting regions. */
    _XmHighlightData old_highlight;/* Old value of above. */
    Widget inner_widget;	/* Pointer to widget which actually contains
				   text (may be same or different from
				   this record).  */
    XmTextLineTable line_table;
    unsigned int table_size;
    unsigned int table_index;

    XtCallbackList  destination_callback;   /* Selection destination cb */
    int          hsbar_scrolling;/* scroring using the horizontal scrollbar */

    OnTheSpotDataTW onthespot;  /* On the spot preedit style support. */
} XmTextPart;

typedef struct _XmTextRec {
    CorePart	core;
    XmPrimitivePart primitive;
    XmTextPart text;
} XmTextRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextP_h */
/* DON't ADD STUFF AFTER THIS #endif */

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
/* $TOG: TextOutP.h /main/17 1997/03/18 10:55:51 dbl $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
#ifndef _XmTextOutP_h
#define _XmTextOutP_h

#include <Xm/XmP.h>
#include <Xm/Text.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Definitions for modules implementing and using text output routines.
 *
 ****************************************************************/

#define ShouldWordWrap(data, widget)	(data->wordwrap && \
       (!(((XmDirectionMatch(XmPrim_layout_direction(widget), \
			     XmTOP_TO_BOTTOM_RIGHT_TO_LEFT)) ? \
            data->scrollvertical : data->scrollhorizontal) \
       && XmIsScrolledWindow(widget->core.parent))) \
       && widget->text.edit_mode != XmSINGLE_LINE_EDIT)

typedef struct _LineTableExtraRec {
    Dimension width;
    Boolean wrappedbychar;
} LineTableExtraRec, *LineTableExtra ;

/*         
 * output.c  (part of stext)
 */

typedef unsigned int LineNum;
typedef enum {on, off} OnOrOff;	/* For when Booleans aren't obvious enough. */

/*
 * Return the line number containing the given position.  If text currently
 * knows of no line containing that position, returns NOLINE.
 */

#define NOLINE	30000

/*
 * These next define the types of the routines that output is required
 * to export for use by text and by input.
 */

typedef struct _OutputDataRec {
    XmFontList fontlist;	/* Fontlist for text. */
    unsigned int blinkrate;
    Boolean wordwrap;		/* Whether to wordwrap. */
    Boolean cursor_position_visible;
    Boolean autoshowinsertpoint;
    Boolean hasfocus;
    Boolean has_rect;
    Boolean handlingexposures;	/* TRUE if in the midst of expose events. */
    Boolean exposevscroll;	/* Non-zero if we expect expose events to be
				   off vertically. */
    Boolean exposehscroll;	/* Non-zero if we expect expose events to be
				   off horizontally. */
    Boolean resizewidth, resizeheight;
    Boolean scrollvertical, scrollhorizontal;
    Boolean scrollleftside, scrolltopside;
    Boolean ignorevbar;		/* Whether to ignore callbacks from vbar. */
    Boolean ignorehbar;		/* Whether to ignore callbacks from hbar. */
    short int cursor_on;		/* Whether IBeam cursor is visible. */
    Boolean refresh_ibeam_off;	/* Indicates whether area under IBeam needs
				 * to be re-captured */
    Boolean suspend_hoffset;	/* temporarily suspend horizontal scrolling */
    Boolean use_fontset;        /* True if font to be used is fontset (and
				 * thus need X11R5 Xmb* routines to draw */
    Boolean have_inverted_image_gc; /* fg/bg of image gc have been swapped;
				     * on == True, off == False */
    OnOrOff blinkstate;
    Position insertx, inserty;
    int number_lines;		/* Number of lines that fit in the window. */
    int leftmargin, rightmargin;
    int topmargin, bottommargin;
    int scrollwidth;		/* Total width of text we have to display. */
    int vsliderSize;		/* How big the thumb is in the vbar. */
    int hoffset;		/* How much we've scrolled off the left. */
    int averagecharwidth;	/* Number of pixels for an "average" char. */
    int tabwidth;		/* Number of pixels for a tab. */
    short columns, rows;
    Dimension lineheight;	/* Number of pixels per line. */
    Dimension minwidth, minheight;
    Dimension prevW;
    Dimension prevH;
    Dimension cursorwidth, cursorheight;
    Dimension font_ascent;      /* ascent of the font[set] */
    Dimension font_descent;     /* descent of the font[set] */
    XtIntervalId timerid;
    Pixmap cursor;		/* Pixmap for IBeam cursor stencil. */
    Pixmap add_mode_cursor;	/* Pixmap to use for add mode cursor. */
    Pixmap ibeam_off;		/* Pixmap for area under the IBeam. */
    Pixmap stipple_tile;	/* stipple for add mode cursor. */
    GC gc, imagegc;
    Widget vbar, hbar;
    XFontStruct *font;		/* font used when NULL font is set. */
/* New for 1.2 */
    GC save_gc;                 /* GC for saving/resotring under IBeam */
    short columns_set, rows_set; /* history of previously set dimensions */
/* New for 2.0 */
    XmRenderTable rendertable;  /* Alias for fontlist - only for resource 
				   setting */
    GC cursor_gc;               /* 1-bit depth GC for creating the I-beam 
				   stipples (normal & add mode) */
    int scrollheight;		/* Total height of text we have to display. */
    int voffset;		/* How much we've scrolled off the top. */
    int tabheight;		/* Number of pixels for a tab. */
    Dimension linewidth;	/* Number of pixels per line (when vw). */
    Boolean suspend_voffset;	/* temporarily suspend horizontal scrolling */
} OutputDataRec, *OutputData;


/*
 * Create a new instance of an output object.  This is expected to fill in
 * info about innerwidget and output in the widget record.
 */

typedef void (*OutputCreateProc)(
			Widget,
			ArgList,
			Cardinal) ;
/*
 * Given an (x,y) coordinate, return the closest corresponding position. (For
 * use by input; text shouldn't ever need to know.)
 */

typedef XmTextPosition (*XYToPosProc)(
			XmTextWidget,
#if NeedWidePrototypes
			int,
			int) ;
#else
			Position,	/* These are relative to the */
			Position) ;	/* innerwindow returned above. */
#endif

/*
 * Return the (x,y) coordinate corresponing to the given position.  If this
 * returns FALSE, then the given position isn't being displayed.
 */

typedef Boolean (*PosToXYProc)(
			XmTextWidget,
			XmTextPosition,
			Position *,	/* These are relative to the */
			Position *) ;	/* innerwindow returned above. */

/*
 * Measures the extent of a line.  Given the line number and starting position
 * of a line, returns the starting position of the next line.  Also returns
 * any extra information that the output module may want to associate with
 * this line for future reference.  (In particular, it will want to associate
 * a vertical coordinate for this line.)  This routine should return FALSE if
 * it decides that this line will not fit in the window.  FALSE should never
 * be returned if the line number is zero.  Output may assume that the line
 * table for all preceeding lines have already been set.  In particular, when
 * this routine will return FALSE, then output knows that the entire linetable
 * has been calculated; that is a good time for it to look over the linetable
 * and decide if it wants to do something obnoxious like resize the window.
 *
 * A possible value to put in nextpos is PASTENDPOS.  This indicates that the
 * current line contains the end of the text in the source.
 *
 * nextpos may be NULL.  If it is, then this indicates that we only want to
 * know if the line will fit on the window.  The caller already has its own
 * idea where the next line will start if it does fit.  (If nextpos is NULL,
 * then no extra information should be generated, and the 'extra' parameter
 * should be ignored.)
 */

#define PASTENDPOS	2147483647  /* Biggest number that can fit in long */

typedef Boolean (*MeasureLineProc)(
			XmTextWidget,
			LineNum,
			XmTextPosition,
			XmTextPosition *,
			LineTableExtraRec **) ;

/*
 * Draw some text within a line.  The output finds out information about the
 * line by calling the line table routines.
 *
 * %%% Document special cases (like lines containing PASTENDPOS).
 */

typedef void (*DrawProc)(
			XmTextWidget,
			LineNum,
			XmTextPosition,
			XmTextPosition,
			XmHighlightMode) ;

/*
 * Output should draw or erase an insertion point at the given position.
 */

typedef void (*DrawInsertionPointProc)(
			XmTextWidget,
			XmTextPosition,
			OnOrOff) ;

/*
 * Output should ensure that the given position is visible (e.g., not scrolled
 * off horizontally).
 */
typedef void (*MakePositionVisibleProc)(
			XmTextWidget,
			XmTextPosition) ;

/* Text would like to move some lines around on the screen.  It would like to
 * move lines fromline through toline (inclusive) to now start at line
 * destline.  If output can perform this move by means of a XCopyArea or
 * similar simple call, it does so and returns TRUE; otherwise, it will return
 * FALSE.  If TRUE, output should modify affected values in the
 * "extra" entries in the linetable, because after returning text will go ahead
 * and move linetable entries around.
 */

typedef Boolean (*MoveLinesProc)(
			XmTextWidget,
			LineNum,
			LineNum,
			LineNum) ;

/*
 * Inform output of invalidated positions.
 */

typedef void (*InvalidateProc)(
			XmTextWidget,
			XmTextPosition,
			XmTextPosition,
			long) ;

/*
 * Get preferred size of text widget based on the row and column
 * resources multiplied by font attributes as well as adding the
 * margin resource values to the computed size.
 */
typedef void (*GetPreferredSizeProc)(
			Widget,
			Dimension *,
			Dimension *) ;

/*
 * Get values out of the output object.
 */

typedef void (*GetValuesProc)(
			Widget,
			ArgList,
			Cardinal) ;

/*
 * Set values in the output object.
 */

typedef Boolean (*SetValuesProc)(
			Widget,
			Widget,
			Widget,
			ArgList,
			Cardinal *) ;


typedef struct _OutputRec {
    struct _OutputDataRec	*data; /* Output-specific data; opaque type. */
    XYToPosProc			XYToPos;
    PosToXYProc			PosToXY;
    MeasureLineProc		MeasureLine;
    DrawProc			Draw;
    DrawInsertionPointProc	DrawInsertionPoint;
    MakePositionVisibleProc	MakePositionVisible;
    MoveLinesProc		MoveLines;
    InvalidateProc		Invalidate;
    GetPreferredSizeProc	GetPreferredSize;
    GetValuesProc		GetValues;
    SetValuesProc		SetValues;
    XmRealizeOutProc		realize;
    XtWidgetProc		destroy;
    XmResizeFlagProc		resize;
    XtExposeProc		expose;
} OutputRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextOutP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

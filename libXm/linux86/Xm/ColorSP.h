/*
 *    Copyright 1990 - 1992, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Mark F. Antonelli, Chris D. Peterson
 *
 */

#ifndef	_XmColorSelectorP_h
#define	_XmColorSelectorP_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
 *      INCLUDE FILES
 *************************************************************/
		
#include <Xm/ManagerP.h>
#include <Xm/ColorS.h>

/************************************************************
 *      TYPEDEFS AND DEFINES
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#define XmColorSelector_COLOR_NAME_SIZE 100
#define XmColorSelector_NUM_TOGGLES 2

typedef struct _ColorSelStrings {
    XmString slider_labels[3];

    XmString tog_labels[XmColorSelector_NUM_TOGGLES];
    XmString no_cell_error, file_read_error;
} ColorSelStrings;

typedef struct _ColorInfo {
    char name[XmColorSelector_COLOR_NAME_SIZE], no_space_lower_name[XmColorSelector_COLOR_NAME_SIZE];
    unsigned short red, green, blue;
} ColorInfo;

typedef struct _ColorSelectorClassPart {
    XtPointer 		extension; 
} ColorSelectorClassPart;

typedef struct _XmColorSelectorClassRec
{
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart         constraint_class;
    XmManagerClassPart          manager_class;
    ColorSelectorClassPart	color_selector_class;
} XmColorSelectorClassRec;

typedef struct _XmColorSelectorPart
{
    /* resources */

    XmColorMode		color_mode;		/* selector mode	   */
    char	       *color_name;		/* the colorname we select */ 
    String	        rgb_file;      		/* where to look for	   */
    Dimension		margin_width;           /* for geom management     */
    Dimension		margin_height;	        /* for geom management	   */

    ColorSelStrings     strings;                /* strings for I18N. */

    /* private state */

    int			slider_red;	/* slider values		    */
    int			slider_green;	/* slider values		    */
    int			slider_blue;	/* slider values		    */
    Widget		bb;		/* area to hold all the sliders     */
    Widget		sliders[3];	/* red,green,blue sliders(slider)   */
    Widget		scrolled_list;	/* list (scrolled window)	    */
    Widget		list;		/* list (simple)		    */
    Widget		color_window;   /* label to show selected color     */
    Widget		chose_radio;	/* selector type radio box	    */
    Widget		chose_mode[2];	/* selector type toggles	    */

    Pixel		color_pixel;		/* pixel value for colors  */
    Boolean 		good_cell;      /* does color_pixel contain
					 * a good value? */
    ColorInfo 		*colors;        /* infomation about all color names */
    short		num_colors;     /* The number of colors. */
} XmColorSelectorPart;

typedef struct _XmColorSelectorRec
{
    CorePart 		core;
    CompositePart 	composite;
    ConstraintPart      constraint;
    XmManagerPart       manager;
    XmColorSelectorPart	cs;
} XmColorSelectorRec;

typedef struct _XmColorSelectorConstraintPart
{
    XtPointer extension;
} XmColorSelectorConstraintPart;

typedef struct _XmColorSelectorConstraintRec
{
    XmManagerConstraintPart       manager;
    XmColorSelectorConstraintPart cs;
} XmColorSelectorConstraintRec;

/************************************************************
 *      MACROS
 *************************************************************/

#define XmColorSelectorIndex (XmManagerIndex + 1)

#define XmColorSField(w,f,t) XmField(w,XmColorS_offsets,XmColorSelector,f,t)

/*
 * Special handling for array members.  The XmField macro can't deal with
 * them
 */
#define XmColorSArrayField(w, field, type)                 \
    ((type*)((char*)w + XmColorS_offsets[XmColorSelectorIndex]  \
            + XtOffsetOf(XmColorSelectorPart, field)))

#define XmColorS_color_mode(w) XmColorSField(w, color_mode, XmColorMode)
#define XmColorS_color_name(w) XmColorSField(w, color_name, char*)
#define XmColorS_rgb_file(w) XmColorSField(w, rgb_file, String)
#define XmColorS_margin_width(w) XmColorSField(w, margin_width, Dimension)
#define XmColorS_margin_height(w) XmColorSField(w, margin_height, Dimension)
#define XmColorS_strings(w) XmColorSField(w, strings, ColorSelStrings)
#define XmColorS_slider_red(w) XmColorSField(w, slider_red, int)
#define XmColorS_slider_green(w) XmColorSField(w, slider_green, int)
#define XmColorS_slider_blue(w) XmColorSField(w, slider_blue, int)
#define XmColorS_bb(w) XmColorSField(w, bb, Widget)
#define XmColorS_sliders(w) XmColorSArrayField(w, sliders, Widget)
#define XmColorS_scrolled_list(w) XmColorSField(w, scrolled_list, Widget)
#define XmColorS_list(w) XmColorSField(w, list, Widget)
#define XmColorS_color_window(w) XmColorSField(w, color_window, Widget)
#define XmColorS_chose_radio(w) XmColorSField(w, chose_radio, Widget)
#define XmColorS_chose_mode(w) XmColorSArrayField(w, chose_mode, Widget)
#define XmColorS_color_pixel(w) XmColorSField(w, color_pixel, Pixel)
#define XmColorS_good_cell(w) XmColorSField(w, good_cell, Boolean)
#define XmColorS_colors(w) XmColorSField(w, colors, ColorInfo*)
#define XmColorS_num_colors(w) XmColorSField(w, num_colors, short)

/************************************************************
 *      GLOBAL DECLARATIONS
 *************************************************************/

/************************************************************
 *       EXTERNAL DECLARATIONS
 ************************************************************/

extern XmColorSelectorClassRec	xiColorSelectorClassRec;

/************************************************************
 *       STATIC DECLARATIONS
 ************************************************************/

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmColorSelectP_h DON'T ADD STUFF AFTER THIS #endif */

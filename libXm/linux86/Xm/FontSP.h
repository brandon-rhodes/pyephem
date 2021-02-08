/*
 *    Copyright 1992, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef	_XmFontSelectorP_h
#define	_XmFontSelectorP_h

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

/************************************************************
 *      INCLUDE FILES
 *************************************************************/
#include <Xm/ManagerP.h>
#include <Xm/PanedP.h>
#include <Xm/FontS.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmFontSelectorIndex (XmPanedIndex + 1)

extern XmOffsetPtr XmFontS_offsets;
extern XmOffsetPtr XmFontSC_offsets;

#define FontSField(w,f,t) XmField(w, XmFontS_offsets, XmFontSelector, f, t)
#define XmFontS_current_font(w) FontSField(w, current_font, String)
#define XmFontS_show_font_name(w) FontSField(w, show_font_name, Boolean)
#define XmFontS_iso_fonts_only(w) FontSField(w, iso_fonts_only, Boolean)
#define XmFontS_use_scaling(w) FontSField(w, use_scaling, Boolean)
#define XmFontS_text_rows(w) FontSField(w, text_rows, Dimension)
#define XmFontS_font_changed_callback(w) FontSField(w, font_changed_callback, XtCallbackList)
#define XmFontS_strings(w) FontSField(w, strings, XmFontSelStringInfo)
#define XmFontS_xlfd_mode(w) FontSField(w, xlfd_mode, Boolean)
#define XmFontS_font_info(w) FontSField(w, font_info, FontInfo*)
#define XmFontS_text(w) FontSField(w, text, Widget)
#define XmFontS_name_label(w) FontSField(w, name_label, Widget)
#define XmFontS_middle_pane(w) FontSField(w, middle_pane, Widget)
#define XmFontS_family_box(w) FontSField(w, family_box, Widget)
#define XmFontS_size_box(w) FontSField(w, size_box, Widget)
#define XmFontS_bold_toggle(w) FontSField(w, bold_toggle, Widget)
#define XmFontS_italic_toggle(w) FontSField(w, italic_toggle, Widget)
#define XmFontS_xlfd_toggle(w) FontSField(w, xlfd_toggle, Widget)
#define XmFontS_other_toggle(w) FontSField(w, other_toggle, Widget)
#define XmFontS_show_font_toggle(w) FontSField(w, show_font_toggle, Widget)
#define XmFontS_use_scaling_toggle(w) FontSField(w, use_scaling_toggle, Widget)
#define XmFontS_option_menu(w) FontSField(w, option_menu, Widget)
#define XmFontS_encoding_menu_shell(w) FontSField(w, encoding_menu_shell, Widget)
#define XmFontS_xlfd_only(w) FontSField(w, xlfd_only, WidgetList)
#define XmFontS_xlfd_sensitive(w) FontSField(w, xlfd_sensitive, WidgetList)
#define XmFontS_num_xlfd_only(w) FontSField(w, num_xlfd_only, char)
#define XmFontS_num_xlfd_sensitive(w) FontSField(w, num_xlfd_sensitive, char)
#define XmFontS_alloc_xlfd_only(w) FontSField(w, alloc_xlfd_only, char)
#define XmFontS_alloc_xlfd_sensitive(w) FontSField(w, alloc_xlfd_sensitive, char)
#define XmFontS_user_state(w) FontSField(w, user_state, Flag)
#define XmFontS_current_text(w) FontSField(w, current_text, String)
#define XmFontS_get_font(w) FontSField(w, get_font, String)
#define XmFontS_encoding(w) FontSField(w, encoding, String)
#define XmFontS_old_fontlist(w) FontSField(w, old_fontlist, XmFontList)
#define XmFontS_old_fontdata(w) FontSField(w, old_fontdata, XFontStruct*)
#define XmFontS_dpi75(w) FontSField(w, dpi75, Widget)
#define XmFontS_dpi100(w) FontSField(w, dpi100, Widget)
#define XmFontS_dpiAny(w) FontSField(w, dpiAny, Widget)
#define XmFontS_proportional(w) FontSField(w, proportional, Widget)
#define XmFontS_monospace(w) FontSField(w, monospace, Widget)
#define XmFontS_any_spacing(w) FontSField(w, any_spacing, Widget)


#define SAMPLE_TEXT(fsw)        (XmFontS_strings((fsw)).sample_text)
#define ANY_STRING(fsw)		(XmFontS_strings((fsw)).any)
#define LOWER_ANY_STRING(fsw) 	(XmFontS_strings((fsw)).lower_any)
#define FAMILY_STRING(fsw)	(XmFontS_strings((fsw)).family)
#define SIZE_STRING(fsw) 	(XmFontS_strings((fsw)).size)
#define BOLD_STRING(fsw)	(XmFontS_strings((fsw)).bold)
#define ITALIC_STRING(fsw)	(XmFontS_strings((fsw)).italic)

#define OPTION_STRING(fsw)	(XmFontS_strings((fsw)).option)

#define BOTH_STRING(fsw)	(XmFontS_strings((fsw)).both)
#define MONO_SPACE_STRING(fsw) 	(XmFontS_strings((fsw)).mono_space)
#define PROPORTIONAL_STRING(fsw)(XmFontS_strings((fsw)).prop_space)

#define XLFD_STRING(fsw)	(XmFontS_strings((fsw)).xlfd)
#define OTHER_FONT_STRING(fsw)	(XmFontS_strings((fsw)).other_font)

#define DPI75_STRING(fsw)	(XmFontS_strings((fsw)).dpi_75)
#define DPI100_STRING(fsw)	(XmFontS_strings((fsw)).dpi_100)

#define SCALING_STRING(fsw)	  (XmFontS_strings((fsw)).scaling)
#define SHOW_NAME_STRING(fsw)	  (XmFontS_strings((fsw)).show_name)

#define ENCODING_ONLY_STRING(fsw) (XmFontS_strings((fsw)).encoding_only)
#define ENCODING_LIST(fsw)    (XmFontS_strings((fsw)).encoding_list)

#define ENCODING_STRING(fsw)    XmFontS_encoding((fsw))

/* this structure provides the table for font selection */

#define WEIGHT_LEN    15
#define SLANT_LEN      3
#define SPACING_LEN    3

typedef unsigned char Flag;
typedef unsigned int LongFlag;

#define FIXED  		((Flag) 1 << 0)
#define BOLD   		((Flag) 1 << 1)
#define ITALIC 		((Flag) 1 << 2)
#define PROPORTIONAL 	((Flag) 1 << 3)
#define SCALED_75       ((Flag) 1 << 4)
#define SCALED_100	((Flag) 1 << 5)
#define DPI_75 		((Flag) 1 << 6)
#define DPI_100		((Flag) 1 << 7)

/*
 * This allows me to reuse flag bits in the user_state since
 * ISO is never set in the user state.
 */

#define USER_PROPORTIONAL   	PROPORTIONAL
#define USER_FIXED		FIXED

typedef struct _FontData {
    XrmQuark familyq;		/* quarkified family name. */
    XrmQuark weightq;		/* quarkified weight name. */
    char slant[SLANT_LEN + 1];
    char spacing[SPACING_LEN + 1];
    short resolution_x, resolution_y;
    short point_size;
    XrmQuark encoding;
    Flag state;
} FontData;

typedef struct FamilyInfo {
    XrmQuark nameq;		/* quarkified family name. */
    XrmQuark bold_nameq, medium_nameq;
    XrmQuark italic_nameq, upright_nameq;
    char fixed_spacing[SPACING_LEN + 1];
    LongFlag sizes_75, sizes_100;
    Flag state;
    XrmQuark *encodings;
    int encoding_alloc;
} FamilyInfo;

typedef struct FontInfo {
    FontData *current_font;
    String *others;
    FamilyInfo *family_info;
    short num_others;
    short num_families;
    short resolution;
} FontInfo;

typedef struct _XmFontSelStringInfo {
    XmString sample_text;
    XmString any, lower_any;
    XmString family, size;
    XmString bold, italic, option, both;
    XmString mono_space, prop_space;
    XmString xlfd, other_font;
    XmString dpi_75, dpi_100;
    XmString scaling, encoding_only, show_name;

    String *encoding_list;
} XmFontSelStringInfo;

/************************************************************
 *      TYPEDEFS AND DEFINES
 *************************************************************/

typedef struct _FontSelectorClassPart
{
    XtPointer extension;	/* For later use. */
} FontSelectorClassPart;

typedef struct _XmFontSelectorClassRec
{
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart 	constraint_class;
    XmManagerClassPart  	manager_class;
    XmPanedClassPart		paned_class;
    FontSelectorClassPart	font_selector_class;
} XmFontSelectorClassRec;

typedef struct _XmFontSelectorPart
{	
    /* Resources */
    String		current_font;   /* The currently shown font. */

    Boolean 		show_font_name;	/* show the font name window? */
    Boolean		iso_fonts_only;	/* show only iso fonts? */
    Boolean		use_scaling;    /* use font scaling? */
    
    Dimension		text_rows; 	/* The number of text rows to display*/

    XtCallbackList	font_changed_callback; /* when font changes. */

    XmFontSelStringInfo strings; /* The font selectors external strings. */

    /* Private Data */

    Boolean xlfd_mode;		/* True if we are int xlfd mode. */

    FontInfo 		*font_info;		/* The font information. */
    Widget text, name_label;	/* Text and label widget below top area. */
    Widget middle_pane;		/* The option info middle pane. */
    Widget family_box;		/* The family choices combo box. */
    Widget size_box;		/* The family choices combo box. */
    Widget bold_toggle, italic_toggle; /* The bold and italic toggle buttons */
    Widget xlfd_toggle, other_toggle; /* The xlfd and other font toggles. */
    Widget show_font_toggle, use_scaling_toggle;
    Widget option_menu;		/* The option menu. */
    Widget encoding_menu_shell;	/* The menu shell associated with the */
    				/* encoding menu. */

    WidgetList xlfd_only;	/* Only visable when in xlfd mode. */
    WidgetList xlfd_sensitive;	/* Only sensitive when in xlfd mode. */

    char num_xlfd_only, num_xlfd_sensitive;
    char alloc_xlfd_only, alloc_xlfd_sensitive;

    Flag user_state;		/* The current user selections. */

    String current_text;	/* The current text in the text widget. */
    String get_font;		/* Where to store returned get values on
				   current_font. */
    String encoding;		/* The encoding, may change. */

    /* 
     * The previously set font_data, free when font changed, or widget
     * destroyed. 
     */

    XmFontList old_fontlist;	
    XFontStruct * old_fontdata;

    /*
     * These values were added 2/1/94
     */
    Widget dpi75, dpi100, dpiAny; /* DPI toggle buttons.		*/
    Widget proportional, monospace, any_spacing;
                                /* Spacing toggle buttons		*/
} XmFontSelectorPart;

typedef struct _XmFontSelectorRec
{
    CorePart		core;
    CompositePart 	composite;
    ConstraintPart 	constraint;
    XmManagerPart 	manager;
    XmPanedPart 	paned;
    XmFontSelectorPart	fs;
} XmFontSelectorRec;

typedef struct _XmFontSelectorConstraintsPart
{
    XtPointer dummy;
} XmFontSelectorConstraintsPart;

typedef struct _XmFontSelectorConstraintsRec
{
    XmManagerConstraintPart       manager;
    XmPanedConstraintsPart        paned;
    XmFontSelectorConstraintsPart fs;
} XmFontSelectorConstraintsRec, *XmFontSelectorConstraints;

/************************************************************
 *      MACROS
 *************************************************************/

/************************************************************
 *      GLOBAL DECLARATIONS
 *************************************************************/

/************************************************************
 *       EXTERNAL DECLARATIONS
 ************************************************************/

/************************************************************
 *       STATIC DECLARATIONS
 ************************************************************/

extern XmFontSelectorClassRec	xmFontSelectorClassRec;

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif	/* _XmFontSelectorP_h - DON'T ADD STUFF AFTER THIS #endif */

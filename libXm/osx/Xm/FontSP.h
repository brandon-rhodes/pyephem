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

#define XmFontS_current_font(w) (((XmFontSelectorWidget)(w))->fs.current_font)
#define XmFontS_show_font_name(w) (((XmFontSelectorWidget)(w))->fs.show_font_name)
#define XmFontS_iso_fonts_only(w) (((XmFontSelectorWidget)(w))->fs.iso_fonts_only)
#define XmFontS_use_scaling(w) (((XmFontSelectorWidget)(w))->fs.use_scaling)
#define XmFontS_text_rows(w) (((XmFontSelectorWidget)(w))->fs.text_rows)
#define XmFontS_font_changed_callback(w) (((XmFontSelectorWidget)(w))->fs.font_changed_callback)
#define XmFontS_strings(w) (((XmFontSelectorWidget)(w))->fs.strings)
#define XmFontS_xlfd_mode(w) (((XmFontSelectorWidget)(w))->fs.xlfd_mode)
#define XmFontS_font_info(w) (((XmFontSelectorWidget)(w))->fs.font_info)
#define XmFontS_text(w) (((XmFontSelectorWidget)(w))->fs.text)
#define XmFontS_name_label(w) (((XmFontSelectorWidget)(w))->fs.name_label)
#define XmFontS_middle_pane(w) (((XmFontSelectorWidget)(w))->fs.middle_pane)
#define XmFontS_family_box(w) (((XmFontSelectorWidget)(w))->fs.family_box)
#define XmFontS_size_box(w) (((XmFontSelectorWidget)(w))->fs.size_box)
#define XmFontS_bold_toggle(w) (((XmFontSelectorWidget)(w))->fs.bold_toggle)
#define XmFontS_italic_toggle(w) (((XmFontSelectorWidget)(w))->fs.italic_toggle)
#define XmFontS_xlfd_toggle(w) (((XmFontSelectorWidget)(w))->fs.xlfd_toggle)
#define XmFontS_other_toggle(w) (((XmFontSelectorWidget)(w))->fs.other_toggle)
#define XmFontS_show_font_toggle(w) (((XmFontSelectorWidget)(w))->fs.show_font_toggle)
#define XmFontS_use_scaling_toggle(w) (((XmFontSelectorWidget)(w))->fs.use_scaling_toggle)
#define XmFontS_option_menu(w) (((XmFontSelectorWidget)(w))->fs.option_menu)
#define XmFontS_encoding_menu_shell(w) (((XmFontSelectorWidget)(w))->fs.encoding_menu_shell)
#define XmFontS_xlfd_only(w) (((XmFontSelectorWidget)(w))->fs.xlfd_only)
#define XmFontS_xlfd_sensitive(w) (((XmFontSelectorWidget)(w))->fs.xlfd_sensitive)
#define XmFontS_num_xlfd_only(w) (((XmFontSelectorWidget)(w))->fs.num_xlfd_only)
#define XmFontS_num_xlfd_sensitive(w) (((XmFontSelectorWidget)(w))->fs.num_xlfd_sensitive)
#define XmFontS_alloc_xlfd_only(w) (((XmFontSelectorWidget)(w))->fs.alloc_xlfd_only)
#define XmFontS_alloc_xlfd_sensitive(w) (((XmFontSelectorWidget)(w))->fs.alloc_xlfd_sensitive)
#define XmFontS_user_state(w) (((XmFontSelectorWidget)(w))->fs.user_state)
#define XmFontS_current_text(w) (((XmFontSelectorWidget)(w))->fs.current_text)
#define XmFontS_get_font(w) (((XmFontSelectorWidget)(w))->fs.get_font)
#define XmFontS_encoding(w) (((XmFontSelectorWidget)(w))->fs.encoding)
#define XmFontS_old_fontlist(w) (((XmFontSelectorWidget)(w))->fs.old_fontlist)
#define XmFontS_old_fontdata(w) (((XmFontSelectorWidget)(w))->fs.old_fontdata)
#define XmFontS_dpi75(w) (((XmFontSelectorWidget)(w))->fs.dpi75)
#define XmFontS_dpi100(w) (((XmFontSelectorWidget)(w))->fs.dpi100)
#define XmFontS_dpiAny(w) (((XmFontSelectorWidget)(w))->fs.dpiAny)
#define XmFontS_proportional(w) (((XmFontSelectorWidget)(w))->fs.proportional)
#define XmFontS_monospace(w) (((XmFontSelectorWidget)(w))->fs.monospace)
#define XmFontS_any_spacing(w) (((XmFontSelectorWidget)(w))->fs.any_spacing)


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

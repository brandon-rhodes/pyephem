#ifndef _XmDropDown_h_
#define _XmDropDown_h_

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/ManagerP.h>
#include <Xm/DropDown.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIX_1446

#define XmDropDown_UP 		((unsigned char) 0)
#define XmDropDown_UNPOSTED	XmDropDown_UP
#define XmDropDown_DOWN        	((unsigned char) 1)
#define XmDropDown_POSTED		XmDropDown_DOWN        	
#define XmDropDown_IN_PROGRESS 	((unsigned char) 2)
#define XmDropDown_BEGIN_POPUP_FROM_TEXT  ((unsigned char) 3)
#ifdef FIX_1446
#define XmDropDown_AFTER_UNPOST 	((unsigned char) 4)
#endif
#define XmDropDown_h_space(w)   (((XmDropDownWidget)(w))->combo.h_space)
#define XmDropDown_v_space(w)   (((XmDropDownWidget)(w))->combo.v_space)
#define XmDropDown_popup_offset(w)  \
   (((XmDropDownWidget)(w))->combo.popup_offset)
#define XmDropDown_verify(w)    (((XmDropDownWidget)(w))->combo.verify)
#define XmDropDown_editable(w)  (((XmDropDownWidget)(w))->combo.editable)
#define XmDropDown_show_label(w)    \
    (((XmDropDownWidget)(w))->combo.show_label)
#define XmDropDown_customized_combo_box(w)  \
   (((XmDropDownWidget)(w))->combo.customized_combo_box)
#define XmDropDown_use_text_field(w)    \
   (((XmDropDownWidget)(w))->combo.use_text_field)
#define XmDropDown_popup_shell(w)   (((XmDropDownWidget)(w))->combo.popup_shell)
#define XmDropDown_popup_cursor(w)  \
    (((XmDropDownWidget)(w))->combo.popup_cursor)
#define XmDropDown_translations(w)  \
    (((XmDropDownWidget)(w))->combo.translations)
#define XmDropDown_verify_text_callback(w)  \
   (((XmDropDownWidget)(w))->combo.verify_text_callback)
#define XmDropDown_verify_text_failed_callback(w)   \
   (((XmDropDownWidget)(w))->combo.verify_text_failed_callback)
#define XmDropDown_update_text_callback(w)  \
   (((XmDropDownWidget)(w))->combo.update_text_callback)
#define XmDropDown_update_shell_callback(w) \
   (((XmDropDownWidget)(w))->combo.update_shell_callback)
#define XmDropDown_visible_items(w) \
   (((XmDropDownWidget)(w))->combo.visible_items)
#define XmDropDown_new_visual_style(w)  \
   (((XmDropDownWidget)(w))->combo.new_visual_style)

#define XmDropDown_old_text(w)      (((XmDropDownWidget)(w))->combo.old_text)
#define XmDropDown_focus_owner(w)   (((XmDropDownWidget)(w))->combo.focus_owner)
#define XmDropDown_focus_state(w)   (((XmDropDownWidget)(w))->combo.focus_state)
#define XmDropDown_list_state(w)    (((XmDropDownWidget)(w))->combo.list_state)
#define XmDropDown_text_x(w)        (((XmDropDownWidget)(w))->combo.text_x)
#define XmDropDown_list(w)          (((XmDropDownWidget)(w))->combo.list)
#define XmDropDown_label(w)         (((XmDropDownWidget)(w))->combo.label)
#define XmDropDown_text(w)          (((XmDropDownWidget)(w))->combo.text)
#define XmDropDown_arrow(w)         (((XmDropDownWidget)(w))->combo.arrow)

#define XmDropDown_autoTraversal(w) (((XmDropDownWidget)(w))->combo.autoTraversal)
#define XmDropDown_activateOnFill(w)    \
    (((XmDropDownWidget)(w))->combo.activateOnFill)
#define XmDropDown_doActivate(w)    (((XmDropDownWidget)(w))->combo.doActivate)
#define XmDropDown_inValueChanged(w)    \
    (((XmDropDownWidget)(w))->combo.inValueChanged)


/* Should return True to ignore invalid entry warning. Combination Box
 *  does not currently use this. Presumes do it in subclasses 
 */
typedef Boolean (*XmDropDownTextProc)(Widget w, char *text);

typedef Boolean (*XmDropDownTextListMapProc)(
    Widget w,		/* combo box */
    Widget text,	/* text */
    Widget list		/* list */
);

/* Version number for the first Revision  */
#define XmDropDownExtensionVersion 2

typedef struct {
        /* standard extension fields */
	XtPointer 		    next_extension;
	XrmQuark                    record_type;
	long                        version;
	Cardinal                    record_size;

	/* extra fields */
	XmDropDownTextProc	    verify;
	XmDropDownTextProc	    update;
	XmDropDownTextListMapProc   setTextFromList;
	XmDropDownTextListMapProc   setListFromText;
} XmDropDownClassPartExtension;

typedef struct {
    XtPointer extension;	/* Just in case we need it later. */
} XmDropDownClassPart;

typedef struct _XmDropDownClassRec {
    CoreClassPart	    core_class;
    CompositeClassPart	    composite_class;
    ConstraintClassPart	    constraint_class;
    XmManagerClassPart	    manager_class;
    XmDropDownClassPart     combo_class;
} XmDropDownClassRec;

typedef struct {
    /* resources */

    Dimension h_space;		/* The amount of space to leave between */
    Dimension v_space;		/* widgets and the box edges. */
    int popup_offset;		/* The offset of the popup offset from the
				   left edge of the text widget. */
    Boolean verify;		/* Verify the contents of the Text widget
				   on leave or CR when this is True. */
    Boolean editable;		/* Allow the text field to be edited? */
    Boolean show_label;		/* Whether or not to show the label. */
    Boolean customized_combo_box; /* Is this a customized combo box. */
    Boolean use_text_field; /* Use XmTextField of XmText for textual input */
    Widget popup_shell;		/* The id of the popup shell. */
    Cursor popup_cursor;	/* Cursor for the Popup Window.  */
    XtTranslations translations; /* The translation table for all children. */

    /*
     * Callbacks to verify, and update the text and shell widgets.
     */

    XtCallbackList verify_text_callback;
    XtCallbackList verify_text_failed_callback;
    XtCallbackList update_text_callback;
    XtCallbackList update_shell_callback;

    /* private state */

    String old_text;		/* The old text value. */
    Window focus_owner;		/* Previous owner and state of the focus. */
    int focus_state;

    unsigned char list_state;	/* XmDropDown_UP, XmDropDown_DOWN or XmDropDown_IN_PROGRESS. */

    Position text_x;		/* X location of the text widget. */

    Widget list;		/* List contained in the popup shell. */

    Widget label;		/* The three children of the combo box. */
    Widget text; 
    Widget arrow;

    int visible_items;		/* only to set/get XmNvisibleItemCount, which is
				** a sop for non-customized combobox users */
    
    Boolean new_visual_style;

    Boolean autoTraversal;	/* traverse next on return */
    int	activateOnFill;		/* activate when we fill this many chars */
    Boolean doActivate;		/* do activate on next value changed */
    Boolean inValueChanged;	/* recursion prevention */

    Widget		vsb;
    Widget		hsb;
    Boolean		scrolling;
} XmDropDownPart;

typedef struct _XmDropDownRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    XmDropDownPart combo;
} XmDropDownRec;

extern XmDropDownClassRec xmDropDownClassRec;

#ifdef __cplusplus
}	/* Closes scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmDropDownP_h_ */

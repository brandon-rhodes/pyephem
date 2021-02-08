/*
 *    Copyright 1990, Integrated Computer Solutions, Inc.
 *
 *		       All Rights Reserved.
 *
 * AUTHOR: Chris D. Peterson
 *
 */

#ifndef _XmExt_h_
#define _XmExt_h_


/************************************************************
*	INCLUDE FILES
*************************************************************/

#include <Xm/Xm.h>

#if defined(__cplusplus)
extern "C" {
#endif



typedef Widget (*XmWidgetFunc)(Widget);


/************************************************************
*	TYPEDEFS AND DEFINES
*************************************************************/

/*
 * General name definitions.
 */


#define XmNbadActionParameters "badActionParameters"
#define XmNbadActionParametersMsg \
    "Bad parameters passed to an action routine of widget '%s'."

#define XmNbadMotionParams "badMotionParams"
#define XmNbadMotionParamsMsg \
"%s - Motion Action : must have exactly 1 parameter, either '%s' or '%s'."

#define XmNbadRowPixmap "badRowPixmap"
#define XmNbadRowPixmapMsg "Extended List: Row pixmaps must have depth of 1."

#define XmNbadXlfdFont "badXlfdFont"
#define XmNbadXlfdFontMsg \
"%s: All Xlfd fonts must contain 14 hyphens\n'%s' is not valid."

#define XmNcellNotEmpty "cellNotEmpty"
#define XmNcellNotEmptyMsg "XmIconBox: Cell %s is not empty"

#define XmNcolorNameTooLong "colorNameTooLong"
#define XmNcolorNameTooLongMsg \
  "%s: Color name '%s' is too long, truncated to '%s'."

#define XmNcontextSaveFailed "contextSaveFailed"
#define XmNcontextSaveFailedMsg "Internal Error: Could not save context data."

#define XmNconversionFailure "conversionFailure"
#define XmNconversionFailureMsg \
	"%s: Unable to perform string to %s conversion."

#define XmNcouldNotFindFamilyData "couldNotFindFamilyData"
#define XmNcouldNotFindFamilyDataMsg \
  "%s: Could not find family data for family '%s'."

#define XmNforceGreaterThanZero "forceGreaterThanZero"
#define XmNforceGreaterThanZeroMsg \
"%s : %s must be greater than zero being reset to one (1)."
    
#define XmNinsertBeforeNotSibling "insertBeforeNotSibling"
#define XmNinsertBeforeNotSiblingMsg "XmHierarchy: InsertBefore Widget\
 is not a sibling of '%s'.\nInserting child at end of list."
    
#define XmNnoComboShell "noComboShell"
#define XmNnoComboShellMsg \
    "Combination Box: When using a custom combo box a shell must be provided."

#define XmNnoEmptyCells "noEmptyCells"
#define XmNnoEmptyCellsMsg "XmIconBox: Could not find any empty cells."

#define XmNnoGadgetSupport "noGadgetSupport"
#define XmNnoGadgetSupportMsg "Widget %s does not support gadget children."
    
#define XmNpixEditBadImageCreate "pixEditBadImageCreate"
#define XmNpixEditBadImageCreateMsg "Pixmap Editor: Can't allocate image data"

#define XmNsameAsImageOrPix "sameAsImageOrPix"
#define XmNsameAsImageOrPixMsg \
"%s : The bitmapMode resource can only be changed at the same time as the pixmap or image"

#define XmNselfOrOutsideOfApplicationDrop "selfOrOutsideOfApplicationDrop"
#define XmNselfOrOutsideOfApplicationDropMsg \
    "Attempt to drop into illegal object."

#define XmNstaticResource "staticResource"
#define XmNstaticResourceMsg \
    "The resource '%s' may not be changed dynamically."

#define XmNtextVerifyFailed "textVerifyFailed"
#define XmNtextVerifyFailedMsg "Combination Box: Text item validation failed."

#define XmNunexpectedEvent "unexpectedEvent"
#define XmNunexpectedEventMsg "%s: Unexpected Event Type %s.\n"

#define XmNunparsableColor "unparsableColor"
#define XmNunparsableColorMsg \
  "%s: Could not parse the color name '%s'."

#define XmNnodeParentIsSelf "nodeParentIsSelf"
#define XmNnodeParentIsSelfMsg \
  "%s: The node parent cannot be self referential."

#define XmNstringGetFailed "stringGetFailed"
#define XmNstringGetFailedMsg "%s: XmGetStringLToR Failed."

#define XmCICSWidgetSetError "ICSWidgetSetError"

/************************************************************
*	MACROS
*************************************************************/

/************************************************************
*	GLOBAL DECLARATIONS
*************************************************************/

/************************************************************
*	EXTERNAL DECLARATIONS
*************************************************************/
	  

void XmCopyISOLatin1Lowered(char *, char *);

int XmCompareISOLatin1(char*, char*);

Boolean XmCompareXtWidgetGeometryToWidget(XtWidgetGeometry*, Widget);

Boolean XmCompareXtWidgetGeometry(XtWidgetGeometry*, XtWidgetGeometry*);


#if defined(__cplusplus)
}
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* __Ext_h__ */

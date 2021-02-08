#ifndef _SKYTOOLBAR_H
#define _SKYTOOLBAR_H

/* glue for skyview tool bar */

#define	SVTB_NHIST	4	/* number of History references */

extern void svtb_create (Widget trc_w, Widget lrc_w, Widget rrc_w);
extern void svtb_newpm (Widget rc_w);

extern void svtb_brighter_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_constel_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_dimmer_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_grid_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_automag_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_hzn_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_fstars_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_names_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_planes_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_magscale_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_orient_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_proj_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_flip_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_print_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_zoomin_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_unzoom_cb (Widget w, XtPointer client, XtPointer call);
extern void svtb_report_cb (Widget w, XtPointer client, XtPointer call);

extern void svtb_imMode (int on);
extern void svtb_zoomok (int whether);
extern void svtb_unzoomok (int whether);
extern int svtb_iszoomok (void);
extern void svtb_updateCTTT (char fclass_table[NCLASSES],
    char type_table[NOBJTYPES]);
extern void svtb_updateHorizon (int on);
extern void svtb_updateFStars (int on);
extern void svtb_updateGrid (int on);
extern void svtb_updateLRFlip (int on);
extern void svtb_updateTBFlip (int on);
extern void svtb_updateAutoMag (int on);
extern void svtb_updateCyl (int on);
extern void svtb_updateCns (int on);
extern void svtb_updateNames (int on);
extern void svtb_updatePlanes (int on);
extern void svtb_updateMagScale (int on);
extern void svtb_updateSlice (int on);
extern void svtb_updateROI (int on);
extern void svtb_updateGauss (int on);
extern void svtb_updateContrast (int on);
extern void svtb_updateGlass (int on);
extern void svtb_updateReport (int on);

extern int svtb_glassIsOn (void);
extern int svtb_reportIsOn (void);
extern int svtb_snapIsOn (void);
extern int svtb_monumentIsOn (void);
extern int svtb_sliceIsOn (void);
extern int svtb_ROIIsOn (void);
extern int svtb_glassIsOn (void);
extern int svtb_gaussIsOn (void);

extern void si_updateGauss (int on);
extern void si_updateROI (int on);
extern void si_updateContrast (int on);
extern void si_updateSlice (int on);
extern void si_updateGlass (int on);

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: skytoolbar.h,v $ $Date: 2008/08/17 20:18:31 $ $Revision: 1.21 $ $Name:  $
 */

#endif /* _SKYTOOLBAR_H */

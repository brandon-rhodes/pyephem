#ifndef _DM_H
#define _DM_H

/* used by funcs who want to print like datamenu */

/* identifiers for each entry in a Data Table column.
 * N.B. these must match the order in the col[] array.
 */
typedef enum {
    CONSTEL_ID, RA_ID, HA_ID, GHA_ID, DEC_ID, AZ_ID, ALT_ID, ZEN_ID, PA_ID,
    JD_ID, HJD_ID, Z_ID, VMAG_ID,
    
    PMRA_ID, PMDEC_ID, SIZE_ID, PHS_ID, ELONG_ID, SPECT_ID, HLAT_ID, HLONG_ID,
    GLAT_ID, GLONG_ID,
    
    ECLAT_ID, ECLONG_ID, EDST_ID, ELGHT_ID, SDST_ID, SLGHT_ID, URANOM_ID,
    URAN2K_ID, MILLSA_ID,

    RSTIME_ID, RSAZ_ID, TRTIME_ID, TRALT_ID, TRAZ_ID, SETTIME_ID, SETAZ_ID, HRSUP_ID,

    SEP_ID,

    NDMCol
} DMCol;

extern int dm_colHeader (DMCol c, char str[]);
extern int dm_colFormat (Now *np, Obj *op, RiseSet *rp, DMCol c, char *str);


/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: dm.h,v $ $Date: 2013/01/06 01:27:18 $ $Revision: 1.10 $ $Name:  $
 */

#endif /* _DM_H */

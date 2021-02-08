#ifndef _SITES_H
#define _SITES_H

/* interface to e_read_sites(). */

/* this is to form a list of sites */
typedef struct {
    float si_lat;	/* lat (+N), rads */
    float si_lng;	/* long (+E), rads */
    float si_elev;	/* elevation above sea level, meters (-1 means ?) */
    char si_tzdefn[64];	/* timezone info.. same format as UNIX tzset(3) */
    char si_name[40];	/* name */
} Site;

extern void mm_setsite (Site *sp, int update);
extern void mm_sitename (char *name);
extern char *mm_getsite (void);

extern int sites_get_list (Site **sipp);
extern int sites_search (char *str);
extern void sites_manage (void);
extern void sites_abbrev (char *full, char ab[], int maxn);



/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: sites.h,v $ $Date: 2003/05/28 23:56:19 $ $Revision: 1.8 $ $Name:  $
 */

#endif /* _SITES_H */

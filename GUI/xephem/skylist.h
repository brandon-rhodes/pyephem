#ifndef _SKYLIST_H
#define _SKYLIST_H

/* include file to hook skyviewmenu.c and skylist.c together.
 */

/* Obj.flags or TSky flags values */
#define OBJF_ONSCREEN	FUSER0	/* bit set if obj is on screen */
#define OBJF_RLABEL	FUSER1	/* set if right-label is to be on */
#define OBJF_LLABEL	FUSER4	/* set if left-label is to be on */
#define OBJF_PERSLB	(OBJF_RLABEL|OBJF_LLABEL) /* either means persistent */
#define OBJF_NLABEL	FUSER5	/* set if name-label is to be on */
#define OBJF_MLABEL	FUSER6	/* set if magnitude-label is to be on */


extern void sl_manage(void);
extern void sl_unmanage(void);

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: skylist.h,v $ $Date: 2003/05/02 04:36:21 $ $Revision: 1.6 $ $Name:  $
 */

#endif /* _SKYLIST_H */

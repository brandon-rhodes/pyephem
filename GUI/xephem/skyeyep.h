#ifndef _SKYEPEP_H
#define _SKYEPEP_H

/* include file to hook skyviewmenu.c and skyeyep.c together.
 */

typedef struct {
    double altdec, azra;	/* location when created */
    double eyepw, eyeph;	/* width and height, rads */
    double eyepa;		/* rotation from Z if aadef else NCP, rads */
    int aamode;			/* whether defined as Alt/Az or else RA/Dec */
    int round;			/* true if want round, false if square */
    int solid;			/* true if want solid, else just border */
} EyePiece;

/* skyeyep.c */

extern void se_add (int aamode, double azra, double altdec);
extern void se_cursor (Cursor c);
extern int se_getlist (EyePiece **ep);
extern void se_unmanage(void);
extern void se_manage(void);
extern int se_isOneHere (int aamode, double azra, double altdec);
extern void se_del (int aamode, double azra, double altdec);


#endif /* _SKYEPEP_H */

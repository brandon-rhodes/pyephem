/* code to convert between .edb format and an Obj */

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#if defined(__STDC__)
#include <stdlib.h>
#include <string.h>
#endif

#include "P_.h"
#include "astro.h"
#include "circum.h"
#include "preferences.h"

extern void zero_mem P_((void *loc, unsigned len));
extern double atod P_((char *buf));

int get_fields P_((char *s, int delim, char *fields[]));
int db_set_field P_((char bp[], int id, PrefDateFormat pref, Obj *op));
int db_chk_planet P_((char name[], Obj *op));

#define MAXDBLINE       256     /* longest allowed db line */

#define FLDSEP          ','     /* major field separator */
#define SUBFLD          '|'     /* subfield separator */
#define MAXFLDS 20              /* must be more than on any expected line */

#define ASIZ(a) (sizeof(a)/sizeof(a[0]))

static int line_candidate P_((char *buf));
static void crack_year P_((char *bp, PrefDateFormat pref, double *p));
static int db_get_field P_((Obj *op, int id, char *bp));

/* crack the given .edb database line into op.
 * return 0 if ok else return -1.
 */
int
db_crack_line (s, op)
char s[];
Obj *op;
{
	char *flds[MAXFLDS];	/* point to each field for easy reference */
	char *sflds[MAXFLDS];	/* point to each sub field for easy reference */
	char copy[MAXDBLINE];	/* work copy; leave s untouched */
	int nf, nsf;		/* number of fields and subfields */
	int i;

	/* basic initial check */
	if (line_candidate (s) < 0)
	    return (-1);

	/* do all the parsing on a copy */
	(void) strcpy (copy, s);
	i = strlen(copy);
	if (copy[i-1] == '\n')
	    copy[i-1] = '\0';

	/* parse into main fields */
	nf = get_fields (copy, FLDSEP, flds);

	/* need at least 2: name and type */
	if (nf < 2)
	    return (-1);

	/* switch out on type of object - the second field */
	switch (flds[1][0]) {
	case 'f': {
	    static int ids[] = {F_RA, F_DEC, F_MAG};
	    if (nf < 5 || nf > 7)
		return (-1);
	    zero_mem ((void *)op, sizeof(ObjF));
	    op->o_type = FIXED;
	    nsf = get_fields(flds[1], SUBFLD, sflds);
	    if (nsf > 1 && db_set_field (sflds[1], F_CLASS, PREF_MDY, op) < 0)
		return (-1);
	    if (nsf > 2)
		(void) db_set_field (sflds[2], F_SPECT, PREF_MDY, op);
	    for (i = 2; i < ASIZ(ids)+2; i++)
		(void) db_set_field (flds[i], ids[i-2], PREF_MDY, op);
	    (void) db_set_field (nf>5 && flds[5][0] ? flds[5] : "2000",
							F_EPOCH, PREF_MDY, op);
	    if (nf == 7)
		(void) db_set_field (flds[6], F_SIZE, PREF_MDY, op);
	    break;
	}

	case 'e': {
	    static int ids[] = {E_INC, E_LAN, E_AOP, E_A, E_N, E_E, E_M,
						E_CEPOCH, E_EPOCH, E_M1, E_M2
	    };
	    if (nf != 13 && nf != 14)
		return (-1);
	    zero_mem ((void *)op, sizeof(ObjE));
	    op->o_type = ELLIPTICAL;
	    for (i = 2; i < ASIZ(ids)+2; i++)
		(void) db_set_field (flds[i], ids[i-2], PREF_MDY, op);
	    if (nf == 14)
		(void) db_set_field (flds[13], E_SIZE, PREF_MDY, op);
	    break;
	}

	case 'h': {
	    static int ids[]= {H_EP,H_INC,H_LAN,H_AOP,H_E,H_QP,H_EPOCH,H_G,H_K};
	    if (nf != 11 && nf != 12)
		return (-1);
	    zero_mem ((void *)op, sizeof(ObjH));
	    op->o_type = HYPERBOLIC;
	    for (i = 2; i < ASIZ(ids)+2; i++)
		(void) db_set_field (flds[i], ids[i-2], PREF_MDY, op);
	    if (nf == 12)
		(void) db_set_field (flds[11], H_SIZE, PREF_MDY, op);
	    break;
	}

	case 'p': {
	    static int ids[] = {P_EP,P_INC,P_AOP,P_QP,P_LAN,P_EPOCH,P_G,P_K};
	    if (nf != 10 && nf != 11)
		return (-1);
	    zero_mem ((void *)op, sizeof(ObjP));
	    op->o_type = PARABOLIC;
	    for (i = 2; i < ASIZ(ids)+2; i++)
		(void) db_set_field (flds[i], ids[i-2], PREF_MDY, op);
	    if (nf == 11)
		(void) db_set_field (flds[10], P_SIZE, PREF_MDY, op);
	    break;
	}

	case 'E': {
	    static int ids[] = {ES_EPOCH,ES_INC,ES_RAAN,ES_E,ES_AP,ES_M,ES_N,
							    ES_DECAY,ES_ORBIT};
	    if (nf != 11)
		return (-1);
	    zero_mem ((void *)op, sizeof(ObjES));
	    op->o_type = EARTHSAT;
	    for (i = 2; i < ASIZ(ids)+2; i++)
		(void) db_set_field (flds[i], ids[i-2], PREF_MDY, op);
	    break;
	}

	case 'P':
	    /* allow, though ignore, anything after the P */
	    return (db_chk_planet (flds[0], op));	/* does o_name too */

	default:
	    return (-1);
	}

	/* load up name */
	(void) db_set_field (flds[0], O_NAME, PREF_MDY, op);

	return (0);
}

/* write the given Obj in .edb format to lp[].
 * we do _not_ include a trailing '\n'.
 */
void
db_write_line (op, lp)
Obj *op;
char *lp;
{
	int i;

	switch (op->o_type) {
	case FIXED: {
	    static int ids[] = {F_CLASS, F_SPECT, F_RA, F_DEC, F_MAG, F_EPOCH,
		F_SIZE
	    };

	    sprintf (lp, "%s,f", op->o_name);
	    lp += strlen(lp);
	    for (i = 0; i < ASIZ(ids); i++)
		lp += db_get_field (op, ids[i], lp);
	    break;
	    }

	case ELLIPTICAL: {
	    static int ids[] = {E_INC, E_LAN, E_AOP, E_A, E_N, E_E, E_M,
					E_CEPOCH, E_EPOCH, E_M1, E_M2, E_SIZE
	    };

	    sprintf (lp, "%s,e", op->o_name);
	    lp += strlen(lp);
	    for (i = 0; i < ASIZ(ids); i++)
		lp += db_get_field (op, ids[i], lp);
	    break;
	    }

	case HYPERBOLIC: {
	    static int ids[]= {H_EP, H_INC, H_LAN, H_AOP, H_E, H_QP, H_EPOCH,
		H_G, H_K, H_SIZE
	    };

	    sprintf (lp, "%s,h", op->o_name);
	    lp += strlen(lp);
	    for (i = 0; i < ASIZ(ids); i++)
		lp += db_get_field (op, ids[i], lp);
	    break;
	    }

	case PARABOLIC: {
	    static int ids[] = {P_EP, P_INC, P_AOP, P_QP, P_LAN, P_EPOCH, P_G,
		P_K, P_SIZE
	    };

	    sprintf (lp, "%s,p", op->o_name);
	    lp += strlen(lp);
	    for (i = 0; i < ASIZ(ids); i++)
		lp += db_get_field (op, ids[i], lp);
	    break;
	    }

	case EARTHSAT: {
	    static int ids[] = {ES_EPOCH, ES_INC, ES_RAAN, ES_E, ES_AP, ES_M,
		ES_N, ES_DECAY,ES_ORBIT
	    };

	    sprintf (lp, "%s,E", op->o_name);
	    lp += strlen(lp);
	    for (i = 0; i < ASIZ(ids); i++)
		lp += db_get_field (op, ids[i], lp);
	    break;
	    }

	case PLANET:
	    sprintf (lp, "%s,P", op->o_name);
	    lp += strlen(lp);
	    break;

	default:
	    printf ("Unknown type for %s: %d\n", op->o_name, op->o_type);
	    exit(1);
	}
}

/* given a text buffer and a field id, and a PREF_DATE_FORMAT,
 *   set the corresponding member in *op.
 * return 0 if ok, else -1.
 */
int
db_set_field (bp, id, pref, op)
char bp[];
int id;
PrefDateFormat pref;
Obj *op;
{
	double tmp;

	/* include all the enums and in numeric order to give us the best
	 * possible chance the compiler will implement this as a jump table.
	 */
	switch (id) {
	case O_TYPE:
	    printf ("db_set_field: called with id==O_TYPE\n");
	    exit(1);
	    break;
	case O_NAME:
	    (void) strncpy (op->o_name, bp, sizeof(op->o_name)-1);
	    op->o_name[sizeof(op->o_name)-1] = '\0';
	    break;
	case F_RA:
	    f_scansex (radhr(op->f_RA), bp, &tmp);
	    op->f_RA = hrrad(tmp);
	    break;
	case F_DEC:
	    f_scansex (raddeg(op->f_dec), bp, &tmp);
	    op->f_dec = degrad(tmp);
	    break;
	case F_EPOCH:
	    tmp = op->f_epoch;
	    crack_year (bp, pref, &tmp);
	    op->f_epoch = tmp;
	    break;
	case F_MAG:
	    set_fmag (op, atod(bp));
	    break;
	case F_SIZE:
	    op->f_size = (short)floor(atod(bp) + 0.5);
	    {
		/* galaxies can have minor axis and position angle subfields */
		char *sflds[MAXFLDS];
		int nsf = get_fields(bp, SUBFLD, sflds);

		if (nsf == 3) {
		    set_ratio(op, op->s_size, atod(sflds[1]));
		    set_pa(op,degrad(atod(sflds[2])));
		} else {
		    set_ratio(op,1,1);	/* round */
		    set_pa(op,0.0);
		}
	    }
	    break;
	case F_CLASS:
	    switch (bp[0]) {
	    case 'A': case 'B': case 'C': case 'D': case 'F': case 'G':
	    case 'H': case 'K': case 'J': case 'L': case 'M': case 'N':
	    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
	    case 'U': case 'V':
		op->f_class = bp[0];
		break;
	    default:
		return (-1);
	    }
	    break;
	case F_SPECT: {
	    int i, j;
	    /* fill f_spect all the way */
	    for (i = j = 0; i < sizeof(op->f_spect); i++)
		if ((op->f_spect[i] = bp[j]) != 0)
		    j++;
	    break;
	}

	case E_INC:
	    op->e_inc = atod (bp);
	    break;
	case E_LAN:
	    op->e_Om = atod (bp);
	    break;
	case E_AOP:
	    op->e_om = atod (bp);
	    break;
	case E_A:
	    op->e_a = atod (bp);
	    break;
	case E_N:
	    /* retired */
	    break;
	case E_E:
	    op->e_e = atod (bp);
	    break;
	case E_M:
	    op->e_M = atod (bp);
	    break;
	case E_CEPOCH:
	    crack_year (bp, pref, &op->e_cepoch);
	    break;
	case E_EPOCH:
	    crack_year (bp, pref, &op->e_epoch);
	    break;
	case E_M1:
	    switch (bp[0]) {
	    case 'g':
		op->e_mag.whichm = MAG_gk;
		bp++;
		break;
	    case 'H':
		op->e_mag.whichm = MAG_HG;
		bp++;
		break;
	    default:
		/* leave type unchanged if no or unrecognized prefix */
		break;
	    }
	    op->e_mag.m1 = atod(bp);
	    break;
	case E_M2:
	    switch (bp[0]) {
	    case 'k':
		op->e_mag.whichm = MAG_gk;
		bp++;
		break;
	    case 'G':
		op->e_mag.whichm = MAG_HG;
		bp++;
		break;
	    default:
		/* leave type unchanged if no or unrecognized prefix */
		break;
	    }
	    op->e_mag.m2 = atod(bp);
	    break;
	case E_SIZE:
	    op->e_size = atod (bp);
	    break;

	case H_EP:
	    crack_year (bp, pref, &op->h_ep);
	    break;
	case H_INC:
	    op->h_inc = atod (bp);
	    break;
	case H_LAN:
	    op->h_Om = atod (bp);
	    break;
	case H_AOP:
	    op->h_om = atod (bp);
	    break;
	case H_E:
	    op->h_e = atod (bp);
	    break;
	case H_QP:
	    op->h_qp = atod (bp);
	    break;
	case H_EPOCH:
	    crack_year (bp, pref, &op->h_epoch);
	    break;
	case H_G:
	    op->h_g = atod (bp);
	    break;
	case H_K:
	    op->h_k = atod (bp);
	    break;
	case H_SIZE:
	    op->h_size = atod (bp);
	    break;

	case P_EP:
	    crack_year (bp, pref, &op->p_ep);
	    break;
	case P_INC:
	    op->p_inc = atod (bp);
	    break;
	case P_AOP:
	    op->p_om = atod (bp);
	    break;
	case P_QP:
	    op->p_qp = atod (bp);
	    break;
	case P_LAN:
	    op->p_Om = atod (bp);
	    break;
	case P_EPOCH:
	    crack_year (bp, pref, &op->p_epoch);
	    break;
	case P_G:
	    op->p_g = atod (bp);
	    break;
	case P_K:
	    op->p_k = atod (bp);
	    break;
	case P_SIZE:
	    op->p_size = atod (bp);
	    break;

	case ES_EPOCH:
	    crack_year (bp, pref, &op->es_epoch);
	    break;
	case ES_INC:
	    op->es_inc = atod (bp);
	    break;
	case ES_RAAN:
	    op->es_raan = atod (bp);
	    break;
	case ES_E:
	    op->es_e = atod (bp);
	    break;
	case ES_AP:
	    op->es_ap = atod (bp);
	    break;
	case ES_M:
	    op->es_M = atod (bp);
	    break;
	case ES_N:
	    op->es_n = atod (bp);
	    break;
	case ES_DECAY:
	    op->es_decay = atod (bp);
	    break;
	case ES_ORBIT:
	    op->es_orbit = (int) atod (bp);
	    break;

	default:
	    printf ("BUG! db_set_field: bad id: %d\n", id);
	    exit (1);
	}

	return (0);
}

/* given a null-terminated string, fill in fields[] with the starting addresses
 * of each field delimited by delim or '\0'.
 * N.B. each character matching delim is REPLACED BY '\0' IN PLACE.
 * N.B. 0-length fields count, so even if *s=='\0' we return 1.
 * return the number of fields.
 */
int
get_fields (s, delim, fields)
char *s;
int delim;
char *fields[];
{
	int n;
	char c;

	*fields = s;
	n = 0;
	do {
	    c = *s++;
	    if (c == delim || c == '\0') {
		s[-1] = '\0';
		*++fields = s;
		n++;
	    }
	} while (c);

	return (n);
}

/* check name for being a planet.
 * if so, fill in *op and return 0, else return -1.
 */
int
db_chk_planet (name, op)
char name[];
Obj *op;
{
	int i;

	/* these must match the order in astro.h */
	static char *planet_names[] = {
	    "Mercury", "Venus", "Mars", "Jupiter", "Saturn",
	    "Uranus", "Neptune", "Pluto", "Sun", "Moon",
	};

	for (i = MERCURY; i <= MOON; i++) {
	    if (strcmp (name, planet_names[i]) == 0) {
		zero_mem ((void *)op, sizeof(ObjPl));
		op->o_type = PLANET;
		(void) strcpy (op->o_name, planet_names[i]);
		op->pl.pl_code = i;
		return (0);
	    }
	}

	return (-1);
}

/* return 0 if buf qualifies as a database line worthy of a cracking
 * attempt, else -1.
 */
static int
line_candidate (buf)
char *buf;
{
	char c = buf[0];

	return (isalpha(c) || isdigit(c) ? 0 : -1);
}

/* given either a decimal year (xxxx[.xxx]) or a calendar (x/x/x)
 * and a DateFormat preference convert it to an mjd and store it at *p.
 */
static void
crack_year (bp, pref, p)
char *bp;
PrefDateFormat pref;
double *p;
{
	int m, y;
	double d;

	mjd_cal (*p, &m, &d, &y);	/* init with current */
	f_sscandate (bp, pref, &m, &d, &y);
	cal_mjd (m, d, y, p);
}

/* given an *op and a field id, add it to the given text buffer bp.
 * return the number of characters added to bp.
 */
static int
db_get_field (op, id, bp)
Obj *op;
int id;
char *bp;
{
	char *bpsave = bp;
	double tmp;

	/* include all the enums and in numeric order to give us the best
	 * possible chance the compiler will implement this as a jump table.
	 */
	switch (id) {
	case F_RA:
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    fs_sexa (bp, radhr(op->f_RA), 2, 36000);
	    bp += strlen(bp);
	    break;
	case F_DEC:
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    fs_sexa (bp, raddeg(op->f_dec), 3, 3600);
	    bp += strlen(bp);
	    break;
	case F_EPOCH:
	    mjd_year (op->f_epoch, &tmp);
	    sprintf (bp, ",%.7g", tmp);
	    bp += strlen(bp);
	    break;
	case F_MAG:
	    sprintf (bp, ",%6.2f", get_mag(op));
	    bp += strlen(bp);
	    break;
	case F_SIZE:
	    sprintf (bp, ",%d", op->f_size);
	    bp += strlen(bp);
	    if (op->f_ratio || op->f_pa) {
		sprintf (bp,"|%g|%g", op->f_size*get_ratio(op),
							    raddeg(get_pa(op)));
		bp += strlen(bp);
	    }
	    break;
	case F_CLASS:
	    if (op->f_class) {
		sprintf (bp, "|%c", op->f_class);
		bp += strlen(bp);
	    }
	    break;
	case F_SPECT:
	    if (op->f_spect[0] != '\0') {
		sprintf (bp, "|%c", op->f_spect[0]);
		bp += strlen(bp);
		if (op->f_spect[1] != '\0') {
		    sprintf (bp, "%c", op->f_spect[1]);
		    bp += strlen(bp);
		}
	    }
	    break;

	case E_INC:
	    sprintf (bp, ",%.7g", op->e_inc);
	    bp += strlen(bp);
	    break;
	case E_LAN:
	    sprintf (bp, ",%.7g", op->e_Om);
	    bp += strlen(bp);
	    break;
	case E_AOP:
	    sprintf (bp, ",%.7g", op->e_om);
	    bp += strlen(bp);
	    break;
	case E_A:
	    sprintf (bp, ",%.7g", op->e_a);
	    bp += strlen(bp);
	    break;
	case E_N:
	    /* retired */
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    break;
	case E_E:
	    sprintf (bp, ",%.7g", op->e_e);
	    bp += strlen(bp);
	    break;
	case E_M:
	    sprintf (bp, ",%.7g", op->e_M);
	    bp += strlen(bp);
	    break;
	case E_CEPOCH:
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    fs_date (bp, op->e_cepoch);
	    bp += strlen(bp);
	    break;
	case E_EPOCH:
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    fs_date (bp, op->e_epoch);
	    bp += strlen(bp);
	    break;
	case E_M1:
	    if (op->e_mag.whichm == MAG_gk) {
		sprintf (bp, ",g%.7g", op->e_mag.m1);
		bp += strlen(bp);
	    } else if (op->e_mag.whichm == MAG_HG) {
		sprintf (bp, ",H%.7g", op->e_mag.m1);
		bp += strlen(bp);
	    } else {
		sprintf (bp, ",%.7g", op->e_mag.m1);
		bp += strlen(bp);
	    }
	    break;
	case E_M2:
	    sprintf (bp, ",%.7g", op->e_mag.m2);
	    bp += strlen(bp);
	    break;
	case E_SIZE:
	    sprintf (bp, ",%.7g", op->e_size);
	    bp += strlen(bp);
	    break;

	case H_EP:
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    fs_date (bp, op->h_ep);
	    bp += strlen(bp);
	    break;
	case H_INC:
	    sprintf (bp, ",%.7g", op->h_inc);
	    bp += strlen(bp);
	    break;
	case H_LAN:
	    sprintf (bp, ",%.7g", op->h_Om);
	    bp += strlen(bp);
	    break;
	case H_AOP:
	    sprintf (bp, ",%.7g", op->h_om);
	    bp += strlen(bp);
	    break;
	case H_E:
	    sprintf (bp, ",%.7g", op->h_e);
	    bp += strlen(bp);
	    break;
	case H_QP:
	    sprintf (bp, ",%.7g", op->h_qp);
	    bp += strlen(bp);
	    break;
	case H_EPOCH:
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    fs_date (bp, op->h_epoch);
	    bp += strlen(bp);
	    break;
	case H_G:
	    sprintf (bp, ",%.7g", op->h_g);
	    bp += strlen(bp);
	    break;
	case H_K:
	    sprintf (bp, ",%.7g", op->h_k);
	    bp += strlen(bp);
	    break;
	case H_SIZE:
	    sprintf (bp, ",%.7g", op->h_size);
	    bp += strlen(bp);
	    break;

	case P_EP:
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    fs_date (bp, op->p_ep);
	    bp += strlen(bp);
	    break;
	case P_INC:
	    sprintf (bp, ",%.7g", op->p_inc);
	    bp += strlen(bp);
	    break;
	case P_AOP:
	    sprintf (bp, ",%.7g", op->p_om);
	    bp += strlen(bp);
	    break;
	case P_QP:
	    sprintf (bp, ",%.7g", op->p_qp);
	    bp += strlen(bp);
	    break;
	case P_LAN:
	    sprintf (bp, ",%.7g", op->p_Om);
	    bp += strlen(bp);
	    break;
	case P_EPOCH:
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    fs_date (bp, op->p_epoch);
	    bp += strlen(bp);
	    break;
	case P_G:
	    sprintf (bp, ",%.7g", op->p_g);
	    bp += strlen(bp);
	    break;
	case P_K:
	    sprintf (bp, ",%.7g", op->p_k);
	    bp += strlen(bp);
	    break;
	case P_SIZE:
	    sprintf (bp, ",%.7g", op->p_size);
	    bp += strlen(bp);
	    break;

	case ES_EPOCH:
	    sprintf (bp, ",");
	    bp += strlen(bp);
	    fs_date (bp, op->es_epoch);
	    bp += strlen(bp);
	    break;
	case ES_INC:
	    sprintf (bp, ",%.7g", op->es_inc);
	    bp += strlen(bp);
	    break;
	case ES_RAAN:
	    sprintf (bp, ",%.7g", op->es_raan);
	    bp += strlen(bp);
	    break;
	case ES_E:
	    sprintf (bp, ",%.7g", op->es_e);
	    bp += strlen(bp);
	    break;
	case ES_AP:
	    sprintf (bp, ",%.7g", op->es_ap);
	    bp += strlen(bp);
	    break;
	case ES_M:
	    sprintf (bp, ",%.7g", op->es_M);
	    bp += strlen(bp);
	    break;
	case ES_N:
	    sprintf (bp, ",%.7g", op->es_n);
	    bp += strlen(bp);
	    break;
	case ES_DECAY:
	    sprintf (bp, ",%.7g", op->es_decay);
	    bp += strlen(bp);
	    break;
	case ES_ORBIT:
	    sprintf (bp, ",%d", op->es_orbit);
	    bp += strlen(bp);
	    break;

	default:
	    printf ("BUG! db_set_field: bad id: %d\n", id);
	    exit (1);
	}

	return (bp - bpsave);
}

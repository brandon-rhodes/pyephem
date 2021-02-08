/* code to handle one request.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "xephem.h"

extern int fsfetch (Now *np, int want_ppm, int want_gsc, double ra,
    double dec, double fov, double mag, ObjF **opp);

#define	MAXFOV	degrad(15)	/* max allowed FOV, rads */

static void pr_info (Now *np, int omode, double ra, double dec, double rov);
static int typeOk (Obj *op, int types);
static void txt_report (Now *np, Obj *op, int topo);
static void txt_common (Now *np, Obj *op);
static void txt_fixed (Now *np, Obj *op);
static void txt_helio (Now *np, Obj *op);
static void txt_topo (Now *np, Obj *op);
static void txt_riset (Now *np, Obj *op);
static void edb_report (Now *np, Obj *op);

/* output mode bits */
#define	COLUMNAR	1
#define	TOPOCENTRIC	2
#define	APPARENT	4
#define	HEADER		8

Now *
mm_get_now()
{
	static Now now;
        return (&now);
}

/* crack the request and write the result to stdout.
 * return number of objects.
 * all errors go to stderr.
 * N.B. we assume ra/dec always come in as J2000
 */
int
handle_request (char *req, int nofs)
{
	Now *np = mm_get_now();
	DBScan dbs;
	ObjF *fstars;
	int nfstars;
        Obj *op;
	int outputmode;
	int types;
	int mask;
	double year;
	double ra;
	double dec;
	double fov;
	double mag;
	double lt;
	double lg;
	double el;
	double rov;
	double cdec;
	int nobjs;
	int n;

	/* basic cracking */
	lt = lg = el = 0.0;
	n = sscanf (req, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
	    &outputmode, &types, &year, &ra, &dec, &fov, &mag, &lt, &lg, &el);
	if (n != 7 && n != 10) {
	    fprintf (stderr, "Bad format: %s\n", req);
	    return(0);
	}
	if ((outputmode & (TOPOCENTRIC|APPARENT)) && n != 10) {
	    fprintf (stderr, "Missing topo data: %s\n", req);
	    return(0);
	}
	if (fov > MAXFOV)
	    fov = MAXFOV;
	rov = fov/2;
	cdec = cos(dec);

	/* fill in now */
	year_mjd (year, &year);
	mjd = year;
	lat = lt;
	lng = lg;
	tz = floor(-raddeg(lg)/15); /* rough, but helps rise/set */
	temp = 10.0;
	pressure = 1010.0;
	elev = el/ERAD;
	dip = 0.0;
	epoch = (outputmode & APPARENT) ? EOD : J2000;

	/* start with header if desired */
	if (outputmode & HEADER)
	    pr_info (np, outputmode, ra, dec, rov);

	/* set equatorial preference */
	pref_set (PREF_EQUATORIAL,
			    (outputmode&TOPOCENTRIC) ? PREF_TOPO : PREF_GEO);

	/* make ra/dec match n_epoch */
	if (epoch == EOD)
	    as_ap (np, J2000, &ra, &dec);

	/* set up the scan mask according to desired types */
	mask = 0;
	if (types &   1) mask |= PLANETM;
	if (types &   2) mask |= ELLIPTICALM;
	if (types &   4) mask |= PARABOLICM;
	if (types &   8) mask |= HYPERBOLICM;
	if (types & 240) mask |= FIXEDM | BINARYSTARM;	/* refined in typeOk */
	if ((types & 768) && !nofs) {
	    /* expects ra/dec to be at np->n_epoch, always returns J2000 */
	    nfstars = fsfetch (np, types&256, types&512, ra, dec, fov,
								mag, &fstars);
	} else {
	    fstars = NULL;
	    nfstars = 0;
	}

	/* scan and print the good stuff */
	nobjs = 0;
	for (db_scaninit(&dbs, mask, fstars, nfstars);
					    (op = db_scan (&dbs)) != NULL; ) {

	    if (!typeOk (op, types))
		continue;
	    obj_cir (np, op);
	    if (get_mag(op) > mag)
		continue;;

	    if (cdec*delra(ra - op->s_ra) > rov || fabs(dec - op->s_dec) > rov)
		continue;

	    /* ok! */
	    nobjs++;
	    if (outputmode & COLUMNAR)
		txt_report (np, op, outputmode & (TOPOCENTRIC|APPARENT));
	    else
		edb_report (np, op);
	}

	if (fstars)
	    free ((void *)fstars);

	return (nobjs);
}

static void
pr_info (Now *np, int omode, double ra, double dec, double rov)
{
	char str1[32], str2[32];

	fs_date (str1, PREF_YMD, mjd_day(mjd));
	fs_sexa (str2, mjd_hr(mjd), 2, 3600);
	printf ("UTC Time:  %s  %s  JD %13.5f\n", str1, str2, mjd+MJD0);

	if (omode & (TOPOCENTRIC|APPARENT)) {
	    fs_sexa (str1, raddeg(lat), 3, 3600);
	    fs_sexa (str2, raddeg(lng), 4, 3600);
	    printf ("Location: %sN %sE\n", str1, str2);
	}

	fs_sexa (str1, radhr(ra), 2, 36000);
	fs_sexa (str2, raddeg(dec), 3, 3600);
	printf ("   Field:  %s %s @ J2000, radius %.3f degrees\n", str1,
							    str2, raddeg(rov));

	printf ("  Output: %s %s\n",
		    omode&TOPOCENTRIC ? "Topocentric" : "Geocentric",
		    omode&APPARENT ? "Apparent @ EOD" : "Astrometric @ J2000");

	printf ("\n");
	printf ("Name          T Cns RA          Dec         Mag    Size HeLong    HeLat     EaDst  SnDst  Phs ");
	if (omode & (TOPOCENTRIC|APPARENT))
	    printf ("HA          Alt       Az        RiseTmRiseAz TrnTm TrnAl SetTm SetAz");
    printf ("\n");
    printf ("              f                                         C Sp                         MnAx PA\n");
	printf ("\n");
}

/* return !0 if op is one of types, else 0 */
static int
typeOk (Obj *op, int type)
{
	if (is_type(op, PLANETM))
	    return (type & 1);
	if (is_type(op, ELLIPTICALM))
	    return (type & 2);
	if (is_type(op, PARABOLICM))
	    return (type & 4);
	if (is_type(op, HYPERBOLICM))
	    return (type & 8);
	if (is_type(op, FIXEDM)) {
	    switch (op->f_class) {
	    case 'S': case 'B': case 'D': case 'M': case 'V': case 'T':
		return (type & (16+256+512));
	    case 'C': case 'O': case 'U':
		return (type & 32);
	    case 'G': case 'H': case 'A':
		return (type & 64);
	    default:
		return (type & 128);
	    }
	}
	if (is_type(op, BINARYSTARM))
	    return (type & 16);

	return (0);
}

static void
txt_report (Now *np, Obj *op, int topo)
{
	txt_common (np, op);
	if (op->o_type == FIXED)
	    txt_fixed (np, op);
	else
	    txt_helio (np, op);
	if (topo)
	    txt_topo (np, op);
	printf ("\n");
}

static void
txt_common (Now *np, Obj *op)
{
	char buf[32];
        char *name;
	int id;

	printf ("%-*s", MAXNM-1, op->o_name);
	switch (op->o_type) {
	case FIXED:      printf (" f"); break; 
	case ELLIPTICAL: printf (" e"); break; 
	case HYPERBOLIC: printf (" h"); break; 
	case PARABOLIC:  printf (" p"); break; 
	case PLANET:     printf (" P"); break; 
	default:
	    fprintf (stderr, "Bogus type for %s: %d\n", op->o_name, op->o_type);
	    exit(1);
	}

	id = cns_pick (op->s_ra, op->s_dec, epoch == EOD ? mjd : epoch);
	name = cns_name (id);
	printf (" %.3s", name);

	fs_sexa (buf, radhr(op->s_ra), 2, 360000);
	printf (" %s", buf);

	fs_sexa (buf, raddeg(op->s_dec), 3, 36000);
	printf (" %s", buf);

	printf (" %6.2f", get_mag (op));

	printf (" %4.0f", op->s_size);
}

static void
txt_fixed (Now *np, Obj *op)
{
	printf (" %c", op->f_class != '\0' ? op->f_class : ' ');

	if (op->f_spect[0] != '\0')
	    printf (" %c%c", op->f_spect[0],
				op->f_spect[1] != '\0' ? op->f_spect[1] : ' ');
	else
	    printf ("   ");

	printf ("%25s", "");	/* gap to align HA same as for planets */

	if (op->f_class == 'G' || op->f_class == 'H')
	    printf ("%4.0f %3.0f", (int)op->f_size*get_ratio(op),
							    raddeg(get_pa(op)));
	else
	    printf ("        ");
}

static void
txt_helio (Now *np, Obj *op)
{
	char buf[32];

	fs_sexa (buf, raddeg(op->s_hlong), 3, 3600);
	printf (" %s", buf);

	fs_sexa (buf, raddeg(op->s_hlat), 3, 3600);
	printf (" %s", buf);

	printf (" %6.3f", op->s_edist);

	printf (" %6.3f", op->s_sdist);

	printf (" %3.0f", op->s_phase);
}

static void
txt_topo (Now *np, Obj *op)
{
	double raeod = op->s_ra;
	double deceod = op->s_dec;
	double tmp;
	char buf[32];

	if (epoch != EOD) {
	    /* need apparent */
	    Obj o;
	    Now n;

	    (void) memcpy ((void *)&o, (void *)op, sizeof(Obj));
	    (void) memcpy ((void *)&n, (void *)np, sizeof(Now));
	    n.n_epoch = EOD;
	    (void) obj_cir (&n, &o);
	    raeod = o.s_ra;
	    deceod = o.s_dec;
	}

	now_lst (np, &tmp);
	tmp = tmp - radhr(raeod);	/* HA = LST - RA */
	if (tmp > 12)
	    tmp -= 24;
	if (tmp < -12)
	    tmp += 24;

	fs_sexa (buf, tmp, 3, 36000);
	printf (" %s", buf);

	fs_sexa (buf, raddeg(op->s_alt), 3, 3600);
	printf (" %s", buf);

	fs_sexa (buf, raddeg(op->s_az), 3, 3600);
	printf (" %s", buf);

	txt_riset (np, op);
}

static void
txt_riset (Now *np, Obj *op)
{
	RiseSet rs;
	double dis;
	char buf[32];

	/* semi-diameter displacement */
	dis = degrad(op->s_size/3600./2.0);
	riset_cir (np, op, dis, &rs);

	if (rs.rs_flags & RS_ERROR)
	    (void) strcpy (buf, "Error");
	else if (rs.rs_flags & RS_CIRCUMPOLAR)
	    (void) strcpy (buf, "CirPl");
	else if (rs.rs_flags & RS_NEVERUP)
	    (void) strcpy (buf, "NvrUp");
	else if (rs.rs_flags & RS_NORISE)
	    (void) strcpy (buf, "NoRis");
	else
	    fs_sexa (buf, mjd_hr(rs.rs_risetm), 2, 60);
	printf (" %s", buf);

	if (rs.rs_flags & RS_ERROR)
	    (void) strcpy (buf, " Error");
	else if (rs.rs_flags & RS_CIRCUMPOLAR)
	    (void) strcpy (buf, "CirPol");
	else if (rs.rs_flags & RS_NEVERUP)
	    (void) strcpy (buf, " NvrUp");
	else if (rs.rs_flags & RS_NORISE)
	    (void) strcpy (buf, "NoRise");
	else
	    fs_sexa (buf, raddeg(rs.rs_riseaz), 3, 60);
	printf (" %s", buf);

	if (rs.rs_flags & RS_ERROR)
	    (void) strcpy (buf, "Error");
	else if (rs.rs_flags & RS_NEVERUP)
	    (void) strcpy (buf, "NvrUp");
	else if (rs.rs_flags & RS_NOTRANS)
	    (void) strcpy (buf, "NoTrn");
	else
	    fs_sexa (buf, mjd_hr(rs.rs_trantm), 2, 60);
	printf (" %s", buf);

	if (rs.rs_flags & RS_ERROR)
	    (void) strcpy (buf, " Error");
	else if (rs.rs_flags & RS_NEVERUP)
	    (void) strcpy (buf, " NvrUp");
	else if (rs.rs_flags & RS_NOTRANS)
	    (void) strcpy (buf, "NoTran");
	else
	    fs_sexa (buf, raddeg(rs.rs_tranalt), 2, 60);
	printf (" %s", buf);

	if (rs.rs_flags & RS_ERROR)
	    (void) strcpy (buf, "Error");
	else if (rs.rs_flags & RS_CIRCUMPOLAR)
	    (void) strcpy (buf, "CirPl");
	else if (rs.rs_flags & RS_NEVERUP)
	    (void) strcpy (buf, "NvrUp");
	else if (rs.rs_flags & RS_NOSET)
	    (void) strcpy (buf, "NoSet");
	else
	    fs_sexa (buf, mjd_hr(rs.rs_settm), 2, 60);
	printf (" %s", buf);

	if (rs.rs_flags & RS_ERROR)
	    (void) strcpy (buf, " Error");
	else if (rs.rs_flags & RS_CIRCUMPOLAR)
	    (void) strcpy (buf, "CirPol");
	else if (rs.rs_flags & RS_NEVERUP)
	    (void) strcpy (buf, " NvrUp");
	else if (rs.rs_flags & RS_NOSET)
	    (void) strcpy (buf, " NoSet");
	else
	    fs_sexa (buf, raddeg(rs.rs_setaz), 3, 60);
	printf (" %s", buf);
}

static void
edb_report (Now *np, Obj *op)
{
	char line[1024];

	db_write_line (op, line);
	printf ("%s\n", line);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: request.c,v $ $Date: 2006/12/24 20:08:26 $ $Revision: 1.8 $ $Name:  $"};

/* print info about op at np, with local stuff if topo */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astro.h"

static void txt_common (Now *np, Obj *op);
static void txt_fixed (Now *np, Obj *op);
static void txt_helio (Now *np, Obj *op);
static void txt_topo (Now *np, Obj *op);
static void txt_riset (Now *np, Obj *op);

void
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

#if WANT_CONSTELLATION
	id = cns_pick (op->s_ra, op->s_dec, epoch == EOD ? mjd : epoch);
	name = cns_name (id);
	printf (" %.3s", name);
#else
	printf (" %.3s", "   ");
#endif

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


/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: print.c,v $ $Date: 2003/04/18 07:36:48 $ $Revision: 1.2 $ $Name:  $"};

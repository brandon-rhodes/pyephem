/* Fetch the desired region of xe3-formatted catalog stars.
 *
 * xe3 is a very compact format storing just ra/dec/mag/class for each star.
 * Each star occupies 3 bytes and stores RA to a precision of +/- .38 arcsecs,
 * Dec to .44 arcsec, Mag in the range 10..20 to .16 and 1 class bit set to 0
 * for star or 1 for other. One xe3 file stores all stars in a band of
 * declination one degree hi, beginning on an integral dec boundary. Thus 180
 * xe3 files are required to cover the entire sky.
 *
 * The overall file structure is an initial line of ASCII giving file type and
 * the base declination, followed by a table of 360 records indexed by RA in
 * whole degs giving the file offset and starting RA into the star records
 * for that RA, followed by any number of star records sorted by increasing RA.
 * Each star record occupies 3 bytes and stores the dec offset from the base
 * dec of the band, the RA offset from the RA of the previous record, the mag
 * and the class.
 *
 * The file starts with 8 ASCII chars including a final newline in the format
 * "xe3 HDD\n", where H is 'N' for north hemishere else 'S'. DD is the base
 * of the dec band, zero filled whole degrees. Dec offsets in the star
 * records are added to the base dec if H is 'N' else subtracted from base.
 *
 * Following the header is a table of 360 records each 8 bytes long. The array
 * is indexed by RA in whole degrees. Each index record contains a file offset
 * in bytes from the front of the file to the first star record with an RA
 * greater than or equal to the index RA value, and the RA of said star. Each
 * of these values is 4 bytes stored big-endian.
 *
 * Following the index table are star records sorted in increasing RA. Each star
 * record is 3 bytes. The bits are packed as follows (stored bigendian):
 *    0.. 5 rabits
 *    6..17 decbits
 *   18..22 magbits
 *   23     star
 * then
 *   ra = (rasum + rabits+.5)*RAPBIT/cos(decband)
 *   dec = (north ? decband + (decbits+.5) : decband - (decbits+.5))*DECPBIT
 *   mag = BRMAG + (magbits + .5)*MAGPBIT
 *   starlike if star else really a star
 * where decband is the integer from the ASCII header line, rasum is an unsigned
 * int to accumulate the rabits values (initialized at the start of a sweep
 * from the index table entry) and dec and ra are in degrees (so beware if
 * your cos() expects rads).
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "xephem.h"

#define	MAXGSC22FOV	degrad(5)	/* max FOV to fetch */

/* packing in each record */
#define DECBITS 12                      /* dec stored in 12 bits */
#define DECHI   1.0                     /* height of dec band, degrees */
#define DECPBIT (DECHI/(1<<DECBITS))    /* dec degs per bit */
#define DECMASK ((1<<DECBITS)-1)        /* dec mask */

#define RABITS  6                       /* delta ra stored in 6 bits */
#define MAXDRA  (49.0/3600.0)           /* max delta RA from one to next, degs*/
#define RAPBIT  (MAXDRA/(1<<RABITS))    /* ra delta degrees per bit */
#define RAMASK  ((1<<RABITS)-1)         /* ra mask */

#define MAGBITS 5                       /* mag stored in 5 bits */
#define BRMAG   10.0                    /* brightest mag we encode */
#define DIMMAG  22.0                    /* dimmest mag we encode */
#define DELMAG  (DIMMAG-BRMAG)          /* mag range */
#define MAGPBIT (DELMAG/(1<<MAGBITS))   /* mags per bit within range */
#define MAGMASK ((1<<MAGBITS)-1)        /* mag mask */

#define HDRLEN	8			/* bytes in ASCII header line */
#define BPIDX   8                       /* bytes per index entry */

/* room for one star packet */
#define XE3PKTSZ        3              /* bytes per packet - DO NOT CHANGE */
typedef unsigned char UC;
typedef unsigned long UL;
typedef UC PKT[XE3PKTSZ];

/* an array of ObjF which can be grown efficiently in multiples of NINC */
typedef struct {
    ObjF *mem;          /* malloced array */
    int max;            /* max number of cells in mem[] or -1 to just count */
    int used;           /* number actually in use */
} ObjFArray;

#define NINC    32      /* grow mem array this many at a time */

static FILE *xe3open (char *dir, int north, int db, char fn[], char msg[]);
static int addDec (ObjFArray *ofap, char *dir, double mag, int dec, double dmin,
    double dmax, double rmin, double rmax, char *msg);
static int addOneObjF (ObjFArray *ap, Obj *op);
static void unpack (unsigned *rasump, double cdb, int db, PKT pkt,
    Obj *op);
static void mkxe3name (char *dir, Obj *op);

/* return 0 if dir looks like it contains some xe3 files, else -1
 */
int
xe3chkdir (char *dir, char *msg)
{
	char fn[1024];
	FILE *fp;

	fp = xe3open (dir, 1, 0, fn, msg);
	if (!fp)
	    return (-1);
	fclose (fp);
	return (0);
}

/* add a collection of XE3 stars around the given location at *opp, which already contains
 * nop entries. if trouble fill msg[] and return -1, else return count.
 * N.B. *opp is only changed if we return > 0.
 */
int
xe3fetch (char *dir, double ra, double dec, double fov, double mag,
ObjF **opp, int nop, char *msg)
{
	int topd, botd, d;
	double dmin, dmax;
	double rov;
	ObjFArray ofa;
	int s = 0;

	/* bug if not given a place to save new stars */
	if (!opp) {
	    printf ("xe3fetch with opp == NULL\n");
	    abort();
	}

	/* enforce limit */
	if (fov > MAXGSC22FOV) {
	    static int fovwarned;
	    if (!fovwarned) {
		xe_msg (1, "All XE3 catalog requests will be limited to %.2f degree FOV", raddeg(MAXGSC22FOV));
		fovwarned = 1;
	    }

	    fov = MAXGSC22FOV;
	}

	/* switch everything to degrees */
	ra = raddeg(ra);
	dec = raddeg(dec);
	rov = raddeg(fov/2);

	/* find top and bottom dec limits, in degrees */
	dmin = dec - rov;
	dmax = dec + rov;
	botd = (int)floor(dmin);
	topd = (int)floor(dmax);

	/* init ofa */
	ofa.mem = *opp;
	ofa.max = nop;
	ofa.used = nop;

	/* read across ra range for each dec band, beware 360 wrap */
	for (d = botd; d <= topd; d++) {
	    double rovdec = d==90||d==-90 ? 180 : rov/cos(degrad(d));
	    double rmin = ra - rovdec, rmax = ra + rovdec;
	    int db = d;
	    if (d < -90) {
		db = -181-d;
		rmin = 0;
		rmax = 360;
	    } else if (d > 89) {
		db = 179-d;
		rmin = 0;
		rmax = 360;
	    } else if (rmin < 0) {
		s = addDec (&ofa, dir, mag, db, dmin, dmax, rmin+360, 360, msg);
		if (s < 0)
		    break;
		rmin = 0;
	    } else if (rmax > 360) {
		s = addDec (&ofa, dir, mag, db, dmin, dmax, 0.0, rmax-360, msg);
		if (s < 0)
		    break;
		rmax = 360;
	    }
	    s = addDec (&ofa, dir, mag, db, dmin, dmax, rmin, rmax, msg);
	    if (s < 0)
		break;
	}

	/* finished */
	if (s < 0) {
	    if (ofa.mem)
		free (ofa.mem);
	    return (-1);
	}
	if (ofa.mem)
	    *opp = ofa.mem;
	return (ofa.used);
}

/* add stars for dec band [-90..89] from dmin..dmax rmin..rmax to ofap.
 * all units in degrees.
 * if ok return 0 else -1 with reason in msg[].
 * dec is floor of band, ie, -90 .. 89.
 */
static int
addDec (ObjFArray *ofap, char *dir, double mag, int dec, double dmin,
    double dmax, double rmin, double rmax, char *msg)
{
	int north = dec >= 0;
	int db = north ? dec : dec+1;
	double cdb = cos(degrad(db));
	int rabase = (int)floor(rmin);
	char fn[1024];
	unsigned rasum, offset;
	UC idx[BPIDX];
	PKT pkt;
	FILE *fp;
	Obj o;

	/*
	fprintf (stderr, "addDec %g .. %g %g .. %g\n", rmin, rmax, dmin, dmax);
	*/

	/* open file */
	fp = xe3open (dir, north, db, fn, msg);
	if (!fp)
	    return (-1);

	/* read index */
	if (fseek (fp, HDRLEN + rabase*BPIDX, SEEK_SET) < 0) {
	    sprintf (msg, "%s:\nindex is short", fn);
	    fclose (fp);
	    return (-1);
	}
	if (fread (idx, BPIDX, 1, fp) != 1) {
	    sprintf (msg, "%s:\nindex entry is short", fn);
	    fclose (fp);
	    return (-1);
	}
	offset = (idx[0] << 24) | (idx[1] << 16) | (idx[2] << 8) | idx[3];
	rasum =  (idx[4] << 24) | (idx[5] << 16) | (idx[6] << 8) | idx[7];

	/* move to first ra packet */
	if (fseek (fp, offset, SEEK_SET) < 0) {
	    sprintf (msg, "%s:\npacket list is short", fn);
	    fclose (fp);
	    return (-1);
	}

	/* scan to rmax, work in rads */
	dmin = degrad(dmin);
	dmax = degrad(dmax);
	rmin = degrad(rmin);
	rmax = degrad(rmax);
	while (1) {
	    if (fread (pkt, sizeof(pkt), 1, fp) != 1) {
		if (feof(fp))
		    break;
		sprintf (msg, "%s:\n%s", fn, syserrstr());
		fclose (fp);
		return (-1);
	    }
	    unpack (&rasum, cdb, dec, pkt, &o);
	    if (o.f_RA > rmax)
		break;
	    if (get_fmag(&o) <= mag && o.f_RA >= rmin && dmin <= o.f_dec &&
							    o.f_dec <= dmax) {
		mkxe3name (dir, &o);
		if (addOneObjF (ofap, &o) < 0) {
		    sprintf (msg, "not enough memory after %d objects",
								ofap->used);
		    fclose (fp);
		    return (-1);
		}
	    } 
	}

	/* ok */
	fclose(fp);
	return (0);
}

/* unpack the raw PKT into Obj *op.
 * dec is -90..89
 */
static void
unpack (unsigned *rasump, double cdb, int dec, PKT pkt, Obj *op)
{
	unsigned v = (pkt[0] << 16) | (pkt[1] << 8) | pkt[2];
	unsigned radelta = (v&RAMASK);
	double decoffset = (((v>>RABITS)&DECMASK) + 0.5)*DECPBIT;
	double mag = (((v>>(RABITS+DECBITS))&MAGMASK) + 0.5)*MAGPBIT + BRMAG;
	int isother = (v >> (RABITS+DECBITS+MAGBITS));
	double ra = (*rasump + radelta + 0.5)*RAPBIT/cdb;
	double dc = dec<0 ? dec+1-decoffset : dec+decoffset;

	*rasump += radelta;

	zero_mem ((void *)op, sizeof(ObjF));

	op->o_type = FIXED;
	op->f_class = isother ? 'T' : 'S';
	op->f_RA = (float)degrad(ra);
	op->f_dec = (float)degrad(dc);
	op->f_epoch = (float)J2000;
	set_fmag (op, mag);
}

/* make up a name from the f_RA/dec fields of the given object */
static void
mkxe3name (char *dir, Obj *op)
{
	char rstr[32], dstr[32];
	char *cat;

	/* get RA and dec as fixed-length strings */
	fs_sexa (rstr, radhr(op->f_RA), 2, 36000);
	if (rstr[0] == ' ') rstr[0] = '0';
	rstr[2] = rstr[3];
	rstr[3] = rstr[4];
	rstr[4] = rstr[6];
	rstr[5] = rstr[7];
	rstr[6] = rstr[9];
	rstr[7] = '\0';

	fs_sexa (dstr, raddeg(op->f_dec), 3, 3600);
	if (dstr[1] == ' ') { dstr[0] = '+'; dstr[1] = '0'; }
	if (dstr[1] == '-') { dstr[0] = '-'; dstr[1] = '0'; }
	if (dstr[0] == ' ')   dstr[0] = '+';
	dstr[3] = dstr[4];
	dstr[4] = dstr[5];
	dstr[5] = dstr[7];
	dstr[6] = dstr[8];
	dstr[7] = '\0';

	/* use as much as possible of basename as catalog name */
	if ((cat = strrchr(dir,'/')) || (cat = strrchr(dir,'\\')))
	    cat++;		/* skip the / */
	else
	    cat = dir;
	sprintf (op->o_name, "%.*s %s%s", MAXNM-(7+7+2), cat, rstr, dstr);
}

/* add one ObjF entry to ap[], growing if necessary.
 * return 0 if ok else return -1
 */
static int
addOneObjF (ObjFArray *ap, Obj *op)
{
	ObjF *newf;

	if (ap->used >= ap->max) {
	    /* add room for NINC more */
	    char *newmem = ap->mem ? realloc ((void *)ap->mem,
						(ap->max+NINC)*sizeof(ObjF))
				   : malloc (NINC*sizeof(ObjF));
	    if (!newmem)
		return (-1);
	    ap->mem = (ObjF *)newmem;
	    ap->max += NINC;
	}

	newf = &ap->mem[ap->used++];
	(void) memcpy ((void *)newf, (void *)op, sizeof(ObjF));

	return (0);
}

/* open the given xe3 file.
 * if ok return name and FILE * else return NULL with reason in msg[]
 * north is 1 for dec floors 0..89, 0 for dec floors -90..-1.
 * db is dec band as used in xe3 file, ie, rounded towards 0, ie, -89..89.
 */
static FILE *
xe3open (char *dir, int north, int db, char fn[], char msg[])
{
	char nschar = north ? 'N' : 'S';
	char header[HDRLEN+1];
	FILE *fp;
	char myns;
	int mydb;

	/* want abs band in file name and header */
	db = abs(db);

	/* open and check header */
	sprintf (fn, "%s/%c%02d.xe3", dir, nschar, db);
	fp = fopenh (fn, "rb");
	if (!fp) {
	    (void) sprintf (msg, "%s:\n%s", fn, syserrstr());
	    return (NULL);
	}
	if (fread (header, HDRLEN, 1, fp) != 1) {
	    sprintf (msg, "%s:\nno header: %s", fn, syserrstr());
	    fclose (fp);
	    return(NULL);
	}
	header[HDRLEN-1] = '\0';
	if (sscanf (header, "xe3 %c%2d", &myns, &mydb) != 2
					|| myns != nschar || mydb != db) {
	    sprintf (msg, "%s:\nnot an XE3 file", fn);
	    fclose (fp);
	    return(NULL);
	}

	/* ok */
	return (fp);
}

#ifdef MAIN_TEST
/* stand alone dump a region of xe3 to stdout.
 * cc -o xe3 -DMAIN_TEST -Ilibastro -Ilibip -Llibastro xe3.c -lastro -lm
 */

#include <errno.h>

int
main (int ac, char *av[])
{
	ObjF *ofp = NULL;
	char msg[1024];
	double ra, dec, fov;
	char *dir;
	int n;

	if (ac != 5) {
	    fprintf (stderr, "usage: %s xe3dir ra dec fov (all degs)\n", av[0]);
	    return(1);
	}

	dir = av[1];
	ra  = degrad(atof(av[2]));
	dec = degrad(atof(av[3]));
	fov = degrad(atof(av[4]));

	n = xe3fetch (dir, ra, dec, fov, 100.0, &ofp, msg);
	if (n < 0) {
	    fprintf (stderr, "%s\n", msg);
	    exit(1);
	}
	while (n) {
	    db_write_line ((Obj *)&ofp[--n], msg);
	    printf ("%s\n", msg);
	}
	return (0);
}

char *
syserrstr ()
{
	return (strerror(errno));
}

FILE *
fopenh (char *name, char *how)
{
	return (fopen (name, how));
}

#endif

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: xe3.c,v $ $Date: 2009/01/05 21:44:18 $ $Revision: 1.5 $ $Name:  $"};

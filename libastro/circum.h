/* the Now and Obj typedefs.
 * also, a few miscellaneous constants and declarations.
 */
#ifndef _CIRCUM_H
#define _CIRCUM_H

#define	SPD	(24.0*3600.0)	/* seconds per day */
#define	MAU	(1.4959787e11)	/* m / au */
#define	LTAU	499.005		/* seconds light takes to travel 1 AU */
#define	ERAD	(6.37816e6)	/* earth equitorial radius, m */
#define	MRAD	(1.740e6)	/* moon equitorial radius, m */
#define	SRAD	(6.95e8)	/* sun equitorial radius, m */
#define	FTPM	3.28084		/* ft per m */
#define	ESAT_MAG	2	/* default satellite magnitude */
#define	FAST_SAT_RPD	0.25	/* max earth sat rev/day considered "fast" */

#define	EOD	(-9786)		/* special epoch flag: use epoch of date */

/* info about the local observing circumstances and misc preferences */
typedef struct {
	double n_mjd;	/* modified Julian date, ie, days since
			 * Jan 0.5 1900 (== 12 noon, Dec 30, 1899), utc.
			 * enough precision to get well better than 1 second.
			 * N.B. if not first member, must move NOMJD inits.
			 */
	double n_lat;	/* geographic (surface-normal) lat, >0 north, rads */
	double n_lng;	/* longitude, >0 east, rads */
	double n_tz;	/* time zone, hrs behind UTC */
	double n_temp;	/* atmospheric temp, degrees C */
	double n_pressure; /* atmospheric pressure, mBar */
	double n_elev;	/* elevation above sea level, earth radii */
	double n_dip;	/* dip of sun below hzn at twilight, >0 below, rads */
	double n_epoch;	/* desired precession display epoch as an mjd, or EOD */
	char n_tznm[8];	/* time zone name; 7 chars or less, always 0 at end */
} Now;

/* handy shorthands for fields in a Now pointer, np */
#define mjd	np->n_mjd
#define lat	np->n_lat
#define lng	np->n_lng
#define tz	np->n_tz
#define temp	np->n_temp
#define pressure np->n_pressure
#define elev	np->n_elev
#define	dip	np->n_dip
#define epoch	np->n_epoch
#define tznm	np->n_tznm
#define mjed	mm_mjed(np)

/* structures to describe objects of various types.
 */

/* magnitude values in two different systems */
typedef struct {
    float m1, m2;	/* either g/k or H/G, depending on... */
    int whichm;		/* one of MAG_gk or MAG_HG */
} Mag;

/* whichm */
#define MAG_HG          0       /* using 0 makes HG the initial default */
#define MAG_gk          1

/* we actually store magnitudes times this scale factor in a short int */
#define	MAGSCALE	100.0
#define	set_smag(op,m)	((op)->s_mag = (short)floor((m)*MAGSCALE + 0.5))
#define	set_fmag(op,m)	((op)->f_mag = (short)floor((m)*MAGSCALE + 0.5))
#define	get_mag(op)	((op)->s_mag / MAGSCALE)

/* longest object name, including trailing '\0' */
#define	MAXNM	48

/* Obj is a massive union.
 * many fields are in common so we use macros to make things a little easier.
 */

typedef unsigned char ObjType_t;
typedef unsigned char ObjAge_t;

/* fields common to *all* structs in the Obj union.
 */
#define	OBJ_COMMON_FLDS							\
    ObjType_t co_type;	/* current object type; see flags, below */	\
    unsigned char co_flags;/* FUSER*... used by others */		\
    char co_name[MAXNM];/* name, including \0 */			\
    float co_ra;	/* geo/topo app/mean ra, rads */		\
    float co_dec;	/* geo/topo app/mean dec, rads */		\
    float co_gaera;	/* geo apparent ra, rads */			\
    float co_gaedec;	/* geo apparent dec, rads */			\
    double co_az;	/* azimuth, >0 e of n, rads */			\
    double co_alt;	/* altitude above topocentric horizon, rads */	\
    float co_elong;	/* angular sep btwen obj and sun, >0 E, degs */	\
    float co_size;	/* angular size, arc secs */			\
    short co_mag;	/* visual magnitude * MAGSCALE */		\
    ObjAge_t co_age	/* update aging code; see db.c */

/* fields common to all solar system objects in the Obj union */
#define	OBJ_SOLSYS_FLDS							\
    OBJ_COMMON_FLDS;	/* all the fixed ones plus ... */		\
    float so_sdist;	/* dist from object to sun, au */		\
    float so_edist;	/* dist from object to earth, au */		\
    float so_hlong;	/* heliocentric longitude, rads */		\
    float so_hlat;	/* heliocentric latitude, rads */		\
    float so_phase	/* phase, % */


/* a generic object */
typedef struct {
    OBJ_COMMON_FLDS;
} ObjAny;

/* a generic sol system object */
typedef struct {
    OBJ_SOLSYS_FLDS;
} ObjSS;

/* basic Fixed object info.
 */
typedef unsigned char byte;
typedef struct {
    OBJ_COMMON_FLDS;
    char  fo_class;	/* object class --  see db.c:db_set_field() et al */
    char  fo_spect[2];	/* spectral codes, if appropriate */
    byte  fo_ratio;	/* minor/major diameter ratio. use s/get_ratio() */
    byte  fo_pa;	/* position angle, E of N, rads. use s/get_pa() */
    float fo_epoch;	/* epoch of f_RA/dec */
    float fo_ra;	/* ra, rads, at given epoch */
    float fo_dec;	/* dec, rads, at given epoch */
} ObjF;

#define	fo_mag	co_mag	/* pseudonym for so_mag since it is not computed */
#define	fo_size	co_size	/* pseudonym for so_size since it is not computed */

/* macros to pack/unpack some fields */
#define	SRSCALE		255.0		/* galaxy size ratio scale */
#define	PASCALE		(255.0/(2*PI))	/* pos angle scale factor */
#define	get_ratio(op)	((int)(op)->f_ratio/SRSCALE)
#define	set_ratio(op,maj,min) ((op)->f_ratio = (byte)(((maj) > 0)	    \
					? ((min)*SRSCALE/(double)(maj)+0.5) \
					: 0))
#define	get_pa(op)	((double)(op)->f_pa/PASCALE)
#define	set_pa(op,s)	((op)->f_pa = (byte)((s)*PASCALE + 0.5))

#define	NCLASSES	128 /* n potential fo_classes -- allow for all ASCII */

/* basic planet object info */
typedef struct {
    OBJ_SOLSYS_FLDS;
    int pl_code;	/* one of the codes in astro.h */
} ObjPl;

/* basic info about an object in elliptical heliocentric orbit */
typedef struct {
    OBJ_SOLSYS_FLDS;
    float  eo_inc;	/* inclination, degrees */
    float  eo_Om;	/* longitude of ascending node, degrees */
    float  eo_om;	/* argument of perihelion, degress */
    float  eo_a;	/* mean distance, aka,semi-maj axis,AU */
    float  eo_M;	/* mean anomaly, ie, degrees from perihelion at cepoch*/
    float  eo_size;	/* angular size, in arc seconds at 1 AU */
    double eo_e;	/* eccentricity (double for when near 1 computing q) */
    double eo_cepoch;	/* epoch date (M reference), as an mjd */
    double eo_epoch;	/* equinox year (inc/Om/om reference), as an mjd. */
    Mag    eo_mag;	/* magnitude */
} ObjE;

/* basic info about an object in hyperbolic heliocentric orbit */
typedef struct {
    OBJ_SOLSYS_FLDS;
    double ho_epoch;	/* equinox year (inc/Om/om reference), as an mjd */
    double ho_ep;	/* epoch of perihelion, as an mjd */
    float  ho_inc;	/* inclination, degs */
    float  ho_Om;	/* longitude of ascending node, degs */
    float  ho_om;	/* argument of perihelion, degs. */
    float  ho_e;	/* eccentricity */
    float  ho_qp;	/* perihelion distance, AU */
    float  ho_g, ho_k;	/* magnitude model coefficients */
    float  ho_size;	/* angular size, in arc seconds at 1 AU */
} ObjH;

/* basic info about an object in parabolic heliocentric orbit */
typedef struct {
    OBJ_SOLSYS_FLDS;
    double po_epoch;	/* reference epoch, as an mjd */
    double po_ep;	/* epoch of perihelion, as an mjd */
    float  po_inc;	/* inclination, degs */
    float  po_qp;	/* perihelion distance, AU */
    float  po_om;	/* argument of perihelion, degs. */
    float  po_Om;	/* longitude of ascending node, degs */
    float  po_g, po_k;	/* magnitude model coefficients */
    float  po_size;	/* angular size, in arc seconds at 1 AU */
} ObjP;

/* basic earth satellite object info */
typedef struct {
    OBJ_COMMON_FLDS;
    double eso_epoch;	/* reference epoch, as an mjd */
    double eso_n;	/* mean motion, rev/day
			 * N.B. we need double due to a sensitive differencing
			 * operation used to compute MeanAnomaly in
			 * esat_main()/satellite.c.
			 */
    float  eso_inc;	/* inclination, degs */
    float  eso_raan;	/* RA of ascending node, degs */
    float  eso_e;	/* eccentricity */
    float  eso_ap;	/* argument of perigee at epoch, degs */
    float  eso_M;	/* mean anomaly, ie, degrees from perigee at epoch */
    float  eso_decay;	/* orbit decay rate, rev/day^2 */
    float  eso_drag;	/* object drag coefficient, (earth radii)^-1 */
    int    eso_orbit;	/* integer orbit number of epoch */

    /* computed "sky" results unique to earth satellites */
    float  ess_elev;	/* height of satellite above sea level, m */
    float  ess_range;	/* line-of-site distance from observer to satellite, m*/
    float  ess_rangev;	/* rate-of-change of range, m/s */
    float  ess_sublat;	/* latitude below satellite, >0 north, rads */
    float  ess_sublng;	/* longitude below satellite, >0 east, rads */
    int    ess_eclipsed;/* 1 if satellite is in earth's shadow, else 0 */
} ObjES;

typedef union {
    ObjAny  any;	/* these fields valid for all types */
    ObjSS   anyss;	/* these fields valid for all solar system types */
    ObjPl   pl;		/* planet */
    ObjF    f;		/* fixed object */
    ObjE    e;		/* object in heliocentric elliptical orbit */
    ObjH    h;		/* object in heliocentric hyperbolic trajectory */
    ObjP    p;		/* object in heliocentric parabolic trajectory */
    ObjES   es;		/* earth satellite */
} Obj;


/* for o_flags -- everybody must agree */
#define	FUSER0		0x01
#define	FUSER1		0x02
#define	FUSER2		0x04
#define	FUSER3		0x08
#define	FUSER4		0x10
#define	FUSER5		0x20
#define	FUSER6		0x40
#define	FUSER7		0x80

/* mark an object as being a "field star" */
#define	FLDSTAR		FUSER3

/* Obj shorthands: */
#define	o_type	any.co_type
#define	o_name	any.co_name
#define	o_flags	any.co_flags
#define	o_age	any.co_age
#define	s_ra	any.co_ra
#define	s_dec	any.co_dec
#define	s_gaera	any.co_gaera
#define	s_gaedec any.co_gaedec
#define	s_az	any.co_az
#define	s_alt	any.co_alt
#define	s_elong	any.co_elong
#define	s_size	any.co_size
#define	s_mag	any.co_mag

#define	s_sdist	anyss.so_sdist
#define	s_edist	anyss.so_edist
#define	s_hlong	anyss.so_hlong
#define	s_hlat	anyss.so_hlat
#define	s_phase anyss.so_phase

#define	s_elev		es.ess_elev
#define	s_range		es.ess_range
#define	s_rangev	es.ess_rangev
#define	s_sublat	es.ess_sublat
#define	s_sublng	es.ess_sublng
#define	s_eclipsed	es.ess_eclipsed

#define	f_class	f.fo_class
#define	f_spect	f.fo_spect
#define	f_ratio	f.fo_ratio
#define	f_pa	f.fo_pa
#define	f_epoch	f.fo_epoch
#define	f_RA	f.fo_ra
#define	f_dec	f.fo_dec
#define	f_mag	f.fo_mag
#define	f_size	f.fo_size

#define	e_cepoch e.eo_cepoch
#define	e_epoch	e.eo_epoch
#define	e_inc	e.eo_inc
#define	e_Om	e.eo_Om
#define	e_om	e.eo_om
#define	e_a	e.eo_a
#define	e_e	e.eo_e
#define	e_M	e.eo_M
#define	e_size	e.eo_size
#define	e_mag	e.eo_mag

#define	h_epoch	h.ho_epoch
#define	h_ep	h.ho_ep
#define	h_inc	h.ho_inc
#define	h_Om	h.ho_Om
#define	h_om	h.ho_om
#define	h_e	h.ho_e
#define	h_qp	h.ho_qp
#define	h_g	h.ho_g
#define	h_k	h.ho_k
#define	h_size	h.ho_size

#define	p_epoch	p.po_epoch
#define	p_ep	p.po_ep
#define	p_inc	p.po_inc
#define	p_qp	p.po_qp
#define	p_om	p.po_om
#define	p_Om	p.po_Om
#define	p_g	p.po_g
#define	p_k	p.po_k
#define	p_size	p.po_size

#define	es_epoch	es.eso_epoch
#define	es_inc		es.eso_inc
#define	es_raan		es.eso_raan
#define	es_e		es.eso_e
#define	es_ap		es.eso_ap
#define	es_M		es.eso_M
#define	es_n		es.eso_n
#define	es_decay	es.eso_decay
#define	es_drag		es.eso_drag
#define	es_orbit	es.eso_orbit

/* insure we always refer to the fields and no monkey business */
#undef OBJ_COMMON_FLDS
#undef OBJ_SOLSYS_FLDS

/* o_type code.
 * N.B. names are assigned in order in objmenu.c
 * N.B. if add one: update the size init in db_init(); switch in obj_cir().
 * N.B. UNDEFOBJ must be zero so new objects are undefinied by being zeroed.
 * N.B. maintain the bitmasks too.
 * N.B. PLANET must be last -- see objmenu.c
 */
enum ObjType {
    UNDEFOBJ=0,
    FIXED, ELLIPTICAL, HYPERBOLIC, PARABOLIC, EARTHSAT, PLANET,
    NOBJTYPES
};

/* types as handy bitmasks too */
#define	FIXEDM		(1<<FIXED)
#define	ELLIPTICALM	(1<<ELLIPTICAL)
#define	HYPERBOLICM	(1<<HYPERBOLIC)
#define	PARABOLICM	(1<<PARABOLIC)
#define	EARTHSATM	(1<<EARTHSAT)
#define	PLANETM		(1<<PLANET)
#define	ALLM		(FIXEDM | HYPERBOLICM | PARABOLICM | EARTHSATM \
						      | ELLIPTICALM | PLANETM)

/* define a code for each member in each object type struct.
 * making them globally unique avoids a nested switch in db_set_field() and
 * helps with dynamic prompting based on preferences.
 */
enum {
    O_TYPE, O_NAME,
    F_RA, F_DEC, F_EPOCH, F_MAG, F_SIZE, F_CLASS, F_SPECT,
    E_INC, E_LAN, E_AOP, E_A, E_N, E_E, E_M, E_CEPOCH, E_EPOCH,E_M1,E_M2,E_SIZE,
    H_EP, H_INC, H_LAN, H_AOP, H_E, H_QP, H_EPOCH, H_G, H_K, H_SIZE,
    P_EP, P_INC, P_AOP, P_QP, P_LAN, P_EPOCH, P_G, P_K, P_SIZE,
    ES_EPOCH,ES_INC,ES_RAAN,ES_E,ES_AP,ES_M,ES_N,ES_DECAY,ES_ORBIT,ES_DRAG
};

/* rise, set and transit information.
 */
typedef struct {
    int rs_flags;	/* info about what has been computed and any
			 * special conditions; see flags, below.
			 */
    double rs_risetm;	/* mjd time of rise today */
    double rs_riseaz;	/* azimuth of rise, rads E of N */
    double rs_trantm;	/* mjd time of transit today */
    double rs_tranalt;	/* altitude of transit, rads up from horizon */
    double rs_settm;	/* mjd time of set today */
    double rs_setaz;	/* azimuth of set, rads E of N */
} RiseSet;

/* RiseSet flags */
#define	RS_NORISE	0x0001	/* object does not rise as such today */
#define	RS_NOSET	0x0002	/* object does not set as such today */
#define	RS_NOTRANS	0x0004	/* object does not transit as such today */
#define	RS_CIRCUMPOLAR	0x0010	/* object stays up all day today */
#define	RS_NEVERUP	0x0020	/* object never up at all today */
#define	RS_ERROR	0x1000	/* can't figure out anything! */
#define	RS_RISERR	(0x0100|RS_ERROR) /* error computing rise */
#define	RS_SETERR	(0x0200|RS_ERROR) /* error computing set */
#define	RS_TRANSERR	(0x0400|RS_ERROR) /* error computing transit */

#define	is_type(op,m)	((1<<(op)->o_type) & (m))
#define	is_planet(op,p)	(is_type(op,PLANETM) && ((ObjPl *)op)->pl_code==(p))
#define	is_ssobj(op)	is_type(op,PLANETM|HYPERBOLICM|PARABOLICM|ELLIPTICALM)

/* used to maintain progress state with db_scanint() and db_scan */
typedef struct {
    int m;	/* mask of *N types desired */
    int t;	/* current Object type, as per ObjType */
    int n;	/* number of objects "scanned" so far */
    ObjF *op;	/* local list to also scan */
    int nop;	/* number in op[] */
} DBScan;

#endif /* _CIRCUM_H */


/* Some handy declarations */

/* ap_as.c */
extern void ap_as P_(( Now *np, double Mjd, double *rap, double *decp));
extern void as_ap P_(( Now *np, double Mjd, double *rap, double *decp));

/* aux.c */
extern double mm_mjed P_((Now *np));

/* circum.c */
extern int obj_cir P_((Now *np, Obj *op));

/* earthsat.c */
extern int obj_earthsat P_((Now *np, Obj *op));

/* dbfmt.c */
extern int db_crack_line P_((char s[], Obj *op, char whynot[]));
extern void db_write_line P_((Obj *op, char *lp));
extern int get_fields P_((char *s, int delim, char *fields[]));
extern int db_chk_planet P_((char name[], Obj *op));
extern int db_tle P_((char *name, char *l1, char *l2, Obj *op));

/* misc.c */
extern void now_lst P_((Now *np, double *lstp));
extern void radec2ha P_((Now *np, double ra, double dec, double *hap));
extern char *obj_description P_((Obj *op));
extern int is_deepsky P_((Obj *op));

/* riset_cir.c */
extern void riset_cir P_((Now *np, Obj *op, double dis, RiseSet *rp));
extern void twilight_cir P_((Now *np, double dis, double *dawn, double *dusk,
    int *status));

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: circum.h,v $ $Date: 2003/04/19 19:17:16 $ $Revision: 1.3 $ $Name:  $
 */

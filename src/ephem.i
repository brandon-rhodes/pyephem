%module ephem
%include typemaps.i

/* PyEphem version 0.9
 * 25 July 1998
 */

/* Copyright 1998 Brandon Craig Rhodes
 * Licensed under the terms of the GNU General Public License
 */

%{
/* Include the raw header files from the library. */
     
#include "libastro/P_.h"
#include "libastro/astro.h"
#include "libastro/circum.h"
#include <math.h>
     
/* Undefine the troublesome field abbreviations defined in circum.h. */

#undef mjd
#undef lat
#undef lng
#undef tz
#undef temp
#undef pressure
#undef elev
#undef dip
#undef epoch
#undef tznm
#undef mjed

#undef o_type
#undef o_name
#undef o_flags
#undef o_age
#undef s_ra
#undef s_dec
#undef s_gaera
#undef s_gaedec
#undef s_az
#undef s_alt
#undef s_elong
#undef s_size
#undef s_mag

#undef s_sdist
#undef s_edist
#undef s_hlong
#undef s_hlat
#undef s_phase

#undef s_elev
#undef s_range
#undef s_rangev
#undef s_sublat
#undef s_sublng
#undef s_eclipsed

#undef f_class
#undef f_spect
#undef f_ratio
#undef f_pa
#undef f_epoch
#undef f_RA
#undef f_dec
#undef f_mag
#undef f_size

#undef e_cepoch
#undef e_epoch
#undef e_inc
#undef e_Om
#undef e_om
#undef e_a
#undef e_e
#undef e_M
#undef e_size
#undef e_mag

#undef h_epoch
#undef h_ep
#undef h_inc
#undef h_Om
#undef h_om
#undef h_e
#undef h_qp
#undef h_g
#undef h_k
#undef h_size

#undef p_epoch
#undef p_ep
#undef p_inc
#undef p_qp
#undef p_om
#undef p_Om
#undef p_g
#undef p_k
#undef p_size

#undef es_epoch
#undef es_inc
#undef es_raan
#undef es_e
#undef es_ap
#undef es_M
#undef es_n
#undef es_decay
#undef es_orbit

#include "libastro/preferences.h"
#include "libastro/vsop87.h"
%}

/* This defines a character buffer for functions that need to be provided
   with one, and uses the error-integer. */

%{
     char format_buffer[256];
     typedef int ERRINT;
%}

/* Boolean error codes returned by some functions (technique described by
   Mr Beasley on the SWIG mailing list in early May 1998) are made to
   raise a Python exception. */

typedef void ERRINT;
%typemap(python,except) ERRINT {
     int e = $function;
     if (e == -1) {
	  $cleanup;
	  PyErr_SetString(PyExc_ValueError, "Ephem function error");
	  return NULL;
     }
}

/* Some functions return strings by writing them into a caller-allocated
   buffer. */

%typemap(python,ignore) char *OUT {
     $target = format_buffer;
}
%typemap(python,argout) char *OUT {
     PyObject *o;
     o = PyString_FromString(format_buffer);
     $target = o;
}








#define MetersPerAU 1.495979e11
#define LightTimeAU 499.005
#define EarthRadius 6.37816e6
#define MoonRadius 1.740e6
#define SunRadius 6.95e8
#define FeetPerMeter 3.28084









#define MJD0 2415020.0

#define J2000 (2451545.0 - MJD0)

#define SecondsPerDay (24.0*3600.0)





%name (formatDate) void fs_date(char *OUT, double mjd);

%name (scanDate) void f_sscandate(char *string, int format = PREF_MDY,
	int *OUTPUT, double *OUTPUT, int *OUTPUT);

%name (rescanDate) void f_sscandate
	(char *string, int format, int *BOTH, double *BOTH, int *BOTH);







%name (fromGregorian) void cal_mjd
	(int month, double day, int year, double *OUTPUT);

%name (toGregorian) void mjd_cal
	(double mjd, int *OUTPUT, double *OUTPUT, int *OUTPUT);

%name (toDayOfWeek) int mjd_dow (double mjd, int *OUTPUT);

%name (toDaysInMonth) void mjd_dpm (double mjd, int *OUTPUT);

%name (toYear) void mjd_year (double mjd, double *OUTPUT);

%name (fromYear) void year_mjd (double year, double *OUTPUT);

%name (toNearestSecond) void rnd_second (double *BOTH);

%name (toDay) double mjd_day (double mjd);

%name (toHourOfDay) double mjd_hr(double mjd);








%name (formatSexagesimal) void fs_sexa (char *OUT, double angle, int w, int fracbase);

%name (scanSexagesimal) ERRINT scansex (char *string, double *OUTPUT);

%name (rescanSexagesimal) void f_scansex(float original, char string[], double *OUTPUT);





%name (deltaRA) double delra (double difference);





%name (degrad) double degrad(double d) {return d*PI/180.;};

%name (raddeg) double raddeg(double r) {return r*180./PI;};

%name (hrdeg) double hrdeg(double h) {return h*15.;};

%name (deghr) double deghr(double d) {return d/15.;};

%name (hrrad) double hrrad(double h) {return degrad(hrdeg(h));};

%name (radhr) double radhr(double r) {return deghr(raddeg(r));};






%name (formatHours) void formatHours(double angle, int fracbase, char *OUT);
%name (formatDegrees) void formatDegrees(double angle, int fracbase, char *OUT);
%name (formatDay) void formatDay(double mjd, char *OUT);
%name (formatTime) void formatTime(double mjd, int fracbase, char *OUT);


%{
void formatHours(double angle, int fracbase, char *OUT) {
	fs_sexa(OUT, radhr(angle), 2, fracbase);
}
void formatDegrees(double angle, int fracbase, char *OUT) {
	fs_sexa(OUT, raddeg(angle), 3, fracbase);
}
void formatDay(double mjd, char *OUT) {
	fs_date(OUT, mjd_day(mjd));
}
void formatTime(double mjd, int fracbase, char *OUT) {
	fs_sexa(OUT, mjd_hr(mjd), 2, fracbase);
}
%}







#define MaxName 14





enum ObjType {
    UNDEFOBJ=0,
    FIXED, ELLIPTICAL, HYPERBOLIC, PARABOLIC, EARTHSAT, PLANET, NOBJTYPES
};




typedef union {
	Obj();
	~Obj();


%name (any) ObjAny any;
%name (anyss) ObjSS anyss;
%name (pl) ObjPl pl;
%name (f) ObjF f;
%name (e) ObjE e;
%name (h) ObjH h;
%name (p) ObjP p;
%name (es) ObjES es;


} Obj;

typedef unsigned char ObjType_t;
typedef unsigned char ObjAge_t;







%inline %{
typedef double magnitude_t;
%}

%typemap(python,memberin) magnitude_t {
	$target = (short) floor (($source)*MAGSCALE + 0.5);
}
%typemap(python,memberout) magnitude_t {
	$target = $source / MAGSCALE;
}

%name (any) typedef struct {

%name (type) ObjType_t co_type;
%name (flags) unsigned char co_flags;
%name (name) char co_name[MAXNM];
%name (ra) float co_ra;
%name (dec) float co_dec;
%name (gaera) float co_gaera;
%name (gaedec) float co_gaedec;
%name (azimuth) float co_az;
%name (altitude) float co_alt;
%name (elongation) float co_elong;
%name (size) unsigned short co_size;
%name (magnitude) magnitude_t co_mag;
%name (age) ObjAge_t co_age;

} ObjAny;





%name (anyss) typedef struct {

%name (sunDistance) float so_sdist;
%name (earthDistance) float so_edist;
%name (helioLongitude) float so_hlong;
%name (helioLatitude) float so_hlat;
%name (phase) float so_phase;

} ObjSS;

typedef unsigned char byte;



%name (pl) typedef struct {

%name (code) int pl_code;

} ObjPl;


#define MERCURY 0
#define VENUS 1
#define MARS 2
#define JUPITER 3
#define SATURN 4
#define URANUS 5
#define NEPTUNE 6
#define PLUTO 7
#define SUN 8
#define MOON 9




%name (f) typedef struct {

%name (class) char fo_class;
%name (spectral) char fo_spect[2];
%name (ratio) byte fo_ratio;
%name (positionAngle) byte fo_pa;
%name (epoch) float fo_epoch;
%name (ra) float fo_ra;
%name (dec) float fo_dec;

} ObjF;


#define SRSCALE 255.0
#define PASCALE (255.0/360.0)






#define NCLASSES 128





%name (e) typedef struct {

%name (inc) float eo_inc;
%name (Omega) float eo_Om;
%name (omega) float eo_om;
%name (a) float eo_a;
%name (e) float eo_e;
%name (M) float eo_M;
%name (size) float eo_size;
%name (cepoch) double eo_cepoch;
%name (epoch) double eo_epoch;
%name (magnitude) Mag eo_mag;

} ObjE;





%name (h) typedef struct {

%name (epoch) double ho_epoch;
%name (ep) double ho_ep;
%name (inc) float ho_inc;
%name (Omega) float ho_Om;
%name (omega) float ho_om;
%name (e) float ho_e;
%name (qp) float ho_qp;
%name (g) float ho_g;
%name (k) float ho_k;
%name (size) float ho_size;

} ObjH;





%name (p) typedef struct {

%name (epoch) double po_epoch;
%name (ep) double po_ep;
%name (inc) float po_inc;
%name (qp) float po_qp;
%name (omega) float po_om;
%name (Omega) float po_Om;
%name (g) float po_g;
%name (k) float po_k;
%name (size) float po_size;

} ObjP;





%name (es) typedef struct {



%name (epoch) double eso_epoch;
%name (n) double eso_n;
%name (inc) float eso_inc;
%name (raan) float eso_raan;
%name (e) float eso_e;
%name (ap) float eso_ap;
%name (M) float eso_M;
%name (decay) float eso_decay;
%name (orbitNumber) int eso_orbit;




%name (elevation) float ess_elev;
%name (range) float ess_range;
%name (rangev) float ess_rangev;
%name (sublatitude) float ess_sublat;
%name (sublongitude) float ess_sublng;
%name (isEclipsed) int ess_eclipsed;

} ObjES;





#define MAG_HG 0
#define MAG_gk 1


typedef struct {

%name (m1) float m1;
%name (m2) float m2;
%name (whichm) int whichm;

} Mag;




%name (describe) char *obj_description (Obj *object);

%name (isDeepsky) int is_deepsky (Obj *op);






%name (Circumstance) typedef struct {
	Now();
	~Now();

%name (mjd) double n_mjd;
%name (latitude) double n_lat;
%name (longitude) double n_lng;
%name (timezone) double n_tz;
%name (temperature) double n_temp;
%name (pressure) double n_pressure;
%name (elevation) double n_elev;
%name (sunDip) double n_dip;
%name (epoch) double n_epoch;
%name (tzname) char n_tznm[8];

} Now;




#define EOD (-9786)





%name (computeLocation) int obj_cir (Now *circumstance, Obj *object);

%name (computeSeparation) double dm_separation (Obj *p, Obj *q);

%name (computeSiderealTime) void now_lst (Now *circumstance, double *OUTPUT);


%{
/* This function is Copyright 1998 by Elwood Charles Downey, and is
   from the GUI/xephem/datamenu.c file of his xephem-3.1 distribution. */
/* compute and display the separation between the two sky locations */
double dm_separation (Obj *p, Obj *q)
{
	double spy, cpy, px, qx, sqy, cqy;
	double sep;
	spy = sin (p->any.co_dec);
	cpy = cos (p->any.co_dec);
	px = p->any.co_ra;
	qx = q->any.co_ra;
	sqy = sin (q->any.co_dec);
	cqy = cos (q->any.co_dec);
	sep = acos(spy*sqy + cpy*cqy*cos(px-qx));
	return sep;
}
%}




typedef struct {

	RiseSet();
	~RiseSet();
%name (flags) int rs_flags;
%name (riseTime) double rs_risetm;
%name (riseAzimuth) double rs_riseaz;
%name (transitTime) double rs_trantm;
%name (transitAltitude) double rs_tranalt;
%name (setTime) double rs_settm;
%name (setAzimuth) double rs_setaz;

} RiseSet;




#define RS_NORISE 0x0001
#define RS_NOSET 0x0002
#define RS_NOTRANS 0x0004
#define RS_CIRCUMPOLAR 0x0010
#define RS_NEVERUP 0x0020
#define RS_ERROR 0x1000
#define RS_RISERR (0x0100|RS_ERROR)
#define RS_SETERR (0x0200|RS_ERROR)
#define RS_TRANSERR (0x0400|RS_ERROR)





%name (computeRiseSet) void riset_cir (Now *circumstance, Obj *object,
		double distance, RiseSet *riseSet);

%name (computeTwilight) void twilight_cir (Now *circumstance, double distance,
		double *OUTPUT, double *OUTPUT, int *OUTPUT);




%{
char *cns_name(int id);
int cns_pick(double r, double d, double e);

void constellation (double ra, double dec, double epoch, char *out)
{
	char *answer = (char *) cns_name(cns_pick(ra,dec,epoch));
	strncpy(out, answer, 3);
	out[3] = '\0';
}

void constellationName (double ra, double dec, double epoch, char *out)
{
	char *answer = (char *) cns_name(cns_pick(ra,dec,epoch));
	strcpy(out, answer+5);
}
%}


%name (constellation) void constellation(double ra, double dec, double epoch, char *OUT);

%name (constellationName) void constellationName(double ra, double dec, double epoch,
		char *OUT);




%{
int twoarg_db_crack_line (char s[], Obj *op)
{
	return db_crack_line(s, op, 0);
}
%}



%name (scanDB) ERRINT twoarg_db_crack_line (char string[], Obj *op);

%name (formatDB) void db_write_line (Obj *op, char *OUT);





%name (ap_as) void ap_as (Now *circumstance, double mjd,
		double *BOTH, double *BOTH);

%name (as_ap) void as_ap (Now *circumstance, double mjd,
		double *BOTH, double *BOTH);

%name (deltat) extern double deltat (double mjd);

%name (heliocorr) extern void heliocorr (double jd, double ra, double dec,
		double *OUTPUT);

%name (mm_mjed) extern double mm_mjed (Now *np);





typedef int Preferences;


%name (pref_get) extern int pref_get(Preferences p);
%name (pref_set) extern int pref_set(Preferences p, int value);


/* NOTE: these constants must remain in agreement with those in the
   libastro preferences.h file. */

#define PREF_EQUATORIAL PREF_EQUATORIAL






#define PREF_TOPO PREF_TOPO
#define PREF_GEO PREF_GEO


#define PREF_DATE_FORMAT PREF_DATE_FORMAT






#define PREF_MDY PREF_MDY
#define PREF_YMD PREF_YMD
#define PREF_DMY PREF_DMY


%init %{
	pref_set(PREF_EQUATORIAL, PREF_TOPO);
	pref_set(PREF_DATE_FORMAT, PREF_MDY);
%}

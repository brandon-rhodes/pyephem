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

TITLE_(PyEphem 1.0)

INTRODUCTION_(`Wrapped by
<A HREF="http://www.rhodesmill.org/brandon/">Brandon Craig Rhodes</A>.
<P>
PyEphem is a library of astronomical computation functions
accessible from the Python language.
The library functions themselves are the hard work of Elwood Downey
and his contributors,
and are taken with his permission from the internals of his excellent
<A HREF="http://iraf.noao.edu/~ecdowney/xephem.html">XEphem</A> application.
These functions have been incorporated into a Python module through
David Beazley''`s <A HREF="http://www.swig.org/">SWIG</A> utility
and a set of M4 scripts that allow the production
of both documentation and SWIG source from the same file.
<P>
Note that in wrapping the XEphem library,
I have not retained
the (rather cryptic) variable and function names of the C library.
Instead I have sought names that, while brief, are expressive enough
that I would not mind reading a program that uses this library.
If you need briefer names for common functions,
you can just use Python''`s equality operator to bind
functions and variables to more convenient names.
<P>
Please mail comments, corrections, or additional ideas
to <A HREF="brandon@rhodesmill.org">brandon@rhodesmill.org</A>.
The PyEphem web page is located at
<A HREF="http://www.rhodesmill.org/pyephem">
http://www.rhodesmill.org/pyephem</A>.
<HR>
Wrapping code and documentation is copyright 1998 by Brandon Craig Rhodes,
and is licensed for use under the terms of the GNU General Public License.
The internal astronomical computation library
is copyright 1998 by Elwood Charles Downey,
and is licensed for noncommercial use and redistribution
(but not modification).
The full text of these licenses is available
in the PyEphem source code package.
')

SECTION_(Constants)

CONSTANT_BEGIN
CONSTANT_(MetersPerAU, 1.495979e11,	Meters per AU.)
CONSTANT_(LightTimeAU, 499.005,		Seconds light takes to travel one AU.)
CONSTANT_(EarthRadius, 6.37816e6,	Earth equatorial radius in meters.)
CONSTANT_(MoonRadius, 1.740e6,		Moon equatorial radius in meters.)
CONSTANT_(SunRadius, 6.95e8,		Sun equatorial radius in meters.)
CONSTANT_(FeetPerMeter, 3.28084,	Feet per meter.)
CONSTANT_END

SECTION_(Measurement)

SUBSECTION_(Time)

TEXT_(
`The library represents a time as a double precision floating point number
indicating the number of days that have passed
since noon universal time on 1 January 1900.
This measure of time is called a "modified Julian day."
The Julian Day itself is a more traditional measure of time,
and indicates the number of days that have passed
since 12h UTC 1 Jan 4714 BC.'
)

CONSTANT_BEGIN
CONSTANT_(MJD0, 2415020.0, `The Julian day of 12h UTC 31 December 1889,
and therefore the difference between
a time expressed in modified Julian days
and the same time expressed in traditional Julian days.')

CONSTANT_(J2000, (2451545.0 - MJD0),
`The modified Julian day of 12h UTC 1 January 2000, which is
very commonly used as the epoch for modern astronomical
computations.')

CONSTANT_(SecondsPerDay, (24.0*3600.0), Number of seconds per day.)
CONSTANT_END

SUBSUBSECTION_(Input and Output)

FUNCTION_BEGIN
FUNCTION_(formatDate, void fs_date(char *OUT, double mjd),
	(float mjd),
	string date,
`Formats the date corresponding to the given modified Julian day
using the format currently selected through the preferences;
see the Preferences section below.')

FUNCTION_(scanDate, void f_sscandate(char *string, int format = PREF_MDY,
	int *OUTPUT, double *OUTPUT, int *OUTPUT),
	(string date, integer format),
	(integer month, float day, integer year),
`Split a date string of the form X/Y/Z (determined by the `format'
preference specifier - see the Preferences section below) into its
components.  It allows the day to be a floating point number.  A
lone component is always a year if it contains a decimal point or
if `format' is MDY or DMY and it is not a reasonable month or day
value, respectively.  The separators may be anything but digits or
a decimal point.')

FUNCTION_(rescanDate, void f_sscandate
	(char *string, int format, int *BOTH, double *BOTH, int *BOTH),
	(string date, format integer,
	 integer month, float day, integer year),
	(integer month, float day, integer year),
`Split a date string just like the function scanDate does, except
that you provide default values for each field and any unspecified
component of the date will be left unchanged.')
FUNCTION_END

SUBSUBSECTION_(Conversions)

TEXT_(`
Note that these routines follow the standard astronomical convention
of having negative years preceding the year <NOBR>AD 1</NOBR>.
So the year <NOBR>1 BC</NOBR>,
which was the year before <NOBR>AD 1</NOBR>,
is called <NOBR>year 0</NOBR>;
<NOBR>2 BC</NOBR> is called the <NOBR>year -1</NOBR>;
and in general year <NOBR><I>n</I> BC</NOBR>
is numbered as the -(<I>n</I>-1)th year.
')

FUNCTION_BEGIN
FUNCTION_(fromGregorian, void cal_mjd
	(int month, double day, int year, double *OUTPUT),
	(integer month, float day, integer year),
	float mjd,
`Given a month, day (which can include a fraction), and year in the
Gregorian calendar, return the corresponding modified Julian
day.')

FUNCTION_(toGregorian, void mjd_cal
	(double mjd, int *OUTPUT, double *OUTPUT, int *OUTPUT),
	(float mjd),
	(integer month, double day, integer year),
`Given a modified Julian day, this returns a tuple containing the
corresponding Gregorian month, day, and year.')

FUNCTION_(toDayOfWeek, int mjd_dow (double mjd, int *OUTPUT),
	(float mjd), int dayOfWeek,
`Given a modified Julian day, this returns the day of the week in
which the given time falls, from 0 (Sunday) to 6 (Saturday).')

FUNCTION_(toDaysInMonth, void mjd_dpm (double mjd, int *OUTPUT),
	(float mjd), int daysInMonth,
`Given a modified Julian day, this returns the number of days in the
month in which it falls.')

FUNCTION_(toYear, void mjd_year (double mjd, double *OUTPUT),
	(float mjd), float year,
`Convert the modified Julian day to a Gregorian year, which might
include a fraction indicating how far into that year the date fell.
Years BC are indicated by nonpositive numbers whose magnitude is one
less than the corresponding year BC.')

FUNCTION_(fromYear, void year_mjd (double year, double *OUTPUT),
	(float year), float mjd,
`Convert a year - which may include a fraction - to a modified
Julian day.')

FUNCTION_(toNearestSecond, void rnd_second (double *BOTH),
	(float mjd), float mjd,
`Return the given modified Julian day, with the result rounded to
the nearest second.')

FUNCTION_(toDay, double mjd_day (double mjd),
	(float mjd), float mjd,
`Return the given modified Julian day, with the result rounded to
the beginning of the previous day.')

FUNCTION_(toHourOfDay, double mjd_hr(double mjd),
	(float mjd), float hours,
`Return how many hours (including any fraction) the given modified
Julian day is from the previous midnight UTC.')
FUNCTION_END

SUBSECTION_(Angles)
TEXT_(`
Angles are of course critical to astronomical computations of all kinds.
The libastro library always uses radians internally when representing
angles, except in the orbital elements of a celestial body, where degrees
are used to maintain the original form of the published elements.
')

SUBSUBSECTION_(Input and Output)

FUNCTION_BEGIN
FUNCTION_(formatSexagesimal,
	void fs_sexa (char *OUT, double angle, int w, int fracbase),
	(float angle, integer w, integer fracbase),
	string output,
`Return the variable a formatted in sexagesimal format.  <B>w</B> is the number
of characters that should be reserved for the whole part.  <B>fracbase</B> is
the number of pieces a whole is to broken into, and may be one of:<PRE>
	360000:	whole:mm:ss.ss
	36000:	whole:mm:ss.s
	3600:	whole:mm:ss
	600:	whole:mm.m
	60:	whole:mm
</PRE>')

FUNCTION_(scanSexagesimal, ERRINT scansex (char *string, double *OUTPUT),
	(string input),
	float output,
`Given a string of the form xx:mm[:ss] or xx:mm.dd, convert it to a
double.  The delimiter ":" may also be ";", "/", or ",".  All
components may be floats.
Raises a ValueError if a problem in translation is encountered.')

FUNCTION_(rescanSexagesimal,
	void f_scansex(float original, char string[], double *OUTPUT),
	(float default, string angle),
	float output,
`Scans a sexagesimal string (like a time or angle, in which
fractional parts are prefixed by ":" and represent sixtieths of the
next larger unit) and returns its numerical value.  A negative
value may be indicated by a "-" character before any component.
Each component may be integral or real.  In addition to ":", the
separator may also be "/", ";", ",", or "-".  Any components not
specified are copied from the corresponding sexagesimal component
of the double provided as the first argument.')
FUNCTION_END

SUBSUBSECTION_(Arithmetic)

FUNCTION_BEGIN
FUNCTION_(deltaRA, double delra (double difference),
	(float difference), float normalized,
`Given the difference between two right ascensions, this function
normalizes the value to an angle between zero and twice pi.')
FUNCTION_END

SUBSUBSECTION_(Conversions)

FUNCTION_BEGIN
FUNCTION_(degrad, double degrad(double d) {return d*PI/180.;},
	(float d), float r, Degrees to radians.)

FUNCTION_(raddeg, double raddeg(double r) {return r*180./PI;},
	(float r), float d, Radians to degrees.)

FUNCTION_(hrdeg, double hrdeg(double h) {return h*15.;},
	(float h), float d, Hours to degrees.)

FUNCTION_(deghr, double deghr(double d) {return d/15.;},
	(float d), float h, Degrees to hours.)

FUNCTION_(hrrad, double hrrad(double h) {return degrad(hrdeg(h));},
	(float h), float r, Hours to radians.)

FUNCTION_(radhr, double radhr(double r) {return deghr(raddeg(r));},
	(float r), float h, Radians to hours.)
FUNCTION_END

SUBSECTION_(Convenient Formatting)
TEXT_(`These functions are not actually present in the raw libastro library,
 but are written just for PyEphem
 to make some common formatting operations more convenient.
 In each of these functions the variable <B>fracbase</B> has the same
 significance as in the <B>formatSexagesimal</B> function described above.')

FUNCTION_BEGIN
FUNCTION_(formatHours,
	void formatHours(double angle, int fracbase, char *OUT),
	(float angle, int fracbase), string output,
	Formats a radian angle as hours (such as those of right ascension).)
FUNCTION_(formatDegrees,
	void formatDegrees(double angle, int fracbase, char *OUT),
	(float angle, int fracbase), string output,
	Formats a radian angle as degrees.)
FUNCTION_(formatDay, void formatDay(double mjd, char *OUT),
	(float mjd), string output,
	`Formats the date corresponding to the given modified Julian day
	using the format currently selected through the preferences
	(see the Preferences section below).
	The fractional part of the day is ignored.')
FUNCTION_(formatTime,
	void formatTime(double mjd, int fracbase, char *OUT),
	(float mjd, int fracbase), string output,
	`Given the time expressed as a modified Julian day,
	this function returns a formatted representation of the time of day.')
FUNCTION_END

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

SECTION_(Objects)
TEXT_(`
The computational routines all take as input an "Obj" instance
which identifies and describes the object whose position is being calculated.
An `Obj' is represented internally by a union of several structures,
which are accessed through its various components.
So a reference to an object attribute will have the form "obj.set.field",
where the possible field sets are listed below under the class Obj.
Note that all of the subobjects will <B>not</B> be valid simultaneously.
Specifically, here are the valid field sets for each type:</P>
<TABLE BORDER>
<TR><TD><TD> <TH COLSPAN=8>Field sets
<TR><TD><TD> <TH><TT>any</TT> <TH><TT>anyss</TT> <TH><TT>f</TT>
 <TH><TT>e</TT> <TH><TT>h</TT> <TH><TT>p</TT> <TH><TT>es</TT> <TH><TT>pl</TT>
<TR><TH ROWSPAN=6>Object type
    <TH>FIXED		<TD>X<TD> <TD>X<TD> <TD> <TD> <TD> <TD> 
<TR><TH>ELLIPTICAL	<TD>X<TD>X<TD> <TD>X<TD> <TD> <TD> <TD> 
<TR><TH>HYPERBOLIC	<TD>X<TD>X<TD> <TD> <TD>X<TD> <TD> <TD> 
<TR><TH>PARABOLIC	<TD>X<TD>X<TD> <TD> <TD> <TD>X<TD> <TD> 
<TR><TH>EARTHSAT	<TD>X<TD> <TD> <TD> <TD> <TD> <TD>X<TD> 
<TR><TH>PLANET		<TD>X<TD>X<TD> <TD> <TD> <TD> <TD> <TD>X
</TABLE>
')

SUBSECTION_(Constants)

CONSTANT_BEGIN
CONSTANT_(MaxName, 14,
	`Longest possible object name, including the trailing "\0".')
CONSTANT_END

SUBSECTION_(Object types)
TEXT_(`
There are six kinds of objects,
each identified by a particular constant in its any.type field:
<TABLE>
<TR><TH ALIGN=LEFT>UNDEFOBJ <TD> An object of undefined type.
<TR><TH ALIGN=LEFT>FIXED <TD> An object whose position in the sky is fixed.
<TR><TH ALIGN=LEFT>ELLIPTICAL <TD> An object in an elliptical solar orbit.
<TR><TH ALIGN=LEFT>HYPERBOLIC <TD> An object in a hyperbolic solar orbit.
<TR><TH ALIGN=LEFT>PARABOLIC <TD> An object in a parabolic solar orbit.
<TR><TH ALIGN=LEFT>EARTHSAT <TD> A satellite in orbit around the Earth.
<TR><TH ALIGN=LEFT>PLANET <TD> One of the planets, or the sun or moon.
</TABLE>
')

enum ObjType {
    UNDEFOBJ=0,
    FIXED, ELLIPTICAL, HYPERBOLIC, PARABOLIC, EARTHSAT, PLANET, NOBJTYPES
};

SUBSECTION_(Object fields)
TEXT_(`
When the user wants to compute information about a celestial object,
he should create an instance of the class Obj
and fill in enough information so that the computation routines
can identify the object.
For a planet,
it is enough to set its type and planet code;
other objects require full sets of orbital elements.
')

typedef union {
	Obj();
	~Obj();

FIELD_BEGIN(Class Obj)
FIELD_(any, ObjAny any,	These fields are valid for all types.)
FIELD_(anyss, ObjSS anyss,	Fields valid for all solar system types.)
FIELD_(pl, ObjPl pl,	Planet fields.)
FIELD_(f, ObjF f,	Fixed object fields.)
FIELD_(e, ObjE e,	Heliocentric elliptical orbit fields.)
FIELD_(h, ObjH h,	Heliocentric hyperbolic trajectory fields.)
FIELD_(p, ObjP p,	Heliocentric parabolic trajectory fields.)
FIELD_(es, ObjES es,	Earth satellite fields.)
FIELD_END

} Obj;

typedef unsigned char ObjType_t;
typedef unsigned char ObjAge_t;

SUBSUBSECTION_(any fields)

TEXT_(Fields that are available for every object.)

TEXT_(`
Note that magnitudes are stored as scaled short integer values
in the "mag" field of the Object class.
This interface does the conversion automatically for you,
but keep it in mind in case you lose any precision of your stored values
and wonder why.
')

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
FIELD_BEGIN
FIELD_(type, ObjType_t co_type, `The object''`s current type; see above.')
FIELD_(flags, unsigned char co_flags, `Flags available for user''`s use.')
FIELD_(name, char co_name[MAXNM], `Text name of object.')
FIELD_(ra, float co_ra,
	`Geocentric/topocentric apparent/mean right ascension, in radians.')
FIELD_(dec, float co_dec,
	`Geocentric/topocentric apparent/mean declination, in radians.')
FIELD_(gaera, float co_gaera,
	`Geocentric apparent right ascension, in radians.')
FIELD_(gaedec, float co_gaedec, `Geocentric apparent declination, in radians.')
FIELD_(azimuth, float co_az, `Azimuth angle, in radians east of north.')
FIELD_(altitude, float co_alt,
	`Altitude above topocentric horizon, in radians.')
FIELD_(elongation, float co_elong, `Angular separation between
	the object and the sun, positive if the object is east of the sun.')
FIELD_(size, unsigned short co_size, `Angular size of object in arc seconds.')
FIELD_(magnitude, magnitude_t co_mag, `Visual magnitude'.)
FIELD_(age, ObjAge_t co_age, `Room for an aging value
	if the user is maintaining a dynamic database.')
FIELD_END
} ObjAny;

SUBSUBSECTION_(anyss fields)

TEXT_(Fields that apply to any solar system object.)

%name (anyss) typedef struct {
FIELD_BEGIN
FIELD_(sunDistance, float so_sdist, `Distance from object to sun in AU.')
FIELD_(earthDistance, float so_edist, `Distance from object to earth in AU.')
FIELD_(helioLongitude, float so_hlong, `Heliocentric longitude in radians.')
FIELD_(helioLatitude, float so_hlat, `Heliocentric latitude radians.')
FIELD_(phase, float so_phase, `Phase (percent illumination).')
FIELD_END
} ObjSS;

typedef unsigned char byte;

SUBSUBSECTION_(pl field)

%name (pl) typedef struct {
FIELD_BEGIN
FIELD_(code, int pl_code, One of the following planet codes:)
FIELD_END
} ObjPl;

CONSTANT_BEGIN
CONSTANT_(MERCURY, 0, `The planet Mercury.')
CONSTANT_(VENUS, 1, `The planet Venus.')
CONSTANT_(MARS, 2, `The planet Mars.')
CONSTANT_(JUPITER, 3, `The planet Jupiter.')
CONSTANT_(SATURN, 4, `The planet Saturn.')
CONSTANT_(URANUS, 5, `The planet Uranus.')
CONSTANT_(NEPTUNE, 6, `The planet Neptune.')
CONSTANT_(PLUTO, 7, `The planet Pluto.')
CONSTANT_(SUN, 8, `The Sun.')
CONSTANT_(MOON, 9, `The satellite of earth.')
CONSTANT_END

SUBSUBSECTION_(f fields)

%name (f) typedef struct {
FIELD_BEGIN
FIELD_(class, char fo_class, `Object class (see below).')
FIELD_(spectral, char fo_spect[2], `Spectral class, if appropriate.')
FIELD_(ratio, byte fo_ratio, `Minor/major diameter ratio (scaled by SRSCALE).')
FIELD_(positionAngle, byte fo_pa,
	`Position angle east of north in degrees (scaled by PASCALE).')
FIELD_(epoch, float fo_epoch, `Epoch of right ascension and declination.')
FIELD_(ra, float fo_ra, `Right ascension at given epoch, in radians.')
FIELD_(dec, float fo_dec, `Declination at given epoch, in radians.')
FIELD_END
} ObjF;

CONSTANT_BEGIN
CONSTANT_(SRSCALE, 255.0, Galaxy size ratio scale factor.)
CONSTANT_(PASCALE, (255.0/360.0), Position angle scale factor.)
CONSTANT_END

TEXT_(`
The object classes used by Ephem and its databases are:
<UL>
<LI>A - Cluster of Galaxies
<LI>B - Binary Star
<LI>C - Globular Cluster
<LI>D - Double Star
<LI>F - Diffuse Nebula
<LI>G - Spiral Galaxy
<LI>H - Spherical Galaxy
<LI>J - Radio
<LI>K - Dark Nebula
<LI>L - Pulsar
<LI>M - Multiple Star
<LI>N - Bright Nebula
<LI>O - Open Cluster
<LI>P - Planetary Nebula
<LI>Q - Quasar
<LI>R - Supernova Remnant
<LI>S - Star
<LI>T - Star-like Object
<LI>U - Cluster, with nebulosity
<LI>V - Variable Star
</UL>
')

TEXT_(`In theory any symbol from zero up to "NCLASSES" may be used
for an object type:')

CONSTANT_(NCLASSES, 128, Number of potential object classes
 (allows for all ASCII values).)

SUBSUBSECTION_(e fields)

TEXT_(Information for an object in elliptical heliocentric orbit.)

%name (e) typedef struct {
FIELD_BEGIN
FIELD_(inc, float eo_inc, `Inclination in degrees.')
FIELD_(Omega, float eo_Om, `Longitude of ascending node in degrees.')
FIELD_(omega, float eo_om, `Argument of perihelion in degress.')
FIELD_(a, float eo_a, `Mean distance (semimajor axis) in AU.')
FIELD_(e, float eo_e, `Eccentricity.')
FIELD_(M, float eo_M, `Mean anomaly (degrees from perihelion at epoch date).')
FIELD_(size, float eo_size, `Angular size in arc seconds at 1 AU distant.')
FIELD_(cepoch, double eo_cepoch, `Epoch date (M reference), as an mjd.')
FIELD_(epoch, double eo_epoch,
	`Equinox year (inc/Om/om reference), as an mjd.')
FIELD_(magnitude, Mag eo_mag, `Magnitude coefficent structure; see below.')
FIELD_END
} ObjE;

SUBSUBSECTION_(h fields)

TEXT_(Information about an object in hyperbolic heliocentric orbit.)

%name (h) typedef struct {
FIELD_BEGIN
FIELD_(epoch, double ho_epoch,
	`Equinox year (inc/Om/om reference), as an mjd.')
FIELD_(ep, double ho_ep, `Epoch of perihelion, as an mjd.')
FIELD_(inc, float ho_inc, `Inclination in degrees.')
FIELD_(Omega, float ho_Om, `Longitude of ascending node in degrees.')
FIELD_(omega, float ho_om, `Argument of perihelion in degrees.')
FIELD_(e, float ho_e, `Eccentricity.')
FIELD_(qp, float ho_qp, `Perihelion distance in AU.')
FIELD_(g, float ho_g, `Magnitude model coefficient.')
FIELD_(k, float ho_k, `Magnitude model coefficient.')
FIELD_(size, float ho_size, `Angular size in arc seconds at 1 AU.')
FIELD_END
} ObjH;

SUBSUBSECTION_(p fields)

TEXT_(Information about an object in parabolic heliocentric orbit.)

%name (p) typedef struct {
FIELD_BEGIN
FIELD_(epoch, double po_epoch, `Reference epoch, as an mjd.')
FIELD_(ep, double po_ep, `Epoch of perihelion, as an mjd.')
FIELD_(inc, float po_inc, `Inclination in degrees.')
FIELD_(qp, float po_qp, `Perihelion distance in AU.')
FIELD_(omega, float po_om, `Argument of perihelion, in degrees.')
FIELD_(Omega, float po_Om, `Longitude of ascending node, in degrees.')
FIELD_(g, float po_g, `Magnitude model coefficient.')
FIELD_(k, float po_k, `Magnitude model coefficient.')
FIELD_(size, float po_size, `Angular size in arc seconds at 1 AU.')
FIELD_END
} ObjP;

SUBSUBSECTION_(es fields)

TEXT_(Information about an earth orbiting satellite.)

%name (es) typedef struct {

TEXT_(Orbital elements:)
FIELD_BEGIN
FIELD_(epoch, double eso_epoch, `Reference epoch, as an mjd.')
FIELD_(n, double eso_n, `Mean motion in revolutions per day.')
FIELD_(inc, float eso_inc, `Inclination in degrees.')
FIELD_(raan, float eso_raan, `Right ascension of ascending node in degrees.')
FIELD_(e, float eso_e, `Eccentricity.')
FIELD_(ap, float eso_ap, `Argument of perigee at epoch, in degrees.')
FIELD_(M, float eso_M,
	`Mean anomaly, in degrees (angle from perigee at epoch).')
FIELD_(decay, float eso_decay,
	`Orbit decay rate, in revolutions per day squared.')
FIELD_(orbitNumber, int eso_orbit, `Integer orbit number of epoch.')
FIELD_END

TEXT_(Computed results:)
FIELD_BEGIN
FIELD_(elevation, float ess_elev, `Height of satellite above sea level,
	in meters.')
FIELD_(range, float ess_range,
	`The line-of-site distance from observer to satellite, .')
FIELD_(rangev, float ess_rangev, `Rate-of-change of range,
	in meters per second.')
FIELD_(sublatitude, float ess_sublat,
	`Latitude of the location below satellite,
	positive is north, in radians.')
FIELD_(sublongitude, float ess_sublng,
	`Longitude of the location below satellite,
	positive is east, in radians.')
FIELD_(isEclipsed, int ess_eclipsed,
	`Set to 1 if satellite is in earth''`s shadow, else 0.')
FIELD_END
} ObjES;

SUBSECTION_(Magnitudes)
TEXT_(`
Objects in elliptical solar orbit are allowed to employ
either of two magnitude models: the gk system or the HG system.
So instead of simple magnitude parameter fields,
elliptical objects contain a small structure called "magnitude" whose fields
and their possible values are as follows.
')

CONSTANT_BEGIN
CONSTANT_(MAG_HG, 0, Select the HG magnitude system.)
CONSTANT_(MAG_gk, 1, Select the gk magnitude system.)
CONSTANT_END

typedef struct {
FIELD_BEGIN(Class Mag)
FIELD_(m1, float m1, The value of g or H.)
FIELD_(m2, float m2, The value of k or G.)
FIELD_(whichm, int whichm, One of the constants MAG_gk or MAG_HG.)
FIELD_END
} Mag;

SUBSECTION_(Functions)

FUNCTION_BEGIN
FUNCTION_(describe, char *obj_description (Obj *object),
	(Obj o), string description,
Returns a textual description of the object.)

FUNCTION_(isDeepsky, int is_deepsky (Obj *op), (Obj o), integer boolean,
`Returns integer true or false indicating whether the object is
a deep-sky object.')
FUNCTION_END

SECTION_(Computation)

TEXT_(`
Routines that compute positions and other information
need to know where and at what time the observer is located.
This information is stored in a "Circumstance" object,
which is then provided to any of the several routines
involving time-specific calculations.
')

%name (Circumstance) typedef struct {
	Now();
	~Now();
FIELD_BEGIN(Class Circumstance)
FIELD_(mjd, double n_mjd, Modified Julian date.)
FIELD_(latitude, double n_lat,
 `Geographic (surface-normal) latitude in radians; positive is north.')
FIELD_(longitude, double n_lng,
	`Longitude in radians; positive is east of Greenwich.')
FIELD_(timezone, double n_tz,
	`Time zone, as the number of hours behind UTC.')
FIELD_(temperature, double n_temp, `Atmospheric temperature, degrees C.')
FIELD_(pressure, double n_pressure, `Atmospheric pressure, mBar.')
FIELD_(elevation, double n_elev, `Elevation above sea level, earth radii.')
FIELD_(sunDip, double n_dip,
	`Angular dip of sun below horizon at twilight,
	 as a positive number of radians.')
FIELD_(epoch, double n_epoch,
	`Desired precession epoch as an mjd,
	or the constant EOD (defined below).')
FIELD_(tzname, char n_tznm[8],
	`Time zone name (7 characters or less).')
FIELD_END
} Now;

TEXT_(`There is a single constant, for use with the epoch field:')

CONSTANT_BEGIN
CONSTANT_(EOD, (-9786), `Special epoch value indicating routines should use
	 the current date as the epoch.')
CONSTANT_END

SUBSECTION_(General Computations)

FUNCTION_BEGIN
FUNCTION_(computeLocation, int obj_cir (Now *circumstance, Obj *object),
	(Circumstance c, Object o), int error,
`Given a circumstance and an object,
the position of the object at that time and location is computed
and its field values are updated to reflect this.')

FUNCTION_(computeSeparation, double dm_separation (Obj *p, Obj *q),
	(Obj o1, Obj o2), (float separation),
	`Given two objects whose locations have been computed with
	 <B>computeLocation</B>, return the angular separation between
	 the two objects.')

FUNCTION_(computeSiderealTime,
	void now_lst (Now *circumstance, double *OUTPUT),
	(Circumstance c), float sidhours,
`Given a circumstance,
this function returns the local sidereal time (in hours).')
FUNCTION_END

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

SUBSECTION_(Risings and Settings)
TEXT_(`
Since the rising, transit, and setting times of an object
are of great interest to observational astronomers,
the library provides a routine to compute these times for an object.
')

typedef struct {
FIELD_BEGIN(Class RiseSet)
	RiseSet();
	~RiseSet();
FIELD_(flags, int rs_flags,
	`Information about what has been computed; see, below.')
FIELD_(riseTime, double rs_risetm, `Time of rising today.')
FIELD_(riseAzimuth, double rs_riseaz,
	`Azimuth of rise, in radians east of north.')
FIELD_(transitTime, double rs_trantm, `Time of transit today.')
FIELD_(transitAltitude, double rs_tranalt,
	`Altitude of transit, in radians up from the horizon.')
FIELD_(setTime, double rs_settm, `Time of setting today.')
FIELD_(setAzimuth, double rs_setaz,
	`Azimuth of set, in radians east of north.')
FIELD_END
} RiseSet;

SUBSUBSECTION_(Flags)

CONSTANT_BEGIN
CONSTANT_(RS_NORISE, 0x0001, `Object does not rise today.')
CONSTANT_(RS_NOSET, 0x0002, `Object does not set today.')
CONSTANT_(RS_NOTRANS, 0x0004, `Object does not transit today.')
CONSTANT_(RS_CIRCUMPOLAR, 0x0010, `Object stays up all day today.')
CONSTANT_(RS_NEVERUP, 0x0020, `Object is never up at all today.')
CONSTANT_(RS_ERROR, 0x1000, `Computation was completely unsuccessful.')
CONSTANT_(RS_RISERR, (0x0100|RS_ERROR), `Error occurred when computing rise.')
CONSTANT_(RS_SETERR, (0x0200|RS_ERROR), `Error occurred when computing set.')
CONSTANT_(RS_TRANSERR, (0x0400|RS_ERROR),
	`Error occurred when computing transit.')
CONSTANT_END

SUBSUBSECTION_(Functions)

FUNCTION_BEGIN
FUNCTION_(computeRiseSet, 
	void riset_cir (Now *circumstance, Obj *object,
		double distance, RiseSet *riseSet),
	(Circumstance c, Object o, float distance, RiseSet riseset),
	None,
`Given a circumstance, an object, and the angular distance in radians
below the ideal horizon at which point an object is considered to be
rising or setting,
this routine fills in the given RiseSet structure
with information about when the object will rise and set.')

FUNCTION_(computeTwilight,
	void twilight_cir (Now *circumstance, double distance,
		double *OUTPUT, double *OUTPUT, int *OUTPUT),
	(Circumstance c, float distance),
	(float mjdDawn, float mjdDusk, integer flags),
`Given a circumstance and a distance similar to that of the above function,
this returns a tuple containing the time of dawn, the time of dusk,
and a set of flags like that returned from the computeRiseSet function
indicating whether the calculation was successful.')
FUNCTION_END

SUBSECTION_(Constellation Identification)

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

FUNCTION_BEGIN
FUNCTION_(constellation,
	`void constellation(double ra, double dec, double epoch, char *OUT)',
	(float ra, float dec, float epoch), (string abbreviation),
`Given a location in the sky and an mjd epoch,
 returns the three-letter abbreviation of the constellation
 at that location.')

FUNCTION_(constellationName,
	`void constellationName(double ra, double dec, double epoch,
		char *OUT)',
	(float ra, float dec, float epoch), (string name),
`Given a location in the sky and an mjd epoch,
 returns the full scientific (Latin) name of the constellation
 at that location.')
FUNCTION_END

SECTION_(Ephem Database Format)

%{
int twoarg_db_crack_line (char s[], Obj *op)
{
	return db_crack_line(s, op, 0);
}
%}

TEXT_(`
These two functions allow you to read and write lines
in the venerable ephem database format.
')
FUNCTION_BEGIN
FUNCTION_(scanDB, ERRINT twoarg_db_crack_line (char string[], Obj *op),
	(string entry, Object o),
	None,
`Interpret the string as a database entry and fill in the object data
structure accordingly.
Raises a ValueError if the routine encounters a failure.')

FUNCTION_(formatDB, void db_write_line (Obj *op, char *OUT),
	(Object o), string entry,
`Generate a line in ephem database format describing the given object.
The line will not include a terminal newline.')
FUNCTION_END

SECTION_(Low-Level Computations)

FUNCTION_BEGIN
FUNCTION_(ap_as, void ap_as (Now *circumstance, double mjd,
		double *BOTH, double *BOTH),
	(Circumstance c, float mjd, float ra, float dec),
	(float ra, float dec),
`Converts the given apparent right ascension and declination
to the astrometrically precessed values
for the given modified Julian date.')

FUNCTION_(as_ap, void as_ap (Now *circumstance, double mjd,
		double *BOTH, double *BOTH),
	(Circumstance c, float mjd, float ra, float dec),
	(float ra, float dec),
`Calculate the given astrometric right ascension and declination
which are precessed to the modified Julian date
of the time of the circumstance.
Returns a (right ascension, declination) double.')

FUNCTION_(deltat, extern double deltat (double mjd),
	(float mjd), (float seconds),
`Number of seconds by which Terrestrial Dynamical Time (known as
Ephemeris Time prior to 1982) leads UTC.')

FUNCTION_(heliocorr,
	extern void heliocorr (double jd, double ra, double dec,
		double *OUTPUT),
	(float mjd, float ra, float dec), float hcp,
`Given geocentric time julian date
and coords of a distant object at the given right ascension
and declination (for the epoch J2000),
find the difference in time between light arriving at earth versus the sun.
"hcp" must be subtracted from geocentric jd to get heliocentric jd.
(From RLM Oct 12, 1995.)')

FUNCTION_(mm_mjed, extern double mm_mjed (Now *np),
	(Circumstance c), float mjd,
`Given a circumstance,
return its modified julian date, modified for terrestrial dynamical time.')
FUNCTION_END

SECTION_(Preferences)
TEXT_(`Two of XEphem''`s numerous options are relevant for this
 computational library.')

/* NOTE: these constants must remain in agreement with those in the
   libastro preferences.h file. */

TEXT_(`The variable <B>preference.whichEquatorial</B> sets whether locations
 should be computed from the center of the earth,
 or for a particular observer on the earth''`s surface.
 If geocentric (based on the center of the earth) computation is selected,
 most of the fields in a Circumstance object
 become irrelevant since they are used to specify a location on earth.
 It may take on either of these values:')

CONSTANT_BEGIN
CONSTANT_(topocentric, PREF_TOPO,
`Asserts that computations should be relative to an
 observer on the surface of the earth (the default).')
CONSTANT_(geocentric, PREF_GEO,
`Asserts that computations should be relative to the
 center of the earth.')
CONSTANT_END

TEXT_(`The variable <B>preference.whichDateFormat</B> selects the manner
 in which the <B>formatDate</B> function formats its output.
 Its value should be one of:')

CONSTANT_BEGIN
CONSTANT_(MDY,PREF_MDY,
`Asserts that dates should be expressed as month/day/year
 in the manner familiar to Americans (this option is selected as the default
 to conform to the default behavior of XEphem,
 even though this system makes less sense than the other two).')
CONSTANT_(YMD,PREF_YMD,
`Asserts that dates should be expressed as year/month/day
 as commonly used by astronomers.')
CONSTANT_(DMY,PREF_DMY,
`Asserts that dates should be expressed as day/month/year
 as used by Europeans.')
CONSTANT_END

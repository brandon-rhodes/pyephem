#include <Python.h>
#include <structmember.h>

#include <string.h>

#include "P_.h"
#include "astro.h"
#include "circum.h"
#include "preferences.h"

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

#include "objects.c"
#include "basics.c"
#include "angle_type.c"
#include "date_type.c"
#include "getset.c"
#include "observer_type.c"
#include "body_types.c"
#include "extras.c"

/* -------- astro.h --------
 *
 * aa_hadec - observation from geographic location -> ra and dec.
 * hadec_aa - geographic location and ra and dec -> alt and az.
 *
 * ab_ecl - aberration correction to sky coordinates
 * ab_eq - ditto
 *
 * airmass - apparent altitude -> airmasses
 *
 * anomaly - find true anomaly from mean anomaly and eccintricity
 *
 *(chap95 - used by plans() for Jupiter through Pluto)
 *
 *(comet - used by obj_cir for parabolic orbits)
 *
 * deltat - difference between ephemeris time and universal time
 *
 * eq_ecl, ecl_eq - converts between equatorial and ecliptic coordinates
 *   for a particular date
 *
 * fs_sexa - format sexigesimal
 *[fs_date - format date; we always use (year, month, day.fraction) tuple]
 * f_scansex - scan sexigesimal (relative to earlier value)
 *[f_sscandate - interpret date string; we always just accept tuple]
 * scansex - scan sexigesimal (absolute)
 *
 * heliocorr - find difference in time between light of distant object
 *   arriving at earth and sun
 *
 * llibration - Moon.libration()
 *
 *(zero_mem - like memset-zero, used internally by libastro)
 * tickmarks - fill in array of useful tick marks for given range of values
 * lc - find segment of line that lies inside a circle
 * hg_mag - compute visual magnitude using H/G parameters
 * magdiam - scale an object given the smallest magnitude on your screen
 *   and at what magnitude intervals you want screen size to increase
 * gk_mag - compute visual magnitude of object with g/k parameters
 * atod - convert string to double
 * solve_sphere - solve unknown angle and side of spherical triangle
 * delra - map difference between two ra's into normal range
 *
 * cal_mjd -
 * mjd_cal -
 * mjd_dow -
 * isleapyear -
 * mjd_dpm -
 * mjd_year -
 * year_mjd -
 * rnd_second -
 * mjd_dayno -
 * mjd_day -
 * mjd_hr -
 * range -
 *
 *(moon - compute position of moon)
 *
 * moon_colong - Moon.illumination()
 *
 * nutation - return nutation in obliquity and longitude for given date
 *(nut_eq - correct an ra and dec in-place for nutation)
 *
 * obliquity - find mean obliquity of ecliptic for given date
 *
 * ta_par - compute apparent height angle and declination for observer
 *   given his latitude and altitude above sea level
 *
 *(plans - find position in space of a planet)
 *
 * precess - convert ra and dec from one precession epoch to another
 *
 * reduce_elements - convert orbital elements from one epoch to another
 *
 * unrefract, refract - convert between apparent and true altitude of
 *   an observation given the altitude and atmospheric conditions
 *
 * satrings - Saturn.ringtilt()
 *
 * riset - compute rising and setting times of object at an ra and dec
 *
 * sphcart, cartsph - convert spherical to cartesian coordinates
 *
 *(sunpos - return position of sun)
 *
 *(vrc - two-body calculation)
 *
 * utc_gst, gst_utc - convert between (mjd + utc) and mean siderial time
 *
 *(vsop87 - planetary theory used to compute their positions)
 *
 * -------- chap95.h --------
 *(routines that compute positions of outer planets)
 *
 * -------- circum.h --------
 * ap_as - ?
 * as_ap - ?
 *
 * mm_mjed - given an mjd, return it modified for terrestial dynamical time
 *
 * obj_cir - in Body.compute()
 *
 *(obj_earthsat - called by obj_cir for satellites)
 *
 * db_crack_line - !
 * db_write_line - !
 * get_fields - return field as string
 * db_chk_planet - check whether a name matches that of a planet
 * db_tle - ?
 *
 * now_lst - now -> local sidereal time
 * radec2ha - convert ra to ha
 * obj_description - !
 * is_deepsky - return whether object is deep-sky ?
 *
 * riset_cir - compute rising and setting times for object
 * twilight_cir - find when sun is given number of radians below horizon
 *
 * -------- deepconst.h --------
 *(constants for deep space routines in deep.c)
 *
 * -------- preferences.h --------
 *
 * -------- satlib.h --------
 *(data definitions for earthsat.h and satspec.h)
 *
 * -------- satspec.h --------
 *(data definitions for deep.c, sdp4.c, and sgp4.c)
 *
 * -------- sattypes.h --------
 *(data definitions for earthsat.c, satspec.c, sdp4.c, and sgp4.)c
 *
 * -------- vector.h --------
 *(data definitions for earthsat.c sdp4.c, and sgp4.c)
 *
 * -------- vsop87.h --------
 *(routines that are used for the positions of the inner planets)
 */

/*
 * The global methods table and the module initialization function.
 */

#define planet_builder_method(name) \
 {#name, (PyCFunction) name, METH_VARARGS | METH_KEYWORDS, \
  "Return a new instance of the planet " #name}

static PyMethodDef ephem_methods[] = {
     planet_builder_method(Mercury),
     planet_builder_method(Venus),
     planet_builder_method(Mars),
     planet_builder_method(Jupiter),
     planet_builder_method(Saturn),
     planet_builder_method(Uranus),
     planet_builder_method(Neptune),
     planet_builder_method(Pluto),
     planet_builder_method(Sun),
     planet_builder_method(Moon),
     /*{"date", rebuild_date, METH_VARARGS, "Parse a date"},*/
     {"degrees", degrees, METH_VARARGS, "build an angle"},
     {"hours", hours, METH_VARARGS, "build an angle"},
     {"now", build_now, METH_VARARGS, "Return the current time"},
     {"readdb", readdb, METH_VARARGS, "Read an ephem database entry"},
     {"readtle", readtle, METH_VARARGS, "Read TLE-format satellite elements"},
     {"separation", separation, METH_VARARGS,
      "Return the angular separation between two objects or positions "
      "(degrees)"},
     {"constellation", (PyCFunction) constellation,
      METH_VARARGS | METH_KEYWORDS,
      "Return the constellation in which the object or coordinates lie"},
     {NULL, NULL, 0, NULL}
};

#define CONSTRUCTOR(name, type) \
 if (PyDict_SetItemString(d, name, (PyObject*) &type)) \
  return;

DL_EXPORT(void)
initephem(void)
{
     PyObject *m, *d, *o;

     PyType_Ready(&Angle_Type);
     PyType_Ready(&Date_Type);

     PyType_Ready(&Observer_Type);

     PyType_Ready(&FixedBody_Type);
     PyType_Ready(&Planet_Type);
     PyType_Ready(&Moon_Type);
     PyType_Ready(&Saturn_Type);
     PyType_Ready(&EllipticalBody_Type);
     PyType_Ready(&HyperbolicBody_Type);
     PyType_Ready(&ParabolicBody_Type);
     PyType_Ready(&EarthSatellite_Type);

     m = Py_InitModule("ephem", ephem_methods);
     d = PyModule_GetDict(m);

     CONSTRUCTOR("date", Date_Type);
     CONSTRUCTOR("Observer", Observer_Type);
     CONSTRUCTOR("FixedBody", FixedBody_Type);
     CONSTRUCTOR("EllipticalBody", EllipticalBody_Type);
     CONSTRUCTOR("ParabolicBody", ParabolicBody_Type);
     CONSTRUCTOR("HyperbolicBody", HyperbolicBody_Type);
     CONSTRUCTOR("EarthSatellite", EarthSatellite_Type);
     
     o = PyFloat_FromDouble(1./24.);
     if (!o || PyDict_SetItemString(d, "hour", o)) return;

     o = PyFloat_FromDouble(1./24./60.);
     if (!o || PyDict_SetItemString(d, "minute", o)) return;

     o = PyFloat_FromDouble(1./24./60./60.);
     if (!o || PyDict_SetItemString(d, "second", o)) return;
     
     pref_set(PREF_DATE_FORMAT, PREF_YMD);
}

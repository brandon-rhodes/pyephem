#include <Python.h>
#include <structmember.h>

#include <string.h>

#include "astro.h"
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

typedef struct {
     PyObject_HEAD
     Now now;
} ObserverObject;

typedef struct {
     PyObject_HEAD
     Now now;			/* cache of last observer */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     unsigned now_valid : 2;	/* 0=no, 1=mjd and epoch only, 2=all */
     unsigned obj_valid : 2;	/* 0=no, 1=geocentric only, 2=all */
     unsigned riset_valid : 2;	/* 0=no, 2=yes */
} BodyObject;

typedef struct {
     PyObject_HEAD
     Now now;			/* cache of last observer */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     unsigned now_valid : 2;	/* 0=no, 1=mjd and epoch only, 2=all */
     unsigned obj_valid : 2;	/* 0=no, 1=geocentric only, 2=all */
     unsigned riset_valid : 2;	/* 0=no, 2=yes */
     unsigned llibration_valid : 2; /* 0=no, nonzero=yes */
     unsigned moon_colong_valid : 2; /* 0=no, nonzero=yes */
     double llat, llon;		/* libration */
     double c, k, s;		/* co-longitude and illumination */
} MoonObject;

typedef struct {
     PyObject_HEAD
     Now now;			/* cache of last observer */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     unsigned now_valid : 2;	/* 0=no, 1=mjd and epoch only, 2=all */
     unsigned obj_valid : 2;	/* 0=no, 1=geocentric only, 2=all */
     unsigned riset_valid : 2;	/* 0=no, 2=yes */
     unsigned satrings_valid : 2; /* 0=no, nonzero=yes */
     double etilt, stilt;	/* tilt of rings */
} SaturnObject;

static double mjd_now(void)
{
     return 25567.5 + time(NULL)/3600.0/24.0;
}

static int PyNumber_AsDouble(PyObject *o, double *dp)
{
     PyObject *f = PyNumber_Float(o);
     if (!f) return -1;
     *dp = PyFloat_AsDouble(f);
     Py_DECREF(f);
     return 0;
}

staticforward PyTypeObject Angle_Type;

typedef struct {
     PyFloatObject f;
     double factor;
} AngleObject;

static PyObject* new_Angle(double radians, double factor)
{
     AngleObject *ea;
     ea = PyObject_NEW(AngleObject, &Angle_Type);
     if (ea) {
	  ea->f.ob_fval = radians;
	  ea->factor = factor;
     }
     return (PyObject*) ea;
}

static char *angle_format(PyObject *self)
{
     AngleObject *ea = (AngleObject*) self;
     static char buffer[12 + 1];
     fs_sexa(buffer, ea->f.ob_fval * ea->factor, 3, 360000);
     return buffer[0] != ' ' ? buffer
	  : buffer[1] != ' ' ? buffer + 1
	  : buffer + 2;
}

static PyObject* angle_str(PyObject *self)
{
     return PyString_FromString(angle_format(self));
}

static int angle_print(PyObject *self, FILE *fp, int flags)
{
     fputs(angle_format(self), fp);
     return 0;
}

static PyTypeObject Angle_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "angle",
     sizeof(AngleObject),
     0,
     0,				/* tp_dealloc */
     angle_print,		/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     angle_str,			/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     0,				/* tp_members */
     0,				/* tp_getset */
     0, /*&PyFloat_Type*/	/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

staticforward PyTypeObject Date_Type;

typedef PyFloatObject DateObject;

/*
 * Modified Julian dates are read and written as (year, month, day)
 * tuples where the day can include a fraction.
 */

static int parse_mjd_from_number(PyObject *o, double *mjdp)
{
     return PyNumber_AsDouble(o, mjdp);
}

static int parse_mjd_from_string(PyObject *so, double *mjdp)
{
     char *s = PyString_AsString(so);
     char datestr[32], timestr[32];
     double day = 1.0;
     int year, month = 1, dchars, tchars;
     int conversions = sscanf(s, " %31[-0-9/.] %n%31[0-9:.] %n",
			      datestr, &dchars, timestr, &tchars);
     if (conversions == -1 || !conversions ||
	 (conversions == 1 && s[dchars] != '\0') ||
	 (conversions == 2 && s[tchars] != '\0')) {
	  PyErr_SetString(PyExc_ValueError,
			  "your date string does not seem to have "
			  "year/month/day followed optionally by "
			  "hours:minutes:seconds");
	  return -1;
     }
     f_sscandate(datestr, PREF_YMD, &month, &day, &year);
     if (conversions == 2) {
	  double hours;
	  if (f_scansexa(timestr, &hours) == -1) {
	       PyErr_SetString(PyExc_ValueError,
			       "the second field of your time string does "
			       "appear to be hours:minutes:seconds");
	       return -1;
	  }
	  day += hours / 24.;
     }
     cal_mjd(month, day, year, mjdp);
     return 0;
}

static int parse_mjd_from_tuple(PyObject *value, double *mjdp)
{
     double day = 1.0, hours = 0.0, minutes = 0.0, seconds = 0.0;
     int year, month = 1;
     if (!PyArg_ParseTuple(value, "i|idddd:date tuple", &year, &month, &day,
			   &hours, &minutes, &seconds)) return -1;
     cal_mjd(month, day, year, mjdp);
     if (hours) *mjdp += hours / 24.;
     if (minutes) *mjdp += minutes / (24. * 60.);
     if (seconds) *mjdp += seconds / (24. * 60. * 60.);
     return 0;
}

static int parse_mjd(PyObject *value, double *mjdp)
{
     if (PyNumber_Check(value))
	  return parse_mjd_from_number(value, mjdp);
     else if (PyString_Check(value))
	  return parse_mjd_from_string(value, mjdp);
     else if (PyTuple_Check(value))
	  return parse_mjd_from_tuple(value, mjdp);
     PyErr_SetString(PyExc_ValueError,
		     "dates must be specified by a number, string, or tuple");
     return -1;
}

/* Helper function used by some of the below. */

static void mjd_six(double mjd, int *yearp, int *monthp, int *dayp,
		    int *hourp, int *minutep, double *secondp)
{
     double fday, fhour, fminute;
     mjd_cal(mjd, monthp, &fday, yearp);
     *dayp = (int) fday;
     fhour = fmod(fday, 1.0) * 24;
     *hourp = (int) fhour;
     fminute = fmod(fhour, 1.0) * 60;
     *minutep = (int) fminute;
     *secondp = fmod(fminute, 1.0) * 60;
}

static PyObject* build_date(double mjd)
{
     DateObject *new = PyObject_New(PyFloatObject, &Date_Type);
     if (new) new->ob_fval = mjd;
     return (PyObject*) new;
}

static PyObject *date_new(PyObject *self, PyObject *args, PyObject *kw)
{
     PyObject *arg;
     double mjd;
     if (kw) {
	  PyErr_SetString(PyExc_TypeError,
			  "this function does not accept keyword arguments");
	  return 0;
     }
     if (!PyArg_ParseTuple(args, "O", &arg)) return 0;
     if (parse_mjd(arg, &mjd)) return 0;
     return build_date(mjd);
}

static char *date_format(PyObject *self)
{
     DateObject *d = (DateObject*) self;
     static char buffer[64];
     int year, month, day, hour, minute;
     double second;
     mjd_six(d->ob_fval, &year, &month, &day, &hour, &minute, &second);
     sprintf(buffer, "%d/%d/%d %02d:%02d:%02d",
	     year, month, day, hour, minute, (int) second);
     return buffer;
}

static PyObject* date_str(PyObject *self)
{
     return PyString_FromString(date_format(self));
}

static int date_print(PyObject *self, FILE *fp, int flags)
{
     char *s = date_format(self);
     fputs(s, fp);
     return 0;
}

static PyObject *date_triple(PyObject *self, PyObject *args)
{
     int year, month;
     double day;
     DateObject *d = (DateObject*) self;
     if (!PyArg_ParseTuple(args, "")) return 0;
     mjd_cal(d->ob_fval, &month, &day, &year);
     return Py_BuildValue("iid", year, month, day);
}

static PyObject *date_tuple(PyObject *self, PyObject *args)
{
     int year, month;
     double fday, fhour, fminute, fsecond;
     int day, hour, minute;
     DateObject *d = (DateObject*) self;
     if (!PyArg_ParseTuple(args, "")) return 0;
     mjd_cal(d->ob_fval, &month, &fday, &year);
     day = (int) fday;
     fhour = fmod(fday, 1.0) * 24;
     hour = (int) fhour;
     fminute = fmod(fhour, 1.0) * 60;
     minute = (int) fminute;
     fsecond = fmod(fminute, 1.0) * 60;
     return Py_BuildValue("iiiiid", year, month, day, hour, minute, fsecond);
}

static PyMethodDef date_methods[] = {
     {"triple", date_triple, METH_VARARGS,
      "Return the date as a (year, month, day.fraction) triple"},
     {"tuple", date_tuple, METH_VARARGS, "generate db entry"},
     {NULL, NULL, 0, NULL}
};

static PyTypeObject Date_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "date",
     sizeof(PyFloatObject),
     0,
     0,				/* tp_dealloc */
     date_print,		/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     date_str,			/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     date_methods,		/* tp_methods */
     0,				/* tp_members */
     0,				/* tp_getset */
     0, /*&PyFloat_Type,*/	/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     (newfunc) date_new,	/* tp_new */
     0,				/* tp_free */
};

/*
 * Both pairs of routines below have to access a structure member to
 * which they have been given the offset.
 */

#define THE_NUMBER \
 ((sp->type == T_FLOAT) ? \
   (* (float*) ((char*) self + m->offset)) : \
   (* (double*) ((char*) self + m->offset)))

#define THE_FLOAT (* (float*) ((char*)self + (int)v))
#define THE_DOUBLE (* (double*) ((char*)self + (int)v))

/*
 * Sexigesimal values can be assigned either floating-point numbers or
 * strings like '33:45:10', and return special Angle floats that
 * give pretty textual representations when asked for their str().
 */

typedef struct {
     int type;			/* T_FLOAT or T_DOUBLE */
     int offset;		/* offset of structure member */
     double ifactor;		/* internal units per radian */
     double efactor;		/* external units per radian */
} AngleMember;

static int parse_angle(PyObject *value, double factor, double *result)
{
     if (PyNumber_Check(value)) {
	  return PyNumber_AsDouble(value, result);
     } else if (PyString_Check(value)) {
	  double scaled;
	  char *s = PyString_AsString(value);
	  char *sc;
	  for (sc=s; *sc && *sc != ':' && *sc != '.'; sc++) ;
	  if (*sc == ':')
	       f_scansexa(s, &scaled);
	  else
	       scaled = atof(s);
	  *result = scaled / factor;
	  return 0;
     } else {
	  PyErr_SetString(PyExc_TypeError,
			  "angle can only be created from string or number");
	  return -1;
     }
}

static double to_angle(PyObject *value, double efactor, int *status)
{
     double r;
     if (PyNumber_Check(value)) {
	  value = PyNumber_Float(value);
	  if (!value) {
	       *status = -1;
	       return 0;
	  }
	  r = PyFloat_AsDouble(value);
	  Py_DECREF(value);
	  *status = 0;
	  return r;
     } else if (PyString_Check(value)) {
	  double scaled;
	  char *sc, *s = PyString_AsString(value);
	  for (sc=s; *sc && *sc != ':' && *sc != '.'; sc++) ;
	  if (*sc == ':')
	       f_scansexa(s, &scaled);
	  else
	       scaled = atof(s);
	  *status = 0;
	  return scaled / efactor;
     } else {
	  PyErr_SetString(PyExc_TypeError,
			  "can only update value with String or number");
	  *status = -1;
	  return 0;
     }
}

/* Hours stored as radian double. */

static PyObject* getf_rh(PyObject *self, void *v)
{
     return new_Angle(THE_FLOAT, radhr(1));
}

static int setf_rh(PyObject *self, PyObject *value, void *v)
{
     int status;
     THE_FLOAT = (float) to_angle(value, radhr(1), &status);
     return status;
}

/* Degrees stored as radian double. */

static PyObject* getd_rd(PyObject *self, void *v)
{
     return new_Angle(THE_DOUBLE, raddeg(1));
}

static int setd_rd(PyObject *self, PyObject *value, void *v)
{
     int status;
     THE_DOUBLE = to_angle(value, raddeg(1), &status);
     return status;
}

/* Degrees stored as radian float. */

static PyObject* getf_rd(PyObject *self, void *v)
{
     return new_Angle(THE_FLOAT, raddeg(1));
}

static int setf_rd(PyObject *self, PyObject *value, void *v)
{
     int status;
     THE_FLOAT = (float) to_angle(value, raddeg(1), &status);
     return status;
}

/* Degrees stored as degrees, but for consistency we return their
   floating point value as radians. */

static PyObject* getf_dd(PyObject *self, void *v)
{
     return new_Angle(THE_FLOAT * degrad(1), raddeg(1));
}

static int setf_dd(PyObject *self, PyObject *value, void *v)
{
     int status;
     THE_FLOAT = (float) to_angle(value, raddeg(1), &status);
     return status;
}

/* MJD stored as float. */

static PyObject* getf_mjd(PyObject *self, void *v)
{
     return build_date(THE_FLOAT);
}

static int setf_mjd(PyObject *self, PyObject *value, void *v)
{
     double result;
     if (parse_mjd(value, &result)) return -1;
     THE_FLOAT = (float) result;
     return 0;
}

/* MDJ stored as double. */

static PyObject* getd_mjd(PyObject *self, void *v)
{
     return build_date(THE_DOUBLE);
}

static int setd_mjd(PyObject *self, PyObject *value, void *v)
{
     double result;
     if (parse_mjd(value, &result)) return -1;
     THE_DOUBLE = result;
     return 0;
}

#undef THE_FLOAT
#undef THE_DOUBLE

/*
 * The following values are ones for which XEphem provides special
 * routines for their reading and writing, usually because they are
 * encoded into one or two bytes to save space in the obj structure
 * itself.  These are simply wrappers around those functions.
 */

/* Magnitude. */

static PyObject* get_s_mag(PyObject *self, void *v)
{
     BodyObject *b = (BodyObject*) self;
     return PyFloat_FromDouble(get_mag(&b->obj));
}

/* Spectral codes. */

static PyObject* get_f_spect(PyObject *self, void *v)
{
     BodyObject *b = (BodyObject*) self;
     return PyString_FromStringAndSize(b->obj.f_spect, 2);
}

static int set_f_spect(PyObject *self, PyObject *value, void *v)
{
     BodyObject *b = (BodyObject*) self;
     char *s;
     if (!PyString_Check(value)) {
	  PyErr_SetString(PyExc_ValueError, "spectral code must be a string");
	  return -1;
     }
     if (PyString_Size(value) != 2) {
	  PyErr_SetString(PyExc_ValueError,
			  "spectral code must be two characters long");
	  return -1;
     }
     s = PyString_AsString(value);
     b->obj.f_spect[0] = s[0];
     b->obj.f_spect[1] = s[1];
     return 0;
}

/* Fixed object diameter ratio. */

static PyObject* get_f_ratio(PyObject *self, void *v)
{
     BodyObject *b = (BodyObject*) self;
     return PyFloat_FromDouble(get_ratio(&b->obj));
}

static int set_f_ratio(PyObject *self, PyObject *value, void *v)
{
     BodyObject *b = (BodyObject*) self;
     float maj, min;
     if (!PyArg_ParseTuple(value, "ff", &maj, &min)) return -1;
     set_ratio(&b->obj, maj, min);
     return 0;
}

/* Position angle of fixed object. */

static PyObject* get_f_pa(PyObject *self, void *v)
{
     BodyObject *b = (BodyObject*) self;
     return PyFloat_FromDouble(get_pa(&b->obj));
}

static int set_f_pa(PyObject *self, PyObject *value, void *v)
{
     BodyObject *b = (BodyObject*) self;
     if (!PyNumber_Check(value)) {
	  PyErr_SetString(PyExc_ValueError, "position angle must be a float");
	  return -1;
     }
     set_pa(&b->obj, PyFloat_AsDouble(value));
     return 0;
}

/*
 * Observer object.
 */

static PyTypeObject Observer_Type;

/*
 * Constructor and methods.
 */

static PyObject *observer_new(PyObject *self, PyObject *args, PyObject *kw)
{
     ObserverObject *o;
     if (kw) {
	  PyErr_SetString(PyExc_TypeError,
			  "this function does not accept keyword arguments");
	  return 0;
     }
     if (!PyArg_ParseTuple(args, ":Observer")) return 0;
     o = PyObject_NEW(ObserverObject, &Observer_Type);
     if (o) {
	  o->now.n_mjd = mjd_now();
	  o->now.n_epoch = J2000;
	  o->now.n_lat = o->now.n_lng = o->now.n_elev = 0;
	  o->now.n_temp = 15.0;
	  o->now.n_pressure = 1013;
	  o->now.n_dip = 0;
	  o->now.n_tz = 0;
     }
     return (PyObject*) o;
}

/*
 * Member access.
 */

static PyObject *get_elev(PyObject *self, void *v)
{
     ObserverObject *o = (ObserverObject*) self;
     return PyFloat_FromDouble(o->now.n_elev * ERAD);
}

static int set_elev(PyObject *self, PyObject *value, void *v)
{
     int r;
     double n;
     ObserverObject *o = (ObserverObject*) self;
     if (!PyNumber_Check(value)) {
	  PyErr_SetString(PyExc_TypeError, "Elevation must be numeric");
	  return -1;
     }
     r = PyNumber_AsDouble(value, &n);
     if (!r) o->now.n_elev = n / ERAD;
     return 0;
}

/*
 * Observer class type.
 */

static PyMethodDef ephem_observer_methods[] = {
     {NULL, NULL, 0, NULL}
};

#define VOFF(member) ((void*) OFF(member))
#define OFF(member) offsetof(ObserverObject, now.member)

static PyGetSetDef ephem_observer_getset[] = {
     {"date", getd_mjd, setd_mjd, "Date", VOFF(n_mjd)},
     {"lat", getd_rd, setd_rd, "Latitude (degrees north)", VOFF(n_lat)},
     {"long", getd_rd, setd_rd, "Longitude (degrees east)", VOFF(n_lng)},
     {"elev", get_elev, set_elev, "Elevation above sea level (meters)", NULL},
     /*{"dip", getd_rd, setd_rd,
       "dip of sun below horizon at twilight (degrees)", VOFF(n_dip)},*/
     {"epoch", getd_mjd, setd_mjd, "Precession epoch", VOFF(n_epoch)},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyMemberDef ephem_observer_members[] = {
     {"temp", T_DOUBLE, OFF(n_temp), 0, "atmospheric temperature (C)"},
     {"pressure", T_DOUBLE, OFF(n_pressure), 0,
      "atmospheric pressure (mBar)"},
     {NULL, 0, 0, 0, NULL}
};

#undef OFF

static PyTypeObject Observer_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "Observer",
     sizeof(ObserverObject),
     0,
     0,	/*_PyObject_Del*/	/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     0,				/* tp_str */
     0, /*PyObject_GenericGetAttr,*/ /* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT,	/* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     ephem_observer_methods,	/* tp_methods */
     ephem_observer_members,	/* tp_members */
     ephem_observer_getset,	/* tp_getset */
     0,				/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     (newfunc) observer_new,	/* tp_new */
     /*_PyObject_GC_Del,*/	/* tp_free */
};

/*
 * XEphem Obj's are more complicated, both because of the several
 * classes of object - for which different fields are valid - and
 * because they support more functions.
 */

staticforward PyTypeObject Body_Type;
staticforward PyTypeObject FixedBody_Type;
staticforward PyTypeObject Planet_Type;
staticforward PyTypeObject Saturn_Type;
staticforward PyTypeObject Moon_Type;
staticforward PyTypeObject EllipticalBody_Type;
staticforward PyTypeObject HyperbolicBody_Type;
staticforward PyTypeObject ParabolicBody_Type;
staticforward PyTypeObject EarthSatellite_Type;

static PyObject* body_compute
(PyObject *self, PyObject *args, PyObject *kwdict);

/* Object initialization. */

static int body_init(PyObject *self, PyObject *args, PyObject *kw)
{
     BodyObject *body = (BodyObject*) self;
     body->now_valid = body->obj_valid = body->riset_valid = 0;
     return 0;
}

static int saturn_init(PyObject *self, PyObject *args, PyObject *kw)
{
     SaturnObject *saturn = (SaturnObject*) self;
     saturn->satrings_valid = 0;
     return body_init(self, args, kw);
}

static int moon_init(PyObject *self, PyObject *args, PyObject *kw)
{
     MoonObject *moon = (MoonObject*) self;
     moon->llibration_valid = 0;
     moon->moon_colong_valid = 0;
     return body_init(self, args, kw);
}

/* Object constructors. */

static PyObject* build_planet
(PyObject* self, PyObject* args, PyObject *kw,
 PyTypeObject *bodytype, char *name, int typecode)
{
     PyObject *compute_result = 0;
     BodyObject *body;
     
     body = (BodyObject*) bodytype->tp_new(bodytype, 0, 0);
     if (!body) return 0;
     body_init((PyObject*) body, 0, 0);
     strcpy(body->obj.o_name, name);
     body->obj.o_type = PLANET; 
     body->obj.pl.plo_code = typecode;
     if (PyTuple_Check(args) && PyTuple_Size(args)) {
	  compute_result = body_compute((PyObject*) body, args, kw);
	  if (! compute_result) return 0;
	  Py_DECREF(compute_result);
     }
     return (PyObject*) body;
}

#define planet_builder(name, typecode, bodytype) \
static PyObject* name \
(PyObject* self, PyObject* args, PyObject *kw) \
{ \
     return build_planet(self, args, kw, &bodytype, #name, typecode); \
}

planet_builder(Mercury, MERCURY, Planet_Type)
planet_builder(Venus, VENUS, Planet_Type)
planet_builder(Mars, MARS, Planet_Type)
planet_builder(Jupiter, JUPITER, Planet_Type)
planet_builder(Saturn, SATURN, Saturn_Type)
planet_builder(Uranus, URANUS, Planet_Type)
planet_builder(Neptune, NEPTUNE, Planet_Type)
planet_builder(Pluto, PLUTO, Planet_Type)
planet_builder(Sun, SUN, Planet_Type)
planet_builder(Moon, MOON, Moon_Type)

/*
 * Perhaps the central function of the entire package: this calls the
 * X routine to compute the position and disposition of the body
 * and fills in the body's parameters with the result.
 *
 * This is a method of every Body, and takes as its argument
 * either a full-blown Observer, in which case a topocentric
 * computation is made, or a date (and optional epoch) in which case a
 * simple geocentric computation is made.
 */

static PyObject* body_compute
(PyObject *self, PyObject *args, PyObject *kwdict)
{
     BodyObject *body = (BodyObject*) self;
     static char *kwlist[] = {"when", "epoch", 0};
     PyObject *when_arg = 0, *epoch_arg = 0;
     double mjd = 0, epoch;

     if (!PyArg_ParseTupleAndKeywords
	 (args, kwdict, "|OO", kwlist, &when_arg, &epoch_arg))
	  return 0;

     if (when_arg && PyObject_TypeCheck(when_arg, &Observer_Type)) {

	  /* compute(observer) */

	  ObserverObject *observer = (ObserverObject*) when_arg;
	  if (epoch_arg) {
	       PyErr_SetString(PyExc_ValueError,
			       "cannot supply an epoch= keyword argument "
			       "with an Observer because it specifies "
			       "its own epoch");
	       return 0;
	  }
	  body->now = observer->now;
	  body->now_valid = 2;
     } else {

	  /* compute(date, epoch) where date defaults to current time
	     and epoch defaults to 2000.0 */

	  if (when_arg) {
	       if (parse_mjd(when_arg, &mjd) == -1) goto fail;
	  } else
	       mjd = mjd_now();

	  if (epoch_arg) {
	       if (parse_mjd(epoch_arg, &epoch) == -1) goto fail;
	  } else
	       epoch = J2000;
     
	  body->now.n_mjd = mjd;
	  body->now.n_epoch = epoch;
	  body->now_valid = 1;
     }

     /* Mark all field collections as invalid. */

     body->obj_valid = body->riset_valid = 0;

     Py_INCREF(Py_None);
     return Py_None;

fail:
     return 0;
}

static PyObject* body_writedb(PyObject *self, PyObject *args)
{
     BodyObject *body = (BodyObject*) self;
     char line[1024];
     if (!PyArg_ParseTuple(args, "")) return 0;
     db_write_line(&body->obj, line);
     return PyString_FromString(line);
}

static PyObject* body_str(PyObject *body_object)
{
     BodyObject *body = (BodyObject*) body_object;
     char *format = body->obj.o_type == PLANET ? "<ephem.%s %s>" :
	  body->obj.o_name[0] ? "<ephem.%s \"%s\">" :
	  "<ephem.%s%s>";
     return PyString_FromFormat
	  (format, body->ob_type->tp_name, body->obj.o_name);
}

static PyMethodDef body_methods[] = {
     {"compute", (PyCFunction) body_compute,
      METH_VARARGS | METH_KEYWORDS, "compute it"},
     {"writedb", body_writedb, METH_VARARGS, "generate db entry"},
     {NULL, NULL, 0, NULL}
};

#define OFF(member) offsetof(BodyObject, obj.member)

static PyObject* build_hours(double radians)
{
     return new_Angle(radians, radhr(1));
}

static PyObject* build_degrees(double radians)
{
     return new_Angle(radians, raddeg(1));
}

static PyObject* build_mag(double raw)
{
     return PyFloat_FromDouble(raw / MAGSCALE);
}

/* These are the functions which are each responsible for filling in
   some of the fields of an ephem.Body or one of its subtypes.  When
   called they determine whether the information in the fields for
   which they are responsible is up-to-date, and re-compute them if
   not.  By checking body->now_valid they can deteremine how much
   information the last compute() call supplied. */

static int body_obj_cir(BodyObject *body, char *fieldname, unsigned threshold)
{
     if (!body->obj_valid) {
	  if (body->now_valid == 0) {
	       if (fieldname)
		    PyErr_Format(PyExc_RuntimeError,
				 "field %s undefined until first compute()",
				 fieldname);
	       return -1;
	  }
	  pref_set(PREF_EQUATORIAL,
		   body->now_valid == 1 ? PREF_GEO : PREF_TOPO);
	  obj_cir(& body->now, & body->obj);
	  body->obj_valid = body->now_valid;
     }
     if (body->obj_valid < threshold) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined because previous compute() "
		       "was supplied a date rather than an Observer",
		       fieldname);
	  return -1;
     }
     return 0;
}

static int body_riset_cir(BodyObject *body, char *fieldname)
{
     if (body->riset_valid) return 0;
     if (body->now_valid == 0) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined until first compute()", fieldname);
	  return -1;
     }
     if (body->now_valid == 1) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined because last compute() supplied "
		       "a date rather than an Observer", fieldname);
	  return -1;
     }
     body->riset_valid = 2;
     riset_cir(& body->now, & body->obj,
	       body->now.n_dip, & body->riset);
     return 0;
}

static int moon_llibration(MoonObject *moon, char *fieldname)
{
     if (moon->llibration_valid) return 0;
     if (moon->now_valid == 0) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined until first compute()", fieldname);
	  return -1;
     }
     llibration(moon->now.n_mjd, &moon->llat, &moon->llon);
     moon->llibration_valid = 1;
     return 0;
}

static int moon_moon_colong(MoonObject *moon, char *fieldname)
{
     if (moon->moon_colong_valid) return 0;
     if (moon->now_valid == 0) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined until first compute()", fieldname);
	  return -1;
     }
     moon_colong(moon->now.n_mjd, 0, 0, &moon->c, &moon->k, 0, &moon->s);
     moon->moon_colong_valid = 1;
     return 0;
}

static int saturn_satrings(SaturnObject *saturn, char *fieldname)
{
     double lsn, rsn, bsn;
     if (saturn->satrings_valid) return 0;
     if (saturn->now_valid == 0) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined until first compute()", fieldname);
	  return -1;
     }
     if (body_obj_cir((BodyObject*) saturn, fieldname, 1) == -1) return -1;
     sunpos (saturn->now.n_mjd, &lsn, &rsn, &bsn);
     satrings(saturn->obj.s_hlat, saturn->obj.s_hlong, saturn->obj.s_sdist,
	      lsn + PI, rsn, saturn->now.n_mjd,
	      &saturn->etilt, &saturn->stilt);
     saturn->satrings_valid = 1;
     return 0;
}

/* The Body and Body-subtype fields themselves. */

#define GET_FIELD(name, field, builder) \
  static PyObject *Get_##name (PyObject *self, void *v) \
  { \
    BODY *body = (BODY*) self; \
    if (CALCULATOR(body, #name CARGS) == -1) return 0; \
    return builder(body->field); \
  }

/* Attributes computed by obj_cir needing only a date and epoch. */

#define BODY BodyObject
#define CALCULATOR body_obj_cir
#define CARGS ,1

GET_FIELD(ra, obj.s_ra, build_hours)
GET_FIELD(dec, obj.s_dec, build_degrees)
GET_FIELD(elong, obj.s_elong, build_degrees)
GET_FIELD(mag, obj.s_mag, build_mag)
GET_FIELD(size, obj.s_size, PyFloat_FromDouble)

GET_FIELD(hlong, obj.s_hlong, build_degrees)
GET_FIELD(hlat, obj.s_hlat, build_degrees)
GET_FIELD(sdist, obj.s_sdist, PyFloat_FromDouble)
GET_FIELD(edist, obj.s_edist, PyFloat_FromDouble)
GET_FIELD(phase, obj.s_phase, PyFloat_FromDouble)

/* Attributes computed by obj_cir that require an Observer. */

#undef CARGS
#define CARGS ,2

GET_FIELD(gaera, obj.s_gaera, build_hours)
GET_FIELD(gaedec, obj.s_gaedec, build_degrees)
GET_FIELD(az, obj.s_az, build_degrees)
GET_FIELD(alt, obj.s_alt, build_degrees)

#undef CALCULATOR
#undef CARGS

/* Attributes computed by riset_cir, which always require an Observer,
   and which might be None. */

#define CALCULATOR body_riset_cir
#define CARGS
#define FLAGGED(mask, builder) \
 (body->riset.rs_flags & mask) ? (Py_INCREF(Py_None), Py_None) : builder

GET_FIELD(risetm, riset.rs_risetm, FLAGGED(RS_NORISE, build_date))
GET_FIELD(riseaz, riset.rs_riseaz, FLAGGED(RS_NORISE, build_degrees))
GET_FIELD(trantm, riset.rs_trantm, FLAGGED(RS_NOTRANS, build_date))
GET_FIELD(tranalt, riset.rs_tranalt, FLAGGED(RS_NOTRANS, build_degrees))
GET_FIELD(settm, riset.rs_settm, FLAGGED(RS_NOSET, build_date))
GET_FIELD(setaz, riset.rs_setaz, FLAGGED(RS_NOSET, build_degrees))

#undef CALCULATOR
#undef BODY

#define BODY MoonObject
#define CALCULATOR moon_llibration

GET_FIELD(llat, llat, build_degrees)
GET_FIELD(llon, llon, build_degrees)

#undef CALCULATOR

#define CALCULATOR moon_moon_colong

GET_FIELD(c, c, build_degrees)
GET_FIELD(k, k, build_degrees)
GET_FIELD(s, s, build_degrees)

#undef CALCULATOR
#undef BODY

#define BODY SaturnObject
#define CALCULATOR saturn_satrings

GET_FIELD(etilt, etilt, build_degrees)
GET_FIELD(stilt, stilt, build_degrees)

#undef CALCULATOR
#undef BODY

/* Get/Set arrays. */

static PyMemberDef body_members[] = {
     {"name", T_STRING_INPLACE, OFF(o_name), RO, "body name"},
     {NULL, 0, 0, 0, NULL}
};

static PyGetSetDef body_getset[] = {
     {"ra", Get_ra, 0, "right ascension (hours of arc)"},
     {"dec", Get_dec, 0, "declination (degrees)"},
     {"elong", Get_elong, 0, "elongation"},
     {"mag", Get_mag, 0, "magnitude"},
     {"size", Get_size, 0, "visual size in arcseconds"},
     {"apparent_ra", Get_gaera, 0, "apparent right ascension (hours of arc)"},
     {"apparent_dec", Get_gaedec, 0, "apparent declination (degrees)"},
     {"az", Get_az, 0, "azimuth"},
     {"alt", Get_alt, 0, "altitude"},

     {"rise_time", Get_risetm, 0, "rise time"},
     {"rise_az", Get_riseaz, 0, "rise azimuth"},
     {"transit_time", Get_trantm, 0, "transit time"},
     {"transit_alt", Get_tranalt, 0, "transit altitude"},
     {"set_time", Get_settm, 0, "set time"},
     {"set_az", Get_setaz, 0, "set azimuth"},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyGetSetDef body_ss_getset[] = {
     {"hlong", Get_hlong, 0, "heliocentric longitude"},
     {"hlat", Get_hlat, 0, "heliocentric latitude"},
     {"sun_distance", Get_sdist, 0, "distance from sun (AU)"},
     {"earth_distance", Get_edist, 0, "distance from earth (AU)"},
     {"phase", Get_phase, 0, "phase (percent illuminated)"},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyGetSetDef moon_getset[] = {
     {"libration_lat", Get_llat, 0, "lunar libration (degrees latitude)"},
     {"libration_long", Get_llon, 0, "lunar libration (degrees longitude)"},
     {"colongitude", Get_c, 0,
      "lunar selenographic colongitude (-lng of rising sun) (degrees)"},
     {"moon_phase", Get_k, 0,
      "illuminated fraction of lunar surface visible from earth"},
     {"subsolar_latitude", Get_s, 0, "lunar latitude of subsolar point"},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyGetSetDef saturn_getset[] = {
     {"earth_tilt", Get_etilt, 0,
      "tilt of rings towards earth (degrees south)"},
     {"sun_tilt", Get_stilt, 0, "tilt of rings towards sun (degrees south)"},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyGetSetDef body_f_getset[] = {
     {"_spect",  get_f_spect, set_f_spect, "spectral codes", 0},
     {"_ratio", get_f_ratio, set_f_ratio,
      "ratio of minor to major diameter", VOFF(f_ratio)},
     {"_pa", get_f_pa, set_f_pa, "position angle E of N", VOFF(f_pa)},
     {"_epoch", getf_mjd, setf_mjd, "Date", VOFF(f_epoch)},
     {"_ra", getf_rh, setf_rh, "Fixed right ascension", VOFF(f_RA)},
     {"_dec", getf_rd, setf_rd, "Fixed declination", VOFF(f_dec)},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyMemberDef body_f_members[] = {
     {"_class", T_CHAR, OFF(f_class), RO, "fixed object classification"},
     {NULL, 0, 0, 0, NULL}
};

static PyGetSetDef body_e_getset[] = {
     {"_inc", getf_dd, setf_dd, "Inclination (degrees)", VOFF(e_inc)},
     {"_Om", getf_dd, setf_dd,
      "longitude of ascending node (degrees)", VOFF(e_Om)},
     {"_om", getf_dd, setf_dd, "argument of perihelion (degrees)", VOFF(e_om)},
     {"_M", getf_dd, setf_dd, "mean anomaly (degrees)", VOFF(e_M)},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyMemberDef body_e_members[] = {
     {"_a", T_FLOAT, OFF(e_a), 0, "mean distance (AU)"},
     {"_size", T_FLOAT, OFF(e_size), 0, "angular size at 1 AU (arcseconds)"},
     {"_e", T_DOUBLE, OFF(e_e), 0, "eccentricity"},
     {"_cepoch", T_DOUBLE, OFF(e_cepoch), 0, "epoch date of mean anomaly"},
     {"_epoch", T_DOUBLE, OFF(e_epoch), 0, "equinox year"},
     /* Mag */
     {NULL, 0, 0, 0, NULL}
};

static PyGetSetDef body_h_getset[] = {
     {"_inc", getf_dd, setf_dd, "Inclination (degrees)", VOFF(h_inc)},
     {"_Om", getf_dd, setf_dd,
      "Longitude of ascending node (degrees)", VOFF(h_Om)},
     {"_om", getf_dd, setf_dd,
      "Argument of perihelion (degrees)", VOFF(h_om)},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyMemberDef body_h_members[] = {
     {"_epoch", T_DOUBLE, OFF(h_epoch), 0,
      "Equinox year of _inc, _Om, and _om (mjd)"},
     {"_ep", T_DOUBLE, OFF(h_ep), 0, "Epoch of perihelion (mjd)"},
     {"_e", T_FLOAT, OFF(h_e), 0, "Eccentricity"},
     {"_qp", T_FLOAT, OFF(h_qp), 0, "Perihelion distance (AU)"},
     {"_g", T_FLOAT, OFF(h_g), 0, "Magnitude coefficient g"},
     {"_k", T_FLOAT, OFF(h_k), 0, "Magnitude coefficient g"},
     {"_size", T_FLOAT, OFF(h_size), 0, "Angular size at 1 AU (arcseconds)"},
     {NULL, 0, 0, 0, NULL}
};

static PyGetSetDef body_p_getset[] = {
     {"_inc", getf_dd, setf_dd, "Inclination (degrees)", VOFF(p_inc)},
     {"_om", getf_dd, setf_dd, "Argument of perihelion (degrees)", VOFF(p_om)},
     {"_Om", getf_dd, setf_dd,
      "Longitude of ascending node (degrees)", VOFF(p_Om)},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyMemberDef body_p_members[] = {
     {"_epoch", T_DOUBLE, OFF(p_epoch), 0, ""},
     {"_ep", T_DOUBLE, OFF(p_ep), 0, ""},
     {"_qp", T_FLOAT, OFF(p_qp), 0, ""},
     {"_g", T_FLOAT, OFF(p_g), 0, ""},
     {"_k", T_FLOAT, OFF(p_k), 0, ""},
     {"_size", T_FLOAT, OFF(p_size), 0, "angular size at 1 AU (arcseconds)"},
     {NULL, 0, 0, 0, NULL}
};

static PyGetSetDef body_es_getset[] = {
     {"_inc", getf_dd, setf_dd, "Inclination (degrees)", VOFF(es_inc)},
     {"_raan", getf_dd, setf_dd,
      "Right ascension of ascending node (degrees)", VOFF(es_raan)},
     {"_ap", getf_dd, setf_dd,
      "Argument of perigee at epoch (degrees)", VOFF(es_ap)},
     {"_M", getf_dd, setf_dd,
      "Mean anomaly (degrees from perigee at epoch)", VOFF(es_M)},
     {"sublat", getf_rd, setf_rd,
      "Latitude below satellite (degrees east)", VOFF(s_sublat)},
     {"sublong", getf_rd, setf_rd,
      "Longitude below satellite (degrees north)", VOFF(s_sublng)},
     {NULL, NULL, NULL, NULL, NULL}
};

static PyMemberDef body_es_members[] = {
     {"_epoch", T_DOUBLE, OFF(es_epoch), 0, "reference epoch (mjd)"},
     {"_n", T_DOUBLE, OFF(es_n), 0, "mean motion (revolutions per day)"},
     {"_e", T_FLOAT, OFF(es_e), 0, "eccentricity"},
     {"_decay", T_FLOAT, OFF(es_decay), 0,
      "orbit decay rate (revolutions per day-squared)"},
     {"_drag", T_FLOAT, OFF(es_drag), 0,
      "object drag coefficient (per earth radius)"},
     {"_orbit", T_INT, OFF(es_orbit), 0, "integer orbit number of epoch"},

     /* results: */

     {"elevation", T_FLOAT, OFF(s_elev), RO, "height above sea level (m)"},
     {"range", T_FLOAT, OFF(s_range), RO,
      "distance from observer to satellite (m)"},
     {"range_velocity", T_FLOAT, OFF(s_rangev), RO,
      "range rate of change (m/s)"},
     {"eclipsed", T_INT, OFF(s_eclipsed), RO,
      "whether satellite is in earth's shadow"},
     {NULL, 0, 0, 0, NULL}
};

#undef OFF

/*
 * The Object corresponds to the `any' object fields of X,
 * and implements most of the computational methods.  While Fixed
 * inherits directly from an Object, all other object types
 * inherit from PlanetObject to make the additional position
 * fields available for inspection.
 *
 * Note that each subclass is exactly the size of its parent, which is
 * rather unusual but reflects the fact that all of these are just
 * various wrappers around the same X Obj structure.
 */

static PyTypeObject Body_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "Body",
     sizeof(BodyObject),
     0,
     0, /*_PyObject_Del,*/	/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     /*(reprfunc)*/ body_str, /* tp_str */
     0, /*PyObject_GenericGetAttr,*/ /* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT /*| Py_TPFLAGS_HAVE_GC*/
     | Py_TPFLAGS_BASETYPE,	/* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     body_methods,		/* tp_methods */
     body_members,		/* tp_members */
     body_getset,		/* tp_getset */
     0,				/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     body_init,			/* tp_init */
     0, /*PyType_GenericAlloc,*/ /* tp_alloc */
     0, /*PyType_GenericNew,*/	/* tp_new */
     0, /*_PyObject_GC_Del,*/	/* tp_free */
};

static PyTypeObject FixedBody_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "FixedBody",
     sizeof(BodyObject),
     0,
     0,				/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     0,				/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     body_f_members,		/* tp_members */
     body_f_getset,		/* tp_getset */
     &Body_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject Planet_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "Planet",
     sizeof(BodyObject),
     0,
     0,				/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     0,				/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     0,				/* tp_members */
     body_ss_getset,		/* tp_getset */
     &Body_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject Saturn_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "Saturn",
     sizeof(SaturnObject),
     0,
     0,				/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     0,				/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     0,				/* tp_members */
     saturn_getset,		/* tp_getset */
     &Planet_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     saturn_init,		/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject Moon_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "Moon",
     sizeof(MoonObject),
     0,
     0,				/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     0,				/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     0,				/* tp_members */
     moon_getset,		/* tp_getset */
     &Planet_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     moon_init,			/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject EllipticalBody_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "EllipticalBody",
     sizeof(BodyObject),
     0,
     0,				/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     0,				/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     body_e_members,	/* tp_members */
     body_e_getset,	/* tp_getset */
     &Planet_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject HyperbolicBody_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "HyperbolicBody",
     sizeof(BodyObject),
     0,
     0,				/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     0,				/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     body_h_members,		/* tp_members */
     body_h_getset,		/* tp_getset */
     &Planet_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject ParabolicBody_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ParabolicBody",
     sizeof(BodyObject),
     0,
     0,				/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     0,				/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     body_p_members,		/* tp_members */
     body_p_getset,		/* tp_getset */
     &Planet_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject EarthSatellite_Type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "EarthSatellite",
     sizeof(BodyObject),
     0,
     0,				/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     0,				/* tp_str */
     0,				/* tp_getattro */
     0,				/* tp_setattro */
     0,				/* tp_as_buffer */
     Py_TPFLAGS_DEFAULT,	/* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     body_es_members,		/* tp_members */
     body_es_getset,		/* tp_getset */
     &Planet_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

/* Return the current time. */

static PyObject* build_now(PyObject *self, PyObject *args)
{
     if (!PyArg_ParseTuple(args, "")) return 0;
     return PyFloat_FromDouble(mjd_now());
}

/* Compute the separation between two objects. */

static int separation_arg(PyObject *arg, double *lngi, double *lati)
{
     char err_message[] = "each separation argument "
	  "must be an Observer, an Body, "
	  "or a pair of numeric coordinates";
     if (PyObject_IsInstance(arg, (PyObject*) &Observer_Type)) {
	  ObserverObject *o = (ObserverObject*) arg;
	  *lngi = o->now.n_lng;
	  *lati = o->now.n_lat;
	  return 0;
     } else if (PyObject_IsInstance(arg, (PyObject*) &Body_Type)) {
	  BodyObject *b = (BodyObject*) arg;
	  if (body_obj_cir(b, "ra", 1)) return -1;
	  *lngi = b->obj.s_ra;
	  *lati = b->obj.s_dec;
	  return 0;
     } else if (PySequence_Check(arg) && PySequence_Size(arg) == 2) {
	  PyObject *lngo, *lato, *lngf, *latf;
	  lngo = PySequence_GetItem(arg, 0);
	  if (!lngo) return -1;
	  lato = PySequence_GetItem(arg, 1);
	  if (!lato) return -1;
	  if (!PyNumber_Check(lngo) || !PyNumber_Check(lato)) goto fail;
	  lngf = PyNumber_Float(lngo);
	  if (!lngf) return -1;
	  latf = PyNumber_Float(lato);
	  if (!latf) return -1;
	  *lngi = PyFloat_AsDouble(lngf);
	  *lati = PyFloat_AsDouble(latf);
	  Py_DECREF(lngf);
	  Py_DECREF(latf);
	  return 0;
     }
fail:
     PyErr_SetString(PyExc_TypeError, err_message);
     return -1;
}

static PyObject* separation(PyObject *self, PyObject *args)
{
     double plat, plng, qlat, qlng;
     double spy, cpy, px, qx, sqy, cqy;
     PyObject *p, *q;
     if (!PyArg_ParseTuple(args, "OO", &p, &q)) return 0;
     if (separation_arg(p, &plng, &plat)) return 0;
     if (separation_arg(q, &qlng, &qlat)) return 0;

     spy = sin (plat);
     cpy = cos (plat);
     px = plng;
     qx = qlng;
     sqy = sin (qlat);
     cqy = cos (qlat);

     return new_Angle(acos(spy*sqy + cpy*cqy*cos(px-qx)), raddeg(1));
}

/* Read an  database entry from a string. */

static PyObject *build_body_from_obj(Obj *op)
{
     PyTypeObject *type;
     BodyObject *body;
     switch (op->o_type) {
     case FIXED:
	  type = &FixedBody_Type;
	  break;
     case ELLIPTICAL:
	  type = &EllipticalBody_Type;
	  break;
     case HYPERBOLIC:
	  type = &HyperbolicBody_Type;
	  break;
     case PARABOLIC:
	  type = &ParabolicBody_Type;
	  break;
     case EARTHSAT:
	  type = &EarthSatellite_Type;
	  break;
     default:
	  PyErr_Format(PyExc_ValueError,
		       "attempting to build object of unknown type %d",
		       op->o_type);
	  return 0;
     }
     //body = PyObject_NEW(BodyObject, type);
     body = (BodyObject*) PyType_GenericNew(type, 0, 0);
     if (body) body->obj = *op;
     return (PyObject*) body;
}

static PyObject* readdb(PyObject *self, PyObject *args)
{
     char *s, errmsg[256];
     Obj obj;
     if (!PyArg_ParseTuple(args, "s:readdb", &s)) return 0;
     if (db_crack_line(s, &obj, 0, 0, errmsg) == -1) {
	  PyErr_SetString(PyExc_ValueError,
			  errmsg[0] ? errmsg :
			  "line does not conform to ephem database format");
	  return 0;
     }
     return build_body_from_obj(&obj);
}

static PyObject* readtle(PyObject *self, PyObject *args)
{
     char *name, *l1, *l2;
     Obj obj;
     if (!PyArg_ParseTuple(args, "sss:readelt", &name, &l1, &l2)) return 0;
     if (db_tle(name, l1, l2, &obj)) {
	  PyErr_SetString(PyExc_ValueError,
			  "line does not conform to tle format");
	  return 0;
     }
     return build_body_from_obj(&obj);
}

/* Create various sorts of angles. */

static PyObject *degrees(PyObject *self, PyObject *args)
{
     PyObject *o;
     double value;
     if (!PyArg_ParseTuple(args, "O", &o)) return 0;
     if (parse_angle(o, raddeg(1), &value) == -1) return 0;
     return new_Angle(value, raddeg(1));
}

static PyObject *hours(PyObject *self, PyObject *args)
{
     PyObject *o;
     double value;
     if (!PyArg_ParseTuple(args, "O", &o)) return 0;
     if (parse_angle(o, radhr(1), &value) == -1) return 0;
     return new_Angle(value, radhr(1));
}

/* Return in which constellation a particular coordinate lies. */

int cns_pick(double r, double d, double e);
char *cns_name(int id);

static PyObject* constellation
(PyObject *self, PyObject *args, PyObject *kwdict)
{
     static char *kwlist[] = {"position", "epoch", NULL};
     PyObject *position_arg = 0, *epoch_arg = 0;
     PyObject *s0 = 0, *s1 = 0, *ora = 0, *odec = 0, *oepoch = 0;
     PyObject *result;
     double ra, dec, epoch = J2000;
     
     if (!PyArg_ParseTupleAndKeywords(args, kwdict, "O|O", kwlist,
				      &position_arg, &epoch_arg))
	  return 0;
     
     if (PyObject_IsInstance(position_arg, (PyObject*) &Body_Type)) {
	  BodyObject *b = (BodyObject*) position_arg;
	  if (epoch_arg) {
	       PyErr_SetString(PyExc_TypeError, "you cannot specify an epoch= "
			       "when providing a body for the position, since "
			       "bodies themselves specify the epoch of their "
			       "coordinates");
	       goto fail;
	  }
	  if (b->obj_valid == 0 && body_obj_cir(b, 0, 0) == -1) {
	       PyErr_SetString(PyExc_TypeError, "you cannot ask about "
			       "the constellation in which a body "
			       "lies until you have used compute() to "
			       "determine its position");
	       goto fail;
	  }
	  ra = b->obj.s_ra;
	  dec = b->obj.s_dec;
	  epoch = b->now.n_epoch;
     } else {
	  if (!PySequence_Check(position_arg)) {
	       PyErr_SetString(PyExc_TypeError, "you must specify a position "
			       "by providing either a body or a sequence of "
			       "two numeric coordinates");
	       goto fail;
	  }
	  if (!PySequence_Length(position_arg) == 2) {
	       PyErr_SetString(PyExc_ValueError, "the sequence specifying a "
			       "position must have exactly two coordinates");
	       goto fail;
	  }
	  if (epoch_arg)
	       if (parse_mjd(epoch_arg, &epoch) == -1) goto fail;
	  
	  s0 = PySequence_GetItem(position_arg, 0);
	  if (!s0) goto fail;
	  s1 = PySequence_GetItem(position_arg, 1);
	  if (!s1 || !PyNumber_Check(s0) || !PyNumber_Check(s1)) goto fail;
	  ora = PyNumber_Float(s0);
	  if (!ora) goto fail;
	  odec = PyNumber_Float(s1);
	  if (!odec) goto fail;
	  ra = PyFloat_AsDouble(ora);
	  dec = PyFloat_AsDouble(odec);
	  
	  if (epoch_arg) {
	       oepoch = PyNumber_Float(epoch_arg);
	       if (!oepoch) goto fail;
	       epoch = PyFloat_AsDouble(oepoch);
	  }
     }
     
     {
	  char *s = cns_name(cns_pick(ra, dec, epoch));
	  result = Py_BuildValue("s#s", s, 3, s+5);
	  goto leave;
     }
     
fail:
     result = 0;
leave:
     Py_XDECREF(s0);
     Py_XDECREF(s1);
     Py_XDECREF(ora);
     Py_XDECREF(odec);
     Py_XDECREF(oepoch);
     return result;
}


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

     /* Initialize non-constant pointers. */

     Angle_Type.ob_type = &PyType_Type;
     Angle_Type.tp_base = &PyFloat_Type;

     Body_Type.ob_type = &PyType_Type;
     Body_Type.tp_dealloc = (destructor) _PyObject_Del;
     Body_Type.tp_getattro = PyObject_GenericGetAttr;
     Body_Type.tp_alloc = PyType_GenericAlloc;
     Body_Type.tp_new = PyType_GenericNew;
     Body_Type.tp_free = _PyObject_GC_Del;

     FixedBody_Type.ob_type = &PyType_Type;

     Planet_Type.ob_type = &PyType_Type;

     Saturn_Type.ob_type = &PyType_Type;

     Moon_Type.ob_type = &PyType_Type;

     EllipticalBody_Type.ob_type = &PyType_Type;

     HyperbolicBody_Type.ob_type = &PyType_Type;

     ParabolicBody_Type.ob_type = &PyType_Type;

     EarthSatellite_Type.ob_type = &PyType_Type;

     Date_Type.ob_type = &PyType_Type;
     Date_Type.tp_base = &PyFloat_Type;

     Observer_Type.ob_type = &PyType_Type;
     Observer_Type.tp_dealloc = (destructor) _PyObject_Del;
     Observer_Type.tp_getattro = PyObject_GenericGetAttr;

     /* Build type dictionaries. */

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

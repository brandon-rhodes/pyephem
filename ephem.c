/* Python header files. */

#include <Python.h>
#include <structmember.h>

#include "astro.h"
#include "preferences.h"

/* Undo the astro.h #defines of common body attribute names. */

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

/* Since different Body attributes are calculated with different
   routines, the compute() function itself just writes the time into
   the Body's `now' cache, and the VALID bits - stored in the user
   flags field of each `obj' - are used to coordinate lazy computation
   of the fields the user actually tries to access. */

#define VALID_GEO   FUSER0	/* Now has mjd and epoch */
#define VALID_TOPO  FUSER1	/* Now has entire Observer */
#define VALID_OBJ   FUSER2	/* object fields have been computed */
#define VALID_RISET FUSER3	/* riset fields have been computed */

#define VALID_LIBRATION FUSER4	/* moon libration fields computed */
#define VALID_COLONG  FUSER5	/* moon co-longitude fields computed */

#define VALID_RINGS   FUSER4	/* saturn ring fields computed */

typedef struct {
     PyObject_HEAD
     Now now;
} Observer;

typedef struct {
     PyObject_HEAD
     Now now;			/* cache of last argument to compute() */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     PyObject *name;		/* object name */
} Body;

typedef Body Planet, PlanetMoon;
typedef Body FixedBody, BinaryStar;
typedef Body EllipticalBody, ParabolicBody;

typedef struct {
     PyObject_HEAD
     Now now;			/* cache of last observer */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     PyObject *name;		/* object name */
     double llat, llon;		/* libration */
     double c, k, s;		/* co-longitude and illumination */
} Moon;

typedef struct {
     PyObject_HEAD
     Now now;			/* cache of last observer */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     PyObject *name;		/* object name */
     double etilt, stilt;	/* tilt of rings */
} Saturn;

typedef struct {
     PyObject_HEAD
     Now now;			/* cache of last observer */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     PyObject *name;		/* object name */
     PyObject *catalog_number;	/* TLE catalog number */
} EarthSatellite;

/* */

static double mjd_now(void)
{
     return 25567.5 + time(NULL)/3600.0/24.0;
}

/* Return the floating-point equivalent value of a Python number. */

static int PyNumber_AsDouble(PyObject *o, double *dp)
{
     PyObject *f = PyNumber_Float(o);
     if (!f) return -1;
     *dp = PyFloat_AsDouble(f);
     Py_DECREF(f);
     return 0;
}

/* Angle: Python float which prints itself as a sexagesimal value,
   like 'hours:minutes:seconds' or 'degrees:minutes:seconds',
   depending on the factor given it. */

staticforward PyTypeObject AngleType;

typedef struct {
     PyFloatObject f;
     double factor;
} AngleObject;

static PyObject* new_Angle(double radians, double factor)
{
     AngleObject *ea;
     ea = PyObject_NEW(AngleObject, &AngleType);
     if (ea) {
	  ea->f.ob_fval = radians;
	  ea->factor = factor;
     }
     return (PyObject*) ea;
}

static char *Angle_format(PyObject *self)
{
     AngleObject *ea = (AngleObject*) self;
     static char buffer[13];
     fs_sexa(buffer, ea->f.ob_fval * ea->factor, 3, 360000);
     return buffer[0] != ' ' ? buffer
	  : buffer[1] != ' ' ? buffer + 1
	  : buffer + 2;
}

static PyObject* Angle_str(PyObject *self)
{
     return PyString_FromString(Angle_format(self));
}

static int Angle_print(PyObject *self, FILE *fp, int flags)
{
     fputs(Angle_format(self), fp);
     return 0;
}

static PyObject *Angle_get_norm(PyObject *self, void *v)
{
     AngleObject *ea = (AngleObject*) self;
     double radians = ea->f.ob_fval;
     if (0 >= radians && radians < 2*PI)
	  return self;
     return new_Angle(fmod(radians, 2*PI), ea->factor);
}

static PyGetSetDef Angle_getset[] = {
     {"norm", Angle_get_norm, NULL, "Date", 0},
     {NULL}
};

static PyTypeObject AngleType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.angle",
     sizeof(AngleObject),
     0,
     0,				/* tp_dealloc */
     Angle_print,		/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     Angle_str,			/* tp_str */
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
     Angle_getset,		/* tp_getset */
     0, /* &PyFloatType*/	/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

/* Date: a Python Float that can print itself as a date, and that
   supports triple() and tuple() methods for extracting its date and
   time components. */

staticforward PyTypeObject DateType;

typedef PyFloatObject DateObject;

static int parse_mjd_from_number(PyObject *o, double *mjdp)
{
     return PyNumber_AsDouble(o, mjdp);
}

static int parse_mjd_from_string(PyObject *so, double *mjdp)
{
     /* Run the Python code: s = so.strip() */
     PyObject *emptytuple = PyTuple_New(0);
     PyObject *split_func = PyObject_GetAttrString(so, "split");
     PyObject *pieces = PyObject_Call(split_func, emptytuple, 0);
     int len = PyObject_Length(pieces);
     char *s = (len >= 1) ? PyString_AsString(PyList_GetItem(pieces, 0)) : 0;
     char *t = (len >= 2) ? PyString_AsString(PyList_GetItem(pieces, 1)) : 0;
     int year, month = 1;
     double day = 1.0;

     Py_DECREF(emptytuple);
     Py_DECREF(split_func);
     Py_DECREF(pieces);

     if ((len < 1) || (len > 2))
	  goto fail;

     if (s) {
	  /* Make sure all characters are in set '-/.0123456789' */

	  int i;
	  for (i=0; s[i]; i++)
	       if (s[i] != '-' && s[i] != '/' && s[i] != '.'
		   && (s[i] < '0' || s[i] > '9'))
		    goto fail;

	  f_sscandate(s, PREF_YMD, &month, &day, &year);
     }

     if (t) {
	  double hours;
	  if (f_scansexa(t, &hours) == -1)
	       goto fail;
	  day += hours / 24.;
     }

     cal_mjd(month, day, year, mjdp);
     return 0;

fail:
     {
	  PyObject *repr = PyObject_Repr(so);
          PyObject *complaint = PyString_FromFormat(
	       "your date string %s does not look like a year/month/day"
	       " optionally followed by hours:minutes:seconds",
	       PyString_AsString(repr));
	  PyErr_SetObject(PyExc_ValueError, complaint);
	  Py_DECREF(repr);
	  Py_DECREF(complaint);
     }
     return -1;
}

static int parse_mjd_from_tuple(PyObject *value, double *mjdp)
{
     double day = 1.0, hours = 0.0, minutes = 0.0, seconds = 0.0;
     int year, month = 1;
     if (!PyArg_ParseTuple(value, "i|idddd:date.tuple", &year, &month, &day,
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

static PyObject* build_Date(double mjd)
{
     DateObject *new = PyObject_New(PyFloatObject, &DateType);
     if (new) new->ob_fval = mjd;
     return (PyObject*) new;
}

static PyObject *Date_new(PyObject *self, PyObject *args, PyObject *kw)
{
     PyObject *arg;
     double mjd;
     if (kw) {
	  PyErr_SetString(PyExc_TypeError,
			  "this function does not accept keyword arguments");
	  return 0;
     }
     if (!PyArg_ParseTuple(args, "O:date", &arg)) return 0;
     if (parse_mjd(arg, &mjd)) return 0;
     return build_Date(mjd);
}

static char *Date_format(PyObject *self)
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

static PyObject* Date_str(PyObject *self)
{
     return PyString_FromString(Date_format(self));
}

static int Date_print(PyObject *self, FILE *fp, int flags)
{
     char *s = Date_format(self);
     fputs(s, fp);
     return 0;
}

static PyObject *Date_triple(PyObject *self, PyObject *args)
{
     int year, month;
     double day;
     DateObject *d = (DateObject*) self;
     if (!PyArg_ParseTuple(args, ":date.triple")) return 0;
     mjd_cal(d->ob_fval, &month, &day, &year);
     return Py_BuildValue("iid", year, month, day);
}

static PyObject *Date_tuple(PyObject *self, PyObject *args)
{
     int year, month;
     double fday, fhour, fminute, fsecond;
     int day, hour, minute;
     DateObject *d = (DateObject*) self;
     if (!PyArg_ParseTuple(args, ":date.tuple")) return 0;
     mjd_cal(d->ob_fval, &month, &fday, &year);
     day = (int) fday;
     fhour = fmod(fday, 1.0) * 24;
     hour = (int) fhour;
     fminute = fmod(fhour, 1.0) * 60;
     minute = (int) fminute;
     fsecond = fmod(fminute, 1.0) * 60;
     return Py_BuildValue("iiiiid", year, month, day, hour, minute, fsecond);
}

static PyMethodDef Date_methods[] = {
     {"triple", Date_triple, METH_VARARGS,
      "Return the date as a (year, month, day_with_fraction) tuple"},
     {"tuple", Date_tuple, METH_VARARGS,
      "Return the date as a (year, month, day, hour, minute, second) tuple"},
     {NULL}
};

static PyTypeObject DateType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.date",
     sizeof(PyFloatObject),
     0,
     0,				/* tp_dealloc */
     Date_print,		/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     0,				/* tp_repr */
     0,				/* tp_as_number */
     0,				/* tp_as_sequence */
     0,				/* tp_as_mapping */
     0,				/* tp_hash */
     0,				/* tp_call */
     Date_str,			/* tp_str */
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
     Date_methods,		/* tp_methods */
     0,				/* tp_members */
     0,				/* tp_getset */
     0, /*&PyFloatType,*/	/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     (newfunc) Date_new,	/* tp_new */
     0,				/* tp_free */
};

/*
 * Both pairs of routines below have to access a structure member to
 * which they have been given the offset.
 */

#define THE_FLOAT (* (float*) ((char*)self + (size_t)v))
#define THE_DOUBLE (* (double*) ((char*)self + (size_t)v))

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
     return build_Date(THE_FLOAT);
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
     return build_Date(THE_DOUBLE);
}

static int setd_mjd(PyObject *self, PyObject *value, void *v)
{
     double result;
     if (parse_mjd(value, &result)) return -1;
     THE_DOUBLE = result;
     return 0;
}

/* #undef THE_FLOAT
   #undef THE_DOUBLE */

/*
 * The following values are ones for which XEphem provides special
 * routines for their reading and writing, usually because they are
 * encoded into one or two bytes to save space in the obj structure
 * itself.  These are simply wrappers around those functions.
 */

/* Spectral codes. */

static PyObject* get_f_spect(PyObject *self, void *v)
{
     Body *b = (Body*) self;
     return PyString_FromStringAndSize(b->obj.f_spect, 2);
}

static int set_f_spect(PyObject *self, PyObject *value, void *v)
{
     Body *b = (Body*) self;
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
     Body *b = (Body*) self;
     return PyFloat_FromDouble(get_ratio(&b->obj));
}

static int set_f_ratio(PyObject *self, PyObject *value, void *v)
{
     Body *b = (Body*) self;
     double maj, min;
     if (!PyArg_ParseTuple(value, "dd", &maj, &min)) return -1;
     set_ratio(&b->obj, maj, min);
     return 0;
}

/* Position angle of fixed object. */

static PyObject* get_f_pa(PyObject *self, void *v)
{
     Body *b = (Body*) self;
     return PyFloat_FromDouble(get_pa(&b->obj));
}

static int set_f_pa(PyObject *self, PyObject *value, void *v)
{
     Body *b = (Body*) self;
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

/*
 * Constructor and methods.
 */

static int Observer_init(PyObject *self, PyObject *args, PyObject *kwds)
{
     Observer *o = (Observer*) self;
     static char *kwlist[] = {0};
     if (!PyArg_ParseTupleAndKeywords(args, kwds, ":Observer", kwlist))
	  return -1;
     o->now.n_mjd = mjd_now();
     o->now.n_lat = o->now.n_lng = o->now.n_tz = o->now.n_elev
	  = o->now.n_dip = 0;
     o->now.n_temp = 15.0;
     o->now.n_pressure = 1010;
     o->now.n_epoch = J2000;
     return 0;
}

static PyObject *Observer_sidereal_time(PyObject *self)
{
     Observer *o = (Observer*) self;
     double lst;
     now_lst(&o->now, &lst);
     return new_Angle(hrrad(lst), radhr(1));
}

static PyObject *Observer_radec_of(PyObject *self, PyObject *args,
				   PyObject *kwds)
{
     Observer *o = (Observer*) self;
     static char *kwlist[] = {"az", "alt", 0};
     PyObject *azo, *alto, *rao, *deco;
     double az, alt, lst, ha, ra, dec;

     if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO:Observer.radec_of",
				      kwlist, &azo, &alto))
	  return 0;

     if (parse_angle(azo, raddeg(1), &az) == -1)
	  return 0;
     if (parse_angle(alto, raddeg(1), &alt) == -1)
	  return 0;

     now_lst(&o->now, &lst);
     lst = hrrad(lst);
     unrefract(o->now.n_pressure, o->now.n_temp, alt, &alt);
     aa_hadec(o->now.n_lat, alt, az, &ha, &dec);
     ra = fmod(lst - ha, 2*PI);

     pref_set(PREF_EQUATORIAL, PREF_TOPO); /* affects call to ap_as? */
     if (o->now.n_epoch != EOD)
	  ap_as(&o->now, o->now.n_epoch, &ra, &dec);

     rao = new_Angle(ra, radhr(1));
     if (!rao) return 0;
     deco = new_Angle(dec, raddeg(1));
     if (!deco) return 0;
     return Py_BuildValue("OO", rao, deco);
}

/*
 * Member access.
 */

static PyObject *get_elev(PyObject *self, void *v)
{
     Observer *o = (Observer*) self;
     return PyFloat_FromDouble(o->now.n_elev * ERAD);
}

static int set_elev(PyObject *self, PyObject *value, void *v)
{
     int r;
     double n;
     Observer *o = (Observer*) self;
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
 *
 * Note that we commandeer the n_dip field for our own purposes.
 * XEphem uses it to compute the beginning and end of twilight; since
 * we have no twilight functions, we use it to store the displacement
 * for rising and setting calculations.
 */

#define VOFF(member) ((void*) OFF(member))
#define OFF(member) offsetof(Observer, now.member)

static PyMethodDef Observer_methods[] = {
     {"sidereal_time", (PyCFunction) Observer_sidereal_time, METH_NOARGS,
      "compute the local sidereal time for this location and time"},
     {"radec_of", (PyCFunction) Observer_radec_of,
      METH_VARARGS | METH_KEYWORDS,
      "compute the right ascension and declination of a point"
      " identified by its azimuth and altitude"},
     {NULL}
};

static PyGetSetDef Observer_getset[] = {
     {"date", getd_mjd, setd_mjd, "Date", VOFF(n_mjd)},
     {"lat", getd_rd, setd_rd, "Latitude (degrees north)", VOFF(n_lat)},
     {"long", getd_rd, setd_rd, "Longitude (degrees east)", VOFF(n_lng)},
     {"elev", get_elev, set_elev, "Elevation above sea level (meters)", NULL},
     {"horizon", getd_rd, setd_rd,
      "The angle above (+) or below (-) the horizon at which an object"
      " should be considered at the moment of rising or setting (degrees)",
      VOFF(n_dip)},
     {"epoch", getd_mjd, setd_mjd, "Precession epoch", VOFF(n_epoch)},
     {NULL}
};

static PyMemberDef Observer_members[] = {
     {"temp", T_DOUBLE, OFF(n_temp), 0, "atmospheric temperature (C)"},
     {"pressure", T_DOUBLE, OFF(n_pressure), 0,
      "atmospheric pressure (mBar)"},
     {NULL}
};

#undef OFF

static PyTypeObject ObserverType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.Observer",
     sizeof(Observer),
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
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     Observer_methods,		/* tp_methods */
     Observer_members,		/* tp_members */
     Observer_getset,		/* tp_getset */
     0,				/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     Observer_init,		/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0				/* tp_free */
};

/*
 *
 * BODIES
 *
 */

staticforward PyTypeObject BodyType;
staticforward PyTypeObject PlanetType;
staticforward PyTypeObject SaturnType;
staticforward PyTypeObject MoonType;
staticforward PyTypeObject PlanetMoonType;

static PyObject* Body_compute(PyObject *self, PyObject *args, PyObject *kwds);

/* Body and Planet may be initialized privately by their subclasses
   using these setup functions. */

static void Body_setup(Body *body)
{
     body->obj.o_flags = 0;
}

static int Planet_setup(Body *planet, PyObject *args, PyObject *kw,
			char *planet_name, int planet_code, int moon_code)
{
     Body_setup(planet);

     planet->name = 0;
     planet->obj.o_type = PLANET;
     strcpy(planet->obj.o_name, planet_name);
     planet->obj.pl_code = planet_code;
     planet->obj.pl_moon = moon_code;

     if (PyTuple_Check(args) && PyTuple_Size(args)) {
	  PyObject *result = Body_compute((PyObject*) planet, args, kw);
	  if (!result) return -1;
	  Py_DECREF(result);
     }
     return 0;
}

/* Body, Planet, and PlanetMoon are abstract classes that resist
   instantiation. */

static int Body_init(PyObject *self, PyObject *args, PyObject *kw)
{
     PyErr_SetString(PyExc_TypeError, "you cannot create a generic Body");
     return -1;
}

static int Planet_init(PyObject *self, PyObject *args, PyObject *kw)
{
     PyErr_SetString(PyExc_TypeError, "you cannot create a generic Planet");
     return -1;
}

static int PlanetMoon_init(PyObject *self, PyObject *args, PyObject *kw)
{
     PyErr_SetString(PyExc_TypeError, 
		     "you cannot create a generic PlanetMoon");
     return -1;
}

/* But Saturn and Moon, as concrete classes, do allow initialization. */

static int Saturn_init(PyObject *self, PyObject *args, PyObject *kw)
{
     return Planet_setup((Body*) self, args, kw, "Saturn", SATURN, X_PLANET);
}

static int Moon_init(PyObject *self, PyObject *args, PyObject *kw)
{
     return Planet_setup((Body*) self, args, kw, "Moon", MOON, X_PLANET);
}

/* Bodies need to unlink any name object when finished. */

static void Body_dealloc(PyObject *self)
{
     Body *body = (Body*) self;
     Py_XDECREF(body->name);
     self->ob_type->tp_free(self);
}

/* These functions create and return planets. */

#define CREATE(TYPE, NAME, CODE, MOON) \
PyObject *create_##NAME(PyObject* self, PyObject *args, PyObject *kw) \
{ \
     Body *body = (Body*) _PyObject_New(&TYPE); \
     if (!body) return 0; \
     if (Planet_setup((Body*) body, args, kw, #NAME, CODE, MOON)) return 0; \
     return (PyObject*) body; \
}

/* Planets */

CREATE(PlanetType, Mercury, MERCURY, X_PLANET)
CREATE(PlanetType, Venus, VENUS, X_PLANET)
CREATE(PlanetType, Mars, MARS, X_PLANET)
CREATE(PlanetType, Jupiter, JUPITER, X_PLANET)
/* Saturn has its own class */
CREATE(PlanetType, Uranus, URANUS, X_PLANET)
CREATE(PlanetType, Neptune, NEPTUNE, X_PLANET)
CREATE(PlanetType, Pluto, PLUTO, X_PLANET)
CREATE(PlanetType, Sun, SUN, X_PLANET)
/* the Moon has its own class */

/* Planetary Moons */

CREATE(PlanetMoonType, Phobos, MARS, M_PHOBOS)
CREATE(PlanetMoonType, Deimos, MARS, M_DEIMOS)
CREATE(PlanetMoonType, Io, JUPITER, J_IO)
CREATE(PlanetMoonType, Europa, JUPITER, J_EUROPA)
CREATE(PlanetMoonType, Ganymede, JUPITER, J_GANYMEDE)
CREATE(PlanetMoonType, Callisto, JUPITER, J_CALLISTO)
CREATE(PlanetMoonType, Mimas, SATURN, S_MIMAS)
CREATE(PlanetMoonType, Enceladus, SATURN, S_ENCELADUS)
CREATE(PlanetMoonType, Tethys, SATURN, S_TETHYS)
CREATE(PlanetMoonType, Dione, SATURN, S_DIONE)
CREATE(PlanetMoonType, Rhea, SATURN, S_RHEA)
CREATE(PlanetMoonType, Titan, SATURN, S_TITAN)
CREATE(PlanetMoonType, Hyperion, SATURN, S_HYPERION)
CREATE(PlanetMoonType, Iapetus, SATURN, S_IAPETUS)
CREATE(PlanetMoonType, Ariel, URANUS, U_ARIEL)
CREATE(PlanetMoonType, Umbriel, URANUS, U_UMBRIEL)
CREATE(PlanetMoonType, Titania, URANUS, U_TITANIA)
CREATE(PlanetMoonType, Oberon, URANUS, U_OBERON)
CREATE(PlanetMoonType, Miranda, URANUS, U_MIRANDA)

#undef CREATE

/* The user-configurable body types also share symmetric
   initialization code. */

#define INIT(NAME, CODE) \
static int NAME##_init(PyObject* self, PyObject* args, PyObject *kw) \
{ \
     Body *body = (Body*) self; \
     Body_setup(body); \
     body->name = Py_None; \
     Py_INCREF(Py_None); \
     body->obj.o_name[0] = '\0'; \
     body->obj.o_type = CODE; \
     return 0; \
}

static int FixedBody_init(PyObject* self, PyObject* args, PyObject *kw)
{
     Body *body = (Body*) self;
     Body_setup(body);
     body->name = Py_None;
     Py_INCREF(Py_None);
     body->obj.o_name[0] = '\0';
     body->obj.o_type = FIXED;
     body->obj.f_epoch = J2000;
     return 0;
}

INIT(BinaryStar, BINARYSTAR)
INIT(EllipticalBody, ELLIPTICAL)
INIT(HyperbolicBody, HYPERBOLIC)
INIT(ParabolicBody, PARABOLIC)

static int EarthSatellite_init(PyObject* self, PyObject* args, PyObject *kw)
{
     EarthSatellite *body = (EarthSatellite*) self;
     Body_setup((Body*) body);
     body->name = Py_None;
     Py_INCREF(Py_None);
     body->catalog_number = Py_None;
     Py_INCREF(Py_None);
     body->obj.o_name[0] = '\0';
     body->obj.o_type = EARTHSAT;
     return 0;
}

#undef INIT

/* This compute() method does not actually compute anything, since
   several different computations would be necessary to determine the
   value of all of a body's fields, and the user may not want them
   all; so compute() just stashes away the Observer or date it is
   given, and the fields are computed when the user actually accesses
   them. */

static PyObject* Body_compute(PyObject *self, PyObject *args, PyObject *kwds)
{
     Body *body = (Body*) self;
     static char *kwlist[] = {"when", "epoch", 0};
     PyObject *when_arg = 0, *epoch_arg = 0;

     if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO:Body.compute", kwlist,
				      &when_arg, &epoch_arg))
	  return 0;

     if (when_arg && PyObject_TypeCheck(when_arg, &ObserverType)) {

	  /* compute(observer) */

	  Observer *observer = (Observer*) when_arg;
	  if (epoch_arg) {
	       PyErr_SetString(PyExc_ValueError,
			       "cannot supply an epoch= keyword argument "
			       "because an Observer specifies its own epoch");
	       return 0;
	  }
	  body->now = observer->now;
	  body->obj.o_flags = VALID_GEO | VALID_TOPO;
     } else {
	  double when_mjd = 0, epoch_mjd;

	  /* compute(date, epoch) where date defaults to current time
	     and epoch defaults to 2000.0 */

	  if (when_arg) {
	       if (parse_mjd(when_arg, &when_mjd) == -1) goto fail;
	  } else
	       when_mjd = mjd_now();

	  if (epoch_arg) {
	       if (parse_mjd(epoch_arg, &epoch_mjd) == -1) goto fail;
	  } else
	       epoch_mjd = J2000;
     
	  /* Since libastro always does topocentric computation, we
	     need to provide a reasonable location and weather. */
	  body->now.n_mjd = when_mjd;
	  body->now.n_lat = body->now.n_lng = body->now.n_tz
	       = body->now.n_elev = body->now.n_dip = 0;
	  body->now.n_temp = 15.0;
	  body->now.n_pressure = 0; /* no refraction */
	  body->now.n_epoch = epoch_mjd;

	  body->obj.o_flags = VALID_GEO;
     }

     Py_INCREF(Py_None);
     return Py_None;

fail:
     return 0;
}

static PyObject* Body_writedb(PyObject *self)
{
     Body *body = (Body*) self;
     char line[1024];
     db_write_line(&body->obj, line);
     if (body->name) {
	  char *name = PyString_AsString(body->name);
	  return PyString_FromFormat("%s%s", name, line);
     } else
	  return PyString_FromString(line);
}

static PyObject* Body_copy(PyObject *self)
{
     PyObject *newbody = _PyObject_New(self->ob_type);
     if (!newbody) return 0;
     memcpy(newbody, self, self->ob_type->tp_basicsize);
     Py_XINCREF(((Body*) self)->name);
     return newbody;
}

static PyObject* Body_repr(PyObject *body_object)
{
     Body *body = (Body*) body_object;
     if (body->name) {
	  char *name;
	  PyObject *repr, *result;
	  repr = PyObject_Repr(body->name);
	  if (!repr) return 0;
	  name = PyString_AsString(repr);
	  Py_DECREF(repr);
	  if (!name) return 0;
	  result = PyString_FromFormat("<%s %s at %p>",
				       body->ob_type->tp_name, name, body);
	  return result;
     } else if (body->obj.o_name[0])
	  return PyString_FromFormat("<%s \"%s\" at %p>",
				     body->ob_type->tp_name,
				     body->obj.o_name, body);
     else
	  return PyString_FromFormat("<%s at %p>",
				     body->ob_type->tp_name, body);
}

static PyMethodDef Body_methods[] = {
     {"compute", (PyCFunction) Body_compute, METH_VARARGS | METH_KEYWORDS,
      "compute the location of the body for the given date or Observer "
      "(or for the current time if no date is supplied)"},
     {"writedb", (PyCFunction) Body_writedb, METH_NOARGS,
      "return a string representation of the body "
      "appropriate for inclusion in an ephem database file"},
     {"copy", (PyCFunction) Body_copy, METH_NOARGS,
      "Return a new copy of this body"},
     {"__copy__", (PyCFunction) Body_copy, METH_VARARGS,
      "Return a new copy of this body"},
     {NULL}
};

static PyObject* build_hours(double radians)
{
     return new_Angle(radians, radhr(1));
}

static PyObject* build_degrees(double radians)
{
     return new_Angle(radians, raddeg(1));
}

static PyObject* build_degrees_from_degrees(double degrees)
{
     return build_degrees(degrees / raddeg(1));
}

static PyObject* build_mag(double raw)
{
     return PyFloat_FromDouble(raw / MAGSCALE);
}

/* These are the functions which are each responsible for filling in
   some of the fields of an ephem.Body or one of its subtypes.  When
   called they determine whether the information in the fields for
   which they are responsible is up-to-date, and re-compute them if
   not.  By checking body->valid they can deteremine how much
   information the last compute() call supplied. */

static int Body_obj_cir(Body *body, char *fieldname, unsigned topocentric)
{
     if (body->obj.o_flags == 0) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined until first compute()",
		       fieldname);
	  return -1;
     }
     if (topocentric && (body->obj.o_flags & VALID_TOPO) == 0) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined because the most recent compute() "
		       "was supplied a date rather than an Observer",
		       fieldname);
	  return -1;
     }
     if (body->obj.o_flags & VALID_OBJ)
	  return 0;
     pref_set(PREF_EQUATORIAL, body->obj.o_flags & VALID_TOPO ?
	      PREF_TOPO : PREF_GEO);
     if (body->obj.o_type == FIXED) {
          float ra = body->obj.f.fo_ra;
          float dec = body->obj.f.fo_dec;
          float epoch = body->obj.f.fo_epoch;
          obj_cir(& body->now, & body->obj);
          body->obj.f.fo_ra = ra;
          body->obj.f.fo_dec = dec;
          body->obj.f.fo_epoch = epoch;
     } else
          obj_cir(& body->now, & body->obj);
     body->obj.o_flags |= VALID_OBJ;
     return 0;
}

static int Body_riset_cir(Body *body, char *fieldname)
{
     if ((body->obj.o_flags & VALID_RISET) == 0) {
	  if (body->obj.o_flags == 0) {
	       PyErr_Format(PyExc_RuntimeError, "field %s undefined"
			    " until first compute()", fieldname);
	       return -1;
	  }
	  if ((body->obj.o_flags & VALID_TOPO) == 0) {
	       PyErr_Format(PyExc_RuntimeError, "field %s undefined because"
			    " last compute() supplied a date"
			    " rather than an Observer", fieldname);
	       return -1;
	  }
	  riset_cir(& body->now, & body->obj,
		    - body->now.n_dip, & body->riset);
	  body->obj.o_flags |= VALID_RISET;
     }
     if (body->riset.rs_flags & RS_ERROR) {
	  PyErr_Format(PyExc_RuntimeError, "error computing rise, transit,"
		       " and set circumstances");
	  return -1;
     }
     return 0;
}

static int Moon_llibration(Moon *moon, char *fieldname)
{
     if (moon->obj.o_flags & VALID_LIBRATION)
	  return 0;
     if (moon->obj.o_flags == 0) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined until first compute()", fieldname);
	  return -1;
     }
     llibration(MJD0 + moon->now.n_mjd, &moon->llat, &moon->llon);
     moon->obj.o_flags |= VALID_LIBRATION;
     return 0;
}

static int Moon_colong(Moon *moon, char *fieldname)
{
     if (moon->obj.o_flags & VALID_COLONG)
	  return 0;
     if (moon->obj.o_flags == 0) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined until first compute()", fieldname);
	  return -1;
     }
     moon_colong(MJD0 + moon->now.n_mjd, 0, 0,
		 &moon->c, &moon->k, 0, &moon->s);
     moon->obj.o_flags |= VALID_COLONG;
     return 0;
}

static int Saturn_satrings(Saturn *saturn, char *fieldname)
{
     double lsn, rsn, bsn;
     if (saturn->obj.o_flags & VALID_RINGS)
	  return 0;
     if (saturn->obj.o_flags == 0) {
	  PyErr_Format(PyExc_RuntimeError,
		       "field %s undefined until first compute()", fieldname);
	  return -1;
     }
     if (Body_obj_cir((Body*) saturn, fieldname, 0) == -1) return -1;
     sunpos(saturn->now.n_mjd, &lsn, &rsn, &bsn);
     satrings(saturn->obj.s_hlat, saturn->obj.s_hlong, saturn->obj.s_sdist,
	      lsn + PI, rsn, saturn->now.n_mjd,
	      &saturn->etilt, &saturn->stilt);
     saturn->obj.o_flags |= VALID_RINGS;
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

/* Attributes computed by obj_cir that need only a date and epoch. */

#define BODY Body
#define CALCULATOR Body_obj_cir
#define CARGS ,0

GET_FIELD(ra, obj.s_ra, build_hours)
GET_FIELD(dec, obj.s_dec, build_degrees)
GET_FIELD(elong, obj.s_elong, build_degrees_from_degrees)
GET_FIELD(mag, obj.s_mag, build_mag)
GET_FIELD(size, obj.s_size, PyFloat_FromDouble)

GET_FIELD(hlong, obj.s_hlong, build_degrees)
GET_FIELD(hlat, obj.s_hlat, build_degrees)
GET_FIELD(sun_distance, obj.s_sdist, PyFloat_FromDouble)
GET_FIELD(earth_distance, obj.s_edist, PyFloat_FromDouble)
GET_FIELD(phase, obj.s_phase, PyFloat_FromDouble)

GET_FIELD(x, obj.pl_x, PyFloat_FromDouble)
GET_FIELD(y, obj.pl_y, PyFloat_FromDouble)
GET_FIELD(z, obj.pl_z, PyFloat_FromDouble)
GET_FIELD(earth_visible, obj.pl_evis, PyFloat_FromDouble)
GET_FIELD(sun_visible, obj.pl_svis, PyFloat_FromDouble)

GET_FIELD(sublat, obj.s_sublat, build_degrees)
GET_FIELD(sublong, obj.s_sublng, build_degrees)
GET_FIELD(elevation, obj.s_elev, PyFloat_FromDouble)
GET_FIELD(eclipsed, obj.s_eclipsed, PyBool_FromLong)

/* Attributes computed by obj_cir that need an Observer. */

#undef CARGS
#define CARGS ,1

GET_FIELD(az, obj.s_az, build_degrees)
GET_FIELD(alt, obj.s_alt, build_degrees)

GET_FIELD(range, obj.s_range, PyFloat_FromDouble)
GET_FIELD(range_velocity, obj.s_rangev, PyFloat_FromDouble)

#undef CALCULATOR
#undef CARGS

/* Attributes computed by riset_cir, which always require an Observer,
   and which might be None. */

#define CALCULATOR Body_riset_cir
#define CARGS
#define FLAGGED(mask, builder) \
 (body->riset.rs_flags & (mask | RS_NEVERUP)) \
 ? (Py_INCREF(Py_None), Py_None) : builder

GET_FIELD(rise_time, riset.rs_risetm,
	  FLAGGED(RS_CIRCUMPOLAR | RS_NORISE, build_Date))
GET_FIELD(rise_az, riset.rs_riseaz,
	  FLAGGED(RS_CIRCUMPOLAR | RS_NORISE, build_degrees))
GET_FIELD(transit_time, riset.rs_trantm, FLAGGED(RS_NOTRANS, build_Date))
GET_FIELD(transit_alt, riset.rs_tranalt, FLAGGED(RS_NOTRANS, build_degrees))
GET_FIELD(set_time, riset.rs_settm,
	  FLAGGED(RS_CIRCUMPOLAR | RS_NOSET, build_Date))
GET_FIELD(set_az, riset.rs_setaz,
	  FLAGGED(RS_CIRCUMPOLAR | RS_NOSET, build_degrees))

#undef CALCULATOR
#undef BODY

#define BODY Moon
#define CALCULATOR Moon_llibration

GET_FIELD(libration_lat, llat, build_degrees)
GET_FIELD(libration_long, llon, build_degrees)

#undef CALCULATOR

#define CALCULATOR Moon_colong

GET_FIELD(colong, c, build_degrees)
GET_FIELD(moon_phase, k, PyFloat_FromDouble)
GET_FIELD(subsolar_lat, s, build_degrees)

#undef CALCULATOR
#undef BODY

#define BODY Saturn
#define CALCULATOR Saturn_satrings

GET_FIELD(earth_tilt, etilt, build_degrees)
GET_FIELD(sun_tilt, stilt, build_degrees)

#undef CALCULATOR
#undef BODY

/* Attribute access that needs to be hand-written. */

static PyObject *Get_name(PyObject *self, void *v)
{
     Body *body = (Body*) self;
     if (body->name) {
	  Py_INCREF(body->name);
	  return body->name;
     } else
	  return PyString_FromString(body->obj.o_name);
}

static int Set_name(PyObject *self, PyObject *value, void *v)
{
     Body *body = (Body*) self;
     Py_XDECREF(body->name);
     body->name = value;
     Py_INCREF(value);
     return 0;
}

static int Set_mag(PyObject *self, PyObject *value, void *v)
{
     Body *b = (Body*) self;
     double mag;
     if (PyNumber_AsDouble(value, &mag) == -1) return -1;
     set_fmag(&b->obj, mag);
     return 0;
}

static PyObject *Get_circumpolar(PyObject *self, void *v)
{
     Body *body = (Body*) self;
     if (Body_riset_cir(body, "circumpolar") == -1) return 0;
     return PyBool_FromLong(body->riset.rs_flags & RS_CIRCUMPOLAR);
}

static PyObject *Get_neverup(PyObject *self, void *v)
{
     Body *body = (Body*) self;
     if (Body_riset_cir(body, "neverup") == -1) return 0;
     return PyBool_FromLong(body->riset.rs_flags & RS_NEVERUP);
}

/* Allow getting and setting of EllipticalBody magnitude model
   coefficients, which can be either H/G or gk but which use the same
   storage either way. */

static PyObject *Get_HG(PyObject *self, void *v)
{
     Body *body = (Body*) self;
     if (body->obj.e_mag.whichm != MAG_HG) {
	  PyErr_Format(PyExc_RuntimeError,
		       "this object has g/k magnitude coefficients");
	  return 0;
     }
     return PyFloat_FromDouble(THE_FLOAT);
}

static PyObject *Get_gk(PyObject *self, void *v)
{
     Body *body = (Body*) self;
     if (body->obj.e_mag.whichm != MAG_gk) {
	  PyErr_Format(PyExc_RuntimeError,
		       "this object has H/G magnitude coefficients");
	  return 0;
     }
     return PyFloat_FromDouble(THE_FLOAT);
}

static int Set_HG(PyObject *self, PyObject *value, void *v)
{
     Body *body = (Body*) self;
     double n;
     if (PyNumber_AsDouble(value, &n) == -1) return -1;
     THE_FLOAT = n;
     body->obj.e_mag.whichm = MAG_HG;
     return 0;
}

static int Set_gk(PyObject *self, PyObject *value, void *v)
{
     Body *body = (Body*) self;
     double n;
     if (PyNumber_AsDouble(value, &n) == -1) return -1;
     THE_FLOAT = n;
     body->obj.e_mag.whichm = MAG_gk;
     return 0;
}

/* Get/Set arrays. */

#define OFF(member) offsetof(Body, obj.member)

static PyGetSetDef Body_getset[] = {
     {"name", Get_name, 0, "object name (read-only string)"},

     {"ra", Get_ra, 0, "right ascension (radians that print as hours of arc)"},
     {"dec", Get_dec, 0, "declination (radians that print as degrees)"},
     {"elong", Get_elong, 0, "elongation (radians that print as degrees)"},
     {"mag", Get_mag, 0, "magnitude"},
     {"size", Get_size, 0, "visual size (seconds of arc)"},

     {"az", Get_az, 0, "azimuth (radians that print as degrees)"},
     {"alt", Get_alt, 0, "altitude (radians that print as degrees)"},

     {"rise_time", Get_rise_time, 0, "rise time"},
     {"rise_az", Get_rise_az, 0, "rise azimuth"
      " (radians that print as degrees)"},
     {"transit_time", Get_transit_time, 0, "transit time"},
     {"transit_alt", Get_transit_alt, 0, "transit altitude"
      " (radians that print as degrees)"},
     {"set_time", Get_set_time, 0, "set time"},
     {"set_az", Get_set_az, 0, "set azimuth"
      " (radians that print as degrees)"},
     {"circumpolar", Get_circumpolar, 0,
      "whether object remains above the horizon this day"},
     {"neverup", Get_neverup, 0,
      "whether object never rises above the horizon this day"},
     {NULL}
};

static PyGetSetDef Planet_getset[] = {
     {"hlong", Get_hlong, 0, "heliocentric longitude"
      " (radians that print as degrees)"},
     {"hlat", Get_hlat, 0, "heliocentric latitude"
      " (radians that print as degrees)"},
     {"sun_distance", Get_sun_distance, 0, "distance from sun (AU)"},
     {"earth_distance", Get_earth_distance, 0, "distance from earth (AU)"},
     {"phase", Get_phase, 0, "phase (percent illuminated)"},
     {NULL}
};

/* Some day, when planetary moons support all Body and Planet fields,
   this will become a list only of fields peculiar to planetary moons,
   and PlanetMoon will inherit from Planet; but for now, there is no
   inheritance, and this must list every fields valid for a planetary
   moon (see libastro's plmoon.c for details). */

static PyGetSetDef PlanetMoon_getset[] = {
     {"name", Get_name, 0, "object name (read-only string)"},

     {"ra", Get_ra, 0, "right ascension (radians that print as hours of arc)"},
     {"dec", Get_dec, 0, "declination (radians that print as degrees)"},

     {"az", Get_az, 0, "azimuth (radians that print as degrees)"},
     {"alt", Get_alt, 0, "altitude (radians that print as degrees)"},

     {"x", Get_x, 0, "how far east or west of its planet"
      " the moon lies in the sky (in planet radii, east is positive)"},
     {"y", Get_y, 0, "how far north or south of its planet"
      " the moon lies in the sky (in planet radii, south is positive)"},
     {"z", Get_z, 0, "how much closer or farther from Earth the moon is"
      " than its planet"
      " (in planet radii, closer to Earth is positive)"},

     {"earth_visible", Get_earth_visible, 0, "whether visible from earth"},
     {"sun_visible", Get_sun_visible, 0, "whether visible from sun"},
     {NULL}
};

static PyGetSetDef Moon_getset[] = {
     {"libration_lat", Get_libration_lat, 0,
      "lunar libration in latitude (radians that print as degrees)"},
     {"libration_long", Get_libration_long, 0,
      "lunar libration in longitude (radians that print as degrees)"},
     {"colong", Get_colong, 0,
      "lunar selenographic colongitude (radians that print as degrees)"},
     {"moon_phase", Get_moon_phase, 0,
      "fraction of lunar surface illuminated when viewed from earth"},
     {"subsolar_lat", Get_subsolar_lat, 0,
      "lunar latitude of subsolar point (radians that print as degrees)"},
     {NULL}
};

static PyGetSetDef Saturn_getset[] = {
     {"earth_tilt", Get_earth_tilt, 0,
      "tilt of rings towards Earth (radians that print as degrees,"
      " positive for southward tilt and negative for northward)"},
     {"sun_tilt", Get_sun_tilt, 0,
      "tilt of rings towards Sun radians that print as degrees,"
      " positive for southward tilt and negative for northward)"},
     {NULL}
};

static PyGetSetDef FixedBody_getset[] = {
     {"name", Get_name, Set_name, "object name"},
     {"mag", Get_mag, Set_mag, "magnitude", 0},
     {"_spect",  get_f_spect, set_f_spect, "spectral codes", 0},
     {"_ratio", get_f_ratio, set_f_ratio,
      "ratio of minor to major diameter", VOFF(f_ratio)},
     {"_pa", get_f_pa, set_f_pa, "position angle E of N", VOFF(f_pa)},
     {"_epoch", getf_mjd, setf_mjd, "epoch for _ra and _dec", VOFF(f_epoch)},
     {"_ra", getf_rh, setf_rh, "fixed right ascension", VOFF(f_RA)},
     {"_dec", getf_rd, setf_rd, "fixed declination", VOFF(f_dec)},
     {NULL}
};

static PyMemberDef FixedBody_members[] = {
     {"_class", T_CHAR, OFF(f_class), RO, "object classification"},
     {NULL}
};

static PyGetSetDef EllipticalBody_getset[] = {
     {"name", Get_name, Set_name, "object name"},
     {"_inc", getf_dd, setf_dd, "inclination (degrees)", VOFF(e_inc)},
     {"_Om", getf_dd, setf_dd,
      "longitude of ascending node (degrees)", VOFF(e_Om)},
     {"_om", getf_dd, setf_dd, "argument of perihelion (degrees)", VOFF(e_om)},
     {"_M", getf_dd, setf_dd, "mean anomaly (degrees)", VOFF(e_M)},
     {"_epoch_M", getd_mjd, setd_mjd, "epoch of mean anomaly",
      VOFF(e_cepoch)},
     {"_epoch", getd_mjd, setd_mjd, "epoch for _inc, _Om, and _om",
      VOFF(e_epoch)},
     {"_H", Get_HG, Set_HG, "magnitude coefficient", VOFF(e_mag.m1)},
     {"_G", Get_HG, Set_HG, "magnitude coefficient", VOFF(e_mag.m2)},
     {"_g", Get_gk, Set_gk, "magnitude coefficient", VOFF(e_mag.m1)},
     {"_k", Get_gk, Set_gk, "magnitude coefficient", VOFF(e_mag.m2)},
     {NULL}
};

static PyMemberDef EllipticalBody_members[] = {
     {"_a", T_FLOAT, OFF(e_a), 0, "mean distance (AU)"},
     {"_size", T_FLOAT, OFF(e_size), 0, "angular size at 1 AU (arcseconds)"},
     {"_e", T_DOUBLE, OFF(e_e), 0, "eccentricity"},
     {NULL}
};

static PyGetSetDef HyperbolicBody_getset[] = {
     {"name", Get_name, Set_name, "object name"},
     {"_epoch", getd_mjd, setd_mjd, "epoch date of _inc, _Om, and _om",
      VOFF(h_epoch)},
     {"_epoch_p", getd_mjd, setd_mjd, "epoch of perihelion", VOFF(h_ep)},
     {"_inc", getf_dd, setf_dd, "inclination (degrees)", VOFF(h_inc)},
     {"_Om", getf_dd, setf_dd,
      "longitude of ascending node (degrees)", VOFF(h_Om)},
     {"_om", getf_dd, setf_dd,
      "argument of perihelion (degrees)", VOFF(h_om)},
     {NULL}
};

static PyMemberDef HyperbolicBody_members[] = {
     {"_e", T_FLOAT, OFF(h_e), 0, "eccentricity"},
     {"_q", T_FLOAT, OFF(h_qp), 0, "perihelion distance (AU)"},
     {"_g", T_FLOAT, OFF(h_g), 0, "magnitude coefficient"},
     {"_k", T_FLOAT, OFF(h_k), 0, "magnitude coefficient"},
     {"_size", T_FLOAT, OFF(h_size), 0, "angular size at 1 AU (arcseconds)"},
     {NULL}
};

static PyGetSetDef ParabolicBody_getset[] = {
     {"name", Get_name, Set_name, "object name"},
     {"_epoch", getd_mjd, setd_mjd, "reference epoch", VOFF(p_epoch)},
     {"_epoch_p", getd_mjd, setd_mjd, "epoch of perihelion", VOFF(p_ep)},
     {"_inc", getf_dd, setf_dd, "inclination (degrees)", VOFF(p_inc)},
     {"_om", getf_dd, setf_dd, "argument of perihelion (degrees)", VOFF(p_om)},
     {"_Om", getf_dd, setf_dd,
      "longitude of ascending node (degrees)", VOFF(p_Om)},
     {NULL}
};

static PyMemberDef ParabolicBody_members[] = {
     {"_q", T_FLOAT, OFF(p_qp), 0, "perihelion distance (AU)"},
     {"_g", T_FLOAT, OFF(p_g), 0, "magnitude coefficient"},
     {"_k", T_FLOAT, OFF(p_k), 0, "magnitude coefficient"},
     {"_size", T_FLOAT, OFF(p_size), 0, "angular size at 1 AU (arcseconds)"},
     {NULL}
};

static PyGetSetDef EarthSatellite_getset[] = {
     {"name", Get_name, Set_name, "object name"},
     {"_epoch", getd_mjd, setd_mjd, "reference epoch (mjd)", VOFF(es_epoch)},
     {"_inc", getf_dd, setf_dd, "inclination (degrees)", VOFF(es_inc)},
     {"_raan", getf_dd, setf_dd,
      "right ascension of ascending node (degrees)", VOFF(es_raan)},
     {"_ap", getf_dd, setf_dd,
      "argument of perigee at epoch (degrees)", VOFF(es_ap)},
     {"_M", getf_dd, setf_dd,
      "mean anomaly (degrees from perigee at epoch)", VOFF(es_M)},

     /* results: */

     {"sublat", Get_sublat, 0, "latitude below satellite (degrees north)"},
     {"sublong", Get_sublong, 0, "longitude below satellite (degrees east)"},
     {"elevation", Get_elevation, 0, "height above sea level (meters)"},
     {"range", Get_range, 0, "distance from observer to satellite (meters)"},
     {"range_velocity", Get_range_velocity, 0,
      "range rate of change (meters per second)"},
     {"eclipsed", Get_eclipsed, 0, "whether satellite is in earth's shadow"},
     {NULL}
};

static PyMemberDef EarthSatellite_members[] = {
     {"_n", T_DOUBLE, OFF(es_n), 0, "mean motion (revolutions per day)"},
     {"_e", T_FLOAT, OFF(es_e), 0, "eccentricity"},
     {"_decay", T_FLOAT, OFF(es_decay), 0,
      "orbit decay rate (revolutions per day-squared)"},
     {"_drag", T_FLOAT, OFF(es_drag), 0,
      "object drag coefficient (per earth radius)"},
     {"_orbit", T_INT, OFF(es_orbit), 0, "integer orbit number of epoch"},
     {"catalog_number", T_OBJECT, offsetof(EarthSatellite, catalog_number), 0,
      "catalog number from TLE file"},
     {NULL}
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

static PyTypeObject BodyType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.Body",
     sizeof(Body),
     0,
     Body_dealloc,		/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     Body_repr,			/* tp_repr */
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
     Body_methods,		/* tp_methods */
     0,				/* tp_members */
     Body_getset,		/* tp_getset */
     0,				/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     Body_init,			/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject PlanetType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.Planet",
     sizeof(Body),
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
     Planet_getset,		/* tp_getset */
     &BodyType,			/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     &Planet_init,		/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject PlanetMoonType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.PlanetMoon",
     sizeof(PlanetMoon),
     0,
     0,				/* tp_dealloc */
     0,				/* tp_print */
     0,				/* tp_getattr */
     0,				/* tp_setattr */
     0,				/* tp_compare */
     Body_repr,			/* tp_repr */
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
     Body_methods,		/* tp_methods */
     0,				/* tp_members */
     PlanetMoon_getset,		/* tp_getset */
     0,				/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     &PlanetMoon_init,		/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject SaturnType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.Saturn",
     sizeof(Saturn),
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
     Saturn_getset,		/* tp_getset */
     &PlanetType,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     Saturn_init,		/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject MoonType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.Moon",
     sizeof(Moon),
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
     Moon_getset,		/* tp_getset */
     &PlanetType,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     Moon_init,			/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject FixedBodyType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.FixedBody",
     sizeof(Body),
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
     FixedBody_members,		/* tp_members */
     FixedBody_getset,		/* tp_getset */
     &BodyType,			/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     FixedBody_init,		/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject BinaryStarType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.BinaryStar",
     sizeof(Body),
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
     0,				/* tp_getset */
     &PlanetType,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     BinaryStar_init,		/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject EllipticalBodyType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.EllipticalBody",
     sizeof(Body),
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
     EllipticalBody_members,	/* tp_members */
     EllipticalBody_getset,	/* tp_getset */
     &PlanetType,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     EllipticalBody_init,	/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject HyperbolicBodyType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.HyperbolicBody",
     sizeof(Body),
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
     HyperbolicBody_members,	/* tp_members */
     HyperbolicBody_getset,	/* tp_getset */
     &PlanetType,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     HyperbolicBody_init,	/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject ParabolicBodyType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.ParabolicBody",
     sizeof(Body),
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
     ParabolicBody_members,	/* tp_members */
     ParabolicBody_getset,	/* tp_getset */
     &PlanetType,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     ParabolicBody_init,	/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

static PyTypeObject EarthSatelliteType = {
     PyObject_HEAD_INIT(NULL)
     0,
     "ephem.EarthSatellite",
     sizeof(EarthSatellite),
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
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
     0,				/* tp_doc */
     0,				/* tp_traverse */
     0,				/* tp_clear */
     0,				/* tp_richcompare */
     0,				/* tp_weaklistoffset */
     0,				/* tp_iter */
     0,				/* tp_iternext */
     0,				/* tp_methods */
     EarthSatellite_members,	/* tp_members */
     EarthSatellite_getset,	/* tp_getset */
     &BodyType,			/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     EarthSatellite_init,	/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

/* Return the current time. */

static PyObject* build_now(PyObject *self, PyObject *args)
{
     if (!PyArg_ParseTuple(args, ":now")) return 0;
     return build_Date(mjd_now());
}

/* Compute the separation between two objects. */

static int separation_arg(PyObject *arg, double *lngi, double *lati)
{
     char err_message[] = "each separation argument "
	  "must be an Observer, an Body, "
	  "or a pair of numeric coordinates";
     if (PyObject_IsInstance(arg, (PyObject*) &ObserverType)) {
	  Observer *o = (Observer*) arg;
	  *lngi = o->now.n_lng;
	  *lati = o->now.n_lat;
	  return 0;
     } else if (PyObject_IsInstance(arg, (PyObject*) &BodyType)) {
	  Body *b = (Body*) arg;
	  if (Body_obj_cir(b, "ra", 0)) return -1;
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
     if (!PyArg_ParseTuple(args, "OO:separation", &p, &q)) return 0;
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

/* Read various database formats, with strings as input. */

static PyObject *build_body_from_obj(PyObject *name, Obj *op)
{
     PyTypeObject *type;
     Body *body;
     switch (op->o_type) {
     case FIXED:
	  type = &FixedBodyType;
	  break;
     case ELLIPTICAL:
	  type = &EllipticalBodyType;
	  break;
     case HYPERBOLIC:
	  type = &HyperbolicBodyType;
	  break;
     case PARABOLIC:
	  type = &ParabolicBodyType;
	  break;
     case EARTHSAT:
	  type = &EarthSatelliteType;
	  break;
     default:
	  PyErr_Format(PyExc_ValueError,
		       "cannot build object of unexpected type %d",
		       op->o_type);
	  Py_DECREF(name);
	  return 0;
     }
     //body = PyObject_NEW(Body, type);
     body = (Body*) PyType_GenericNew(type, 0, 0);
     if (!body) {
	  Py_DECREF(name);
	  return 0;
     }
     body->obj = *op;
     body->name = name;
     body->obj.o_name[0] = '\0';
     return (PyObject*) body;
}

static PyObject* readdb(PyObject *self, PyObject *args)
{
     char *line, *comma, errmsg[256];
     PyObject *name;
     Obj obj;
     if (!PyArg_ParseTuple(args, "s:readdb", &line)) return 0;
     if (db_crack_line(line, &obj, 0, 0, errmsg) == -1) {
	  PyErr_SetString(PyExc_ValueError,
			  errmsg[0] ? errmsg :
			  "line does not conform to ephem database format");
	  return 0;
     }
     comma = strchr(line, ',');
     if (comma)
	  name = PyString_FromStringAndSize(line, comma - line);
     else
	  name = PyString_FromString(line);
     if (!name)
	  return 0;
     return build_body_from_obj(name, &obj);
}

static PyObject* readtle(PyObject *self, PyObject *args)
{
     char *l1, *l2;
     PyObject *name, *stripped_name, *body, *catalog_number;
     Obj obj;
     if (!PyArg_ParseTuple(args, "O!ss:readtle",
			   &PyString_Type, &name, &l1, &l2))
	  return 0;
     if (db_tle(PyString_AsString(name), l1, l2, &obj)) {
	  PyErr_SetString(PyExc_ValueError,
			  "line does not conform to tle format");
	  return 0;
     }
     stripped_name = PyObject_CallMethod(name, "strip", 0);
     if (!stripped_name)
	  return 0;
     body = build_body_from_obj(stripped_name, &obj);
     if (!body)
	  return 0;
     catalog_number = PyInt_FromLong(strtod(l1+2, 0));
     if (!catalog_number)
	  return 0;
     ((EarthSatellite*) body)->catalog_number = catalog_number;
     return body;
}

/* Create various sorts of angles. */

static PyObject *degrees(PyObject *self, PyObject *args)
{
     PyObject *o;
     double value;
     if (!PyArg_ParseTuple(args, "O:degrees", &o)) return 0;
     if (parse_angle(o, raddeg(1), &value) == -1) return 0;
     return new_Angle(value, raddeg(1));
}

static PyObject *hours(PyObject *self, PyObject *args)
{
     PyObject *o;
     double value;
     if (!PyArg_ParseTuple(args, "O:hours", &o)) return 0;
     if (parse_angle(o, radhr(1), &value) == -1) return 0;
     return new_Angle(value, radhr(1));
}

/* Finally, we wrap some helpful libastro functions. */

/* Return which page of a star atlas on which a location lies. */

static PyObject* uranometria(PyObject *self, PyObject *args)
{
     double ra, dec;
     if (!PyArg_ParseTuple(args, "dd:uranometria", &ra, &dec)) return 0;
     return PyString_FromString(um_atlas(ra, dec));
}

static PyObject* uranometria2000(PyObject *self, PyObject *args)
{
     double ra, dec;
     if (!PyArg_ParseTuple(args, "dd:uranometria2000", &ra, &dec)) return 0;
     return PyString_FromString(u2k_atlas(ra, dec));
}

static PyObject* millennium_atlas(PyObject *self, PyObject *args)
{
     double ra, dec;
     if (!PyArg_ParseTuple(args, "dd:millennium_atlas", &ra, &dec)) return 0;
     return PyString_FromString(msa_atlas(ra, dec));
}

/* Return in which constellation a particular coordinate lies. */

int cns_pick(double r, double d, double e);
char *cns_name(int id);

static PyObject* constellation
(PyObject *self, PyObject *args, PyObject *kwds)
{
     static char *kwlist[] = {"position", "epoch", NULL};
     PyObject *position_arg = 0, *epoch_arg = 0;
     PyObject *s0 = 0, *s1 = 0, *ora = 0, *odec = 0, *oepoch = 0;
     PyObject *result;
     double ra, dec, epoch = J2000;
     
     if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O:constellation", kwlist,
				      &position_arg, &epoch_arg))
	  return 0;
     
     if (PyObject_IsInstance(position_arg, (PyObject*) &BodyType)) {
	  Body *b = (Body*) position_arg;
	  if (epoch_arg) {
	       PyErr_SetString(PyExc_TypeError, "you cannot specify an epoch= "
			       "when providing a body for the position, since "
			       "bodies themselves specify the epoch of their "
			       "coordinates");
	       goto fail;
	  }
	  if (b->obj.o_flags == 0) {
	       PyErr_SetString(PyExc_TypeError, "you cannot ask about "
			       "the constellation in which a body "
			       "lies until you have used compute() to "
			       "determine its position");
	       goto fail;
	  }
	  if (Body_obj_cir(b, "ra", 0) == -1)
	       goto fail;
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

static PyObject *julian_date(PyObject *self, PyObject *args)
{
     PyObject *o = 0;
     double mjd;
     if (!PyArg_ParseTuple(args, "|O:julian_date", &o)) return 0;
     if (!o)
	  mjd = mjd_now();
     else if (PyObject_IsInstance(o, (PyObject*) &ObserverType))
	  mjd = ((Observer*) o)->now.n_mjd;
     else if (parse_mjd(o, &mjd) == -1)
	  return 0;
     return PyFloat_FromDouble(mjd + 2415020.0);
}

static PyObject *delta_t(PyObject *self, PyObject *args)
{
     PyObject *o = 0;
     double mjd;
     if (!PyArg_ParseTuple(args, "|O:delta_t", &o)) return 0;
     if (!o)
	  mjd = mjd_now();
     else if (PyObject_IsInstance(o, (PyObject*) &ObserverType))
	  mjd = ((Observer*) o)->now.n_mjd;
     else if (parse_mjd(o, &mjd) == -1)
	  return 0;
     return PyFloat_FromDouble(deltat(mjd));
}

static PyObject *moon_phases(PyObject *self, PyObject *args)
{
     PyObject *o = 0, *d;
     double mjd, mjn, mjf;
     if (!PyArg_ParseTuple(args, "|O:moon_phases", &o)) return 0;
     if (!o)
	  mjd = mjd_now();
     else if (PyObject_IsInstance(o, (PyObject*) &ObserverType))
	  mjd = ((Observer*) o)->now.n_mjd;
     else if (parse_mjd(o, &mjd) == -1)
	  return 0;
     moonnf(mjd, &mjn, &mjf);
     o = PyDict_New();
     if (!o) return 0;
     d = build_Date(mjn);
     if (!d) return 0;
     if (PyDict_SetItemString(o, "new", d) == -1) return 0;
     d = build_Date(mjf);
     if (!d) return 0;
     if (PyDict_SetItemString(o, "full", d) == -1) return 0;
     return o;
}

/*
 * The global methods table and the module initialization function.
 */

#undef CREATE
#define CREATE(NAME) \
 {#NAME, (PyCFunction) create_##NAME, METH_VARARGS | METH_KEYWORDS, \
  "return a new instance of " #NAME}

static PyMethodDef ephem_methods[] = {

     CREATE(Mercury),
     CREATE(Venus),
     CREATE(Mars),
     CREATE(Jupiter),
     CREATE(Uranus),
     CREATE(Neptune),
     CREATE(Pluto),
     CREATE(Sun),

     CREATE(Phobos),
     CREATE(Deimos),
     CREATE(Io),
     CREATE(Europa),
     CREATE(Ganymede),
     CREATE(Callisto),
     CREATE(Mimas),
     CREATE(Enceladus),
     CREATE(Tethys),
     CREATE(Dione),
     CREATE(Rhea),
     CREATE(Titan),
     CREATE(Hyperion),
     CREATE(Iapetus),
     CREATE(Ariel),
     CREATE(Umbriel),
     CREATE(Titania),
     CREATE(Oberon),
     CREATE(Miranda),

     {"degrees", degrees, METH_VARARGS, "build an angle measured in degrees"},
     {"hours", hours, METH_VARARGS, "build an angle measured in hours of arc"},
     {"now", build_now, METH_VARARGS, "Return the current time"},
     {"readdb", readdb, METH_VARARGS, "Read an ephem database entry"},
     {"readtle", readtle, METH_VARARGS, "Read TLE-format satellite elements"},
     {"separation", separation, METH_VARARGS,
      "Return the angular separation between two objects or positions "
      "(degrees)"},

     {"uranometria", uranometria, METH_VARARGS,
      "given right ascension and declination (in radians), return the page"
      " of the original Uranometria displaying that location"},
     {"uranometria2000", uranometria2000, METH_VARARGS,
      "given right ascension and declination (in radians), return the page"
      " of the Uranometria 2000.0 displaying that location"},
     {"millennium_atlas", millennium_atlas, METH_VARARGS,
      "given right ascension and declination (in radians), return the page"
      " of the Millenium Star Atlas displaying that location"},

     {"constellation", (PyCFunction) constellation,
      METH_VARARGS | METH_KEYWORDS,
      "Return the constellation in which the object or coordinates lie"},
     {"julian_date", (PyCFunction) julian_date, METH_VARARGS,
      "compute the Julian date"},
     {"delta_t", (PyCFunction) delta_t, METH_VARARGS,
      "compute the difference between Terrestrial Time and Coordinated"
      " Universal Time"},
     {"moon_phases", (PyCFunction) moon_phases, METH_VARARGS,
      "compute the new and full moons nearest a given date"},
     {NULL}
};

#ifdef PyMODINIT_FUNC
PyMODINIT_FUNC
#else
DL_EXPORT(void)
#endif
initephem(void)
{
     PyObject *module;

     /* Initialize pointers to external objects. */

     AngleType.tp_base = &PyFloat_Type;
     DateType.tp_base = &PyFloat_Type;

     ObserverType.tp_new = PyType_GenericNew;
     BodyType.tp_new = PyType_GenericNew;

     /* Ready each type. */

     PyType_Ready(&AngleType);
     PyType_Ready(&DateType);

     PyType_Ready(&ObserverType);

     PyType_Ready(&BodyType);
     PyType_Ready(&PlanetType);
     PyType_Ready(&PlanetMoonType);

     PyType_Ready(&SaturnType);
     PyType_Ready(&MoonType);

     PyType_Ready(&FixedBodyType);
     PyType_Ready(&BinaryStarType);
     PyType_Ready(&EllipticalBodyType);
     PyType_Ready(&HyperbolicBodyType);
     PyType_Ready(&ParabolicBodyType);
     PyType_Ready(&EarthSatelliteType);

     module = Py_InitModule3("ephem", ephem_methods,
			"Astronomical calculations for Python");
     if (!module) return;

     {
	  struct {
	       char *name;
	       PyObject *obj;
	  } objects[] = {
	       { "angle", (PyObject*) & AngleType },
	       { "date", (PyObject*) & DateType },

	       { "Observer", (PyObject*) & ObserverType },

	       { "Body", (PyObject*) & BodyType },
	       { "Planet", (PyObject*) & PlanetType },
	       { "PlanetMoon", (PyObject*) & PlanetMoonType },
	       { "Saturn", (PyObject*) & SaturnType },
	       { "Moon", (PyObject*) & MoonType },

	       { "FixedBody", (PyObject*) & FixedBodyType },
	       /* { "BinaryStar", (PyObject*) & BinaryStarType }, */
	       { "EllipticalBody", (PyObject*) & EllipticalBodyType },
	       { "ParabolicBody", (PyObject*) & ParabolicBodyType },
	       { "HyperbolicBody", (PyObject*) & HyperbolicBodyType },
	       { "EarthSatellite", (PyObject*) & EarthSatelliteType },

	       { "hour", PyFloat_FromDouble(1./24.) },
	       { "minute", PyFloat_FromDouble(1./24./60.) },
	       { "second", PyFloat_FromDouble(1./24./60./60.) },
	       { "meters_per_au", PyFloat_FromDouble(MAU) },
	       { "earth_radius", PyFloat_FromDouble(ERAD) },
	       { "moon_radius", PyFloat_FromDouble(MRAD) },
	       { "sun_radius", PyFloat_FromDouble(SRAD) },

	       { NULL }
	  };
	  int i;
	  for (i=0; objects[i].name; i++)
	       if (PyModule_AddObject(module, objects[i].name, objects[i].obj)
		   == -1)
		    return;
     }

     /* add:
	eq_ecl functions
	eq_gal functions
     */

     /* Set a default preference. */

     pref_set(PREF_DATE_FORMAT, PREF_YMD);

     /* Tell libastro that we do not have data files anywhere. */

     setMoonDir(NULL);
}

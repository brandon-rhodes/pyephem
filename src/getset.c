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
	       scansex(s, &scaled);
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
	       /*f_scansex(scaled, s, &scaled);*/
	       scansex(s, &scaled);
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

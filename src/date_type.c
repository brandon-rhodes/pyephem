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
	  if (scansex(timestr, &hours) == -1) {
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
     PyObject_HEAD_INIT(&PyType_Type)
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
     &PyFloat_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     (newfunc) date_new,	/* tp_new */
     0,				/* tp_free */
};

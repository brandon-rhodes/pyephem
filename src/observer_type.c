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
     PyObject_HEAD_INIT(&PyType_Type)
     0,
     "Observer",
     sizeof(ObserverObject),
     0,
     (destructor) _PyObject_Del, /* tp_dealloc */
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
     PyObject_GenericGetAttr,	/* tp_getattro */
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

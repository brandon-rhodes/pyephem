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
     body->obj.pl.pl_code = typecode;
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
	  switch (body->now_valid) {
	  case 0:
	       PyErr_Format(PyExc_RuntimeError,
			    "field %s undefined until first compute()",
			    fieldname);
	       return -1;
	  case 1:
	       pref_set(PREF_EQUATORIAL, PREF_GEO);
	       body->obj_valid = 1;
	       break;
	  case 2:
	       pref_set(PREF_EQUATORIAL, PREF_TOPO);
	       body->obj_valid = 2;
	       break;
	  }
	  obj_cir(& body->now, & body->obj);
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

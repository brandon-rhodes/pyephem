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
     if (db_crack_line(s, &obj, errmsg)) {
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
	  ra = b->obj.s_ra;
	  dec = b->obj.s_dec;
	  /*epoch = b->epoch;*/
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

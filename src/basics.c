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

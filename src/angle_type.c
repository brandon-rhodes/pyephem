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
     PyObject_HEAD_INIT(&PyType_Type)
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
     &PyFloat_Type,		/* tp_base */
     0,				/* tp_dict */
     0,				/* tp_descr_get */
     0,				/* tp_descr_set */
     0,				/* tp_dictoffset */
     0,				/* tp_init */
     0,				/* tp_alloc */
     0,				/* tp_new */
     0,				/* tp_free */
};

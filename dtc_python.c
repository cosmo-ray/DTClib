#include "dtc-python.h"

static PyObject *dtclib_parse(PyObject *self, PyObject *args)
{
	const char *html;
	struct dtc_error_ctx errctx = {};
	PyObject *ret;

	(void)self;

	if (!PyArg_ParseTuple(args, "s", &html))
		return NULL;

	ret = dtc_python_parse((char *)html, &errctx);
	if (!ret) {
		if (PyErr_Occurred())
			return NULL;
		PyErr_Format(PyExc_ValueError,
			     "parse error at line %d col %d: %s",
			     errctx.line, errctx.col,
			     errctx.err ? errctx.err : "unknown error");
		return NULL;
	}

	if (PyErr_Occurred()) {
		Py_DECREF(ret);
		return NULL;
	}

	return ret;
}

static PyMethodDef dtclib_methods[] = {
	{"parse", dtclib_parse, METH_VARARGS,
	 "Parse HTML string and return a Python list/dict structure."},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef dtclib_module = {
	PyModuleDef_HEAD_INIT,
	"dtclib",
	"DTClib Python backend",
	-1,
	dtclib_methods
};

PyMODINIT_FUNC PyInit_dtclib(void)
{
	return PyModule_Create(&dtclib_module);
}

#ifndef DTCLIB_PYTHON_H_
#define DTCLIB_PYTHON_H_

#include <Python.h>
#include <string.h>

/* Python holds the GIL during parsing, so a non-thread-local error buffer is
 * safe and avoids depending on C11 thread_local support. */
#define DTC_NO_THREAD

#define DTC_UNLIKELY(args...) args

#define DTC_NULL NULL

#define DTC_IS_NULL(c) !(c)

#define DTCLIB_PREFIX python

#define DTC_PTR PyObject *

#define DTC_FREE Py_DECREF

#define DTC_NEW_ARRAY() PyList_New(0)

static PyObject *dtc_python_new_object(PyObject *parent_array)
{
	PyObject *r = PyDict_New();
	if (DTC_UNLIKELY(!r))
		return NULL;
	if (DTC_UNLIKELY(PyList_Append(parent_array, r) < 0)) {
		Py_DECREF(r);
		return NULL;
	}
	Py_DECREF(r);
	return r;
}
#define DTC_NEW_OBJECT(parent_array) dtc_python_new_object(parent_array)

static int dtc_python_tag_name(PyObject *obj_cnt, char name[static 1],
			       size_t name_l)
{
	PyObject *s = PyUnicode_FromStringAndSize(name, name_l);
	if (DTC_UNLIKELY(!s))
		return -1;
	int ret = PyDict_SetItemString(obj_cnt, "name", s);
	Py_DECREF(s);
	return ret;
}
#define DTC_TAG_NAME(obj_cnt, name, name_l) dtc_python_tag_name(obj_cnt, name, name_l)

static PyObject *dtc_python_new_object_object(PyObject *obj_cnt, char name[static 1])
{
	PyObject *r = PyDict_New();
	if (DTC_UNLIKELY(!r))
		return NULL;
	if (DTC_UNLIKELY(PyDict_SetItemString(obj_cnt, name, r) < 0)) {
		Py_DECREF(r);
		return NULL;
	}
	Py_DECREF(r);
	return r;
}
#define DTC_NEW_OBJECT_OBJECT(obj_cnt, name) dtc_python_new_object_object(obj_cnt, name)

static PyObject *dtc_python_store_strl_keyl(PyObject *obj_cnt,
					    size_t nl, char name[static nl],
					    size_t vl, char val[static vl])
{
	PyObject *key = PyUnicode_FromStringAndSize(name, nl);
	if (DTC_UNLIKELY(!key))
		return NULL;
	PyObject *value = PyUnicode_FromStringAndSize(val, vl - 1);
	if (DTC_UNLIKELY(!value)) {
		Py_DECREF(key);
		return NULL;
	}
	int ret = PyDict_SetItem(obj_cnt, key, value);
	Py_DECREF(key);
	Py_DECREF(value);
	if (DTC_UNLIKELY(ret < 0))
		return NULL;
	return value;
}
#define DTC_STORE_STRL_KEYL(obj_cnt, nl, name, vl, val) \
	dtc_python_store_strl_keyl(obj_cnt, nl, name, vl, val)

static PyObject *dtc_python_store_bool_keyl(PyObject *obj_cnt,
					    size_t nl, char name[static nl],
					    _Bool val)
{
	PyObject *key = PyUnicode_FromStringAndSize(name, nl);
	if (DTC_UNLIKELY(!key))
		return NULL;
	PyObject *value = PyBool_FromLong(val ? 1 : 0);
	if (DTC_UNLIKELY(!value)) {
		Py_DECREF(key);
		return NULL;
	}
	int ret = PyDict_SetItem(obj_cnt, key, value);
	Py_DECREF(key);
	Py_DECREF(value);
	if (DTC_UNLIKELY(ret < 0))
		return NULL;
	return value;
}
#define DTC_STORE_BOOL_KEYL(obj_cnt, nl, name, val) \
	dtc_python_store_bool_keyl(obj_cnt, nl, name, val)

static PyObject *dtc_python_store_strl_key(PyObject *obj_cnt,
					   char key[static 1],
					   size_t vl, char val[static vl])
{
	PyObject *value = PyUnicode_FromStringAndSize(val, vl - 1);
	if (DTC_UNLIKELY(!value))
		return NULL;
	int ret = PyDict_SetItemString(obj_cnt, key, value);
	Py_DECREF(value);
	if (DTC_UNLIKELY(ret < 0))
		return NULL;
	return value;
}
#define DTC_STORE_STRL_KEY(obj_cnt, key, vl, val) \
	dtc_python_store_strl_key(obj_cnt, key, vl, val)

static PyObject *dtc_python_store_array(PyObject *obj_cnt,
					char key[static 1])
{
	PyObject *r = PyList_New(0);
	if (DTC_UNLIKELY(!r))
		return NULL;
	if (DTC_UNLIKELY(PyDict_SetItemString(obj_cnt, key, r) < 0)) {
		Py_DECREF(r);
		return NULL;
	}
	Py_DECREF(r);
	return r;
}
#define DTC_STORE_ARRAY(obj_cnt, key) dtc_python_store_array(obj_cnt, key)

static PyObject *dtc_python_push_strl(PyObject *parent_array, size_t vl,
				      char val[static vl])
{
	PyObject *r = PyUnicode_FromStringAndSize(val, vl - 1);
	if (DTC_UNLIKELY(!r))
		return NULL;
	if (DTC_UNLIKELY(PyList_Append(parent_array, r) < 0)) {
		Py_DECREF(r);
		return NULL;
	}
	Py_DECREF(r);
	return r;
}
#define DTC_PUSH_STRL(parent_array, vl, val) dtc_python_push_strl(parent_array, vl, val)

#include "dtclib.h"

#endif

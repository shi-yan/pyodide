#include "epiimport.h"

#include <emscripten.h>

#include "hiwire.h"
#include "js2python.h"

static PyObject* epi_module = NULL;

static PyObject*
EpiImport_GetAttr(PyObject* self, PyObject* attr)
{
  const char* c = PyUnicode_AsUTF8(attr);
  if (c == NULL) {
    return NULL;
  }
  int idval = hiwire_get_global((int)c);
  if (idval == -1) {
    PyErr_Format(PyExc_AttributeError, "Unknown attribute '%s'", attr);
    return NULL;
  }
  PyObject* result = js2python(idval);
  hiwire_decref(idval);
  return result;
}

static PyObject*
EpiImport_Dir()
{
  int idwindow = hiwire_get_global((int)"self");
  int iddir = hiwire_dir(idwindow);
  hiwire_decref(idwindow);
  PyObject* pydir = js2python(iddir);
  hiwire_decref(iddir);
  return pydir;
}

static PyMethodDef EpiModule_Methods[] = {
  { "__getattr__",
    (PyCFunction)EpiImport_GetAttr,
    METH_O,
    "Get an object from the global Javascript namespace" },
  { "__dir__",
    (PyCFunction)EpiImport_Dir,
    METH_NOARGS,
    "Returns a list of object name in the global Javascript namespace" },
  { NULL }
};

static struct PyModuleDef EpiModule = {
  PyModuleDef_HEAD_INIT,
  "epi",
  "Provides access to Epiphany functions.",
  0,
  EpiModule_Methods
};

int
EpiImport_init()
{
  PyObject* module_dict = PyImport_GetModuleDict();
  if (module_dict == NULL) {
    return 1;
  }

  epi_module = PyModule_Create(&EpiModule);
  if (epi_module == NULL) {
    return 1;
  }

  if (PyDict_SetItemString(module_dict, "epi", epi_module)) {
    Py_DECREF(epi_module);
    return 1;
  }

  return 0;
}

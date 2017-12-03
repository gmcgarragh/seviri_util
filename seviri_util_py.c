/*******************************************************************************
 *
 *    Copyright (C) 2014-2015 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#include <python2.7/Python.h>
#include <python2.7/structmember.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <float.h>

#include "seviri_util.h"


struct seviri_preproc_data_py {
     PyObject_HEAD
     char *filename;
     uint n_bands;
     uint n_lines;
     uint n_columns;
     double fill_value;
     PyObject *time;
     PyObject *lat;
     PyObject *lon;
     PyObject *sza;
     PyObject *saa;
     PyObject *vza;
     PyObject *vaa;
     PyObject *data;
     struct seviri_preproc_data d;
};


static PyObject *SEVIRI_PREPROC_Error;


static int bounds_string_to_enum(const char *s, enum seviri_bounds *bounds) {

     char temp[128];

     if (strcmp(s, "full_disk") == 0)
          *bounds = SEVIRI_BOUNDS_FULL_DISK;
     else if (strcmp(s, "actual_image") == 0)
          *bounds = SEVIRI_BOUNDS_ACTUAL_IMAGE;
     else if (strcmp(s, "line_column") == 0)
          *bounds = SEVIRI_BOUNDS_LINE_COLUMN;
     else if (strcmp(s, "lat_lon") == 0)
          *bounds = SEVIRI_BOUNDS_LAT_LON;
     else {
          snprintf(temp, 128, "ERROR: invalid bounds type: %s", s);
          fprintf(stderr, "%s\n", temp);
          PyErr_SetString(SEVIRI_PREPROC_Error, temp);
          return -1;
     }

     return 0;
};


static int unit_string_to_enum(const char *s, enum seviri_bounds *unit) {

     char temp[128];

     if (strcmp(s, "Radiance") == 0)
          *unit = SEVIRI_UNIT_RAD;
     else if (strcmp(s, "BRF") == 0)
          *unit = SEVIRI_UNIT_BRF;
     else if (strcmp(s, "BT") == 0)
          *unit = SEVIRI_UNIT_BT;
     else {
          snprintf(temp, 128, "ERROR: invalid unit type: %s", s);
          fprintf(stderr, "%s\n", temp);
          PyErr_SetString(SEVIRI_PREPROC_Error, temp);
          return -1;
     }

     return 0;
};



static PyObject *seviri_preproc_new(PyTypeObject *type,
                                        PyObject *args, PyObject *keywords) {
     struct seviri_preproc_data_py *self;

     self = (struct seviri_preproc_data_py *) type->tp_alloc(type, 0);
     if (self == NULL) {
          PyErr_SetString(SEVIRI_PREPROC_Error, "ERROR: error allocating memory "
                          "for seviri_util object");
          return NULL;
     }

     return (PyObject *) self;
}



static int seviri_preproc_init(struct seviri_preproc_data_py *self,
                               PyObject *args, PyObject *keywords) {

     char *s;

     char *bounds_string;

      /* Return value initialized to -1 (failure) */
     int r = -1;

     uint i;

     uint i_line;
     uint i_column;
     uint n_lines;
     uint n_columns;

     enum seviri_bounds bounds;

     uint line0;
     uint line1;
     uint column0;
     uint column1;

     uint n_bands;

     uint *band_ids_array   = NULL;
     uint *band_units_array = NULL;

     double lat0;
     double lat1;
     double lon0;
     double lon1;

     /* Objects initialized to NULL for Py_XDECREF */
     PyListObject *band_ids_list   = NULL;
     PyListObject *band_units_list = NULL;

     static char *kw[] = {"filename", "band_ids", "units", "bounds", "pixel_coords",
                          "lat_lon_coords", NULL};

     if (! PyArg_ParseTupleAndKeywords(args, keywords, "sOOs|(IIII)(dddd)", kw,
          &self->filename, &band_ids_list, &band_units_list, &bounds_string,
          &line0, &line1, &column0, &column1, &lat0, &lat1, &lon0, &lon1))
          goto error;

     n_bands = PyList_Size((PyObject *) band_ids_list);

     band_ids_array = malloc(n_bands * sizeof(uint));
     for (i = 0; i < n_bands; ++i)
          band_ids_array[i] =
               PyLong_AsLong(PyList_GetItem((PyObject *) band_ids_list, i));

     if (n_bands != PyList_Size((PyObject *) band_units_list)) {
          PyErr_SetString(SEVIRI_PREPROC_Error, "ERROR: number of band ids and "
                          "number of units do not match");
          goto error;
     }
     band_units_array = malloc(n_bands * sizeof(uint));
     for (i = 0; i < n_bands; ++i) {
          s = PyString_AsString(PyList_GetItem((PyObject *) band_units_list, i));
          if (unit_string_to_enum(s, &band_units_array[i])) {
               fprintf(stderr, "ERROR: unit_string_to_enum()\n");
               goto error;
          }
     }

     if (bounds_string_to_enum(bounds_string, &bounds)) {
          fprintf(stderr, "ERROR: bounds_string_to_enum()\n");
          goto error;
     }

     if (PyDict_Contains(keywords, PyString_FromString("pixel_coords")) &&
         PyDict_Contains(keywords, PyString_FromString("lat_lon_coords"))) {
          PyErr_SetString(SEVIRI_PREPROC_Error, "ERROR: cannot use both the "
               "\"pixel_coords\" and \"lat_lon_coords\" keywords");
          goto error;
     }

     if (bounds == SEVIRI_BOUNDS_FULL_DISK ||
         bounds == SEVIRI_BOUNDS_ACTUAL_IMAGE) {
          if (PyDict_Contains(keywords, PyString_FromString("pixel_coords")) ||
              PyDict_Contains(keywords, PyString_FromString("lat_lon_coords"))) {
               PyErr_SetString(SEVIRI_PREPROC_Error, "ERROR: cannot use the "
                    "\"pixel_coords\" or \"lat_lon_coords\" keywords with "
                    "\"full_disk\" or \"actual_image\" bounds");
               goto error;
          }
     }
     else if (bounds == SEVIRI_BOUNDS_LINE_COLUMN) {
          if (! PyDict_Contains(keywords, PyString_FromString("pixel_coords"))) {
               PyErr_SetString(SEVIRI_PREPROC_Error, "ERROR: must use the "
                    "\"pixel_coords\" keyword with \"line_column\" bounds");
               goto error;
          }
     }
     else if (bounds == SEVIRI_BOUNDS_LAT_LON) {
          if (! PyDict_Contains(keywords, PyString_FromString("lat_lon_coords"))) {
               PyErr_SetString(SEVIRI_PREPROC_Error, "ERROR: must use the "
                    "\"lat_lon_coords\" keyword with \"lat_lon\" bounds");
               goto error;
          }
     }

     if (seviri_read_and_preproc(self->filename, &self->d, n_bands, band_ids_array,
          band_units_array, bounds, line0, line1, column0, column1, lat0, lat1,
          lon0, lon1, 0)) {
          PyErr_SetString(SEVIRI_PREPROC_Error, "ERROR: seviri_read_and_preproc()");
          goto error;
     }

     self->n_bands    = self->d.n_bands;
     self->n_lines    = self->d.n_lines;
     self->n_columns  = self->d.n_columns;
     self->fill_value = self->d.fill_value;

     npy_intp dims[3];

     dims[0] = self->n_lines;
     dims[1] = self->n_columns;

     self->time = PyArray_SimpleNewFromData(2, dims, NPY_DOUBLE, self->d.time);
     self->lat  = PyArray_SimpleNewFromData(2, dims, NPY_FLOAT,  self->d.lat);
     self->lon  = PyArray_SimpleNewFromData(2, dims, NPY_FLOAT,  self->d.lon);
     self->sza  = PyArray_SimpleNewFromData(2, dims, NPY_FLOAT,  self->d.sza);
     self->saa  = PyArray_SimpleNewFromData(2, dims, NPY_FLOAT,  self->d.saa);
     self->vza  = PyArray_SimpleNewFromData(2, dims, NPY_FLOAT,  self->d.vza);
     self->vaa  = PyArray_SimpleNewFromData(2, dims, NPY_FLOAT,  self->d.vaa);

     dims[0] = n_bands;
     dims[1] = self->n_lines;
     dims[2] = self->n_columns;

     self->data = PyArray_SimpleNewFromData(3, dims, NPY_FLOAT,  self->d.data2);

     r = 0; /* Success */

error:
     /* Cleanup code, shared by success and failure path */
     free(band_ids_array);
     free(band_units_array);

     /* Decrement reference counts with Py_XDECREF() to ignore NULL references. */
     Py_XDECREF(band_ids_list);
     Py_XDECREF(band_units_list);

     return r;
}


static void seviri_preproc_dealloc(struct seviri_preproc_data_py *self) {

     int r;

     r = seviri_preproc_free(&self->d);
     if (r) {
          PyErr_SetString(SEVIRI_PREPROC_Error, "ERROR: seviri_free()");
          return;
     }

     self->ob_type->tp_free((PyObject *) self);
}



static PyMethodDef seviri_preproc_methods[] = {
     {NULL}
};



static PyMemberDef seviri_preproc_members[] = {
     {"n_bands",   T_UINT, offsetof(struct seviri_preproc_data_py, n_bands),
      0, "n_bands"},
     {"n_lines",   T_UINT, offsetof(struct seviri_preproc_data_py, n_lines),
      0, "n_lines"},
     {"n_columns", T_UINT, offsetof(struct seviri_preproc_data_py, n_columns),
      0, "n_columns"},
     {"fill_value", T_DOUBLE, offsetof(struct seviri_preproc_data_py, fill_value),
      0, "fill_value"},
     {"time", T_OBJECT, offsetof(struct seviri_preproc_data_py, time),
      0, "time"},
     {"lat", T_OBJECT, offsetof(struct seviri_preproc_data_py, lat),
      0, "lat"},
     {"lon", T_OBJECT, offsetof(struct seviri_preproc_data_py, lon),
      0, "lon"},
     {"sza", T_OBJECT, offsetof(struct seviri_preproc_data_py, sza),
      0, "sza"},
     {"saa", T_OBJECT, offsetof(struct seviri_preproc_data_py, saa),
      0, "saa"},
     {"vza", T_OBJECT, offsetof(struct seviri_preproc_data_py, vza),
      0, "vza"},
     {"vaa", T_OBJECT, offsetof(struct seviri_preproc_data_py, vaa),
      0, "vaa"},
     {"data", T_OBJECT, offsetof(struct seviri_preproc_data_py, data),
      0, "data"},
     {NULL}
};


static PyTypeObject seviri_type = {
     PyObject_HEAD_INIT(NULL)
     0,
     "seviri_util.seviri_preproc",
     sizeof(struct seviri_preproc_data_py),
     0,
     (destructor) seviri_preproc_dealloc,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
     "SEVIRI_NATIVE object",
     0,
     0,
     0,
     0,
     0,
     0,
     seviri_preproc_methods,
     seviri_preproc_members,
     0,
     0,
     0,
     0,
     0,
     0,
     (initproc) seviri_preproc_init,
     0,
     (newfunc) seviri_preproc_new,
};


static PyMethodDef module_methods[] = {
     {NULL}
};


#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif


PyMODINIT_FUNC initseviri_util(void) {
     PyObject *module;

     if (PyType_Ready(&seviri_type) < 0)
          return;

     module = Py_InitModule3("seviri_util", module_methods,
                             "Module for accessing seviri_util");
     if (module == NULL)
          return;

     import_array();

     Py_INCREF(&seviri_type);
     PyModule_AddObject(module, "seviri_preproc", (PyObject *) &seviri_type);
     SEVIRI_PREPROC_Error = PyErr_NewException("seviri_preproc.error", NULL, NULL);
     Py_INCREF(SEVIRI_PREPROC_Error);
     PyModule_AddObject(module, "error", SEVIRI_PREPROC_Error);
}

#define PY_SSIZE_T_CLEAN
#include "kmeansmodule.h"
#include <Python.h>

vector *build_vectors(PyObject *PyVectors, int dim, int n) {

    vector *head_vec, *curr_vec;
    cord *head_cord, *curr_cord;
    PyObject *item;
    int i = 0;
    double x;
    head_cord = (cord *)malloc(sizeof(cord));
    curr_cord = head_cord;
    curr_cord->next = NULL;

    head_vec = (vector *)malloc(sizeof(vector));
    curr_vec = head_vec;
    curr_vec->next = NULL;

    if (head_cord == NULL || head_vec == NULL) {
        printf("Error Has Occurred\n");
        free(curr_cord);
        free(curr_vec);
        return NULL;
    }
    for (i = 0; i < n; i++) {

        for (int j = 0; j < (dim - 1); j++) {
            PyObject *item = PyList_GetItem(PyVectors, (i * dim) + j);
            x = PyFloat_AsDouble(item);
            curr_cord->value = x;
            curr_cord->next = (cord *)malloc(sizeof(cord));
            curr_cord = curr_cord->next;
            if (curr_cord == NULL) {
                printf("Error Has Occurred\n");
                free_vectors(head_vec);
                return NULL;
            }
        }
        PyObject *item = PyList_GetItem(PyVectors, (i + 1) * dim - 1);
        x = PyFloat_AsDouble(item);
        curr_cord->next = NULL;
        curr_cord->value = x;
        curr_vec->cords = head_cord;
        curr_vec->next = (vector *)malloc(sizeof(vector));
        curr_vec = curr_vec->next;
        curr_vec->cords = NULL;
        curr_vec->next = NULL;
        head_cord = (cord *)malloc(sizeof(cord));
        curr_cord = head_cord;
        curr_cord->next = NULL;

        if (curr_cord == NULL || curr_vec == NULL) {
            printf("Error Has Occurred\n");
            free_vectors(head_vec);
            free_cords(head_cord);
            return NULL;
        }
    }
    free_cords(head_cord);
    return head_vec;
}

static PyObject *fit(PyObject *self, PyObject *args) {
    PyObject *PyVectors;
    PyObject *PyCentroids;
    int k, max_iter, dim, n;
    if (!PyArg_ParseTuple(args, "OOiiii", &PyVectors, &PyCentroids, &k, &max_iter, &dim, &n))
        return NULL;
    vector *head_vec = build_vectors(PyVectors, dim, n);
    vector *centroid_vec = build_vectors(PyCentroids, dim, k);
    centroid *head_cen = initialize_centroids(k, centroid_vec);
    free_vectors(centroid_vec);
    if (assign_clusters(max_iter, head_vec, head_cen) != 0) {
        free_all(head_vec, head_cen);
    } else {
        Py_ssize_t total = (Py_ssize_t)k * dim;
        PyObject *result = PyList_New(total);
        if (!result) {
            return NULL;
        }
        centroid *curr_cen = head_cen;
        cord *curr_cord;
        Py_ssize_t ind = 0;
        for (int i = 0; i < k; i++) {
            curr_cord = curr_cen->center.cords;
            for (int j = 0; j < dim; j++) {
                double x = curr_cord->value;
                PyList_SetItem(result, ind, PyFloat_FromDouble(x));
                ind++;
                curr_cord = curr_cord->next;
            }
            curr_cen = curr_cen->next;
        }
        free_all(head_vec, head_cen);
        return result;
    }
    return NULL;
}

static PyMethodDef ModuleMethods[] = {
    {"fit",
     (PyCFunction)fit,
     METH_VARARGS,
     "Run k-means clustering and return the final centroids. expects a list of vectors and a list of centroids. then: k, max_iter, dim, n. returns a list of centroids."},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef mymodule = {
    PyModuleDef_HEAD_INIT,
    "mykmeanspp",
    NULL,
    -1,
    ModuleMethods};

PyMODINIT_FUNC PyInit_myext(void) {
    PyObject *m = PyModule_Create(&mymodule);
    return !m ? NULL : m;
}
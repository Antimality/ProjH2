#define PY_SSIZE_T_CLEAN
#include "kmeansmodule.h"
#include <Python.h>

vector *build_vectors(PyObject *PyPoints, int dim, int n) {

    vector *head_vec, *curr_vec;
    cord *head_cord, *curr_cord;
    PyObject *p, *c;
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
        p = PyList_GetItem(PyPoints, i);
        for (int j = 0; j < (dim - 1); j++) {
            c = PyTuple_GetItem(p, j);
            x = PyFloat_AsDouble(c);
            curr_cord->value = x;
            curr_cord->next = (cord *)malloc(sizeof(cord));
            curr_cord = curr_cord->next;
            if (curr_cord == NULL) {
                printf("Error Has Occurred\n");
                free_vectors(head_vec);
                return NULL;
            }
        }
        c = PyTuple_GetItem(p, dim - 1);
        x = PyFloat_AsDouble(c);
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
    PyObject *PyPoints;
    PyObject *PyCentroids;
    int k, max_iter, dim, n;
    double eps;

    if (!PyArg_ParseTuple(args, "OOiiiid", &PyPoints, &PyCentroids, &k, &max_iter, &dim, &n, &eps)) {
        return NULL;
    }

    // Translate points and centroids to C
    vector *head_vec = build_vectors(PyPoints, dim, n);
    vector *centroid_vec = build_vectors(PyCentroids, dim, k);
    if (!head_vec || !centroid_vec)
        return NULL;
    // Translate vector list to centroid list
    centroid *head_cen = initialize_centroids(k, centroid_vec);
    free_vectors(centroid_vec);
    if (!head_cen)
        return NULL;

    // Kmeans loop
    if (assign_clusters(max_iter, head_vec, head_cen, eps) != 0) {
        free_all(head_vec, head_cen);
        return NULL;
    }

    // Generate return (python) list
    PyObject *cent, *centroids = PyList_New(k);
    if (!centroids)
        return NULL;
    centroid *curr_cen = head_cen;
    cord *curr_cord;
    for (int i = 0; i < k; i++) {
        curr_cord = curr_cen->center.cords;
        cent = PyTuple_New(dim);
        for (int j = 0; j < dim; j++) {
            double x = curr_cord->value;
            PyTuple_SetItem(cent, j, PyFloat_FromDouble(x));
            curr_cord = curr_cord->next;
        }
        curr_cen = curr_cen->next;
        PyList_SetItem(centroids, i, cent);
    }
    free_all(head_vec, head_cen);
    return centroids;
}

static PyMethodDef ModuleMethods[] = {
    {"fit",
     (PyCFunction)fit,
     METH_VARARGS,
     PyDoc_STR("Run k-means clustering and return the final centroids. "
               "expects a list of points and a list of centroids. "
               "then: k, max_iter, dim, n. returns a list of centroids.")},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef mymodule = {
    PyModuleDef_HEAD_INIT,
    "mykmeanspp",
    NULL,
    -1,
    ModuleMethods};

PyMODINIT_FUNC PyInit_mykmeanspp(void) {
    PyObject *m = PyModule_Create(&mymodule);
    return !m ? NULL : m;
}
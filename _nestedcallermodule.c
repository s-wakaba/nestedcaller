#include "Python.h"
#include "structmember.h"

/* _nestedcaller module written and maintained
   by s-wakaba@github.com
*/

#define Py_SETREF(op, op2)                      \
    do {                                        \
        PyObject *_py_tmp = (PyObject *)(op);   \
        (op) = (op2);                           \
        Py_DECREF(_py_tmp);                     \
    } while (0)


typedef struct {
    PyObject_HEAD
    PyObject *funcs;
    PyObject *weakreflist; /* List of weak references */
} nestedcallerobject;

static PyTypeObject nestedcaller_type;

static PyObject *
nestedcaller_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
    nestedcallerobject *pto;
    int i, m, n, nested;

    if(kw != NULL && PyDict_Size(kw) > 0) {
        PyErr_SetString(PyExc_TypeError,
                        "type 'nestedcaller' takes no keyword arguments");
        return NULL;
    }

    pto = (nestedcallerobject *)type->tp_alloc(type, 0);
    if (pto == NULL) {
        return NULL;
    }

    m = PyTuple_GET_SIZE(args);
    n = 0;
    nested = 0;
    if(type == &nestedcaller_type) {
        for(i=0; i<m; ++i) {
            PyObject *func = PyTuple_GET_ITEM(args, i);
            if(Py_TYPE(func) != &nestedcaller_type) {
                n += 1;
            }
            else {
                PyObject *subfuncs = ((nestedcallerobject*)func)->funcs;
                n += PyTuple_GET_SIZE(subfuncs);
                nested = 1;
            }
        }
    }
    if(!nested) {
        Py_INCREF(args);
        pto->funcs = args;
    }
    else {
        int j = 0;
        pto->funcs = PyTuple_New(n);
        if(pto->funcs == NULL) {
            Py_DECREF(pto);
            return NULL;
        }
        for(i=0; i<m; ++i) {
            PyObject *func = PyTuple_GET_ITEM(args, i);
            if(Py_TYPE(func) != &nestedcaller_type) {
                PyTuple_SET_ITEM(pto->funcs, j++, func);
            }
            else {
                PyObject *subfuncs = ((nestedcallerobject*)func)->funcs;
                int subi;
                int subm = PyTuple_GET_SIZE(subfuncs);
                for(subi = 0; subi < subm; ++subi) {
                    PyTuple_SET_ITEM(pto->funcs, j++,
                            PyTuple_GET_ITEM(subfuncs, subi));
                }
            }
        }
    }

    return (PyObject *)pto;
}

static void
nestedcaller_dealloc(nestedcallerobject *pto)
{
    PyObject_GC_UnTrack(pto);
    if (pto->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *) pto);
    Py_XDECREF(pto->funcs);
    Py_TYPE(pto)->tp_free(pto);
}

static PyObject *
nestedcaller_call(nestedcallerobject *pto, PyObject *args, PyObject *kw)
{
    PyObject *ret;
    int i, n;

    assert (PyTuple_Check(pto->funcs));

    if(kw != 0 && PyDict_Size(kw) > 0) {
        PyErr_SetString(PyExc_ValueError,
                        "'nestedcaller' takes no keyword arguments");
        return NULL;
    }
    if(PyTuple_GET_SIZE(args) != 1) {
        PyErr_SetString(PyExc_ValueError,
                        "'nestedcaller' takes just one argument");
        return NULL;
    }

    ret = PyTuple_GET_ITEM(args, 0);
    Py_INCREF(ret);
    n = PyTuple_GET_SIZE(pto->funcs);
    for(i=0; i<n; ++i) {
        Py_SETREF(ret,
            PyObject_CallFunctionObjArgs(PyTuple_GET_ITEM(pto->funcs, i),
            ret, NULL));
        if(ret == NULL)
            return NULL;
    }
    return ret;
}

static int
nestedcaller_traverse(nestedcallerobject *pto, visitproc visit, void *arg)
{
    Py_VISIT(pto->funcs);
    return 0;
}

PyDoc_STRVAR(nestedcaller_doc,
"nestedcaller(*funcs) - new function with calling nested functions.\n");

#define OFF(funcs) offsetof(nestedcallerobject, funcs)
static PyMemberDef nestedcaller_memberlist[] = {
    {"funcs", T_OBJECT, offsetof(nestedcallerobject, funcs), READONLY,
     "tuple object to use in future nestedcaller calls"},
    {NULL}  /* Sentinel */
};

static PyGetSetDef nestedcaller_getsetlist[] = {
    {NULL} /* Sentinel */
};

static PyObject *
nestedcaller_repr(nestedcallerobject *pto)
{
    PyObject *result = NULL;
    PyObject *arglist;
    Py_ssize_t i, n;
    int status;

    status = Py_ReprEnter((PyObject *)pto);
    if (status != 0) {
        if (status < 0)
            return NULL;
        return PyUnicode_FromString("...");
    }

    arglist = PyUnicode_FromString("");
    if (arglist == NULL)
        goto done;

    assert (PyTuple_Check(pto->funcs));
    n = PyTuple_GET_SIZE(pto->funcs);
    for (i = 0; i < n; i++) {
        if(i == 0) {
            Py_SETREF(arglist, PyUnicode_FromFormat("%R",
                                        PyTuple_GET_ITEM(pto->funcs, i)));
        }
        else {
            Py_SETREF(arglist, PyUnicode_FromFormat("%U, %R", arglist,
                                        PyTuple_GET_ITEM(pto->funcs, i)));
        }
        if (arglist == NULL)
            goto done;
    }
    result = PyUnicode_FromFormat("%s(%U)", Py_TYPE(pto)->tp_name,
                                  arglist);
    Py_DECREF(arglist);

 done:
    Py_ReprLeave((PyObject *)pto);
    return result;
}

static PyObject *
nestedcaller_reduce(nestedcallerobject *pto, PyObject *unused)
{
    return Py_BuildValue("OO", Py_TYPE(pto), pto->funcs);
}

static PyMethodDef nestedcaller_methods[] = {
    {"__reduce__", (PyCFunction)nestedcaller_reduce, METH_NOARGS},
    {NULL,              NULL}           /* sentinel */
};

static PyTypeObject nestedcaller_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nestedcaller.nestedcaller",        /* tp_name */
    sizeof(nestedcallerobject),         /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)nestedcaller_dealloc,   /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_reserved */
    (reprfunc)nestedcaller_repr,        /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    (ternaryfunc)nestedcaller_call,     /* tp_call */
    0,                                  /* tp_str */
    PyObject_GenericGetAttr,            /* tp_getattro */
    PyObject_GenericSetAttr,            /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
        Py_TPFLAGS_BASETYPE,            /* tp_flags */
    nestedcaller_doc,                   /* tp_doc */
    (traverseproc)nestedcaller_traverse,/* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    offsetof(nestedcallerobject, weakreflist), /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    nestedcaller_methods,               /* tp_methods */
    nestedcaller_memberlist,            /* tp_members */
    nestedcaller_getsetlist,            /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    nestedcaller_new,                   /* tp_new */
    PyObject_GC_Del,                    /* tp_free */
};

/* module level code ********************************************************/

PyDoc_STRVAR(module_doc,
"nestedcaller module (C implementation)");

static PyMethodDef module_methods[] = {
    {NULL,              NULL}           /* sentinel */
};

static void
module_free(void *m)
{
}

static struct PyModuleDef _functoolsmodule = {
    PyModuleDef_HEAD_INIT,
    "_nestedcaller",
    module_doc,
    -1,
    module_methods,
    NULL,
    NULL,
    NULL,
    module_free,
};

PyMODINIT_FUNC
PyInit__nestedcaller(void)
{
    int i;
    PyObject *m;
    char *name;
    PyTypeObject *typelist[] = {
        &nestedcaller_type,
        NULL
    };

    m = PyModule_Create(&_functoolsmodule);
    if (m == NULL)
        return NULL;

    for (i=0 ; typelist[i] != NULL ; i++) {
        if (PyType_Ready(&nestedcaller_type) < 0) {
            Py_DECREF(m);
            return NULL;
        }
        name = strchr(typelist[i]->tp_name, '.');
        assert (name != NULL);
        Py_INCREF(typelist[i]);
        PyModule_AddObject(m, name+1, (PyObject *)typelist[i]);
    }
    return m;
}

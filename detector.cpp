// Copyright deepintell
// License
// Author: tripleha
//

#include "/home/tripleha/anaconda2/envs/python36/include/python3.6m/Python.h"
#include "/home/tripleha/anaconda2/envs/python36/include/python3.6m/structmember.h"
#include "acdetector.h"
#include <iostream>

using std::cout;
using std::endl;

typedef struct
{
    PyObject_HEAD
        TrieTreeNode *root; // detector类的AC自动机根节点指针
    int using_count;
} ACDetector;

static void
ACDetector_dealloc(ACDetector *self)
{
    // 增加对于root的析构函数，要加一层判断，如果root已经析构就不需要
    if (self->root)
    {
        delete self->root;
        self->root = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
ACDetector_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    ACDetector *self;

    self = (ACDetector *)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->root = NULL;
        self->using_count = 0;
    }

    return (PyObject *)self;
}

static int
ACDetector_init(ACDetector *self, PyObject *args, PyObject *kwds)
{
    self->root = NULL;
    self->using_count = 0;
    return 0;
}

// 定义类的方法

static PyObject *_build_ac(ACDetector *self, PyObject *args)
{
    if (self->using_count > 0)
    {
        PyErr_SetString(PyExc_PermissionError, "the ac automata is working");
        return NULL;
    }

    PyObject *str_list;
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &str_list))
    {
        return NULL;
    }

    Py_ssize_t words_count = PyList_Size(str_list);
    if (!words_count)
    {
        if (self->root)
        {
            delete self->root;
            self->root = NULL;
        }
        Py_RETURN_TRUE;
    }

    // 将python 列表中的字符串导入到vector中，并检查类型是否正确
    vector<string> word_list;
    word_list.reserve(words_count);
    PyObject *tmp_str;
    for (Py_ssize_t i = 0; i < words_count; i++)
    {
        tmp_str = PyList_GetItem(str_list, i);
        if (!PyUnicode_CheckExact(tmp_str))
        {
            PyErr_Format(PyExc_TypeError, "list position %d is not a unicode string object", i);
            return NULL;
        }
        if (PyUnicode_READY(tmp_str) == -1)
        {
            return NULL;
        }
        const char *chars = PyUnicode_AsUTF8(tmp_str);
        if (!chars)
        {
            return NULL;
        }
        string word(chars);
        word_list.push_back(word);
    }

    if (self->root)
    {
        delete self->root;
        self->root = NULL;
    }
    self->root = create_ac(&word_list);
    vector<string>().swap(word_list);

    Py_RETURN_TRUE;
}

static PyObject *_processing(ACDetector *self, PyObject *args)
{
    const char *raw_string;
    vector<TripleIntNode> result;

    if (!PyArg_ParseTuple(args, "s", &raw_string))
    {
        return NULL;
    }
    string words(raw_string);

    if (self->root)
    {
        self->using_count++;
        find_all_ac(self->root, &words, &result);
        self->using_count--;
    }

    PyObject *tuple = PyTuple_New(result.size());

    int i = 0;
    for (auto xyz : result)
    {
        PyTuple_SET_ITEM(tuple, i++, Py_BuildValue("{s:i, s:i, s:i}", "head", xyz.first, "tail", xyz.second, "pos", xyz.third));
    }
    return tuple;
}

static PyObject *_is_active(ACDetector *self)
{
    if (self->using_count > 0)
    {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *_clear_ac(ACDetector *self)
{
    if (self->using_count > 0)
    {
        PyErr_SetString(PyExc_PermissionError, "the ac automata is working");
        return NULL;
    }
    if (self->root)
    {
        delete self->root;
        self->root = NULL;
    }
    Py_RETURN_TRUE;
}

static PyMemberDef ACDetector_members[] = {
    // 因为成员变量不需要在python中进行操作，所以此处未定义成员变量
    {NULL} // Sentinel
};

static PyMethodDef ACDetector_methods[] = {
    {"build_ac", (PyCFunction)_build_ac, METH_VARARGS, "build ac automata"},
    {"processing", (PyCFunction)_processing, METH_VARARGS, "searching by ac automata"},
    {"is_active", (PyCFunction)_is_active, METH_NOARGS, "check if the ac automata is working"},
    {"clear_ac", (PyCFunction)_clear_ac, METH_NOARGS, "free the memory that ac automata has used"},
    {NULL} // Sentinel
};

static PyTypeObject acdetector_ACDetectorType = {
    PyVarObject_HEAD_INIT(NULL, 0) "acdetector.ACDetector", // tp_name
    sizeof(ACDetector),                                     // tp_basicsize
    0,                                                      // tp_itemsize
    (destructor)ACDetector_dealloc,                         // tp_dealloc
    0,                                                      // tp_print
    0,                                                      // tp_getattr
    0,                                                      // tp_setattr
    0,                                                      // tp_as_async
    0,                                                      // tp_repr
    0,                                                      // tp_as_number
    0,                                                      // tp_as_sequence
    0,                                                      // tp_as_mapping
    0,                                                      // tp_hash
    0,                                                      // tp_call
    0,                                                      // tp_str
    0,                                                      // tp_getattro
    0,                                                      // tp_setattro
    0,                                                      // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                                     // tp_flags
    "ACDetector objects",                                   // tp_doc
    0,                                                      // tp_traverse
    0,                                                      // tp_clear
    0,                                                      // tp_richcompare
    0,                                                      // tp_weaklistoffset
    0,                                                      // tp_iter
    0,                                                      // tp_iternext
    ACDetector_methods,                                     // tp_methods
    ACDetector_members,                                     // tp_members
    0,                                                      // tp_getset
    0,                                                      // tp_base
    0,                                                      // tp_dict
    0,                                                      // tp_descr_get
    0,                                                      // tp_descr_set
    0,                                                      // tp_dictoffset
    (initproc)ACDetector_init,                              // tp_init
    0,                                                      // tp_alloc
    ACDetector_new,                                         // tp_new
};

static PyModuleDef acdetectormodule = {
    PyModuleDef_HEAD_INIT,
    "acdetector",
    "Word detection module based on ac automata.",
    -1,
    NULL, NULL, NULL, NULL, NULL // Sentinel
};

PyMODINIT_FUNC
PyInit_acdetector(void)
{
    PyObject *m;

    if (PyType_Ready(&acdetector_ACDetectorType) < 0)
        return NULL;

    m = PyModule_Create(&acdetectormodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&acdetector_ACDetectorType);
    PyModule_AddObject(m, "ACDetector", (PyObject *)&acdetector_ACDetectorType);
    return m;
}

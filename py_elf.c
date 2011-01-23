/*
 * This file is part of ccid-utils
 * Copyright (c) 2008 Gianni Tedesco <gianni@scaramanga.co.uk>
 * Released under the terms of the GNU GPL version 3
*/

#include <Python.h>
#include <libelf.h>

static PyObject *pyelf_err_base;
static PyObject *pyelf_err_fmt;

struct pyelf_elf {
	PyObject_HEAD;
	PyObject *file;
	Elf *elf;
};

static void pyelf_error(void)
{
	PyErr_SetString(PyExc_SystemError, elf_errmsg(elf_errno()));
}

static int pyelf_elf_init(struct pyelf_elf *self, PyObject *args, PyObject *kwds)
{
	PyObject *file;
	FILE *tmp;

	if ( !PyArg_ParseTuple(args, "O", &file) )
		return -1;
	
	if ( !PyFile_Check(file) ) {
		PyErr_SetString(PyExc_TypeError, "Expected file object");
		return -1;
	}

	tmp = PyFile_AsFile(file);
	if ( NULL == tmp )
		return -1;

	self->elf = elf_begin(fileno(tmp), ELF_C_READ, NULL);
	if ( NULL == self->elf ) {
		pyelf_error();
		return -1;
	}

	switch(elf_kind(self->elf)) {
	case ELF_K_ELF:
		break;
	default:
		PyErr_SetString(pyelf_err_fmt, "Not a valid ELF object");
		return -1;
	}

	self->file = file;
	Py_INCREF(self->file);

	return 0;
}

static void pyelf_elf_dealloc(struct pyelf_elf *self)
{
	if ( self->elf )
		elf_end(self->elf);
	if ( self->file ) {
		Py_DECREF(self->file);
	}
	self->ob_type->tp_free((PyObject*)self);
}

static PyMethodDef pyelf_elf_methods[] = {
	{NULL, }
};

static PyTypeObject elf_pytype = {
	PyObject_HEAD_INIT(NULL)
	.tp_name = "elf.elf",
	.tp_basicsize = sizeof(struct pyelf_elf),
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_new = PyType_GenericNew,
	.tp_init = (initproc)pyelf_elf_init,
	.tp_dealloc = (destructor)pyelf_elf_dealloc,
	.tp_methods = pyelf_elf_methods,
	.tp_doc = "ELF file",
};

static PyMethodDef methods[] = {
	{NULL, }
};

PyMODINIT_FUNC initelf(void)
{
	PyObject *m;

	if ( elf_version(EV_CURRENT) == EV_NONE )
		return;

	if ( PyType_Ready(&elf_pytype) < 0 )
		return;

	pyelf_err_base = PyErr_NewException("elf.Error",
						PyExc_RuntimeError, NULL);
	if ( NULL == pyelf_err_base )
		return;
	pyelf_err_fmt = PyErr_NewException("elf.FormatError",
						pyelf_err_base, NULL);
	if ( NULL == pyelf_err_fmt )
		return;

	m = Py_InitModule3("elf", methods, "ELF library");
	if ( NULL == m )
		return;

	Py_INCREF(&elf_pytype);
	PyModule_AddObject(m, "Error", pyelf_err_base);
	PyModule_AddObject(m, "FormatError", pyelf_err_fmt);
	PyModule_AddObject(m, "elf", (PyObject *)&elf_pytype);
}
